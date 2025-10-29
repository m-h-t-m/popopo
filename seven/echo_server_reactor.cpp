#include "EventLoop.h"
#include "Channel.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <functional>  

int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return 0;
}

int main() {
    EventLoop loop;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed\n";
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        std::cerr << "Listen failed\n";
        close(listen_fd);
        return -1;
    }

    setNonBlocking(listen_fd);

    Channel listen_channel(&loop, listen_fd);

    listen_channel.setReadCallback([&loop, &listen_channel, listen_fd]() {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd;

        while ((client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len)) >= 0) {
            setNonBlocking(client_fd);

            std::cout << "New connection from "
                      << inet_ntoa(client_addr.sin_addr)
                      << ":" << ntohs(client_addr.sin_port)
                      << " (fd=" << client_fd << ")\n";

            Channel* client_channel = new Channel(&loop, client_fd);

            client_channel->setReadCallback([client_channel, client_fd]() {
                char buf[1024];
                ssize_t n = read(client_fd, buf, sizeof(buf));
                if (n > 0) {
                    write(client_fd, buf, n);
                } else if (n == 0) {
                    std::cout << "Client closed connection (fd=" << client_fd << ")\n";
                    close(client_fd);
                    delete client_channel;
                } else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        std::cerr << "Read error on fd=" << client_fd << "\n";
                        close(client_fd);
                        delete client_channel;
                    }
                }
            });

            client_channel->enableReading();
        }

        if (errno != EWOULDBLOCK) {
            std::cerr << "Accept error: " << strerror(errno) << "\n";
        }
    });

    listen_channel.enableReading();

    std::cout << "Echo server is running on port 8080...\n";
    std::cout << "Press Ctrl+C to exit.\n";

    loop.loop();

    close(listen_fd);
    return 0;
}