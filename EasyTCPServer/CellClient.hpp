#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_

#include "CellHeader.hpp"

//客户端类型定义
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

	//发送数据
	int SendData(DataHeader* header)
	{
		if (_sockfd == INVALID_SOCKET)
		{
			return -1;
		}

		int ret = SOCKET_ERROR;
		//定量发送数据，与接收相匹配,消息缓冲
		int nSendLen = header->dataLength;					//发送数据的长度
		const char *pSendData = (const char*)header;		//要发送的数据

		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
			{
				//计算发送缓冲区中还有的空间
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				//拷贝数据
				memcpy(_szSendBuf+_lastSendPos, pSendData, nCopyLen);
				//移动到还未拷贝的待发送的数据位置
				pSendData += nCopyLen;
				//计算还未拷贝的数据大小
				nSendLen -= nCopyLen;
				//将发送缓冲区数据发出去
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				//缓冲区数据位置置零
				_lastSendPos = 0;
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				//将剩下的数据拷入缓冲区
				memcpy(_szSendBuf+_lastSendPos, pSendData, nSendLen);
				_lastSendPos += nSendLen;
				ret = 1;
				break;
			}
		}
		return ret;
	}

private:
	SOCKET _sockfd;							//客户端socket
	char _szMsgBuf[RECV_BUFF_SIZE * 5];		//第二缓冲区、消息缓冲区
	int _lastPos;							//记录第二缓冲区中数据位置

	char _szSendBuf[RECV_BUFF_SIZE * 5];		//第二缓冲区、发送缓冲区
	int _lastSendPos;							//记录第二缓冲区中数据位置

};

typedef std::shared_ptr<CellClient> CellClientPtr;

#endif //	!_CELLCLIENT_HPP_