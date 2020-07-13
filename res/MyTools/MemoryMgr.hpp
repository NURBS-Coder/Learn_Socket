/*
*	�ڴ������ʵ��

*	MemoryMgr	:	�ڴ������
*	MemoryBlock	:	�ڴ浥Ԫ��Ϣͷ
*	MemoryAlloc	:	�ڴ�أ�ʵ���������������

*/

#ifndef _MEMORY_MGR_HPP_
#define _MEMORY_MGR_HPP_
#include <cstdlib>
#include <cassert>
#include <mutex>

#ifdef _DEBUG	//���Զ���
	#ifndef xPrint
		#include <cstdio>
		#define xPrint(...) printf(__VA_ARGS__)		//�ɱ����
	#endif
#else
	#ifndef xPrint
		#define xPrint(...)
	#endif
#endif	//! _DEBUG

//����
class MemoryBlock;
class MemoryAlloc;

//�ڴ浥Ԫ���飩��Ϣͷ���ڴ���е���С��λ����Ϣ
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
	int m_id;				//�ڴ����
	int m_refCount;			//���ô���
	bool m_bRef;			//�Ƿ�����
	bool m_bInPool;			//�Ƿ����ڴ����
	MemoryAlloc *m_pAlloc;	//�������ڴ�飨�أ�
	MemoryBlock *m_pNext;	//��һ���ڴ��
private:
	//bool m_cNULL;		//Ԥ��[�ڴ����]
	//friend  class MemoryAlloc;
	//32λ��4 + 4 + 1 + 1 + 4 + 4 = 18 + 2��Ԥ����= 20 % 4 = 0��32λ��4�ֽڷ����ڴ桿
	//64λ��4 + 4 + 1 + 1 + 8 + 8 = 26 + 6��Ԥ����= 32 % 8 = 0��32λ��8�ֽڷ����ڴ桿
};

const int g_MemoryBlockSize = sizeof(MemoryBlock);

//�ڴ��
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

	//��ʼ�ڴ��
	void InitMemory()
	{
		assert(nullptr == m_pBuf);		//�����Ƿǿ�ָ��
		if (nullptr != m_pBuf)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//�����������������߳�

		//��ϵͳ������ڴ�
		size_t blockSize = m_BlockSize + sizeof(MemoryBlock);
		size_t buffSize = blockSize * m_BlockNumber;
		m_pBuf = (char*)malloc(buffSize);

		//��ʼ�����ڴ�
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
			//����ǰ��Ԫ������ֵ
			pCurrentBlock = (MemoryBlock*) (m_pBuf + i * blockSize);
			pCurrentBlock->m_id = i;
			pCurrentBlock->m_refCount = 0;
			pCurrentBlock->m_bRef = false;
			pCurrentBlock->m_bInPool = true;
			pCurrentBlock->m_pAlloc = this;
			pCurrentBlock->m_pNext = nullptr;
			
			//����ǰ��Ԫ���׵�ַ������һ��Ԫ��Ӧ����
			pPreviousBlock->m_pNext = pCurrentBlock;	
			pPreviousBlock = pCurrentBlock;
		}

		xPrint("InitMemoryAlloc: BlockSize=%d, BlockNumber=%d, BuffSize=%d\n",blockSize, m_BlockNumber, buffSize);
	}

	//�����ڴ�
	void *Alloc(size_t nSize)
	{
		if (nullptr == m_pBuf)
		{
			InitMemory();
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//�����������������߳�
		MemoryBlock *pReturn = nullptr;		//���ص��ڴ浥Ԫ
		if (nullptr == m_pHeader)
		{
			//�ڴ�����ڴ浥Ԫ�Ѿ�����-->������
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
			//�ڴ���л��п��õ��ڴ浥Ԫ-->ֱ�ӷ���
			pReturn = m_pHeader;
			assert(false == m_pHeader->m_bRef);		//������û�����õ�
			m_pHeader->m_bRef = true;
			m_pHeader->m_refCount++;

			//ָ����һ�������ڴ浥Ԫ
			m_pHeader = m_pHeader->m_pNext;
			xPrint("allocMen: %lx, blockSize=%d+%d, id=%d, size=%d\n", pReturn, sizeof(MemoryBlock), pReturn->m_pAlloc->m_BlockSize, pReturn->m_id, nSize);
		}
		
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//�ͷ��ڴ�
	void Free(void *pMem)
	{
		MemoryBlock *pBlock = (MemoryBlock*) ( (char*) pMem - sizeof(MemoryBlock));
		assert(true == pBlock->m_bRef);			//�������Ѿ������õ�

		if (pBlock->m_bInPool)
		{
			//std::lock_guard<std::mutex> lock(m_mutex);		//�����������������߳�
			if (0 != --pBlock->m_refCount)
			{	//�����ڴ浥Ԫ�����ü�����һ
				return;
			}
			//�����ڴ浥Ԫ�Żس���
			pBlock->m_bRef = false;
			pBlock->m_pNext = m_pHeader;
			m_pHeader = pBlock;
		}
		else
		{
			if (0 != --pBlock->m_refCount)
			{	//�����ڴ浥Ԫ�����ü�����һ
				return;
			}
			//�ǳ����ڴ浥Ԫֱ���ͷ�
			free(pBlock);
		}
	}

protected:
	char* m_pBuf;				//�ڴ���׵�ַ
	MemoryBlock* m_pHeader;		//����ͷ�ڴ浥Ԫ
	size_t m_BlockSize;			//�ڴ浥Ԫ�Ĵ�С
	size_t m_BlockNumber;		//�ڴ浥Ԫ������
	std::mutex m_mutex;			//���߳���
	
};
//�ڴ��ģ�塾����������������ʱ���ʼ����
template<size_t nSize, size_t nNumber>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//��ȡϵͳ�ֽڶ�����
		const size_t n = sizeof(void*);

		m_BlockSize = (nSize / n) * n + ((nSize % n) ? n : 0);
		m_BlockNumber = nNumber;
	}
};



//�ڴ�����ߡ�����ģʽ��
class MemoryMgr
{
private:		
	//����ģʽ������˽�У�ʹ��������ֻ��һ��ʵ��
	//�ṩһ����������ȫ�ַ��ʵ㣬��ʵ�������г���ģ�鹲��
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
		//����ʽ����������ʹ�þֲ���̬�����ķ�ʽ
		//ֻ���ڵ���ʱʵ����,ʹ�ö���,�Զ������������ڴ�й¶��
		//�˱������ھ�̬����C++11�Դ����μ����������ȷ��static����ʵ����һ��
		static MemoryMgr mgr;		
		return mgr;
	}

	//�����ڴ�
	void *Alloc(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SIZE)
		{
			//�����С�ڹ���Χ��
			return m_szAlloc[nSize]->Alloc(nSize);
		}
		else
		{
			//�����С��������Χ
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

	//�ͷ��ڴ�
	void Free(void *pMem)
	{
		MemoryBlock *pBlock = (MemoryBlock*) ( (char*) pMem - sizeof(MemoryBlock));
		assert(true == pBlock->m_bRef);			//�������Ѿ������õ�
		xPrint("free Men: %lx, id=%d\n", pBlock, pBlock->m_id);
		if (pBlock->m_bInPool)
		{	//�����ڴ浥Ԫ�ɶ�Ӧ�ڴ���ͷ�
			pBlock->m_pAlloc->Free(pMem);
		}
		else
		{
			if (0 == --pBlock->m_refCount)
			{	//�����ڴ浥Ԫ�����ü�����һ
				//�ǳ����ڴ浥Ԫֱ���ͷ�
				free(pBlock);
			}
		}
	}

private:
	//�ڴ��ӳ�������ʼ��
	void InitAlloctor(int nBegin, int nEnd, MemoryAlloc *pMem)
	{
		//��nBegin��nEnd��Χ(����End)���ڴ�ض�ӳ�䵽ͳһ�ڴ��
		for (int i = nBegin; i < nEnd + 1; i++)
		{
			m_szAlloc[i] = pMem;
		}
	}


private:
	enum {MAX_MEMORY_SIZE = 256};					//ö�ٳ�������������ڴ���
	MemoryAlloc *m_szAlloc[MAX_MEMORY_SIZE + 1];	//�ڴ��ӳ������

	//�ڴ�ش�С��(m_BlockSize + sizeof(MemoryBlock)�� * m_BlockNumber;
	MemoryAlloctor<32, 30> m_mem32;			// (32 + 20) * 30 = 1860 bytes = 1.8 kb
	MemoryAlloctor<64, 30> m_mem64;
	MemoryAlloctor<128, 30> m_mem128;
	MemoryAlloctor<256, 30> m_mem256;
};

//����new/delete�����
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