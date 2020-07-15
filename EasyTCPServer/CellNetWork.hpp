#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_

#ifdef _WIN32
	#define FD_SETSIZE      2506

	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ���� WIN32_LEAN_AND_MEAN ��
	#pragma comment(lib,"ws2_32.lib")	//��Ӷ�̬��
#else

#endif

class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		//---------------------
		//����WinSock2.x���绷��
		WORD ver = MAKEWORD(2,2);	//�����汾��
		WSADATA dat;
		WSAStartup(ver, &dat);		//����socket��ض�̬���ӿ�

		CellLog::Instance().Info("����WinSock2.x���绷��...\n");
#endif
	}

	~CellNetWork()
	{
#ifdef _WIN32
		//---------------------
		//���Windows socket����
		WSACleanup();
		CellLog::Instance().Info("���Windows socket����...\n");
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