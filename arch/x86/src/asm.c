// todo: break this up

#include <iasm/helpers.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iasm/asm.h>
#include <iasm/regs.h>
#include <iasm/vars.h>
#include <iasm/setup.h>

// // stack and ram must be fixed
// not anymore buddy
uint8_t * exec_buffer  = NULL;
uint8_t * jmp_buffer   = NULL;
uint8_t * stack_buffer = NULL;
size_t exec_buffer_size = 0;
size_t jmp_buffer_size = 0;

int call_nasm(int * pipefd) {
	int pipes[2];
	if (pipe(pipes) == -1) {
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		return -1;
	}
	if (pid == 0) {
		close(0);
		close(1);
		dup2(pipes[1], 2);
		close(pipes[0]);
		close(pipes[1]);
		execl("/usr/bin/nasm", "nasm", "-Xgnu", "-w+error", "-w-error=zeroing", "-w-zeroing", asm_src_path, "-o", asm_out_path, NULL);
		exit(0);
	}
	int stat = 0;
	waitpid(pid, &stat, 0);

	if (pipefd) {
		*pipefd = pipes[0];
	} else {
		close(pipes[0]);
	}
	close(pipes[1]);
	return stat;
}

void setup_executable_buffer() {
	exec_buffer  = (uint8_t *) EXEC_ADDRESS;
	jmp_buffer   = (uint8_t *) JMP_ADDRESS;
	stack_buffer = (uint8_t *) STACK_ADDRESS;

	exec_buffer = (uint8_t *) mmap(exec_buffer, EXEC_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // whatever :shrug:
	jmp_buffer = (uint8_t *) mmap(jmp_buffer, JMP_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // whatever :shrug:
	stack_buffer = (uint8_t *) mmap(stack_buffer, STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // executable stack because yeah
	return_point = (uintptr_t) exec_buffer;
}

void setup_registers() {
	//memset(&register_save, 0, sizeof(registers_t));
	//memset(&fpu_save, 0, 1024);
	memset(exec_buffer, 0, EXEC_SIZE);
	memset(jmp_buffer, 0, JMP_SIZE);
	memset(stack_buffer, 0, STACK_SIZE);

	return_point = (uintptr_t) exec_buffer;
	register_save.esp = ((uintptr_t) stack_buffer) + STACK_SIZE;
	setup_cpu();
}

uint64_t asm_execute(void * buffer, ssize_t size, int skip) {
	memcpy(exec_buffer + exec_buffer_size, buffer, size);

	exec_buffer_size += size;
	exec_buffer[exec_buffer_size + 0] = 0x68; // push asm_exit_context
	exec_buffer[exec_buffer_size + 5] = 0xc3; // ret
	*(uint32_t *) &exec_buffer[exec_buffer_size + 1] = (uint32_t) asm_exit_context;

	if (!skip) {
		asm_resume();
	}
	return_point = ((uintptr_t) exec_buffer) + exec_buffer_size;
	return 0;
}

void * asm_append_jmptable(void * symbol) {
	void * trampoline = &jmp_buffer[jmp_buffer_size];

	jmp_buffer[jmp_buffer_size + 0] = 0x68; // push ? (symbol)
	jmp_buffer[jmp_buffer_size + 5] = 0xc3; // ret
	*(uint32_t *) &jmp_buffer[jmp_buffer_size + 1] = (uintptr_t) symbol;

	jmp_buffer_size += 6;
	return trampoline;
}

void asm_reset() {
	remove(asm_src_path);

	char header[128] = {0};
	snprintf(header, 128, "bits 32\ndefault abs\ncpu all\norg %p\n", (uintptr_t) exec_buffer);

	int fd = open(asm_src_path, O_CREAT | O_RDWR, 0664);
	write(fd, header, strlen(header));
	close(fd);

	setup_registers();
	setup_fpu();
}

void asm_cleanup() {
	remove(asm_src_path);
}

void asm_rewind() {
	int fd = open(asm_src_path, O_CREAT | O_RDWR, 0664);
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

int assemble(char * instruction, void ** buffer, ssize_t * size, int * pipefd) {
	int nasm_status = 0;
	int fd = open(asm_src_path, O_CREAT | O_RDWR | O_APPEND, 0664);
	write(fd, instruction, strlen(instruction));
	write(fd, "\n", 1);
	close(fd);

	nasm_status = call_nasm(pipefd);

	fd = open(asm_out_path, O_RDONLY);
	*size = my_fdsize(fd) - exec_buffer_size;
	*buffer = malloc(*size);
	lseek(fd, exec_buffer_size, SEEK_SET);
	read(fd, *buffer, *size);
	close(fd);

	remove(asm_out_path);
	return nasm_status;
}

void arch_save_registers(void * regsave) {
        uintptr_t register_size = (uintptr_t) (((void*) &register_save_end) - ((void*) &register_save));
        memcpy(regsave, &register_save, register_size);
        memcpy(regsave + 512, (void *) &fpu_save, 16384);
}

void arch_load_registers(void * regsave) {
        uintptr_t register_size = (uintptr_t) (((void*) &register_save_end) - ((void*) &register_save));
        memcpy(&register_save, regsave, register_size);
        memcpy((void *) &fpu_save, regsave + 512, 16384);
}
