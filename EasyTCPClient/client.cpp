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
const int cCount =10;
//const int cCount =  FD_SETSIZE - 1 ;		//Ĭ��Windows��select����ģ��ֻ��FD_SETSIZE��Socket����
EasyTcpClient* client[cCount];

atomic<int> readyCount = 0;

//�����߳�
void recvThread(int begin, int end)
{
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->OnRun();
		}

		//Sleep(50);
	}
}

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

	//���������߳�
	thread t(recvThread, begin, end);
	t.detach();

	const int n = 10;
	Heart_C2S login[n];
			
	int nlen = sizeof(login);
	while (g_bRun)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->SendData(login, nlen);
		}
		//�����ӳ�
		chrono::milliseconds t(1000);
		this_thread::sleep_for(t);
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
			printf("Ts<%d>,time<%lfs>,Cs<%d>,Ss<%d>,Rs<%d>,Ms<%d>\n",tCount ,t,connectCount,sendCount,recvCount,msgCount);
			timer.reset();
			sendCount = 0;
			recvCount = 0;
			msgCount = 0;
		}

		Sleep(100);
	}

	printf("���˳����ͻ������������\n");
	getchar();
	getchar();
	return 0;
}