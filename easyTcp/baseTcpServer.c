/*
 * baseTcp.c
 *
 *  Created on: Aug 25, 2019
 *      Author: JianRong
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>		// socket API
#include <netinet/in.h> 	// INADDR_ANY
#include <unistd.h>

/*******************************************************************************************************************
 * 1. int socket(int family, int type, int protocol);
 * family:
 * 		指定使用的协议蔟: AF_INET (IPV4), AF_INET6 (IPV6), AF_LOCAL (Unix域), AF_ROUTE (路由套接字), AF_KEY (密钥套接字)
 * type:
 * 		套接字类型: SOCK_STREAM (字节流套接字), SOCK_DGRAM (数据报套接字),SOCK_SEQPACKET (有序分组套接字), SOCK_RAW (原始套接字)
 * protocol:
 *		套接字协议： 如果套接字类型不为原始套接字, 参数可为0, TPPROTO_TCP (TCP传输协议), IPPROTO_UDP (UDP传输协议)， IPPROTO_SCTP (SCTP传输协议)
 *
 * 2. bind: bind(int sockfd, struct sockaddr *mySockAddr, int mySockAddrLen);
 * sockaddr:
 *		socket返回的套接字描述符
 * mySockAddr:
 *		指向本地IP地址的结构体指针
 * mySockAddrLen:
 * 		结构长度
 * struct sockaddr {
 *      unsigned short sa_family; // 通信协议类型族
 *		char sa_data[14]; 		  // 14字节协议地址,包含socket的IP地址和端口号
 * };
 *
 * struct sockaddr_in {
 *		short int sa_family;			//	通信协议类型族
 *		unsigned short int sin_port;	// 端口号
 *		struct in_addr sin_addr;		// IP地址
 *		unsigned char si_zero[8];		// 填充0保持以sockaddr结构体长度相同
 * }
 *
 * 3. int connect(int sockfd, const struct sockaddr *_serverAddr, socklen_t addrlen)
 * sockfd:
 * 		socket函数返回套接字描述符
 * _serverAddr:
 * 		服务器IP地址结构指针
 * addrlen:
 *		结构体指针长度
 * 4. int listen(int sockfd, int backlog)
 * sockfd:
 * 		socket函数绑定bind后套接字描述符
 * backlog:
 *		设置可连接客户端的最大连接数，当有多个客户端向服务器发送请求时，受到此值影响，默认20
 *
 * 5.int accept(int sockfd, struct sockaddr *client_addr, socklen_t *addrlen)
 *  sockfd:
 *  	socket函数经过linsten后套接字描述符
 * client_addr:
 * 		客户端套接字接口地址结构体
 * addrlen:
 * 		客户端地址结构长度
 * 6. int send(int sockfd, const void *msg, int len, int flags)
 * 7. int recv(int sockfd, void * buf, int len, unsighed int flags)
 * sockfd:
 * 		sockfd函数的套接字描述符
 * msg:
 * 		发送数据的指针
 * buf:
 * 		存放接收数据的缓冲区
 * len:
 * 		数据的长度,把flags设置为0
 *******************************************************************************************************************/
int main()
{
	int fd, _sockLen, new_fd, numbytes;
	struct sockaddr_in serv_sock;
	struct sockaddr_in client_sock;
	char buff[BUFSIZ];

	serv_sock.sin_family = AF_INET;
	serv_sock.sin_port = htons(8080);
	serv_sock.sin_addr.s_addr = INADDR_ANY; // htonl("IP ADDRESS");

	_sockLen = sizeof(struct sockaddr_in);

	// establish socket
	fd = socket(PF_INET, SOCK_STREAM, 0);

	// bind
	if(bind(fd, (struct sockaddr *)&serv_sock, _sockLen) == -1) {
		printf("Bind address failed\n");
		exit(EXIT_FAILURE);
	}
	printf("Bind Success\n");

	// listen
	if (listen(fd, 10) == -1) {
		printf("Listen failed\n");
		exit(EXIT_FAILURE);
	}
	printf("Listening......\n");
	printf("Ready for Accept, waitting...\n");

		while (1) {
			// accept
			new_fd = accept(fd, (struct sockaddr *)&client_sock, &_sockLen);
			if (new_fd == -1) {
				printf("accept failed\n");
				continue;
			}
			printf("Get the Client\n");

			if (fork() == 0) {
				// recv
				while(1) {
					numbytes = recv(new_fd, buff, BUFSIZ, 0);
					if (numbytes < 0) {
						close(new_fd);
						break;
					}

					send(new_fd, "Welcome to my server\n", 21, 0);
				}
				close(new_fd);
				exit(EXIT_SUCCESS);
			}
	}

	// close
	close(fd);
	return 0;
}
