#define WIN32_LEAN_AND_MEAN
#include "CELLTimestamp.hpp"

#include <WinSock2.h>
#include <Windows.h>			//里面有WinSock1.h，不能放前面,否则要定义宏

#pragma comment(lib,"ws2_32.lib")	//添加动态库

#include <iostream>
using namespace std;
#include <thread>		//c++11 线程
#include <mutex>		//锁
#include <atomic>		//原子

#include <algorithm>
mutex m;
const int tCount = 4;
//int sum = 0;
atomic<int> sum = 0;
void workFun(int index)
{
	
	for (int i = 0; i < 4 ; i++)
	{
		//锁与解锁：尽可能少使用锁，会占用系统资源，不要频繁使用锁，加锁一定要解锁
		m.lock();	//临界区域--开始
		cout << "Hello," << index << "thread." << i <<endl;
		//sum++;
		m.unlock();	//临界区域--结束

		//自解锁：离开当前作用域就解锁
		//lock_guard<mutex> lg(m);
		//sum++;

		//原子操作:自带锁，防止被其他线程误操作
		sum++;
	}
}
//抢占式占用系统资源,多线程使用cout资源，会输出错乱，需要对资源进行保护
//使用锁、自解锁
//原子操作：计算机不可分割的处理时最小的操作单位,不会被线程调度机制打断的操作（一系列指令）,要么全部完成，要么一条也不完成
//原子操作的实现跟普通数据类型类似，但是它能够在保证结果正确的前提下，提供比mutex等锁机制更好的性能
//如果我们要访问的共享资源可以用原子数据类型表示，那么在多线程程序中使用这种新的等价数据类型，是一个不错的选择。


int main()
{
//----------Hello WinSocket-----------//

	//WORD ver = MAKEWORD(2,2);	//创建版本号
	//WSADATA dat;
	//WSAStartup(ver, &dat);		//加载socket相关动态链接库
	////---------------------


	////清除Windows socket环境
	//WSACleanup();

//----------Hello Thread-----------//
	
	//线程传参
	//thread t(workFun, 4);
	//t.detach();			
	//线程分离，与主线程并行运行
	//线程 detach 脱离主线程的绑定，主线程挂了，子线程不报错，子线程执行完自动退出。
	//线程 detach以后，子线程会成为孤儿线程，线程之间将无法通信。

	//t.join();		
	//join 是让当前主线程等待所有的子线程执行完，才能退出。
	//线程运行完成后，返回主线程，但join()线程间是并行运行的

	//线程数组
	thread ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(workFun, i);
	}

	CELLTimestamp tTime;

	for (int i = 0; i < tCount; i++)
	{
		ts[i].join();		//join()线程间是并行运行的,都完成后才继续主线程
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