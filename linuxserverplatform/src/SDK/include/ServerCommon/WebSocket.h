#pragma once

#include "TCPSocket.h"


class CWebSocketManage :public CTCPSocketManage
{
public:
	CWebSocketManage();
	~CWebSocketManage();


public:
	// 发送数据函数
	virtual bool SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, void* pBufferevent, unsigned int uIdentification = 0);

protected:
	// 解析数据函数
	virtual bool RecvData(bufferevent* bev, int index);

public: //websocket相关函数







};
