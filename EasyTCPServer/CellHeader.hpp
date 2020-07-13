#ifndef _CELL_HEADER_HPP_
#define _CELL_HEADER_HPP_

#ifdef _WIN32
	#define FD_SETSIZE      2506

	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//里面有WinSock1.h，不能放前面,否则要定义 WIN32_LEAN_AND_MEAN 宏
	#pragma comment(lib,"ws2_32.lib")	//添加动态库
#else

#endif // _WIN32

#include <stdio.h>				//printf()....
#include <iostream>				//std::cout....
#include <vector>
#include <list>
#include <thread>				//c++标准线程库
#include <mutex>				//锁定义库
#include <atomic>				//原子操作库
#include <memory>

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef SEND_BUFF_SIZE
	#define SEND_BUFF_SIZE 1024		//发送缓冲区最小单元大小 40kb
#endif

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024		//接收缓冲区最小单元大小 40kb
#endif



#endif	// !_CELL_HEADER_HPP_
