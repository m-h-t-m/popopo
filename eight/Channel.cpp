// Channel.cpp
#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>
#include <iostream>
#include <cassert>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1) // 初始状态为未添加
{
    assert(loop_ != nullptr);
}

Channel::~Channel() {
    assert(!handlingEvent_);
    // 通常不需要在这里显式关闭 fd，由更高层管理
}

Channel::Channel(Channel&& other) noexcept
    : loop_(other.loop_),
      fd_(other.fd_),
      events_(other.events_),
      revents_(other.revents_),
      index_(other.index_),
      readCallback_(std::move(other.readCallback_)),
      writeCallback_(std::move(other.writeCallback_)),
      closeCallback_(std::move(other.closeCallback_)),
      errorCallback_(std::move(other.errorCallback_))
{
    // 移动后，原对象的资源已被转移，需要重置其状态
    other.index_ = -1; // 标记为无效
    // 注意: loop_ 和 fd_ 是基础类型或指针，直接复制没问题
    // 回调函数已通过 move 转移
}

Channel& Channel::operator=(Channel&& other) noexcept {
    if (this != &other) {
        loop_ = other.loop_;
        fd_ = other.fd_;
        events_ = other.events_;
        revents_ = other.revents_;
        index_ = other.index_;

        readCallback_ = std::move(other.readCallback_);
        writeCallback_ = std::move(other.writeCallback_);
        closeCallback_ = std::move(other.closeCallback_);
        errorCallback_ = std::move(other.errorCallback_);

        other.index_ = -1;
    }
    return *this;
}

void Channel::enableReading() {
    events_ |= kReadEvent;
    update();
}

void Channel::disableAll() {
    events_ = kNoneEvent;
    update();
}

void Channel::update() {
    // 调用所属 EventLoop 的更新函数
    loop_->updateChannel(shared_from_this());
}

void Channel::handleEvent() {
    // 简单的事件分发
    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (readCallback_) {
            readCallback_();
        }
    }
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
    if (revents_ & EPOLLHUP) {
        if (closeCallback_) {
            closeCallback_();
        }
    }
    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
}