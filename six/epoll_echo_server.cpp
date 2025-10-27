#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

const int MAX_EVENTS = 1024;
const int PORT = 8080;

void set_nonblocking(int fd) {

}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "socket failed" << std::endl;
        return 1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "bind failed" << std::endl;
        close(server_fd);
        return 1;
    }
    if(listen(server_fd, 10) == -1) {
        std::cerr << "listen failed" << std::endl;
        close(server_fd);
        return 1;
    }
    int epfd = epoll_create1(0);
    if(epfd == -1) {
        std::cerr << "epoll_create failed" << std::endl;
        close(server_fd);
        return 1;
    }
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        std::cerr << "epoll_ctl add failed" << std::endl;
        close(server_fd);
        close(epfd);
        return 1;
    }
    std::cout << "epoll server running on port " << PORT << std::endl;
    
    epoll_event events[MAX_EVENTS];
    while (true) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if(n ==-1) {
            std::cerr << "epoll_wait failed" << std::endl;
            break;
        }
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if(fd == server_fd) {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr,&len);
                if (client_fd != -1) {
                    std::cout << "New client: " 
                              << inet_ntoa(client_addr.sin_addr) 
                              << ":" << ntohs(client_addr.sin_port) 
                              << std::endl;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
            }
           } else {
                char buffer[1024];
                ssize_t bytes = read(fd, buffer, sizeof(buffer));
                if(bytes > 0) {
                    buffer[bytes] = '\0';
                    std::cout<< "Recv from " << fd << ": " << buffer;
                    write(fd, buffer, bytes);
                } else {
                    std::cout << "Client " << fd << " disconnected" << std::endl;\
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                }
             }
        }
    }
    close(server_fd);
    close(epfd);
    return 0;
}

























