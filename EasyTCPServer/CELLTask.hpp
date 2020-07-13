#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

#include "CellClient.hpp"

//�������
class CellTask
{
public:
	CellTask(){}
	//����������
	virtual ~CellTask(){}
	//ִ������
	virtual void DoTask()
	{
		
	}

private:

};

typedef std::shared_ptr<CellTask>  CellTaskPtr;

//ִ������ķ�������
class CellTaskServer
{
public:
	CellTaskServer(){}

	~CellTaskServer(){}

	//�������
	void AddTask(CellTaskPtr& task)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tasksBuf.push_back(task);
	}

	//��������
	void Start()
	{
		//�߳�
		std::thread thread(std::mem_fn(&CellTaskServer::OnRun), this);
		thread.detach();
	}

	//��������
	void OnRun()
	{
		while (true)
		{
			//������������б�
			if (!m_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto ptask : m_tasksBuf)
				{
					m_tasks.push_back(ptask);
				}
				m_tasksBuf.clear();
			}

			//���û������
			if (m_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			//��������,�������ɾ��
			auto iter = m_tasks.begin();
			while (iter != m_tasks.end())
			{
				(*iter)->DoTask();
				iter = m_tasks.erase(iter);
			}

		}
	}
private:
	//��������
	std::list<CellTaskPtr> m_tasks;
	//�������ݻ�����
	std::list<CellTaskPtr> m_tasksBuf;
	//��,���ڶ��ٽ����ݡ����ݻ����������в���
	std::mutex m_mutex;
};


//������Ϣ��������
class CellSendMsg2ClientTask  : public CellTask
{
public:
	CellSendMsg2ClientTask(CellClientPtr& pClient, DataHeader *pHeader)
	{
		m_pClient = pClient;
		m_pHeader = pHeader;
	}

	~CellSendMsg2ClientTask(){}

	//ִ������
	virtual void DoTask()
	{
		m_pClient->SendData(m_pHeader);
		delete m_pHeader;
	}

private:
	CellClientPtr m_pClient;
	DataHeader *m_pHeader;
};

typedef std::shared_ptr<CellSendMsg2ClientTask>  CellSendMsg2ClientTaskPtr;

#endif //!_CELL_TASK_HPP_