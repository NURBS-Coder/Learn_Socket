#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_

#include "CellHeader.hpp"

//客户端类型定义
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
		printf("4、CellClient[%d]<%d>.Close...\n",m_serverID, m_id);
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
	
	//立即发送数据
	int SendDataReal()
	{
		int ret = SOCKET_ERROR;
		if (_lastSendPos > 0)
		{
			//将发送缓冲区数据发出去
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			//缓冲区数据位置置零
			_lastSendPos = 0;
			//重置时间
			resetDTSend();
		}
		return ret;
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
				//重置时间
				resetDTSend();
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

	void resetDTHeart()
	{
		m_dtHeart = 0;
	}

	void resetDTSend()
	{
		m_dtSend = 0;
	}

	//心跳检测
	bool checkHeart(time_t dt)
	{
		m_dtHeart += dt;
		if (m_dtHeart >= 5000)		//5s的心跳死亡时间
		{
			printf("checkHeart dead:s=%d\n", _sockfd);
			return false;
		}
		//printf("检测<%d>的心跳，time=%d.....\n",_sockfd, m_dtHeart);
		return true;
	}

	//发送检测：清空发送缓冲区，将数据发送出去
	bool checkSend(time_t dt)
	{
		m_dtSend += dt;
		if (m_dtSend >= 1000)		//500ms的发送极限时间
		{
			SendDataReal();

			//printf("<%d>checkSend,Succeed....\n", _sockfd);
			return false;
		}
		//printf("检测<%d>的心跳，time=%d.....\n",_sockfd, m_dtHeart);
		return true;
	}

private:
	SOCKET _sockfd;							//客户端socket
	char _szMsgBuf[RECV_BUFF_SIZE * 5];		//第二缓冲区、消息缓冲区
	int _lastPos;							//记录第二缓冲区中数据位置

	char _szSendBuf[RECV_BUFF_SIZE * 5];		//第二缓冲区、发送缓冲区
	int _lastSendPos;							//记录第二缓冲区中数据位置

	time_t m_dtHeart;						//心跳计时
	time_t m_dtSend;						//发送计时

};

typedef std::shared_ptr<CellClient> CellClientPtr;

#endif //	!_CELLCLIENT_HPP_