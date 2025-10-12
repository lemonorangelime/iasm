#include <sys/sendfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <helpers.h>
#include <signals.h>
#include <builtins.h>
#include <stdio.h>
#include <regs.h>
#include <asm.h>
#include <main.h>

help_topic_t help_topics[];
int topic_count;

int decode_type(char * type) {
	if (strcasecmp(type, "float64") == 0) { // float
		return FLOAT64;
	} else if (strcasecmp(type, "float32") == 0) {
		return FLOAT32;
	} else if (strcasecmp(type, "int128") == 0) { // int
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
	char statement[255];
	void * buffer = NULL;
	ssize_t size = 0;
	int stat = sprintf(statement, "dq %s", label);
	if (stat <= 0) {
		puts("error");
		return 1;
	}
	stat = assemble(statement, &buffer, &size);
	asm_rewind();
	if ((stat != 0) || (size != 8)) {
		puts("assembler error");
		return 1;
	}
	uint64_t * p = buffer;
	printf("%.16llx\n", *p);
	return 1;
}

void print_topic(char * topic_name) {
	help_topic_t * topic = help_topics;
	int i = 0;
	for (; i < topic_count; i++, topic++) {
		if (strcmp(topic->name, topic_name)) {
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
		puts("freeze      |  pause execution");
		puts("unfreeze    |  unpause execution");
		puts("assemble    |  assemble instruction (assemble addsd xmm0, xmm1)");
		puts("resolve     |  resolve address (resolve LABEL)");
		puts("xmm_type    |  set default type for xmm registers (xmm_type INT128/64/32 / FLOAT64/32)");
		puts("save_state  |  save state to file (save_state file.bin)");
		puts("load_state  |  load state from file (load_state file.bin)");
		puts("print       |  print value of register (print xmm0 / print FLOAT64 xmm0)");
		puts("dump        |  print all registers");
		puts("exit        |  exit program");
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
	return 0;
}


// topics


help_topic_t help_topics[] = {
	{"test", " test message"}
};

int topic_count = sizeof(help_topics) / sizeof(help_topics[0]);
