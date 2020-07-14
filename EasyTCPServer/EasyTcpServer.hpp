#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include "CellServer.hpp"

//new -> 堆内存    int等定义 -> 栈内存（小，只有几M）
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;								//服务器socket	
	CellThread m_thread;						//接收客户端连接线程
	std::vector<CellServerPtr> _cellServers;	//消费者线程队列

	CELLTimestamp _tTime;						//计时工具对象
	std::atomic_int _recvCount;					//recv计数
	std::atomic_int _msgCount;					//消息包计数
	std::atomic_int _recvBytes;					//数据字节数计数
	std::atomic_int _clientCount;				//客户计数

	
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_msgCount = 0;
		_recvBytes = 0;
		_clientCount = 0;

	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//启动消费者线程,参数为启动的线程数
	int Start(int nCellServer)
	{
		if (INVALID_SOCKET == _sock)
		{
			return -1;
		}

		m_thread.Start(nullptr, [this](CellThread& pThread){OnRun(pThread);});

		for (int i = 0; i < nCellServer; i++)
		{
			auto ser = std::make_shared<CellServer>(i + 1);
			_cellServers.push_back(ser);
			ser->setEventObj(this);		//注册网络消息事件接受对象
			ser->Start();				//启动
			printf("启动第<%d>个Thread\n",i+1);
		}
		
		return 0;
	}

	//关闭Socekt
	void Close()
	{
		printf("1、EasyTCPServer.Close	Start...\n");
		if (_sock != INVALID_SOCKET)
		{
			m_thread.Close();
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
		printf("1、EasyTCPServer.Close	End...\n");
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

protected:
	//客户端加入消息处理函数
	virtual void OnNetJoin(CellClientPtr& pClient)
	{
		_clientCount++;
	}

	//客户端离开消息处理函数
	virtual void OnNetLeave(CellClientPtr& pClient)
	{
		_clientCount--;
	}

	//网络消息统计处理函数
	virtual void OnNetMsg(CellClientPtr& pClient, DataHeader* header)
	{
		_msgCount++;
		_recvBytes += header->dataLength;
	}

	virtual void OnNetRecv(CellClientPtr& pClient)
	{
		_recvCount++;
	}

	//主监听线程定时读取所有子线程的数据进行统计并输出的函数【不用了，现在用网络接口类，子线程直接更新主监听线程的数据】
	void Time4Thread()		
	{

		auto t1 = _tTime.getElapsedTimeInSecond();
		if (t1 >= 1.0)
		{
			printf("Ts<%d>,time<%lfs>,Cs<%d>,Rs<%d>,Ms<%d>,RBs<%d>\n",_cellServers.size(),t1,_clientCount/*_clients.size()*/,_recvCount,_msgCount,_recvBytes);
			_tTime.reset();
			_recvCount = 0;
			_msgCount = 0;
			_recvBytes = 0;
		}
	}

	//查询客户较少的消费者线程,将新客户端加入其中
	void addClientToCellServer(std::shared_ptr<CellClient>& pClient)
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


	//处理网络消息
	void OnRun(CellThread& pThread)
	{
		while (pThread.IsRun())
		{
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
				printf("EasyTCPServer.Accept.select任务结束。\n");
				pThread.Exit();
				break;
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
		}
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
			std::shared_ptr<CellClient> pClient = std::make_shared<CellClient>(cSock);
			addClientToCellServer(pClient);
			//_clients.push_back(pClient);
			//printf("socket = <%d>有新客户端加入[%d]：socket= %d, IP= %s \n",_sock,_clients.size(), cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

private:

};

#endif  //!_EasyTcpServer_hpp_