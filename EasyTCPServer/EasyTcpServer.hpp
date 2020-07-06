#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define FD_SETSIZE      2506

	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ���� WIN32_LEAN_AND_MEAN ��
	#pragma comment(lib,"ws2_32.lib")	//��Ӷ�̬��
#else

#endif // _WIN32

#include <stdio.h>				//printf()....
using namespace std;
#include <iostream>				//std::cout....
#include <vector>
#include <thread>				//c++��׼�߳̿�
#include <mutex>				//�������
#include <atomic>				//ԭ�Ӳ�����

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024*10		//���ջ�������С��Ԫ��С 40kb
#endif
				
//�ͻ������Ͷ���
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

	//��������
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
	SOCKET _sockfd;							//�ͻ���socket
	char _szMsgBuf[RECV_BUFF_SIZE * 5];	//�ڶ�����������Ϣ������
	int _lastPos;							//��¼�ڶ�������������λ��

};

//�����¼��ӿ�
class INetEvent		//�ӿ���
{
public:
	INetEvent(){}
	~INetEvent(){}
public:
	//���麯����ֻ���������̳�ʱʵ��
	//�ͻ��˼����¼�,1�̵߳���
	virtual void OnNetJoin(ClientSocket* pClient) = 0; 
	//�ͻ����뿪�¼�,4�̵߳���
	virtual void OnNetLeave(ClientSocket* pClient) = 0; 
	//��Ӧ����ӿ��¼�,4�̵߳���
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0; 
private:

};

class CellServer
{
private:
	SOCKET _sock;						//������socket	
	vector<ClientSocket*> _clients;		//��̬���飺��ͻ���socket,��ָ�룬�ڶ��ڴ棬��ֹջ�ڴ治��
	vector<ClientSocket*> _clientsBuff;	//�ͻ��������
	char _szRecv[RECV_BUFF_SIZE];		//���ջ�����
	
	thread _thread;						//�����ߴ����߳�
	mutex _mutex;						//������е���
	INetEvent *_pNetEvent;				//�����¼�����

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

	//������������ӿͻ���
	void addClientToBuff(ClientSocket * pClient)
	{
		lock_guard<mutex> lock(_mutex); //ʹ���Խ�������ֹ���ʧ���˳���û�н���
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	//�������������߳�
	void Start()
	{
		_thread = thread(mem_fn(&CellServer::OnRun), this);
		//�ܶ�C++�ĸ��ֺ�ţ���Ƕ��������һ���Ҹ棬�Ǿ��ǣ��ڴ���STL�����������ʱ�򣬾�����Ҫ�Լ�дѭ����
		//ԭ�򣺡�STL��������������Լ��ı�����������STL�ṩ��for_each�Ⱥ�����
		//Ч�ʣ��㷨ͨ���ȳ���Ա������ѭ������Ч�� 
		//��ȷ�ԣ�дѭ��ʱ�ȵ����㷨�����ײ�������
		//��ά���ԣ��㷨ͨ��ʹ�������Ӧ����ʽѭ�����ɾ�����ֱ�ۡ�

		//����������std::mem_fun,std::mem_fun_ref,std::mem_fn()
		//mem_fun����Ա����ת��Ϊ��������(ָ��汾)��ָ��汾ָ����ͨ��ָ������ָ����ö���������std::mem_fun_ref�Ƕ�Ӧ�����ð汾��
		//��Ա����ָ������ͨ����ָ�벻ͬ��������󶨵��ض��Ķ����ϲ���ʹ�ã����Գ�Ա����ָ�벻��һ���ɵ��ö���
		//����ʹ��һЩ��Ҫ�ɵ��ö���ĺ���ʱ����std::fot_each������Ҫ���г�Ա���������������ת��
		
		_thread.detach();  //��detachҲ��join���߳�Ҳ�������У�������
	}

	//��ѯ��ǰ�û�����
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//�������ݣ������û���		// ���߳������㲥������
	//void SendData2All(DataHeader* header)
	//{
	//	for (size_t i = 0; i < _clients.size(); i++)	//ÿ��ѭ����Ҫ��һ��_clients.size()�����жϣ���
	//	{
	//		//���Ͱ��壨��ͷ+���壩
	//		SendData(_clients[i]->sockfd(), header);
	//	}
	//}

	//�Ƿ�����
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

	//�ر�Socekt
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			//�ر��߳�
			_bIsRun = false;
			//�ر��׽��� closesocket
			for (int n = (int)_clients.size() - 1 ; n >= 0; n--)		//ѭ��ֻ����һ��_clients.size()����
			{															//��size_t���޷�����������--��size_t��ֵΪ0��ʱ���ټ����Լ��ͻ����0
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			
#else

#endif
			_clients.clear();
			_clientsBuff.clear();
		}
	}
		
	//����������Ϣ
	bool OnRun()
	{
		_bIsRun = true;
		while (_bIsRun)
		{
			//�ӻ��������ӵ���ʽ����
			if (_clientsBuff.size() > 0)	//����sizeû�м���
			{
				lock_guard<mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				}
				_clientsBuff.clear();
			}

			//û�пͻ��˾�����
			if (_clients.empty()){		//C++11 �Ŀ�ƽ̨����
				chrono::milliseconds t(1);
				this_thread::sleep_for(t);
				continue;
			}

			//fd_set����һ��Socket����
			fd_set fdRead;
			//fd_set fdWrite;
			//fd_set fdExcept;

			//��ʼ��fd_set�����㣩
			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExcept);

			for (size_t i = 0; i < _clients.size(); i++)	//ÿ��ѭ����Ҫ��һ��_clients.size()�����жϣ���
			{
				//���ͻ���socket����ɶ�����
				FD_SET(_clients[i]->sockfd(), &fdRead);
			}

			//�ȴ��������пɲ���Socket��������ģʽ,timevalû�н���ͷ��أ�����ִ��
			//ʹ�õ��߳�Ҳ�������޿ͻ�����Ϣ����ʱ������������������
			//timeval t = {1,0};	
			int ret = select(0,&fdRead,nullptr,nullptr, nullptr/*&t*/);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return false;
			}
		
			//�����пɶ���Ϣ�Ŀͻ���socket
			for (int n = (int)_clients.size() -1; n >=0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(),&fdRead))
				{		
					//����ͻ�������
					if(-1 == RecvData(_clients[n]))
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							//�ͻ����˳���ɾ��Socket
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

			//����ʱ�䴦������ҵ��
			//printf("����ʱ�䴦������ҵ�񡣡���\n");
		}
		printf("�˳�һ�������ߴ����߳�....\n");
		return true;
	}

	//�������� (����ճ���������)
	int RecvData(ClientSocket* pClient)
	{
		//���հ�ͷ
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		
		if (nLen <= 0)
		{
			//printf("�ͻ���<socket = %d>���˳������������\n",pClient->sockfd());
			return -1;
		}

		//����ȡ�����ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(),_szRecv, nLen);
		//��Ϣ����������β��λ�ú���
		pClient->setLastPos( pClient->getLastPos() + nLen);

		//�ж����ݳ��ȴ�������ͷ����
		while (pClient->getLastPos() >= sizeof(DataHeader))			//ѭ����������һ���յ��������ݣ�ѭ���ָ�ȫ������
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();   
			//�ж���Ϣ��������С������Ϣ����
			if (pClient->getLastPos() >= header->dataLength)			//�жϴ����ٰ������ݲ����͵ȴ��´����ݽ���
			{
				//��Ϣ������ʣ�����ݳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient,header);
				//����Ϣ������δ���������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ����������λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else
			{
				//��Ϣ���������ݲ���һ����Ϣ
				break;
			}			
		}
		return 0;
	}

	//��Ӧ������Ϣ
	void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		//_recvCount++;
		//_recvBytes += header->dataLength;

		_pNetEvent->OnNetMsg(pClient, header);	//����������Ϣ��֪ͨ�������̸߳�������

		// 6.switch �������� 
		// 7.send ��ͻ��˷�����������
		switch (header->cmd)
		{
		case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				//printf("socket = <%d>�յ��ͻ���<socket = %d>����CMD_LOGIN\t���ݳ��ȣ�%d\n",_sock, cSock,login->dataLength);
				//�����ж��û���������
				//printf(" --> ��½��UserName=%s,PassWord=%s\n",login->userName,login->passWord);
				//�ظ��ͻ��˵�½���
				LoginResult ret;
				//�������ݰ�����ͷ+���壩
				pClient->SendData(&ret);
				//SendData(pClient->sockfd(), &ret);
			}
			break;
		case CMD_LOGOUT:
			{
				Logout *logout = (Logout*)header;
				//printf("socket = <%d>�յ��ͻ���<socket = %d>����CMD_LOGOUT\t���ݳ��ȣ�%d\n",_sock, cSock,logout->dataLength);
				//�����ж��û���
				//printf(" --> �ǳ���UserName=%s\n",logout->userName);
				//�ظ��ͻ��˵ǳ����
				LogoutResult ret;
				//���Ͱ��壨��ͷ+���壩
				//SendData(cSock, &ret);
			}
			break;
		default:
			{
				//���Ͱ�ͷ
				//DataHeader ret;
				//SendData(cSock, &ret);
				printf("socket = <%d>�յ�δ������Ϣ\t���ݳ��ȣ�%d", _sock, header->dataLength);
			}
			break;
		}
	}

	////��������
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

//new -> ���ڴ�    int�ȶ��� -> ջ�ڴ棨С��ֻ�м�M��
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;							//������socket	
	vector<CellServer*> _cellServers;		//�������߳�
	//vector<ClientSocket*> _clients;			//�ͻ��˶��С����̴߳����ûʲô��,�Ľ���

	CELLTimestamp _tTime;					//��ʱ���߶���
	atomic_int _recvCount;					//��Ϣ������
	atomic_int _recvBytes;					//�����ֽ�������
	atomic_int _clientCount;				//�ͻ�����


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

	//�ͻ��˼�����Ϣ������
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}

	//�ͻ����뿪��Ϣ������
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

	//������Ϣͳ�ƴ�����
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{

		_recvCount++;
		_recvBytes += header->dataLength;
	
	}

	//�������̶߳�ʱ��ȡ�������̵߳����ݽ���ͳ�Ʋ�����ĺ����������ˣ�����������ӿ��࣬���߳�ֱ�Ӹ����������̵߳����ݡ�
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

	//�ر�Socekt
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
			//���Windows socket����
			WSACleanup();
#else

#endif
			_sock = INVALID_SOCKET;
		}
	}

	//��ʼ��Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����WinSock2.x���绷��
		WORD ver = MAKEWORD(2,2);	//�����汾��
		WSADATA dat;
		WSAStartup(ver, &dat);		//����socket��ض�̬���ӿ�
#endif
		//---------------------
		//-- ��Socket API��������TCP������
		if (_sock != INVALID_SOCKET)
		{
			printf("�ر�socket = <%d>������...\n", _sock);
			Close();	//??WSACleanup();??
		}
		// 1.����һ��socket(IPV4�Ļ����ֽ�����TCPЭ����׽���)
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(SOCKET_ERROR == _sock)
		{
			printf("����,����socketʧ��...\n");
		}
		printf("����socket = <%d>�ɹ�...\n", _sock);
		return _sock;
	}

	//��IP�Ͷ˿ں�
	int Bind(char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2.bind �����ڽ��տͻ������ӵ�����˿�
		sockaddr_in _sin = {};		//������ͱ�bind����Ҫ��sockaddr����
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
			printf("����,socket = <%d>������˿�<%d>ʧ��...\n",_sock,port);
		}
		else
		{
			printf("socket = <%d>������˿�<%d>�ɹ�...\n" ,_sock, port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		if (INVALID_SOCKET == _sock)
		{
			return -1;
		}
		// 3.listen ��������˿�(�ȴ���������5)
		int ret = listen(_sock, n);
		if(SOCKET_ERROR == ret)
		{
			printf("socket = <%d>����,��������˿�ʧ��...\n",_sock);
		}
		else
		{
			printf("socket = <%d>��������˿ڳɹ�...\n",_sock);
		}
		return ret;
	}

	//�����������߳�,����Ϊ�������߳���
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
			ser->setEventObj(this);		//ע��������Ϣ�¼����ܶ���
			ser->Start();				//����
			printf("������<%d>��Thread\n",i+1);
		}
		
		return 0;
	}

	//��ѯ�ͻ����ٵ��������߳�,���¿ͻ��˼�������
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


	//�Ƿ�����
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

	//����������Ϣ
	bool OnRun()
	{
		if (!isRun())
		{
			return false;
		}
		//nfds��һ������ֵ����ָfd_set������������������Socket���ķ�Χ����������������������������Socket�����ֵ��һ��
		//nfds��Windows������,�������㣬��Linux�����ã�Ϊ�˼��ݲ�����socket
		//readfds���ɶ�fd�ļ���
		//writefds����дfd�ļ���
		//exceptfds���쳣fd�ļ���
		//timeout����ѯ�ӳ٣��ÿվ�������ģʽ

		//fd_set����һ��Socket����
		fd_set fdRead;
		//fd_set fdWrite;
		//fd_set fdExcept;

		//��ʼ��fd_set�����㣩
		FD_ZERO(&fdRead);
		//FD_ZERO(&fdWrite);
		//FD_ZERO(&fdExcept);

		//socket ���뼯��
		FD_SET(_sock,&fdRead);
		//FD_SET(_sock,&fdWrite);
		//FD_SET(_sock,&fdExcept);

		//�ȴ��������пɲ���Socket��������ģʽ,timevalû�н���ͷ��أ�����ִ��
		//ʹ�õ��߳�Ҳ�������޿ͻ�����Ϣ����ʱ������������������
		timeval t = {1,0};	
		int ret = select(0,&fdRead,nullptr/*&fdWrite*/,nullptr/*&fdExcept*/, &t);
		if (ret < 0)
		{
			printf("Accept select���������\n");
			Close();
			return false;
		}
		
		//��ѯ�ɶ���������û�м���Socket
		if (FD_ISSET(_sock,&fdRead))	
		{
			FD_CLR(_sock,&fdRead);		//����Socket�Ӽ��������
			Accept();
		}

		//����ʱ�䴦������ҵ��
		//printf("����ʱ�䴦������ҵ�񡣡���\n");

		Time4Thread();		//��ʱ���ͳ�����ݵĺ���

		return true;
	}

	//���տͻ�������
	SOCKET Accept()
	{
		// 4.accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddrLen);
#else

#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("����,socket = <%d>���յ���Ч�ͻ���Socket...\n", _sock);
		}
		else
		{
			//int nSendBuf,nRecvBuf;
			//int len = sizeof(int);
			//getsockopt(cSock,SOL_SOCKET, SO_SNDBUF,(char*)&nSendBuf, &len);
			//getsockopt(cSock,SOL_SOCKET, SO_RCVBUF,(char*)&nRecvBuf, &len);
			//cout << "SendBufSize = " << nSendBuf <<endl;
			//cout << "SendBufSize = " << nRecvBuf <<endl;

			////���ý��ջ����С
			//nSendBuf = 1024 * 1024 * 10;
			//setsockopt(cSock, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
			//nRecvBuf = 1024 * 1024 * 10;
			//setsockopt(cSock, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
			//getsockopt(cSock,SOL_SOCKET, SO_SNDBUF,(char*)&nSendBuf, &len);
			//getsockopt(cSock,SOL_SOCKET, SO_RCVBUF,(char*)&nRecvBuf, &len);
			//cout << "SendBufSize_Change = " << nSendBuf <<endl;
			//cout << "SendBufSize_Change = " << nRecvBuf <<endl;

			//�¿ͻ��˼��룬Ⱥ������socket
			//NewUserJoin newUserJoin;
			//newUserJoin.sock = cSock;
			//SendData2All(&newUserJoin);

			//�¿ͻ��˼����������̵߳Ľ��ٵĻ������vector
			ClientSocket *pClient = new ClientSocket(cSock);
			addClientToCellServer(pClient);
			//_clients.push_back(pClient);
			//printf("socket = <%d>���¿ͻ��˼���[%d]��socket= %d, IP= %s \n",_sock,_clients.size(), cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

private:

};

#endif  //!_EasyTcpServer_hpp_