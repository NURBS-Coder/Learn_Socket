#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

#include "CellClient.hpp"

#include <functional>

#include "CellThread.hpp"

//任务基类 --> 用lambda表达式升级
//class CellTask
//{
//public:
//	CellTask(){}
//	//虚析构函数
//	virtual ~CellTask(){}
//	//执行任务
//	virtual void DoTask()
//	{
//		
//	}
//
//private:
//
//};

//typedef std::shared_ptr<CellTask>  CellTaskPtr;

//执行任务的服务类型
class CellTaskServer
{
	typedef std::function<void()> CellTask;		//一个函数变量
public:
	CellTaskServer(){

	}

	~CellTaskServer(){}

	//添加任务
	void AddTask(CellTask task)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_tasksBuf.push_back(task);
	}

	//启动服务
	void Start()
	{
		//线程
		m_thread.Start(nullptr,[this](CellThread& pThread){OnRun(pThread);});
	}

	//关闭服务
	void Close()
	{
		printf("3、TaskServer<%d>.Close	Start...\n", m_id);
		m_thread.Close();
		printf("3、TaskServer<%d>.Close	End...\n", m_id);
	}

	//工作函数
	void OnRun(CellThread& pThread)
	{
		while (pThread.IsRun())
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
				(*iter)();
				iter = m_tasks.erase(iter);
			}
		}

		//处理缓冲队列中的任务
		for (auto pTask : m_tasksBuf)
		{
			pTask();
		}

		printf("3、TaskServer<%d>.OnRun  Close...\n", m_id);
	}
private:
	//任务数据
	std::list<CellTask> m_tasks;
	//任务数据缓冲区
	std::list<CellTask> m_tasksBuf;
	//锁,用于对临界数据【数据缓冲区】进行操作
	std::mutex m_mutex;

public:
	CellThread m_thread;
	int m_id;
};


//网络消息发送任务 --> 用lambda表达式升级,就不需要了
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
//	//执行任务
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