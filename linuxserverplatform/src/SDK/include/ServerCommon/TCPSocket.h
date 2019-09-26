#pragma once

#include "Lock.h"
#include "Interface.h"
#include "BaseMessage.h"
#include <CommCtrl.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// 常量定义
const int SOCKET_SEND_BUF_SIZE = 128 * 1024;	// TCP发送缓冲区大小
const int SOCKET_RECV_BUF_SIZE = 64 * 1024;		// TCP接收缓冲区大小

// socket事件类型
enum SocketEventType
{
	SOCKET_EVENT_TYPE_RECV = 1,		// 发送事件
	SOCKET_EVENT_TYPE_SEND = 2,		// 接收事件
};

//自定义重叠结构
struct OverLappedStruct
{
	OVERLAPPED	overLapped;		//重叠结构
	WSABUF		WSABuffer;		//数据缓冲
	int			type;			//类型
	int			handleID;		//处理ID

	OverLappedStruct()
	{
		Clear();
	}

	void Clear()
	{
		memset(this, 0, sizeof(OverLappedStruct));
	}
};

//////////////////////////////////////////////////////////////////////
class KERNEL_CLASS CListenSocket
{
public:
	CListenSocket();
	~CListenSocket();

public:
	bool Init(int port, const char *ip = NULL);
	bool CloseSocket();
	SOCKET GetSocket() { return m_hSocket; }

protected:
	SOCKET	m_hSocket;
};

//////////////////////////////////////////////////////////////////////
class CTCPSocketManage;
class CDataLine;
class KERNEL_CLASS CTCPSocket
{
public:
	CTCPSocket();
	~CTCPSocket();

public:
	// 初始化(服务启动时主线程调用)
	bool Init(int index, CTCPSocketManage* pManage);
	// 清空非永久属性(初始化或者重新启用一个对象调用)
	void Clear();
	// 建立连接性(接收连接线程调用)
	bool AcceptNewConnection(SOCKET sock, unsigned int handleID, HANDLE hCompletePort, unsigned long ip);
	// 投递recv请求
	bool PostRecv();
	// 投递send请求
	bool PostSend();

public:
	// 发送数据(业务逻辑线程调用)
	bool SendData(const char* pData, int size);
	// 是否连接
	inline bool IsConnect();
	//发送完成函数(收发线程调用)
	bool OnSendCompleted(unsigned int handleID, int sendedBytes);
	//接收完成函数(收发线程调用)
	bool OnRecvCompleted(unsigned int handleID, int recvedBytes);
	// 关闭连接(只能业务逻辑线程调用)
	bool CloseSocket();

private:
	// 派发数据包(收发线程调用)
	bool DispatchPacket(NetMessageHead* pHead, void* pData, int size);

public:
	// 获取处理标识
	inline unsigned int GetHandleID() { return m_handleID; }
	// 获取对象数组索引
	inline int GetIndex() { return m_index; }
	// 获取上次发送消息时间
	inline time_t GetLastRecvMsgTime() { return m_lastRecvMsgTime; }
	// 获取建立连接的时间
	inline time_t GetAcceptMsgTime() { return m_acceptMsgTime; }
	// 获取锁
	inline CSignedLock* GetLock() { return &m_lock; }
	// 获取IP
	inline const char* GetIP() { return m_ip; }

private:
	// 永久属性(构造对象时候确定)
	CTCPSocketManage*	m_pManage;			// 管理对象
	int					m_index;			// 所在数组索引

	// 非永久属性
	SOCKET				m_socket;
	unsigned int		m_handleID;			// 处理ID
	char				m_ip[24];			// 客户端自己的ip

	char				m_recvBuf[SOCKET_RECV_BUF_SIZE];
	int					m_recvBufLen;

	char				m_sendBuf[SOCKET_SEND_BUF_SIZE];
	int					m_sendBufLen;
	bool				m_bSending;							//是否发送中

	OverLappedStruct	m_recvOverLapped;	// 接收重叠结构
	OverLappedStruct	m_sendOverLapped;	// 发送重叠结构

	volatile time_t		m_lastRecvMsgTime;	// 上次发送消息的时间(用来做心跳)
	volatile time_t		m_acceptMsgTime;	// 建立连接的时间

	CSignedLock		m_lock;
};


/**************************************************************************************************************/
// TCP SOCKET 网络部分 
/**************************************************************************************************************/
//类说明
class CTCPSocket;
class CTCPSocketManage;
/**************************************************************************************************************/
#define NO_CONNECT				0								//没有连接
#define CONNECTING				1								//正在连接
#define CONNECTED				2								//成功连接

/**************************************************************************************************************/
//CloseSocket和OnSocketClose区别：
//1、调用CloseSocket无法执行到CCenterServerManage::OnSocketClose逻辑。
//2、OnSocketClose可能会执行两次CCenterServerManage::OnSocketClose逻辑，但是会保证关闭socket之前，消息发送出去
class KERNEL_CLASS CTCPSocketManage
{
public:
	CTCPSocketManage();
	~CTCPSocketManage();

public:
	// 初始化
	bool Init(IServerSocketService * pService, int maxCount, int port, int secretKey, const char *ip = NULL);
	// 取消初始化
	virtual bool UnInit();
	// 开始服务
	virtual bool Start(int serverType);
	// 停止服务
	virtual bool Stop();

public:
	// 发送数据函数
	bool SendData(int idx, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int handleID, unsigned int uIdentification = 0);
	// 中心服务器发送数据
	bool CenterServerSendData(int idx, UINT msgID, void* pData, int size, int mainID, int assistID, int handleCode, int userID);
	// 批量发送函数
	int SendDataBatch(void * pData, UINT uSize, UINT uMainID, UINT bAssistantID, UINT uHandleCode);
	// 关闭连接(业务逻辑线程调用)
	bool CloseSocket(int index);
	// 接收关闭连接事件。
	bool OnSocketClose(int index);
	// 检查心跳
	void CheckHeartBeat(time_t llLastSendHeartBeatTime, int iHeartBeatTime);
	// 获取dataline
	CDataLine* GetDataLine();
	// 获取连接ip
	const char* GetSocketIP(int socketIdx);
	// 获取CTCPSocket属性
	void GetTCPSocketInfo(UINT uIndex, bool &bIsConnect, time_t &llAcceptMsgTime, time_t &llLastRecvMsgTime);
private:
	// 分配一个可用的socket(线程环境：ThreadAccept)
	CTCPSocket * TCPSocketNew();
	// 获取处理ID(线程环境：ThreadAccept)
	static unsigned int GetHandleID();

	// 线程函数
private:
	// SOCKET 连接应答线程
	static unsigned __stdcall ThreadAccept(LPVOID pThreadData);
	// SOCKET 数据接收线程
	static unsigned __stdcall ThreadRSSocket(LPVOID pThreadData);

private:
	CListenSocket				m_listenSocket;
	IServerSocketService*		m_pService;
	std::vector<CTCPSocket*>	m_socketVec;
	UINT	m_uRSThreadCount;
	HANDLE	m_hEventThread;
	HANDLE	m_hCompletionPortRS;
	HANDLE	m_hThreadAccept;
	volatile bool	m_running;
	UINT	m_uMaxSocket;

public:
	unsigned int	m_iServiceType;
};
