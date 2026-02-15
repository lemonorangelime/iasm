#include <iasm/features.h>
#include <stdio.h>

void arch_setup() {
	if (verbose) {
		printf("NO MMU\n");
	}
}
