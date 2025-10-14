#pragma once

// abnormal float types

#include <stdint.h>

typedef struct {
	uint8_t mantissa : 3;
	uint8_t exponent : 4;
	uint8_t sign : 1;
} float8_t;

typedef struct {
	uint16_t mantissa : 10;
	uint16_t exponent : 5;
	uint16_t sign : 1;
} float16_t;

typedef float float32_t;
typedef double float64_t;

double float8_decode(float8_t * f);
double float16_decode(float16_t * f);
double float32_decode(float * f);
double float64_decode(double * f);
