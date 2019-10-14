#include "CommonHead.h"
#include "basemessage.h"
#include "Lock.h"
#include "log.h"
#include "configManage.h"
#include "DataLine.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "TCPSocket.h"

/**************************************************************************************************************/

const int SOCKET_SEND_BUF_SIZE = 128 * 1024;	// TCP发送缓冲区大小
const int SOCKET_RECV_BUF_SIZE = 64 * 1024;		// TCP接收缓冲区大小

//通用变量的定义
const int ERROR_SERVICE_FULL = 15;			// 服务器人数已满
const unsigned int MSG_MAIN_TEST = 1;		// 测试消息
const unsigned int 	MSG_ASS_TEST = 1;		// 测试消息
const unsigned int MSG_MAIN_CONECT = 2;		// 连接测试

/**************************************************************************************************************/
void * CTCPSocketManage::ThreadAccept(void* pData)
{
	INFO_LOG("ThreadAccept thread begin...");

	CTCPSocketManage* pThis = (CTCPSocketManage *)pData;
	if (!pThis)
	{
		return 0;
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

	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

	ret = bind(m_hSocket, (struct sockaddr*)&svrAddr, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		SYS_ERROR_LOG("bind fail. port=%d", port);
		return false;
	}

	ret = listen(m_hSocket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		SYS_ERROR_LOG("listen fail. port=%d", port);
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

	close(m_hSocket);
	m_hSocket = INVALID_SOCKET;

	return true;
}

//////////////////////////////////////////////////////////////////////
CTCPSocketManage::CTCPSocketManage()
{
	m_pService = NULL;
	m_running = false;
	m_uMaxSocket = 0;
	m_iServiceType = 0;
}

CTCPSocketManage::~CTCPSocketManage()
{
	
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

	




	INFO_LOG("service TCPSocketManage stop end...");

	return true;
}

bool CTCPSocketManage::SendData(int idx, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int handleID, unsigned int uIdentification/* = 0*/)
{
	

	return true;
}

//中心服务器发送数据
bool CTCPSocketManage::CenterServerSendData(int idx, UINT msgID, void* pData, int size, int mainID, int assistID, int handleCode, int userID)
{
	
	return true;
}

//批量发送函数
int CTCPSocketManage::SendDataBatch(void * pData, UINT uSize, UINT uMainID, UINT uAssistantID, UINT uHandleCode)
{
	

	return uSize;
}

bool CTCPSocketManage::CloseSocket(int index)
{
	

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


	return NULL;
}

void CTCPSocketManage::GetTCPSocketInfo(UINT uIndex, bool &bIsConnect, time_t &llAcceptMsgTime, time_t &llLastRecvMsgTime)
{
	
}

void* CTCPSocketManage::ThreadRSSocket(void* pData)
{
	CTCPSocketManage* pThis = (CTCPSocketManage*)pData;
	if (!pThis)
	{
		return 0;
	}

	

	return 0;
}