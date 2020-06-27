#include "EasyTcpClient.hpp"
#include <thread>					//c++标准线程库

void cmdThread(EasyTcpClient* client)
{
	while (true)
	{
// 3.用户输入请求命令 scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.处理请求 strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			client->Close();
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
			printf("不支持的命令。\n");
		}
	}
}

int main()
{
	EasyTcpClient client;
	client.Connect("127.0.0.1",4567);

	//启动线程 thread
	thread t1(cmdThread, &client);
	t1.detach();	//分离线程

	while (client.isRun())
	{
		client.OnRun();
	}

	client.Close();

	printf("已退出，客户端任务结束。\n");
	getchar();
	getchar();
	return 0;
}