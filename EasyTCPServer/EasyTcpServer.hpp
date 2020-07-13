#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include "CellServer.hpp"

//new -> ���ڴ�    int�ȶ��� -> ջ�ڴ棨С��ֻ�м�M��
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;							//������socket	
	std::vector<CellServerPtr> _cellServers;		//�������߳�
	//vector<CellClientPtr> _clients;			//�ͻ��˶��С����̴߳����ûʲô��,�Ľ���

	CELLTimestamp _tTime;					//��ʱ���߶���
	std::atomic_int _recvCount;					//recv����
	std::atomic_int _msgCount;					//��Ϣ������
	std::atomic_int _recvBytes;					//�����ֽ�������
	std::atomic_int _clientCount;				//�ͻ�����


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
		//�ͷ�CellServer
		//............
	}

	//�ͻ��˼�����Ϣ������
	virtual void OnNetJoin(CellClientPtr& pClient)
	{
		_clientCount++;
	}

	//�ͻ����뿪��Ϣ������
	virtual void OnNetLeave(CellClientPtr& pClient)
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
	virtual void OnNetMsg(CellClientPtr& pClient, DataHeader* header)
	{
		_msgCount++;
		_recvBytes += header->dataLength;
	}

	virtual void OnNetRecv(CellClientPtr& pClient)
	{
		_recvCount++;
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
			printf("Ts<%d>,time<%lfs>,Cs<%d>,Rs<%d>,Ms<%d>,RBs<%d>\n",_cellServers.size(),t1,_clientCount/*_clients.size()*/,_recvCount,_msgCount,_recvBytes);
			_tTime.reset();
			_recvCount = 0;
			_msgCount = 0;
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
			auto ser = std::make_shared<CellServer>(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);		//ע��������Ϣ�¼����ܶ���
			ser->Start();				//����
			printf("������<%d>��Thread\n",i+1);
		}
		
		return 0;
	}

	//��ѯ�ͻ����ٵ��������߳�,���¿ͻ��˼�������
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
			std::shared_ptr<CellClient> pClient = std::make_shared<CellClient>(cSock);
			addClientToCellServer(pClient);
			//_clients.push_back(pClient);
			//printf("socket = <%d>���¿ͻ��˼���[%d]��socket= %d, IP= %s \n",_sock,_clients.size(), cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

private:

};

#endif  //!_EasyTcpServer_hpp_