#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm.h>
#define __USE_GNU
#include <signal.h>
#include <ucontext.h>

char * messages[] = {
	"",
	"",
	"",
	"Illegal instruction",
	"Breakpoint for debugging",
	"Abnormal termination",
	"Bus error",
	"Floating-point exception",
	"",
	"",
	"Segmentation Fault",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"Equivalent to SIGSYS"
};

void signal_handler(int signum, siginfo_t * info, ucontext_t * context) {
	signal(signum, (void *) signal_handler);

	char * message = messages[signum - 1];
	printf(*message ? "%s\n" : "", message);
	if (context_switching && *message) {
		context->uc_mcontext.gregs[REG_RIP] = (uint64_t) reload_state; // return to reload_state
		return;
	} else {
		printf("fatal internal error (sig %d)\n", signum);
		exit(-1);
	}
}

void register_handlers() {
	signal(SIGILL, (void *) signal_handler);
	signal(SIGSEGV, (void *) signal_handler);
	signal(SIGBUS, (void *) signal_handler);
	signal(SIGFPE, (void *) signal_handler);
	signal(SIGTRAP, (void *) signal_handler);
	signal(SIGSYS, (void *) signal_handler);
}
