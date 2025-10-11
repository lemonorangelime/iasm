#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

ssize_t fdsize(int fd) {
	ssize_t cur = lseek(fd, 0, SEEK_CUR);
	ssize_t end = lseek(fd, 0, SEEK_END);
	lseek(fd, cur, SEEK_SET);
	return end;
}

ssize_t fsize(char * filename) {
	int fd = open(filename, O_RDONLY, 0664);
	ssize_t size = fdsize(fd);
	close(fd);
	return size;
}
