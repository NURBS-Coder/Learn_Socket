#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//里面有WinSock1.h，不能放前面,否则要定义 WIN32_LEAN_AND_MEAN 宏
	#pragma comment(lib,"ws2_32.lib")	//添加动态库
#else

#endif // _WIN32

#include <stdio.h>				//printf()....

using namespace std;
#include <iostream>				//std::cout....
#include <vector>

#include "MessageHeader.hpp"

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024*40		//接收缓冲区最小单元大小 40kb
#endif
				

class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf,0,RECV_BUFF_SIZE * 10);
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}

	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

private:
	SOCKET _sockfd;							//客户端socket
	char _szMsgBuf[RECV_BUFF_SIZE * 10];	//第二缓冲区、消息缓冲区
	int _lastPos;							//记录第二缓冲区中数据位置

	
};

//new -> 堆内存    int等定义 -> 栈内存（小，只有几M）
class EasyTcpServer
{
private:
	SOCKET _sock;						//服务器socket	
	vector<ClientSocket*> _clients;		//动态数组：存客户端socket,用指针，在堆内存，防止栈内存不够
	char _szRecv[RECV_BUFF_SIZE];		//接收缓冲区

public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		memset(_szRecv,0,RECV_BUFF_SIZE);
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//初始化Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动WinSock2.x网络环境
		WORD ver = MAKEWORD(2,2);	//创建版本号
		WSADATA dat;
		WSAStartup(ver, &dat);		//加载socket相关动态链接库
#endif
		//---------------------
		//-- 用Socket API建立简易TCP服务器
		if (_sock != INVALID_SOCKET)
		{
			printf("关闭socket = <%d>旧连接...\n", _sock);
			Close();	//??WSACleanup();??
		}
		// 1.建立一个socket(IPV4的基于字节流的TCP协议的套接字)
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(SOCKET_ERROR == _sock)
		{
			printf("错误,建立socket失败...\n");
		}
		printf("建立socket = <%d>成功...\n", _sock);
		return _sock;
	}

	//绑定IP和端口号
	int Bind(char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2.bind 绑定用于接收客户端连接的网络端口
		sockaddr_in _sin = {};		//这个类型比bind中需要的sockaddr好填
		_sin.sin_family = AF_INET;	//IPV4
		_sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else

#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if(SOCKET_ERROR == ret)
		{
			printf("错误,socket = <%d>绑定网络端口<%d>失败...\n",_sock,port);
		}
		else
		{
			printf("socket = <%d>绑定网络端口<%d>成功...\n" ,_sock, port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		if (INVALID_SOCKET == _sock)
		{
			return -1;
		}
		// 3.listen 监听网络端口(等待监听数：5)
		int ret = listen(_sock, n);
		if(SOCKET_ERROR == ret)
		{
			printf("socket = <%d>错误,监听网络端口失败...\n",_sock);
		}
		else
		{
			printf("socket = <%d>监听网络端口成功...\n",_sock);
		}
		return ret;
	}

	//接收客户端连接
	SOCKET Accept()
	{
		// 4.accept 等待接受客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddrLen);
#else

#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("错误,socket = <%d>接收到无效客户端Socket...\n", _sock);
		}
		else
		{
			//新客户端加入，群发他的socket
			NewUserJoin newUserJoin;
			newUserJoin.sock = cSock;
			//SendData2All(&newUserJoin);
			//新客户端加入vector
			_clients.push_back(new ClientSocket(cSock));
			printf("socket = <%d>有新客户端加入：socket= %d, IP= %s \n",_sock, cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	//关闭Socekt
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8.关闭套接字 closesocket
			for (int n = (int)_clients.size() - 1 ; n >= 0; n--)		//循环只调用一次_clients.size()，快
			{															//但size_t是无符号数，不能--，size_t的值为0的时候，再继续自减就会大于0
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			//---------------------
			//清除Windows socket环境
			WSACleanup();
#else

#endif
			_sock = INVALID_SOCKET;
			_clients.clear();
		}
	}

	//发送数据(指定用户)
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//发送数据（所有用户）
	void SendData2All(DataHeader* header)
	{
		for (size_t i = 0; i < _clients.size(); i++)	//每次循环都要算一遍_clients.size()用于判断，慢
		{
			//发送包体（包头+包体）
			SendData(_clients[i]->sockfd(), header);
		}
	}

	//接收数据 (处理粘包、拆包等)
	int RecvData(ClientSocket* pClient)
	{

		//接收包头
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		
		if (nLen <= 0)
		{
			printf("客户端<socket = %d>已退出，任务结束。\n",pClient->sockfd());
			return -1;
		}

		//将收取的数据拷贝到消息缓冲区
		memcpy(pClient->msgBuf() + pClient->getLastPos(),_szRecv, nLen);
		//消息缓冲区数据尾部位置后移
		pClient->setLastPos( pClient->getLastPos() + nLen);

		//判断数据长度大于数据头长度
		while (pClient->getLastPos() >= sizeof(DataHeader))			//循环处理黏包，一次收到大量数据，循环分割全部处理
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();   
			//判断消息缓冲区大小大于消息长度
			if (pClient->getLastPos() >= header->dataLength)			//判断处理少包，数据不够就等待下次数据接收
			{
				//消息缓冲区剩余数据长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient->sockfd(),header);
				//将消息缓冲区未处理的数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区数据位置前移
				pClient->setLastPos(nSize);
			}
			else
			{
				//消息缓冲区数据不够一条消息
				break;
			}			
		}
		return 0;
	}

	//响应网络消息
	void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		// 6.switch 处理请求 
		// 7.send 向客户端返回请求数据
		switch (header->cmd)
		{
		case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				printf("socket = <%d>收到客户端<socket = %d>请求：CMD_LOGIN\t数据长度：%d\n",_sock, cSock,login->dataLength);
				//忽略判断用户名和密码
				//printf(" --> 登陆：UserName=%s,PassWord=%s\n",login->userName,login->passWord);
				//回复客户端登陆结果
				LoginResult ret;
				//发送数据包（包头+包体）
				//SendData(cSock, &ret);
			}
			break;
		case CMD_LOGOUT:
			{
				Logout *logout = (Logout*)header;
				printf("socket = <%d>收到客户端<socket = %d>请求：CMD_LOGOUT\t数据长度：%d\n",_sock, cSock,logout->dataLength);
				//忽略判断用户名
				printf(" --> 登出：UserName=%s\n",logout->userName);
				//回复客户端登出结果
				LogoutResult ret;
				//发送包体（包头+包体）
				SendData(cSock, &ret);
			}
			break;
		default:
			{
				//发送包头
				//DataHeader ret;
				//SendData(cSock, &ret);
				printf("socket = <%d>收到未定义消息\t数据长度：%d", _sock, header->dataLength);
			}
			break;
		}
	}

	//处理网络消息
	bool OnRun()
	{
		if (!isRun())
		{
			return false;
		}
		//nfds：一个整数值，是指fd_set集合中所有描述符（Socket）的范围，而不是数量，是所有描述符（Socket）最大值加一；
		//nfds：Windows无意义,可以置零，在Linux中有用，为了兼容伯克利socket
		//readfds：可读fd的集合
		//writefds：可写fd的集合
		//exceptfds：异常fd的集合
		//timeout：查询延迟，置空就是阻塞模式

		//fd_set就是一个Socket集合
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExcept;

		//初始化fd_set（清零）
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		//socket 加入集合
		FD_SET(_sock,&fdRead);
		FD_SET(_sock,&fdWrite);
		FD_SET(_sock,&fdExcept);

		for (size_t i = 0; i < _clients.size(); i++)	//每次循环都要算一遍_clients.size()用于判断，慢
		{
			//将客户端socket加入可读集合
			FD_SET(_clients[i]->sockfd(), &fdRead);
		}

		//等待集合中有可操作Socket，非阻塞模式,timeval没有结果就返回，继续执行
		//使得单线程也可以在无客户端消息处理时，继续进行其他任务
		timeval t = {1,0};	
		int ret = select(0,&fdRead,&fdWrite,&fdExcept, &t);
		if (ret < 0)
		{
			printf("select任务结束。\n");
			Close();
			return false;
		}
		
		//查询可读集合中有没有监听Socket
		if (FD_ISSET(_sock,&fdRead))	
		{
			FD_CLR(_sock,&fdRead);		//将该Socket从集合中清除
			Accept();
		}

		//处理有可读消息的客户端socket
		for (int n = (int)_clients.size() -1; n >=0; n--)
		{
			if (FD_ISSET(_clients[n]->sockfd(),&fdRead))
			{		
				//处理客户端请求
				if(-1 == RecvData(_clients[n]))
				{
					auto iter = _clients.begin() + n;
					if (iter != _clients.end())
					{
						//客户端退出，删除Socket
						delete _clients[n];
						_clients.erase(iter);
					}
				}
			}
		}

		//空闲时间处理其他业务
		//printf("空闲时间处理其他业务。。。\n");

		return true;
	}

	//是否工作中
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

private:

};

#endif  //!_EasyTcpServer_hpp_