// todo: break this up a bit more

#include <sys/sendfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <helpers.h>
#include <ctype.h>
#include <builtins.h>
#include <stdio.h>
#include <asm.h>
#include <main.h>
#include <version.h>
#include <examiner.h>

help_topic_t help_topics[];
int topic_count;

int decode_type(char * type) {
	if (strcasecmp(type, "float64") == 0) { // float
		return FLOAT64;
	} else if (strcasecmp(type, "float32") == 0) {
		return FLOAT32;
	} else if (strcasecmp(type, "int256") == 0) { // int
		return INT256;
	} else if (strcasecmp(type, "int128") == 0) {
		return INT128;
	} else if (strcasecmp(type, "int64") == 0) {
		return INT64;
	} else if (strcasecmp(type, "int32") == 0) {
		return INT32;
	} else if (strcasecmp(type, "int16") == 0) {
		return INT16;
	} else if (strcasecmp(type, "int8") == 0) {
		return INT8;
	}
	return INT8;
}

int decode_print_flag(char * type) {
	if (strcasecmp(type, "general") == 0) {
		return PRINT_GENERAL;
	} else if (strcasecmp(type, "xmm") == 0) {
		return PRINT_XMM;
	} else if (strcasecmp(type, "ymm") == 0) {
		return PRINT_YMM;
	} else if (strcasecmp(type, "fpu") == 0) {
		return PRINT_FPU;
	}
	return -1;
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
	write(fd, stack_buffer, 4096);
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
	read(fd, stack_buffer, 4096);
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

int print_ymm_register(char * regname, char * type) {
	void * yp = lookup_ymmregister(regname);
	if (!yp) {
		return 1;
	}
	print_ymm(yp, *type ? decode_type(type) : ymm_type);
	putchar('\n');
	return 0;
}

int print_function(char * regname, char * type) {
	int errors = 0;
	errors += print_register(regname, type);
	errors += print_fpu_register(regname, type);
	errors += print_xmm_register(regname, type);
	errors += print_ymm_register(regname, type);
	return !(errors == 4);
}

int assemble_function(char * instruction) {
	void * buffer = NULL;
	ssize_t size = 0;
	int stat = assemble(instruction, &buffer, &size);
	asm_rewind();
	if ((stat != 0) || (size <= 0)) {
		puts("assembler error");
		return 1;
	}
	uint8_t * u8buffer = buffer;
	while (size--) {
		printf("0x%.2x%c", *u8buffer++, (size == 0) ? '\0' : ' ');
	}
	putchar('\n');
	return 1;
}

int resolve_function(char * label) {
	uint64_t address = 0;
	if (resolve_label(label, &address)) {
		return 1;
	}
	printf("0x%.16llx\n", address);
	return 1;
}

void print_topic(char * topic_name) {
	help_topic_t * topic = help_topics;
	int i = 0;
	for (; i < topic_count; i++, topic++) {
		if (strcmp(topic->name, topic_name) == 0) {
			break;
		}
	}
	if (i == topic_count) {
		return;
	}
	puts(topic->message);
}

int execute_builtins(char * line) {
	char buffer[255] = {0};
	char buffer2[255] = {0};
	if (strcmp(line, "exit") == 0) {
		return 2;
	}
	if (strcmp(line, "ver") == 0 || strcmp(line, "version") == 0) {
		printf("%d.%d\n", major_version, minor_version);
		return 1;
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
	if (strlen(line) >= 4 && memcmp(line, "help", 4) == 0) {
		if (line[4] == ' ' && line[5] != 0) {
			print_topic(line + 5);
			return 1;
		}
		puts("freeze        |  pause execution");
		puts("unfreeze      |  unpause execution");
		puts("assemble      |  assemble instruction (assemble addsd xmm0, xmm1)");
		puts("resolve       |  resolve address (resolve LABEL)");
		puts("xmm_type      |  set default type for xmm registers (xmm_type INT256/128/64/32/16/8 / FLOAT64/32)");
		puts("ymm_type      |  set default type for ymm registers (ymm_type INT256/128/64/32/16/8 / FLOAT64/32)");
		puts("dump_enable   |  enable function of `dump` command (general, xmm, ymm, fpu)");
		puts("dump_disable  |  disable function of `dump` command (general, xmm, ymm, fpu)");
		puts("save_state    |  save state to file (save_state file.bin)");
		puts("load_state    |  load state from file (load_state file.bin)");
		puts("print         |  print value of register (print xmm0 / print FLOAT64 xmm0)");
		puts("x             |  examine memory (x/10xq 0x1234)");
		puts("dump          |  print all registers");
		puts("exit          |  exit program");
		return 1;
	}
	if (sscanf(line, "xmm_type %s", buffer) > 0) {
		xmm_type = decode_type(buffer);
		return 1;
	}
	if (sscanf(line, "ymm_type %s", buffer) > 0) {
		ymm_type = decode_type(buffer);
		return 1;
	}
	if (sscanf(line, "dump_enable %s", buffer) > 0) {
		int flag = decode_print_flag(buffer);
		if (flag == -1) {
			return 0;
		}
		print_flags |= flag;
		return 1;
	}
	if (sscanf(line, "dump_disable %s", buffer) > 0) {
		int flag = decode_print_flag(buffer);
		if (flag == -1) {
			return 0;
		}
		print_flags |= flag;
		return 1;
	}
	if (sscanf(line, "save_state %s", buffer) > 0) {
		return save_state(buffer);
	}
	if (sscanf(line, "load_state %s", buffer) > 0) {
		return load_state(buffer);
	}
	if (strlen(line) > 9 && memcmp(line, "assemble ", 9) == 0) {
		return assemble_function(line + 9);
	}
	if (strlen(line) > 8 && memcmp(line, "resolve ", 8) == 0) {
		return resolve_function(line + 8);
	}
	if (sscanf(line, "print %s%s", buffer, buffer2) > 0) {
		int swap = *buffer2;
		char * name = swap ? buffer2 : buffer;
		char * type = swap ? buffer : buffer2;
		return print_function(name, type);
	}
	return examine(line);
}


// topics


help_topic_t help_topics[] = {
	{"xmm_type",	"sets default dump/print type for xmm registers\ntype can be FLOAT64 / 32 or INT256 / 128 / 64 / 32 / 16 / 8"},
	{"ymm_type",	"sets default dump/print type for ymm registers\ntype can be FLOAT64 / 32 or INT256 / 128 / 64 / 32 / 16 / 8"},
	{"print",	"prints register value (print rax)\ncan also take a type for xmm registers (print FLOAT64 xmm0)"},
	{"resolve",	"resolves the address of a label\n\n        > label:\n        > resolve label\n        0x0000000001000000\n"},
	{"assemble",	"assembles an instruction and prints the result as a series of bytes\n\n        > assemble fldpi\n        0xd9 0xeb\n"},
	{"x",		"examines memory (x/[count][type][size])\n\ncount: number of units to print\ntype: unit type (x = hexadecimal, d = signed digit, u = unsigned digit, o = octal, c = character, b = binary, f = float)\nsize: unit size (b = byte, w = word, d = dword, q = qword)\n\n        > ~example: dd 0x12345678\n        > x/1xd example\n        0x12345678\n        > x/4x example\n        0x78 0x56 0x34 0x12\n"}
};

int topic_count = sizeof(help_topics) / sizeof(help_topics[0]);
