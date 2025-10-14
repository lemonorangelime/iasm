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
int ymm_type = INT256;

uint16_t print_flags = PRINT_GENERAL | PRINT_XMM | PRINT_FPU; // no PRINT_YMM



void print_uppercase(char * string) {
	while (*string) {
		putchar(toupper(*string++));
	}
}

void print_xmm(void * p, int as) {
	double * float64 = p;
	float * float32 = p;
	uint64_t * int64 = p;
	uint32_t * int32 = p;
	uint16_t * int16 = p;
	uint8_t * int8 = p;

	switch (as) {
		case FLOAT64:
			printf("%lf %lf", float64[0], float64[1]);
			return;
		case FLOAT32:
			printf("%f %f %f %f", float32[0], float32[1], float32[2], float32[3]);
			return;
		case INT256:
		case INT128:
			printf("0x%.16lx%.16lx", int64[1], int64[0]);
			return;
		case INT64:
			printf("0x%.16lx 0x%.16lx", int64[1], int64[0]);
			return;
		case INT32:
			printf("0x%.8lx 0x%.8lx 0x%.8lx 0x%.8lx", int32[0], int32[1], int32[2], int32[3]);
			return;
		case INT16:
			printf("0x%.4lx 0x%.4lx 0x%.4lx 0x%.4lx ", int16[0], int16[1], int16[2], int16[3]);
			printf("0x%.4lx 0x%.4lx 0x%.4lx 0x%.4lx", int16[4], int16[5], int16[6], int16[7]);
			return;
		case INT8:
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[0], int8[1], int8[2], int8[3]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[4], int8[5], int8[6], int8[7]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[8], int8[9], int8[10], int8[11]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx", int8[12], int8[13], int8[14], int8[15]);
			return;
	}
}

void print_ymm(void * p, int as) {
	double * float64 = p;
	float * float32 = p;
	uint64_t * int64 = p;
	uint32_t * int32 = p;
	uint16_t * int16 = p;
	uint8_t * int8 = p;

	p += ((uint64_t) &fpu_save.ymm0_high - (uint64_t) &fpu_save.xmm0) - 16;
	double * float64h = p;
	float * float32h = p;
	uint64_t * int64h = p;
	uint32_t * int32h = p;
	uint16_t * int16h = p;
	uint8_t * int8h = p;

	switch (as) {
		case FLOAT64:
			printf("%lf %lf %lf %lf", float64[0], float64[1], float64h[2], float64h[3]);
			return;
		case FLOAT32:
			printf("%f %f %f %f ", float32[0], float32[1], float32[2], float32[3]);
			printf("%f %f %f %f", float32h[4], float32h[5], float32h[6], float32h[7]);
			return;
		case INT256:
			printf("0x%.16lx%.16lx%.16lx%.16lx", int64h[3], int64h[2], int64[1], int64[0]);
			return;
		case INT128:
			printf("0x%.16lx%.16lx ", int64[1], int64[0]);
			printf("0x%.16lx%.16lx", int64h[3], int64h[2]);
			return;
		case INT64:
			printf("0x%.16lx 0x%.16lx ", int64[0], int64[1]);
			printf("0x%.16lx 0x%.16lx", int64h[2], int64h[3]);
			return;
		case INT32:
			printf("0x%.8lx 0x%.8lx 0x%.8lx 0x%.8lx ", int32[0], int32[1], int32[2], int32[3]);
			printf("0x%.8lx 0x%.8lx 0x%.8lx 0x%.8lx", int32h[4], int32h[5], int32h[6], int32h[7]);
			return;
		case INT16:
			printf("0x%.4lx 0x%.4lx 0x%.4lx 0x%.4lx ", int16[0], int16[1], int16[2], int16[3]);
			printf("0x%.4lx 0x%.4lx 0x%.4lx 0x%.4lx ", int16[4], int16[5], int16[6], int16[7]);
			printf("0x%.4lx 0x%.4lx 0x%.4lx 0x%.4lx ", int16h[8], int16h[9], int16h[10], int16h[11]);
			printf("0x%.4lx 0x%.4lx 0x%.4lx 0x%.4lx", int16h[12], int16h[13], int16h[14], int16h[15]);
			return;
		case INT8:
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[0], int8[1], int8[2], int8[3]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[4], int8[5], int8[6], int8[7]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[8], int8[9], int8[10], int8[11]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8[12], int8[13], int8[14], int8[15]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8h[16], int8h[17], int8h[18], int8h[19]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8h[20], int8h[21], int8h[22], int8h[23]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx ", int8h[24], int8h[25], int8h[26], int8h[27]);
			printf("0x%.2lx 0x%.2lx 0x%.2lx 0x%.2lx", int8h[28], int8h[29], int8h[30], int8h[31]);
			return;
	}
}

int xmm_per_line(int as) {
	switch (as) {
		case FLOAT64: return 4;
		case FLOAT32: return 2;
		case INT256: return 2;
		case INT128: return 2;
		case INT64: return 2;
		case INT32: return 2;
		case INT16: return 2;
		case INT8: return 1;
	}
}

int ymm_per_line(int as) {
	switch (as) {
		case FLOAT64: return 2;
		case FLOAT32: return 1;
		case INT256: return 1;
		case INT128: return 1;
		case INT64: return 1;
		case INT32: return 1;
		case INT16: return 1;
		case INT8: return 1;
	}
}

// dont care about this either

void dump_general_registers() {
	if (!(print_flags & PRINT_GENERAL)) {
		return;
	}
	uint64_t * p = (uint64_t *) &register_save;
	int printed = 1;
	while (printed < reg_count) {
		for (int i = 0; i < 4; i++) {
			if (printed >= reg_count) {
				break;
			}
			char * name = regnames[printed];
			char c = (strlen(name) < 3) ? ' ' : 0;
			print_uppercase(name);
			printf("%c=0x%.16llx%c", c, p[printed - 1], i == 3 ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
	putchar('\n');
}

void dump_xmm_registers() {
	if (!(print_flags & PRINT_XMM)) {
		return;
	}
	int printed = 1;
	while (printed <= 16) {
		int perline = xmm_per_line(xmm_type);
		for (int i = 0; i < perline; i++) {
			if (printed > 16) {
				break;
			}
			char * name = xmmregnames[printed];
			char c = (strlen(name) < 5) ? ' ' : 0;
			xmm_float_t * f = lookup_xmmregister(name);
			print_uppercase(name);
			putchar(c);
			putchar('=');
			print_xmm(f, xmm_type);
			putchar(i == (perline - 1) ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
	putchar('\n');
}

void dump_ymm_registers() {
	if (!(print_flags & PRINT_YMM)) {
		return;
	}
	int printed = 1;
	while (printed <= 16) {
		int perline = ymm_per_line(ymm_type);
		for (int i = 0; i < perline; i++) {
			if (printed > 16) {
				break;
			}
			char * name = ymmregnames[printed];
			char c = (strlen(name) < 5) ? ' ' : 0;
			ymm_float_t * f = lookup_ymmregister(name);
			print_uppercase(name);
			putchar(c);
			putchar('=');
			print_ymm(f, ymm_type);
			putchar(i == (perline - 1) ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
	putchar('\n');
}

void dump_fpu_registers() {
	if (!(print_flags & PRINT_FPU)) {
		return;
	}
	int printed = 1;
	while (printed <= 8) {
		for (int i = 0; i < 4; i++) {
			if (printed > 8) {
				break;
			}
			char * name = fpuregnames[printed];
			fpu_float_t * f = lookup_fpuregister(name);
			print_uppercase(name);
			printf("=%lf%c", fpu_float_to_double(f), i == 3 ? 0 : ' ');
			printed++;
		}
		putchar('\n');
	}
}

void dump_registers() {
	dump_general_registers();
	dump_xmm_registers();
	dump_ymm_registers();
	dump_fpu_registers();
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

xmm_float_t * lookup_xmmregister(char * name) {
	xmm_float_t * registers = &fpu_save.xmm0;
	int index = lookup_index(xmmregnames, xmmreg_count, name);
	if (index == 0) {
		return NULL;
	}
	index -= 1;
	return &registers[index];
}

ymm_float_t * lookup_ymmregister(char * name) {
	xmm_float_t * registers = (xmm_float_t *) &fpu_save.xmm0;
	int index = lookup_index(ymmregnames, ymmreg_count, name);
	if (index == 0) {
		return NULL;
	}
	index -= 1;
	return (ymm_float_t *) &registers[index];
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

char * int64_specifier = "0x%.16llx";
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
