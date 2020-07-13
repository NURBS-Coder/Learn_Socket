/*
*	���������ʵ��

*	ObjectMgr	:		���������
*	NodeHeader	:		����Ԫ��Ϣͷ
*	ObjectPool	:		����أ�ʵ���������������
*	ObjectPoolBase	:	�����ģ����

*/

#ifndef _OBJECT_MGR_HPP_
#define _OBJECT_MGR_HPP_
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

//�����ģ�塾����������������ʱ���ʼ����
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
		int m_id;				//�������
		int m_refCount;			//���ô���
		bool m_bRef;			//�Ƿ�����
		bool m_bInPool;			//�Ƿ����ڴ����
		NodeHeader *m_pNext;	//��һ����
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
	//��ʼ�������
	void InitPool()
	{
		assert(nullptr == m_pBuf);		//�����Ƿǿ�ָ��
		if (nullptr != m_pBuf)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//��ʼ�����������������߳�

		//��ϵͳ������ڴ�
		size_t blockSize = m_ObjectSize + m_HeaderSize;
		size_t PoolSize = nPoolSize * blockSize;
		m_pBuf = new char[PoolSize];

		//��ʼ�����ڴ�
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
			//����ǰ��Ԫ������ֵ
			pCurrentBlock = (NodeHeader*) (m_pBuf + i * blockSize);
			pCurrentBlock->m_id = i;
			pCurrentBlock->m_refCount = 0;
			pCurrentBlock->m_bRef = false;
			pCurrentBlock->m_bInPool = true;
			pCurrentBlock->m_pNext = nullptr;
			
			//����ǰ��Ԫ���׵�ַ������һ��Ԫ��Ӧ����
			pPreviousBlock->m_pNext = pCurrentBlock;	
			pPreviousBlock = pCurrentBlock;
		}

		xPrint("InitObjectPool: BlockSize=%d, ObjectNumber=%d, PoolSize=%d\n",m_HeaderSize + m_ObjectSize, nPoolSize, PoolSize);
	}

public:
	//�������
	void *AllocObject()
	{
		if (nullptr == m_pBuf)
		{
			InitPool();
		}

		std::lock_guard<std::mutex> lock(m_mutex);		//�����������������߳�
		NodeHeader *pReturn = nullptr;		//���ص��ڴ浥Ԫ
		if (nullptr == m_pHeader)
		{
			//�ڴ�����ڴ浥Ԫ�Ѿ�����-->������
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
			//�ڴ���л��п��õ��ڴ浥Ԫ-->ֱ�ӷ���
			pReturn = m_pHeader;
			assert(false == m_pHeader->m_bRef);		//������û�����õ�
			m_pHeader->m_bRef = true;
			m_pHeader->m_refCount++;

			//ָ����һ�������ڴ浥Ԫ
			m_pHeader = m_pHeader->m_pNext;
			xPrint("allocObject: %lx, id=%d, blockSize=%d+%d\n", pReturn, pReturn->m_id, m_HeaderSize, m_ObjectSize);
		}
		
		return ((char*)pReturn + sizeof(NodeHeader));
	}

	//�ͷŶ���
	void FreeObject(void *pMem)
	{
		NodeHeader *pBlock = (NodeHeader*) ( (char*) pMem - sizeof(NodeHeader));
		assert(true == pBlock->m_bRef);			//�������Ѿ������õ�
		xPrint("free Object: %lx, id=%d\n", pBlock, pBlock->m_id);
		if (pBlock->m_bInPool)
		{	
			std::lock_guard<std::mutex> lock(m_mutex);		//�ͷŶ����ٽ���������������߳�
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
			delete[] (void*)pBlock;  //?????������-->Ҫ��void*
			//free(pBlock);
		}
	}

private:
	size_t m_ObjectSize;		//һ��������С
	size_t m_HeaderSize;		//һ���������Ϣͷ��С
	char* m_pBuf;				//������׵�ַ
	NodeHeader* m_pHeader;		//���ö����ڴ浥Ԫ��ͷ
	std::mutex m_mutex;			//���߳���
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
	//������������ĸ��������Զ���ض�����������ͷŽ��п��ƣ������̫ǿ-->ʹ��ģ��-->�������඼�����ĸ�����
	//�������������Ĵ���������ʱ����
	void *operator new(const size_t size)
	{
		return InstancePool().AllocObject();
	}

	void operator delete(void *p)
	{
		InstancePool().FreeObject(p);
	}

	//ʹ�þ�̬�������������٣����ԶԴ���������в���
	//template <class... Args>					//��ͬ��Ĺ��������һ�����ÿɱ�� vs2012��֧��
	static Type *CreatObject()
	{
		Type *obj = new Type();
		//����������������


		return obj;
	}

	static void DestroyObject(Type *obj)
	{
		delete obj;
	}

private:
	//������̬�����
	typedef ObjectPool<Type, nPoolSize> ObjectTypePool;
	static ObjectTypePool& InstancePool()
	{
		static ObjectTypePool sPool;
		return sPool;
	}
};


#endif	// !_OBJECT_MGR_HPP_