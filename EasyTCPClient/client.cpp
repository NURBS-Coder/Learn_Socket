#include "EasyTcpClient.hpp"
#include <thread>					//c++��׼�߳̿�

void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
// 3.�û������������� scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.�������� strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			client->Close();
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
			printf("��֧�ֵ����\n");
		}
	}
}

int main()
{
	EasyTcpClient client;
	client.Connect("127.0.0.1",4567);

	//�����߳� thread
	thread t1(cmdThread, &client);
	t1.detach();	//�����߳�

	while (client.isRun())
	{
		client.OnRun();
	}

	client.Close();

	printf("���˳����ͻ������������\n");
	getchar();
	getchar();
	return 0;
}