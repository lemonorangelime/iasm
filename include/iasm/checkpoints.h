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
	char * statement;
	int in_use;
	int setup;
} checkpoint_t;

extern checkpoint_t * checkpoints;
extern int checkpoint_count;
extern int present_checkpoint;
extern int past_checkpoint;
extern int current_checkpoint;

char * get_checkpoint_statement();

void checkpoint_save_p(checkpoint_t * checkpoint, char * statement);
void checkpoint_load_p(checkpoint_t * checkpoint);
void checkpoint_save(char * statement);
void checkpoint_load();
int checkpoint_limit(int checkpoint);
void checkpoint_advance(char * statement);
int checkpoint_wind();
int checkpoint_rewind();
void checkpoint_update();
void checkpoint_init();
