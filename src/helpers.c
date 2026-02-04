#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <iasm/regs.h>
#include <iasm/asm.h>
#include <iasm/setup.h>

char * file_charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
void tmpname(char * path) {
	char * prefix = "/tmp/.iasm-";
	strcpy(path, prefix);
	path += strlen(prefix);

	int i = 16;
	int characters = strlen(file_charset);
	while (i--) {
		*path++ = file_charset[random() % characters];
	}
	strcpy(path, ".tmp");
}

ssize_t my_fdsize(int fd) {
	ssize_t cur = lseek(fd, 0, SEEK_CUR);
	ssize_t end = lseek(fd, 0, SEEK_END);
	lseek(fd, cur, SEEK_SET);
	return end;
}

ssize_t asm_src_fdsize() {
	int fd = open(asm_src_path, O_RDWR, 0664);
	if (!fd) {
		return 0;
	}
	ssize_t size = my_fdsize(fd);
	close(fd);
	return size;
}

void asm_src_readall(void * p) {
	int fd = open(asm_src_path, O_RDWR, 0664);
	if (!fd) {
		return;
	}
	ssize_t size = my_fdsize(fd);
	read(fd, p, size);
	close(fd);
}

void asm_src_writeall(void * p, size_t size) {
	int fd = open(asm_src_path, O_RDWR, 0664);
	if (!fd) {
		return;
	}
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, size);
	lseek(fd, 0, SEEK_SET);
	write(fd, p, size);
	close(fd);
}

int resolve_label(char * label, uintptr_t * p) {
	char statement[255];
	void * buffer = NULL;
	ssize_t size = 0;
	int stat = sprintf(statement, "dq %s", label);
	if (stat <= 0) {
		return 1;
	}
	stat = assemble(statement, &buffer, &size, NULL);
	asm_rewind();
	if ((stat != 0) || (size != 8)) {
		return 1;
	}
	*p = *(uintptr_t *) buffer;
	return 0;
}

// ONLY general registers, allowing `x/1q xmm0` would be perposterous and utterly absurd
int resolve_register_or_label(char * name, uintptr_t * p) {
	uintptr_t address = 0;
	if (resolve_label(name, &address)) {
		uint64_t * regp = lookup_register(name);
		if (!regp) {
			return 1;
		}
		int size = lookup_register_size(name);
		int ptrsize = sizeof(uintptr_t);
		memcpy(p, regp, (ptrsize > size) ? ptrsize : size);
		return 0;
	}
	*p = address;
	return 0;
}
