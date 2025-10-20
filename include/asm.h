#pragma once

#include <stdint.h>
#include <regs.h>

extern uint64_t return_point;
extern uint8_t * exec_buffer;
extern uint8_t * stack_buffer;
extern size_t exec_buffer_size;
extern int context_switching;

void setup_executable_buffer();
uint64_t asm_resume();
uint64_t asm_execute(void * buffer, ssize_t size, int skip);
int assemble(char * instruction, void ** buffer, ssize_t * size);
void asm_reset();
void reload_state();
void setup_fpu();
double fpu_float_to_double(fpu_float_t * fpu_float);
void asm_rewind();
void asm_continue();
void safe_memcpy(void * dest, void * src, size_t size);
