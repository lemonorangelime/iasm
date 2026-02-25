#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <iasm/types.h>
#include <iasm/arch.h>
#include <iasm/floats.h>
#include <iasm/helpers.h>
#include <iasm/asm.h>
#include <iasm/mcdonalds.h>
#include <arpa/inet.h>

int xmm_type = INT128;
int ymm_type = INT256;
int zmm_type = INT512;
int tmm_type = MAT16X32_F16;

int rgb_style = RGB_CSS;

uint32_t lemonos_legacy_colour[16] = {
	0x000000,
	0x0000aa,
	0x00aa00,
	0x00aaaa,
	0xaa0000,
	0xaa00aa,
	0xaa5500,
	0xaaaaaa,
	0x555555,
	0x5555ff,
	0x55ff55,
	0x55ffff,
	0xff5555,
	0xff55ff,
	0xffff55,
	0xffffff
};

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
	} else if (strcasecmp(type, "int8192") == 0) { // int
		return INT8192;
	} else if (strcasecmp(type, "int512") == 0) {
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
	} else if (strcasecmp(type, "bool_i8") == 0) {
		return BOOL_I8;
	} else if (strcasecmp(type, "bool_i16") == 0) {
		return BOOL_I16;
	} else if (strcasecmp(type, "bool_i32") == 0) {
		return BOOL_I32;
	} else if (strcasecmp(type, "bool_i64") == 0) {
		return BOOL_I64;
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

	if ((strcasecmp(type, "mcdonalds") == 0)           || (strcasecmp(type, "mcdonalds_us") == 0)           || (strcasecmp(type, "mcdonalds_usa") == 0)) {
		return MCDONALDS_US;
	} else if ((strcasecmp(type, "mcdonalds_ca") == 0) || (strcasecmp(type, "mcdonalds_canada") == 0)) {
		return MCDONALDS_CA;
	} else if ((strcasecmp(type, "mcdonalds_ch") == 0) || (strcasecmp(type, "mcdonalds_schweiz") == 0)      || (strcasecmp(type, "mcdonalds_switzerland") == 0) || (strcasecmp(type, "mcdonalds_suisse") == 0) || (strcasecmp(type, "mcdonalds_svizzera") == 0) || (strcasecmp(type, "mcdonalds_svizra") == 0)) {
		return MCDONALDS_CH;
	} else if ((strcasecmp(type, "mcdonalds_uk") == 0) || (strcasecmp(type, "mcdonalds_ukraine") == 0)      || (strcasecmp(type, "mcdonalds_ukrain") == 0)   || (strcasecmp(type, "mcdonalds_ukraina") == 0)) {
		return MCDONALDS_UK;
	} else if ((strcasecmp(type, "mcdonalds_gb") == 0) || (strcasecmp(type, "mcdonalds_england") == 0)      || (strcasecmp(type, "mcdonalds_britian") == 0)  || (strcasecmp(type, "mcdonalds_great_britian") == 0)) {
		return MCDONALDS_GB;											       /* culturally insensitive? */
	} else if ((strcasecmp(type, "mcdonalds_sa") == 0) || (strcasecmp(type, "mcdonalds_saudi_arabia") == 0) || (strcasecmp(type, "mcdonalds_saudi") == 0)) {
		return MCDONALDS_SA;
	} else if ((strcasecmp(type, "mcdonalds_de") == 0) || (strcasecmp(type, "mcdonalds_germany") == 0)      || (strcasecmp(type, "mcdonalds_deutschland") == 0)) {
		return MCDONALDS_DE;
	}

	if ((strcasecmp(type, "bin80") == 0) || (strcasecmp(type, "binary80") == 0)) {
		return BIN80;
	} else if ((strcasecmp(type, "bin64") == 0) || (strcasecmp(type, "binary64") == 0)) {
		return BIN64;
	} else if ((strcasecmp(type, "bin32") == 0) || (strcasecmp(type, "binary32") == 0)) {
		return BIN32;
	} else if ((strcasecmp(type, "bin16") == 0) || (strcasecmp(type, "binary16") == 0)) {
		return BIN16;
	} else if ((strcasecmp(type, "bin8") == 0) || (strcasecmp(type, "binary8") == 0)) {
		return BIN8;
	}

	if ((strcasecmp(type, "ipv4") == 0) || (strcasecmp(type, "ipv5") == 0)) { // networking
		return IPV4;
	} else if (strcasecmp(type, "ipv6") == 0) {
		return IPV6;
	} else if (strcasecmp(type, "mac") == 0) {
		return MAC;
	}

	if (strcasecmp(type, "md5") == 0) {
		return MD5;
	}

	if (strcasecmp(type, "rgb") == 0) {
		return RGB24;
	} else if (strcasecmp(type, "rgb32") == 0) {
		return RGB32;
	} else if ((strcasecmp(type, "rgba") == 0) || (strcasecmp(type, "rgba32") == 0)) {
		return RGBA32;
	} else if ((strcasecmp(type, "rgba_lemonos") == 0) || (strcasecmp(type, "rgba32_lemonos") == 0) || (strcasecmp(type, "rgba32_vga") == 0) || (strcasecmp(type, "rgba32_legacy") == 0) || (strcasecmp(type, "rgba32_vga_legacy") == 0) || (strcasecmp(type, "rgba_legacy") == 0) || (strcasecmp(type, "rgba_vga") == 0) || (strcasecmp(type, "rgba_vga_legacy") == 0)) {
		return RGBA32_LEMONOS;
	}

	if ((strcasecmp(type, "mat3x3") == 0) || strcasecmp(type, "mat3x3_f32") == 0) { // matrices
		return MAT3X3_F32;
	} else if ((strcasecmp(type, "mat4x4") == 0) || strcasecmp(type, "mat4x4_f32") == 0) {
		return MAT4X4_F32;
	} else if ((strcasecmp(type, "mat6x6") == 0) || (strcasecmp(type, "mat6x6_f32") == 0)) {
		return MAT6X6_F32;
	} else if ((strcasecmp(type, "mat9x9") == 0) || (strcasecmp(type, "mat9x9_f32") == 0)) {
		return MAT9X9_F32;
	} else if (strcasecmp(type, "mat3x3_f16") == 0) {
		return MAT3X3_F16;
	} else if (strcasecmp(type, "mat4x4_f16") == 0) {
		return MAT4X4_F16;
	} else if (strcasecmp(type, "mat6x6_f16") == 0) {
		return MAT6X6_F16;
	} else if (strcasecmp(type, "mat9x9_f16") == 0) {
		return MAT9X9_F16;
	} else if (strcasecmp(type, "mat4x4_i8") == 0) {
		return MAT4X4_I8;
	} else if (strcasecmp(type, "mat6x6_i8") == 0) {
		return MAT6X6_I8;
	} else if (strcasecmp(type, "mat9x9_i8") == 0) {
		return MAT9X9_I8;
	} else if ((strcasecmp(type, "mat16x16_f32") == 0) || (strcasecmp(type, "tile16x16_f32") == 0)) {
		return MAT16X16_F32;
	} else if ((strcasecmp(type, "mat16x32_f16") == 0) || (strcasecmp(type, "tile16x32") == 0)) {
		return MAT16X32_F16;
	} else if ((strcasecmp(type, "mat16x32_b16") == 0) || (strcasecmp(type, "tile16x32_b16") == 0)) {
		return MAT16X32_B16;
	} else if ((strcasecmp(type, "mat16x64_i8") == 0) || (strcasecmp(type, "tile16x64") == 0)) {
		return MAT16X64_I8;
	}

	if ((strcasecmp(type, "string") == 0) || strcasecmp(type, "ascii") == 0) { // ascii
		return STRING;
	}
	if ((strcasecmp(type, "char") == 0) || (strcasecmp(type, "character") == 0)) {
		return CHARACTER;
	}

	if (strcasecmp(type, "iasm_internal_type_null") == 0) {
		return IASM_TYPE_NULL;
	} else if (strcasecmp(type, "iasm_internal_type_type") == 0) {
		return IASM_TYPE_TYPE;
	} else if (strcasecmp(type, "iasm_internal_type_operator") == 0) {
		return IASM_TYPE_OPERATOR;
	} else if ((strcasecmp(type, "iasm_internal_type_function") == 0) || strcasecmp(type, "function") == 0) {
		return IASM_TYPE_FUNCTION;
	}

	return IASM_TYPE_NULL;
}

int decode_rgb_style(char * style) {
	if ((strcasecmp(style, "css") == 0) || (strcasecmp(style, "values") == 0)) {
		return RGB_CSS;
	} else if ((strcasecmp(style, "block") == 0) || (strcasecmp(style, "blocks") == 0)) {
		return RGB_BLOCKS;
	}
	return RGB_CSS;
}

int decode_type_size(int type) {
	switch (type) {
		case IASM_TYPE_FUNCTION: return sizeof(uintptr_t);
		case INT8192: return 1024;
		case INT512: return 64;
		case INT256: return 32;
		case INT128: return 16;
		case FLOAT80:
		case BIN80:
		case INT80: return 10;
		case FLOAT64:
		case BIN64:
		case BOOL_I64:
		case INT64: return 8;
		case FLOAT32:
		case BIN32:
		case BOOL_I32:
		case IASM_TYPE_TYPE:
		case INT32: return 4;
		case FLOAT16:
		case BIN16:
		case BOOL_I16:
		case INT16: return 2;
		case FLOAT8:
		case STRING:
		case CHARACTER:
		case BOOL:
		case BOOL_I8:
		case BIN8:
		case IASM_TYPE_NULL:
		case IASM_TYPE_OPERATOR:
		case INT8: return 1;
		case IPV4: return 4;
		case IPV6: return 16;
		case MAC: return 6;
		case MD5: return 16;
		case RGB24: return 3;
		case RGB32:
		case RGBA32_LEMONOS:
		case RGBA32: return 4;
		case MCDONALDS_US:
		case MCDONALDS_CA:
		case MCDONALDS_CH:
		case MCDONALDS_UK:
		case MCDONALDS_GB:
		case MCDONALDS_DE: return 4;
		case MCDONALDS_SA: return 8;
		case MAT3X3_F32: return (3 * 3) * 4;
		case MAT4X4_F32: return (4 * 4) * 4;
		case MAT6X6_F32: return (6 * 6) * 4;
		case MAT9X9_F32: return (9 * 9) * 4;
		case MAT3X3_F16: return (3 * 3) * 2;
		case MAT4X4_F16: return (4 * 4) * 2;
		case MAT6X6_F16: return (6 * 6) * 2;
		case MAT9X9_F16: return (9 * 9) * 2;
		case MAT3X3_I8: return 3 * 3;
		case MAT4X4_I8: return 4 * 4;
		case MAT6X6_I8: return 6 * 6;
		case MAT9X9_I8: return 9 * 9;
		case MAT16X16_F32: return (16 * 16) * 4;
		case MAT16X32_F16: return (16 * 32) * 2;
		case MAT16X32_B16: return (16 * 32) * 2;
		case MAT16X64_I8: return (16 * 64);
	}
	return 1;
}

char * decode_type_separator(int type) {
	switch (type) {
		case INT8192:
		case INT512:
		case INT256:
		case INT128:
		case FLOAT80:
		case INT80:
		case BIN80:
		case FLOAT64:
		case INT64:
		case BIN64:
		case FLOAT32:
		case INT32:
		case BIN32:
		case FLOAT16:
		case INT16:
		case BIN16:
		case FLOAT8:
		case BOOL:
		case BOOL_I8:
		case BOOL_I16:
		case BOOL_I32:
		case BOOL_I64:
		case CHARACTER:
		case IPV4:
		case IPV6:
		case MAC:
		case MD5:
		case RGB24:
		case RGB32:
		case RGBA32_LEMONOS:
		case RGBA32:
		case BIN8:
		case MCDONALDS_US:
		case MCDONALDS_CA:
		case MCDONALDS_CH:
		case MCDONALDS_UK:
		case MCDONALDS_GB:
		case MCDONALDS_DE:
		case MCDONALDS_SA:
		case IASM_TYPE_TYPE:
		case IASM_TYPE_OPERATOR:
		case IASM_TYPE_FUNCTION:
		case INT8: return ", ";
		case STRING: return "";
		case MAT3X3_F32:
		case MAT4X4_F32:
		case MAT6X6_F32:
		case MAT9X9_F32:
		case MAT3X3_F16:
		case MAT4X4_F16:
		case MAT6X6_F16:
		case MAT9X9_F16:
		case MAT3X3_I8:
		case MAT4X4_I8:
		case MAT6X6_I8:
		case MAT9X9_I8:
		case MAT16X16_F32:
		case MAT16X32_F16:
		case MAT16X32_B16:
		case MAT16X64_I8: return "\n";
		case IASM_TYPE_NULL: return "";
	}
	return ", ";
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

void print_matrix(void * p, int columns, int rows, int type) {
	int size = decode_type_size(type);
	for (int y = 0; y < columns; y++) {
		for (int x = 0; x < rows; x++) {
			print_typed_value(p, type, size);
			p += size;
			printf(", ");
		}
		printf("\n");
	}
}

void print_address(void * p, int type) {
	char buffer[32];
	uint8_t * i8 = p;
	switch (type) {
		case IPV4: inet_ntop(AF_INET, p, buffer, 32); break;
		case IPV6: inet_ntop(AF_INET6, p, buffer, 32); break;
		case MAC: snprintf(buffer, 32, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", i8[0], i8[1], i8[2], i8[3], i8[4], i8[5]); break;
	}
	printf("%s", buffer);
}

void print_mcdonalds(mcdonalds_item_t * items, uint64_t bcd) {
	while (items->bcd != 0xffffffff) {
		if (items->bcd == bcd) {
			printf("%s", items->name);
			return;
		}
		items++;
	}
	printf("???");
}

void print_rgb(void * p, int type) {
	// 0xAARRGGBB,
	uint8_t * i8 = p;
	uint16_t * i16 = p;
	uint32_t * i32 = p;
	switch (type) {
		case RGB32:
		case RGB24: {
			uint32_t colour = i8[0] | (i8[1] << 8) | (i8[2] << 16);
			print_ansii_colour(colour);
			if (rgb_style == RGB_CSS) {
				printf("rgb(%d, %d, %d)", i8[2], i8[1], i8[0]);
			} else if (rgb_style == RGB_BLOCKS) {
				printf("██");
			}
			break;
		}
		case RGBA32_LEMONOS:
			uint32_t _i32 = *i32;
			if (_i32 < 16) {
				uint32_t colour = lemonos_legacy_colour[_i32] | 0xff000000;
				return print_rgb(&colour, RGBA32);
			}
			return print_rgb(p, RGBA32);
		case RGBA32:
			print_ansii_colour(*i32);
			if (rgb_style == RGB_CSS) {
				printf("rgba(%d, %d, %d, %.1f)", i8[2], i8[1], i8[0], ((float) i8[3]) / 255.0f);
			} else if (rgb_style == RGB_BLOCKS) {
				printf("██");
			}
			break;
	}
	print_ansii_reset();
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

	if ((type == CHARACTER || type == STRING) && *int8 == 0) {
		size = remaining; // consum,e whole thing
		return size;
	}

	switch (type) {
#if defined(__X86_64)
		case FLOAT80:	printf("%lf", fpu_float_to_double(float80)); break;
#elif defined(__X86)
		case FLOAT80:	printf("%llf", *(long double *) float80); break;
#endif
		case FLOAT64:	printf("%lf", *float64); break;
		case FLOAT32:	printf("%f", *float32); break;
		case FLOAT16:	printf("%.3lf", float16_decode(*int16)); break;
		case FLOAT8:	printf("%.2lf", float8_decode(*int8)); break;
		case INT8192:	printf("0x");
				for (int i = 0; i < 1024; i++) {
					printf("%.2x", int8[1023 - i]);
				}
				break;
		case INT512:	printf("0x%.16llx%.16llx%.16llx%.16llx%.16llx%.16llx%.16llx%.16llx", int64[7], int64[6], int64[5], int64[4], int64[3], int64[2], int64[1], int64[0]); break;
		case INT256:	printf("0x%.16llx%.16llx%.16llx%.16llx", int64[3], int64[2], int64[1], int64[0]); break;
		case INT128:	printf("0x%.16llx%.16llx", int64[1], int64[0]); break;
		case INT80:	printf("0x%.2x%.16lx", int16[4], *int64); break;
		case INT64:	printf("0x%.16lx", *int64); break;
		case INT32:	printf("0x%.8lx", *int32); break;
		case INT16:	printf("0x%.4x", *int16); break;
		case INT8:	printf("0x%.2x", *int8); break;
		case BIN80:	printf("0b%.16lb%.64lb", int16[4], *int64); break;
		case BIN64:	printf("0b%.64lb", *int64); break;
		case BIN32:	printf("0b%.32lb", *int32); break;
		case BIN16:	printf("0b%.16b", *int16); break;
		case BIN8:	printf("0b%.8b", *int8); break;
		case BOOL:	size = remaining; print_bool(p, size); break; // consume all
		case BOOL_I8:	print_bool(p, size); break;
		case BOOL_I16:	print_bool(p, size); break;
		case BOOL_I32:	print_bool(p, size); break;
		case BOOL_I64:	print_bool(p, size); break;
		case CHARACTER:	printf("'%c'", *int8); break;
		case STRING:	printf("%c", *int8); break;
		case IPV4:	print_address(p, IPV4); break;
		case IPV6:	print_address(p, IPV6); break;
		case MAC:	print_address(p, MAC); break;
		case MD5:	printf("%.16llx%.16llx", int64[1], int64[0]); break;
		case RGB24:		print_rgb(p, RGB24); break;
		case RGB32:		print_rgb(p, RGB32); break;
		case RGBA32:		print_rgb(p, RGBA32); break;
		case RGBA32_LEMONOS:	print_rgb(p, RGBA32_LEMONOS); break;
		case MCDONALDS_US:	print_mcdonalds(us_items, *int32); break;
		case MCDONALDS_CA:	print_mcdonalds(ca_items, *int32); break;
		case MCDONALDS_CH:	print_mcdonalds(ch_items, *int32); break;
		case MCDONALDS_UK:	print_mcdonalds(uk_items, *int32); break;
		case MCDONALDS_GB:	print_mcdonalds(gb_items, *int32); break;
		case MCDONALDS_DE:	print_mcdonalds(de_items, *int32); break;
		case MCDONALDS_SA:	print_mcdonalds(sa_items, *int64); break;
		case MAT3X3_F32:	print_matrix(p, 3, 3, FLOAT32); break;
		case MAT4X4_F32:	print_matrix(p, 4, 4, FLOAT32); break;
		case MAT6X6_F32:	print_matrix(p, 6, 6, FLOAT32); break;
		case MAT9X9_F32:	print_matrix(p, 9, 9, FLOAT32); break;
		case MAT3X3_F16:	print_matrix(p, 3, 3, FLOAT16); break;
		case MAT4X4_F16:	print_matrix(p, 4, 4, FLOAT16); break;
		case MAT6X6_F16:	print_matrix(p, 6, 6, FLOAT16); break;
		case MAT9X9_F16:	print_matrix(p, 9, 9, FLOAT16); break;
		case MAT3X3_I8:		print_matrix(p, 3, 3, INT8); break;
		case MAT4X4_I8:		print_matrix(p, 4, 4, INT8); break;
		case MAT6X6_I8:		print_matrix(p, 6, 6, INT8); break;
		case MAT9X9_I8:		print_matrix(p, 9, 9, INT8); break;
		case MAT16X16_F32:	print_matrix(p, 16, 32, FLOAT32); break;
		case MAT16X32_F16:	print_matrix(p, 16, 32, FLOAT16); break;
		case MAT16X32_B16:	print_matrix(p, 16, 32, FLOAT16); break; // todo...
		case MAT16X64_I8:	print_matrix(p, 16, 64, INT8); break;
		case IASM_TYPE_TYPE:	printf("???"); break;
		case IASM_TYPE_OPERATOR:printf("???"); break;
		case IASM_TYPE_FUNCTION:
			if (sizeof(uintptr_t) == 8) {
				printf("@0x%.16lx(...)", *int64);
				break;
			}
			printf("@0x%.8lx(...)", *int32);
			break;
		case IASM_TYPE_NULL:	break;
	}
	return size;
}

void print_typed_bytes(void * p, int type, int size) {
	char * separator = decode_type_separator(type);
	while (size > 0) {
		int printed = print_typed_value(p, type, size);
		p += printed;
		size -= printed;
		if (separator && size) {
			printf("%s", separator);
		}
	}
}
