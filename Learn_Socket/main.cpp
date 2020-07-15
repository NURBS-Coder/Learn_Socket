#define WIN32_LEAN_AND_MEAN
#include "CELLTimestamp.hpp"
#include "MemoryMgr.hpp"
#include "ObjectMgr.hpp"
#include "CellStream.hpp"

#include <WinSock2.h>
#include <Windows.h>			//里面有WinSock1.h，不能放前面,否则要定义宏

#pragma comment(lib,"ws2_32.lib")	//添加动态库

#include <iostream>
using namespace std;
#include <thread>		//c++11 线程
#include <mutex>		//锁
#include <atomic>		//原子
#include <memory>		//内存相关：智能指针

#include <algorithm>	//算法

#include <functional>	//匿名函数、函数指针

//应用对象池实例
class ClassA : public ObjectPoolBase<ClassA, 5>
{
public:
	ClassA()
	{
		num = 0;
		printf("ClassA\n");
	}

	~ClassA()
	{
		printf("~ClassA\n");
	}

	int num;
private:

};

class ClassB : public ObjectPoolBase<ClassB, 5>
{
public:
	ClassB()
	{
		num = 0;
		printf("ClassB\n");
	}

	~ClassB()
	{
		printf("~ClassB\n");
	}

	int num;
private:

};

mutex m;
//int sum = 0;
atomic<int> sum = 0;

const int tCount = 4;		//线程数
const int mCount = 8;		//总数
const int nCount = mCount / tCount;	//每个线程的数
void workFun(int index)
{
//----------多线程锁测试-----------//
/*
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
*/


//----------多线程内存池测试-----------//
/*
	char *data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[1+i];
	
	}

	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
//*/

//----------多线程对象池测试-----------//
/*
	ClassA *data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = ClassA::CreatObject();
	
	}

	for (size_t i = 0; i < nCount; i++)
	{
		ClassA::DestroyObject(data[i]);
	}
//*/

}
//抢占式占用系统资源,多线程使用cout资源，会输出错乱，需要对资源进行保护
//使用锁、自解锁
//原子操作：计算机不可分割的处理时最小的操作单位,不会被线程调度机制打断的操作（一系列指令）,要么全部完成，要么一条也不完成
//原子操作的实现跟普通数据类型类似，但是它能够在保证结果正确的前提下，提供比mutex等锁机制更好的性能
//如果我们要访问的共享资源可以用原子数据类型表示，那么在多线程程序中使用这种新的等价数据类型，是一个不错的选择。

int funA(int a, int b)
{
	printf("funA\n");
	return 0;
}


int main()
{

//----------Hello WinSocket-----------//
/*
	WORD ver = MAKEWORD(2,2);	//创建版本号
	WSADATA dat;
	WSAStartup(ver, &dat);		//加载socket相关动态链接库
	//---------------------


	//清除Windows socket环境
	WSACleanup();
*/

//----------Hello Thread-----------//
/*	
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
	//thread ts[tCount];
	//for (int i = 0; i < tCount; i++)
	//{
	//	ts[i] = thread(workFun, i);
	//}

	//CELLTimestamp tTime;

	//for (int i = 0; i < tCount; i++)
	//{
	//	ts[i].join();		//join()线程间是并行运行的,都完成后才继续主线程
	//}

	//cout << tTime.getElapsedTimeInMilliSec() << "ms" <<endl; 
	//cout << tTime.getElapsedTimeInMicroSec() << "us" << endl;
	//cout << "Hello,main thread." << sum << endl;

	//sum=0;
	//tTime.reset();
	//for (int i = 0; i < 800000; i++)
	//{
	//	sum++;
	//}
	//
 //   cout << tTime.getElapsedTimeInMilliSec() << "ms" <<endl; 
	//cout << tTime.getElapsedTimeInMicroSec() << "us" << endl; 
	//cout << "Hello,main thread." << sum << endl;

*/

//----------内存管理-----------//
/*
	////申请数组
	//char *data1 = new char[128];
	//delete[] data1;
	////申请变量
	//char *data2 = new char;
 //   delete data2;
	////函数申请
	//char *data3 = (char*)mem_alloc(64);
	//mem_free(data3);

	//char* data[260];
	//for (int i = 0; i < 260; i++)
	//{
	//	data[i] = new char[1+i];
	//	if (0 != i)
	//	{
	//		//printf("Previous - Current = %d\n",data[i] - data[i-1]);
	//	}
	//}

	//for (int i = 0; i < 260; i++)
	//{
	//	delete[] data[i];
	//	//printf("%lx\n",data[i]);
	//}

	//内存池多线程测试
	thread ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(workFun, i);
		ts[i].detach();
	}
//*/

//----------智能指针-----------//
/*
	//int *a = new int;
	//*a = 100;
	//printf("a=%d\n", *a);
	//delete a;
	////c++标准库中智能指针的一种
	//shared_ptr<int> b = make_shared<int>();
	//*b = 100;
	//printf("b=%d\n", *b);

	//ClassA *Aa = new ClassA;
	//Aa->num = 100;
	//delete Aa;
	////智能指针创建类指针，自动delete
	//shared_ptr<ClassA> bb = make_shared<ClassA>();
	//bb->num = 100;				//指向使用和普通类指针类似，指向ClassA的成员变量与函数
	//ClassA *Aa2 = bb.get();		//.操作符使用智能指针类的成员变量与函数，get()获取ClassA的原始指针
	//int number = bb.use_count();	//智能指针引用次数。为0就释放
	//ClassA aa;
*/

//----------对象池-----------//
/*
	//ClassA *Aa = new ClassA;
	//Aa->num = 100;
	//delete Aa;

	//ClassA *Aa1 = ClassA::CreatObject();
	//Aa1->num = 100;
	//ClassA::DestroyObject(Aa1);

	//ClassB *Bb = new ClassB;
	//Bb->num = 100;
	//delete Bb;

	//ClassB *Bb1 = ClassB::CreatObject();
	//Bb1->num = 100;
	//ClassB::DestroyObject(Bb1);

	//对象池多线程测试
	thread ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(workFun, i);
		ts[i].detach();
	}

	//智能指针与对象池
	//printf("---------------1---------------\n");
	//{	//正常使用智能指针，不会调用类重载的new，不会使用对象池
	//	shared_ptr<ClassA> s0 = make_shared<ClassA>();
	//}
	//printf("---------------2---------------\n");
	//{	//显式调用重载的new运算符，使对象池生效
	//	shared_ptr<ClassA> s1(new ClassA());;
	//}

//*/

//----------function-----------//
/*
	//定义一个变量call 是int型的有两个参数的函数，指向funA，可以用call调用funA
	std::function< int(int, int) > call = funA;	
	int a = call(1, 2);

	//匿名函数，lambda表达式
	std::function< int(int) > call_1;	

	//lambda表达式
	//[ capture ] ( params ) opt -> ret { body; }
	//[ 外部变量捕获列表 ] ( 参数列表 ) 特殊操作符 -> 返回值类型 { 函数体 }
	//
	//捕获列表; 精细控制了表达式能够访问的外部变量，以及如何访问这些变量
	//1）	[]：不捕获任何变量
	//2）	[&]：捕获外部作用域中所有变量，并作为引用在函数体中调用（按引用捕获）
	//3）	[=]：捕获外部作用域中所有变量，并作为副本在函数体中调用（按值捕获）
	//4）	[=, &foo]：捕获外部作用域中所有变量，并按引用捕获foo变量
	//5）	[bar]：只捕获bar变量，多个变量用，隔开
	//6）	[this]：捕获当前类中的this指针，让lambda表达式拥有和当前类成员函数相同的访问权限，如果适用&或=，默认包含此选项

	//特殊操作符
	//mutable：lambda表达式内部代码可以修改被捕获变量
	//exception：lambda表达式是否抛出异常以及何种异常
	//attribute：声明属性

	call_1 = [a ](int b) -> int //返回值类型
	{
		//函数体
		printf("%d\n", a+b);
		return 2;
	};

	int c = call_1(2);
	printf("%d\n", c);

//*/

//----------字节流-----------//
/*
	//CellStream byteStream;
	//byteStream.WriteInt8(1);
	//byteStream.WriteInt16(2);
	//byteStream.WriteInt32(3);
	//byteStream.WriteFloat(4.5);
	//byteStream.WriteDouble(6.7);
	//char * str = "hehehe";
	//byteStream.WriteArray(str, strlen(str));
	//char a[] = "hahaha";
	//byteStream.WriteArray(a, strlen(a));
	//int b[] = {1, 2, 3, 4};
	//byteStream.WriteArray(b, 4);

	//auto n1 = byteStream.ReadInt8();
	//auto n2 = byteStream.ReadInt16();
	//auto n3 = byteStream.ReadInt32();
	//auto n4 = byteStream.ReadFloat();
	//auto n5 = byteStream.ReadDouble();

	//uint32_t n = 0;
	//char name[32] = {};
	//n = byteStream.ReadArraySize();
	//auto n6 = byteStream.ReadArray(name , 5);
	//if (0 == n6)
	//{
	//	auto n6_1 = byteStream.ReadArray(name , 32);
	//}
	//char pw[32] = {};
	//n = byteStream.ReadArraySize();
	//auto n7 = byteStream.ReadArray(pw , 32);
	//int data[32] = {};
	//n = byteStream.ReadArraySize();
	//auto n8 = byteStream.ReadArray(data , 32);

//*/
	getchar();
	return 0;
}