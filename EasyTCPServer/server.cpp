#include "EasyTcpServer.hpp"

int main()
{
	EasyTcpServer server;
	//server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(2);

	while (true)
	{
// 3.用户输入请求命令 scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.处理请求 strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			server.Close();
			printf("退出cmdThread线程\n"); 
			break;
		}
		else
		{
			printf("不支持的命令。\n");
		}
	}

	
	printf("服务器已关闭，任务结束。\n");
	getchar();
	getchar();
	return 0;
}