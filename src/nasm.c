#include <iasm/setup.h>
#include <unistd.h>

char * nasm_buffer_cut(int * read, char * buffer) {
	char * p = buffer;
	char * src = asm_src_path;
	while (--*read && *p++ == *src++) {}
	while (--*read && *p++ != ':') {}
	if (*read <= 0) {
		return p;
	}
	p++;
	--*read;
	return p;
}

int nasm_buffer_line_length(int * read, char * buffer) {
	char * p = buffer;
	int r = *read;
	int length = 0;
	while (r-- && *p++ != '\n') {
		length++;
	}
	return length;
}

char * nasm_buffer_printl(int * read, char * buffer) {
	char * cut = nasm_buffer_cut(read, buffer);
	if (!*read) {
		return NULL;
	}
	int line_length = nasm_buffer_line_length(read, cut);
	(void) write(1, cut, line_length + 1);
	*read -= line_length + 1;
	if (!*read) {
		return NULL;
	}
	return cut + line_length + 1;
}

void nasm_print_buffer(int fd) {
	char buffer[4096];
	int bread = read(fd, buffer, 4096);
	char * p = buffer;
	while (p) {
		p = nasm_buffer_printl(&bread, p);
	}
}
