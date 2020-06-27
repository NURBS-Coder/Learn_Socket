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
	CMD_NEW_USER_JOIN	//新用户进入
};
//包头
struct DataHeader							//该方法，必须保证字节序一致（二进制流），否则不同平台会有问题
{
	short cmd;
	short dataLength;
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

#endif  //_MessageHeader_hpp_