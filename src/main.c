#include <sys/sendfile.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <iasm/helpers.h>
#include <iasm/signals.h>
#include <iasm/regs.h>
#include <iasm/asm.h>
#include <iasm/builtins.h>
#include <iasm/dynamic.h>
#include <iasm/setup.h>
#include <iasm/nasm.h>
#include <iasm/checkpoints.h>

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
		return 2;
	} else if (builtin_matches == 1) {
		return 1;
	}

	void * buffer = NULL;
	ssize_t size = 0;
	int skip = line[0] == '~';
	int pipefd;
	int stat = assemble(line + skip, &buffer, &size, &pipefd);
	if (size == 0) {
		close(pipefd);
		return 0; // dont waste time executing nothing
	}
	if (size == -1 || stat != 0) {
		nasm_print_buffer(pipefd);
		close(pipefd);
		asm_rewind();
		return 0;
	}
	close(pipefd);

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

int is_whitespace(char * line) {
	int line_size = strlen(line);
	for (size_t i = 0; i < line_size; i++) {
		if (!isspace(line[i])) {
			return 0;
		}
	}
	return 1;
}

int main(int argc, char * argv[]) {
	char * line = NULL;
	size_t line_size = 0;
	register_handlers();
	setup();
	setup_executable_buffer();
	asm_reset();
	setup_builtins();
	checkpoint_init();
	dynamic_load_defaults();
	while (read_input(&line, &line_size) != -1) {
		if (line_size == 0 || is_whitespace(line)) {
			continue;
		}
		int r = handle_statement(line);
		signaled = 0;
		if (r == 2) {
			break;
		} else if (r == 1) {
			continue;
		}
		checkpoint_advance();
	}
	asm_cleanup();
	return 0;
}
