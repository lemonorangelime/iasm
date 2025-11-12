#include <regs.h>

int cpufeat_level = 0;

void test_cpufeats();
void setup_builtins() {
	print_flags = PRINT_GENERAL | PRINT_FPU;

	test_cpufeats();
	switch (cpufeat_level) {
		case 1:	print_flags |= PRINT_XMM; return;
		case 2: print_flags |= PRINT_YMM; return;
		case 3: print_flags |= PRINT_ZMM; return;
	}
}
