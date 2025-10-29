#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include <unordered_map>

class Channel;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    bool looping_;
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
    std::unordered_map<int, Channel*> channels_;

    void fillActiveChannels(int numEvents, std::vector<Channel*>& activeChannels);
};

#endif // EVENTLOOP_H