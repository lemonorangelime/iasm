#pragma once

#include <stdint.h>

enum {
	TRAP_EMULATION   = 0b00000001,
	FEAT_CHECKPOINTS = 0b00000010,
};

extern uint32_t feature_flags;
extern int verbose;
extern int allow_colour;
extern char * replay_file;

uint32_t decode_feature_flag(char * name);
