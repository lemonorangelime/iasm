// todo: break this up

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
#include <regs.h>
#include <vars.h>

// stack and ram must be fixed
uint8_t * exec_buffer = (uint8_t *) 0x01000000;
uint8_t * stack_buffer = (uint8_t *) 0x00800000;
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
		execl("/usr/bin/nasm", "nasm", "/tmp/.temp_asm.asm", "-o", "/tmp/.temp_output.bin", NULL);
		exit(0);
	}
	int stat = 0;
	waitpid(pid, &stat, 0);
	return stat;
}

void setup_executable_buffer() {
	exec_buffer = (uint8_t *) mmap(exec_buffer, EXEC_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // whatever :shrug:
	stack_buffer = (uint8_t *) mmap(stack_buffer, STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // executable stack because yeah
	return_point = (uintptr_t) exec_buffer;
}

void setup_registers() {
	//memset(&register_save, 0, sizeof(registers_t));
	//memset(&fpu_save, 0, 1024);
	memset(exec_buffer, 0, 4096);
	memset(stack_buffer, 0, STACK_SIZE);

	return_point = (uintptr_t) exec_buffer;
	register_save.esp = ((uintptr_t) stack_buffer) + STACK_SIZE;
}

uint64_t asm_execute(void * buffer, ssize_t size, int skip) {
	memcpy(exec_buffer + exec_buffer_size, buffer, size);

	exec_buffer_size += size;
	exec_buffer[exec_buffer_size + 0] = 0x68; // push asm_continue
	exec_buffer[exec_buffer_size + 5] = 0xc3; // ret
	*(uint32_t *) &exec_buffer[exec_buffer_size + 1] = (uint32_t) asm_continue;

	if (!skip) {
		asm_resume();
	}
	return_point = ((uintptr_t) exec_buffer) + exec_buffer_size;
	return 0;
}

void asm_reset() {
	remove("/tmp/.temp_asm.asm");

	int fd = open("/tmp/.temp_asm.asm", O_CREAT | O_RDWR, 0664);
	write(fd, "bits 32\norg 0x01000000\n", 23);
	close(fd);

	setup_registers();
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
	*size = my_fdsize(fd) - exec_buffer_size;
	*buffer = malloc(*size);
	lseek(fd, exec_buffer_size, SEEK_SET);
	read(fd, *buffer, *size);
	close(fd);

	remove("/tmp/.temp_output.bin");
	return nasm_status;
}
