#pragma once

#include <vm.h>

extern int vmmode;
extern vm_t * vm;

int vmmode_init();
uint64_t vmmode_resume();
