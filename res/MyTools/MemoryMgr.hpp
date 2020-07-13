/*
*	内存管理工具实现

*	MemoryMgr	:	内存管理工具
*	MemoryBlock	:	内存单元信息头
*	MemoryAlloc	:	内存池，实际数据申请与控制

*/

#ifndef _MEMORY_MGR_HPP_
#define _MEMORY_MGR_HPP_
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

//声明
class MemoryBlock;
class MemoryAlloc;

//内存单元（块）信息头，内存池中的最小单位的信息
class MemoryBlock
{
public:		
	MemoryBlock()
	{
		m_id = 0;
		m_refCount = 0;
		m_bRef = false;
		m_bInPool = false;
		m_pAlloc = nullptr;
		m_pNext = nullptr;	
	}

	~MemoryBlock(){}

public:
	int m_id;				//内存块编号
	int m_refCount;			//引用次数
	bool m_bRef;			//是否被引用
	bool m_bInPool;			//是否在内存池中
	MemoryAlloc *m_pAlloc;	//所属大内存块（池）
	MemoryBlock *m_pNext;	//下一个内存块
private:
	//bool m_cNULL;		//预留[内存对齐]
	//friend  class MemoryAlloc;
	//32位：4 + 4 + 1 + 1 + 4 + 4 = 18 + 2（预留）= 20 % 4 = 0【32位按4字节分配内存】
	//64位：4 + 4 + 1 + 1 + 8 + 8 = 26 + 6（预留）= 32 % 8 = 0【32位按8字节分配内存】
};

const int g_MemoryBlockSize = sizeof(MemoryBlock);

//内存池
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		m_pBuf = nullptr;
		m_pHeader = nullptr;
		m_BlockSize = 0;		
		m_BlockNumber = 0;	
	}

	~MemoryAlloc()
	{
		if (nullptr != m_pBuf)
		{
			free(m_pBuf);
		}
	}

	//初始内存池
	void InitMemory()
	{
		assert(nullptr == m_pBuf);		//必须是非空指针
		if (nullptr != m_pBuf)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//申请对象需加锁，多线程

		//向系统申请池内存
		size_t blockSize = m_BlockSize + sizeof(MemoryBlock);
		size_t buffSize = blockSize * m_BlockNumber;
		m_pBuf = (char*)malloc(buffSize);

		//初始化池内存
		m_pHeader = (MemoryBlock*) m_pBuf;
		m_pHeader->m_id = 0;
		m_pHeader->m_refCount = 0;
		m_pHeader->m_bRef = false;
		m_pHeader->m_bInPool = true;
		m_pHeader->m_pAlloc = this;
		m_pHeader->m_pNext = nullptr;	
		

		MemoryBlock *pPreviousBlock = m_pHeader;
		MemoryBlock *pCurrentBlock = m_pHeader;
		for (size_t i = 1; i < m_BlockNumber; i++)
		{
			//给当前单元变量赋值
			pCurrentBlock = (MemoryBlock*) (m_pBuf + i * blockSize);
			pCurrentBlock->m_id = i;
			pCurrentBlock->m_refCount = 0;
			pCurrentBlock->m_bRef = false;
			pCurrentBlock->m_bInPool = true;
			pCurrentBlock->m_pAlloc = this;
			pCurrentBlock->m_pNext = nullptr;
			
			//将当前单元的首地址赋给上一单元对应变量
			pPreviousBlock->m_pNext = pCurrentBlock;	
			pPreviousBlock = pCurrentBlock;
		}

		xPrint("InitMemoryAlloc: BlockSize=%d, BlockNumber=%d, BuffSize=%d\n",blockSize, m_BlockNumber, buffSize);
	}

	//申请内存
	void *Alloc(size_t nSize)
	{
		if (nullptr == m_pBuf)
		{
			InitMemory();
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//申请对象需加锁，多线程
		MemoryBlock *pReturn = nullptr;		//返回的内存单元
		if (nullptr == m_pHeader)
		{
			//内存池中内存单元已经用完-->新申请
			pReturn = (MemoryBlock *)malloc(nSize + sizeof(MemoryBlock));
			pReturn->m_id = -1;
			pReturn->m_refCount = 1;
			pReturn->m_bRef = true;
			pReturn->m_bInPool = false;
			pReturn->m_pAlloc = nullptr;
			pReturn->m_pNext = nullptr;
			xPrint("allocMen: %lx, blockSize=%d+size, id=%d, size=%d\n", pReturn, sizeof(MemoryBlock), pReturn->m_id, nSize);
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
			xPrint("allocMen: %lx, blockSize=%d+%d, id=%d, size=%d\n", pReturn, sizeof(MemoryBlock), pReturn->m_pAlloc->m_BlockSize, pReturn->m_id, nSize);
		}
		
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//释放内存
	void Free(void *pMem)
	{
		MemoryBlock *pBlock = (MemoryBlock*) ( (char*) pMem - sizeof(MemoryBlock));
		assert(true == pBlock->m_bRef);			//必须是已经被引用的

		if (pBlock->m_bInPool)
		{
			//std::lock_guard<std::mutex> lock(m_mutex);		//申请对象需加锁，多线程
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
			free(pBlock);
		}
	}

protected:
	char* m_pBuf;				//内存池首地址
	MemoryBlock* m_pHeader;		//可用头内存单元
	size_t m_BlockSize;			//内存单元的大小
	size_t m_BlockNumber;		//内存单元的数量
	std::mutex m_mutex;			//多线程锁
	
};
//内存池模板【便于在申明变量的时候初始化】
template<size_t nSize, size_t nNumber>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//获取系统字节对齐数
		const size_t n = sizeof(void*);

		m_BlockSize = (nSize / n) * n + ((nSize % n) ? n : 0);
		m_BlockNumber = nNumber;
	}
};



//内存管理工具【单例模式】
class MemoryMgr
{
private:		
	//单例模式：构造私有，使得类有且只有一个实例
	//提供一个访问它的全局访问点，该实例被所有程序模块共享
	MemoryMgr()
	{
		InitAlloctor(0, 32, &m_mem32);
		InitAlloctor(33, 64, &m_mem64);
		InitAlloctor(65, 128, &m_mem128);
		InitAlloctor(129, 256, &m_mem256);
	}

	~MemoryMgr(){}

public:
	static MemoryMgr &Instance()
	{
		//饿汉式：单例对象使用局部静态变量的方式
		//只有在调用时实例化,使用对象,自动析构，不会内存泄露。
		//此变量存在静态区，C++11自带两段检查锁机制来确保static变量实例化一次
		static MemoryMgr mgr;		
		return mgr;
	}

	//申请内存
	void *Alloc(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE)
		{
			//申请大小在管理范围内
			return m_szAlloc[nSize]->Alloc(nSize);
		}
		else
		{
			//申请大小超出管理范围
			MemoryBlock *pNew = (MemoryBlock *)malloc(nSize + sizeof(MemoryBlock));
			pNew->m_id = -1;
			pNew->m_refCount = 1;
			pNew->m_bRef = true;
			pNew->m_bInPool = false;
			pNew->m_pAlloc = nullptr;
			pNew->m_pNext = nullptr;
			xPrint("allocMen: %lx, blockSize=%d+size , id=%d, size=%d\n", pNew, sizeof(MemoryBlock), pNew->m_id, nSize);
			return ((char*)pNew + sizeof(MemoryBlock));
		}
	}

	//释放内存
	void Free(void *pMem)
	{
		MemoryBlock *pBlock = (MemoryBlock*) ( (char*) pMem - sizeof(MemoryBlock));
		assert(true == pBlock->m_bRef);			//必须是已经被引用的
		xPrint("free Men: %lx, id=%d\n", pBlock, pBlock->m_id);
		if (pBlock->m_bInPool)
		{	//池中内存单元由对应内存池释放
			pBlock->m_pAlloc->Free(pMem);
		}
		else
		{
			if (0 == --pBlock->m_refCount)
			{	//共享内存单元，引用计数减一
				//非池中内存单元直接释放
				free(pBlock);
			}
		}
	}

private:
	//内存池映射数组初始化
	void InitAlloctor(int nBegin, int nEnd, MemoryAlloc *pMem)
	{
		//将nBegin到nEnd范围(包括End)的内存池都映射到统一内存池
		for (int i = nBegin; i < nEnd + 1; i++)
		{
			m_szAlloc[i] = pMem;
		}
	}


private:
	enum {MAX_MEMORY_SIZE = 256};					//枚举常量，最大申请内存数
	MemoryAlloc *m_szAlloc[MAX_MEMORY_SIZE + 1];	//内存池映射数组

	//内存池大小：(m_BlockSize + sizeof(MemoryBlock)） * m_BlockNumber;
	MemoryAlloctor<32, 30> m_mem32;			// (32 + 20) * 30 = 1860 bytes = 1.8 kb
	MemoryAlloctor<64, 30> m_mem64;
	MemoryAlloctor<128, 30> m_mem128;
	MemoryAlloctor<256, 30> m_mem256;
};

//重载new/delete运算符
void *operator new(const size_t size)
{
	return MemoryMgr::Instance().Alloc(size);
}

void operator delete(void *p)
{
	MemoryMgr::Instance().Free(p);
}

void *operator new[](const size_t size)
{
	return MemoryMgr::Instance().Alloc(size);
}

void operator delete[](void *p)
{
	MemoryMgr::Instance().Free(p);
}

void *mem_alloc(const size_t size)
{
	return MemoryMgr::Instance().Alloc(size);
}

void mem_free(void *p)
{
	MemoryMgr::Instance().Free(p);
}

#endif	//!_MEMORY_MGR_HPP_