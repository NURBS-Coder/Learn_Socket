#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_

#include "CellHeader.hpp"

//�ͻ������Ͷ���
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf,0,RECV_BUFF_SIZE * 5);
		_lastPos = 0;

		memset(_szSendBuf,0,SEND_BUFF_SIZE * 5);
		_lastSendPos = 0;
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

private:
	SOCKET _sockfd;							//�ͻ���socket
	char _szMsgBuf[RECV_BUFF_SIZE * 5];		//�ڶ�����������Ϣ������
	int _lastPos;							//��¼�ڶ�������������λ��

	char _szSendBuf[RECV_BUFF_SIZE * 5];		//�ڶ������������ͻ�����
	int _lastSendPos;							//��¼�ڶ�������������λ��

};

typedef std::shared_ptr<CellClient> CellClientPtr;

#endif //	!_CELLCLIENT_HPP_