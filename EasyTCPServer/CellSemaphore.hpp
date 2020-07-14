#ifndef _CELL_SEMAPHORE_HPP_
#define _CELL_SEMAPHORE_HPP_

#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>

//�ź���	������
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

	//�ȴ���������ǰ�߳�
	void Wait()
	{
		//--------------��ª���ź���ʵ��--------------//
		//���̲߳���m_isWaitExit,����ȫ�����ֲ��ܼ�����Wait��һֱѭ�������ò����ͷ�
		//��whileѭ��������Դ�����ִ󣬲�����

		//m_isWaitExit = true;
		////�����ȴ��˳��ź�
		//while (m_isWaitExit)
		//{
		//	std::chrono::milliseconds t(1);
		//	std::this_thread::sleep_for(t);
		//}

		//--------------���������뻥����--------------//
	
		std::unique_lock<std::mutex> lock(m_mutex);		//��lock()��unlock()��Ա����,lock_guard<>��������
		if (--m_waitCount < 0)
		{
			//�����ȴ�-->����ʱ,�Զ��ͷ���Ȩ��,�Ա������߳����л�������
			m_cv.wait(lock, [this]() -> bool {
				return m_wakeUpCount > 0;
			});	//�˳��ȴ�ʱ���ж������Ƿ�����

			--m_wakeUpCount;	//�ɹ����ѣ����Ѽ�����һ
		}
		

	}

	//���ѵ�ǰ�߳�
	void WakeUp()
	{
		//--------------��ª���ź���ʵ��--------------//

		//if (m_isWaitExit)
		//{
		//	m_isWaitExit = false;
		//}
		//else
		//{
		//	printf("CellSemaphore wakeup error.\n");
		//}

		//--------------���������뻥����--------------//

		std::lock_guard<std::mutex> lock(m_mutex);
		if (++m_waitCount <= 0)
		{
			++m_wakeUpCount;	//׼�����ѣ����Ѽ�����һ
			m_cv.notify_one();
		}
		
	}

private:
	//�ж��Ƿ�����(��ª�ź�����)
	bool m_isWaitExit;

	//��
	std::mutex m_mutex;
	//�����ȴ�--��������
	std::condition_variable m_cv;
	//�ȴ�����
	int m_waitCount;
	//���Ѽ���
	int m_wakeUpCount;
};

#endif	//	!_CELL_SEMAPHORE_HPP_