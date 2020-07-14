#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CellSemaphore.hpp"

class CellThread
{
private:
	//类模板std :: function是一个通用的多态函数包装器
	//其实例可以存储，复制和调用任何可调用的目标 ：
	//1）函数
	//2）lambda表达式
	//3）绑定表达式或其他函数对象
	//4）指向成员函数和指向数据成员的指针
	typedef std::function<void(CellThread&)> EventCall;	//事件回调函数对象

public:
	CellThread()
	{
		m_isRun = false;
		EventCall m_onCreate = nullptr;
		EventCall m_onRun = nullptr;		
		EventCall m_onDestory = nullptr;	
	}

	~CellThread()
	{

	}

	//启动线程
	void Start(
		EventCall onCreate = nullptr, 
		EventCall onRun = nullptr, 
		EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_isRun)
		{
			//注册三个事件回调函数
			if (nullptr != onCreate)
			{
				m_onCreate = onCreate;
			}

			if (nullptr != onRun)
			{
				m_onRun = onRun;
			}

			if (nullptr != onDestory)
			{
				m_onDestory = onDestory;
			}

			//启动线程函数
			m_isRun = true;
			std::thread thread(std::mem_fn(&CellThread::OnWork), this);
			//很多C++的高手和牛人们都会给我们一个忠告，那就是：在处理STL里面的容器的时候，尽量不要自己写循环。
			//原因：【STL里面的容器都有自己的遍历器，而且STL提供了for_each等函数】
			//效率：算法通常比程序员产生的循环更高效。 
			//正确性：写循环时比调用算法更容易产生错误。
			//可维护性：算法通常使代码比相应的显式循环更干净、更直观。

			//函数适配器std::mem_fun,std::mem_fun_ref,std::mem_fn()
			//mem_fun将成员函数转换为函数对象(指针版本)。指针版本指的是通过指向对象的指针调用对象的情况。std::mem_fun_ref是对应的引用版本。
			//成员函数指针与普通函数指针不同，它必须绑定到特定的对象上才能使用，所以成员函数指针不是一个可调用对象。
			//当在使用一些需要可调用对象的函数时。如std::fot_each，就需要进行成员函数到函数对象的转换
			thread.detach();
		}
	}

	//关闭线程（其他线程关闭本线程）
	void Close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_isRun)
		{
			//退出OnRun
			m_isRun = false;
			//等待OnRun退出完毕（信号量）
			m_sem.Wait();
		}
	}

	//关闭线程（本线程中自己关闭）
	//不需要使用信号量进行阻塞等待
	void Exit()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_isRun)
		{
			//退出OnRun
			m_isRun = false;
		}
	}

	//返回线程运行状态
	bool IsRun()
	{
		return m_isRun;
	}

protected:
	//线程工作函数
	void OnWork()
	{
		//创建线程
		if (nullptr != m_onCreate)
		{
			m_onCreate(*this);
		}
		//运行线程
		if (nullptr != m_onRun)
		{
			m_onRun(*this);
		}
		//销毁线程
		if (nullptr != m_onDestory)
		{
			m_onDestory(*this);
		}

		//全部结束后，唤醒信号量，完成Close操作
		m_sem.WakeUp();
	}

private:
	EventCall m_onCreate;	//创建线程事件回调函数
	EventCall m_onRun;		//运行线程事件回调函数
	EventCall m_onDestory;	//销毁线程事件回调函数

	bool m_isRun;			//线程是否在运行中
	CellSemaphore m_sem;	//控制线程的终止与退出
	std::mutex m_mutex;		//锁,防止多个线程启动或关闭本线程
};


//使用方式：
//1） #include "CellThread.hpp"
//2） CellThread m_thread;
//3） m_thread.Start(nullptr,[this](CellThread& pThread){OnRun(pThread);});
//4） void OnRun(CellThread& pThread)
//    {
//	     while (pThread.IsRun())
//	     {
//		    //工作内容
//	     }
//    }
//5） m_thread.Close();

#endif	//	!_CELL_THREAD_HPP_