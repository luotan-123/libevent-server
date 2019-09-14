#pragma once

///常量定义
#define MAX_SEND_SIZE			2044								//最大消息包
#define NET_HEAD_SIZE			sizeof(NetMessageHead)				//数据包头

///支持类型定义
#define SUP_NORMAL_GAME			0x01								///普通游戏
#define SUP_MATCH_GAME			0x02								///比赛游戏
#define SUP_MONEY_GAME			0x04								///金币游戏


/********************************************************************************************/
//	系统内核使用 （MAINID 使用 0 - 49 ）
/********************************************************************************************/

///特殊消息
#define MDM_NO_FAILE					0			///不失败消息

///连接消息
#define MDM_CONNECT						1			///连接消息类型
///辅助处理消息标志
#define ASS_NET_TEST					1			///网络测试
#define ASS_CONNECT_SUCCESS 			3			///连接成功

/********************************************************************************************/
#pragma pack(1)
//网络数据包结构头
/*
关于uIdentification的说明：
1、前端 -------> logonserver 或 logonserver -------> 前端， uIdentification = roomID
2、logonserver -----> gameserver 或 gameserver -----> logonserver，uIdentification = userID
*/
struct NetMessageHead
{
	UINT						uMessageSize;						///数据包大小
	UINT						uMainID;							///处理主类型
	UINT						uAssistantID;						///辅助处理类型 ID
	UINT						uHandleCode;						///数据包处理代码
	UINT						uIdentification;					///身份标识（不同的协议里面有不同的含义）

	NetMessageHead()
	{
		memset(this, 0, sizeof(NetMessageHead));
	}
};

//中心服包头
struct CenterServerMessageHead
{
	UINT msgID;				// ID
	int userID;				// 用户ID或者俱乐部id
	CenterServerMessageHead()
	{
		memset(this, 0, sizeof(CenterServerMessageHead));
	}
};

// 中心服务器消息头
struct PlatformMessageHead
{
	NetMessageHead				MainHead;	//主包头
	CenterServerMessageHead		AssHead;	//副包头

	PlatformMessageHead()
	{
		memset(this, 0, sizeof(PlatformMessageHead));
	}
};

#pragma pack()