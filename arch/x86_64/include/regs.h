#pragma once

#include <stdint.h>

typedef struct {
	uint16_t sign : 1;
	uint16_t exponent : 15;
	uint64_t integer : 1;
	uint64_t fraction : 63;
	uint8_t padding[6];
} __attribute__((packed)) fpu_float_t; // 80 bits

typedef struct {
	uint8_t a[16];
} __attribute__((packed)) xmm_float_t;

typedef struct {
	uint8_t a[32];
} __attribute__((packed)) ymm_float_t;

typedef struct {
	uint8_t a[64];
} __attribute__((packed)) zmm_float_t;

typedef struct {
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rflags;
	uint64_t rip;
} __attribute__((packed)) registers_t;

typedef struct {
	uint16_t fcw;
	uint16_t fsw;
	uint16_t ftw;
	uint16_t fop;
	uint64_t fip;
	uint64_t fdp;
	uint32_t mxcsr;
	uint32_t mxcsr_mask;
	fpu_float_t st0;
	fpu_float_t st1;
	fpu_float_t st2;
	fpu_float_t st3;
	fpu_float_t st4;
	fpu_float_t st5;
	fpu_float_t st6;
	fpu_float_t st7;
	xmm_float_t xmm0;
	xmm_float_t xmm1;
	xmm_float_t xmm2;
	xmm_float_t xmm3;
	xmm_float_t xmm4;
	xmm_float_t xmm5;
	xmm_float_t xmm6;
	xmm_float_t xmm7;
	xmm_float_t xmm8;
	xmm_float_t xmm9;
	xmm_float_t xmm10;
	xmm_float_t xmm11;
	xmm_float_t xmm12;
	xmm_float_t xmm13;
	xmm_float_t xmm14;
	xmm_float_t xmm15;
	xmm_float_t reserved[10];
	xmm_float_t ymm0_high;
	xmm_float_t ymm1_high;
	xmm_float_t ymm2_high;
	xmm_float_t ymm3_high;
	xmm_float_t ymm4_high;
	xmm_float_t ymm5_high;
	xmm_float_t ymm6_high;
	xmm_float_t ymm7_high;
	xmm_float_t ymm8_high;
	xmm_float_t ymm9_high;
	xmm_float_t ymm10_high;
	xmm_float_t ymm11_high;
	xmm_float_t ymm12_high;
	xmm_float_t ymm13_high;
	xmm_float_t ymm14_high;
	xmm_float_t ymm15_high;
	char padding[0x40];
	ymm_float_t zmm0_high;
	ymm_float_t zmm1_high;
	ymm_float_t zmm2_high;
	ymm_float_t zmm3_high;
	ymm_float_t zmm4_high;
	ymm_float_t zmm5_high;
	ymm_float_t zmm6_high;
	ymm_float_t zmm7_high;
	ymm_float_t zmm8_high;
	ymm_float_t zmm9_high;
	ymm_float_t zmm10_high;
	ymm_float_t zmm11_high;
	ymm_float_t zmm12_high;
	ymm_float_t zmm13_high;
	ymm_float_t zmm14_high;
	ymm_float_t zmm15_high;
} __attribute__((packed)) fpu_registers_t;

extern char * regnames[];
extern char * fpuregnames[];
extern registers_t register_save;
extern fpu_registers_t fpu_save;
extern int xmm_type;
extern int ymm_type;
extern int zmm_type;
extern uint16_t print_flags;

enum {
	PRINT_GENERAL	= 0b00000001,
	PRINT_XMM	= 0b00000010,
	PRINT_YMM	= 0b00000100,
	PRINT_FPU	= 0b00001000,
	PRINT_ZMM	= 0b00010000,
};

void print_xmm(void * p, int as);
void print_ymm(void * p, int as);
void print_zmm(void * p, int as);
void dump_registers();
void * lookup_register(char * name);
uint64_t lookup_register_mask(char * name);
int lookup_register_type(char * name);
int lookup_register_size(char * name);
fpu_float_t * lookup_fpuregister(char * name);
xmm_float_t * lookup_xmmregister(char * name);
ymm_float_t * lookup_ymmregister(char * name);
zmm_float_t * lookup_zmmregister(char * name);
