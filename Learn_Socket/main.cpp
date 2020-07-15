#define WIN32_LEAN_AND_MEAN
#include "CELLTimestamp.hpp"
#include "MemoryMgr.hpp"
#include "ObjectMgr.hpp"
#include "CellStream.hpp"

#include <WinSock2.h>
#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ�����

#pragma comment(lib,"ws2_32.lib")	//��Ӷ�̬��

#include <iostream>
using namespace std;
#include <thread>		//c++11 �߳�
#include <mutex>		//��
#include <atomic>		//ԭ��
#include <memory>		//�ڴ���أ�����ָ��

#include <algorithm>	//�㷨

#include <functional>	//��������������ָ��

//Ӧ�ö����ʵ��
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

const int tCount = 4;		//�߳���
const int mCount = 8;		//����
const int nCount = mCount / tCount;	//ÿ���̵߳���
void workFun(int index)
{
//----------���߳�������-----------//
/*
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
*/


//----------���߳��ڴ�ز���-----------//
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

//----------���̶߳���ز���-----------//
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
//��ռʽռ��ϵͳ��Դ,���߳�ʹ��cout��Դ����������ң���Ҫ����Դ���б���
//ʹ�������Խ���
//ԭ�Ӳ�������������ɷָ�Ĵ���ʱ��С�Ĳ�����λ,���ᱻ�̵߳��Ȼ��ƴ�ϵĲ�����һϵ��ָ�,Ҫôȫ����ɣ�Ҫôһ��Ҳ�����
//ԭ�Ӳ�����ʵ�ָ���ͨ�����������ƣ��������ܹ��ڱ�֤�����ȷ��ǰ���£��ṩ��mutex�������Ƹ��õ�����
//�������Ҫ���ʵĹ�����Դ������ԭ���������ͱ�ʾ����ô�ڶ��̳߳�����ʹ�������µĵȼ��������ͣ���һ�������ѡ��

int funA(int a, int b)
{
	printf("funA\n");
	return 0;
}


int main()
{

//----------Hello WinSocket-----------//
/*
	WORD ver = MAKEWORD(2,2);	//�����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);		//����socket��ض�̬���ӿ�
	//---------------------


	//���Windows socket����
	WSACleanup();
*/

//----------Hello Thread-----------//
/*	
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
	//thread ts[tCount];
	//for (int i = 0; i < tCount; i++)
	//{
	//	ts[i] = thread(workFun, i);
	//}

	//CELLTimestamp tTime;

	//for (int i = 0; i < tCount; i++)
	//{
	//	ts[i].join();		//join()�̼߳��ǲ������е�,����ɺ�ż������߳�
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

//----------�ڴ����-----------//
/*
	////��������
	//char *data1 = new char[128];
	//delete[] data1;
	////�������
	//char *data2 = new char;
 //   delete data2;
	////��������
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

	//�ڴ�ض��̲߳���
	thread ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(workFun, i);
		ts[i].detach();
	}
//*/

//----------����ָ��-----------//
/*
	//int *a = new int;
	//*a = 100;
	//printf("a=%d\n", *a);
	//delete a;
	////c++��׼��������ָ���һ��
	//shared_ptr<int> b = make_shared<int>();
	//*b = 100;
	//printf("b=%d\n", *b);

	//ClassA *Aa = new ClassA;
	//Aa->num = 100;
	//delete Aa;
	////����ָ�봴����ָ�룬�Զ�delete
	//shared_ptr<ClassA> bb = make_shared<ClassA>();
	//bb->num = 100;				//ָ��ʹ�ú���ͨ��ָ�����ƣ�ָ��ClassA�ĳ�Ա�����뺯��
	//ClassA *Aa2 = bb.get();		//.������ʹ������ָ����ĳ�Ա�����뺯����get()��ȡClassA��ԭʼָ��
	//int number = bb.use_count();	//����ָ�����ô�����Ϊ0���ͷ�
	//ClassA aa;
*/

//----------�����-----------//
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

	//����ض��̲߳���
	thread ts[tCount];
	for (int i = 0; i < tCount; i++)
	{
		ts[i] = thread(workFun, i);
		ts[i].detach();
	}

	//����ָ��������
	//printf("---------------1---------------\n");
	//{	//����ʹ������ָ�룬������������ص�new������ʹ�ö����
	//	shared_ptr<ClassA> s0 = make_shared<ClassA>();
	//}
	//printf("---------------2---------------\n");
	//{	//��ʽ�������ص�new�������ʹ�������Ч
	//	shared_ptr<ClassA> s1(new ClassA());;
	//}

//*/

//----------function-----------//
/*
	//����һ������call ��int�͵������������ĺ�����ָ��funA��������call����funA
	std::function< int(int, int) > call = funA;	
	int a = call(1, 2);

	//����������lambda���ʽ
	std::function< int(int) > call_1;	

	//lambda���ʽ
	//[ capture ] ( params ) opt -> ret { body; }
	//[ �ⲿ���������б� ] ( �����б� ) ��������� -> ����ֵ���� { ������ }
	//
	//�����б�; ��ϸ�����˱��ʽ�ܹ����ʵ��ⲿ�������Լ���η�����Щ����
	//1��	[]���������κα���
	//2��	[&]�������ⲿ�����������б���������Ϊ�����ں������е��ã������ò���
	//3��	[=]�������ⲿ�����������б���������Ϊ�����ں������е��ã���ֵ����
	//4��	[=, &foo]�������ⲿ�����������б������������ò���foo����
	//5��	[bar]��ֻ����bar��������������ã�����
	//6��	[this]������ǰ���е�thisָ�룬��lambda���ʽӵ�к͵�ǰ���Ա������ͬ�ķ���Ȩ�ޣ��������&��=��Ĭ�ϰ�����ѡ��

	//���������
	//mutable��lambda���ʽ�ڲ���������޸ı��������
	//exception��lambda���ʽ�Ƿ��׳��쳣�Լ������쳣
	//attribute����������

	call_1 = [a ](int b) -> int //����ֵ����
	{
		//������
		printf("%d\n", a+b);
		return 2;
	};

	int c = call_1(2);
	printf("%d\n", c);

//*/

//----------�ֽ���-----------//
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