/*
	To implement "ls | wc -l" in C.
	This only work in linux system.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

	int pfds[2];

	pipe(pfds);

	/*
		dup(int fd) : it duplicates the specified fd to the free fd value with lowest
				numeric value.
	*/

	if (!fork()) {

		close(1); /* closing the stdout whose fd value is 1 for a process. */
		dup(pfds[1]); /* setting the stdout of process to writing end of the pipe. */
		close(pfds[0]); /* Reading end of the pipe is not needed by the child process. */
		execlp("ls", "ls", NULL);
	}
	else {

		close(0); /* closing the stdin whose fd value 0 for a process. */
		dup(pfds[0]); /* setting the stdin of process to reading end of the pipe. */
		close(pfds[1]); /* Writing end of the pipe is not needed by the parent process. */
		execlp("wc", "wc", "-l", NULL);

	}


	return 0;
}