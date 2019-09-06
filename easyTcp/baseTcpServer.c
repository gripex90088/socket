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
 * 		ָ��ʹ�õ�Э����: AF_INET (IPV4), AF_INET6 (IPV6), AF_LOCAL (Unix��), AF_ROUTE (·���׽���), AF_KEY (��Կ�׽���)
 * type:
 * 		�׽�������: SOCK_STREAM (�ֽ����׽���), SOCK_DGRAM (���ݱ��׽���),SOCK_SEQPACKET (��������׽���), SOCK_RAW (ԭʼ�׽���)
 * protocol:
 *		�׽���Э�飺 ����׽������Ͳ�Ϊԭʼ�׽���, ������Ϊ0, TPPROTO_TCP (TCP����Э��), IPPROTO_UDP (UDP����Э��)�� IPPROTO_SCTP (SCTP����Э��)
 *
 * 2. bind: bind(int sockfd, struct sockaddr *mySockAddr, int mySockAddrLen);
 * sockaddr:
 *		socket���ص��׽���������
 * mySockAddr:
 *		ָ�򱾵�IP��ַ�Ľṹ��ָ��
 * mySockAddrLen:
 * 		�ṹ����
 * struct sockaddr {
 *      unsigned short sa_family; // ͨ��Э��������
 *		char sa_data[14]; 		  // 14�ֽ�Э���ַ,����socket��IP��ַ�Ͷ˿ں�
 * };
 *
 * struct sockaddr_in {
 *		short int sa_family;			//	ͨ��Э��������
 *		unsigned short int sin_port;	// �˿ں�
 *		struct in_addr sin_addr;		// IP��ַ
 *		unsigned char si_zero[8];		// ���0������sockaddr�ṹ�峤����ͬ
 * }
 *
 * 3. int connect(int sockfd, const struct sockaddr *_serverAddr, socklen_t addrlen)
 * sockfd:
 * 		socket���������׽���������
 * _serverAddr:
 * 		������IP��ַ�ṹָ��
 * addrlen:
 *		�ṹ��ָ�볤��
 * 4. int listen(int sockfd, int backlog)
 * sockfd:
 * 		socket������bind���׽���������
 * backlog:
 *		���ÿ����ӿͻ��˵���������������ж���ͻ������������������ʱ���ܵ���ֵӰ�죬Ĭ��20
 *
 * 5.int accept(int sockfd, struct sockaddr *client_addr, socklen_t *addrlen)
 *  sockfd:
 *  	socket��������linsten���׽���������
 * client_addr:
 * 		�ͻ����׽��ֽӿڵ�ַ�ṹ��
 * addrlen:
 * 		�ͻ��˵�ַ�ṹ����
 * 6. int send(int sockfd, const void *msg, int len, int flags)
 * 7. int recv(int sockfd, void * buf, int len, unsighed int flags)
 * sockfd:
 * 		sockfd�������׽���������
 * msg:
 * 		�������ݵ�ָ��
 * buf:
 * 		��Ž������ݵĻ�����
 * len:
 * 		���ݵĳ���,��flags����Ϊ0
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
