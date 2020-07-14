#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

//信号量	互斥锁
class CellSemaphore
{
public:
	CellSemaphore()
	{
		m_isWaitExit = false;
		m_waitCount = 0;
		m_wakeUpCount = 0;
	}
	~CellSemaphore(){}

	//等待、阻塞当前线程
	void Wait()
	{
		//--------------简陋的信号量实现--------------//
		//多线程操作m_isWaitExit,不安全，但又不能加锁，Wait中一直循环，锁得不到释放
		//用while循环，对资源消耗又大，不理想

		//m_isWaitExit = true;
		////阻塞等待退出信号
		//while (m_isWaitExit)
		//{
		//	std::chrono::milliseconds t(1);
		//	std::this_thread::sleep_for(t);
		//}

		//--------------条件变量与互斥锁--------------//
	
		std::unique_lock<std::mutex> lock(m_mutex);		//有lock()和unlock()成员函数,lock_guard<>的升级版
		if (--m_waitCount < 0)
		{
			//阻塞等待-->阻塞时,自动释放锁权限,以便其他线程能有机会获得锁
			m_cv.wait(lock, [this]() -> bool {
				return m_wakeUpCount > 0;
			});	//退出等待时，判断条件是否满足

			--m_wakeUpCount;	//成功唤醒，唤醒计数减一
		}
		

	}

	//唤醒当前线程
	void WakeUp()
	{
		//--------------简陋的信号量实现--------------//

		//if (m_isWaitExit)
		//{
		//	m_isWaitExit = false;
		//}
		//else
		//{
		//	printf("CellSemaphore wakeup error.\n");
		//}

		//--------------条件变量与互斥锁--------------//

		std::lock_guard<std::mutex> lock(m_mutex);
		if (++m_waitCount <= 0)
		{
			++m_wakeUpCount;	//准备唤醒，唤醒计数加一
			m_cv.notify_one();
		}
		
	}

private:
	//判断是否阻塞(简陋信号量用)
	bool m_isWaitExit;

	//锁
	std::mutex m_mutex;
	//阻塞等待--条件变量
	std::condition_variable m_cv;
	//等待计数
	int m_waitCount;
	//唤醒计数
	int m_wakeUpCount;
};

#endif	//	!_CELL_SEMAPHORE_HPP_