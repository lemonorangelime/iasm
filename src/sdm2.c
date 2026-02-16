#include <iasm/sdm.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

int sdm_part_length(char * part) {
	int i = 0;
	while (*part != 0 && *part != ':') { i++; part++; }
	return i;
}

int sdm_is_part(char * part) {
	while (*part != 0 && *part != ':') { part++; }
	return *part == ':';
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
	char * part_end = part + strlen(part);
	while (*part && (part < part_end)) {
		int part_length = sdm_part_length(part);
		if ((part_length == instr_length) && (memcmp(part, instruction, part_length) == 0)) {
			return 1;
		}
		part += part_length + sdm_is_part(part);
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
