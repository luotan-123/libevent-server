#pragma once

#define GSERVER_SOCKET_SEND_BUF		(128 * 1024)		// 发送缓冲区大小
#define GSERVER_SOCKET_RECV_BUF		(128 * 1024)		// 接收缓冲区大小
#define MAX_RECONNECT_COUNT			5					// 最多重连次数，0：无限重连

class CDataLine;
class CGServerConnect;
class CSignedLock;

// 单个socket
class CGServerClient
{
public:
	CGServerClient();
	~CGServerClient();

public:
	bool Init(CDataLine* pDataLine, CGServerConnect* pCGServerConnect, int index, char ip[24], int port);
	bool Connect();

	// 读数据
	bool OnRead();

	// 发数据
	bool Send(const void* pData, int size);

	// socket关闭
	bool OnClose();

	// 获取socket属性
	bool IsConnected();
	int GetSocket() { return m_socket; }
	int GetSocketIdx() { return m_index; }
	int GetRemainSendBufSize() { return m_remainSendBytes; }

	// 清空非永久属性(初始化或者重新启用一个对象调用)
	void Clear();

	// 重连相关函数
	bool IsNeedReConnect();
	bool ReConnect();
private:
	int m_socket;
	volatile bool m_isConnected;
	volatile int m_ReConnectCount;

	char m_recvBuf[GSERVER_SOCKET_RECV_BUF];
	int  m_remainRecvBytes;

	char m_sendBuf[GSERVER_SOCKET_SEND_BUF];
	int m_remainSendBytes;

	CDataLine* m_pRecvDataLine;			//共享的dataline对象
	CSignedLock m_lock;
	CGServerConnect* m_pCGServerConnect;		//共享管理类
	int m_index;
	char m_ip[24];
	int m_port;
};

// socket管理
class CGServerConnect
{
public:
	CGServerConnect();
	~CGServerConnect();

public:
	bool Start(CDataLine* pDataLine, int roomID, bool bStartSendThread = false);
	bool Stop();

public:
	// 发送数据函数
	bool SendData(int idx, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int uIdentification);

	// 接口
public:
	int GetRoomID();
	const std::vector<CGServerClient*>& GetSocketVec();
	void GetIndexByThreadID(pthread_t uThreadID, size_t& uMin, size_t& uMax);

	// 线程函数
private:
	// SOCKET 检测连接线程
	static void* ThreadCheckConnect(void* pThreadData);
	// SOCKET 数据接收线程
	static void* ThreadRSSocket(void* pThreadData);
	// SOCKET 数据发送线程
	static void* ThreadSendMsg(void* pThreadData);

private:
	std::vector<CGServerClient*> m_socketVec;
	std::map<pthread_t, int> m_threadIDToIndexMap;
	CDataLine* m_pRecvDataLine;
	CDataLine* m_pSendDataLine;
	int m_roomID;
	volatile bool m_running;
	pthread_t	m_hThreadCheckConnect;
	pthread_t	m_hThreadSendMsg;
};