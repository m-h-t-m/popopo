#include "EventLoop.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <vector>
#include <unordered_map>  // ✅ 多余但无害

EventLoop::EventLoop()
    : looping_(false),
      epoll_fd_(epoll_create1(0)),
      events_(64) {
    if (epoll_fd_ < 0) {
        std::cerr << "Failed to create epoll instance\n";
        exit(1);
    }
}

EventLoop::~EventLoop() {
    close(epoll_fd_);
}

void EventLoop::loop() {
    looping_ = true;
    std::vector<Channel*> activeChannels;

    while (looping_) {
        activeChannels.clear();
        int numEvents = epoll_wait(epoll_fd_, events_.data(), events_.size(), 10000);
        if (numEvents < 0) {
            if (errno == EINTR) continue;
            std::cerr << "epoll_wait error\n";
            break;
        }

        if (numEvents == static_cast<int>(events_.size())) {
            events_.resize(events_.size() * 2);
        }

        fillActiveChannels(numEvents, activeChannels);

        for (Channel* ch : activeChannels) {
            ch->handleEvent();
        }
    }
}

void EventLoop::quit() {
    looping_ = false;
}

void EventLoop::updateChannel(Channel* channel) {
    struct epoll_event ev;
    ev.events = channel->events();
    ev.data.ptr = channel;

    int op = EPOLL_CTL_ADD;
    auto it = channels_.find(channel->fd());
    if (it != channels_.end()) {
        op = EPOLL_CTL_MOD;
    } else {
        channels_[channel->fd()] = channel;
    }

    if (epoll_ctl(epoll_fd_, op, channel->fd(), &ev) < 0) {
        std::cerr << "epoll_ctl error for fd=" << channel->fd() << "\n";
        exit(1);
    }
}

void EventLoop::removeChannel(Channel* channel) {
    channels_.erase(channel->fd());
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->fd(), nullptr);
}

void EventLoop::fillActiveChannels(int numEvents, std::vector<Channel*>& activeChannels) {
    for (int i = 0; i < numEvents; ++i) {
        Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
        ch->set_revents(events_[i].events);
        activeChannels.push_back(ch);
    }
}