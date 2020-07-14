#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "CellTask.hpp"
#include "INetEvent.hpp"

class CellServer
{
private:
	std::vector<CellClientPtr> _clients;		//动态数组：存客户端socket,用指针，在堆内存，防止栈内存不够
	std::vector<CellClientPtr> _clientsBuff;	//客户缓冲队列
	char _szRecv[RECV_BUFF_SIZE];				//接收缓冲区
	
	std::mutex _mutex;							//缓冲队列的锁
	INetEvent *_pNetEvent;						//网络事件对象
	CellTaskServer m_taskServer;				//任务执行线程
	CELLTimestamp m_time;						//定时时间戳
	CellThread m_thread;						//消费者处理线程
	fd_set _fdRead_bak;							//feset备份，用于直接赋值
	bool _clients_change;						//客户列表是否变化
	int m_id;

public:
	CellServer(int id)
	{
		m_id = id;
		_pNetEvent = nullptr;
		memset(_szRecv,0,RECV_BUFF_SIZE);

		m_taskServer.m_id = m_id;
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
		_clientsBuff.push_back(pClient);
	}

	//启动该消费者线程
	void Start()
	{
		m_thread.Start(nullptr, [this](CellThread& pThread){OnRun(pThread);});
		m_taskServer.Start();
	}

	//关闭该消费者线程
	void Close()
	{
		printf("2、CellServer<%d>.Close	Start...\n", m_id);

		m_taskServer.Close();
		m_thread.Close();

		//清空客户端，析构时就关闭客户端socket
		_clients.clear();
		_clientsBuff.clear();

		printf("2、CellServer<%d>.Close	End...\n", m_id);
	}

	//查询当前用户数量
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
		
	//处理网络消息
	void OnRun(CellThread& pThread)
	{
		_clients_change = true;
		while (pThread.IsRun())
		{
			//从缓冲队列添加到正式队列
			if (_clientsBuff.size() > 0)	//访问size没有加锁
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					pClient->m_serverID = m_id;
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
				printf("CellServer<%d>.OnRun.Select任务结束。\n", m_id);
				pThread.Exit();
				break;
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
		printf("2、CellServer<%d>.OnRun  Close...\n", m_id);
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
				printf("socket = <%d>收到未定义消息\t数据长度：%d", pClient->sockfd(), header->dataLength);
			}
			break;
		}
	}

private:

};

typedef std::shared_ptr<CellServer> CellServerPtr;

#endif	//	!_CELL_SERVER_HPP_