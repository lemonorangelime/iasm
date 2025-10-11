#include <sys/sendfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <helpers.h>
#include <signals.h>
#include <stdio.h>
#include <regs.h>
#include <asm.h>
#include <main.h>

int decode_type(char * type) {
	if (strcasecmp(type, "float64") == 0) {
		return FLOAT64;
	} else if (strcasecmp(type, "float32") == 0) {
		return FLOAT32;
	} else if (strcasecmp(type, "int128") == 0) {
		return INT128;
	} else if (strcasecmp(type, "int64") == 0) {
		return INT64;
	} else if (strcasecmp(type, "int32") == 0) {
		return INT32;
	}
}

int save_state(char * filename) {
	int fd = open(filename, O_CREAT | O_RDWR, 0664);
	int asmfd = open("/tmp/.temp_asm.asm", O_CREAT | O_RDWR, 0664);
	if ((fd < 0) || (asmfd < 0)) {
		printf("error writing file\n");
		close(fd);
		close(asmfd);
		return 1;
	}
	ssize_t asmsize = fdsize(asmfd);
	if (asmsize < 0) {
		printf("error writing file\n");
		close(fd);
		close(asmfd);
		return 1;
	}
	write(fd, &register_save, sizeof(registers_t));
	write(fd, &return_point, 8);
	write(fd, &fpu_save, 1024); // 
	write(fd, exec_buffer, 4096);
	sendfile(fd, asmfd, NULL, asmsize);
	close(fd);
	close(asmfd);
	return 1;
}

int load_state(char * filename) {
	int fd = open(filename, O_RDWR, 0664);
	int asmfd = open("/tmp/.temp_asm.asm", O_CREAT | O_RDWR, 0664);
	if ((fd < 0) || (asmfd < 0)) {
		printf("could not find file\n");
		close(fd);
		close(asmfd);
		return 1;
	}
	ssize_t asmsize = fdsize(fd) - (sizeof(registers_t) + 8 + 1024 + 4096);
	if (asmsize < 0) {
		printf("file not big enough\n");
		close(fd);
		close(asmfd);
		return 1;
	}
	read(fd, &register_save, sizeof(registers_t));
	read(fd, &return_point, 8);
	read(fd, &fpu_save, 1024);
	read(fd, exec_buffer, 4096);
	sendfile(asmfd, fd, NULL, asmsize);
	close(fd);
	close(asmfd);
	return 1;
}

int print_register(char * regname, char * type) {
	uint64_t * p = lookup_register(regname);
	if (!p) {
		return 1;
	}
	uint64_t mask = lookup_register_mask(regname);
	char * specifier = lookup_register_specifier(regname);
	printf(specifier, *p & mask);
	putchar('\n');
	return 0;
}

int print_fpu_register(char * regname, char * type) {
	fpu_float_t * f = lookup_fpuregister(regname);
	if (!f) {
		return 1;
	}
	printf("%lf\n", fpu_float_to_double(f));
	return 0;
}

int print_xmm_register(char * regname, char * type) {
	void * xp = lookup_xmmregister(regname);
	if (!xp) {
		return 1;
	}
	// future: allow this `print FLOAT64 xmm0` thing for general registers aswell?
	print_xmm(xp, *type ? decode_type(type) : xmm_type);
	putchar('\n');
	return 0;
}

int print_function(char * regname, char * type) {
	int errors = 0;
	errors += print_register(regname, type);
	errors += print_fpu_register(regname, type);
	errors += print_xmm_register(regname, type);
	return !(errors == 3);
}

int execute_builtins(char * line) {
	char buffer[255] = {0};
	char buffer2[255] = {0};
	if (strcmp(line, "exit") == 0) {
		return 2;
	}
	if (strcmp(line, "dump") == 0) {
		dump_registers();
		return 1;
	}
	if (strcmp(line, "freeze") == 0) {
		paused = 1;
		return 1;
	}
	if (strcmp(line, "unfreeze") == 0) {
		paused = 0;
		return 1;
	}
	if (strcmp(line, "help") == 0) {
		puts("exit        |  exit program");
		puts("dump        |  print all registers");
		puts("freeze      |  pause execution");
		puts("unfreeze    |  unpause execution");
		puts("xmm_type    |  set default type for xmm registers (INT128/64/32 / FLOAT64/32)");
		puts("save_state  |  save state to file (filename)");
		puts("load_state  |  load state from file (filename)");
		puts("print       |  print value of register (register)");
		return 1;
	}
	if (sscanf(line, "xmm_type %s", buffer) > 0) {
		xmm_type = decode_type(buffer);
		return 1;
	}
	if (sscanf(line, "save_state %s", buffer) > 0) {
		return save_state(buffer);
	}
	if (sscanf(line, "load_state %s", buffer) > 0) {
		return load_state(buffer);
	}
	if (sscanf(line, "print %s%s", buffer, buffer2) > 0) {
		int swap = *buffer;
		char * name = swap ? buffer : buffer2;
		char * type = swap ? buffer2 : buffer;
		return print_function(name, type);
	}
	return 0;
}
