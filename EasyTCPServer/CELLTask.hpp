#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

#include <thread>
#include <mutex>
#include <list>
#include <memory>

class CellTask;

typedef std::shared_ptr<CellTask>  CellTaskPtr;

//任务基类
class CellTask
{
public:
	CellTask(){}
	//虚析构函数
	virtual ~CellTask(){}
	//执行任务
	virtual void DoTask()
	{
		
	}

private:

};

//执行任务的服务类型
class CellTaskServer
{
public:
	CellTaskServer(){}

	~CellTaskServer(){}

	//添加任务
	void AddTask(CellTaskPtr& task)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tasksBuf.push_back(task);
	}

	//启动服务
	void Start()
	{
		//线程
		std::thread thread(std::mem_fn(&CellTaskServer::OnRun), this);
		thread.detach();
	}

	//工作函数
	void OnRun()
	{
		while (true)
		{
			//添加任务到任务列表
			if (!m_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto ptask : m_tasksBuf)
				{
					m_tasks.push_back(ptask);
				}
				m_tasksBuf.clear();
			}

			//如果没有任务
			if (m_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			//处理任务,处理完就删除
			auto iter = m_tasks.begin();
			while (iter != m_tasks.end())
			{
				(*iter)->DoTask();
				iter = m_tasks.erase(iter);
			}

		}
	}
private:
	//任务数据
	std::list<CellTaskPtr> m_tasks;
	//任务数据缓冲区
	std::list<CellTaskPtr> m_tasksBuf;
	//锁,用于对临界数据【数据缓冲区】进行操作
	std::mutex m_mutex;
};

#endif //!_CELL_TASK_HPP_