#ifndef _CELL_HEADER_HPP_
#define _CELL_HEADER_HPP_

#ifdef _WIN32
	#define FD_SETSIZE      2506

	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ���� WIN32_LEAN_AND_MEAN ��
	#pragma comment(lib,"ws2_32.lib")	//��Ӷ�̬��
#else

#endif // _WIN32

#include <stdio.h>				//printf()....
#include <iostream>				//std::cout....
#include <vector>
#include <list>
#include <thread>				//c++��׼�߳̿�
#include <mutex>				//�������
#include <atomic>				//ԭ�Ӳ�����
#include <memory>

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef SEND_BUFF_SIZE
	#define SEND_BUFF_SIZE 1024		//���ͻ�������С��Ԫ��С 40kb
#endif

#ifndef RECV_BUFF_SIZE
	#define RECV_BUFF_SIZE 1024		//���ջ�������С��Ԫ��С 40kb
#endif



#endif	// !_CELL_HEADER_HPP_
