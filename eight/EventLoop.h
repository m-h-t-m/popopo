// EventLoop.h
#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

class Channel;

/**
 * @brief EventLoop 类实现了 Reactor 模式的事件循环。
 * 它负责运行一个无限循环，等待并分发 I/O 事件给对应的 Channel。
 */
class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // 禁止拷贝
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    /**
     * @brief 启动事件循环
     * 这是一个阻塞调用，直到 quit() 被调用。
     */
    void loop();

    /**
     * @brief 停止事件循环
     * 通常在另一个线程中调用以退出 loop()。
     */
    void quit();

    /**
     * @brief 更新一个 Channel 的注册状态（添加、修改、删除）
     * 这个函数是线程安全的，可以在其他线程中调用。
     * @param channel 需要更新的 Channel 的共享指针
     */
    void updateChannel(const std::shared_ptr<Channel>& channel);

    /**
     * @brief 删除一个 Channel
     * 通常由 Channel 自身或更高层逻辑触发。
     * @param channel 需要删除的 Channel 的共享指针
     */
    void removeChannel(const std::shared_ptr<Channel>& channel);

    /**
     * @brief 在事件循环线程中执行一个函数
     * 如果当前线程是事件循环线程，则立即执行；否则将函数放入队列，等待事件循环处理。
     * @param cb 要执行的函数
     */
    void runInLoop(Functor cb);

    /**
     * @brief 将一个函数加入到待执行队列中
     * 供内部或其他组件使用。
     * @param cb 要排队的函数
     */
    void queueInLoop(Functor cb);

    /**
     * @brief 检查当前线程是否是创建此 EventLoop 的线程
     * @return true 如果是，false 否则
     */
    bool isInLoopThread() const;

private:
    static const int kPollTimeMs = 10000; // epoll_wait 超时时间 (ms)

    void poll();                          // 执行 epoll_wait
    void handleEvent(std::shared_ptr<Channel> channel); // 处理单个 Channel 的事件
    void doPendingFunctors();             // 执行待处理的函数

    int epollfd_;                          // epoll 实例的文件描述符
    bool looping_;                         // 是否正在循环
    bool quit_;                            // 是否请求退出
    bool callingPendingFunctors_;         // 是否正在执行待处理函数
    int threadId_;                         // 创建 EventLoop 的线程 ID

    std::vector<std::shared_ptr<Channel>> activeChannels_; // 当前活跃的 Channel 列表 (epoll_wait 返回的)
    std::unordered_map<int, std::shared_ptr<Channel>> channels_; // fd -> Channel 映射

    int wakeupFd_;                        // 用于唤醒事件循环的管道读端 fd
    std::shared_ptr<Channel> wakeupChannel_; // 用于监听 wakeupFd_ 的 Channel

    std::vector<Functor> pendingFunctors_; // 待执行的函数队列
    mutable std::mutex mutex_;            // 保护 pendingFunctors_ 的互斥锁
};