#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <asm.h>
#include <regs.h>

// dont care calling these whatever
char * regnames[] = {"none", "rax", "rbx", "rcx", "rdx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rflags"};
char * reg32names[] = {"none", "eax", "ebx", "ecx", "edx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", "eflags"};
char * reg16names[] = {"none", "ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", "flags"};
char * fpuregnames[] = {"none", "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7"};
char * xmmregnames[] = {"none", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"};
char * ymmregnames[] = {"none", "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"};

int reg_count = sizeof(regnames) / sizeof(regnames[0]);
int fpureg_count = sizeof(fpuregnames) / sizeof(fpuregnames[0]);
int xmmreg_count = sizeof(xmmregnames) / sizeof(xmmregnames[0]);
int ymmreg_count = sizeof(ymmregnames) / sizeof(ymmregnames[0]);

int xmm_type = INT128;

void print_uppercase(char * string) {
	while (*string) {
		putchar(toupper(*string++));
	}
}

void print_xmm(void * p, int as) {
	xmm_float64_t * float64 = p;
	xmm_float32_t * float32 = p;
	xmm_int128_t * _int128 = p;
	xmm_int32_t * _int32 = p;

	switch (as) {
		case FLOAT64:
			printf("%lf %lf", float64->a, float64->b);
			return;
		case FLOAT32:
			printf("%f %f %f %f", float32->a, float32->b, float32->c, float32->d);
			return;
		case INT128:
			printf("0x%.16lx%.16lx", _int128->b, _int128->a);
			return;
		case INT64:
			printf("0x%.16lx 0x%.16lx", _int128->a, _int128->b);
			return;
		case INT32:
			printf("0x%.8lx 0x%.8lx 0x%.8lx 0x%.8lx", _int32->a, _int32->b, _int32->c, _int32->d);
			return;
	}
}

void dump_registers() { // dont care about this either
	uint64_t * p = (uint64_t *) &register_save;
	uint64_t * rp = (uint64_t *) &rregister_save;
	int printed = 1;
	while (printed < reg_count) {
		for (int i = 0; i < 4; i++) {
			char * name = regnames[printed];
			if (printed >= reg_count) {
				break;
			}
			char c = (strlen(name) < 3) ? ' ' : 0;
			print_uppercase(name);
			printf("%c=0x%.16llx%c", c, p[printed - 1], i == 3 ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
	putchar('\n');
	printed = 1;
	while (printed <= 16) {
		int perline = (xmm_type == FLOAT64) ? 4 : 2;
		for (int i = 0; i < perline; i++) {
			char * name = xmmregnames[printed];
			if (printed > 16) {
				break;
			}
			char c = (strlen(name) < 5) ? ' ' : 0;
			xmm_float64_t * f = lookup_xmmregister(name);
			print_uppercase(name);
			putchar(c);
			putchar('=');
			print_xmm(f, xmm_type);
			putchar(i == 3 ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
	putchar('\n');
	printed = 1;
	while (printed <= 8) {
		for (int i = 0; i < 4; i++) {
			char * name = fpuregnames[printed];
			if (printed > 8) {
				break;
			}
			fpu_float_t * f = lookup_fpuregister(name);
			print_uppercase(name);
			printf("=%lf%c", fpu_float_to_double(f), i == 3 ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
}

int lookup_index(char ** regnames, int count, char * name) {
	for (int i = 0; i < count; i++) {
		if (strcmp(regnames[i], name) == 0) {
 			return i;
		}
	}
	return 0;
}

uint64_t * lookup_register(char * name) {
	uint64_t * registers = (uint64_t *) &register_save;
	int index = lookup_index(regnames, reg_count, name);
	index |= lookup_index(reg32names, reg_count, name);
	index |= lookup_index(reg16names, reg_count, name);
	if (index == 0) {
		return NULL;
	}
	index -= 1;
	return &registers[index];
}

fpu_float_t * lookup_fpuregister(char * name) {
	fpu_float_t * registers = &fpu_save.st0;
	int index = lookup_index(fpuregnames, fpureg_count, name);
	if (index == 0) {
		return NULL;
	}
	index -= 1;
	return &registers[index];
}

xmm_float64_t * lookup_xmmregister(char * name) {
	xmm_float64_t * registers = &fpu_save.xmm0;
	int index = lookup_index(xmmregnames, xmmreg_count, name);
	if (index == 0) {
		return NULL;
	}
	index -= 1;
	return &registers[index];
}

uint64_t lookup_register_mask(char * name) {
	if (lookup_index(regnames, reg_count, name) != 0) {
		return 0xffffffffffffffff;
	}

	if (lookup_index(reg32names, reg_count, name) != 0) {
		return 0xffffffff;
	}

	if (lookup_index(reg16names, reg_count, name) != 0) {
		return 0xffff;
	}
	return 0;
}

char * int64_specifier = "0x%.16x";
char * int32_specifier = "0x%.8x";
char * int16_specifier = "0x%.4x";
char * lookup_register_specifier(char * name) {
	if (lookup_index(regnames, reg_count, name) != 0) {
		return int64_specifier;
	}

	if (lookup_index(reg32names, reg_count, name) != 0) {
		return int32_specifier;
	}

	if (lookup_index(reg16names, reg_count, name) != 0) {
		return int16_specifier;
	}
	return 0;
}
