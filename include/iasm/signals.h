#pragma once

#define __USE_GNU
#include <signal.h>
#include <ucontext.h>
#include <iasm/arch.h>
#if defined(__X86_64)
#define REG_PC REG_RIP
#elif defined(__X86)
#define REG_PC REG_EIP
#endif

extern int sigint;
extern int signaled;

void register_handlers();
