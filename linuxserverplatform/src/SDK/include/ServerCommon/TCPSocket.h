#pragma once

// 别名定义
typedef int SOCKET;

//////////////////////////////////////////////////////////////////////
class CListenSocket
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


/**************************************************************************************************************/
// TCP SOCKET 网络部分 
/**************************************************************************************************************/
#define NO_CONNECT				0								//没有连接
#define CONNECTING				1								//正在连接
#define CONNECTED				2								//成功连接
/**************************************************************************************************************/
//CloseSocket和OnSocketClose区别：
//1、调用CloseSocket无法执行到CCenterServerManage::OnSocketClose逻辑。
//2、OnSocketClose可能会执行两次CCenterServerManage::OnSocketClose逻辑，但是会保证关闭socket之前，消息发送出去
class CTCPSocketManage
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


	// 线程函数
private:
	// SOCKET 连接应答线程
	static void* ThreadAccept(void* pThreadData);
	// SOCKET 数据接收线程
	static void* ThreadRSSocket(void* pThreadData);

private:
	CListenSocket				m_listenSocket;
	IServerSocketService*		m_pService;
	UINT	m_uMaxSocket;
	volatile bool	m_running;
public:
	unsigned int	m_iServiceType;
};
