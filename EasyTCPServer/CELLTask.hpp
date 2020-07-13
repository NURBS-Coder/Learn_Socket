#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

#include "CellClient.hpp"

#include <functional>

//������� --> ��lambda���ʽ����
//class CellTask
//{
//public:
//	CellTask(){}
//	//����������
//	virtual ~CellTask(){}
//	//ִ������
//	virtual void DoTask()
//	{
//		
//	}
//
//private:
//
//};

//typedef std::shared_ptr<CellTask>  CellTaskPtr;

//ִ������ķ�������
class CellTaskServer
{
	typedef std::function<void()> CellTask;		//һ����������
public:
	CellTaskServer(){}

	~CellTaskServer(){}

	//�������
	void AddTask(CellTask task)
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
				(*iter)();
				iter = m_tasks.erase(iter);
			}

		}
	}
private:
	//��������
	std::list<CellTask> m_tasks;
	//�������ݻ�����
	std::list<CellTask> m_tasksBuf;
	//��,���ڶ��ٽ����ݡ����ݻ����������в���
	std::mutex m_mutex;
};


//������Ϣ�������� --> ��lambda���ʽ����,�Ͳ���Ҫ��
//class CellSendMsg2ClientTask  : public CellTask
//{
//public:
//	CellSendMsg2ClientTask(CellClientPtr& pClient, DataHeader *pHeader)
//	{
//		m_pClient = pClient;
//		m_pHeader = pHeader;
//	}
//
//	~CellSendMsg2ClientTask(){}
//
//	//ִ������
//	virtual void DoTask()
//	{
//		m_pClient->SendData(m_pHeader);
//		delete m_pHeader;
//	}
//
//private:
//	CellClientPtr m_pClient;
//	DataHeader *m_pHeader;
//};
//
//typedef std::shared_ptr<CellSendMsg2ClientTask>  CellSendMsg2ClientTaskPtr;

#endif //!_CELL_TASK_HPP_