#include "EasyTcpServer.hpp"

#include "CellNetWork.hpp"

int main()
{
	CellLog::Instance().SetLogPath("serverLog.txt", "w");

	CellNetWork::Instance();
	

	EasyTcpServer server;
	//server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(1);

	EasyTcpServer server1;
	//server.InitSocket();
	server1.Bind(nullptr, 5555);
	server1.Listen(5);
	server1.Start(1);

	while (true)
	{
// 3.�û������������� scanf
		char cmdBuf[128] = {};
		scanf("%s",cmdBuf);
// 4.�������� strcmp
		if (0 == strcmp(cmdBuf, "exit"))
		{ 
			server.Close();
			server1.Close();
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