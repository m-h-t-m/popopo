// Channel.h
#pragma once
#include <functional>
#include <memory>

class EventLoop;

/**
 * @brief Channel 类封装了一个文件描述符(fd)以及其上发生的事件和对应的处理回调。
 * 它是 Reactor 模式中的基本单位，代表一个可以被 EventLoop 监控的 I/O 通道。
 */
class Channel {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void()>;

    /**
     * @brief 构造函数
     * @param loop 所属的事件循环
     * @param fd 要监控的文件描述符
     */
    Channel(EventLoop* loop, int fd);
    
    ~Channel();

    // 禁止拷贝
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    // 移动构造和赋值（允许移动）
    Channel(Channel&&) noexcept;
    Channel& operator=(Channel&&) noexcept;

    /**
     * @brief 将此 Channel 注册到其 EventLoop 的 epoll 实例中
     */
    void enableReading();

    /**
     * @brief 从其 EventLoop 的 epoll 实例中注销此 Channel
     */
    void disableAll();

    /**
     * @brief 处理发生的事件
     * 这个函数由 EventLoop 在检测到事件后调用
     */
    void handleEvent();

    // 设置回调函数
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    int fd() const { return fd_; }
    int events() const { return events_; }
    int revents() const { return revents_; }
    bool isNoneEvent() const { return events_ == 0; }

    // 用于调试
    int index() { return index_; }
    void setIndex(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }

private:
    static const int kNoneEvent;  // 无事件
    static const int kReadEvent;  // 可读事件
    static const int kWriteEvent; // 可写事件

    EventLoop* loop_;             // 所属的事件循环
    const int fd_;                // 文件描述符
    int events_;                  // 用户感兴趣的事件 (注册给 epoll 的)
    int revents_;                 // 实际发生的事件 (epoll 返回的)
    int index_;                   // 在 EventLoop 中的状态: -1 未添加, 1 已添加, 2 已删除

    // 事件回调
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

// 为了方便，定义一个共享指针类型别名
using ChannelPtr = std::shared_ptr<Channel>;