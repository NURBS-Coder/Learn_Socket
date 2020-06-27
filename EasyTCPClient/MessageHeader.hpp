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
	CMD_NEW_USER_JOIN	//���û�����
};
//��ͷ
struct DataHeader							//�÷��������뱣֤�ֽ���һ�£�����������������ͬƽ̨��������
{
	short cmd;
	short dataLength;
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

#endif  //_MessageHeader_hpp_