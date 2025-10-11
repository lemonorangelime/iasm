#include <helpers.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <asm.h>

uint8_t * exec_buffer = (uint8_t *) 0x01000000;
size_t exec_buffer_size = 0;

int call_nasm() {
	pid_t pid = fork();
	if (pid == -1) {
		return -1;
	}
	if (pid == 0) {
		close(0);
		close(1);
		close(2);
		execl("/usr/bin/nasm", "nasm", "/tmp/.temp_asm.asm", "-o", "/tmp/.temp_output.bin", (char *) NULL);
		exit(0);
	}
	int stat = 0;
	waitpid(pid, &stat, 0);
	return stat;
}

void setup_executable_buffer() {
	exec_buffer = (uint8_t *) mmap(exec_buffer, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // whatever :shrug:
	return_point = (uint64_t) exec_buffer;
}

uint64_t execute(void * buffer, ssize_t size, int skip) {
	memcpy(exec_buffer + exec_buffer_size, buffer, size);

	exec_buffer_size += size;
	exec_buffer[exec_buffer_size + 0] = 0xff;
	exec_buffer[exec_buffer_size + 1] = 0x25;
	exec_buffer[exec_buffer_size + 2] = 0x00;
	exec_buffer[exec_buffer_size + 3] = 0x00;
	exec_buffer[exec_buffer_size + 4] = 0x00;
	exec_buffer[exec_buffer_size + 5] = 0x00;
	*(uint64_t *) &exec_buffer[exec_buffer_size + 6] = (uint64_t) &asm_continue;
	if (!skip) {
		asm_resume();
	}
	return_point = ((uint64_t) exec_buffer) + exec_buffer_size;
	return 0;
}

void asm_reset() {
	remove("/tmp/.temp_asm.asm");

	int fd = open("/tmp/.temp_asm.asm", O_CREAT | O_RDWR, 0664);
	write(fd, "bits 64\norg 0x01000000\n", 23);
	close(fd);

	setup_fpu();
}

void asm_rewind() {
	int fd = open("/tmp/.temp_asm.asm", O_CREAT | O_RDWR, 0664);
	ssize_t size = 0;
	char c = 0;

	lseek(fd, 0, SEEK_END);
	lseek(fd, -2, SEEK_CUR);

	read(fd, &c, 1);
	size = lseek(fd, -1, SEEK_CUR);
	while (c != '\n') {
		read(fd, &c, 1);
		size = lseek(fd, -2, SEEK_CUR);
	}
	lseek(fd, 0, SEEK_SET);
	ftruncate(fd, size + 2);
	close(fd);
}

int assemble(char * instruction, void ** buffer, ssize_t * size) {
	int nasm_status = 0;
	int fd = open("/tmp/.temp_asm.asm", O_CREAT | O_RDWR | O_APPEND, 0664);
	write(fd, instruction, strlen(instruction));
	write(fd, "\n", 1);
	close(fd);

	nasm_status = call_nasm();

	fd = open("/tmp/.temp_output.bin", O_RDONLY);
	*size = fdsize(fd) - exec_buffer_size;
	*buffer = malloc(*size);
	lseek(fd, exec_buffer_size, SEEK_SET);
	read(fd, *buffer, *size);
	close(fd);

	remove("/tmp/.temp_output.bin");
	return nasm_status;
}
