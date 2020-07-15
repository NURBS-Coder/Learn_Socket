#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include "CellClient.hpp"


//�����¼��ӿ�
class INetEvent		//�ӿ���
{
public:
	INetEvent(){}
	~INetEvent(){}
public:
	//���麯����ֻ���������̳�ʱʵ��
	//�ͻ��˼����¼�,1�̵߳���
	virtual void OnNetJoin(CellClientPtr& pClient) = 0; 
	//�ͻ����뿪�¼�,4�̵߳���
	virtual void OnNetLeave(CellClientPtr& pClient) = 0; 
	//��Ӧ����ӿ��¼�,4�̵߳���
	virtual void OnNetMsg(CellClientPtr& pClient, DataHeader* header) = 0; 
	//��Ӧ����ӿ��¼�,4�̵߳���
	virtual void OnNetRecv(CellClientPtr& pClient) = 0; 
private:

};



#endif	//	!_I_NET_EVENT_HPP_