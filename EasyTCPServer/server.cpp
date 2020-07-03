#include "EasyTcpServer.hpp"
#include <thread>					//c++标准线程库

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
// 3.用户输入请求命令 scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.处理请求 strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else
		{
			printf("不支持的命令。\n");
		}
	}
}

int main()
{
	EasyTcpServer server;
	//server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);

	//启动线程 发送thread
	thread t1(cmdThread);
	t1.detach();	//分离线程
	
	while (g_bRun)
	{
		server.OnRun();
	}

	server.Close();

	printf("服务器已关闭，任务结束。\n");
	getchar();
	getchar();
	return 0;
}