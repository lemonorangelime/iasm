#include <iasm/features.h>
#include <stdint.h>
#include <string.h>

uint32_t feature_flags = FEAT_CHECKPOINTS; //TRAP_EMULATION;
int verbose = 0;
int allow_colour = 1;
char * replay_file = NULL;

uint32_t decode_feature_flag(char * name) {
	if (strcmp(name, "emulation") == 0) {
		return TRAP_EMULATION;
	} else if (strcmp(name, "checkpoint") == 0) {
		return FEAT_CHECKPOINTS;
	}
	return 0;
}

