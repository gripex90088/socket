/*
 * epollTcpServer.c
 *
 *  Created on: Sep 6, 2019
 *      Author: JianRong
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>  // header file include epoll

/****************************************************************************
 * URL: http://www.man7.org/linux/man-pages/man7/epoll.7.html
 * [epoll]
 * 		epoll - I/O event notification facility
 * header file: <sys/epoll.h>
 *
 * [interface]
 * 		epoll_create(2)
 *
 *
 ***************************************************************************/

int main()
{
	int i, rest, _sSock, _cSock, _sockLen, epfd, fd_num, time_out, read_len;

	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	struct epoll_event ep_event;
	struct epoll_event ep_events[10];
	char buf[BUFSIZ];
	char send_buf[24];

	if ((_sSock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("establish socket failed\n");
		exit(EXIT_FAILURE);
	}
	printf("establish socket success, <socket=%d>\n", _sSock);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(20202);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);
	_sockLen = sizeof(struct sockaddr_in);

	if ( -1 == bind(_sSock, (struct sockaddr *)&serv_addr, _sockLen) ) {
		printf("bind failed\n");
		exit(EXIT_FAILURE);
	}
	printf("bind success\n");

	if ( -1 == listen(_sSock, 0)) {
		printf("listen failed\n");
		exit(EXIT_FAILURE);
	}
	printf("listen success\n");

	epfd = epoll_create(2);

	ep_event.events = EPOLLIN;
	ep_event.data.fd = _sSock;
	if ( -1 == epoll_ctl(epfd, EPOLL_CTL_ADD, _sSock, &ep_event) ) {
		printf("epoll create failure\n");
		exit(EXIT_FAILURE);
	}

	fd_num = 0;
	time_out = -1;
	while(1) {
		fd_num = epoll_wait(epfd, ep_events, 10, time_out);

		for (i = 0; i < fd_num; i++) {

			// new client join
			if (_sSock == ep_events[i].data.fd) {

				_cSock = accept(_sSock, (struct sockaddr*)&client_addr, (socklen_t *)&_sockLen);
				if (-1 == _cSock) {
					printf("accept failure\n");
					continue;
				} else {
					ep_event.events = EPOLLIN;
					ep_event.data.fd = _cSock;
					epoll_ctl(epfd, EPOLL_CTL_ADD, _cSock, &ep_event);
				}
				printf("New client join, <socket=%d>\n", _cSock);

				// recv data
			} else if (ep_events[i].events & EPOLLIN) {

				_cSock = ep_events[i].data.fd;
				read_len = read(_cSock, buf, BUFSIZ);
				printf("%d\n", buf[0]);
				if (read_len <= 0) {
					printf("<socket=%d> exit\n", _cSock);
					// client quit
					epoll_ctl(epfd, EPOLL_CTL_DEL, _cSock, NULL);
					close(_cSock);
					ep_events[i].data.fd = -1;
					continue;
				}

				ep_event.data.fd = _cSock;
				ep_event.events = EPOLLOUT;
				epoll_ctl(epfd, EPOLL_CTL_MOD, _cSock, &ep_event);

				// send msg
			} else if (ep_events[i].events & EPOLLOUT) {

				memset(send_buf, 0x00, sizeof(send_buf));
				strcpy(send_buf, "Hello, This is Server");
				rest = send(_cSock, send_buf, sizeof(send_buf), 0);

				ep_event.events = EPOLLIN;
				ep_event.data.fd = _cSock;
				epoll_ctl(epfd, EPOLL_CTL_MOD, _cSock, &ep_event);
			}

		}
	}

	close(epfd);
	close(_sSock);
	exit(EXIT_SUCCESS);
}
