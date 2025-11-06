#pragma once

#include <sys/types.h>
#include <stdint.h>

ssize_t my_fdsize(int fd);
int resolve_label(char * label, uintptr_t * p);
int resolve_register_or_label(char * name, uintptr_t * p);
