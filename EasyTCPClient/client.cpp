#include "EasyTcpClient.hpp"
#include <thread>					//c++��׼�߳̿�

bool g_bRun = true;
void cmdThread(EasyTcpClient * client)
{
	while (true)
	{
// 3.�û������������� scanf
		char cmdBuf[128] = {};
		//strcpy(cmdBuf,"logout");
		scanf("%s",cmdBuf);
// 4.�������� strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
// 5.��������������� send
			//�������ݰ�����ͷ+���壩
			Login login;
			strcpy(login.userName,"GK");
			strcpy(login.passWord,"GKmm");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
// 5.��������������� send
			//�������ݰ�����ͷ+���壩
			Logout logout;
			strcpy(logout.userName,"GK");
			client->SendData(&logout);
		}
		else
		{
			//printf("��֧�ֵ����\n");
		}

		Sleep(40);
	}
}

//�����߳�����
const int tCount = 2;
//Socket�ͻ�������
const int cCount =10;
//const int cCount =  FD_SETSIZE - 1 ;		//Ĭ��Windows��select����ģ��ֻ��FD_SETSIZE��Socket����
EasyTcpClient* client[cCount];


void sendThread(int id)
{
	//4���߳� ID 1~4
	int c = cCount / tCount;
	int begin = c * (id-1);
	int end = c * id;

	for (int i = begin; i < end; i++)
	{
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1",4567);
		printf("Connect=%d\n", i);
	}

	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			Login login;
			strcpy(login.userName,"GK");
			strcpy(login.passWord,"GKmm");
			client[i]->SendData(&login);
		}

		//Sleep(20);
	}

	for (int i = begin; i < end; i++)
	{
		client[i]->Close();
	}
}

int main()
{
	//����UI�߳�
	//thread tUI(cmdThread,);


	//��������thread�߳� 
	for (int i = 0; i < tCount; i++)
	{
		thread t(sendThread, i+1);
		t.detach();
	}

	while (g_bRun)
	{
		Sleep(1000);
	}

	printf("���˳����ͻ������������\n");
	getchar();
	getchar();
	return 0;
}