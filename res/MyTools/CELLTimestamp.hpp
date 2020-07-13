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
		//获取当前时间
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
    //默认输出毫秒[const在函数名后面表示是常成员函数，该函数不能修改对象内的任何成员，只能发生读操作，不能发生写操作]
    int64_t elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
	double getElapsedTimeInMilliSec() const
	{
		return this->elapsed_nano() * 0.000001;
	}
    //微秒
    int64_t elapsed_micro() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    } 
	double getElapsedTimeInMicroSec() const
	{
		return this->elapsed_nano() * 0.001;
	}
    //纳秒
    int64_t elapsed_nano() const
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
    //秒
    int64_t elapsed_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
	double getElapsedTimeInSecond() const
	{
		return this->elapsed_nano() * 0.000000001;
	}
    //分
    int64_t elapsed_minutes() const
    {
        return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
    //时
    int64_t elapsed_hours() const
    {
        return std::chrono::duration_cast<std::chrono::hours>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
};

#endif //!_CELLTimestamp_hpp_