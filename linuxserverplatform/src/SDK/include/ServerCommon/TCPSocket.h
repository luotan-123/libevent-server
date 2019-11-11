#pragma once

#include "KernelDefine.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

// 常量定义
const int SOCKET_SEND_BUF_SIZE = MAX_TEMP_SENDBUF_SIZE * 8;		// TCP发送缓冲区最小值
const int SOCKET_RECV_BUF_SIZE = MAX_TEMP_SENDBUF_SIZE * 8;		// TCP接收缓冲区最小值
const int MAX_EVBUFFER_WRITE_SIZE = 1024 * 1024;				// 应用层发送缓冲区的最大值
const int MAX_POST_CONNECTED_COUNT = 128;						// 连接线程投递到接收线程socket最大数量

#pragma pack(1)

// TCP信息
struct TCPSocketInfo
{
	volatile bool isConnect;
	time_t acceptMsgTime;
	char ip[MAX_NUM_IP_ADDR_SIZE];
	unsigned short port;
	int acceptFd;			//自己的socket
	bufferevent* bev;
	CSignedLock* lock;

	TCPSocketInfo()
	{
		memset(this, 0, sizeof(TCPSocketInfo));
	}
};

// socket简单信息
struct SocketSimpleInfo
{
	unsigned int socketIdx;
	void* pBufferevent;

	SocketSimpleInfo()
	{
		socketIdx = -1;
		pBufferevent = nullptr;
	}

	SocketSimpleInfo(unsigned int socketIdx, void* pBufferevent)
	{
		this->socketIdx = socketIdx;
		this->pBufferevent = pBufferevent;
	}

	bool operator<(const SocketSimpleInfo& info)const
	{
		if (socketIdx < info.socketIdx)
		{
			return true;
		}
		else if (socketIdx == info.socketIdx)
		{
			if (pBufferevent < info.pBufferevent)
			{
				return true;
			}
		}
		return false;
	}
};

// 工作线程信息
struct WorkThreadInfo
{
	struct event_base* base;
	struct event* event;   //read_fd的读事件
	int read_fd;
	int write_fd;
	WorkThreadInfo()
	{
		memset(this, 0, sizeof(WorkThreadInfo));
	}
};

#pragma pack()

class CTCPSocketManage
{
public:
	CTCPSocketManage();
	~CTCPSocketManage();

public:
	// 初始化
	bool Init(IServerSocketService* pService, int maxCount, int port, const char* ip = NULL);
	// 取消初始化
	virtual bool UnInit();
	// 开始服务
	virtual bool Start(int serverType);
	// 停止服务
	virtual bool Stop();


public:
	// 发送数据函数
	bool SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, void* pBufferevent, unsigned int uIdentification = 0);
	// 中心服务器发送数据
	bool CenterServerSendData(int index, UINT msgID, void* pData, int size, int mainID, int assistID, int handleCode, int userID, void* pBufferevent);
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
	// 获取连接ip
	const char* GetSocketIP(int index);
	// 获取TcpSocketInfo
	const TCPSocketInfo* GetTCPSocketInfo(int index);
	// 分配socketIndex算法
	int GetSocketIndex();
	// 添加TCPSocketInfo
	void AddTCPSocketInfo(int threadIndex, PlatformSocketInfo* pTCPSocketInfo);
	// 设置tcp为未连接状态
	void RemoveTCPSocketStatus(int index, bool isClientAutoClose = false);
	// 派发数据包
	bool DispatchPacket(void* pBufferevent, int index, NetMessageHead* pHead, void* pData, int size);
	// 设置tcp收发缓冲区
	static void SetTcpRcvSndBUF(int fd, int rcvBufSize, int sndBufSize);
	// 设置应用层单次读取数据包的大小 bufferevent_set_max_single_read
	static void SetMaxSingleReadAndWrite(bufferevent* bev, int rcvBufSize, int sndBufSize);

	// 线程函数
private:
	// SOCKET 连接应答线程
	static void* ThreadAccept(void* pThreadData);
	// SOCKET 数据接收线程
	static void* ThreadRSSocket(void* pThreadData);
	// SOCKET 数据发送线程
	static void* ThreadSendMsg(void* pThreadData);

	// 回调函数
private:
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
	// libEvent日志回调函数
	static void EventLog(int severity, const char* msg);

private:
	event_base* m_listenerBase;
	std::vector<WorkThreadInfo> m_workBaseVec;
	IServerSocketService* m_pService;
	CDataLine* m_pSendDataLine;
	volatile bool				m_running;
	UINT						m_uMaxSocketSize; // libevent 单线程默认的32000
	volatile UINT				m_uCurSocketSize;
	char						m_bindIP[48];
	unsigned short				m_port;

	// 内核管理socket模块
	CSignedLock					m_csSocketInfoLock;
	std::vector<TCPSocketInfo>	m_socketInfoVec;
	volatile UINT				m_uCurSocketIndex;
	std::set<UINT>				m_heartBeatSocketSet;

public:
	unsigned int	m_iServiceType;
};
