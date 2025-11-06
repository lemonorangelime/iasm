#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm.h>
#include <arch.h>
#define __USE_GNU
#include <signal.h>
#include <ucontext.h>

char * messages[] = {
	NULL,
	"Control-C from Keyboard",
	NULL,
	"Illegal instruction",
	"Breakpoint for debugging",
	"Abnormal termination",
	"Bus error",
	"Floating-point exception",
	NULL,
	NULL,
	"Segmentation Fault",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Equivalent to SIGSYS"
};

int sigint = 0;
int signaled = 0;

#if defined(__X86_64)
#define REG_PC REG_RIP
#elif defined(__X86)
#define REG_PC REG_EIP
#endif

void safe_puts(char * string) {
	if (!string) {
		return;
	}
	puts(string);
}

void signal_handler(int signum, siginfo_t * info, ucontext_t * context) {
	signal(signum, (void *) signal_handler);

	signaled = 1;
	sigint = signum == SIGINT;
	if (sigint) {
		putchar('\n');
	}

	char * message = messages[signum - 1];
	safe_puts(message);
	if (context_switching && message) {
		context->uc_mcontext.gregs[REG_PC] = (uintptr_t) reload_state; // return to reload_state
		return;
	} else if (temp_context_switching && message) {
		context->uc_mcontext.gregs[REG_PC] = (uintptr_t) temp_reload_state; // return to temp_reload_state
		return;
	} else if (!sigint) {
		printf("fatal internal error (sig %d)\n", signum);
		exit(-1);
	}
	write(1, "> ", 2);
}

// signal() implies SA_SIGINFO on x86-64 but not 32, so switch to sigaction
void contextual_signal(int n, void * p) {
	struct sigaction a = {};
	a.sa_flags = SA_SIGINFO;
	a.sa_sigaction = p;
	sigaction(n, &a, NULL);
}

void register_handlers() {
	contextual_signal(SIGILL, (void *) signal_handler);
	contextual_signal(SIGSEGV, (void *) signal_handler);
	contextual_signal(SIGBUS, (void *) signal_handler);
	contextual_signal(SIGFPE, (void *) signal_handler);
	contextual_signal(SIGTRAP, (void *) signal_handler);
	contextual_signal(SIGSYS, (void *) signal_handler);
	contextual_signal(SIGINT, (void *) signal_handler);
}
