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
#include <dynamic.h>
#include <types.h>
#include <platform.h>
#include <vmmode.h>

help_topic_t help_topics[];
int topic_count;

int linking_allowed = 1;
int linking_performed = 0;

int decode_print_flag(char * type) {
	if (strcasecmp(type, "general") == 0) {
		return PRINT_GENERAL;
	} else if (strcasecmp(type, "xmm") == 0) {
		return PRINT_XMM;
	} else if (strcasecmp(type, "ymm") == 0) {
		return PRINT_YMM;
	} else if (strcasecmp(type, "zmm") == 0) {
		return PRINT_ZMM;
	} else if (strcasecmp(type, "fpu") == 0) {
		return PRINT_FPU;
	}
	return -1;
}

int print_register(char * regname, char * type) {
	uint64_t * p = lookup_register(regname);
	if (!p) {
		return 1;
	}
	int type_identifier = *type ? decode_type(type) : lookup_register_type(regname); // use type prefix OR lookup default type
	int type_size = decode_type_size(type_identifier); // size
	int register_size = lookup_register_size(regname);
	if (type_size > register_size) {
		printf("type too large\n");
		return 0;
	}

	print_typed_bytes(p, type_identifier, register_size);
	putchar('\n');
	return 0;
}

int print_fpu_register(char * regname, char * type) {
	fpu_float_t * f = lookup_fpuregister(regname);
	if (!f) {
		return 1;
	}

	int type_identifier = *type ? decode_type(type) : FLOAT80; // use type prefix OR default type
	int type_size = decode_type_size(type_identifier); // size
	if (type_size > 10) {
		printf("type too large\n");
		return 0;
	}

	print_typed_bytes(f, type_identifier, 10);
	putchar('\n');
	return 0;
}

int print_xmm_register(char * regname, char * type) {
	void * xp = lookup_xmmregister(regname);
	if (!xp) {
		return 1;
	}
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

int print_zmm_register(char * regname, char * type) {
	void * zp = lookup_zmmregister(regname);
	if (!zp) {
		return 1;
	}
	print_zmm(zp, *type ? decode_type(type) : zmm_type);
	putchar('\n');
	return 0;
}

int print_label(char * label, char * type) {
	uintptr_t address = 0;
	if (resolve_label(label, &address)) {
		return 1;
	}
	int type_identifier = *type ? decode_type(type) : INT64;
	int type_size = decode_type_size(type_identifier);
	if (type_size > 8) {
		printf("type too large\n");
		return 0;
	}
	print_typed_value(&address, type_identifier, 8); // intentional print_typed_value (singular), if you do `print DWORD label`, you likely expect it to be truncated
	putchar('\n');
	//printf("0x%.16llx\n", address);
	return 0;
}

int print_function(char * regname, char * type) {
	int errors = 0;
	errors += print_register(regname, type);
	errors += print_label(regname, type);
	switch (platform) {
		case PLATFORM_X86_64:
		case PLATFORM_X86:
			errors += print_fpu_register(regname, type);
			errors += print_xmm_register(regname, type);
			errors += print_ymm_register(regname, type);
			errors += print_zmm_register(regname, type);
			return !(errors == 6);
	}
	return !(errors == 2);
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

void fake_symbol_resolution(char * name, uintptr_t address) {
	char buffer[2048];
	int c = sprintf(buffer, "%s equ 0x%llx", name, address);
	if (c < 1) {
		return;
	}

	void * codebuffer = NULL;
	ssize_t size = 0;
        int stat = assemble(buffer, &codebuffer, &size);
        if (size == -1 || stat != 0) {
                asm_rewind();
                return;
        }
}

int long_extern_function(char * line, char * library, char * local_name, char * lib_name) {
	void * handle = dynamic_lookup_library(library);
	if (!handle) {
		if (dynamic_load_library(library)) {
			printf("symbol resolution failed\n");
			return 1;
		}
		handle = dynamic_lookup_library(library);
	}
	void * p = dynamic_resolve_handle(handle, lib_name);
	if (!p) {
		printf("symbol resolution failed\n");
		return 1;
	}
	if (line[6] == 'f') {
		p = asm_append_jmptable(p);
	}
	fake_symbol_resolution(local_name, (uintptr_t) p);
	return 1;
}

int short_extern_function(char * line, char * library, char * symbol) {
	void * handle = dynamic_lookup_library(library);
	if (!handle) {
		if (dynamic_load_library(library)) {
			printf("symbol resolution failed\n");
			return 1;
		}
		handle = dynamic_lookup_library(library);
	}
	void * p = dynamic_resolve_handle(handle, symbol);
	if (!p) {
		printf("symbol resolution failed\n");
		return 1;
	}
	if (line[6] == 'f') {
		p = asm_append_jmptable(p);
	}
	fake_symbol_resolution(symbol, (uintptr_t) p);
	return 1;
}

void clean_comment(char * buffer, char * next) {
	char * s = buffer;
	while (*s && *s != ';') { s++; }
	if ((s == buffer || *s != 0) && next) {
		*next = 0;
	}
	*s = 0;
}

int execute_builtins(char * line) {
	char buffer[0xff] = {0};
	char buffer2[0xff] = {0};
	char buffer3[0xff] = {0};
	if (strcmp(line, "exit") == 0) {
		return 2;
	}
	if (strcmp(line, "ver") == 0 || strcmp(line, "version") == 0) {
		printf("%d.%d.%d\n", major_version, minor_version, patch_version);
		return 1;
	}
	if (strlen(line) >= 4 && memcmp(line, "help", 4) == 0) {
		if (line[4] == ' ' && line[5] != 0) {
			print_topic(line + 5);
			return 1;
		}
		puts("version       |  print iasm version");
		puts("freeze        |  pause execution");
		puts("unfreeze      |  unpause execution");
		puts("assemble      |  assemble instruction (assemble addsd xmm0, xmm1)");
		puts("xmm_type      |  set default type for xmm registers (xmm_type INT256/128/64/32/16/8 / FLOAT64/32/16/8)");
		puts("ymm_type      |  set default type for ymm registers (ymm_type INT256/128/64/32/16/8 / FLOAT64/32/16/8)");
		puts("zmm_type      |  set default type for zmm registers (zmm_type INT256/128/64/32/16/8 / FLOAT64/32/16/8)");
		puts("dump_enable   |  enable function of `dump` command (general, xmm, ymm, zmm, fpu)");
		puts("dump_disable  |  disable function of `dump` command (general, xmm, ymm, zmm, fpu)");
		puts("print         |  print value of register (print xmm0 / print FLOAT64 xmm0)");
		puts("x             |  examine memory (x/10xq 0x1234)");
		puts("dump          |  print all registers");
		puts("exit          |  exit program");
		return 1;
	}
	if (strlen(line) >= 4 && memcmp(line, "dump", 4) == 0 && (line[4] == 0 || line[4] == ' ')) {
		if (line[4] == ' ' && line[5] != 0) {
			int flag = decode_print_flag(line + 5);
			if (flag == -1) {
				printf("bad flag\n");
				return 1;
			}
			int old_flags = print_flags;
			print_flags = flag;
			dump_registers();
			print_flags = old_flags;
			return 1;
		}
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
	if (strcmp(line, "vmmode") == 0) {
		printf("TODO: impliment\n");
		return 1;
		/* if (linking_performed) {
			printf("can not enter VM mode while dynamically linked\n");
			return 1;
		}
		linking_allowed = 0;
		vmmode_init();
		return 1; */
	}
	if ((sscanf(line, "externf %s%s%s", buffer, buffer2, buffer3) > 0) || (sscanf(line, "extern %s%s%s", buffer, buffer2, buffer3) > 0)) {
		if (!linking_allowed) {
			printf("can not dynamiclly link while in VM mode\n");
			return 1;
		}

		clean_comment(buffer, buffer2);
		clean_comment(buffer2, buffer3);
		clean_comment(buffer3, NULL);

		linking_performed = 1;
		if (*buffer3) {
			return long_extern_function(line, buffer2, buffer, buffer3);
		}
		if (*buffer2) {
			return short_extern_function(line, buffer2, buffer);
		}

		void * p = dynamic_resolve(buffer);
		if (!p) {
			printf("symbol resolution failed\n");
			return 1;
		}
		if (line[6] == 'f') {
			p = asm_append_jmptable(p);
		}
		fake_symbol_resolution(buffer, (uintptr_t) p);
		return 1;
	}
	if (sscanf(line, "dlopen %s", buffer) > 0) {
		if (!linking_allowed) {
			printf("can not dynamiclly link while in VM mode\n");
			return 1;
		}
		linking_performed = 1;
		if (dynamic_load_library(buffer)) {
			printf("dlopen failed\n");
		}
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
	if (sscanf(line, "zmm_type %s", buffer) > 0) {
		zmm_type = decode_type(buffer);
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
		print_flags &= ~flag;
		return 1;
	}
	if (strlen(line) > 9 && memcmp(line, "assemble ", 9) == 0) {
		return assemble_function(line + 9);
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
	{"xmm_type",	"sets default dump/print type for xmm registers\ntype can be FLOAT64/32/16/8 or INT256/128/64/32/16/8"},
	{"ymm_type",	"sets default dump/print type for ymm registers\ntype can be FLOAT64/32/16/8 or INT256/128/64/32/16/8"},
	{"zmm_type",	"sets default dump/print type for zmm registers\ntype can be FLOAT64/32/16/8 or INT256/128/64/32/16/8"},
	{"print",	"prints register or label value (print rax, print label)\ncan also take a type for xmm registers (print FLOAT64 xmm0)"},
	{"assemble",	"assembles an instruction and prints the result as a series of bytes\n\n        > assemble fldpi\n        0xd9 0xeb\n"},
	{"x",		"examines memory (x/[count][type][size])\n\ncount: number of units to print\ntype: unit type (x = hexadecimal, d = signed digit, u = unsigned digit, o = octal, c = character, b = binary, f = float)\nsize: unit size (b = byte, w = word, d = dword, q = qword)\n\n        > ~example: dd 0x12345678\n        > x/1xd example\n        0x12345678\n        > x/4x example\n        0x78 0x56 0x34 0x12\n"}
};

int topic_count = sizeof(help_topics) / sizeof(help_topics[0]);
