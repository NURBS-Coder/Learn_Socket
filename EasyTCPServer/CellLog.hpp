/*
日志输出管理类
单例模式
*/

#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_

#include <cstdlib>
//#include <cassert>

#include <ctime>
#include "CellTask.hpp"

class CellLog
{
private:
	CellLog()
	{
		m_logFile = nullptr;
		m_Task.Start();
	}
	~CellLog()
	{
		if (nullptr != m_logFile)
		{
			Info("CellLog::~CellLog(m_logFile) success\n");

			m_Task.Close();
			fclose(m_logFile);
			m_logFile = nullptr;
		}
	}

public:
	static CellLog& Instance()
	{
		static CellLog log;
		return log;
	}

	//Info	Debug	Warring		Error
	static void Info(const char* pStr)
	{
		printf(pStr);
		CellLog &pLog = Instance();

		//任务线程
		if (nullptr != pStr && nullptr != pLog.m_logFile)
		{
			pLog.m_Task.AddTask([&pLog, pStr](){	

					auto t = std::chrono::system_clock::now();
					auto tNow = std::chrono::system_clock::to_time_t(t);
					//fprintf(pLog.m_logFile, "%s", ctime(&tNow));
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog.m_logFile, "<%d-%d-%d %2d:%2d:%2d>", now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour+8, now->tm_min, now->tm_sec);

					fprintf(pLog.m_logFile, "%s", "Info:  ");
					fprintf(pLog.m_logFile, "%s", pStr);
					fflush(pLog.m_logFile);
			
			});
		}
	}


	void SetLogPath(const char *logPath, const char *mode)
	{
		if (nullptr != m_logFile)
		{
			fclose(m_logFile);
			m_logFile = nullptr;
			Info("CellLog fclose(old m_logFile) success\n");
		}

		m_logFile = fopen(logPath, mode);
		if (nullptr != m_logFile)
		{
			Info("CellLog::setLogPath(m_logFile) success\n");
			//Info("CellLog::setLogPath success,<%s, %s>\n",logPath, mode);
		}
		else
		{
			Info("CellLog::setLogPath(m_logFile) failed\n");
			//Info("CellLog::setLogPath failed,<%s, %s>\n",logPath, mode);
		}
	}

private:
	FILE *m_logFile;

	CellTaskServer m_Task;
};

#endif	//	!_CELL_LOG_HPP_