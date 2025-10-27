#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "创建 socket 失败" << std::endl;
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "连接失败" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "连接成功！输入消息（输入 'quit' 退出）：" << std::endl;

    char buffer[1024];
    while (std::cin.getline(buffer, sizeof(buffer))) {
        if (strcmp(buffer, "quit") == 0) break;

        write(sock, buffer, strlen(buffer));
        
        ssize_t n = read(sock, buffer, sizeof(buffer));
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "服务器回复: " << buffer << std::endl;
        }
    }

    close(sock);
    return 0;
}
