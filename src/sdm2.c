#include <iasm/sdm.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

int sdm_part_length(char * part) {
	int i = 0;
	while (*part != 0 && *part != ':') { i++; part++; }
	return i;
}

int sdm_parts_remaining(char * part) {
	return *part != 0;
}

int sdm_name_match(intel_sdm_t * entry, char * instruction) {
	if (strcmp(entry->name, instruction) == 0) {
		return 1;
	}
	int entry_length = strlen(entry->name);
	int instr_length = strlen(instruction);
	if (instr_length >= entry_length) {
		return 0;
	}

	int stat = 1;
	char * part = entry->name;
	while (sdm_parts_remaining(part)) {
		int part_length = sdm_part_length(entry->name);
		if ((part_length == instr_length) && (memcmp(part, instruction, part_length) == 0)) {
			return 1;
		}
		part += part_length + 1;
	}
	return 0;
}

intel_sdm_t * sdm_entry(char * instruction) {
	intel_sdm_t * entry = &sdm[0];
	while (entry->name) {
		if (sdm_name_match(entry, instruction)) {
			return entry;
		}
		entry++;
	}
	return NULL;
}

void sdm_print(intel_sdm_t * entry) {
	char * description = entry->description;
	int s = strlen(description);
	if (description[s-1] != '\n') {
		printf("%s\n", description);
	} else {
		printf("%s", description);
	}
}
