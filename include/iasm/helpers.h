#pragma once

#include <sys/types.h>
#include <stdint.h>

void tmpname(char * path);
ssize_t my_fdsize(int fd);
ssize_t my_fdlinesize(int fd);
ssize_t asm_src_fdsize();
void asm_src_readall(void * p);
void asm_src_writeall(void * p, size_t size);
void print_ansii_colour(uint32_t colour);
void print_ansii_reset();
int resolve_label(char * label, uintptr_t * p);
int resolve_register_or_label(char * name, uintptr_t * p);
