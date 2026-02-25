#pragma once

enum {
	FLOAT80,
	FLOAT64,
	FLOAT32,
	FLOAT16,
	FLOAT8,
	INT8192,
	INT512,
	INT256,
	INT128,
	INT80,
	INT64,
	INT32,
	INT16,
	INT8,
	BIN80,
	BIN64,
	BIN32,
	BIN16,
	BIN8,
	BOOL,
	BOOL_I8,
	BOOL_I16,
	BOOL_I32,
	BOOL_I64,
	STRING,
	CHARACTER,
	IPV4,
	IPV6,
	MAC,
	MD5,
	RGB24,
	RGB32,
	RGBA32,
	RGBA32_LEMONOS,
	MCDONALDS_US,
	MCDONALDS_CA,
	MCDONALDS_CH,
	MCDONALDS_UK,
	MCDONALDS_GB,
	MCDONALDS_SA,
	MCDONALDS_DE,
	MAT3X3_F32,
	MAT4X4_F32,
	MAT6X6_F32,
	MAT9X9_F32,
	MAT3X3_F16,
	MAT4X4_F16,
	MAT6X6_F16,
	MAT9X9_F16,
	MAT3X3_I8,
	MAT4X4_I8,
	MAT6X6_I8,
	MAT9X9_I8,
	MAT16X16_F32,
	MAT16X32_F16,
	MAT16X32_B16,
	MAT16X64_I8,
	IASM_TYPE_NULL,
	IASM_TYPE_OPERATOR,
	IASM_TYPE_TYPE,
	IASM_TYPE_FUNCTION,
};

enum {
	RGB_CSS,
	RGB_BLOCKS,
};

extern int rgb_style;

int decode_rgb_style(char * style);
int decode_type(char * type);
int decode_type_size(int type);
int print_typed_value(void * p, int type, int remaining);
void print_typed_bytes(void * p, int type, int size);
