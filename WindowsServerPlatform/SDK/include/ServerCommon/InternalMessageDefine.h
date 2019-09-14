#pragma once

#include "DataBase.h"
#include "basemessage.h"
#include "KernelDefine.h"

//处理类型定义
#define HD_SOCKET_READ					1							//SOCKET 读取事件处理			
#define HD_SOCKET_CLOSE					2							//SOCKET 关闭事件处理			
#define HD_ASYN_THREAD_RESULT			3							//异步线程结果处理
#define HD_TIMER_MESSAGE				4							//定时器消息处理
#define HD_PLATFORM_SOCKET_READ			5							//中心服读取事件处理

//SOCKET关闭通知结构定义
struct SocketCloseLine
{
	DataLineHead						LineHead;					//队列头
	UINT								uIndex;						//SOCKT 索引
	ULONG								uAccessIP;					//SOCKET IP
	UINT								uConnectTime;				//连接时间
};

//SOCKET读取通知结构定义
struct SocketReadLine
{
	DataLineHead						LineHead;					//队列头
	NetMessageHead						NetMessageHead;				//数据包头
	UINT								uHandleSize;				//数据包处理大小
	UINT								uIndex;						//SOCKET 索引
	ULONG								uAccessIP;					//SOCKET IP
	DWORD								dwHandleID;					//SOCKET 处理 ID
};

//定时器消息结构定义
struct WindowTimerLine
{
	DataLineHead						LineHead;					//队列头
	UINT								uTimerID;					//定时器 ID
};

//////////////////////////////////////////////////////////////////////////
// 内部通信相关

// 内部通信验证协议
const int COMMON_VERIFY_MESSAGE = 20190601;

// dataline 平台数据头
struct PlatformDataLineHead
{
	DataLineHead lineHead;
	PlatformMessageHead platformMessageHead;
	int	socketIdx;
};

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

///连接成功消息 
struct MSG_S_ConnectSuccess
{
	BYTE						bMaxVer;							///最新版本号码
	BYTE						bLessVer;							///最低版本号码
	BYTE						bReserve[2];						///保留字段
	UINT						i64CheckCode;						///加密后的校验码，由客户端解密在包头中返回

	MSG_S_ConnectSuccess()
	{
		memset(this, 0, sizeof(MSG_S_ConnectSuccess));
	}
};

//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#define DTK_GP_CHECK_DB_CONNECTION		487						// 检查数据库连接性
#define DTK_GP_SQL_STATEMENT			488						// SQL语句
#define DTK_GP_REGISTR_SEND             489                     // send uid to web
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
struct InternalSqlStatement
{
	DataBaseLineHead	head;
	char	sql[MAX_SQL_STATEMENT_SIZE];

	InternalSqlStatement()
	{
		memset(this, 0, sizeof(InternalSqlStatement));
	}
};

///////////////////////////////////////////////////////////////////////////////