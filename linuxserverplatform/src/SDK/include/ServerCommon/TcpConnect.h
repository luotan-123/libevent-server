#pragma once

#define CLIENT_SOCKET_SEND_BUF	(64 * 1024)		// 发送缓冲区大小
#define CLIENT_SOCKET_RECV_BUF  (32 * 1024)		// 接收缓冲区大小

class CDataLine;
class CTcpConnect;
class CSignedLock;
class CTcpClient
{
public:
	CTcpClient();
	~CTcpClient();

public:
	bool SetDataLine(CDataLine* pDataLine);
	bool SetTcpConnect(CTcpConnect* pCTcpConnect);
	void SetIndex(int index) { m_index = index; }
	int GetSocketIdx();

	// 是否有效
	inline bool Enable();
	bool OnRead();

	bool Send(const void* pData, int size);

	bool OnClose();

	void SetSocket(int sock) { m_socket = sock; }
	void SetConnection(bool isConneted) { m_isConnected = true; }

	bool IsConnected() { return m_isConnected; }

	int GetSocket() { return m_socket; }
	int GetRemainSendBufSize() { return m_remainSendBytes; }

	// 清空非永久属性(初始化或者重新启用一个对象调用)
	void Clear();
private:
	int m_index;
	int m_socket;
	bool m_isConnected;

	char m_recvBuf[CLIENT_SOCKET_RECV_BUF];
	int  m_remainRecvBytes;

	char m_sendBuf[CLIENT_SOCKET_SEND_BUF];
	int m_remainSendBytes;

	CDataLine* m_pDataLine;			//共享的dataline对象
	CSignedLock m_lock;
	CTcpConnect* m_pTcpConnect;		//共享管理类
};

class CTcpConnect
{
public:
	CTcpConnect();
	~CTcpConnect();

	bool Start(CDataLine* pDataLine, const char* ip, int port, int serverType, int serverID);
	bool Stop();

	bool Connect();
	bool Send(UINT msgID, const void* pData, UINT size, int userID = 0, NetMessageHead * pNetHead = NULL);

	bool EventLoop();
	bool CheckConnection(); //return ，连接状态：true，非连接状态：false

	bool ReStart();
private:
	char m_ip[24];
	int m_port;
	CTcpClient m_tcpClient;
	CDataLine* m_pDataLine;
	int m_connectServerID;
	int m_connectServerType;
	volatile bool m_bStart;
};