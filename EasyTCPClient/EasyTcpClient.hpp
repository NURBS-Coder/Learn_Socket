#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ���� WIN32_LEAN_AND_MEAN ��
	#pragma comment(lib,"ws2_32.lib")	//���Ӷ�̬��
#else

#endif // _WIN32

#include <stdio.h>				//printf()....

using namespace std;
#include <iostream>				//std::cout....

#include "MessageHeader.hpp"

class EasyTcpClient
{
private:
	SOCKET _sock;				//������socket
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	//����������
	virtual ~EasyTcpClient()	//��ֹ�ڴ�й¶,ָ�붨����󣬿����ͷŵ�����
	{
		Close();
	}

public:
	//��ʼ��socket
	int InitSocket()
	{
#ifdef _WIN32
		//����WinSock2.x���绷��
		WORD ver = MAKEWORD(2,2);	//�����汾��
		WSADATA dat;
		WSAStartup(ver, &dat);		//����socket��ض�̬���ӿ�
#endif
		//---------------------
		//-- ��Socket API��������TCP�ͻ���
		if (_sock != INVALID_SOCKET)
		{
			printf("�ر�socket = <%d>������...\n", _sock);
			closesocket(_sock);
			_sock = INVALID_SOCKET;
		}
		// 1.����һ��socket
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(SOCKET_ERROR == _sock)
		{
			printf("����,����socketʧ��...\n");
			return -1;
		}
		printf("����socket = <%d>�ɹ�...\n", _sock);
		return 0;
	}

	//���ӷ�����
	int Connect(char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2.���ӷ����� connect
		printf("socket = <%d>�������ӷ�����<%s:%d>...\n", _sock, ip, port);
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
			printf("����,socket = <%d>���ӷ�����<%s:%d>ʧ��...\n", _sock, ip, port);
		}
		else
		{
			printf("socket = <%d>���ӷ�����<%s:%d>�ɹ�...\n", _sock, ip, port);
		}
		return ret;
	}

	//�رշ�����
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			// 7.�ر��׽��� closesocket
			closesocket(_sock);
			//�ر�WinSock2.x���绷��
			WSACleanup();
#else

#endif
			_sock = INVALID_SOCKET;
		}
	}

	//��������
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//�������� (����ճ���������)
	int RecvData()
	{
		//// 6.recv ���շ��������ص�����
		////���ջ�����
		//char szRecv[1024] = {};
		////���հ�ͷ
		//int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
		//DataHeader* header = (DataHeader*)szRecv;
		//if (nLen <= 0)
		//{
		//	printf("socket = <%d>��������Ͽ����ӣ����������\n",_sock);
		//	return -1;
		//}
		////���հ��壺ǰ���չ�һ��DataHeader�ĳ��ȣ�����������ƫ��
		//recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		//OnNetMsg(header);
		//return 0;

		// 6.recv ���շ��������ص�����
		//���ջ�����
		char _szRecv[1024] = {};
		char _szMsgBuf[10240] = {};
		int _lastPos;
		//��������
		int nLen = recv(_sock, _szRecv, 1024, 0);
		if (nLen <= 0)
		{
			printf("socket = <%d>��������Ͽ����ӣ����������\n",_sock);
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(_szMsgBuf+_lastPos,_szRecv, nLen);
		//��Ϣ����������λ�ú���
		_lastPos += nLen;
		//�ж����ݳ��ȴ�������ͷ
		while (_lastPos >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)_szRecv;
			//�ж���Ϣ��������С������Ϣ����
			if (_lastPos >= header->dataLength)
			{
				//��Ϣ������ʣ�����ݳ���
				int nSize = _lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(header);
				//����Ϣ������δ����������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf+header->dataLength, nSize);
				//��Ϣ����������λ�ú���
				_lastPos = nSize;
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
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
			{
				LoginResult* loginRet = (LoginResult*)header;		//����ṹ��ֱ���ɻ���ת��
				printf("socket = <%d>�յ���������Ϣ��CMD_LOGIN_RESULT\t���ݳ��ȣ�%d", _sock, loginRet->dataLength);
				//���صĵ�½���
				printf(" --> LoginResult��%d\n",loginRet->result);
			}
			break;
		case CMD_LOGOUT_RESULT:
			{
				LogoutResult *logoutRet = (LogoutResult*)header;	//����ṹ��ֱ���ɻ���ת��
				printf("socket = <%d>�յ���������Ϣ��CMD_LOGOUT_RESULT\t���ݳ��ȣ�%d", _sock, logoutRet->dataLength);
				//���صĵ�½���
				printf(" --> LogoutResult��%d\n",logoutRet->result);
			}
			break;
		case CMD_NEW_USER_JOIN:
			{
				NewUserJoin *newUserJoinRet = (NewUserJoin*)header;	//����ṹ��ֱ���ɻ���ת��
				printf("socket = <%d>�յ���������Ϣ��CMD_NEW_USER_JOIN\t���ݳ��ȣ�%d", _sock, newUserJoinRet->dataLength);
				//���صĵ�½���
				printf(" --> NewUserJoin��<newSocket = %d>\n",newUserJoinRet->sock);
			}
			break;
		}
	}

	//��ѯ������Ϣ
	bool OnRun()
	{
		if (!isRun())
		{
			return false;
		}

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		//�ȴ��������пɲ���Socket��������ģʽ,timevalû�н���ͷ��أ�����ִ��
		//ʹ�õ��߳�Ҳ�����������Է���������Ϣ����ʱ������������������
		timeval t = {1,0};	
		int ret = select(0,&fdReads,0,0,&t);
		if (ret < 0)
		{
			printf("socket = <%d>select�������....\n",_sock);
			Close();
			return false;
		}
		//��ѯ�ɶ���������û�пͻ���Socket
		if (FD_ISSET(_sock,&fdReads))	
		{
			FD_CLR(_sock,&fdReads);
			if(-1 == RecvData())
			{
				printf("socket = <%d>select�������....\n",_sock);
				Close();
				return false;
			}
		}

		//����ʱ�䴦������ҵ��
		//printf("����ʱ�䴦������ҵ�񡣡���\n");

		return true;
	}

	//�ж��Ƿ����
	bool isRun()
	{
		return INVALID_SOCKET != _sock;
	}

private:

};

#endif  //_EasyTcpClient_hpp_