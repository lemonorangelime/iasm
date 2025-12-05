#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <iasm/features.h>

uint64_t rdmsr(uint32_t ecx) {
	uint64_t value;
	asm volatile ("rdmsr" : "=A"(value) : "c"(ecx));
	return value;
}

void arch_setup() {
	if (getuid() != 0) {
		return;
	}
	uint32_t oldflags = feature_flags;
	feature_flags |= TRAP_EMULATION;

	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	uint64_t apic_address = rdmsr(0x1b); // APIC_BASE_MSR
	mmap((void *) (uintptr_t) (apic_address & ~0xfffllu), 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, apic_address & ~0xfffllu);
	feature_flags = oldflags;
}
