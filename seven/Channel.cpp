#include "Channel.h"
#include "EventLoop.h"      // ✅ 正确：需要 EventLoop 定义
#include <sys/epoll.h>
#include <iostream>

// 构造函数
Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0) {}

int Channel::fd() const { return fd_; }
uint32_t Channel::events() const { return events_; }
void Channel::set_events(uint32_t events) { events_ = events; }
uint32_t Channel::revents() const { return revents_; }
void Channel::set_revents(uint32_t revents) { revents_ = revents; }

// enable/disable 方法
void Channel::enableReading() {
    events_ |= EPOLLIN;
    loop_->updateChannel(this);  // 需要 EventLoop 定义 → 已包含 EventLoop.h ✅
}

void Channel::disableReading() {
    events_ &= ~EPOLLIN;
    loop_->updateChannel(this);
}

void Channel::enableWriting() {
    events_ |= EPOLLOUT;
    loop_->updateChannel(this);
}

void Channel::disableWriting() {
    events_ &= ~EPOLLOUT;
    loop_->updateChannel(this);
}

void Channel::disableAll() {
    events_ = 0;
    loop_->updateChannel(this);
}

// 回调设置
void Channel::setReadCallback(std::function<void()> cb) {
    read_callback_ = std::move(cb);
}

void Channel::setWriteCallback(std::function<void()> cb) {
    write_callback_ = std::move(cb);
}

// 处理事件
void Channel::handleEvent() {
    if (revents_ & EPOLLIN && read_callback_) {
        read_callback_();
    }
    if (revents_ & EPOLLOUT && write_callback_) {
        write_callback_();
    }
}