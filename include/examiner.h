#pragma once

#include <stdint.h>
#include <string.h>

typedef struct {
	char code;
	char * specifiers[4];
} examine_type_t;

enum {
	EXAMINE_BYTE_INDEX = 0,
	EXAMINE_WORD_INDEX = 1,
	EXAMINE_DWORD_INDEX = 2,
	EXAMINE_QWORD_INDEX = 3
};

enum {
	EXAMINE_BYTE_SIZE = 1,
	EXAMINE_WORD_SIZE = 2,
	EXAMINE_DWORD_SIZE = 4,
	EXAMINE_QWORD_SIZE = 8
};

int examiner_decode_size_index(char size);
int examiner_decode_size(char size);
uint64_t examiner_read(void ** p, char size);
char * examiner_type_specifier(char type, char size);
int examine(char * line);
