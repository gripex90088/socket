#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	// 连接win32库
	#pragma comment(lib, "ws2_32.lib")

#else
	#include <unistd.h> //uni std
	#include <arpa/inet.h>
	#include <string.h>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET) (~0)
	#define SOCKET_ERROR (~1)
#endif

#include <iostream>
#include <thread>
#include <vector>

//#pragma warning(disable:4996)

using std::cout;
using std::endl;

std::vector<SOCKET> g_clients;

/*
* 网络报文的数据格式的定义及使用
*/
enum CMD
{
	CMD_LOGIN, // 登录
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT, // 退出
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR // 错误信息
};

// 消息头
struct Header
{
	short dataLength; // 数据长度
	short cmd; // 命令
};

// 保证 客户端服务端字节顺序相同
// 需要网络字节序一致
// 内存对齐 前后端保持占用内存一致
struct Login : public Header
{
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char username[32];
	char password[32];
};

// 登录结果
struct LoginResult : public Header
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};

// 退出
struct LoginOut : public Header
{
	LoginOut()
	{
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LoginOutResult : public Header
{
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin : public Header
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

int processor(SOCKET _cSock)
{
	/*
	* 网络报文的数据格式定义及使用
	*/
	char szRecv[1024];
	// 5 recv
	int nLen = (int)recv(_cSock, szRecv, sizeof(Header), 0);
	Header* header = (Header*)& szRecv;
	if (nLen <= 0)
	{
		cout << "client <SOCKET = "
			<< (int)_cSock
			<< "> exited....."
			<< endl;
		return -1;
	}

	cout << "receiving client"
		<< ", <SOCKET = "
		<< (int)_cSock
		<< ">  data , <DataLength = "
		<< header->dataLength
		<< ">"
		<< endl;

	// 6 处理请求
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		// 上面接收头部, 造成内存偏移
		recv(_cSock, szRecv + sizeof(Header), header->dataLength - sizeof(Header), 0);

		Login* login = (Login*)szRecv;
		cout << "user : "
			<< login->username
			<< " \t password : "
			<< login->password
			<< "\t 登录成功.....\n"
			<< "datalength = "
			<< login->dataLength
			<< endl;

		// 忽略判断过程
		LoginResult loginResult;
		send(_cSock, (char*)& loginResult, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{
		recv(_cSock, szRecv + sizeof(Header), header->dataLength - sizeof(Header), 0);
		LoginOut* loginOut = (LoginOut*)szRecv;

		LoginOutResult logoutRet;
		send(_cSock, (char*)& logoutRet, sizeof(logoutRet), 0);
		cout << "user: " << loginOut->userName << "  退出....\n";
	}
	break;
	default:
	{
		header->cmd = CMD_NEW_USER_JOIN;
		header->dataLength = 0;
		send(_cSock, (char*)& header, sizeof(header), 0);
	}
	break;
	}
	return 0;
}

int main()
{
#ifdef _WIN32
	// 启动win socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);
#endif
	//--  用Socket API建立TCP服务器
	// 1. 建立一个socket套接字
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		cout << "establish socket error.....\n";
	}
	else
	{
		cout << "establish stocket success.....\n";
	}
	// 2. bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(5000);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("");
#else
	_sin.sin_addr.s_addr = INADDR_ANY;// inet_addr("");
#endif
	int ret = bind(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		cout << "bind error......\n";
	}
	else
	{
		cout << "bind success.......\n";
	}

	// 3 listen
	listen(_sock, 10);

	// char recv_buf[120];
	while (true)
	{
		// 伯克利 socket 最大描述符＋1
		fd_set fdRead; // 描述符 (socket)集合
		fd_set fdWrite;
		fd_set fdExp;

		// 清空集合里的数据
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// 将描述符加入集合
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		// 最大描述符
		SOCKET maxSock = _sock;
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fdRead);
			if (maxSock < g_clients[n])
			{
				maxSock = g_clients[n];
			}
		}

		/* nfds : 整数值
		*   是指fd_set集合中所有socket(描述符)的范围,而不是数量
		*   既是所有文件描述符最大值 + 1, 在windows中这个参数可以为0
		*
		* timeval : 将服务端select模型设置为非阻塞
		*
		*/
		timeval t = { 0,0 };
		//int setRes = select(_sock + 1, &fdRead, &fdWrite, &fdExp, nullptr);
		int setRes = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (setRes < 0)
		{
			cout << "select end" << endl;
			break;
		}

		// 判断描述符是否在集合中
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);

			// 4 accept
			sockaddr_in _clientaddr = {};
			int _clientaddrLen = sizeof(_clientaddr);
			SOCKET _cSock = INVALID_SOCKET;

#ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&_clientaddr, &_clientaddrLen);
#else
			_cSock = accept(_sock, (sockaddr*)&_clientaddr, (socklen_t *)&_clientaddrLen);
#endif

			if (INVALID_SOCKET == _cSock)
			{
				cout << "accept client error......\n";
			}
			else
			{
				// 推送
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin newUser;
					newUser.sock = _cSock;
					send(g_clients[n], (const char*)& newUser, sizeof(NewUserJoin), 0);
				}

				// 新客户端加入
				g_clients.push_back(_cSock);
				cout << "accept client success "
					<< ",<SOCKET = "
					<< (int)_cSock
					<< "> ,< IP = "
					<< inet_ntoa(_clientaddr.sin_addr)
					<< ">"
					<< endl;
			}
		}

		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(g_clients[n], &fdRead))
			{
				if (-1 == processor(g_clients[n]))
				{
					//auto iter = g_clients.begin()+n; // C++ 11
					std::vector<SOCKET>::iterator iter = g_clients.begin()+n;
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}
				}
			}
		}

		//cout << "当前空闲,可处理其它业务" << endl;// 将timeval设置
	}

#ifdef _WIN32
	// close client socket
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}

	// 7 closesocket
	closesocket(_sock);

	// WSACleanup
	WSACleanup();
#else
	// close client socket
	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	//for (int n = (int)g_clients.size() - 1; n < 0; n--)
	{
		close(g_clients[n]);
	}

	// 7 closesocket
	close(_sock);
#endif

	system("pause");
	return 0;
}
