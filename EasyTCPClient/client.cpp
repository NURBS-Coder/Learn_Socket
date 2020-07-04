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

int main()
{
	const int cCount = 1;
	//const int cCount =  FD_SETSIZE - 1 ;		//默认Windows下select网络模型只有FD_SETSIZE个Socket连接
	EasyTcpClient* client[cCount];

	for (int i = 0; i < cCount; i++)
	{
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1",4567);
	}

	
	//启动线程 发送thread
	thread t1(cmdThread,client[0]);
	t1.detach();	//分离线程

	while (g_bRun)
	{
		for (int i = 0; i < cCount; i++)
		{
			client[0]->OnRun();
		}

		for (int i = 0; i < cCount; i++)
		{
			Login login;
			strcpy(login.userName,"GK");
			strcpy(login.passWord,"GKmm");
			//client[i]->SendData(&login);
		}

		Sleep(20);
	}

	for (int i = 0; i < cCount; i++)
	{
		client[i]->Close();
	}
	

	printf("已退出，客户端任务结束。\n");
	getchar();
	getchar();
	return 0;
}