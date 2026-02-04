#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	void * code_contents;
	void * jump_table;
	void * text_contents;
	void * stack_contents;
	void * registers_save;
	size_t text_size;
	size_t code_size;
	int setup;
} checkpoint_t;

extern checkpoint_t * checkpoints;
extern int checkpoint_count;
extern int current_checkpoint;

void checkpoint_save();
void checkpoint_load();
void checkpoint_advance();
void checkpoint_wind();
void checkpoint_rewind();
void checkpoint_update();
void checkpoint_init();
