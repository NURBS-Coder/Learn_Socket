#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>			//������WinSock1.h�����ܷ�ǰ��,����Ҫ�����

#pragma comment(lib,"ws2_32.lib")	//��Ӷ�̬��

int main()
{
	WORD ver = MAKEWORD(2,2);	//�����汾��
	WSADATA dat;
	WSAStartup(ver, &dat);		//����socket��ض�̬���ӿ�
	//---------------------


	//���Windows socket����
	WSACleanup();
	return 0;
}