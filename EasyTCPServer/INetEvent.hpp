#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_

#include "CellClient.hpp"


//网络事件接口
class INetEvent		//接口类
{
public:
	INetEvent(){}
	~INetEvent(){}
public:
	//纯虚函数，只有声明，继承时实现
	//客户端加入事件,1线程调用
	virtual void OnNetJoin(CellClientPtr& pClient) = 0; 
	//客户端离开事件,4线程调用
	virtual void OnNetLeave(CellClientPtr& pClient) = 0; 
	//响应网络接口事件,4线程调用
	virtual void OnNetMsg(CellClientPtr& pClient, DataHeader* header) = 0; 
	//响应网络接口事件,4线程调用
	virtual void OnNetRecv(CellClientPtr& pClient) = 0; 
private:

};



#endif	//	!_I_NET_EVENT_HPP_