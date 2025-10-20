#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
	char * out;
	size_t out_size;
} disassembler_stream_t;

void disassemble(void * in, size_t in_size, char * out, size_t out_size);
