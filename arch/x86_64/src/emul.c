#include <stdint.h>
#include <iasm/signals.h>
#include <fcntl.h>
#include <unistd.h>

int attempt_emulation(greg_t * registers) {
	uintptr_t * eip = (uintptr_t *) &registers[REG_PC];
	uint8_t * p = (uint8_t *) *eip;
	if ((p[0] == 0x0f) && (p[1] == 0x32)) { // rdmsr
		int fd = open("/dev/cpu/0/msr", O_RDONLY); // todo: don't assume CPU 0
		if (fd <= 0) {
			return 0;
		}
		uint64_t msr;
		if (pread(fd, &msr, 8, registers[REG_RCX] & 0xffffffff) != 8) {
			close(fd);
			return 0;
		}
		registers[REG_RAX] &= ~0xffffffff;
		registers[REG_RDX] &= ~0xffffffff;
		registers[REG_RAX] |= msr         & 0xffffffff;
		registers[REG_RDX] |= (msr >> 32) & 0xffffffff;
		*eip += 2;
		close(fd);
		return 1;
	} else if ((p[0] == 0x0f) && (p[1] == 0x30)) { // wrmsr
		int fd = open("/dev/cpu/0/msr", O_WRONLY); // todo: don't assume CPU 0
		if (fd <= 0) {
			return 0;
		}
		uint64_t msr = ((registers[REG_RDX] & 0xffffffff) << 32) | (registers[REG_RAX] & 0xffffffff);
		if (pwrite(fd, &msr, 8, registers[REG_RCX] & 0xffffffff) != 8) {
			close(fd);
			return 0;
		}
		*eip += 2;
		close(fd);
		return 1;
	}
	return 0;
}
