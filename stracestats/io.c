/*
 * Copyright (c) 2013, John A. Brunelle
 * All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

//tcp port to which to bind
#define PORT 5678

//size of each i/o operation
#define MSG_SIZE 512

//number of read/send/recv/write passes to make
#define LOOP_COUNT 10000

int main(int argc, char **argv) {
	//--- declarations

	int fd_sock_lstn;  //listening socket
	int fd_sock_srvr;  //socket client uses, connected to the server
	int fd_sock_clnt;  //socket server uses, connected to the client
	
	//address structs for the above, respectively
	struct sockaddr_in addr_lstn, addr_srvr, addr_clnt;
	
	//file descriptors for /dev/zero and /dev/null
	int fd_zero, fd_null, fd_urnd;
	
	//data buffer
	char *msg = malloc(MSG_SIZE);
	
	//misc
	socklen_t addr_clnt_size;
	int i;

	
	//--- setup

	//create server socket; bind and listen
	fd_sock_lstn = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock_lstn<0) {
		perror("unable to create socket for listening");
		exit(EXIT_FAILURE);
	}
	addr_lstn.sin_family = AF_INET;
	addr_lstn.sin_addr.s_addr = INADDR_ANY;
	addr_lstn.sin_port = htons(PORT);
	if (bind(fd_sock_lstn, (struct sockaddr *)&addr_lstn, sizeof(addr_lstn))) {
		perror("unable to bind socket");
		exit(EXIT_FAILURE);
	}
	if (listen(fd_sock_lstn, 0)) {
		perror("unable to listen");
		exit(EXIT_FAILURE);
	}

	//create client socket; connect to the server
	fd_sock_srvr = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock_lstn<0) {
		perror("unable to create socket for connecting to server");
		exit(EXIT_FAILURE);
	}
	addr_srvr.sin_family = AF_INET;
	addr_srvr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr_srvr.sin_port = htons(PORT);
	if (connect(fd_sock_srvr, (struct sockaddr *)&addr_srvr, sizeof(addr_srvr))) {
		puts("unable to connect to server");
		exit(1);
	}

	//accept the client connection
	addr_clnt_size = sizeof(struct sockaddr_in);
	fd_sock_clnt = accept(fd_sock_lstn, (struct sockaddr *)&addr_clnt, &addr_clnt_size);
	if (fd_sock_clnt<0) {
		perror("unable to accept connection");
		exit(EXIT_FAILURE);
	}

	//open source and sink files
	fd_zero = open("/dev/zero", O_RDONLY);
	fd_urnd = open("/dev/urandom", O_RDONLY);
	fd_null = open("/dev/null", O_WRONLY);

	
	//--- main loop
	
	for (i=0; i<LOOP_COUNT; i++) {
		if (i%2==0)
			read(fd_zero, msg, MSG_SIZE);
		else
			read(fd_urnd, msg, MSG_SIZE);
		send(fd_sock_srvr, msg, MSG_SIZE, 0);
		recv(fd_sock_clnt, msg, MSG_SIZE, 0);
		write(fd_null, msg, MSG_SIZE);
	}
}
