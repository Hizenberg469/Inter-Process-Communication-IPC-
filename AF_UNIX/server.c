﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


#define SOCKET_NAME "/tmp/DemoSocket" /* Unique name of the socket */
#define BUFFER_SIZE 128				  /* Size of the buffer which will be used by
										for Communication. */

int
main(int argc, char* argv[]) {

	/* Structure provided by Unix library for using
		it as an identifier for managing the socket */

	struct sockaddr_un name;

#if 0
	
	struct sockaddr_un {

		/* This member describe what type of data it
			will handle. Define the family of the 
			socket. Here, we are using UNIX DOMAIN socket
			so AF_UNIX constant should be used for
			classification. */
		sa_family_t sun_family;			/* AF_UNIX */

		/* Hold the name of the socket with the path where it
			is created. */
		char sun_path[108];				/* pathname */
	};
#endif

	int ret;
	int connection_socket;
	int data_socket;
	int result;
	int data;
	char buffer[BUFFER_SIZE];

	/* In case the program exited inadvertently on the last run, 
		remove the socket. */


	unlink(SOCKET_NAME);

	/* Create Master socket */

	/* SOCK_DGRAM for Datagram based communication */
	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if (connection_socket == -1) {

		perror("socket");
		exit(EXIT_FAILURE);

	}

	printf("Master socket created\n");

	/* initialize */
	memset(&name, 0, sizeof(struct sockaddr_un));

	/* Specify the socket Cridentials */
	name.sun_family = AF_UNIX;
	strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

	/* Bind Socket to socket name. */
	/* Purpose of bind() system call is that application() dictate the underlying
	   operating system the criteria of receiving the data. Here, bind() system call
	   is telling the OS that if sender process sends the data destined to socket 
	   "tmp/DemoSocket", then such data needs to be delievered to this server process
	   ( the server process ) */

	ret = bind(connection_socket, (const struct sockaddr* )&name,
		sizeof(struct sockaddr_un));

	if (ret == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}


	printf("bind() call succeed\n");

	/* Prepare for accepting connections. The backlog size is set to 20.
	   So, while one request is being processed other requests can be 
	   waiting.
	   */

	ret = listen(connection_socket, 20);

	if (ret == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	/* This is the main loop for handling connections. */
	/* All Server process usually runs 24 x 7. Good Servers should always up 
	   running and should never go down. */

	for (;;) {

		/* Wait for incoming connection. */

		printf("Waiting on accept() sys call\n");

		data_socket = accept(connection_socket, NULL, NULL);

		if (data_socket == -1) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		printf("Connection accepted from client\n");

		result = 0;
		for (;;) {

			/* Prepare the buffer to recv the data*/
			memset(buffer, 0, BUFFER_SIZE);

			/*Wait for next data packet. */
			/* Server is blocked here. Waiting for the data to arrive from client
			   'read' is a blocking system call.  */
			printf("Waiting for data from the client\n");

			ret = read(data_socket, buffer, BUFFER_SIZE);

			if (ret == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}

			/* Add received summand */

			memcpy(&data, buffer, sizeof(int));
			if (data == 0) break;
			result += data;

		}

		/* Send result. */
		memset(buffer, 0, BUFFER_SIZE);
		sprintf(buffer, "Result = %d", result);


		printf("Sending final result back to client\n");

		/* non-blockding system call */
		ret = write(data_socket, buffer, BUFFER_SIZE);
		if (ret == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		/*close socket*/
		close(data_socket);


	}
	
	/*close the master socket*/
	close(connection_socket);
	printf("connection closed..\n");

	/* Server should release resources before getting terminated.
		Unlink the socket. */

	unlink(SOCKET_NAME);
	exit(EXIT_SUCCESS);
	return 0;
}

