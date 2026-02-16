#pragma once

typedef struct {
	char * name;
	char * description;
} intel_sdm_t;

extern intel_sdm_t sdm[];

intel_sdm_t * sdm_entry(char * instruction);
void sdm_print(intel_sdm_t * entry);
