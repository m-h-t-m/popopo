#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>

int main(int argc, char* argv[]) {
	if(argc != 3) {
		std::cerr << "用法: " << argv[0] << " <源文件> <目标文件>\n";
		return 1;
	}
	
	const char* src = argv[1];
	const char* dest = argv[2];
	
	int fd_src = open(src,O_RDONLY);
	if(fd_src == -1) {
	    std::cerr << "无法打开源文件 '" << src << "': " << strerror(errno) << "\n";
          return 1;
	}
	int fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	 if (fd_dest == -1) {
          std::cerr << "无法创建目标文件 '" << dest << "': " << strerror(errno) << "\n";
         close(fd_src);
          return 1;
      }
    pid_t pid = fork();
    if (pid == -1) {
          std::cerr << "fork 失败: " << strerror(errno) << "\n";
          close(fd_src);
          close(fd_dest);
        return 1;
      }
    if (pid == 0) {
        char buffer[4096];
        ssize_t n;
        while ((n = read(fd_src, buffer, sizeof(buffer))) > 0){
            ssize_t written = 0;
    while (written < n) {
        ssize_t w = write(fd_dest, buffer + written, n - written);
        if (w == -1) {
                      std::cerr << "写入失败: " << strerror(errno) << "\n";
                      close(fd_src);
                      close(fd_dest);
                      return 1;
                  }
        written += w;
    }
}
    if (n == -1) {
              std::cerr << "读取失败: " << strerror(errno) << "\n";
              close(fd_src);
              close(fd_dest);
                            return 1;
          }
    close(fd_src);
    close(fd_dest);
    return 0;
}else{
    int status;
    wait(&status);
          if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
              std::cout << "✅ 拷贝完成: " << src << " → " << dest << "\n";
          } else {
              std::cerr << "❌ 子进程拷贝失败\n";
          }
    close(fd_src);
    close(fd_dest);
}
    return 0;
}

