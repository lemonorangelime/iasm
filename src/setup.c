#include <sys/io.h>
#include <iasm/helpers.h>
#include <time.h>
#include <stdlib.h>

char asm_src_path[128];
char asm_out_path[128];

void setup() {
	ioperm(0x0000, 0xffff, 1);
	iopl(3);

	srandom(time(NULL));
	tmpname(asm_src_path);
	tmpname(asm_out_path);
}
