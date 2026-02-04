#include <iasm/checkpoints.h>
#include <iasm/helpers.h>
#include <iasm/arch.h>
#include <iasm/vars.h>
#include <iasm/regs.h>
#include <iasm/asm.h>
#include <string.h>
#include <stdlib.h>

checkpoint_t * checkpoints = NULL;
int checkpoint_count = 16;
int current_checkpoint = 0;

void checkpoint_save() {
        ssize_t size = asm_src_fdsize();
	checkpoint_t * checkpoint = &checkpoints[current_checkpoint];
	memcpy(checkpoint->code_contents, (void*) EXEC_ADDRESS, EXEC_SIZE);
	memcpy(checkpoint->jump_table, (void*) JMP_ADDRESS, JMP_SIZE);
	memcpy(checkpoint->stack_contents, (void*) STACK_ADDRESS, STACK_SIZE);
	if (checkpoint->text_contents != NULL) {
		free(checkpoint->text_contents);
	}
	checkpoint->text_contents = malloc(size);
	checkpoint->code_size = exec_buffer_size;
	checkpoint->text_size = size;
	asm_src_readall(checkpoint->text_contents);
	arch_save_registers(checkpoint->registers_save);
}

void checkpoint_load() {
	checkpoint_t * checkpoint = &checkpoints[current_checkpoint];
	memcpy((void*) EXEC_ADDRESS, checkpoint->code_contents, EXEC_SIZE);
	memcpy((void*) JMP_ADDRESS, checkpoint->jump_table, JMP_SIZE);
	memcpy((void*) STACK_ADDRESS, checkpoint->stack_contents, STACK_SIZE);
	if (checkpoint->text_contents != NULL) {
		asm_src_writeall(checkpoint->text_contents, checkpoint->text_size);
	}
	exec_buffer_size = checkpoint->code_size;
	arch_load_registers(checkpoint->registers_save);
}

void checkpoint_bound_check() {
	if (current_checkpoint < 0) {
		current_checkpoint = 0;
	}
	if (current_checkpoint >= checkpoint_count) {
		current_checkpoint %= checkpoint_count;
	}
}

void checkpoint_update() {
	for (int i = 0; i < checkpoint_count; i++) {
		checkpoint_t * checkpoint = &checkpoints[i];
		if (checkpoint->setup) {
			continue;
		}
		checkpoint->code_contents = malloc(EXEC_SIZE);
		checkpoint->jump_table = malloc(JMP_SIZE);
		checkpoint->text_contents = NULL; //malloc(file_size);
		checkpoint->stack_contents = malloc(STACK_SIZE);
		checkpoint->registers_save = malloc(REGISTER_SAVE_SIZE);
		checkpoint->text_size = 0;
		checkpoint->setup = 1;
	}
}

void checkpoint_advance() {
	current_checkpoint++;
	checkpoint_bound_check();
	checkpoint_save(); // init new checkpoint to continue
}

void checkpoint_wind() {
	current_checkpoint++;
	checkpoint_bound_check();
	checkpoint_load(); // load new state
}

void checkpoint_rewind() {
	current_checkpoint--;
	checkpoint_bound_check();
	checkpoint_load(); // load old state
}

void checkpoint_init() {
	checkpoints = malloc(sizeof(checkpoint_t) * checkpoint_count);
	memset(checkpoints, 0, sizeof(checkpoint_t) * checkpoint_count);
	checkpoint_update();

	for (int i = 0; i < checkpoint_count; i++) {
		current_checkpoint = i;
		checkpoint_save();
	}

	current_checkpoint = 0;
}
