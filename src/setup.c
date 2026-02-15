#include <stdio.h>
#include <sys/io.h>
#include <iasm/helpers.h>
#include <iasm/features.h>
#include <iasm/setup.h>
#include <time.h>
#include <stdlib.h>

char asm_src_path[128];
char asm_out_path[128];

void setup() {
	ioperm(0x0000, 0xffff, 1);
	iopl(3);

	arch_setup();

	srandom(time(NULL));
	tmpname(asm_src_path);
	tmpname(asm_out_path);

	if (verbose) {
		printf("temp in: %s\n", asm_src_path);
		printf("temp out: %s\n", asm_out_path);
	}
}
