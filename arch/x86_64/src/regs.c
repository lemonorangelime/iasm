#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <asm.h>
#include <regs.h>
#include <types.h>

// dont care calling these whatever
char * regnames[] = {"none", "rax", "rbx", "rcx", "rdx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rflags"};
char * reg32names[] = {"none", "eax", "ebx", "ecx", "edx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", "eflags"};
char * reg16names[] = {"none", "ax", "bx", "cx", "dx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", "flags"};
char * reg8lnames[] = {"none", "al", "bl", "cl", "dl", "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b", "flagsl"};
char * reg8hnames[] = {"none", "ah", "bh", "ch", "dh", "sph", "bph", "sih", "dih", "r8bh", "r9bh", "r10bh", "r11bh", "r12bh", "r13bh", "r14bh", "r15bh", "flagsh"};
char * fpuregnames[] = {"none", "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7"};
char * fpuctrlnames[] = {"none", "fcw", "fsw", "ftw", "fop"};
char * xmmregnames[] = {"none", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"};
char * ymmregnames[] = {"none", "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15"};
char * zmmregnames[] = {"none", "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15"};

int reg_count = sizeof(regnames) / sizeof(regnames[0]);
int fpureg_count = sizeof(fpuregnames) / sizeof(fpuregnames[0]);
int fpuctrlreg_count = sizeof(fpuctrlnames) / sizeof(fpuctrlnames[0]);
int xmmreg_count = sizeof(xmmregnames) / sizeof(xmmregnames[0]);
int ymmreg_count = sizeof(ymmregnames) / sizeof(ymmregnames[0]);
int zmmreg_count = sizeof(zmmregnames) / sizeof(zmmregnames[0]);

uint16_t print_flags = PRINT_GENERAL | PRINT_XMM | PRINT_FPU; // no PRINT_YMM



void print_uppercase(char * string) {
	while (*string) {
		putchar(toupper(*string++));
	}
}

void print_xmm(void * p, int type) {
	print_typed_bytes(p, type, 16);
}

void print_ymm(void * p, int type) {
	uint8_t buffer[32];
	memcpy(buffer, p, 16);
	p += ((uintptr_t) &fpu_save.ymm0_high - (uintptr_t) &fpu_save.xmm0);
	memcpy(buffer + 16, p, 16);

	p = buffer;
	print_typed_bytes(p, type, 32);
}

void print_zmm(void * p, int type) {
	uint8_t buffer[64];
	int index = (((uintptr_t) p) - ((uintptr_t) &fpu_save.xmm0)) >> 4;
	memcpy(buffer, p, 16);
	p += ((uintptr_t) &fpu_save.ymm0_high - (uintptr_t) &fpu_save.xmm0);
	memcpy(buffer + 16, p, 16);
	p = (void *) ((uintptr_t) &fpu_save.zmm0_high) + (index << 5);
	memcpy(buffer + 32, p, 32);
	p = buffer;
	print_typed_bytes(p, type, 64);
}

int xmm_per_line(int type) {
	uint8_t perline[] = {4, 2, 2, 2, 2, 2, 2, 1};
	return perline[type];
}

int ymm_per_line(int type) {
	return 1 + (type == FLOAT64);
}

// dont care about this either

int dump_general_registers(int index) {
	if (!(print_flags & PRINT_GENERAL)) {
		return 0;
	}
	uint64_t * p = (uint64_t *) &register_save;
	int printed = 1;
	putchar((!!index) * '\n');
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
	return 1;
}

int dump_xmm_registers(int index) {
	if (!(print_flags & PRINT_XMM)) {
		return 0;
	}
	int printed = 1;
	putchar((!!index) * '\n');
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
	return 1;
}

int dump_ymm_registers(int index) {
	if (!(print_flags & PRINT_YMM)) {
		return 0;
	}
	int printed = 1;
	putchar((!!index) * '\n');
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
	return 1;
}

int dump_zmm_registers(int index) {
	if (!(print_flags & PRINT_ZMM)) {
		return 0;
	}
	int printed = 1;
	putchar((!!index) * '\n');
	while (printed <= 16) {
		char * name = zmmregnames[printed];
		char c = (strlen(name) < 5) ? ' ' : 0;
		zmm_float_t * f = lookup_zmmregister(name);
		print_uppercase(name);
		putchar(c);
		putchar('=');
		print_zmm(f, zmm_type);
		putchar('\n');
		printed++;
	}
	return 1;
}

int dump_fpu_registers(int index) {
	if (!(print_flags & PRINT_FPU)) {
		return 0;
	}
	int printed = 1;
	putchar((!!index) * '\n');
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

	printf("\nFCW=0x%.4x  ", fpu_save.fcw);
	printf("FSW=0x%.4x ", fpu_save.fsw);
	printf("FTW=0x%.4x  ", fpu_save.ftw);

	printf("FOP=0x%.4x\n", fpu_save.fop);
	printf("FIP=0x%.16lx ", fpu_save.fip);
	printf("FDP=0x%.16lx\n", fpu_save.fdp);
	return 1;
}

void dump_registers() {
	int c = 0;
	c += dump_general_registers(c);
	c += dump_xmm_registers(c);
	c += dump_ymm_registers(c);
	c += dump_zmm_registers(c);

	if (print_flags & (PRINT_XMM | PRINT_YMM | PRINT_ZMM)) {
		printf("\nMXCSR=0x%.8lx\n", fpu_save.mxcsr);
	}

	c += dump_fpu_registers(c);
}

int lookup_index(char ** names, int count, char * name) {
	for (int i = 0; i < count; i++) {
		if (strcasecmp(names[i], name) == 0) {
 			return i;
		}
	}
	return 0;
}

void * lookup_register(char * name) {
	uint64_t * registers = (uint64_t *) &register_save;
	int index = lookup_index(regnames, reg_count, name);
	index |= lookup_index(reg32names, reg_count, name);
	index |= lookup_index(reg16names, reg_count, name);
	index |= lookup_index(reg8lnames, reg_count, name);
	index |= lookup_index(reg8hnames, reg_count, name);
	if (strcasecmp(name, "mxcsr") == 0) {
		return (void *) &fpu_save.mxcsr;
	} else if (strcasecmp(name, "fcw") == 0) {
		return (void *) &fpu_save.fcw;
	} else if (strcasecmp(name, "fsw") == 0) {
		return (void *) &fpu_save.fsw;
	} else if (strcasecmp(name, "ftw") == 0) {
		return (void *) &fpu_save.ftw;
	} else if (strcasecmp(name, "fop") == 0) {
		return (void *) &fpu_save.fop;
	}
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

zmm_float_t * lookup_zmmregister(char * name) {
	xmm_float_t * registers = (xmm_float_t *) &fpu_save.xmm0;
	int index = lookup_index(zmmregnames, zmmreg_count, name);
	if (index == 0) {
		return NULL;
	}
	index -= 1;
	return (zmm_float_t *) &registers[index];
}

uint64_t lookup_register_mask(char * name) {
	if ((lookup_index(regnames, reg_count, name) != 0) || (strcasecmp(name, "fip") == 0) || (strcasecmp(name, "fdp") == 0)) {
		return 0xffffffffffffffff;
	}

	if ((lookup_index(reg32names, reg_count, name) != 0) || (strcasecmp(name, "mxcsr") == 0)) {
		return 0xffffffff;
	}

	if ((lookup_index(reg16names, reg_count, name) != 0) || (lookup_index(fpuctrlnames, fpuctrlreg_count, name) != 0)) {
		return 0xffff;
	}

	if ((lookup_index(reg8lnames, reg_count, name) != 0) || (lookup_index(reg8hnames, reg_count, name) != 0)) {
		return 0xff;
	}
	return 0;
}

int lookup_register_type(char * name) {
	if ((lookup_index(regnames, reg_count, name) != 0) || (strcasecmp(name, "fip") == 0) || (strcasecmp(name, "fdp") == 0)) {
		return INT64;
	}

	if ((lookup_index(reg32names, reg_count, name) != 0) || (strcasecmp(name, "mxcsr") == 0)) {
		return INT32;
	}

	if ((lookup_index(reg16names, reg_count, name) != 0) || (lookup_index(fpuctrlnames, fpuctrlreg_count, name) != 0)) {
		return INT16;
	}

	if ((lookup_index(reg8lnames, reg_count, name) != 0) || (lookup_index(reg8hnames, reg_count, name) != 0)) {
		return INT8;
	}
	return 0;
}

int lookup_register_size(char * name) {
	if ((lookup_index(regnames, reg_count, name) != 0) || (strcasecmp(name, "fip") == 0) || (strcasecmp(name, "fdp") == 0)) {
		return 8;
	}

	if ((lookup_index(reg32names, reg_count, name) != 0) || (strcasecmp(name, "mxcsr") == 0)) {
		return 4;
	}

	if ((lookup_index(reg16names, reg_count, name) != 0) || (lookup_index(fpuctrlnames, fpuctrlreg_count, name) != 0)) {
		return 2;
	}

	if ((lookup_index(reg8lnames, reg_count, name) != 0) || (lookup_index(reg8hnames, reg_count, name) != 0)) {
		return 1;
	}
	return 0;
}
