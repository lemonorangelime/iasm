#include <sys/sendfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <helpers.h>
#include <signals.h>
#include <stdio.h>
#include <regs.h>
#include <asm.h>
#include <builtins.h>

int paused = 0;

void cut_newline(char * string) {
	char c = 0;
	while (c = *string) {
		if (c == '\n') {
			*string = '\0';
			return;
		}
		string++;
	}
}

int handle_statement(char * line) {
	int builtin_matches = 0;
	cut_newline(line);
	builtin_matches = execute_builtins(line);
	if (builtin_matches == 2) {
		return 1;
	} else if (builtin_matches == 1) {
		return 0;
	}

	void * buffer = NULL;
	ssize_t size = 0;
	int skip = line[0] == '~';
	int stat = assemble(line + skip, &buffer, &size);
	if (size == 0) {
		return 0; // dont waste time executing nothing
	}
	if (size == -1 || stat != 0) {
		printf("assembler error\n");
		asm_rewind();
		return 0;
	}
	asm_execute(buffer, size, paused || skip);
	free(buffer);
	return 0;
}

ssize_t read_input(char ** line, size_t * size) {
	ssize_t r = 0;
	write(1, "> ", 2);
	r = getline(line, size, stdin);
	if (r == -1) {
		putchar('\n');
	}
	return r;
}

int main(int argc, char * argv[]) {
	char * line = NULL;
	size_t line_size = 0;
	register_handlers();
	setup_executable_buffer();
	asm_reset();
	while (read_input(&line, &line_size) != -1) {
		if (line_size == 0) {
			continue;
		}
		if (handle_statement(line)) {
			break;
		}
		signaled = 0;
	}
	return 0;
}
