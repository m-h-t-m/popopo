#include <unistd.h>     // fork, pipe, read, write, close, wait
#include <iostream>
#include <cstring>      // strlen
#include <sys/wait.h>   // wait

int main() {
    int pipefd[2];  // 文件描述符数组：pipefd[0] 读端，pipefd[1] 写端

    // 创建管道
    if (pipe(pipefd) == -1) {
        std::cerr << "创建管道失败" << std::endl;
        return 1;
    }

    pid_t pid = fork();  // 创建子进程

    if (pid == 0) {
        // 子进程：读数据
        close(pipefd[1]);  // 关闭写端
        char buffer[128];
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer));
        if (n > 0) {
            buffer[n] = '\0';  // 添加字符串结束符
            std::cout << "子进程收到: " << buffer << std::endl;
        }
        close(pipefd[0]);  // 关闭读端
    } else {
        // 父进程：写数据
        close(pipefd[0]);  // 关闭读端
        const char* msg = "Hello from parent";
        write(pipefd[1], msg, strlen(msg));  // 写入消息
        close(pipefd[1]);  // 关闭写端
        wait(nullptr);     // 等待子进程结束
    }

    return 0;
}
