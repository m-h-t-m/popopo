#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <cstdint>

class EventLoop; // 前向声明

class Channel {
public:
    Channel(EventLoop* loop, int fd);

    int fd() const;
    uint32_t events() const;
    void set_events(uint32_t events);
    uint32_t revents() const;
    void set_revents(uint32_t revents);

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

    void setReadCallback(std::function<void()> cb);
    void setWriteCallback(std::function<void()> cb);

    void handleEvent();

private:
    EventLoop* loop_;
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    std::function<void()> read_callback_;
    std::function<void()> write_callback_;
};

#endif // CHANNEL_H