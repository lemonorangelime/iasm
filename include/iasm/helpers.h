#pragma once

#include <sys/types.h>
#include <stdint.h>

void tmpname(char * path);
ssize_t my_fdsize(int fd);
ssize_t asm_src_fdsize();
void asm_src_readall(void * p);
void asm_src_writeall(void * p, size_t size);
int resolve_label(char * label, uintptr_t * p);
int resolve_register_or_label(char * name, uintptr_t * p);
