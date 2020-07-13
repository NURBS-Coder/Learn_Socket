#include "EasyTcpClient.hpp"
#include <thread>					//c++标准线程库


bool g_bRun = true;
void cmdThread(EasyTcpClient * client)
{
	while (true)
	{
// 3.用户输入请求命令 scanf
		char cmdBuf[128] = {};
		//strcpy(cmdBuf,"logout");
		scanf("%s",cmdBuf);
// 4.处理请求 strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
// 5.向服务器发送请求 send
			//发送数据包（包头+包体）
			Login login;
			strcpy(login.userName,"GK");
			strcpy(login.passWord,"GKmm");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
// 5.向服务器发送请求 send
			//发送数据包（包头+包体）
			Logout logout;
			strcpy(logout.userName,"GK");
			client->SendData(&logout);
		}
		else
		{
			//printf("不支持的命令。\n");
		}

		Sleep(40);
	}
}

//发送线程数量
const int tCount =1;
//Socket客户端数量
const int cCount =10;
//const int cCount =  FD_SETSIZE - 1 ;		//默认Windows下select网络模型只有FD_SETSIZE个Socket连接
EasyTcpClient* client[cCount];

atomic<int> readyCount = 0;

//接收线程
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
	//4个线程 ID 1~4

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
	{	//等待其他线程都连接完毕，才开始发数据
		chrono::milliseconds t(10);
		this_thread::sleep_for(t);
	}

	//启动接收线程
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
		//发送延迟
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
	//启动UI线程
	//thread tUI(cmdThread,);


	//启动发送thread线程 
	for (int i = 0; i < tCount; i++)
	{
		thread t(sendThread, i+1);
		t.detach();
	}

	while (readyCount < tCount)
	{	//等待其他线程都连接完毕，才开始发数据
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

	printf("已退出，客户端任务结束。\n");
	getchar();
	getchar();
	return 0;
}