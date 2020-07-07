#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

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
#include <atomic>				//原子操作库

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024*10		//接收缓冲区最小单元大小 40kb
#endif

class EasyTcpClient
{
private:
	SOCKET _sock;							//服务器socket
	char _szRecv[RECV_BUFF_SIZE];			//接收缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 5];	//第二缓冲区、消息缓冲区
	int _lastPos;							//记录第二缓冲区中数据位置
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
		memset(_szRecv,0,RECV_BUFF_SIZE);
		memset(_szMsgBuf,0,RECV_BUFF_SIZE * 5);
		_lastPos = 0;
	}

	//虚析构函数
	virtual ~EasyTcpClient()	//防止内存泄露,指针定义对象，可以释放到子类
	{
		Close();
	}

public:
	//初始化socket
	int InitSocket()
	{
#ifdef _WIN32
		//启动WinSock2.x网络环境
		WORD ver = MAKEWORD(2,2);	//创建版本号
		WSADATA dat;
		WSAStartup(ver, &dat);		//加载socket相关动态链接库
#endif
		//---------------------
		//-- 用Socket API建立简易TCP客户端
		if (_sock != INVALID_SOCKET)
		{
			printf("关闭socket = <%d>旧连接...\n", _sock);
			closesocket(_sock);
			_sock = INVALID_SOCKET;
		}
		// 1.建立一个socket
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(SOCKET_ERROR == _sock)
		{
			printf("错误,建立socket失败...\n");
			return -1;
		}
		//printf("建立socket = <%d>成功...\n", _sock);
		return 0;
	}

	//连接服务器
	int Connect(char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2.连接服务器 connect
		//printf("socket = <%d>正在连接服务器<%s:%d>...\n", _sock, ip, port);
		sockaddr_in _sin = {};		
		_sin.sin_family = AF_INET;	//IPV4
		_sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else

#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if(SOCKET_ERROR == ret)
		{
			printf("错误,socket = <%d>连接服务器<%s:%d>失败...\n", _sock, ip, port);
		}
		else
		{
			//printf("socket = <%d>连接服务器<%s:%d>成功...\n", _sock, ip, port);
		}

		//int nSendBuf,nRecvBuf;
		//int len = sizeof(int);
		//getsockopt(_sock,SOL_SOCKET, SO_SNDBUF,(char*)&nSendBuf, &len);
		//getsockopt(_sock,SOL_SOCKET, SO_RCVBUF,(char*)&nRecvBuf, &len);
		//cout << "SendBufSize = " << nSendBuf <<endl;
		//cout << "SendBufSize = " << nRecvBuf <<endl;

		////设置接收缓存大小
		//nSendBuf = 1024 * 1024 * 10;
		//setsockopt(_sock, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
		//nRecvBuf = 1024 * 1024 * 10;
		//setsockopt(_sock, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
		//getsockopt(_sock,SOL_SOCKET, SO_SNDBUF,(char*)&nSendBuf, &len);
		//getsockopt(_sock,SOL_SOCKET, SO_RCVBUF,(char*)&nRecvBuf, &len);
		//cout << "SendBufSize_Change = " << nSendBuf <<endl;
		//cout << "SendBufSize_Change = " << nRecvBuf <<endl;

		return ret;
	}

	//关闭客户端
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 7.关闭套接字 closesocket
			closesocket(_sock);
			//关闭WinSock2.x网络环境
			WSACleanup();
#else

#endif
			_sock = INVALID_SOCKET;
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	int SendData(DataHeader* header, int nlen)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, nlen, 0);
		}
		return SOCKET_ERROR;
	}

	//接收数据 (处理粘包、拆包等)
	int RecvData()
	{
		// 6.recv 接收服务器返回的数据
		//接收数据
		int nLen = recv(_sock, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("socket = <%d>与服务器断开连接，任务结束。\n",_sock);
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf+_lastPos,_szRecv, nLen);
		//消息缓冲区数据尾部位置后移
		_lastPos += nLen;
		//判断数据长度大于数据头长度
		while (_lastPos >= sizeof(DataHeader))			//循环处理黏包，一次收到大量数据，循环分割全部处理
		{
			DataHeader* header = (DataHeader*)_szMsgBuf;   
			//判断消息缓冲区大小大于消息长度
			if (_lastPos >= header->dataLength)			//判断处理少包，数据不够就等待下次数据接收
			{
				//消息缓冲区剩余数据长度
				int nSize = _lastPos - header->dataLength;
				//处理网络消息
				OnNetMsg(header);
				//将消息缓冲区未处理的数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				//消息缓冲区数据位置前移
				_lastPos = nSize;
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
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
			{
				LoginResult* loginRet = (LoginResult*)header;		//子类结构体直接由基类转化
				//printf("socket = <%d>收到服务器消息：CMD_LOGIN_RESULT\t数据长度：%d\n", _sock, loginRet->dataLength);
				//返回的登陆结果
				//printf(" --> LoginResult：%d\n",loginRet->result);
			}
			break;
		case CMD_LOGOUT_RESULT:
			{
				LogoutResult *logoutRet = (LogoutResult*)header;	//子类结构体直接由基类转化
				printf("socket = <%d>收到服务器消息：CMD_LOGOUT_RESULT\t数据长度：%d\n", _sock, logoutRet->dataLength);
				//返回的登陆结果
				//printf(" --> LogoutResult：%d\n",logoutRet->result);
			}
			break;
		case CMD_NEW_USER_JOIN:
			{
				NewUserJoin *newUserJoinRet = (NewUserJoin*)header;	//子类结构体直接由基类转化
				printf("socket = <%d>收到服务器消息：CMD_NEW_USER_JOIN\t数据长度：%d\n", _sock, newUserJoinRet->dataLength);
				//返回的登陆结果
				printf(" --> NewUserJoin：<newSocket = %d>\n",newUserJoinRet->sock);
			}
			break;
		default:
			{
				printf("socket = <%d>收到未定义消息\t数据长度：%d", _sock, header->dataLength);
			}
		}
	}

	//查询网络消息
	bool OnRun()
	{
		if (!isRun())
		{
			return false;
		}

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		//等待集合中有可操作Socket，非阻塞模式,timeval没有结果就返回，继续执行
		//使得单线程也可以在无来自服务器的消息处理时，继续进行其他任务
		timeval t = {0,10};	
		int ret = select(0,&fdReads,0,0,&t);
		if (ret < 0)
		{
			printf("socket = <%d>select任务结束....\n",_sock);
			Close();
			return false;
		}
		//查询可读集合中有没有客户端Socket
		if (FD_ISSET(_sock,&fdReads))	
		{
			FD_CLR(_sock,&fdReads);
			if(-1 == RecvData())
			{
				printf("socket = <%d>select任务结束....\n",_sock);
				Close();
				return false;
			}
		}

		//空闲时间处理其他业务
		//printf("空闲时间处理其他业务。。。\n");
	
		return true;
	}

	//判断是否可用
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

private:

};

#endif  //_EasyTcpClient_hpp_