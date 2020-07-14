#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "CellTask.hpp"
#include "INetEvent.hpp"

class CellServer
{
private:
	std::vector<CellClientPtr> _clients;		//��̬���飺��ͻ���socket,��ָ�룬�ڶ��ڴ棬��ֹջ�ڴ治��
	std::vector<CellClientPtr> _clientsBuff;	//�ͻ��������
	char _szRecv[RECV_BUFF_SIZE];				//���ջ�����
	
	std::mutex _mutex;							//������е���
	INetEvent *_pNetEvent;						//�����¼�����
	CellTaskServer m_taskServer;				//����ִ���߳�
	CELLTimestamp m_time;						//��ʱʱ���
	CellThread m_thread;						//�����ߴ����߳�
	fd_set _fdRead_bak;							//feset���ݣ�����ֱ�Ӹ�ֵ
	bool _clients_change;						//�ͻ��б��Ƿ�仯
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

	//��ӷ�������
	void addSendTask(CellClientPtr &pClient, DataHeader *pHeader)
	{
		//lambda���ʽ
		m_taskServer.AddTask([pClient, pHeader](){
			pClient->SendData(pHeader);
			delete pHeader;
		});
	}

	//������������ӿͻ���
	void addClientToBuff(CellClientPtr& pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex); //ʹ���Խ�������ֹ���ʧ���˳���û�н���
		_clientsBuff.push_back(pClient);
	}

	//�������������߳�
	void Start()
	{
		m_thread.Start(nullptr, [this](CellThread& pThread){OnRun(pThread);});
		m_taskServer.Start();
	}

	//�رո��������߳�
	void Close()
	{
		printf("2��CellServer<%d>.Close	Start...\n", m_id);

		m_taskServer.Close();
		m_thread.Close();

		//��տͻ��ˣ�����ʱ�͹رտͻ���socket
		_clients.clear();
		_clientsBuff.clear();

		printf("2��CellServer<%d>.Close	End...\n", m_id);
	}

	//��ѯ��ǰ�û�����
	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
		
	//����������Ϣ
	void OnRun(CellThread& pThread)
	{
		_clients_change = true;
		while (pThread.IsRun())
		{
			//�ӻ��������ӵ���ʽ����
			if (_clientsBuff.size() > 0)	//����sizeû�м���
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

			//û�пͻ��˾�����
			if (_clients.empty()){		//C++11 �Ŀ�ƽ̨����
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				m_time.reset();
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

			if (_clients_change)
			{
				for (size_t i = 0; i < _clients.size(); i++)	//ÿ��ѭ����Ҫ��һ��_clients.size()�����жϣ���
				{
					//���ͻ���socket����ɶ�����
					FD_SET(_clients[i]->sockfd(), &fdRead);
				}
				memcpy(&_fdRead_bak,&fdRead,sizeof(fd_set));
				_clients_change = false;
			}
			else
			{
				memcpy(&fdRead,&_fdRead_bak,sizeof(fd_set));
			}
			

			//�ȴ��������пɲ���Socket��������ģʽ,timevalû�н���ͷ��أ�����ִ��
			//ʹ�õ��߳�Ҳ�������޿ͻ�����Ϣ����ʱ������������������
			timeval t = {0,1};	
			int ret = select(0,&fdRead,nullptr,nullptr, /*nullptr*/&t);  //��ѯ��fdset�ڵ����ݻ�ı䣬ֻ������Ϣ��socket������
			if (ret < 0)
			{
				printf("CellServer<%d>.OnRun.Select���������\n", m_id);
				pThread.Exit();
				break;
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
							_clients.erase(iter);
							_clients_change = true;

						}
					}
				}
			}

			//�������
			auto time = m_time.elapsed();
			for (auto iter = _clients.begin(); iter != _clients.end();)
			{
				//��ʱ��������
				(*iter)->checkSend(time);
				//�������
				if (!(*iter)->checkHeart(time))
				{
					//�ͻ����˳���ɾ��Socket
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

			//����ʱ�䴦������ҵ��
			//printf("����ʱ�䴦������ҵ�񡣡���\n");
		}
		printf("2��CellServer<%d>.OnRun  Close...\n", m_id);
	}

	//�������� (����ճ���������)
	int RecvData(CellClientPtr& pClient)
	{
		//���հ�ͷ
		int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		_pNetEvent->OnNetRecv(pClient);
		
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
	void OnNetMsg(CellClientPtr& pClient, DataHeader* header)
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
				LoginResult *ret = new LoginResult();
				//�������ݰ�����ͷ+���壩
				//pClient->SendData(&ret);
				addSendTask(pClient, ret);
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
		case CMD_HEART_C2S:
			{	//������������
				pClient->resetDTHeart();
				Heart_S2C *ret = new Heart_S2C();
				addSendTask(pClient, ret);
				//printf("�յ�<%d>���������ݣ�time=%d,�Ѿ�����...\n",pClient->sockfd(), pClient->m_dtHeart);
			}
			break;
		default:
			{
				//���Ͱ�ͷ
				//DataHeader ret;
				//SendData(cSock, &ret);
				printf("socket = <%d>�յ�δ������Ϣ\t���ݳ��ȣ�%d", pClient->sockfd(), header->dataLength);
			}
			break;
		}
	}

private:

};

typedef std::shared_ptr<CellServer> CellServerPtr;

#endif	//	!_CELL_SERVER_HPP_