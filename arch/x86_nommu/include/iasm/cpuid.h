#pragma once

#include <stdint.h>

void cpuid(int eax_in, int ebx_in, int ecx_in, int edx_in, uint32_t * eax, uint32_t * ebx, uint32_t * ecx, uint32_t * edx);
