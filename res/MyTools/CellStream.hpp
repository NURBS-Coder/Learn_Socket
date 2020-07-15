/*
	流数据管理类型
*/

#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_

#include "MessageHeader.hpp"

#include <cstdint>
#include <cstring>

//字节流BYTE，统一不同系统中的数据传输
class CellStream
{
public:
	//自己申请内存构造（一般是写入数据流）
	CellStream(int nSize = 1024)
	{
		m_nSize = nSize;
		m_pBuffer = new char[m_nSize];
		m_bDelete = true;
		m_nWritePos = 0;
		m_nReadPos = 0;
	}

	//已有内存传入构造（一般是读取数据流）
	CellStream(char* pData, int nSize, bool bDelete = false)
	{
		m_nSize = nSize;
		m_pBuffer = pData;
		m_bDelete = bDelete;
		m_nWritePos = 0;
		m_nReadPos = 0;
	}

	virtual ~CellStream()
	{
		if (m_bDelete && nullptr != m_pBuffer)
		{
			delete [] m_pBuffer;
			m_pBuffer = nullptr;
		}
	}


	//与宏替换有区别<有参数、有返回值、有类型检查>
	//所谓"内联函数"就是将很简单的函数"内嵌"到调用他的程序代码中,节约时空开销
	//函数体必须十分简单,不能含有循环、条件、选择等复杂的结构，否则就不能做为内联函数
	inline bool CanRead(const size_t& n)
	{
		return m_nSize - m_nReadPos >= n;
	}

	inline bool CanWrite(const size_t& n)
	{
		return m_nSize - m_nWritePos >= n;
	}

	inline int GetWritePos()
	{
		return m_nWritePos;
	}

	inline void SetWritePos(const size_t &n)
	{
		m_nWritePos = n;
	}

////读取数据
	template<typename Type>		//基本类型
	bool Read(Type& n, const bool &bOffSet = true)
	{
		//计算读取数据的大小
		size_t nLen = sizeof(Type);
		//判断能不能读取
		if (CanRead(nLen))
		{
			//缓冲区数据拷贝出来
			memcpy(&n, m_pBuffer + m_nReadPos, nLen);
			//更新尾部位置
			if (bOffSet)
			{	//默认偏移
				m_nReadPos += nLen;
			}
			return true;
		}
		return false;
	}
	int8_t ReadInt8()
	{
		int8_t n =0;
		Read(n);
		return n;
	}
	int16_t ReadInt16()
	{
		int16_t n =0;
		Read(n);
		return n;
	}
	int32_t ReadInt32()
	{
		int32_t n =0;
		Read(n);
		return n;
	}
	float ReadFloat()
	{
		float n =0.0f;
		Read(n);
		return n;
	}
	double ReadDouble()
	{
		double n =0.0f;
		Read(n);
		return n;
	}

	template<typename Type>		//数组
	uint32_t ReadArray(Type *pData, const uint32_t& nSize)
	{
		//读取数组元素个数
		uint32_t nNumber = 0;
		Read(nNumber, false);	//先不偏移，全部成功再偏移
		//判断能不能放下
		if (nNumber < nSize)
		{
			//计算读取的字节数
			auto nLen = nNumber * sizeof(Type);
			//判断能不能读取
			if (CanRead(sizeof(uint32_t) + nLen))
			{
				//偏移<之前没有偏移的>数组元素个数大小
				m_nReadPos += sizeof(uint32_t);
				//数据拷贝到缓冲区尾部
				memcpy(pData, m_pBuffer + m_nReadPos, nLen);
				//更新尾部位置
				m_nReadPos += nLen;
			}
			return nNumber;
		}
		return 0;
	}
	int32_t ReadArraySize()
	{
		int32_t n =0;
		Read(n, false);
		return n;
	}

////写入数据
	template<typename Type>		//基本类型
	bool Write(const Type &n)
	{
		//计算写入数据的大小
		size_t nLen = sizeof(Type);
		//判断能不能写入
		if (CanWrite(nLen))
		{
			//数据拷贝到缓冲区尾部
			memcpy(m_pBuffer + m_nWritePos, &n, nLen);
			//更新尾部位置
			m_nWritePos += nLen;
			return true;
		}
		return false;
	}
	bool WriteInt8(int8_t n)
	{
		return Write(n);
	}
	bool WriteInt16(int16_t n)		
	{
		return Write(n);
	}
	bool WriteInt32(int32_t n)
	{
		return Write(n);
	}
	bool WriteFloat(float n)
	{
		return Write(n);
	}
	bool WriteDouble(double n)
	{
		return Write(n);
	}

	template<typename Type>		//数组
	bool WriteArray(Type *pData, const uint32_t &len)
	{
		//计算写入数据的大小
		size_t nLen = sizeof(Type) * len;
		//判断能不能写入
		if (CanWrite(nLen + sizeof(uint32_t)))
		{
			//写入数组大小[元素个数]，方便读取时获取
			WriteInt32(len);
			//数据拷贝到缓冲区尾部
			memcpy(m_pBuffer + m_nWritePos, pData, nLen);
			//更新尾部位置
			m_nWritePos += nLen;
			return true;
		}
		return false;
	}

private:
	char *m_pBuffer;		//数据缓冲区
	size_t m_nSize;			//缓冲区总空间大小
	size_t m_nWritePos;		//缓冲区当前写入数据位置
	size_t m_nReadPos;		//缓冲区当前读取数据位置
	bool m_bDelete;			//是否释放数据缓冲区
};

//消息接收流
class CellRecvStream : public CellStream
{
public:
	CellRecvStream(DataHeader *header)
		:CellStream((char*)header, header->dataLength)
	{
		SetWritePos(header->dataLength);
		//预读取消息流头部位置的长度数据
		m_nLength = ReadInt32();
	}

private:
	uint32_t m_nLength;

};

//消息发送流
class CellSendStream : public CellStream
{
public:
	CellSendStream(int nSize = 1024)
		:CellStream(nSize)
	{
		//先占一个4字节，用来最后储存这个流的长度
		Write<uint32_t>(0);
	}

	CellSendStream(char *pData, int nSize, bool bDelete = false)
		:CellStream(pData, nSize, bDelete)
	{
		//先占一个4字节，用来最后储存这个流的长度
		Write<uint32_t>(0);
	}

	void Finsh()
	{
		//将该消息流的长度写入头部位置
		int pos = GetWritePos();
		SetWritePos(0);
		WriteInt32(pos);
		SetWritePos(pos);
	}

	//写入字符串
	bool WriteString(const char* str, int len)
	{
		return WriteArray(str, len);
	}
	bool WriteString(const char* str)
	{
		return WriteArray(str, strlen(str));
	}
	bool WriteString(const std::string &str)
	{
		return WriteArray(str.c_str(), str.length());
	}


private:

};


#endif	//	!_CELL_STREAM_HPP_