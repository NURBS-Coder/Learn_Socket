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
// 3.�û������������� scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.�������� strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			server.Close();
			printf("�˳�cmdThread�߳�\n"); 
			break;
		}
		else
		{
			printf("��֧�ֵ����\n");
		}
	}

	
	printf("�������ѹرգ����������\n");
	getchar();
	getchar();
	return 0;
}