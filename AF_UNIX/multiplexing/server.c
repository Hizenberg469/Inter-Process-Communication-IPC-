#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <unistd.h>


#define SOCKET_NAME "/tmp/DemoSocket" /* Unique name of the socket */
#define BUFFER_SIZE 128				  /* Size of the buffer which will be used by
										for Communication. */

#define MAX_CLIENT_SUPPORTED 32

/*
	An array of File descriptors which the server process 
	is maintaining in order to talk with the connected clients.
	Master skt FD is also a member of this array.
*/
int monitored_fd_set[MAX_CLIENT_SUPPORTED];

/*
	Each connected client's intermediate result is
	maintained in this client array.
*/
int client_result[MAX_CLIENT_SUPPORTED] = { 0 };

/*
	Remove all the FDs, if any, from the array.
	Use for initialization.
*/
static void
initialize_monitor_fd_set() {

	int i = 0;
	for (; i < MAX_CLIENT_SUPPORTED; i++)
		monitored_fd_set[i] = -1;
}

/* Add a new FD to the monitored_fd_set array */
static void
add_to_monitored_fd_set(int skt_fd) {

	int i = 0;
	for (; i < MAX_CLIENT_SUPPORTED; i++) {

		if (monitored_fd_set[i] != -1)
			continue;
		monitored_fd_set[i] = skt_fd;
		break;
	}

}

/* Remove the FD from monitored_fd_set array */
static void
remove_from_monitored_fd_set(int skt_fd) {

	int i = 0;
	for (; i < MAX_CLIENT_SUPPORTED; i++) {
		
		if (monitored_fd_set[i] != skt_fd)
			continue;

		monitored_fd_set[i] = -1;
		break;
	}

}

/* 
	Clone all the FDs in monitored_fd_set array into
	fd_set Data structure.
*/

static void
refresh_fd_set(fd_set* fd_set_ptr) {

	FD_ZERO(fd_set_ptr);
	int i = 0;
	for (; i < MAX_CLIENT_SUPPORTED; i++) {
		if (monitored_fd_set[i] != -1) {
			FD_SET(monitored_fd_set[i], fd_set_ptr);
		}
	}
}

/*
	Get the numerical max value among all the FDs which server
	is monitoring.
*/

static int
get_max_fd() {

	int i = 0;
	int max = -1;

	for (; i < MAX_CLIENT_SUPPORTED; i++) {
		if (monitored_fd_set[i] > max)
			max = monitored_fd_set[i];
	}

	return max;
}

//Done till here....

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
	int data;
	char buffer[BUFFER_SIZE];
	fd_set readfds;
	int comm_socket_fd, i;
	initialize_monitor_fd_set();
	add_to_monitored_fd_set(0);

	/* 
		In case the program exited inadvertently on the last run,
		remove the socket. 
	*/

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

	ret = bind(connection_socket, (const struct sockaddr*)&name,
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

	/*
		Add master socket to Monitored set of FDs.
	*/
	add_to_monitored_fd_set(connection_socket);

	/* This is the main loop for handling connections. */
	/* All Server process usually runs 24 x 7. Good Servers should always up
	   running and should never go down. */


	for (;;) {

		/*
			Copy entire monitored FDs to readfds.
		*/
		refresh_fd_set(&readfds);


		/*Wait for next data packet. */
		/* Server is blocked here. Waiting for the data to arrive from client
			'read' is a blocking system call.  */
		printf("Waiting for data from the client\n");

		/*
			Call the select system call, server process blocks here.
			Linux OS keeps this process blocked until the connection initiation 
			request Or data requests arrives on any of the file descriptors in the 'readfds'
			sets.
		*/

		select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

		if (FD_ISSET(connection_socket, &readfds)) {

			/*
				Data arrives on Master socket only when new client connects with the server
				(that is, 'connect' call is invoked on client side)
			*/

			printf("New connection received recvd, accept the connection\n");

			data_socket = accept(connection_socket, NULL, NULL);

			if (data_socket == -1) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("Connection accepted from client\n");

			add_to_monitored_fd_set(data_socket);
		}
		else if (FD_ISSET(0, &readfds)) {
			memset(buffer, 0, BUFFER_SIZE);
			ret = read(0, buffer, BUFFER_SIZE);
			printf("Input read from console : %s\n", buffer);
		}
		else { /* Data arrives on some other client FD*/

			/*Find the client which has send us the data request*/
			i = 0, comm_socket_fd = -1;
			for (; i < MAX_CLIENT_SUPPORTED; i++) {

				if (FD_ISSET(monitored_fd_set[i], &readfds)) {
					comm_socket_fd = monitored_fd_set[i];

					/* Prepare the buffer to recv the data */
					memset(buffer, 0, BUFFER_SIZE);

					/* Wait for next data packet. */
					/*
						Server is blocked here. Waiting for the data to arrive from
						client 'read' is a blocking system call.
					*/
					printf("Waiting for data from the client\n");
					ret = read(comm_socket_fd, buffer, BUFFER_SIZE);

					if (ret == -1) {
						perror("read");
						exit(EXIT_FAILURE);
					}

					/* Add received summand. */
					memcpy(&data, buffer, sizeof(int));
					if (data == 0) {
						/* Send result */
						memset(buffer, 0, BUFFER_SIZE);

						sprintf(buffer, "Result = %d", client_result[i]);

						printf("Sending final result data to client\n");
						ret = write(comm_socket_fd, buffer, BUFFER_SIZE);
						if (ret == -1) {
							perror("write");
							exit(EXIT_FAILURE);
						}

						/* Close Socket */
						close(comm_socket_fd);
						client_result[i] = 0;
						remove_from_monitored_fd_set(comm_socket_fd);
						continue; /* go to select() and block */
					}
					client_result[i] += data;
				}
			}
		} 
	} /* go to select() and block */

	/*close the master socket*/
	close(connection_socket);
	remove_from_monitored_fd_set(connection_socket);
	printf("connection closed..\n");

	/* Server should release resources before getting terminated.
		Unlink the socket. */

	unlink(SOCKET_NAME);
	exit(EXIT_SUCCESS);
	return 0;
}

