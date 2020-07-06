#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      2506

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
#include <thread>				//c++标准线程库
#include <mutex>				//锁定义库
#include <atomic>				//原子操作库

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024*10		//接收缓冲区最小单元大小 40kb
#endif
				
//客户端类型定义
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf,0,RECV_BUFF_SIZE * 5);
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

	//发送数据
	int SendData(DataHeader* header)
	{
		if (_sockfd == INVALID_SOCKET)
		{
			return -1;
		}

		if (header)
		{
			return send(_sockfd, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:
	SOCKET _sockfd;							//客户端socket
	char _szMsgBuf[RECV_BUFF_SIZE * 5];	//第二缓冲区、消息缓冲区
	int _lastPos;							//记录第二缓冲区中数据位置

};

//网络事件接口
class INetEvent		//接口类
{
public:
	INetEvent(){}
	~INetEvent(){}
public:
	//纯虚函数，只有声明，继承时实现
	//客户端加入事件,1线程调用
	virtual void OnNetJoin(ClientSocket* pClient) = 0; 
	//客户端离开事件,4线程调用
	virtual void OnNetLeave(ClientSocket* pClient) = 0; 
	//响应网络接口事件,4线程调用
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0; 
private:

};

class CellServer
{
private:
	SOCKET _sock;						//服务器socket	
	vector<ClientSocket*> _clients;		//动态数组：存客户端socket,用指针，在堆内存，防止栈内存不够
	vector<ClientSocket*> _clientsBuff;	//客户缓冲队列
	char _szRecv[RECV_BUFF_SIZE];		//接收缓冲区
	
	thread _thread;						//消费者处理线程
	mutex _mutex;						//缓冲队列的锁
	INetEvent *_pNetEvent;				//网络事件对象

	bool _bIsRun;

public:
	//atomic_int _recvCount;
	//atomic_int _recvBytes;

public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
		memset(_szRecv,0,RECV_BUFF_SIZE);

		//_recvCount = 0;
		//_recvBytes = 0;
		
		_bIsRun = false;
	}

	~CellServer()
	{
		Close();
	}

	void setEventObj(INetEvent* pNetEvent)
	{
		_pNetEvent = pNetEvent;
	}

	//往缓冲队列增加客户端
	void addClientToBuff(ClientSocket * pClient)
	{
		lock_guard<mutex> lock(_mutex); //使用自解锁，防止添加失败退出而没有解锁
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	//启动该消费者线程
	void Start()
	{
		_thread = thread(mem_fn(&CellServer::OnRun), this);
		//很多C++的高手和牛人们都会给我们一个忠告，那就是：在处理STL里面的容器的时候，尽量不要自己写循环。
		//原因：【STL里面的容器都有自己的遍历器，而且STL提供了for_each等函数】
		//效率：算法通常比程序员产生的循环更高效。 
		//正确性：写循环时比调用算法更容易产生错误。
		//可维护性：算法通常使代码比相应的显式循环更干净、更直观。

		//函数适配器std::mem_fun,std::mem_fun_ref,std::mem_fn()
		//mem_fun将成员函数转换为函数对象(指针版本)。指针版本指的是通过指向对象的指针调用对象的情况。std::mem_fun_ref是对应的引用版本。
		//成员函数指针与普通函数指针不同，它必须绑定到特定的对象上才能使用，所以成员函数指针不是一个可调用对象。
		//当在使用一些需要可调用对象的函数时。如std::fot_each，就需要进行成员函数到函数对象的转换
		
		_thread.detach();  //不detach也不join，线程也可以运行？？？？
	}

	//查询当前用户数量
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//发送数据（所有用户）		// 多线程这样广播不合理
	//void SendData2All(DataHeader* header)
	//{
	//	for (size_t i = 0; i < _clients.size(); i++)	//每次循环都要算一遍_clients.size()用于判断，慢
	//	{
	//		//发送包体（包头+包体）
	//		SendData(_clients[i]->sockfd(), header);
	//	}
	//}

	//是否工作中
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

	//关闭Socekt
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//关闭线程
			_bIsRun = false;
			//关闭套接字 closesocket
			for (int n = (int)_clients.size() - 1 ; n >= 0; n--)		//循环只调用一次_clients.size()，快
			{															//但size_t是无符号数，不能--，size_t的值为0的时候，再继续自减就会大于0
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			
#else

#endif
			_clients.clear();
			_clientsBuff.clear();
		}
	}
		
	//处理网络消息
	bool OnRun()
	{
		_bIsRun = true;
		while (_bIsRun)
		{
			//从缓冲队列添加到正式队列
			if (_clientsBuff.size() > 0)	//访问size没有加锁
			{
				lock_guard<mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}

			//没有客户端就跳过
			if (_clients.empty()){		//C++11 的跨平台休眠
				chrono::milliseconds t(1);
				this_thread::sleep_for(t);
				continue;
			}

			//fd_set就是一个Socket集合
			fd_set fdRead;
			//fd_set fdWrite;
			//fd_set fdExcept;

			//初始化fd_set（清零）
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExcept);

			for (size_t i = 0; i < _clients.size(); i++)	//每次循环都要算一遍_clients.size()用于判断，慢
			{
				//将客户端socket加入可读集合
				FD_SET(_clients[i]->sockfd(), &fdRead);
			}

			//等待集合中有可操作Socket，非阻塞模式,timeval没有结果就返回，继续执行
			//使得单线程也可以在无客户端消息处理时，继续进行其他任务
			//timeval t = {1,0};	
			int ret = select(0,&fdRead,nullptr,nullptr, nullptr/*&t*/);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
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
							if (_pNetEvent)
							{
								_pNetEvent->OnNetLeave(_clients[n]);
							}

							delete _clients[n];
							_clients.erase(iter);

						}
					}
				}
			}

			//空闲时间处理其他业务
			//printf("空闲时间处理其他业务。。。\n");
		}
		printf("退出一个消费者处理线程....\n");
		return true;
	}

	//接收数据 (处理粘包、拆包等)
	int RecvData(ClientSocket* pClient)
	{
		//接收包头
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		
		if (nLen <= 0)
		{
			//printf("客户端<socket = %d>已退出，任务结束。\n",pClient->sockfd());
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
				OnNetMsg(pClient,header);
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
	void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		//_recvCount++;
		//_recvBytes += header->dataLength;

		_pNetEvent->OnNetMsg(pClient, header);	//调用网络消息，通知主接收线程更新数据

		// 6.switch 处理请求 
		// 7.send 向客户端返回请求数据
		switch (header->cmd)
		{
		case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				//printf("socket = <%d>收到客户端<socket = %d>请求：CMD_LOGIN\t数据长度：%d\n",_sock, cSock,login->dataLength);
				//忽略判断用户名和密码
				//printf(" --> 登陆：UserName=%s,PassWord=%s\n",login->userName,login->passWord);
				//回复客户端登陆结果
				LoginResult ret;
				//发送数据包（包头+包体）
				pClient->SendData(&ret);
				//SendData(pClient->sockfd(), &ret);
			}
			break;
		case CMD_LOGOUT:
			{
				Logout *logout = (Logout*)header;
				//printf("socket = <%d>收到客户端<socket = %d>请求：CMD_LOGOUT\t数据长度：%d\n",_sock, cSock,logout->dataLength);
				//忽略判断用户名
				//printf(" --> 登出：UserName=%s\n",logout->userName);
				//回复客户端登出结果
				LogoutResult ret;
				//发送包体（包头+包体）
				//SendData(cSock, &ret);
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

	////发送数据
	//int SendData(SOCKET sock, DataHeader* header)
	//{
	//	if (header)
	//	{
	//		return send(sock, (const char*)header, header->dataLength, 0);
	//	}
	//	return SOCKET_ERROR;
	//}

private:

};

//new -> 堆内存    int等定义 -> 栈内存（小，只有几M）
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;							//服务器socket	
	vector<CellServer*> _cellServers;		//消费者线程
	//vector<ClientSocket*> _clients;			//客户端队列【该线程存这个没什么用,改进】

	CELLTimestamp _tTime;					//计时工具对象
	atomic_int _recvCount;					//消息包计数
	atomic_int _recvBytes;					//数据字节数计数
	atomic_int _clientCount;				//客户计数


public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_recvBytes = 0;
		_clientCount = 0;

	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//客户端加入消息处理函数
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}

	//客户端离开消息处理函数
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		//for (int i = (int)_clients.size() - 1 ; i >= 0; i--)
		//{
		//	if (_clients[i] == pClient)	
		//	{
		//		auto iter = _clients.begin() + i;
		//		if (iter != _clients.end())
		//		{
		//			_clients.erase(iter);
		//		}
		//	}
		//}
		_clientCount--;
	}

	//网络消息统计处理函数
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{

		_recvCount++;
		_recvBytes += header->dataLength;
	
	}

	//主监听线程定时读取所有子线程的数据进行统计并输出的函数【不用了，现在用网络接口类，子线程直接更新主监听线程的数据】
	void Time4Thread()		
	{
	//	int recvCount=0;
	//	int recvBytes=0;
		auto t1 = _tTime.getElapsedTimeInSecond();
		if (t1 >= 1.0)
		{
	//		for (auto ser : _cellServers)
	//		{
	//			recvCount += ser->_recvCount;
	//			ser->_recvCount = 0;
	//			recvBytes += ser->_recvBytes;
	//			ser->_recvBytes = 0;
	//		}
			printf("thread<%d>,time<%lfs>,socketCount<%d>,recvCount<%d>,recvBytes<%d>\n",_cellServers.size(),t1,_clientCount/*_clients.size()*/,_recvCount,_recvBytes);
			_tTime.reset();
			_recvCount = 0;
			_recvBytes = 0;

			//int i = 0;
			//for (auto CellServer : _cellServers)
			//{
			//	printf("thread<%d>,recvCount<%d>\n",++i,CellServer->getClientCount());
			//}
		}
	}

	//关闭Socekt
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for(auto ser :_cellServers)
			{
				ser->Close();
			}

			closesocket(_sock);
			//---------------------
			//清除Windows socket环境
			WSACleanup();
#else

#endif
			_sock = INVALID_SOCKET;
		}
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
		int ret = ::bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
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

	//启动消费者线程,参数为启动的线程数
	int Start(int nCellServer)
	{
		if (INVALID_SOCKET == _sock)
		{
			return -1;
		}

		for (int i = 0; i < nCellServer; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);		//注册网络消息事件接受对象
			ser->Start();				//启动
			printf("启动第<%d>个Thread\n",i+1);
		}
		
		return 0;
	}

	//查询客户较少的消费者线程,将新客户端加入其中
	void addClientToCellServer(ClientSocket * pClient)
	{
		if (0 == _cellServers.size())
		{
			return;
		}

		auto pMinServer = _cellServers[0];
		for (auto CellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > CellServer->getClientCount())
			{
				pMinServer = CellServer;
			}
		}
		pMinServer->addClientToBuff(pClient);
		OnNetJoin(pClient);
	}


	//是否工作中
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
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
		//fd_set fdWrite;
		//fd_set fdExcept;

		//初始化fd_set（清零）
		FD_ZERO(&fdRead);
		//FD_ZERO(&fdWrite);
		//FD_ZERO(&fdExcept);

		//socket 加入集合
		FD_SET(_sock,&fdRead);
		//FD_SET(_sock,&fdWrite);
		//FD_SET(_sock,&fdExcept);

		//等待集合中有可操作Socket，非阻塞模式,timeval没有结果就返回，继续执行
		//使得单线程也可以在无客户端消息处理时，继续进行其他任务
		timeval t = {1,0};	
		int ret = select(0,&fdRead,nullptr/*&fdWrite*/,nullptr/*&fdExcept*/, &t);
		if (ret < 0)
		{
			printf("Accept select任务结束。\n");
			Close();
			return false;
		}
		
		//查询可读集合中有没有监听Socket
		if (FD_ISSET(_sock,&fdRead))	
		{
			FD_CLR(_sock,&fdRead);		//将该Socket从集合中清除
			Accept();
		}

		//空闲时间处理其他业务
		//printf("空闲时间处理其他业务。。。\n");

		Time4Thread();		//定时输出统计数据的函数

		return true;
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
			//int nSendBuf,nRecvBuf;
			//int len = sizeof(int);
			//getsockopt(cSock,SOL_SOCKET, SO_SNDBUF,(char*)&nSendBuf, &len);
			//getsockopt(cSock,SOL_SOCKET, SO_RCVBUF,(char*)&nRecvBuf, &len);
			//cout << "SendBufSize = " << nSendBuf <<endl;
			//cout << "SendBufSize = " << nRecvBuf <<endl;

			////设置接收缓存大小
			//nSendBuf = 1024 * 1024 * 10;
			//setsockopt(cSock, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
			//nRecvBuf = 1024 * 1024 * 10;
			//setsockopt(cSock, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
			//getsockopt(cSock,SOL_SOCKET, SO_SNDBUF,(char*)&nSendBuf, &len);
			//getsockopt(cSock,SOL_SOCKET, SO_RCVBUF,(char*)&nRecvBuf, &len);
			//cout << "SendBufSize_Change = " << nSendBuf <<endl;
			//cout << "SendBufSize_Change = " << nRecvBuf <<endl;

			//新客户端加入，群发他的socket
			//NewUserJoin newUserJoin;
			//newUserJoin.sock = cSock;
			//SendData2All(&newUserJoin);

			//新客户端加入消费者线程的较少的缓冲队列vector
			ClientSocket *pClient = new ClientSocket(cSock);
			addClientToCellServer(pClient);
			//_clients.push_back(pClient);
			//printf("socket = <%d>有新客户端加入[%d]：socket= %d, IP= %s \n",_sock,_clients.size(), cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

private:

};

#endif  //!_EasyTcpServer_hpp_