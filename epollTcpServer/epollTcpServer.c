/*
 * epollTcpServer.c
 *
 *  Created on: Sep 6, 2019
 *      Author: xuxing
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{

	int fd, con_fd, _sockLen;
	struct sockaddr_in serv_sock;

	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("establish socket failed\n");
		exit(EXIT_FAILURE);
	}
	printf("establish socket success, <socket=%d>\n", serv_sock);

	serv_sock.sin_family = AF_INET;
	serv_sock.sin_port = htons(8081);
	serv_sock.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_sock.sin_zero), 8);
	_sockLen = sizeof(struct sockaddr_in);

	if ( -1 == bind(fd, (struct sockaddr *)&serv_sock, _sockLen) ) {
		printf("bind failed\n");
		exit(EXIT_FAILURE);
	}
	printf("bind success\n");

	if ( -1 == listen(fd, 0)) {
		printf("listen failed\n");
		exit(EXIT_FAILURE);
	}
	printf("listen success\n");

	while(1) {

	}

	close(fd);
	exit(EXIT_SUCCESS);
}
