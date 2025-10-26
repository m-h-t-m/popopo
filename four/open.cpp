#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main() {
	int src_fd = open("input.txt", O_RDONLY);
	if(src_fd == -1) {
		std::cout<< "no open file!" << std::endl;
		return 1;
	}
	int dst_fd = open("output.txt",O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(dst_fd == -1) {
		std::cerr << "no build file" << std::endl;
	}
	char buffer[4096];
	ssize_t bytesRead;
	while((bytesRead = read(src_fd, buffer, sizeof(buffer)))>0) {
		write(dst_fd, buffer, bytesRead);
		}
	close(src_fd);
	close(dst_fd);
	std::cout<<"file cp comliet!" << std::endl;
	return 0;
}
