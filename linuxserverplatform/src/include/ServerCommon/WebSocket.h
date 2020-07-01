#pragma once

#include "TCPSocket.h"

#define	MIN_WEBSOCKET_HEAD_SIZE			2	//websocket数据包最小包头，实际包头大小2-10
#define	MAX_WEBSOCKET_HEAD_SIZE			10	//websocket数据包最小包头，实际包头大小2-10
#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" // websocket key
#define WS_FRAGMENT_FIN (1 << 7)

enum WEBSOCKET_STATUS {
	WEBSOCKET_UNCONNECT = 0,
	WEBSOCKET_HANDSHARKED = 1,
};

enum WS_FrameType
{
	WS_EMPTY_FRAME = 0xF0,
	WS_ERROR_FRAME = 0xF1,
	WS_TEXT_FRAME = 0x01,
	WS_BINARY_FRAME = 0x02,
	WS_PING_FRAME = 0x09,
	WS_PONG_FRAME = 0x0A,
	WS_OPENING_FRAME = 0xF3,
	WS_CLOSING_FRAME = 0x08
};

struct WebSocketMsg;
class CWebSocketManage
{
public:
	CWebSocketManage();
	virtual ~CWebSocketManage();

public:
	// 初始化
	virtual bool Init(IServerSocketService* pService, int maxCount, int port, const char* ip = NULL, SocketType socketType = SOCKET_TYPE_TCP);
	// 取消初始化
	virtual bool UnInit();
	// 开始服务
	virtual bool Start(int serverType);
	// 停止服务
	virtual bool Stop();

public:
	// 发送数据函数
	virtual bool SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, void* pBufferevent, unsigned int uIdentification = 0);
	// 关闭连接(业务逻辑线程调用)
	bool CloseSocket(int index);
	// 获取接收dataline
	CDataLine* GetRecvDataLine();
	// 获取发送dataline
	CDataLine* GetSendDataLine();
	// 获取当前socket连接总数
	UINT GetCurSocketSize();
	// 判断socket是否连接
	bool IsConnected(int index);
	// 获取socketSet
	void GetSocketSet(std::vector<UINT>& vec);
	// 获取socketVector
	const std::vector<TCPSocketInfo>& GetSocketVector();
	// 获取连接ip
	const char* GetSocketIP(int index);
	// 获取TcpSocketInfo
	const TCPSocketInfo* GetTCPSocketInfo(int index);

protected:
	// 分配socketIndex算法
	int GetSocketIndex();
	// 添加TCPSocketInfo
	void AddTCPSocketInfo(int threadIndex, PlatformSocketInfo* pTCPSocketInfo);
	// 设置tcp为未连接状态
	void RemoveTCPSocketStatus(int index, int closetype = 0);
	// 派发数据包
	bool DispatchPacket(void* pBufferevent, int index, NetMessageHead* pHead, void* pData, int size);
	// 最底层处理收到的数据函数
	virtual bool RecvData(bufferevent* bev, int index);
	//websocket的第一次握手
	bool HandShark(bufferevent* bev, int index);

public: // 设置tcp属性
	// 设置tcp收发缓冲区
	static void SetTcpRcvSndBUF(int fd, int rcvBufSize, int sndBufSize);
	// 设置应用层单次读取数据包的大小 bufferevent_set_max_single_read
	static void SetMaxSingleReadAndWrite(bufferevent* bev, int rcvBufSize, int sndBufSize);

	// 线程函数
protected:
	// SOCKET 连接应答线程
	static void* ThreadAccept(void* pThreadData);
	// SOCKET 数据接收线程
	static void* ThreadRSSocket(void* pThreadData);
	// SOCKET 数据发送线程
	static void* ThreadSendMsg(void* pThreadData);

	// 回调函数
protected:
	// 新的连接到来，ThreadAccept线程函数
	static void ListenerCB(struct evconnlistener*, evutil_socket_t, struct sockaddr*, int socklen, void*);
	// 新的数据到来，ThreadRSSocket线程函数
	static void ReadCB(struct bufferevent*, void*);
	// 连接关闭等等错误消息，ThreadRSSocket线程函数
	static void EventCB(struct bufferevent*, short, void*);
	// accept失败，ThreadAccept线程函数
	static void AcceptErrorCB(struct evconnlistener* listener, void*);
	// 新的连接到来，ThreadRSSocket线程函数
	static void ThreadLibeventProcess(int readfd, short which, void* arg);


	// websocket解析数据包函数
private:
	static int FetchFin(char* msg, int& pos, WebSocketMsg& wbmsg);
	static int FetchOpcode(char* msg, int& pos, WebSocketMsg& wbmsg);
	static int FetchMask(char* msg, int& pos, WebSocketMsg& wbmsg);
	static int FetchMaskingKey(char* msg, int& pos, WebSocketMsg& wbmsg);
	static int FetchPayloadLength(char* msg, int& pos, WebSocketMsg& wbmsg);
	static int FetchPayload(char* msg, int& pos, WebSocketMsg& wbmsg);
	static void FetchPrint(const WebSocketMsg& wbmsg);


protected:
	event_base* m_listenerBase;
	std::vector<WorkThreadInfo> m_workBaseVec;
	IServerSocketService* m_pService;
	CDataLine* m_pSendDataLine;
	volatile bool				m_running;
	UINT						m_uMaxSocketSize; // libevent 单线程默认的32000
	volatile UINT				m_uCurSocketSize;
	char						m_bindIP[48];
	unsigned short				m_port;
	BYTE						m_socketType;

	// 内核管理socket模块
	CSignedLock					m_csSocketInfoLock;
	std::vector<TCPSocketInfo>	m_socketInfoVec;
	volatile UINT				m_uCurSocketIndex;
	std::set<UINT>				m_heartBeatSocketSet;

public:
	unsigned int	m_iServiceType;
};
