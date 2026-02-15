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
int present_checkpoint = 0;
int past_checkpoint = 0;
int current_checkpoint = 0;

char * get_checkpoint_statement() {
	return checkpoints[current_checkpoint].statement;
}

void checkpoint_save(char * statement) {
        ssize_t size = asm_src_fdsize();
	checkpoint_t * checkpoint = &checkpoints[current_checkpoint];
	memcpy(checkpoint->code_contents, exec_buffer, EXEC_SIZE);
	memcpy(checkpoint->jump_table, jmp_buffer, JMP_SIZE);
	memcpy(checkpoint->stack_contents, stack_buffer, STACK_SIZE);
	if (checkpoint->text_contents != NULL) {
		free(checkpoint->text_contents);
	}
	checkpoint->text_contents = malloc(size);
	checkpoint->code_size = exec_buffer_size;
	checkpoint->text_size = size;
	asm_src_readall(checkpoint->text_contents);
	arch_save_registers(checkpoint->registers_save);

	if (checkpoint->statement) {
		free(checkpoint->statement);
	}
	checkpoint->statement = statement ? strdup(statement) : NULL;
	checkpoint->in_use = 1;
}

void checkpoint_load() {
	checkpoint_t * checkpoint = &checkpoints[current_checkpoint];
	memcpy(exec_buffer, checkpoint->code_contents, EXEC_SIZE);
	memcpy(jmp_buffer, checkpoint->jump_table, JMP_SIZE);
	memcpy(stack_buffer, checkpoint->stack_contents, STACK_SIZE);
	if (checkpoint->text_contents != NULL) {
		asm_src_writeall(checkpoint->text_contents, checkpoint->text_size);
	}
	exec_buffer_size = checkpoint->code_size;
	arch_load_registers(checkpoint->registers_save);
}

int checkpoint_limit(int checkpoint) {
	if (checkpoint < 0) { return 0; }
	if (checkpoint >= checkpoint_count) { return checkpoint % checkpoint_count; }
	return checkpoint;
}

void checkpoint_bound_check() {
	current_checkpoint = checkpoint_limit(current_checkpoint);
	present_checkpoint = checkpoint_limit(present_checkpoint);
	past_checkpoint = checkpoint_limit(past_checkpoint);
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
		checkpoint->statement = NULL;
		checkpoint->in_use = 0;
		checkpoint->setup = 1;
	}
}

void checkpoint_advance(char * statement) {
	if (current_checkpoint != present_checkpoint) {
		present_checkpoint = current_checkpoint;
	}
	if (current_checkpoint == present_checkpoint) {
		present_checkpoint++;
	}
	current_checkpoint++;
	checkpoint_bound_check();
	if (checkpoints[current_checkpoint].in_use) {
		past_checkpoint++; // rollover
	}
	checkpoint_save(statement); // init new checkpoint to continue
}

int checkpoint_wind() {
	if (present_checkpoint == current_checkpoint) {
		return 0;
	}
	current_checkpoint++;
	checkpoint_bound_check();
	checkpoint_load(); // load new state
	return 1;
}

int checkpoint_rewind() {
	if (current_checkpoint == past_checkpoint) {
		return 0;
	}
	current_checkpoint--;
	checkpoint_bound_check();
	checkpoint_load(); // load old state
	return 1;
}

void checkpoint_init() {
	checkpoints = malloc(sizeof(checkpoint_t) * checkpoint_count);
	memset(checkpoints, 0, sizeof(checkpoint_t) * checkpoint_count);
	checkpoint_update();

	for (int i = 0; i < checkpoint_count; i++) {
		current_checkpoint = i;
		checkpoint_save(NULL);
		checkpoints[current_checkpoint].in_use = 0;
	}

	current_checkpoint = 0;
}
