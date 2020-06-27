#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>			//里面有WinSock1.h，不能放前面,否则要定义宏

#pragma comment(lib,"ws2_32.lib")	//添加动态库

int main()
{
	WORD ver = MAKEWORD(2,2);	//创建版本号
	WSADATA dat;
	WSAStartup(ver, &dat);		//加载socket相关动态链接库
	//---------------------


	//清楚Windows socket环境
	WSACleanup();
	return 0;
}