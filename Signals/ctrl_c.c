/*
	Consider a program below, 
	There is some special thing attached to SIGABRT signal.

	You can see, in below code, line no 23 registers a handler routine with signal SIGABRT.
	when abort() is called, it delivers the signal SIGABRT to this process, and  abort_signal_handler() will be invoked.
	Note that, it is mandatory to terminate the process by calling exit(0) from abort_signal_handler(). Handler routine
	for SIGABRT signal must not return to caller. It it returns, Kernel will kill the process instead. You can perform experiment
	by removing the line 16 in below code, and you will notice that process is terminated after execution of abort_signal_handler().
	Hence, Process either commit suicide or it will be killed by the OS. SIGABRT signal cannot be blocked(= ignore) by the process.

*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void
ctrlC_signal_handler(int sig) {

	printf("Ctrl-C pressed\n");
	printf("Bye Bye\n");
	exit(0);

}

static void
abort_signal_handler(int sig) {

	printf("process is aborted\n");
	printf("Bye Bye\n");
	exit(0);

}

int
main(int argc, char* argv[]) {

	signal(SIGINT, ctrlC_signal_handler);
	signal(SIGABRT, abort_signal_handler);

	char ch;
	printf("About process (y/n) ?\n");
	scanf("%c", &ch);
	if (ch == 'y')
		abort();


	return 0;
}