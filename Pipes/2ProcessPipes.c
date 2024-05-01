#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

	int pfds[2];
	char buf[30];

	pipe(pfds);


	/*
		fork() : 
			-> Generate the clone of the calling process.
			-> Calling process is parent process and new
				process is child process.
			-> Return value :
				1. value 0 : means the current process is child process
							which is newly forked and forking is successful.
				2. value -1 : means the forking the process has failed.
								The value is retured to parent process.
				3. value <Some integer value> : means the current process is 
								parent process and it is the process id of 
								the newly forked child process on the system.
	*/
	if (!fork()) {

		printf(" CHILD : writing to the pipe\n");
		write(pfds[1], "test", 5);
		printf(" CHILD: exiting\n");
		exit(0);
	}
	else {
		printf("PARENT: reading from pipe\n");
		read(pfds[0], buf, 5);
		printf("PARENT: read \"%s\"\n", buf);
		wait(NULL);
	}

	/*
		The parent process should wait for forked child
		process to finish the execution and notify the parent.
		Otherwise, the parent would also terminating without 
		knowing the process execution info about the child
		process and the child process would keep running turning
		itself to a zombie process.
		Here, we use wait() sys call for parent to wait for 
		child process.
	*/
	return 0;
}