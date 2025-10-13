#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <regs.h>
#include <asm.h>

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

int resolve_label(char * label, uint64_t * p) {
	char statement[255];
	void * buffer = NULL;
	ssize_t size = 0;
	int stat = sprintf(statement, "dq %s", label);
	if (stat <= 0) {
		return 1;
	}
	stat = assemble(statement, &buffer, &size);
	asm_rewind();
	if ((stat != 0) || (size != 8)) {
		return 1;
	}
	*p = *(uint64_t *) buffer;
	return 0;
}
