/*
	�����ݹ�������
*/

#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_

#include "MessageHeader.hpp"

#include <cstdint>
#include <cstring>

//�ֽ���BYTE��ͳһ��ͬϵͳ�е����ݴ���
class CellStream
{
public:
	//�Լ������ڴ湹�죨һ����д����������
	CellStream(int nSize = 1024)
	{
		m_nSize = nSize;
		m_pBuffer = new char[m_nSize];
		m_bDelete = true;
		m_nWritePos = 0;
		m_nReadPos = 0;
	}

	//�����ڴ洫�빹�죨һ���Ƕ�ȡ��������
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


	//����滻������<�в������з���ֵ�������ͼ��>
	//��ν"��������"���ǽ��ܼ򵥵ĺ���"��Ƕ"���������ĳ��������,��Լʱ�տ���
	//���������ʮ�ּ�,���ܺ���ѭ����������ѡ��ȸ��ӵĽṹ������Ͳ�����Ϊ��������
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

////��ȡ����
	template<typename Type>		//��������
	bool Read(Type& n, const bool &bOffSet = true)
	{
		//�����ȡ���ݵĴ�С
		size_t nLen = sizeof(Type);
		//�ж��ܲ��ܶ�ȡ
		if (CanRead(nLen))
		{
			//���������ݿ�������
			memcpy(&n, m_pBuffer + m_nReadPos, nLen);
			//����β��λ��
			if (bOffSet)
			{	//Ĭ��ƫ��
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

	template<typename Type>		//����
	uint32_t ReadArray(Type *pData, const uint32_t& nSize)
	{
		//��ȡ����Ԫ�ظ���
		uint32_t nNumber = 0;
		Read(nNumber, false);	//�Ȳ�ƫ�ƣ�ȫ���ɹ���ƫ��
		//�ж��ܲ��ܷ���
		if (nNumber < nSize)
		{
			//�����ȡ���ֽ���
			auto nLen = nNumber * sizeof(Type);
			//�ж��ܲ��ܶ�ȡ
			if (CanRead(sizeof(uint32_t) + nLen))
			{
				//ƫ��<֮ǰû��ƫ�Ƶ�>����Ԫ�ظ�����С
				m_nReadPos += sizeof(uint32_t);
				//���ݿ�����������β��
				memcpy(pData, m_pBuffer + m_nReadPos, nLen);
				//����β��λ��
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

////д������
	template<typename Type>		//��������
	bool Write(const Type &n)
	{
		//����д�����ݵĴ�С
		size_t nLen = sizeof(Type);
		//�ж��ܲ���д��
		if (CanWrite(nLen))
		{
			//���ݿ�����������β��
			memcpy(m_pBuffer + m_nWritePos, &n, nLen);
			//����β��λ��
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

	template<typename Type>		//����
	bool WriteArray(Type *pData, const uint32_t &len)
	{
		//����д�����ݵĴ�С
		size_t nLen = sizeof(Type) * len;
		//�ж��ܲ���д��
		if (CanWrite(nLen + sizeof(uint32_t)))
		{
			//д�������С[Ԫ�ظ���]�������ȡʱ��ȡ
			WriteInt32(len);
			//���ݿ�����������β��
			memcpy(m_pBuffer + m_nWritePos, pData, nLen);
			//����β��λ��
			m_nWritePos += nLen;
			return true;
		}
		return false;
	}

private:
	char *m_pBuffer;		//���ݻ�����
	size_t m_nSize;			//�������ܿռ��С
	size_t m_nWritePos;		//��������ǰд������λ��
	size_t m_nReadPos;		//��������ǰ��ȡ����λ��
	bool m_bDelete;			//�Ƿ��ͷ����ݻ�����
};

//��Ϣ������
class CellRecvStream : public CellStream
{
public:
	CellRecvStream(DataHeader *header)
		:CellStream((char*)header, header->dataLength)
	{
		SetWritePos(header->dataLength);
		//Ԥ��ȡ��Ϣ��ͷ��λ�õĳ�������
		m_nLength = ReadInt32();
	}

private:
	uint32_t m_nLength;

};

//��Ϣ������
class CellSendStream : public CellStream
{
public:
	CellSendStream(int nSize = 1024)
		:CellStream(nSize)
	{
		//��ռһ��4�ֽڣ�������󴢴�������ĳ���
		Write<uint32_t>(0);
	}

	CellSendStream(char *pData, int nSize, bool bDelete = false)
		:CellStream(pData, nSize, bDelete)
	{
		//��ռһ��4�ֽڣ�������󴢴�������ĳ���
		Write<uint32_t>(0);
	}

	void Finsh()
	{
		//������Ϣ���ĳ���д��ͷ��λ��
		int pos = GetWritePos();
		SetWritePos(0);
		WriteInt32(pos);
		SetWritePos(pos);
	}

	//д���ַ���
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