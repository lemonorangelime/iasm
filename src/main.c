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

int paused = 0;

void cut_newline(char * string) {
	char c = 0;
	while (c = *string) {
		if (c == '\n') {
			*string = '\0';
			return;
		}
		string++;
	}
}

int decode_type(char * type) {
	if (strcasecmp(type, "float32") == 0) {
		return FLOAT32;
	} else if (strcasecmp(type, "float64") == 0) {
		return FLOAT64;
	} else if (strcasecmp(type, "int128") == 0) {
		return INT128;
	} else if (strcasecmp(type, "int64") == 0) {
		return INT64;
	} else if (strcasecmp(type, "int32") == 0) {
		return INT32;
	}
}

int execute_builtins(char * line) {
	char regname[64] = {0};
	char filename[128] = {0};
	char type[32] = {0};
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
	if (sscanf(line, "xmm_type %s", filename) > 0) {
		xmm_type = decode_type(filename);
		return 1;
	}
	if (sscanf(line, "save_state %s", filename) > 0) {
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
	if (sscanf(line, "load_state %s", filename) > 0) {
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
	if (sscanf(line, "print %s%s", regname, type) > 0) {
		int swap = *type;
		char * name = swap ? type : regname;
		char * type = swap ? regname : type;
		uint64_t * p = lookup_register(name);
		if (!p) {
			fpu_float_t * f = lookup_fpuregister(name);
			if (!f) {
				void * xp = lookup_xmmregister(name);
				if (!xp) {
					return 0;
				}
				print_xmm(xp, *type ? decode_type(type) : xmm_type);
				putchar('\n');
				return 1;
			}
			printf("%lf\n", fpu_float_to_double(f));
			return 1;
		}
		uint64_t mask = lookup_register_mask(name);
		printf("%p\n", *p & mask);
		return 1;
	}
	return 0;
}

int handle_statement(char * line, size_t lsize) {
	int bmatches = 0;
	if (lsize == 0) {
		return 0;
	}
	cut_newline(line);
	bmatches = execute_builtins(line);
	if (bmatches == 2) {
		return 1;
	} else if (bmatches == 1) {
		return 0;
	}
	void * buffer = NULL;
	ssize_t size;
	int skip = line[0] == '~';
	int stat = assemble(line + skip, &buffer, &size);
	if (size == -1 || stat != 0) {
		printf("assembler error\n");
		asm_rewind();
		return 0;
	}
	execute(buffer, size, paused || skip);
	free(buffer);
	return 0;
}

ssize_t read_input(char ** line, size_t * size) {
	ssize_t r = getline(line, size, stdin);
	if (r == -1) {
		putchar('\n');
	}
	return r;
}

int main(int argc, char * argv[]) {
	char * line = NULL;
	size_t lsize = 0;
	register_handlers();
	setup_executable_buffer();
	asm_reset();
	write(1, "> ", 2);
	while (read_input(&line, &lsize) != -1) {
		if (handle_statement(line, lsize)) {
			break;
		}
		write(1, "> ", 2);
	}
	return 0;
}
