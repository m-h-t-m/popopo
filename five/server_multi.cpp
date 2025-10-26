// server_multi.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

// 子进程处理函数
void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected.\n";
            break;
        }
        std::cout << "Client says: " << buffer;
        write(client_fd, buffer, bytes_read);
    }
    close(client_fd);
    exit(0); // 子进程退出
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 1. 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. 地址复用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. 绑定
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. 监听
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Multi-client Echo Server running on port " << PORT << "...\n";

    // 5. 主循环：接受连接并 fork 子进程
    while (true) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        std::cout << "New client connected.\n";

        // fork 子进程处理客户端
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            close(server_fd); // 子进程不需要监听套接字
            handle_client(client_fd);
        } else if (pid > 0) {
            // 父进程
            close(client_fd); // 父进程关闭客户端套接字
        } else {
            // fork 失败
            perror("Fork failed");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
