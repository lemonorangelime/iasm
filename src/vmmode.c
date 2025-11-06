#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <vars.h>
#include <asm.h>
#include <vmmode.h>
#include <vm.h>

static uint32_t ram_size = 0x1000;
static int cpus = 1;

static char * bios_paths[] = {
	"/usr/share/iasm/bios/bios.bin",
	"share/bios/bios.bin",
	"bios/bios.bin",
};
static int paths = sizeof(bios_paths) / sizeof(bios_paths[0]);

vm_t * vm = NULL;
int vmmode = 0;

char * lookup_bios_path() {
	for (int i = 0; i < paths; i++) {
		if (access(bios_paths[i], R_OK) == 0) {
			return bios_paths[i];
		}
	}
	return NULL;
}

int vmmode_init() {
	char * bios_path = lookup_bios_path();
	if (!bios_path) {
		printf("NO SUITABLE BIOS FOUND\n");
		return -1;
	}

	vm = vm_create(ram_size, cpus);
	if (vm_load_system_bios(vm, bios_path)) {
		printf("BAD BIOS\n");
		return -1;
	}
	vm_mmap(vm, exec_buffer, exec_buffer, EXEC_SIZE); // map the code and stack buffers to the VM
	vm_mmap(vm, stack_buffer, stack_buffer, STACK_SIZE);
	vm_reset(vm);
	vm_register_default_devices(vm);
	vmmode = 1;
	return 0;
}

uint64_t vmmode_resume() {
	if (!vm) {
		printf("VM MODE ERROR\n");
		return 1;
	}

	vm_cpu_t * bsp = vm_get_bsp(vm);
	vm_cpu_enter(vm, bsp);
	vm_destroy(vm);
	printf("HELLO!");
	return 0;
}
