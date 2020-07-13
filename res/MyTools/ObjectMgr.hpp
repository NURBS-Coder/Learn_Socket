/*
*	对象管理工具实现

*	ObjectMgr	:		对象管理工具
*	NodeHeader	:		对象单元信息头
*	ObjectPool	:		对象池，实际数据申请与控制
*	ObjectPoolBase	:	对象池模板类

*/

#ifndef _OBJECT_MGR_HPP_
#define _OBJECT_MGR_HPP_
#include <cstdlib>
#include <cassert>
#include <mutex>

#ifdef _DEBUG	//调试定义
	#ifndef xPrint
		#include <cstdio>
		#define xPrint(...) printf(__VA_ARGS__)		//可变参数
	#endif
#else
	#ifndef xPrint
		#define xPrint(...)
	#endif
#endif	//! _DEBUG

//对象池模板【便于在申明变量的时候初始化】
template<class Type, size_t nPoolSize>
class ObjectPool
{
private:
	class NodeHeader
	{
	public:
		NodeHeader(){}
		~NodeHeader(){}

	public:
		int m_id;				//对象块编号
		int m_refCount;			//引用次数
		bool m_bRef;			//是否被引用
		bool m_bInPool;			//是否在内存池中
		NodeHeader *m_pNext;	//下一个块
	};

public:
	ObjectPool()
	{
		m_pBuf = nullptr;
		m_pHeader = nullptr;
		m_ObjectSize = sizeof(Type);
		m_HeaderSize = sizeof(NodeHeader);
		InitPool();
	}

	~ObjectPool()
	{
		delete[] m_pBuf;
	}

private:
	//初始化对象池
	void InitPool()
	{
		assert(nullptr == m_pBuf);		//必须是非空指针
		if (nullptr != m_pBuf)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//初始化对象池需加锁，多线程

		//向系统申请池内存
		size_t blockSize = m_ObjectSize + m_HeaderSize;
		size_t PoolSize = nPoolSize * blockSize;
		m_pBuf = new char[PoolSize];

		//初始化池内存
		m_pHeader = (NodeHeader*) m_pBuf;
		m_pHeader->m_id = 0;
		m_pHeader->m_refCount = 0;
		m_pHeader->m_bRef = false;
		m_pHeader->m_bInPool = true;
		m_pHeader->m_pNext = nullptr;	

		NodeHeader *pPreviousBlock = m_pHeader;
		NodeHeader *pCurrentBlock = m_pHeader;
		for (size_t i = 1; i < nPoolSize; i++)
		{
			//给当前单元变量赋值
			pCurrentBlock = (NodeHeader*) (m_pBuf + i * blockSize);
			pCurrentBlock->m_id = i;
			pCurrentBlock->m_refCount = 0;
			pCurrentBlock->m_bRef = false;
			pCurrentBlock->m_bInPool = true;
			pCurrentBlock->m_pNext = nullptr;
			
			//将当前单元的首地址赋给上一单元对应变量
			pPreviousBlock->m_pNext = pCurrentBlock;	
			pPreviousBlock = pCurrentBlock;
		}

		xPrint("InitObjectPool: BlockSize=%d, ObjectNumber=%d, PoolSize=%d\n",m_HeaderSize + m_ObjectSize, nPoolSize, PoolSize);
	}

public:
	//申请对象
	void *AllocObject()
	{
		if (nullptr == m_pBuf)
		{
			InitPool();
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//申请对象需加锁，多线程
		NodeHeader *pReturn = nullptr;		//返回的内存单元
		if (nullptr == m_pHeader)
		{
			//内存池中内存单元已经用完-->新申请
			pReturn = (NodeHeader *)new char[sizeof(Type) + sizeof(NodeHeader)];
			//pReturn = (NodeHeader *)malloc(sizeof(Type) + sizeof(NodeHeader));
			pReturn->m_id = -1;
			pReturn->m_refCount = 1;
			pReturn->m_bRef = true;
			pReturn->m_bInPool = false;
			pReturn->m_pNext = nullptr;
			xPrint("allocObject: %lx, id=%d ,blockSize=%d+%d\n", pReturn, pReturn->m_id, sizeof(NodeHeader), sizeof(Type));
		}
		else
		{
			//内存池中还有可用的内存单元-->直接返回
			pReturn = m_pHeader;
			assert(false == m_pHeader->m_bRef);		//必须是没被引用的
			m_pHeader->m_bRef = true;
			m_pHeader->m_refCount++;

			//指向下一个可用内存单元
			m_pHeader = m_pHeader->m_pNext;
			xPrint("allocObject: %lx, id=%d, blockSize=%d+%d\n", pReturn, pReturn->m_id, m_HeaderSize, m_ObjectSize);
		}
		
		return ((char*)pReturn + sizeof(NodeHeader));
	}

	//释放对象
	void FreeObject(void *pMem)
	{
		NodeHeader *pBlock = (NodeHeader*) ( (char*) pMem - sizeof(NodeHeader));
		assert(true == pBlock->m_bRef);			//必须是已经被引用的
		xPrint("free Object: %lx, id=%d\n", pBlock, pBlock->m_id);
		if (pBlock->m_bInPool)
		{	
			std::lock_guard<std::mutex> lock(m_mutex);		//释放对象【临界区域】需加锁，多线程
			if (0 != --pBlock->m_refCount)
			{	//共享内存单元，引用计数减一
				return;
			}
			//池中内存单元放回池中
			pBlock->m_bRef = false;
			pBlock->m_pNext = m_pHeader;
			m_pHeader = pBlock;
		}
		else
		{
			if (0 != --pBlock->m_refCount)
			{	//共享内存单元，引用计数减一
				return;
			}
			//非池中内存单元直接释放
			delete[] (void*)pBlock;  //?????有问题-->要加void*
			//free(pBlock);
		}
	}

private:
	size_t m_ObjectSize;		//一个对象块大小
	size_t m_HeaderSize;		//一个对象块信息头大小
	char* m_pBuf;				//对象池首地址
	NodeHeader* m_pHeader;		//可用对象内存单元链头
	std::mutex m_mutex;			//多线程锁
};

template <class Type, size_t nPoolSize>
class ObjectPoolBase
{
public:
	ObjectPoolBase()
	{

	}

	~ObjectPoolBase()
	{

	}

public:
	//类中添加以下四个函数可以对相关对象的申请与释放进行控制，但耦合太强-->使用模板-->创建的类都有这四个函数
	//重载运算符，类的创建与销毁时调用
	void *operator new(const size_t size)
	{
		return InstancePool().AllocObject();
	}

	void operator delete(void *p)
	{
		InstancePool().FreeObject(p);
	}

	//使用静态函数创建与销毁，可以对创建对象进行操作
	//template <class... Args>					//不同类的构造参数不一样，用可变的 vs2012不支持
	static Type *CreatObject()
	{
		Type *obj = new Type();
		//可以做点想做的事


		return obj;
	}

	static void DestroyObject(Type *obj)
	{
		delete obj;
	}

private:
	//单例静态对象池
	typedef ObjectPool<Type, nPoolSize> ObjectTypePool;
	static ObjectTypePool& InstancePool()
	{
		static ObjectTypePool sPool;
		return sPool;
	}
};


#endif	// !_OBJECT_MGR_HPP_