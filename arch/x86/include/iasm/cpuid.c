#pragma once

#include <stdint.h>

void cpuid(int parameter, uint32_t * eax, uint32_t * ebx, uint32_t * ecx, uint32_t * edx);
