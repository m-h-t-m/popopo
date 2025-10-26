#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main() {
    // 1. 创建 socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "创建 socket 失败" << std::endl;
        return 1;
    }

    // 2. 绑定地址
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有网卡
    addr.sin_port = htons(8080);        // 端口 8080

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "bind 失败" << std::endl;
        close(server_fd);
        return 1;
    }

    // 3. 监听
    if (listen(server_fd, 10) == -1) {
        std::cerr << "listen 失败" << std::endl;
        close(server_fd);
        return 1;
    }

    std::cout << "服务器启动，监听 8080 端口..." << std::endl;

    while (true) {
        // 4. 接受连接
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            std::cerr << "accept 失败" << std::endl;
            continue;
        }

        std::cout << "客户端连接: " 
                  << inet_ntoa(client_addr.sin_addr) 
                  << ":" << ntohs(client_addr.sin_port) 
                  << std::endl;

        // 5. 读取并回显
        char buffer[1024];
        while (true) {
            ssize_t n = read(client_fd, buffer, sizeof(buffer));
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "收到: " << buffer;
                write(client_fd, buffer, n);  // 回显
            } else {
                std::cout << "客户端断开连接" << std::endl;
                break;
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
