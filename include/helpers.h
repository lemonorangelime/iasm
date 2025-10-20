#pragma once

#include <sys/types.h>
#include <stdint.h>

ssize_t fdsize(int fd);
ssize_t fsize(char * filename);
int resolve_label(char * label, uint64_t * p);
int resolve_register_or_label(char * name, uint64_t * p);
