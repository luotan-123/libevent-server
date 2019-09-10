#include "pch.h"
#include "TCPSocket.h"
#include "Exception.h"
#include "Function.h"
#include "DataBase.h"
#include "log.h"
#include "configManage.h"
#include "InternalMessageDefine.h"

/**************************************************************************************************************/
#define ID_SOCKET_WND			10								// SOCKET 窗口 ID
#define WM_SOCKET_MESSAGE		WM_USER + 12					// SOCKET 消息

//通用变量的定义
const int ERROR_SERVICE_FULL = 15;			// 服务器人数已满

const unsigned int MSG_MAIN_TEST = 1;		// 测试消息

const unsigned int 	MSG_ASS_TEST = 1;		// 测试消息

const unsigned int MSG_MAIN_CONECT = 2;		// 连接测试

/**************************************************************************************************************/
CTCPSocket* CTCPSocketManage::TCPSocketNew()
{
	// 可能要加锁 todo
	for (size_t i = 0; i < m_socketVec.size(); i++)
	{
		CTCPSocket* pSocket = m_socketVec[i];
		if (pSocket && pSocket->IsConnect() == false)
		{
			return pSocket;
		}
	}

	// 动态分配一个
	if (m_socketVec.size() < m_uMaxSocket)
	{
		CTCPSocket* pTcpSocket = new CTCPSocket;
		if (!pTcpSocket)
		{
			ERROR_LOG("alloc a CTCPSocket object failed");
			return NULL;
		}

		pTcpSocket->Init((int)m_socketVec.size(), this);
		m_socketVec.push_back(pTcpSocket);

		return pTcpSocket;
	}

	return NULL;
}

unsigned __stdcall CTCPSocketManage::ThreadAccept(LPVOID pData)
{
	Sleep(1);

	INFO_LOG("ThreadAccept thread begin...");

	CTCPSocketManage* pThis = (CTCPSocketManage *)pData;
	if (!pThis)
	{
		return -1;
	}

	HANDLE hCompletionPort = pThis->m_hCompletionPortRS;
	if (!hCompletionPort)
	{
		return -3;
	}

	SOCKET hListenScoket = pThis->m_listenSocket.GetSocket();

	// 通知主线程读取线程参数完成
	SetEvent(pThis->m_hEventThread);

	while (pThis->m_running == true)
	{
		try
		{
			sockaddr_in addr;
			int addrLen = sizeof(sockaddr_in);
			SOCKET hSocket = WSAAccept(hListenScoket, (sockaddr *)&addr, &addrLen, NULL, NULL);
			if (hSocket == INVALID_SOCKET)
			{
				ERROR_LOG("WSAAccept return INVALID_SOCKET err=%d", WSAGetLastError());
				continue;
			}

			// 分配空间
			CTCPSocket* pNewTCPSocket = pThis->TCPSocketNew();
			if (!pNewTCPSocket)
			{
				ERROR_LOG("TCPSocketNew failed");

				// 分配失败
				NetMessageHead netHead;

				netHead.uMainID = 100;
				netHead.uAssistantID = 3;
				netHead.uHandleCode = ERROR_SERVICE_FULL;//服务器人数已满
				netHead.uMessageSize = sizeof(NetMessageHead);

				sendto(hSocket, (char *)&netHead, sizeof(NetMessageHead), 0, (sockaddr*)&addr, sizeof(sockaddr_in));

				continue;
			}

			unsigned int newHandleID = GetHandleID();
			unsigned long ip = addr.sin_addr.s_addr;
			if (pNewTCPSocket->AcceptNewConnection(hSocket, newHandleID, hCompletionPort, ip) == false)
			{
				ERROR_LOG("AcceptNewConnection failed hSocket=%d newHandleID=%d hCompletionPort=%p", hSocket, newHandleID, hCompletionPort);
				continue;
			}

			//发送连接成功消息
			if (pThis->m_iServiceType == SERVICE_TYPE_CENTER)
			{
				pThis->CenterServerSendData(pNewTCPSocket->GetIndex(), 0, NULL, 0, MDM_CONNECT, ASS_CONNECT_SUCCESS, 0, 0);
			}
			else
			{
				pThis->SendData(pNewTCPSocket->GetIndex(), NULL, 0, MDM_CONNECT, ASS_CONNECT_SUCCESS, 0, 0);
			}

			INFO_LOG("accept an new connect idx=%d", pNewTCPSocket->GetIndex());
		}
		catch (...)
		{
			ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
		}
	}

	INFO_LOG("ThreadAccept thread exit.");

	return 0;
}

/**************************************************************************************************************/
CListenSocket::CListenSocket()
{
	m_hSocket = INVALID_SOCKET;
}

CListenSocket::~CListenSocket()
{
	CloseSocket();
}

bool CListenSocket::Init(int port, const char *ip/* = NULL*/)
{
	if (port <= 1000)		//1000 todo
	{
		// log todo
		return false;
	}

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	m_hSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	int ret = 0;
	int optval = 0;
	int optLen = sizeof(int);

	optval = 1;
	ret = setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	optval = 1;
	if (setsockopt(m_hSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval)))
	{
		return false;
	}

	optval = SOCKET_RECV_BUF_SIZE;
	ret = setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	optval = SOCKET_SEND_BUF_SIZE;
	ret = setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	sockaddr_in svrAddr;

	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = ip == NULL ? INADDR_ANY : inet_addr(ip);
	svrAddr.sin_port = htons(port);

	ret = bind(m_hSocket, (SOCKADDR*)&svrAddr, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	ret = listen(m_hSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		return false;
	}

	return true;
}

bool CListenSocket::CloseSocket()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	closesocket(m_hSocket);
	m_hSocket = INVALID_SOCKET;

	return true;
}


/**************************************************************************************************************/
CTCPSocket::CTCPSocket()
{
	m_index = -1;
	m_pManage = NULL;

	Clear();
}

CTCPSocket::~CTCPSocket()
{
	CloseSocket();
}

bool CTCPSocket::Init(int index, CTCPSocketManage * pManage)
{
	if (index < 0 || !pManage)
	{
		return false;
	}

	m_index = index;
	m_pManage = pManage;

	Clear();

	return true;
}

void CTCPSocket::Clear()
{
	m_socket = INVALID_SOCKET;
	m_handleID = 0;

	memset(m_recvBuf, 0, sizeof(m_recvBuf));
	m_recvBufLen = 0;

	memset(m_sendBuf, 0, sizeof(m_sendBuf));
	m_sendBufLen = 0;
	m_bSending = false;

	m_lastRecvMsgTime = 0;
	m_acceptMsgTime = 0;

	memset(m_ip, 0, sizeof(m_ip));
}

bool CTCPSocket::AcceptNewConnection(SOCKET sock, unsigned int handleID, HANDLE hCompletePort, unsigned long ip)
{
	if (sock == INVALID_SOCKET || handleID == 0 || hCompletePort == NULL)
	{
		ERROR_LOG("invalid input params sock=%d hanleID=%d hCompletePort=%p", sock, handleID, hCompletePort);
		return false;
	}

	// lock
	CSignedLockObject lock(&m_lock, true);

	m_socket = sock;
	m_handleID = handleID;
	m_lastRecvMsgTime = time(NULL);
	m_acceptMsgTime = m_lastRecvMsgTime;

	HANDLE hExistingCpmpletionPort = CreateIoCompletionPort((HANDLE)m_socket, hCompletePort, (ULONG_PTR)this, 0);
	if (hExistingCpmpletionPort == NULL)
	{
		ERROR_LOG("CreateIoCompletionPort return NULL err=%d", WSAGetLastError());
		return false;
	}

	// 转化ip
	sprintf(m_ip, "%d.%d.%d.%d", FOURTH_IPADDRESS(ip), THIRD_IPADDRESS(ip), SECOND_IPADDRESS(ip), FIRST_IPADDRESS(ip));

	return PostRecv();
}

bool CTCPSocket::PostRecv()
{
	// 加锁	todo
	CSignedLockObject RecvLock(&m_lock, true);

	DWORD transferBytes = 0;
	DWORD flags = 0;

	m_recvOverLapped.Clear();
	m_recvOverLapped.handleID = m_handleID;
	m_recvOverLapped.WSABuffer.buf = m_recvBuf + m_recvBufLen;
	m_recvOverLapped.WSABuffer.len = sizeof(m_recvBuf) - m_recvBufLen;
	m_recvOverLapped.type = SOCKET_EVENT_TYPE_RECV;

	int ret = WSARecv(m_socket, &m_recvOverLapped.WSABuffer, 1, &transferBytes, &flags, (LPWSAOVERLAPPED)&m_recvOverLapped, NULL);
	int err = WSAGetLastError();

	if (ret == 0)
	{
		return true;
	}
	else if (ret == SOCKET_ERROR)
	{
		if (err == WSA_IO_PENDING)
		{
			return true;
		}
	}

	//ERROR_LOG("PostRecv failed ret=%d err=%d", ret, err);

	return false;
}

// 投递发送请求
bool CTCPSocket::PostSend()
{
	CSignedLockObject lock(&m_lock, true);

	// 发送中
	if (m_bSending == true)
	{
		return true;
	}

	if (m_sendBufLen <= 0)
	{
		return false;
	}

	m_sendOverLapped.Clear();
	m_sendOverLapped.handleID = m_handleID;
	m_sendOverLapped.WSABuffer.buf = m_sendBuf;
	m_sendOverLapped.WSABuffer.len = m_sendBufLen;
	m_sendOverLapped.type = SOCKET_EVENT_TYPE_SEND;

	m_bSending = true;		// 立即返回了

	DWORD bytes = 0;
	int ret = WSASend(m_socket, &m_sendOverLapped.WSABuffer, 1, &bytes, 0, (LPWSAOVERLAPPED)&m_sendOverLapped, NULL);
	int err = WSAGetLastError();

	if (ret == 0)
	{
		return true;
	}
	else if (ret == SOCKET_ERROR)
	{
		if (err == WSA_IO_PENDING)
		{
			m_bSending = false;
			return true;
		}
	}

	//ERROR_LOG("PostSend failed ret=%d err=%d", ret, err);

	return false;
}

bool CTCPSocket::SendData(const char * pData, int size)
{
	if (!pData || size <= 0)
	{
		ERROR_LOG("invalid pData or size");
		return false;
	}

	CSignedLockObject lock(&m_lock, true);

	if (!IsConnect())
	{
		ERROR_LOG("send data error,socket closed");

		// 关闭socket
		m_pManage->OnSocketClose(m_index);

		return false;
	}

	// 检查缓冲区溢出的情况
	int leftSendBufLen = m_sendBufLen + size;
	if (leftSendBufLen >= SOCKET_SEND_BUF_SIZE)
	{
		ERROR_LOG("======= leftSendBufLen too long .m_sendBufLen=%d needSendDataSize=%d destIP=%s,socket=%d m_lastRecvMsgTime=%lld m_acceptMsgTime=%lld m_index=%d =======",
			m_sendBufLen, size, m_ip, m_socket, m_lastRecvMsgTime, m_acceptMsgTime, m_index);

		// 关闭socket
		//m_pManage->OnSocketClose(m_index);
		return false;
	}

	memcpy(m_sendBuf + m_sendBufLen, pData, size);
	m_sendBufLen += size;

	PostSend();

	return true;
}

bool CTCPSocket::IsConnect()
{
	if (m_socket == INVALID_SOCKET)
	{
		return false;
	}

	return true;
}

bool CTCPSocket::CloseSocket()
{
	CSignedLockObject lock(&m_lock, true);

	/*if (m_socket == INVALID_SOCKET)
	{
		return false;
	}*/

	// 关闭连接
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;

	// 清理数据
	Clear();

	return true;
}

bool CTCPSocket::DispatchPacket(NetMessageHead* pHead, void* pData, int size)
{
	if (!pHead)
	{
		// log todo
		return false;
	}

	if (!m_pManage)
	{
		// log todo
		return false;
	}

	CDataLine* pDataLine = m_pManage->GetDataLine();
	if (!pDataLine)
	{
		// log todo
		return false;
	}

	m_lastRecvMsgTime = time(NULL);

	if (pHead->uMainID == MSG_MAIN_TEST) //心跳包
	{
		return true;
	}

	if (pHead->uMainID == MSG_MAIN_CONECT) //测试连接包
	{
		// 回复测试连接包
		char buf[48] = "";
		int pos = 0;

		// 拼接包头
		NetMessageHead* pTestHead = (NetMessageHead*)buf;
		pTestHead->uMainID = pHead->uMainID;
		pTestHead->uAssistantID = pHead->uAssistantID;
		pTestHead->uMessageSize = pHead->uMessageSize;
		pTestHead->uHandleCode = pHead->uHandleCode;

		pos += sizeof(NetMessageHead);

		// 交给具体的socket
		SendData(buf, pos);

		return true;
	}

	SocketReadLine msg;

	msg.uHandleSize = size;
	msg.uIndex = m_index;
	msg.dwHandleID = m_handleID;
	msg.uAccessIP = 0;
	msg.NetMessageHead = *pHead;

	unsigned int addBytes = pDataLine->AddData(&msg.LineHead, sizeof(SocketReadLine), HD_SOCKET_READ, pData, size);

	if (addBytes == 0)
	{
		return false;
	}

	return true;
}

//发送完成函数
bool CTCPSocket::OnSendCompleted(unsigned int handleID, int sendedBytes)
{
	CSignedLockObject lock(&m_lock, true);

	if (handleID == 0 || m_handleID != handleID)
	{
		return false;
	}

	if (!m_pManage)
	{
		return false;
	}

	m_bSending = false;

	if (sendedBytes <= 0)
	{
		return m_pManage->OnSocketClose(m_index);
	}

	if (sendedBytes > m_sendBufLen)
	{
		// 发送完成的数据比总数据还大，不太现实
		return false;
	}

	m_sendBufLen -= sendedBytes;
	memmove(m_sendBuf, m_sendBuf + sendedBytes, m_sendBufLen);

	// 继续投递
	return PostSend();
}

//接收完成函数
bool CTCPSocket::OnRecvCompleted(unsigned int handleID, int recvedBytes)
{
	//lock
	CSignedLockObject lock(&m_lock, true);

	if (!m_pManage)
	{
		return false;
	}

	if (handleID == 0 || handleID != m_handleID)
	{
		return false;
	}

	if (recvedBytes <= 0)
	{
		m_pManage->OnSocketClose(m_index);
		return false;
	}

	int newRecvBufLen = m_recvBufLen + recvedBytes;
	if (newRecvBufLen > SOCKET_RECV_BUF_SIZE)
	{
		ERROR_LOG("======= newRecvBufLen=%d too long socketIP=%s =======", newRecvBufLen, m_ip);
		//m_pManage->OnSocketClose(m_index);
		return false;
	}

	m_recvBufLen = newRecvBufLen;

	// 解出包头
	NetMessageHead * pNetHead = (NetMessageHead *)m_recvBuf;
	if (!pNetHead)
	{
		// 这里基本不可能
		return false;
	}

	while (m_recvBufLen >= sizeof(NetMessageHead) && m_recvBufLen >= (int)pNetHead->uMessageSize)
	{
		unsigned int messageSize = pNetHead->uMessageSize;
		if (messageSize > MAX_TEMP_SENDBUF_SIZE)
		{
			// 走到这里一般都是消息发错了
			m_pManage->OnSocketClose(m_index);

			ERROR_LOG("send data too long. messageSize=%d,recvedBytes=%d", messageSize, recvedBytes);
			return false;
		}

		int realSize = messageSize - sizeof(NetMessageHead);
		if (realSize < 0)
		{
			// 数据包的大小居然不够一个包头
			m_pManage->OnSocketClose(m_index);

			ERROR_LOG("DataHead Error. messageSize=%d", messageSize);
			return false;
		}

		void* pData = NULL;
		if (realSize > 0)
		{
			// 没数据就为NULL
			pData = (void *)(m_recvBuf + sizeof(NetMessageHead));
		}

		DispatchPacket(pNetHead, pData, realSize);

		// 移动缓冲区，包头指针依然指向缓冲区的起始位置
		MoveMemory(m_recvBuf, m_recvBuf + messageSize, m_recvBufLen - messageSize);
		m_recvBufLen -= messageSize;
	}

	// 继续投递请求
	return PostRecv();
}

//////////////////////////////////////////////////////////////////////
CTCPSocketManage::CTCPSocketManage()
{
	m_pService = NULL;
	m_socketVec.clear();
	m_uRSThreadCount = 0;
	m_hEventThread = NULL;
	m_hCompletionPortRS = NULL;
	m_hThreadAccept = NULL;
	m_iServiceType = 0;
	m_uMaxSocket = 0;
	m_running = false;
}

CTCPSocketManage::~CTCPSocketManage()
{
	for (size_t i = 0; i < m_socketVec.size(); i++)
	{
		SAFE_DELETE(m_socketVec[i]);
	}

	m_socketVec.clear();
}

bool CTCPSocketManage::Init(IServerSocketService * pService, int maxCount, int port, int secretKey, const char *ip/* = NULL*/)
{
	INFO_LOG("service TCPSocketManage init begin...");

	if (!pService || maxCount <= 0 || port <= 1000)
	{
		ERROR_LOG("invalid params input pService=%p maxCount=%d port=%d", pService, maxCount, port);
		return false;
	}

	m_pService = pService;

	if (!m_listenSocket.Init(port, ip))
	{
		ERROR_LOG("listen socket init failed");
		return false;
	}

	m_socketVec.clear();
	m_uMaxSocket = maxCount;

	INFO_LOG("service TCPSocketManage init end.");

	return true;
}

bool CTCPSocketManage::UnInit()
{
	// TODO

	return true;
}

bool CTCPSocketManage::Start(int serverType)
{
	INFO_LOG("service TCPSocketManage start begin...");

	if (m_running == true)
	{
		ERROR_LOG("service TCPSocketManage already have been running");
		return false;
	}

	m_running = true;
	m_iServiceType = serverType;

	//建立事件
	m_hEventThread = ::CreateEvent(NULL, TRUE, false, NULL);

	const CommonConfig& commonConfig = ConfigManage()->GetCommonConfig();
	int IOCPWorkThreadNumber = commonConfig.IOCPWorkThreadNumber;
	if (IOCPWorkThreadNumber <= 1)
	{
		//获取系统信息
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		IOCPWorkThreadNumber = sysInfo.dwNumberOfProcessors * 2;
	}

	//建立完成端口
	m_hCompletionPortRS = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, IOCPWorkThreadNumber);
	if (!m_hCompletionPortRS)
	{
		ERROR_LOG("CreateIoCompletionPort failed err=%d", GetLastError());
		return false;
	}

	unsigned int acceptThreadID = 0;
	m_hThreadAccept = (HANDLE)_beginthreadex(NULL, 0, ThreadAccept, this, 0, &acceptThreadID);
	if (!m_hThreadAccept)
	{
		ERROR_LOG("begin ThreadAccept failed");
		return false;
	}

	GameLogManage()->AddLogFile(acceptThreadID, THREAD_TYPE_ACCEPT);

	::WaitForSingleObject(m_hEventThread, INFINITE);
	::ResetEvent(m_hEventThread);

	for (int i = 0; i < IOCPWorkThreadNumber; i++)
	{
		unsigned int threadID = 0;
		HANDLE hThreadHandle = (HANDLE)_beginthreadex(NULL, 0, ThreadRSSocket, this, 0, &threadID);
		if (!hThreadHandle)
		{
			ERROR_LOG("begin ThreadRSSocket failed");
			return false;
		}

		m_uRSThreadCount++;
		WaitForSingleObject(m_hEventThread, INFINITE);
		ResetEvent(m_hEventThread);
		CloseHandle(hThreadHandle);
	}

	INFO_LOG("service TCPSocketManage start end");

	return true;
}

//停止服务
bool CTCPSocketManage::Stop()
{
	INFO_LOG("service TCPSocketManage stop begin...");

	if (!m_running)
	{
		ERROR_LOG("TCPSocketManage is not running......");
		return false;
	}

	m_running = false;

	m_listenSocket.CloseSocket();

	m_uRSThreadCount = 0;

	//关闭完成端口
	if (m_hCompletionPortRS)
	{
		//PostQueuedCompletionStatus(m_hCompletionPortRS, 0, NULL, NULL);
		CloseHandle(m_hCompletionPortRS);
		m_hCompletionPortRS = NULL;
	}

	//关闭 SOCKET
	for (size_t i = 0; i < m_socketVec.size(); i++)
	{
		CTCPSocket* pSocket = m_socketVec[i];
		if (pSocket)
		{
			pSocket->CloseSocket();
			SAFE_DELETE(m_socketVec[i]);
		}
	}

	//关闭事件
	if (m_hEventThread)
	{
		CloseHandle(m_hEventThread);
		m_hEventThread = NULL;
	}

	// 关闭接收线程句柄
	if (m_hThreadAccept)
	{
		WaitForSingleObject(m_hThreadAccept, INFINITE);
		CloseHandle(m_hThreadAccept);
		m_hThreadAccept = NULL;
	}

	// 关闭收发线程句柄 TODO

	INFO_LOG("service TCPSocketManage stop end...");

	return true;
}

bool CTCPSocketManage::SendData(int idx, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int handleID, unsigned int uIdentification/* = 0*/)
{
	if (idx < 0 || idx >= (int)m_socketVec.size())
	{
		ERROR_LOG("invalid socketIdx, idx=%d, mainID=%d assistID=%d", idx, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(NetMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	CTCPSocket* pTcpSocket = m_socketVec[idx];
	if (!pTcpSocket)
	{
		ERROR_LOG("get pTcpSocket failed, idx=%d", idx);
		return false;
	}

	// 整合一下数据
	char buf[MAX_TEMP_SENDBUF_SIZE] = "";

	int pos = 0;

	// 拼接包头
	NetMessageHead* pHead = (NetMessageHead*)buf;
	pHead->uMainID = mainID;
	pHead->uAssistantID = assistID;
	pHead->uMessageSize = sizeof(NetMessageHead) + size;
	pHead->uHandleCode = handleCode;
	pHead->uIdentification = uIdentification;

	pos += sizeof(NetMessageHead);

	// 包体
	if (pData && size > 0)
	{
		memcpy(buf + pos, pData, size);
		pos += size;
	}

	// 交给具体的socket
	pTcpSocket->SendData(buf, pos);

	return true;
}

//中心服务器发送数据
bool CTCPSocketManage::CenterServerSendData(int idx, UINT msgID, void* pData, int size, int mainID, int assistID, int handleCode, int userID)
{
	if (idx < 0 || idx >= (int)m_socketVec.size())
	{
		ERROR_LOG("invalid socketIdx, idx=%d, mainID=%d assistID=%d", idx, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(PlatformMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	CTCPSocket* pTcpSocket = m_socketVec[idx];
	if (!pTcpSocket)
	{
		ERROR_LOG("get pTcpSocket failed, idx=%d", idx);
		return false;
	}

	// 整合一下数据
	char buf[MAX_TEMP_SENDBUF_SIZE] = "";

	int pos = 0;

	// 拼接包头
	PlatformMessageHead* pHead = (PlatformMessageHead*)buf;
	pHead->MainHead.uMainID = mainID;
	pHead->MainHead.uAssistantID = assistID;
	pHead->MainHead.uMessageSize = sizeof(PlatformMessageHead) + size;
	pHead->MainHead.uHandleCode = handleCode;
	pHead->AssHead.msgID = msgID;
	pHead->AssHead.userID = userID;

	pos += sizeof(PlatformMessageHead);

	// 包体
	if (pData && size > 0)
	{
		memcpy(buf + pos, pData, size);
		pos += size;
	}

	// 交给具体的socket
	pTcpSocket->SendData(buf, pos);

	return true;
}

//批量发送函数
int CTCPSocketManage::SendDataBatch(void * pData, UINT uSize, UINT uMainID, UINT uAssistantID, UINT uHandleCode)
{
	for (size_t i = 0; i < m_socketVec.size(); i++)
	{
		CTCPSocket* pTcpSocket = m_socketVec[i];
		if (pTcpSocket && pTcpSocket->IsConnect())
		{
			SendData(i, pData, uSize, uMainID, uAssistantID, uHandleCode, 0);
		}
	}

	return uSize;
}

bool CTCPSocketManage::CloseSocket(int index)
{
	if (index < 0 || index >(int)m_socketVec.size())
	{
		// log todo
		return false;
	}

	CTCPSocket* pTcpSocket = m_socketVec[index];
	if (!pTcpSocket)
	{
		// log todo
		return false;
	}

	if (!m_pService)
	{
		// log todo
		return false;
	}

	if (!pTcpSocket->CloseSocket())
	{
		// log todo
		return false;
	}

	return true;
}

bool CTCPSocketManage::OnSocketClose(int index)
{
	if (!m_pService)
	{
		return false;
	}

	// 回调业务层
	return m_pService->OnSocketCloseEvent(0, index, 0);
}

void CTCPSocketManage::CheckHeartBeat(time_t llLastSendHeartBeatTime, int iHeartBeatTime)
{
	int avtiveCount = 0;
	int negativeCount = 0;
	time_t currTime = time(NULL);

	for (size_t i = 0; i < m_socketVec.size(); ++i)
	{
		CTCPSocket* pTcpSocket = m_socketVec[i];
		if (pTcpSocket && pTcpSocket->IsConnect())
		{
			time_t lastRecvMsgTime = pTcpSocket->GetLastRecvMsgTime();
			if ((currTime - lastRecvMsgTime) >= (iHeartBeatTime * (KEEP_ACTIVE_HEARTBEAT_COUNT - 1) + currTime - llLastSendHeartBeatTime))
			{
				OnSocketClose(i);
				negativeCount++;
			}
			else
			{
				// 发送心跳
				SendData(i, NULL, 0, MSG_MAIN_TEST, MSG_ASS_TEST, 0, 0);
				avtiveCount++;
			}
		}
	}

	if (negativeCount > 0)
	{
		INFO_LOG("avtiveCount=%d negativeCount=%d", avtiveCount, negativeCount);
	}
}

CDataLine* CTCPSocketManage::GetDataLine()
{
	if (m_pService)
	{
		return m_pService->GetDataLine();
	}

	return NULL;
}

const char* CTCPSocketManage::GetSocketIP(int socketIdx)
{
	if (socketIdx < 0 || socketIdx >= (int)m_socketVec.size())
	{
		return NULL;
	}

	CTCPSocket* pTcpSocket = m_socketVec[socketIdx];
	if (!pTcpSocket)
	{
		return NULL;
	}

	return pTcpSocket->GetIP();
}

void CTCPSocketManage::GetTCPSocketInfo(UINT uIndex, bool &bIsConnect, time_t &llAcceptMsgTime, time_t &llLastRecvMsgTime)
{
	if (uIndex >= m_socketVec.size())
	{
		bIsConnect = false;
		return;
	}

	CTCPSocket* pSocket = m_socketVec[uIndex];
	if (pSocket)
	{
		bIsConnect = pSocket->IsConnect();
		llAcceptMsgTime = pSocket->GetAcceptMsgTime();
		llLastRecvMsgTime = pSocket->GetLastRecvMsgTime();
	}
}

unsigned __stdcall CTCPSocketManage::ThreadRSSocket(LPVOID pData)
{
	CTCPSocketManage* pThis = (CTCPSocketManage*)pData;
	if (!pThis)
	{
		return -1;
	}

	HANDLE hCompletionPort = pThis->m_hCompletionPortRS;
	if (!hCompletionPort)
	{
		return -2;
	}

	// 通知主线程读取线程参数完成
	SetEvent(pThis->m_hEventThread);

	while (pThis->m_running == true)
	{
		try
		{
			DWORD bytes = 0;
			CTCPSocket* pTcpSocket = NULL;
			OverLappedStruct* pOverData = NULL;

			BOOL ret = GetQueuedCompletionStatus(hCompletionPort, &bytes, (PULONG_PTR)&pTcpSocket, (LPOVERLAPPED *)&pOverData, INFINITE);
			if (ret == FALSE || !pTcpSocket || !pOverData)
			{
				int err = WSAGetLastError();

				if (ret == FALSE && err == ERROR_NETNAME_DELETED)
				{
					// 奇葩的错误
					if (pTcpSocket)
					{
						pThis->OnSocketClose(pTcpSocket->GetIndex());
					}
				}

				continue;
			}

			switch (pOverData->type)
			{
			case SOCKET_EVENT_TYPE_RECV:
			{
				pTcpSocket->OnRecvCompleted(pOverData->handleID, bytes);
				break;
			}
			case SOCKET_EVENT_TYPE_SEND:
			{
				pTcpSocket->OnSendCompleted(pOverData->handleID, bytes);
				break;
			}
			}
		}
		catch (...)
		{
			ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
		}
	}

	return 0;
}

unsigned int CTCPSocketManage::GetHandleID()
{
	static unsigned int handleID = 1;
	if (handleID > 3000000)
	{
		handleID = 1;
	}

	return InterlockedIncrement(&handleID);
}