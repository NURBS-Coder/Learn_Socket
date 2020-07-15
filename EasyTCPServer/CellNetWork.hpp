#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_

#ifdef _WIN32
	#define FD_SETSIZE      2506

	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//里面有WinSock1.h，不能放前面,否则要定义 WIN32_LEAN_AND_MEAN 宏
	#pragma comment(lib,"ws2_32.lib")	//添加动态库
#else

#endif

class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		//---------------------
		//启动WinSock2.x网络环境
		WORD ver = MAKEWORD(2,2);	//创建版本号
		WSADATA dat;
		WSAStartup(ver, &dat);		//加载socket相关动态链接库

		CellLog::Instance().Info("启动WinSock2.x网络环境...\n");
#endif
	}

	~CellNetWork()
	{
#ifdef _WIN32
		//---------------------
		//清除Windows socket环境
		WSACleanup();
		CellLog::Instance().Info("清除Windows socket环境...\n");
#endif
	}


public:
	static CellNetWork& Instance()
	{
		static CellNetWork network;
		return network;
	}

private:

};

#endif	//	! _CELL_NET_WORK_HPP_