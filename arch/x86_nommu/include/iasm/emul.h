#pragma once

#define __USE_GNU
#include <signal.h>
#include <ucontext.h>

int attempt_emulation(greg_t * pc);
