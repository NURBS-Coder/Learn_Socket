#include "EasyTcpClient.hpp"
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
//		else if (0 == strcmp(cmdBuf, "login"))
//		{
//// 5.��������������� send
//			//�������ݰ�����ͷ+���壩
//			Login login;
//			strcpy(login.userName,"GK");
//			strcpy(login.passWord,"GKmm");
//			//client->SendData(&login);
//		}
//		else if (0 == strcmp(cmdBuf, "logout"))
//		{
//// 5.��������������� send
//			//�������ݰ�����ͷ+���壩
//			Logout logout;
//			strcpy(logout.userName,"GK");
//			//client->SendData(&logout);
//		}
		else
		{
			printf("��֧�ֵ����\n");
		}
	}
}

int main()
{
	const int cCount = 10;
	//const int cCount =  FD_SETSIZE - 1 ;		//Ĭ��Windows��select����ģ��ֻ��FD_SETSIZE��Socket����
	EasyTcpClient* client[cCount];

	for (int i = 0; i < cCount; i++)
	{
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1",4567);
	}

	
	//�����߳� ����thread
	thread t1(cmdThread);
	t1.detach();	//�����߳�

	while (g_bRun)
	{
		//for (int i = 0; i < cCount; i++)
		//{
		//	client[i]->OnRun();
		//}
		//
		for (int i = 0; i < cCount; i++)
		{
			Login login;
			strcpy(login.userName,"GK");
			strcpy(login.passWord,"GKmm");
			client[i]->SendData(&login);
		}

		Sleep(50);
	}

	for (int i = 0; i < cCount; i++)
	{
		client[i]->Close();
	}
	

	printf("���˳����ͻ������������\n");
	getchar();
	getchar();
	return 0;
}