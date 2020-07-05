#include "EasyTcpServer.hpp"
#include <thread>					//c++��׼�߳̿�

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
// 3.�û������������� scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.�������� strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else
		{
			printf("��֧�ֵ����\n");
		}
	}
}

int main()
{
	EasyTcpServer server;
	//server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);

	//�����߳� ����thread
	thread t1(cmdThread);
	t1.detach();	//�����߳�
	
	while (g_bRun)
	{
		server.OnRun();
	}

	server.Close();

	printf("�������ѹرգ����������\n");
	getchar();
	getchar();
	return 0;
}