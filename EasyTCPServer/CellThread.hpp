#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CellSemaphore.hpp"

class CellThread
{
private:
	//��ģ��std :: function��һ��ͨ�õĶ�̬������װ��
	//��ʵ�����Դ洢�����ƺ͵����κοɵ��õ�Ŀ�� ��
	//1������
	//2��lambda���ʽ
	//3���󶨱��ʽ��������������
	//4��ָ���Ա������ָ�����ݳ�Ա��ָ��
	typedef std::function<void(CellThread&)> EventCall;	//�¼��ص���������

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

	//�����߳�
	void Start(
		EventCall onCreate = nullptr, 
		EventCall onRun = nullptr, 
		EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_isRun)
		{
			//ע�������¼��ص�����
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

			//�����̺߳���
			m_isRun = true;
			std::thread thread(std::mem_fn(&CellThread::OnWork), this);
			//�ܶ�C++�ĸ��ֺ�ţ���Ƕ��������һ���Ҹ棬�Ǿ��ǣ��ڴ���STL�����������ʱ�򣬾�����Ҫ�Լ�дѭ����
			//ԭ�򣺡�STL��������������Լ��ı�����������STL�ṩ��for_each�Ⱥ�����
			//Ч�ʣ��㷨ͨ���ȳ���Ա������ѭ������Ч�� 
			//��ȷ�ԣ�дѭ��ʱ�ȵ����㷨�����ײ�������
			//��ά���ԣ��㷨ͨ��ʹ�������Ӧ����ʽѭ�����ɾ�����ֱ�ۡ�

			//����������std::mem_fun,std::mem_fun_ref,std::mem_fn()
			//mem_fun����Ա����ת��Ϊ��������(ָ��汾)��ָ��汾ָ����ͨ��ָ������ָ����ö���������std::mem_fun_ref�Ƕ�Ӧ�����ð汾��
			//��Ա����ָ������ͨ����ָ�벻ͬ��������󶨵��ض��Ķ����ϲ���ʹ�ã����Գ�Ա����ָ�벻��һ���ɵ��ö���
			//����ʹ��һЩ��Ҫ�ɵ��ö���ĺ���ʱ����std::fot_each������Ҫ���г�Ա���������������ת��
			thread.detach();
		}
	}

	//�ر��̣߳������̹߳رձ��̣߳�
	void Close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_isRun)
		{
			//�˳�OnRun
			m_isRun = false;
			//�ȴ�OnRun�˳���ϣ��ź�����
			m_sem.Wait();
		}
	}

	//�ر��̣߳����߳����Լ��رգ�
	//����Ҫʹ���ź������������ȴ�
	void Exit()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_isRun)
		{
			//�˳�OnRun
			m_isRun = false;
		}
	}

	//�����߳�����״̬
	bool IsRun()
	{
		return m_isRun;
	}

protected:
	//�̹߳�������
	void OnWork()
	{
		//�����߳�
		if (nullptr != m_onCreate)
		{
			m_onCreate(*this);
		}
		//�����߳�
		if (nullptr != m_onRun)
		{
			m_onRun(*this);
		}
		//�����߳�
		if (nullptr != m_onDestory)
		{
			m_onDestory(*this);
		}

		//ȫ�������󣬻����ź��������Close����
		m_sem.WakeUp();
	}

private:
	EventCall m_onCreate;	//�����߳��¼��ص�����
	EventCall m_onRun;		//�����߳��¼��ص�����
	EventCall m_onDestory;	//�����߳��¼��ص�����

	bool m_isRun;			//�߳��Ƿ���������
	CellSemaphore m_sem;	//�����̵߳���ֹ���˳�
	std::mutex m_mutex;		//��,��ֹ����߳�������رձ��߳�
};


//ʹ�÷�ʽ��
//1�� #include "CellThread.hpp"
//2�� CellThread m_thread;
//3�� m_thread.Start(nullptr,[this](CellThread& pThread){OnRun(pThread);});
//4�� void OnRun(CellThread& pThread)
//    {
//	     while (pThread.IsRun())
//	     {
//		    //��������
//	     }
//    }
//5�� m_thread.Close();

#endif	//	!_CELL_THREAD_HPP_