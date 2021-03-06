#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

//--------------------------
//网络数据报文，包头和包体
//包头：本次消息包大小
//包体：数据
//--------------------------
//命令列表
enum CMD
{
	CMD_ERROR,
	CMD_LOGIN,			//登陆
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,			//登出
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,	//新用户进入
	CMD_HEART_C2S,
	CMD_HEART_S2C
};
//包头
struct DataHeader							//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	DataHeader()
	{
		cmd = CMD_ERROR;
		dataLength = sizeof(DataHeader);
	}
	short cmd;
	int dataLength;
};
//包体
struct Login: public DataHeader				//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	Login()				//继承数据头，并初始化
	{
		cmd = CMD_LOGIN;
		dataLength = sizeof(Login);
	}
	char userName[32];
	char passWord[32];
	//char data[932];
	//char data_more[300000];
};
struct LoginResult: public DataHeader		//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	LoginResult()		//继承数据头，并初始化
	{
		cmd = CMD_LOGIN_RESULT;
		dataLength = sizeof(LoginResult);
		result=0;
	}
	int result;
	//char data[992];
	//char data_more[300000];
};
struct Logout: public DataHeader			//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	Logout()			//继承数据头，并初始化
	{
		cmd = CMD_LOGOUT;
		dataLength = sizeof(Logout);
	}
	char userName[32];
};
struct LogoutResult: public DataHeader		//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	LogoutResult()		//继承数据头，并初始化
	{
		cmd = CMD_LOGOUT_RESULT;
		dataLength = sizeof(LogoutResult);
		result=0;
	}
	int result;
};
struct NewUserJoin: public DataHeader		//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	NewUserJoin()		//继承数据头，并初始化
	{
		cmd = CMD_NEW_USER_JOIN;
		dataLength = sizeof(NewUserJoin);
		sock = 0;
	}
	int sock;
};

struct Heart_C2S: public DataHeader		//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	Heart_C2S()		//继承数据头，并初始化
	{
		cmd = CMD_HEART_C2S;
		dataLength = sizeof(Heart_C2S);
	}
};

struct Heart_S2C: public DataHeader		//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	Heart_S2C()		//继承数据头，并初始化
	{
		cmd = CMD_HEART_S2C;
		dataLength = sizeof(Heart_S2C);
	}
};

#endif  //_MessageHeader_hpp_