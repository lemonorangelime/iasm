#include <iasm/examiner.h>
#include <iasm/helpers.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <iasm/floats.h>
#include <iasm/signals.h>
#include <iasm/asm.h>
#include <iasm/disasm.h>

static examine_type_t examiner_types[] = {
//	{code, {"byte",  "word",    "dword",    "qword"}}
	{'x', {"0x%.2x", "0x%.4x",  "0x%.8x",   "0x%.16llx"}},
	{'d', {"%d",     "%d",      "%d",       "%lld"}},
	{'u', {"%u",     "%u",	    "%u",       "%llu"}},
	{'o', {"0%.3o",  "0%.6o",   "0%.11o",   "0%.22llo"}},
	{'c', {"'%c'",   "'%lc'",   "'%llc'",   "'%llc'"}},
	{'b', {"0b%.8b", "0b%.16b", "0b%.32b",  "0b%.64llb"}},
	{'f', {"%lf",	 "%lf",	    "%lf",	"%lf"}},
	{'i', {"%s",	 "%s",	    "%s",	"%s"}}
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
	return (size != 'b') && (size != 'w') && (size != 'd') && (size != 'q');
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
	safe_memcpy(&data, *p, byte_size);
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

int print_float(uint64_t data, char size) {
	double (* decoders[])(uint64_t) = {float8_decode, float16_decode, float32_decode, float64_decode};
	register double d = 0.0;
	int i = examiner_decode_size_index(size);
	return printf("%lf", decoders[i](data));
}

int examine(char * line) {
	if ((*line != 'x') || (line[1] != '/' && line[1] != ' ')) {
		return 0; // match failed
	}
	uintptr_t address = 0;
	int count = 1;
	char type = 'x';
	char size = 'b';

	line = examiner_parse_slash(line, &type, &size, &count);
	if (examiner_validate_size(size) || examiner_validate_type(type)) {
		return 0; // match failed
	}

	if (resolve_register_or_label(line, &address)) {
		return 0; // match failed
	}

	char * specifier = examiner_type_specifier(type, size);
	void * p = (void *) address;
	int printed = (type == 'i') ? 0 : printf("0x%.16llx: ", p);
	while (count--) {
		if (type == 'i') {
			char s[16] = {0};
			void * oldp = p;
			p += disasm_one(p, s);
			printf("0x%.16llx: %s\n", oldp, s);
		} else {
			uint64_t data = examiner_read(&p, size);
			if (signaled) { // shit
				return 1;
			}
			if (type == 'f') {
				printed += print_float(data, size);
			} else {
				printed += printf(specifier, data);
			}
			fflush(stdout);
			putchar(count ? ' ' : 0);
			fflush(stdout);
			if ((printed > 80) && count) {
				printed = 0;
				write(1, "\n", 1);
				printed += printf("0x%.16llx: ", p);
				fflush(stdout);
			}
		}
	}
	write(1, "\n", !!printed);
	return 1;
}
