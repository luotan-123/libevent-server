#pragma once

#include "Interface.h"
#include "basemessage.h"
#include "Function.h"

//处理类型定义
#define HD_SOCKET_READ					1							//SOCKET 读取事件处理			
#define HD_SOCKET_CLOSE					2							//SOCKET 关闭事件处理			
#define HD_ASYN_THREAD_RESULT			3							//异步线程结果处理
#define HD_TIMER_MESSAGE				4							//定时器消息处理
#define HD_PLATFORM_SOCKET_READ			5							//中心服读取事件处理

#pragma pack(1)

//数据队列信息头
struct DataLineHead
{
	UINT						uSize;								//数据大小
	UINT						uDataKind;							//数据类型
	DataLineHead()
	{
		uSize = 0;
		uDataKind = 0;
	}
};

///异步线程结果消息结构定义
struct AsynThreadResultLine
{
	DataLineHead						LineHead;					///队列头
	UINT								uHandleResult;				///结果结果 0:成功，非0失败
	UINT								uHandleKind;				///处理类型 
	UINT								uIndex;						///数据库索引或者玩家索引
	UINT								uMsgID;						///消息id
	AsynThreadResultLine()
	{
		memset(this, 0, sizeof(AsynThreadResultLine));
	}
};

//SOCKET关闭通知结构定义
struct SocketCloseLine
{
	DataLineHead						LineHead;					//队列头
	UINT								uIndex;						//SOCKT 索引
	ULONG								uAccessIP;					//SOCKET IP
	UINT								uConnectTime;				//连接时间
	BYTE								socketType;					//socket类型 enum SocketType

	SocketCloseLine()
	{
		memset(this, 0, sizeof(SocketCloseLine));
	}
};

//SOCKET读取通知结构定义
struct SocketReadLine
{
	DataLineHead						LineHead;					//队列头
	NetMessageHead						netMessageHead;				//数据包头
	UINT								uHandleSize;				//数据包处理大小
	UINT								uIndex;						//SOCKET 索引
	ULONG								uAccessIP;					//SOCKET IP
	void*								pBufferevent;				//bufferevent
	BYTE								socketType;					//socket类型 enum SocketType

	SocketReadLine()
	{
		memset(this, 0, sizeof(SocketReadLine));
	}
};

//定时器消息结构定义
struct ServerTimerLine
{
	DataLineHead						LineHead;					//队列头
	UINT								uTimerID;					//定时器 ID
	ServerTimerLine()
	{
		memset(this, 0, sizeof(ServerTimerLine));
	}
};

//数据库数据包头结构
struct DataBaseLineHead
{
	DataLineHead					dataLineHead;							///队列头
	UINT							uHandleKind;							///处理类型
	UINT							uIndex;									///数据库索引或者玩家索引
	UINT							uMsgID;									///消息id
	DataBaseLineHead()
	{
		memset(this, 0, sizeof(DataBaseLineHead));
	}
};

//发送数据队列包头
struct SendDataLineHead
{
	DataLineHead					dataLineHead;							///队列头
	int								socketIndex;							///socket索引或者文件描述符
	void*							pBufferevent;							///bufferevent		
	SendDataLineHead()
	{
		memset(this, 0, sizeof(SendDataLineHead));
	}
};

// dataline 平台数据头
struct PlatformDataLineHead
{
	DataLineHead lineHead;
	PlatformMessageHead platformMessageHead;
	int	socketIdx;
	PlatformDataLineHead()
	{
		memset(this, 0, sizeof(PlatformDataLineHead));
	}
};

//////////////////////////////////////////////////////////////////////////
// 内部通信相关

// 内部通信验证协议
const int COMMON_VERIFY_MESSAGE = 20191101;

// 中心服认证
struct PlatformCenterServerVerify
{
	int serverType; // 服务器类型   SERVICE_TYPE_LOGON//大厅    SERVICE_TYPE_LOADER//游戏
	int serverID;	// 大厅服是logonID，游戏服是roomID

	PlatformCenterServerVerify()
	{
		memset(this, 0, sizeof(PlatformCenterServerVerify));
	}
};

// 登陆服认证
struct PlatformLogonServerVerify
{
	int roomID;
	char passwd[128];

	PlatformLogonServerVerify()
	{
		memset(this, 0, sizeof(PlatformCenterServerVerify));
	}
};

// 投递soket
struct PlatformSocketInfo
{
	time_t acceptMsgTime;
	char ip[MAX_NUM_IP_ADDR_SIZE];
	unsigned short port;
	int acceptFd;			//自己的socket

	PlatformSocketInfo()
	{
		memset(this, 0, sizeof(PlatformSocketInfo));
	}
};

#pragma pack()

///////////////////////////////////////////////////////////////////////////////