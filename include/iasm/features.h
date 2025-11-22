#pragma once

#include <stdint.h>

enum {
	TRAP_EMULATION = 0b00000001,
};

extern uint32_t feature_flags;

uint32_t decode_feature_flag(char * name);
