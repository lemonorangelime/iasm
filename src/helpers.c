#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <regs.h>
#include <asm.h>

ssize_t my_fdsize(int fd) {
	ssize_t cur = lseek(fd, 0, SEEK_CUR);
	ssize_t end = lseek(fd, 0, SEEK_END);
	lseek(fd, cur, SEEK_SET);
	return end;
}

int resolve_label(char * label, uintptr_t * p) {
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
