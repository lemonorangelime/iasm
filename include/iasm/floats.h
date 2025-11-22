#pragma once

#include <stdint.h>

typedef struct {
	uint64_t mantissa : 52;
	uint64_t exponent : 11;
	uint64_t sign : 1;
} float64_t;

double float8_decode(uint64_t f);
double float16_decode(uint64_t f);
double float32_decode(uint64_t f);
double float64_decode(uint64_t f);
