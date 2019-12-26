#pragma once


/********************************************************************************************/
//	系统内核使用 （MAINID 使用 0 - 49 ）
/********************************************************************************************/
///连接消息
#define MDM_CONNECT						1			///连接消息类型
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

struct WebSocketMsg
{
	uint8_t fin;
	uint8_t opcode;
	uint8_t mask;
	uint8_t maskingKey[4];
	uint32_t dataLength;	//数据包总长度
	uint32_t payloadLength; //数据部分长度，不包含包头
	char* payload;			//数据部分起始地址

	WebSocketMsg()
	{
		Init();
	}

	void Init()
	{
		fin = 0;
		opcode = 0;
		mask = 0;
		maskingKey[0] = 0;
		dataLength = 0;
		payloadLength = 0;
		payload = nullptr;
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