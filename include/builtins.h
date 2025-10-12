#pragma once

typedef struct {
	char * name;
	char * message;
} help_topic_t;

int execute_builtins(char * line);
