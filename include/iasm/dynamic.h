#pragma once

typedef struct {
	char * path;
	void * handle;
} dyn_library_t;

int dynamic_load_library(char * path);
void * dynamic_lookup_library(char * path);
void * dynamic_resolve_handle(void * handle, char * symbol);
int dynamic_load_defaults();
void * dynamic_resolve(char * sym);
