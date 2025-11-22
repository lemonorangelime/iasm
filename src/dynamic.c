#include <iasm/dynamic.h>
#include <limits.h>
#include <iasm/linked.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

linked_t * libraries = NULL;

int dynamic_load_library(char * path) {
	char * pathbuf = malloc(PATH_MAX);
	pathbuf[0] = 0;
	if (path) {
		path = realpath(path, pathbuf);
		if (!path) {
			free(pathbuf);
			return 1;
		}
	}

	dyn_library_t * dynlib = malloc(sizeof(dyn_library_t));
	dynlib->path = path;
	dynlib->handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
	if (!dynlib->handle) {
		free(pathbuf);
		free(dynlib);
		return 1;
	}
	libraries = linked_append(libraries, dynlib);
	return 0;
}

void * dynamic_lookup_library(char * path) {
	char * pathbuf = malloc(PATH_MAX);
	pathbuf[0] = 0;
	path = realpath(path, pathbuf);
	if (!path) {
		free(pathbuf);
		return NULL;
	}

	linked_t * node = libraries;
	while (node) {
		dyn_library_t * dynlib = node->p;
		if (!dynlib->path) {
			node = node->next;
			continue;
		}
		if (strcmp(dynlib->path, path) == 0) {
			return dynlib->handle;
		}
		node = node->next;
	}
	free(pathbuf);
	return NULL;
}

void * dynamic_resolve_handle(void * handle, char * symbol) {
	return dlsym(handle, symbol);
}

int dynamic_load_defaults() {
	return dynamic_load_library(NULL);
}

void * dynamic_resolve(char * sym) {
	linked_t * node = libraries;
	while (node) {
		dyn_library_t * dynlib = node->p;
		void * p = dlsym(dynlib->handle, sym);
		if (p) { // this is WRONG
			return p;
		}
		node = node->next;
	}
	return NULL;
}
