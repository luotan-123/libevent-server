#include "CommonHead.h"
#include "basemessage.h"
#include "Lock.h"
#include "log.h"
#include "DataLine.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "TcpConnect.h"

CTcpClient::CTcpClient()
{
	m_pDataLine = NULL;			//共享的dataline对象
	m_pTcpConnect = NULL;
	Clear();
}

CTcpClient::~CTcpClient()
{

}

bool CTcpClient::SetDataLine(CDataLine* pDataLine)
{
	if (!pDataLine)
	{
		return false;
	}

	m_pDataLine = pDataLine;

	return true;
}

bool CTcpClient::SetTcpConnect(CTcpConnect* pCTcpConnect)
{
	if (!pCTcpConnect)
	{
		return false;
	}

	m_pTcpConnect = pCTcpConnect;

	return true;
}

int CTcpClient::GetSocketIdx()
{
	return m_index;
}

inline bool CTcpClient::Enable()
{
	if (m_socket != INVALID_SOCKET && m_isConnected == true)
	{
		return true;
	}

	return false;
}

bool CTcpClient::OnRead()
{
	if (m_isConnected == false || m_socket == INVALID_SOCKET)
	{
		return false;
	}

	if (!m_pDataLine)
	{
		return false;
	}

	// 接受数据
	int recvBytes = recv(m_socket, m_recvBuf + m_remainRecvBytes, sizeof(m_recvBuf) - m_remainRecvBytes, 0);
	if (recvBytes <= 0)
	{
		return OnClose();
	}

	m_remainRecvBytes += recvBytes;

	PlatformMessageHead* pHead = (PlatformMessageHead*)m_recvBuf;
	if (!pHead)
	{
		// 这里不太可能
		return false;
	}

	// 处理数据
	while (m_remainRecvBytes >= sizeof(PlatformMessageHead) && m_remainRecvBytes >= (int)pHead->MainHead.uMessageSize)
	{
		UINT messageSize = pHead->MainHead.uMessageSize;
		int realSize = messageSize - sizeof(PlatformMessageHead);
		void* pData = NULL;
		if (realSize < 0)
		{
			ERROR_LOG("包头字节大小错误。已丢弃：%d字节，收到数据：%d字节", m_remainRecvBytes, recvBytes);

			// 丢包
			m_remainRecvBytes = 0;
			return false;
		}

		if (realSize > 0)
		{
			// 没数据就为NULL
			pData = (void*)(m_recvBuf + sizeof(PlatformMessageHead));
		}

		if (pHead->MainHead.uMainID != MDM_CONNECT)	// 过滤掉连接测试消息
		{
			PlatformDataLineHead dataLineHead;
			dataLineHead.platformMessageHead = *pHead;
			dataLineHead.socketIdx = m_index;
			m_pDataLine->AddData(&dataLineHead.lineHead, sizeof(PlatformDataLineHead), HD_PLATFORM_SOCKET_READ, pData, realSize);
		}

		// 有内存重叠的时候，不能用memcpy 只能用memmove
		memmove(m_recvBuf, m_recvBuf + messageSize, m_remainRecvBytes - messageSize);
		m_remainRecvBytes -= messageSize;
	}

	return true;
}

bool CTcpClient::Send(const void* pData, int size)
{
	CSignedLockObject lock(&m_lock, true);

	if (!pData || size <= 0)
	{
		return false;
	}

	// 检查是否有滞留数据
	int bytes = 0;
	while (m_remainSendBytes > 0)
	{
		bytes = send(m_socket, m_sendBuf, m_remainSendBytes, 0);
		if (bytes == 0)
		{
			return false;
		}

		if (bytes < 0)
		{
			break;
		}

		memmove(m_sendBuf, m_sendBuf + bytes, m_remainSendBytes - bytes);
		m_remainSendBytes -= bytes;
	}

	if (m_remainSendBytes + size >= sizeof(m_sendBuf))
	{
		return false;
	}

	memcpy(m_sendBuf + m_remainSendBytes, pData, size);
	m_remainSendBytes += size;

	// 就只发送一次吧
	bytes = send(m_socket, m_sendBuf, m_remainSendBytes, 0);
	if (bytes > 0)
	{
		memmove(m_sendBuf, m_sendBuf + bytes, m_remainSendBytes - bytes);
		m_remainSendBytes -= bytes;
	}

	return true;
}

bool CTcpClient::OnClose()
{
	CSignedLockObject lock(&m_lock, true);

	close(m_socket);

	Clear();

	return true;
}

void CTcpClient::Clear()
{
	m_socket = INVALID_SOCKET;
	m_isConnected = false;

	memset(m_recvBuf, 0, sizeof(m_recvBuf));
	m_remainRecvBytes = 0;

	memset(m_sendBuf, 0, sizeof(m_sendBuf));
	m_remainSendBytes = 0;
}


//////////////////////////////////////////////////////////////////////////
//CTcpConnect
CTcpConnect::CTcpConnect()
{
	m_ip[0] = '\0';
	m_port = 0;
	m_pDataLine = NULL;
	m_connectServerID = 0;
	m_connectServerType = 0;
	m_bStart = false;
}

CTcpConnect::~CTcpConnect()
{

}

bool CTcpConnect::Start(CDataLine* pDataLine, const char* ip, int port, int serverType, int serverID)
{
	if (!pDataLine || !ip || port <= 0)
	{
		return false;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		return false;
	}

	strcpy(m_ip, ip);
	m_port = port;

	m_tcpClient.SetSocket(sock);
	m_tcpClient.SetDataLine(pDataLine);
	m_tcpClient.SetTcpConnect(this);
	m_pDataLine = pDataLine;
	m_connectServerID = serverID;
	m_connectServerType = serverType;
	m_bStart = true;

	// 先不连接，稍后连接
	//Connect();

	return true;
}

bool CTcpConnect::Stop()
{
	m_bStart = false;
	m_tcpClient.OnClose();

	return true;
}

bool CTcpConnect::Connect()
{
	if (!m_bStart)
	{
		return false;
	}
	int sock = m_tcpClient.GetSocket();
	if (sock == INVALID_SOCKET)
	{
		return false;
	}

	sockaddr_in svrAddr;

	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = inet_addr(m_ip);
	svrAddr.sin_port = htons((uint16_t)m_port);

	int ret = connect(sock, (sockaddr*)& svrAddr, sizeof(sockaddr_in));
	if (ret == 0)
	{
		m_tcpClient.SetConnection(true);

		//发送认证消息
		PlatformCenterServerVerify msg;
		msg.serverType = m_connectServerType;
		msg.serverID = m_connectServerID;
		Send(COMMON_VERIFY_MESSAGE, &msg, sizeof(msg));

		return true;
	}

	ERROR_LOG("连接中心服务器失败，稍后会重连。ip=%s,port=%d", m_ip, m_port);

	return false;
}

bool CTcpConnect::Send(UINT msgID, const void* pData, UINT size, int userID/* = 0*/, NetMessageHead* pNetHead/* = NULL*/)
{
	if (size + sizeof(PlatformMessageHead) > MAX_TEMP_SENDBUF_SIZE)
	{
		return false;
	}

	if (m_tcpClient.Enable() == false)
	{
		return false;
	}

	char buf[MAX_TEMP_SENDBUF_SIZE] = "";

	// head
	PlatformMessageHead* pHead = (PlatformMessageHead*)buf;
	pHead->AssHead.msgID = msgID;
	pHead->AssHead.userID = userID;
	pHead->MainHead.uMessageSize = sizeof(PlatformMessageHead) + size;
	if (pNetHead)
	{
		pHead->MainHead.uMainID = pNetHead->uMainID;
		pHead->MainHead.uAssistantID = pNetHead->uAssistantID;
		pHead->MainHead.uHandleCode = pNetHead->uHandleCode;
	}

	// body
	if (pData && size > 0)
	{
		memcpy(buf + sizeof(PlatformMessageHead), pData, size);
	}

	return m_tcpClient.Send(buf, pHead->MainHead.uMessageSize);
}

bool CTcpConnect::EventLoop()
{
	if (m_tcpClient.Enable() == false)
	{
		return false;
	}

	int sock = m_tcpClient.GetSocket();

	fd_set fdRead;
	FD_ZERO(&fdRead);

	FD_SET(sock, &fdRead);

	int ret = select(sock + 1, &fdRead, NULL, NULL, NULL);
	if (ret == -1)
	{
		//输出错误消息
		SYS_ERROR_LOG("##### CGServerConnect::ThreadRSSocket select error,thread Exit.#####");
		return false;
	}

	if (ret == 0)
	{
		// 正常
		return true;
	}

	if (FD_ISSET(sock, &fdRead))
	{
		m_tcpClient.OnRead();
	}

	return true;
}

bool CTcpConnect::CheckConnection()
{
	if (m_tcpClient.Enable() == false)
	{
		ReStart();
		if (!Connect())
		{
			sleep(5);
			return false;
		}
	}

	return true;
}

bool CTcpConnect::ReStart()
{
	if (!m_bStart)
	{
		return false;
	}

	if (m_tcpClient.GetSocket() != INVALID_SOCKET)
	{
		return false;
	}

	if (!m_pDataLine || !m_ip || m_port <= 0)
	{
		return false;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		return false;
	}

	m_tcpClient.SetSocket(sock);
	m_tcpClient.SetDataLine(m_pDataLine);
	m_tcpClient.SetTcpConnect(this);

	return true;
}