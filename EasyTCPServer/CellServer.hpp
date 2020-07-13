#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "CellTask.hpp"
#include "INetEvent.hpp"

class CellServer
{
private:
	SOCKET _sock;						//������socket	
	std::vector<CellClientPtr> _clients;		//��̬���飺��ͻ���socket,��ָ�룬�ڶ��ڴ棬��ֹջ�ڴ治��
	std::vector<CellClientPtr> _clientsBuff;	//�ͻ��������
	char _szRecv[RECV_BUFF_SIZE];		//���ջ�����
	
	std::thread _thread;						//�����ߴ����߳�
	std::mutex _mutex;						//������е���
	INetEvent *_pNetEvent;				//�����¼�����

	CellTaskServer m_taskServer;		//����ִ���߳�

	CELLTimestamp m_time;

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

	//��ӷ�������
	void addSendTask(CellClientPtr &pClient, DataHeader *pHeader)
	{
		//CellSendMsg2ClientTaskPtr pCellTask = std::make_shared<CellSendMsg2ClientTask>(pClient,pHeader);
		//m_taskServer.AddTask((CellTaskPtr)pCellTask);

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
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	//�������������߳�
	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
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

		m_taskServer.Start();
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
			}
			
#else

#endif
			_clients.clear();
			_clientsBuff.clear();
		}
	}
		
	//����������Ϣ
	//feset���ݣ�����ֱ�Ӹ�ֵ
	fd_set _fdRead_bak;
	//�ͻ��б��Ƿ�仯
	bool _clients_change;
	bool OnRun()
	{
		_clients_change = true;
		_bIsRun = true;
		while (_bIsRun)
		{
			

			//�ӻ��������ӵ���ʽ����
			if (_clientsBuff.size() > 0)	//����sizeû�м���
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
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
							closesocket((*iter)->sockfd());
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
					closesocket((*iter)->sockfd());
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
		printf("�˳�һ�������ߴ����߳�....\n");
		return true;
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
				printf("socket = <%d>�յ�δ������Ϣ\t���ݳ��ȣ�%d", _sock, header->dataLength);
			}
			break;
		}
	}

private:

};

typedef std::shared_ptr<CellServer> CellServerPtr;

#endif	//	!_CELL_SERVER_HPP_