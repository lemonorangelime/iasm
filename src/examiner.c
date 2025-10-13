#include <examiner.h>
#include <helpers.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

static examine_type_t examiner_types[] = {
//	{code, {"byte",  "word",    "dword",    "qword"}}
	{'x', {"0x%.2x", "0x%.4x",  "0x%.8x",   "0x%.16llx"}},
	{'d', {"%d",     "%d",      "%d",       "%lld"}},
	{'u', {"%u",     "%u",	    "%u",       "%llu"}},
	{'o', {"0%.3o",  "0%.6o",   "0%.11o",   "0%.22llo"}},
	{'c', {"'%c'",   "'%lc'",   "'%llc'",   "'%llc'"}},
	{'b', {"0b%.8b", "0b%.16b", "0b%.32b",  "0b%.64llb"}}
};

static int examiner_type_count = sizeof(examiner_types) / sizeof(examiner_types[0]);
static char * default_specifier = "0x%.2x";



int examiner_decode_size_index(char size) {
	switch (size) {
		case 'b': return EXAMINE_BYTE_INDEX;
		case 'w': return EXAMINE_WORD_INDEX;
		case 'd': return EXAMINE_DWORD_INDEX;
		case 'q': return EXAMINE_QWORD_INDEX;
	}
	return EXAMINE_DWORD_INDEX;
}

int examiner_decode_size(char size) {
	switch (size) {
		case 'b': return EXAMINE_BYTE_SIZE;
		case 'w': return EXAMINE_WORD_SIZE;
		case 'd': return EXAMINE_DWORD_SIZE;
		case 'q': return EXAMINE_QWORD_SIZE;
	}
	return EXAMINE_DWORD_SIZE;
}

char * examiner_type_specifier(char type, char size) {
	int index = examiner_decode_size_index(size);

	for (int i = 0; i < examiner_type_count; i++) {
		if (examiner_types[i].code == type) {
			return examiner_types[i].specifiers[index];
		}
	}
	return default_specifier;
}

int examiner_validate_size(char size) {
	return (size != 'b') && (size != 'w') && (size != 'd') && (size == 'q');
}

int examiner_validate_type(char type) {
	for (int i = 0; i < examiner_type_count; i++) {
		if (examiner_types[i].code == type) {
			return 0;
		}
	}
	return 1;
}

uint64_t examiner_read(void ** p, char size) {
	uint64_t data = 0;
	int byte_size = examiner_decode_size(size);
	memcpy(&data, *p, byte_size);
	*p += byte_size;
	return data;
}

char * read_char(char * string, char * c) {
	if (*string == ' ') {
		return string;
	}
	*c = *string++;
	return string;
}

char * examiner_parse_slash(char * line, char * type, char * size, int * count) {
	if (line[1] == ' ') {
		return line + 2;
	}

	char * count_str = line + 2;
	line += 2;
	while (isdigit(*line)) { line++; }

	if (count_str != line) {
		*count = atoi(count_str);
	}

	line = read_char(line, type);
	line = read_char(line, size);
	return line + 1;
}

int examine(char * line) {
	if ((*line != 'x') || (line[1] != '/' && line[1] != ' ')) {
		return 0; // match failed
	}
	uint64_t address = 0;
	int count = 1;
	char type = 'x';
	char size = 'b';

	line = examiner_parse_slash(line, &type, &size, &count);

	if (examiner_validate_size(size) || examiner_validate_type(type)) {
		return 0; // match failed
	}

	if (resolve_label(line, &address)) {
		return 0; // match failed
	}

	char * specifier = examiner_type_specifier(type, size);
	void * p = (void *) address;
	int i = 0;
	while (count--) {
		uint64_t data = examiner_read(&p, size);
		printf(specifier, data);
		putchar(count ? ' ' : 0);
		fflush(stdout);
		if (i++ == 8) {
			write(1, "\n", 1);
		}
	}
	write(1, "\n", !!(i % 8));
	return 1;
}
