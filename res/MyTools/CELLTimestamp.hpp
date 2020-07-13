#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono>

class CELLTime
{
public:
	CELLTime(){}
	~CELLTime(){}
public:
	static time_t GetNowInMilliSec()
	{
		//��ȡ��ǰʱ��
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}
private:

};

class CELLTimestamp
{
public:
	CELLTimestamp() : m_begin(std::chrono::high_resolution_clock::now()) {}
    void reset() 
	{ 
		m_begin = std::chrono::high_resolution_clock::now(); 
	}
    //Ĭ���������[const�ں����������ʾ�ǳ���Ա�������ú��������޸Ķ����ڵ��κγ�Ա��ֻ�ܷ��������������ܷ���д����]
    int64_t elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
	double getElapsedTimeInMilliSec() const
	{
		return this->elapsed_nano() * 0.000001;
	}
    //΢��
    int64_t elapsed_micro() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    } 
	double getElapsedTimeInMicroSec() const
	{
		return this->elapsed_nano() * 0.001;
	}
    //����
    int64_t elapsed_nano() const
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
    //��
    int64_t elapsed_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
	double getElapsedTimeInSecond() const
	{
		return this->elapsed_nano() * 0.000000001;
	}
    //��
    int64_t elapsed_minutes() const
    {
        return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
    //ʱ
    int64_t elapsed_hours() const
    {
        return std::chrono::duration_cast<std::chrono::hours>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
};

#endif //!_CELLTimestamp_hpp_