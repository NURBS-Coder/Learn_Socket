#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

//--------------------------
//�������ݱ��ģ���ͷ�Ͱ���
//��ͷ��������Ϣ����С
//���壺����
//--------------------------
//�����б�
enum CMD
{
	CMD_ERROR,
	CMD_LOGIN,			//��½
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,			//�ǳ�
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,	//���û�����
	CMD_HEART_C2S,
	CMD_HEART_S2C
};
//��ͷ
struct DataHeader							//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	DataHeader()
	{
		cmd = CMD_ERROR;
		dataLength = sizeof(DataHeader);
	}
	short cmd;
	int dataLength;
};
//����
struct Login: public DataHeader				//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	Login()				//�̳�����ͷ������ʼ��
	{
		cmd = CMD_LOGIN;
		dataLength = sizeof(Login);
	}
	char userName[32];
	char passWord[32];
	//char data[932];
	//char data_more[300000];
};
struct LoginResult: public DataHeader		//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	LoginResult()		//�̳�����ͷ������ʼ��
	{
		cmd = CMD_LOGIN_RESULT;
		dataLength = sizeof(LoginResult);
		result=0;
	}
	int result;
	//char data[992];
	//char data_more[300000];
};
struct Logout: public DataHeader			//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	Logout()			//�̳�����ͷ������ʼ��
	{
		cmd = CMD_LOGOUT;
		dataLength = sizeof(Logout);
	}
	char userName[32];
};
struct LogoutResult: public DataHeader		//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	LogoutResult()		//�̳�����ͷ������ʼ��
	{
		cmd = CMD_LOGOUT_RESULT;
		dataLength = sizeof(LogoutResult);
		result=0;
	}
	int result;
};
struct NewUserJoin: public DataHeader		//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	NewUserJoin()		//�̳�����ͷ������ʼ��
	{
		cmd = CMD_NEW_USER_JOIN;
		dataLength = sizeof(NewUserJoin);
		sock = 0;
	}
	int sock;
};

struct Heart_C2S: public DataHeader		//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	Heart_C2S()		//�̳�����ͷ������ʼ��
	{
		cmd = CMD_HEART_C2S;
		dataLength = sizeof(Heart_C2S);
	}
};

struct Heart_S2C: public DataHeader		//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	Heart_S2C()		//�̳�����ͷ������ʼ��
	{
		cmd = CMD_HEART_S2C;
		dataLength = sizeof(Heart_S2C);
	}
};

#endif  //_MessageHeader_hpp_