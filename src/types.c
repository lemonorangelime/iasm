#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <types.h>
#include <arch.h>
#include <asm.h>

int xmm_type = INT128;
int ymm_type = INT256;

int decode_type(char * type) {
	if (strcasecmp(type, "float80") == 0) { // float
		return FLOAT80;
	} else if (strcasecmp(type, "float64") == 0) {
		return FLOAT64;
	} else if (strcasecmp(type, "float32") == 0) {
		return FLOAT32;
	} else if (strcasecmp(type, "int256") == 0) { // int
		return INT256;
	} else if (strcasecmp(type, "int128") == 0) {
		return INT128;
	} else if (strcasecmp(type, "int64") == 0) {
		return INT64;
	} else if (strcasecmp(type, "int32") == 0) {
		return INT32;
	} else if (strcasecmp(type, "int16") == 0) {
		return INT16;
	} else if (strcasecmp(type, "int8") == 0) {
		return INT8;
	}

	if (strcasecmp(type, "yword") == 0) {
		return INT256;
	} else if ((strcasecmp(type, "dqword") == 0) || (strcasecmp(type, "oword") == 0)) {
		return INT128;
	} else if (strcasecmp(type, "tword") == 0) { // the t word
		return FLOAT80;
	} else if (strcasecmp(type, "qword") == 0) {
		return INT64;
	} else if ((strcasecmp(type, "dword") == 0) || (strcasecmp(type, "long") == 0)) {
		return INT32;
	} else if (strcasecmp(type, "word") == 0) {
		return INT16;
	} else if ((strcasecmp(type, "byte") == 0) || (strcasecmp(type, "hword") == 0)) {
		return INT8;
	}
	return INT8;
}

int decode_general_type(char * name) {
	int type = decode_type(name);
	switch (type) {
		case INT128:
		case INT256: return INT64;
	}
	return type;
}

int decode_type_size(int type) {
	switch (type) {
		case FLOAT80: return 10;
		case FLOAT64: return 8;
		case FLOAT32: return 4;

		case INT256: return 32;
		case INT128: return 16;
		case INT64: return 8;
		case INT32: return 4;
		case INT16: return 2;
		case INT8: return 1;
	}
}

int print_typed_value(void * p, int type, int remaining) {
	fpu_float_t * float80 = p;
	double * float64 = p;
	float * float32 = p;
	uint64_t * int64 = p;
	uint32_t * int32 = p;
	uint16_t * int16 = p;
	uint8_t * int8 = p;
	int size = decode_type_size(type);

	if (size > remaining) {
		printf("...");
		return remaining;
	}

	switch (type) {
#if defined(__X86_64)
		case FLOAT80:	printf("%lf", fpu_float_to_double(float80)); break;
#elif defined(__X86)
		case FLOAT80:	printf("%llf", *(long double *) float80); break;
#endif
		case FLOAT64:	printf("%lf", *float64); break;
		case FLOAT32:	printf("%f", *float32); break;
		case INT256:	printf("0x%.16llx%.16llx%.16llx%.16llx", int64[3], int64[2], int64[1], int64[0]); break;
		case INT128:	printf("0x%.16llx%.16llx", int64[1], int64[0]); break;
		case INT64:	printf("0x%.16lx", *int64); break;
		case INT32:	printf("0x%.8lx", *int32); break;
		case INT16:	printf("0x%.4x", *int16); break;
		case INT8:	printf("0x%.2x", *int8); break;
	}
	return size;
}

void print_typed_bytes(void * p, int type, int size) {
	while (size > 0) {
		int printed = print_typed_value(p, type, size);
		p += printed;
		size -= printed;
		putchar(size ? ' ' : 0);
	}
}
