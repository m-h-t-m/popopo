// EventLoop.cpp
#include "EventLoop.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cassert>
#include <thread>
#include <cstring>

using namespace std;

EventLoop::EventLoop()
    : epollfd_(epoll_create1(EPOLL_CLOEXEC)),
      looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(std::this_thread::get_id()),
      activeChannels_(),
      channels_(),
      wakeupFd_(-1),
      wakeupChannel_(nullptr)
{
    if (epollfd_ < 0) {
        cerr << "Failed to create epoll instance" << endl;
        exit(1);
    }

    // 创建用于唤醒的管道
    int pipefds[2];
    if (pipe(pipefds) < 0) {
        cerr << "Failed to create wakeup pipe" << endl;
        exit(1);
    }
    wakeupFd_ = pipefds[0]; // 读端
    int wakeupWriteFd = pipefds[1]; // 写端

    // 将 wakeupFd_ 设置为非阻塞
    int flags = fcntl(wakeupFd_, F_GETFL, 0);
    fcntl(wakeupFd_, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(wakeupWriteFd, F_GETFL, 0);
    fcntl(wakeupWriteFd, F_SETFL, flags | O_NONBLOCK);

    // 创建监听 wakeupFd_ 的 Channel
    wakeupChannel_ = std::make_shared<Channel>(this, wakeupFd_);
    wakeupChannel_->setReadCallback([wakeupWriteFd]() {
        uint64_t one = 1;
        ssize_t n = write(wakeupWriteFd, &one, sizeof(one));
        if (n != sizeof(one)) {
            cerr << "EventLoop::wakeup writes " << n << " bytes instead of 8" << endl;
        }
    });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    ::close(epollfd_);
    ::close(wakeupFd_);
    if (wakeupChannel_) {
        wakeupChannel_->disableAll();
        wakeupChannel_.reset();
    }
}

void EventLoop::loop() {
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    quit_ = false;

    cout << "EventLoop " << this << " start looping" << endl;

    while (!quit_) {
        activeChannels_.clear();
        poll();

        // 处理所有活跃的 Channel
        for (const auto& channel : activeChannels_) {
            handleEvent(channel);
        }

        doPendingFunctors();
    }

    cout << "EventLoop " << this << " stop looping" << endl;
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // 如果不是在 EventLoop 线程中调用 quit，需要唤醒它
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::poll() {
    int numEvents = epoll_wait(epollfd_, 
                               reinterpret_cast<struct epoll_event*>(activeChannels_.data()), 
                               static_cast<int>(activeChannels_.size()), 
                               kPollTimeMs);
    int savedErrno = errno;

    if (numEvents > 0) {
        // 调整 activeChannels_ 的大小以容纳实际的事件数量
        activeChannels_.resize(numEvents);
        // Note: 实际填充 activeChannels_ 的工作应该在更底层完成，
        // 这里简化了，假设 epoll_wait 直接填充了我们的 vector。
        // 在真实实现中，可能需要一个临时缓冲区然后映射到 channels_。
        // 为了演示目的，我们假设有一个机制来获取正确的 Channel 指针。
        // 更好的做法是在 epoll_event 的 data.ptr 中存储 Channel* 或 weak_ptr。
        // 这里省略了这部分复杂性。
    } else if (numEvents == 0) {
        // 超时，正常情况
    } else {
        // 错误
        if (savedErrno != EINTR) {
            cerr << "EventLoop::poll() error" << endl;
        }
    }
}

void EventLoop::handleEvent(std::shared_ptr<Channel> channel) {
    // 从 channels_ 中查找，确保 Channel 仍然存在且有效
    auto it = channels_.find(channel->fd());
    if (it != channels_.end() && it->second.get() == channel.get()) {
        channel->handleEvent();
    }
    // 否则，Channel 可能已经被移除
}

void EventLoop::updateChannel(const std::shared_ptr<Channel>& channel) {
    assert(channel->ownerLoop() == this);
    assert(isInLoopThread());

    const int fd = channel->fd();
    const int index = channel->index();

    if (index == -1 || index == 2) {
        // 新增或重新添加
        if (index == -1) {
            channels_[fd] = channel;
        } else { // index == 2
            assert(channels_.find(fd) != channels_.end());
            channels_[fd] = channel;
        }
        channel->setIndex(1); // 标记为已添加

        struct epoll_event event;
        event.events = channel->events();
        event.data.ptr = channel.get(); // 存储 Channel 指针，便于快速查找
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0) {
            cerr << "epoll_ctl add error fd=" << fd << endl;
            // 处理错误...
        }
    } else {
        // 修改
        assert(index == 1);
        struct epoll_event event;
        event.events = channel->events();
        event.data.ptr = channel.get();
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0) {
            cerr << "epoll_ctl mod error fd=" << fd << endl;
            // 处理错误...
        }
    }
}

void EventLoop::removeChannel(const std::shared_ptr<Channel>& channel) {
    assert(channel->ownerLoop() == this);
    assert(isInLoopThread());

    const int fd = channel->fd();
    channels_.erase(fd);
    channel->setIndex(-2); // 标记为已删除

    struct epoll_event event;
    epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event);
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    // 如果不是在 EventLoop 线程中，或者正在执行 pending functors，需要唤醒
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

bool EventLoop::isInLoopThread() const {
    return std::this_thread::get_id() == threadId_;
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }

    callingPendingFunctors_ = false;
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        cerr << "EventLoop::wakeup writes " << n << " bytes instead of 8" << endl;
    }
}