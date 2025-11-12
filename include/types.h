#pragma once

enum {
	FLOAT80,
	FLOAT64,
	FLOAT32,
	FLOAT16,
	FLOAT8,
	INT512,
	INT256,
	INT128,
	INT80,
	INT64,
	INT32,
	INT16,
	INT8,
	BOOL
};

int decode_type(char * type);
int decode_type_size(int type);
int print_typed_value(void * p, int type, int remaining);
void print_typed_bytes(void * p, int type, int size);
