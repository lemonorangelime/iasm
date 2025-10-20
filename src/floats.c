#include <floats.h>
#include <math.h>
#include <stdio.h>

double float8_decode(uint64_t f) {
	uint64_t d = 0;
	uint64_t exponent = (f >> 3) & 0b1111;
	uint64_t exponent_top = (f >> 6) & 0b1;
	d |=  (f       & 0b00000111) << 49;
	d |= ((f >> 3) & 0b00111) << 52;
	d |= ((f >> 6) & 0b01) << 62;
	d |= ((f >> 7) & 0b1) << 63;

	d |= (exponent ? 0b01111111000l : 0l) << 52;
	d &= ~((exponent_top ? 0b01111111000l : 0l) << 52);
	return *(double *) &d;
}

double float16_decode(uint64_t f) {
	uint64_t d = 0;
	uint64_t exponent = (f >> 10) & 0b11111;
	uint64_t exponent_top = (f >> 14) & 0b1;
	d |=  (f        & 0b0000001111111111) << 42;
	d |= ((f >> 10) & 0b001111) << 52;
	d |= ((f >> 14) & 0b01) << 62;
	d |= ((f >> 15) & 0b1) << 63;

	d |= (exponent ? 0b01111110000l : 0l) << 52;
	d &= ~((exponent_top ? 0b01111110000l : 0l) << 52);
	return *(double *) &d;
}

// these arent so abnormal
double float32_decode(uint64_t f) {
	return *(float *) &f;
}

double float64_decode(uint64_t f) {
	return *(double *) &f;
}
