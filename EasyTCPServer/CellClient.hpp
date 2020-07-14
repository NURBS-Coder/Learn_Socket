#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_

#include "CellHeader.hpp"

//�ͻ������Ͷ���
class CellClient
{
public:
	int m_id;
	int m_serverID;
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf,0,RECV_BUFF_SIZE * 5);
		_lastPos = 0;

		memset(_szSendBuf,0,SEND_BUFF_SIZE * 5);
		_lastSendPos = 0;

		m_dtHeart = 0;
		m_dtSend = 0;

		static int n = 1;
		m_id = n++;
		m_serverID = 0;
	}

	~CellClient()
	{
		printf("4��CellClient[%d]<%d>.Close...\n",m_serverID, m_id);
		if (_sockfd != INVALID_SOCKET)
		{
			closesocket(_sockfd);
		}
		_sockfd = INVALID_SOCKET;	
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	char* msgBuf()
	{
		return _szMsgBuf;
	}

	char* sendBuf()
	{
		return _szSendBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}

	int getLastSendPos()
	{
		return _lastSendPos;
	}

	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

	void setLastSendPos(int pos)
	{
		_lastSendPos = pos;
	}
	
	//������������
	int SendDataReal()
	{
		int ret = SOCKET_ERROR;
		if (_lastSendPos > 0)
		{
			//�����ͻ��������ݷ���ȥ
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			//����������λ������
			_lastSendPos = 0;
			//����ʱ��
			resetDTSend();
		}
		return ret;
	}

	//��������
	int SendData(DataHeader* header)
	{
		if (_sockfd == INVALID_SOCKET)
		{
			return -1;
		}

		int ret = SOCKET_ERROR;
		//�����������ݣ��������ƥ��,��Ϣ����
		int nSendLen = header->dataLength;					//�������ݵĳ���
		const char *pSendData = (const char*)header;		//Ҫ���͵�����

		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
			{
				//���㷢�ͻ������л��еĿռ�
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				//��������
				memcpy(_szSendBuf+_lastSendPos, pSendData, nCopyLen);
				//�ƶ�����δ�����Ĵ����͵�����λ��
				pSendData += nCopyLen;
				//���㻹δ���������ݴ�С
				nSendLen -= nCopyLen;
				//�����ͻ��������ݷ���ȥ
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				//����������λ������
				_lastSendPos = 0;
				//����ʱ��
				resetDTSend();
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				//��ʣ�µ����ݿ��뻺����
				memcpy(_szSendBuf+_lastSendPos, pSendData, nSendLen);
				_lastSendPos += nSendLen;
				ret = 1;
				break;
			}
		}
		return ret;
	}

	void resetDTHeart()
	{
		m_dtHeart = 0;
	}

	void resetDTSend()
	{
		m_dtSend = 0;
	}

	//�������
	bool checkHeart(time_t dt)
	{
		m_dtHeart += dt;
		if (m_dtHeart >= 5000)		//5s����������ʱ��
		{
			printf("checkHeart dead:s=%d\n", _sockfd);
			return false;
		}
		//printf("���<%d>��������time=%d.....\n",_sockfd, m_dtHeart);
		return true;
	}

	//���ͼ�⣺��շ��ͻ������������ݷ��ͳ�ȥ
	bool checkSend(time_t dt)
	{
		m_dtSend += dt;
		if (m_dtSend >= 1000)		//500ms�ķ��ͼ���ʱ��
		{
			SendDataReal();

			//printf("<%d>checkSend,Succeed....\n", _sockfd);
			return false;
		}
		//printf("���<%d>��������time=%d.....\n",_sockfd, m_dtHeart);
		return true;
	}

private:
	SOCKET _sockfd;							//�ͻ���socket
	char _szMsgBuf[RECV_BUFF_SIZE * 5];		//�ڶ�����������Ϣ������
	int _lastPos;							//��¼�ڶ�������������λ��

	char _szSendBuf[RECV_BUFF_SIZE * 5];		//�ڶ������������ͻ�����
	int _lastSendPos;							//��¼�ڶ�������������λ��

	time_t m_dtHeart;						//������ʱ
	time_t m_dtSend;						//���ͼ�ʱ

};

typedef std::shared_ptr<CellClient> CellClientPtr;

#endif //	!_CELLCLIENT_HPP_