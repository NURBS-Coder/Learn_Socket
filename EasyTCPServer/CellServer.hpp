#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "CellTask.hpp"
#include "INetEvent.hpp"

class CellServer
{
private:
	SOCKET _sock;						//服务器socket	
	std::vector<CellClientPtr> _clients;		//动态数组：存客户端socket,用指针，在堆内存，防止栈内存不够
	std::vector<CellClientPtr> _clientsBuff;	//客户缓冲队列
	char _szRecv[RECV_BUFF_SIZE];		//接收缓冲区
	
	std::thread _thread;						//消费者处理线程
	std::mutex _mutex;						//缓冲队列的锁
	INetEvent *_pNetEvent;				//网络事件对象

	CellTaskServer m_taskServer;		//任务执行线程

	CELLTimestamp m_time;

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

	//添加发送任务
	void addSendTask(CellClientPtr &pClient, DataHeader *pHeader)
	{
		//CellSendMsg2ClientTaskPtr pCellTask = std::make_shared<CellSendMsg2ClientTask>(pClient,pHeader);
		//m_taskServer.AddTask((CellTaskPtr)pCellTask);

		//lambda表达式
		m_taskServer.AddTask([pClient, pHeader](){
			pClient->SendData(pHeader);
			delete pHeader;
		});
	}

	//往缓冲队列增加客户端
	void addClientToBuff(CellClientPtr& pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex); //使用自解锁，防止添加失败退出而没有解锁
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	//启动该消费者线程
	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
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

		m_taskServer.Start();
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
			}
			
#else

#endif
			_clients.clear();
			_clientsBuff.clear();
		}
	}
		
	//处理网络消息
	//feset备份，用于直接赋值
	fd_set _fdRead_bak;
	//客户列表是否变化
	bool _clients_change;
	bool OnRun()
	{
		_clients_change = true;
		_bIsRun = true;
		while (_bIsRun)
		{
			

			//从缓冲队列添加到正式队列
			if (_clientsBuff.size() > 0)	//访问size没有加锁
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//没有客户端就跳过
			if (_clients.empty()){		//C++11 的跨平台休眠
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				m_time.reset();
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

			if (_clients_change)
			{
				for (size_t i = 0; i < _clients.size(); i++)	//每次循环都要算一遍_clients.size()用于判断，慢
				{
					//将客户端socket加入可读集合
					FD_SET(_clients[i]->sockfd(), &fdRead);
				}
				memcpy(&_fdRead_bak,&fdRead,sizeof(fd_set));
				_clients_change = false;
			}
			else
			{
				memcpy(&fdRead,&_fdRead_bak,sizeof(fd_set));
			}
			

			//等待集合中有可操作Socket，非阻塞模式,timeval没有结果就返回，继续执行
			//使得单线程也可以在无客户端消息处理时，继续进行其他任务
			timeval t = {0,1};	
			int ret = select(0,&fdRead,nullptr,nullptr, /*nullptr*/&t);  //查询后，fdset内的数据会改变，只有有消息的socket在里面
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
							closesocket((*iter)->sockfd());
							_clients.erase(iter);
							_clients_change = true;

						}
					}
				}
			}

			//心跳检测
			auto time = m_time.elapsed();
			for (auto iter = _clients.begin(); iter != _clients.end();)
			{
				//定时发送数据
				(*iter)->checkSend(time);
				//心跳检测
				if (!(*iter)->checkHeart(time))
				{
					//客户端退出，删除Socket
					if (_pNetEvent)
					{
						_pNetEvent->OnNetLeave(*iter);
					}
					closesocket((*iter)->sockfd());
					iter = _clients.erase(iter);
					_clients_change = true;
				}
				else
				{
					++iter;
				}
			}
			m_time.reset();

			//空闲时间处理其他业务
			//printf("空闲时间处理其他业务。。。\n");
		}
		printf("退出一个消费者处理线程....\n");
		return true;
	}

	//接收数据 (处理粘包、拆包等)
	int RecvData(CellClientPtr& pClient)
	{
		//接收包头
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		_pNetEvent->OnNetRecv(pClient);
		
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
	void OnNetMsg(CellClientPtr& pClient, DataHeader* header)
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
				LoginResult *ret = new LoginResult();
				//发送数据包（包头+包体）
				//pClient->SendData(&ret);
				addSendTask(pClient, ret);
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
		case CMD_HEART_C2S:
			{	//重置心跳计数
				pClient->resetDTHeart();
				Heart_S2C *ret = new Heart_S2C();
				addSendTask(pClient, ret);
				//printf("收到<%d>的心跳数据，time=%d,已经重置...\n",pClient->sockfd(), pClient->m_dtHeart);
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

private:

};

typedef std::shared_ptr<CellServer> CellServerPtr;

#endif	//	!_CELL_SERVER_HPP_