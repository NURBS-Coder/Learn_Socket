#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

#include <thread>
#include <mutex>
#include <list>

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

//ִ������ķ�������
class CellTaskServer
{
public:
	CellTaskServer(){}

	~CellTaskServer(){}

	//�������
	void AddTask(CellTask *task)
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
				delete *iter;
				iter = m_tasks.erase(iter);
			}

		}
	}
private:
	//��������
	std::list<CellTask*> m_tasks;
	//�������ݻ�����
	std::list<CellTask*> m_tasksBuf;
	//��,���ڶ��ٽ����ݡ����ݻ����������в���
	std::mutex m_mutex;
};

#endif //!_CELL_TASK_HPP_