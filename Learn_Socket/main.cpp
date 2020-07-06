#define WIN32_LEAN_AND_MEAN
#include "CELLTimestamp.hpp"

#include <WinSock2.h>
#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ�����

#pragma comment(lib,"ws2_32.lib")	//��Ӷ�̬��

#include <iostream>
using namespace std;
#include <thread>		//c++11 �߳�
#include <mutex>		//��
#include <atomic>		//ԭ��

#include <algorithm>
mutex m;
const int tCount = 4;
//int sum = 0;
atomic<int> sum = 0;
void workFun(int index)
{
	
	for (int i = 0; i < 4 ; i++)
	{
		//�����������������ʹ��������ռ��ϵͳ��Դ����ҪƵ��ʹ����������һ��Ҫ����
		m.lock();	//�ٽ�����--��ʼ
		cout << "Hello," << index << "thread." << i <<endl;
		//sum++;
		m.unlock();	//�ٽ�����--����

		//�Խ������뿪��ǰ������ͽ���
		//lock_guard<mutex> lg(m);
		//sum++;

		//ԭ�Ӳ���:�Դ�������ֹ�������߳������
		sum++;
	}
}
//��ռʽռ��ϵͳ��Դ,���߳�ʹ��cout��Դ����������ң���Ҫ����Դ���б���
//ʹ�������Խ���
//ԭ�Ӳ�������������ɷָ�Ĵ���ʱ��С�Ĳ�����λ,���ᱻ�̵߳��Ȼ��ƴ�ϵĲ�����һϵ��ָ�,Ҫôȫ����ɣ�Ҫôһ��Ҳ�����
//ԭ�Ӳ�����ʵ�ָ���ͨ�����������ƣ��������ܹ��ڱ�֤�����ȷ��ǰ���£��ṩ��mutex�������Ƹ��õ�����
//�������Ҫ���ʵĹ�����Դ������ԭ���������ͱ�ʾ����ô�ڶ��̳߳�����ʹ�������µĵȼ��������ͣ���һ�������ѡ��


int main()
{
//----------Hello WinSocket-----------//

	//WORD ver = MAKEWORD(2,2);	//�����汾��
	//WSADATA dat;
	//WSAStartup(ver, &dat);		//����socket��ض�̬���ӿ�
	////---------------------


	////���Windows socket����
	//WSACleanup();

//----------Hello Thread-----------//
	
	//�̴߳���
	//thread t(workFun, 4);
	//t.detach();			
	//�̷߳��룬�����̲߳�������
	//�߳� detach �������̵߳İ󶨣����̹߳��ˣ����̲߳��������߳�ִ�����Զ��˳���
	//�߳� detach�Ժ����̻߳��Ϊ�¶��̣߳��߳�֮�佫�޷�ͨ�š�

	//t.join();		
	//join ���õ�ǰ���̵߳ȴ����е����߳�ִ���꣬�����˳���
	//�߳�������ɺ󣬷������̣߳���join()�̼߳��ǲ������е�

	//�߳�����
	thread ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(workFun, i);
	}

	CELLTimestamp tTime;

	for (int i = 0; i < tCount; i++)
	{
		ts[i].join();		//join()�̼߳��ǲ������е�,����ɺ�ż������߳�
	}

	cout << tTime.getElapsedTimeInMilliSec() << "ms" <<endl; 
	cout << tTime.getElapsedTimeInMicroSec() << "us" << endl;
	cout << "Hello,main thread." << sum << endl;

	sum=0;
	tTime.reset();
	for (int i = 0; i < 800000; i++)
	{
		sum++;
	}
	
    cout << tTime.getElapsedTimeInMilliSec() << "ms" <<endl; 
	cout << tTime.getElapsedTimeInMicroSec() << "us" << endl; 
	cout << "Hello,main thread." << sum << endl;

//----------Next -----------//
	

	getchar();
	return 0;
}