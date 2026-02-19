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
#include <iasm/features.h>
#include <iasm/platform.h>
#include <iasm/types.h>

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

ssize_t my_fdlinesize(int fd) {
	ssize_t cur = lseek(fd, 0, SEEK_CUR);
	ssize_t end = lseek(fd, 0, SEEK_END);
	lseek(fd, cur, SEEK_SET);

	char c = 0;
	ssize_t p = cur;
	while (p < end && c != '\n') {
		read(fd, &c, 1);
		if (c == '\n') { break; }
		p++;
	}

	lseek(fd, cur, SEEK_SET);
	return p - cur;
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

void print_ansii_colour(uint32_t colour) {
	if (!allow_colour) { return; }
	printf("\x1b[38;2;%d;%d;%dm", (colour >> 16) & 0xff, (colour >> 8) & 0xff, colour & 0xff);
}

void print_ansii_reset() {
	if (!allow_colour) { return; }
	printf("\x1b[0m");
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
int resolve_register_or_label(char * name, uintptr_t * p, int * size) {
	uintptr_t address = 0;
	if (resolve_label(name, &address)) {
		uint64_t * regp = lookup_register(name);
		if (!regp) {
			return 1;
		}
		*size = lookup_register_size(name);

		int ptrsize = sizeof(uintptr_t);
		memcpy(p, regp, (ptrsize > *size) ? ptrsize : *size);
		return 0;
	}
	*p = address;
	*size = sizeof(uintptr_t);
	return 0;
}

// like anything at all
int resolve_any_register_or_label(char * name, void * p, int * size, int * type) {
	uintptr_t address = 0;
	if (resolve_label(name, &address)) {
		uint64_t * regp = lookup_register(name);
		if (!regp) {
			// resolve some bullshit SSE or FPU or AVX or AMX or APX or whatever the fuck register
			void * reg = NULL;
			switch (platform) {
				case PLATFORM_X86_64:
				case PLATFORM_X86:
					reg = lookup_fpuregister(name);
					if (reg) {
						*size = 10;
						*type = FLOAT80;
						break;
					}
					reg = lookup_mmxregister(name);
					if (reg) {
						*size = 8;
						*type = FLOAT64;
						break;
					}
					reg = lookup_xmmregister(name);
					if (reg) {
						*size = 16;
						*type = xmm_type;
						break;
					}
					reg = lookup_ymmregister(name);
					if (reg) {
						*size = 32;
						*type = ymm_type;
						read_ymm(reg, p);
						return 0;
					}
					reg = lookup_zmmregister(name);
					if (reg) {
						*size = 64;
						*type = zmm_type;
						read_ymm(reg, p);
						return 0;
					}
					reg = lookup_tmmregister(name);
					if (reg) {
						*size = 1024;
						*type = tmm_type;
						break;
					}
					reg = NULL;
					break;
			}
			if (reg) {
				memcpy(p, reg, *size);
				return 0;
			}
			return 1;
		}
		*size = lookup_register_size(name);
		*type = lookup_register_type(name);
		memcpy(p, regp, *size);
		return 0;
	}
	*(uintptr_t *) p = address;
	*size = sizeof(uintptr_t);
	return 0;
}
