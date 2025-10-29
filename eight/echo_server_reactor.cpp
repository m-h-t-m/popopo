// echo_server_reactor.cpp
#include "EventLoop.h"
#include "Channel.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <memory>

using namespace std;

const int kBufferSize = 1024;

// 创建非阻塞套接字
int createNonblockingSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        cerr << "Failed to create socket" << endl;
        exit(1);
    }
    return sockfd;
}

// 绑定地址和端口
void bindAddress(int sockfd, const string& ip, uint16_t port) {
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Failed to bind address" << endl;
        exit(1);
    }
}

// 接受新连接的回调
void acceptConnection(int listenfd, const shared_ptr<EventLoop>& loop) {
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &clientLen);

        if (connfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 所有连接都已接受
                break;
            } else {
                cerr << "Error in accepting connection" << endl;
                break;
            }
        } else {
            // 将新连接设置为非阻塞
            int flags = fcntl(connfd, F_GETFL, 0);
            fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

            cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) 
                 << ":" << ntohs(clientAddr.sin_port) << ", fd=" << connfd << endl;

            // 为新连接创建 Channel 并加入事件循环
            auto connChannel = make_shared<Channel>(loop.get(), connfd);
            
            // 设置读回调：回显数据
            connChannel->setReadCallback([connChannel, connfd, loop]() {
                char buffer[kBufferSize];
                ssize_t n = read(connfd, buffer, sizeof(buffer));
                if (n > 0) {
                    // 回显
                    write(connfd, buffer, n);
                } else if (n == 0) {
                    // 客户端关闭连接
                    cout << "Connection closed by peer, fd=" << connfd << endl;
                    // 清理资源
                    loop->removeChannel(connChannel);
                    close(connfd);
                } else {
                    // 错误
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        cerr << "Error reading from fd=" << connfd << endl;
                        loop->removeChannel(connChannel);
                        close(connfd);
                    }
                }
            });

            // 设置关闭和错误回调
            connChannel->setCloseCallback([connfd, loop, connChannel]() {
                cout << "Channel close callback for fd=" << connfd << endl;
                loop->removeChannel(connChannel);
                close(connfd);
            });
            connChannel->setErrorCallback([connfd, loop, connChannel]() {
                cerr << "Channel error on fd=" << connfd << endl;
                loop->removeChannel(connChannel);
                close(connfd);
            });

            // 注册读事件
            connChannel->enableReading();
            loop->updateChannel(connChannel);
        }
    }
}

int main() {
    int listenfd = createNonblockingSocket();
    bindAddress(listenfd, "127.0.0.1", 8888);

    if (listen(listenfd, SOMAXCONN) < 0) {
        cerr << "Listen failed" << endl;
        return 1;
    }

    cout << "Echo server listening on 127.0.0.1:8888" << endl;

    // 创建事件循环
    unique_ptr<EventLoop> loop = make_unique<EventLoop>();

    // 为监听套接字创建 Channel
    auto listenChannel = make_shared<Channel>(loop.get(), listenfd);
    
    // 设置读回调（有新连接到达）
    listenChannel->setReadCallback(std::bind(acceptConnection, listenfd, std::ref(loop)));
    
    // 注册监听套接字的可读事件
    listenChannel->enableReading();
    loop->updateChannel(listenChannel);

    // 启动事件循环
    loop->loop();

    close(listenfd);
    return 0;
}