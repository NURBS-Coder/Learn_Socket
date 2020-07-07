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
const int tCount =1;
//Socket�ͻ�������
const int cCount =1000;
//const int cCount =  FD_SETSIZE - 1 ;		//Ĭ��Windows��select����ģ��ֻ��FD_SETSIZE��Socket����
EasyTcpClient* client[cCount];
atomic<int> sendCount = 0;
atomic<int> readyCount = 0;

void sendThread(int id)
{
	//4���߳� ID 1~4

	printf("thread<%d>,Start\n", id);
	int c = cCount / tCount;
	int begin = c * (id-1);
	int end = c * id;

	for (int i = begin; i < end; i++)
	{
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1",4567);
	}

	printf("thread<%d>,Connect=<begin=%d,end=%d>\n", id ,begin ,end);

	readyCount++;
	while (readyCount < tCount)
	{	//�ȴ������̶߳�������ϣ��ſ�ʼ������
		chrono::milliseconds t(10);
		this_thread::sleep_for(t);
	}

	const int n = 10;
	Login login[n];
	for (int i = 0; i < n; i++)
	{
		strcpy(login[i].userName,"GK");
		strcpy(login[i].passWord,"GKmm");
	}
			
	int nlen = sizeof(login);
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			
			client[i]->SendData(login, nlen);
			client[i]->OnRun();
			sendCount++;
		}

		//Sleep(50);
	}

	for (int i = begin; i < end; i++)
	{
		client[i]->Close();
		delete client[i];
	}

	printf("thread<%d>,Exit\n", id);
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

	while (readyCount < tCount)
	{	//�ȴ������̶߳�������ϣ��ſ�ʼ������
		chrono::milliseconds t(10);
		this_thread::sleep_for(t);
	}

	CELLTimestamp timer;
	while (g_bRun)
	{
		auto t = timer.getElapsedTimeInSecond();
		if (t >= 1.0)
		{
			printf("thread<%d>,time<%lfs>,clientCount<%d>,SendCount<%d>\n",tCount,t,cCount,sendCount);
			timer.reset();
			sendCount = 0;
		}

		Sleep(100);
	}

	printf("���˳����ͻ������������\n");
	getchar();
	getchar();
	return 0;
}