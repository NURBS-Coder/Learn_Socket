#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
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

#include "MessageHeader.hpp"

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024*40		//���ջ�������С��Ԫ��С 40kb
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
	SOCKET _sockfd;							//�ͻ���socket
	char _szMsgBuf[RECV_BUFF_SIZE * 10];	//�ڶ�����������Ϣ������
	int _lastPos;							//��¼�ڶ�������������λ��

	
};

//new -> ���ڴ�    int�ȶ��� -> ջ�ڴ棨С��ֻ�м�M��
class EasyTcpServer
{
private:
	SOCKET _sock;						//������socket	
	vector<ClientSocket*> _clients;		//��̬���飺��ͻ���socket,��ָ�룬�ڶ��ڴ棬��ֹջ�ڴ治��
	char _szRecv[RECV_BUFF_SIZE];		//���ջ�����

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
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
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
			//�¿ͻ��˼��룬Ⱥ������socket
			NewUserJoin newUserJoin;
			newUserJoin.sock = cSock;
			//SendData2All(&newUserJoin);
			//�¿ͻ��˼���vector
			_clients.push_back(new ClientSocket(cSock));
			printf("socket = <%d>���¿ͻ��˼��룺socket= %d, IP= %s \n",_sock, cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return cSock;
	}

	//�ر�Socekt
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 8.�ر��׽��� closesocket
			for (int n = (int)_clients.size() - 1 ; n >= 0; n--)		//ѭ��ֻ����һ��_clients.size()����
			{															//��size_t���޷�����������--��size_t��ֵΪ0��ʱ���ټ����Լ��ͻ����0
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			//---------------------
			//���Windows socket����
			WSACleanup();
#else

#endif
			_sock = INVALID_SOCKET;
			_clients.clear();
		}
	}

	//��������(ָ���û�)
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//�������ݣ������û���
	void SendData2All(DataHeader* header)
	{
		for (size_t i = 0; i < _clients.size(); i++)	//ÿ��ѭ����Ҫ��һ��_clients.size()�����жϣ���
		{
			//���Ͱ��壨��ͷ+���壩
			SendData(_clients[i]->sockfd(), header);
		}
	}

	//�������� (����ճ���������)
	int RecvData(ClientSocket* pClient)
	{

		//���հ�ͷ
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		
		if (nLen <= 0)
		{
			printf("�ͻ���<socket = %d>���˳������������\n",pClient->sockfd());
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
				OnNetMsg(pClient->sockfd(),header);
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
	void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		// 6.switch �������� 
		// 7.send ��ͻ��˷�����������
		switch (header->cmd)
		{
		case CMD_LOGIN:
			{
				Login* login = (Login*)header;
				printf("socket = <%d>�յ��ͻ���<socket = %d>����CMD_LOGIN\t���ݳ��ȣ�%d\n",_sock, cSock,login->dataLength);
				//�����ж��û���������
				//printf(" --> ��½��UserName=%s,PassWord=%s\n",login->userName,login->passWord);
				//�ظ��ͻ��˵�½���
				LoginResult ret;
				//�������ݰ�����ͷ+���壩
				//SendData(cSock, &ret);
			}
			break;
		case CMD_LOGOUT:
			{
				Logout *logout = (Logout*)header;
				printf("socket = <%d>�յ��ͻ���<socket = %d>����CMD_LOGOUT\t���ݳ��ȣ�%d\n",_sock, cSock,logout->dataLength);
				//�����ж��û���
				printf(" --> �ǳ���UserName=%s\n",logout->userName);
				//�ظ��ͻ��˵ǳ����
				LogoutResult ret;
				//���Ͱ��壨��ͷ+���壩
				SendData(cSock, &ret);
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
		fd_set fdWrite;
		fd_set fdExcept;

		//��ʼ��fd_set�����㣩
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		//socket ���뼯��
		FD_SET(_sock,&fdRead);
		FD_SET(_sock,&fdWrite);
		FD_SET(_sock,&fdExcept);

		for (size_t i = 0; i < _clients.size(); i++)	//ÿ��ѭ����Ҫ��һ��_clients.size()�����жϣ���
		{
			//���ͻ���socket����ɶ�����
			FD_SET(_clients[i]->sockfd(), &fdRead);
		}

		//�ȴ��������пɲ���Socket��������ģʽ,timevalû�н���ͷ��أ�����ִ��
		//ʹ�õ��߳�Ҳ�������޿ͻ�����Ϣ����ʱ������������������
		timeval t = {1,0};	
		int ret = select(0,&fdRead,&fdWrite,&fdExcept, &t);
		if (ret < 0)
		{
			printf("select���������\n");
			Close();
			return false;
		}
		
		//��ѯ�ɶ���������û�м���Socket
		if (FD_ISSET(_sock,&fdRead))	
		{
			FD_CLR(_sock,&fdRead);		//����Socket�Ӽ��������
			Accept();
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
						delete _clients[n];
						_clients.erase(iter);
					}
				}
			}
		}

		//����ʱ�䴦������ҵ��
		//printf("����ʱ�䴦������ҵ�񡣡���\n");

		return true;
	}

	//�Ƿ�����
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

private:

};

#endif  //!_EasyTcpServer_hpp_