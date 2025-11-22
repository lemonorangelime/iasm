#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <iasm/types.h>
#include <iasm/arch.h>
#include <iasm/floats.h>
#include <iasm/asm.h>

int xmm_type = INT128;
int ymm_type = INT256;
int zmm_type = INT512;

int decode_type(char * type) {
	if (strcasecmp(type, "float80") == 0) { // float
		return FLOAT80;
	} else if (strcasecmp(type, "float64") == 0) {
		return FLOAT64;
	} else if (strcasecmp(type, "float32") == 0) {
		return FLOAT32;
	} else if (strcasecmp(type, "float16") == 0) {
		return FLOAT16;
	} else if (strcasecmp(type, "float8") == 0) {
		return FLOAT8;
	} else if (strcasecmp(type, "int512") == 0) { // int
		return INT512;
	} else if (strcasecmp(type, "int256") == 0) {
		return INT256;
	} else if (strcasecmp(type, "int128") == 0) {
		return INT128;
	} else if (strcasecmp(type, "int80") == 0) {
		return INT80;
	} else if (strcasecmp(type, "int64") == 0) {
		return INT64;
	} else if (strcasecmp(type, "int32") == 0) {
		return INT32;
	} else if (strcasecmp(type, "int16") == 0) {
		return INT16;
	} else if (strcasecmp(type, "int8") == 0) {
		return INT8;
	} else if (strcasecmp(type, "bool") == 0) {
		return BOOL;
	}

	if (strcasecmp(type, "zword") == 0) {
		return INT512;
	} else if (strcasecmp(type, "yword") == 0) {
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

int decode_type_size(int type) {
	switch (type) {
		case INT512: return 64;
		case INT256: return 32;
		case INT128: return 16;
		case FLOAT80:
		case INT80: return 10;
		case FLOAT64:
		case INT64: return 8;
		case FLOAT32:
		case INT32: return 4;
		case FLOAT16:
		case INT16: return 2;
		case FLOAT8:
		case BOOL:
		case INT8: return 1;
	}
}

void print_bool(void * p, int size) {
	uint8_t * b = p;
	while (size--) {
		if (*b++ != 0) {
			printf("true");
			return;
		}
	}
	printf("false");
	return;
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
		case FLOAT16:	printf("%lf", float16_decode(*int16)); break;
		case FLOAT8:	printf("%lf", float8_decode(*int8)); break;
		case INT512:	printf("0x%.16llx%.16llx%.16llx%.16llx%.16llx%.16llx%.16llx%.16llx", int64[7], int64[6], int64[5], int64[4], int64[3], int64[2], int64[1], int64[0]); break;
		case INT256:	printf("0x%.16llx%.16llx%.16llx%.16llx", int64[3], int64[2], int64[1], int64[0]); break;
		case INT128:	printf("0x%.16llx%.16llx", int64[1], int64[0]); break;
		case INT80:	printf("0x%.2x%.16lx", int16[4], *int64); break;
		case INT64:	printf("0x%.16lx", *int64); break;
		case INT32:	printf("0x%.8lx", *int32); break;
		case INT16:	printf("0x%.4x", *int16); break;
		case INT8:	printf("0x%.2x", *int8); break;
		case BOOL:	size = remaining; print_bool(p, size); break;
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
