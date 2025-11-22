#include <iasm/features.h>
#include <stdint.h>
#include <string.h>

uint32_t feature_flags = 0; //TRAP_EMULATION;

uint32_t decode_feature_flag(char * name) {
	if (strcmp(name, "emulation") == 0) {
		return TRAP_EMULATION;
	}
	return 0;
}

