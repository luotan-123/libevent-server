#include "CommonHead.h"
#include "basemessage.h"
#include "log.h"
#include "configManage.h"
#include "DataLine.h"
#include "Xor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "TCPSocket.h"


//通用变量的定义
const static int ERROR_SERVICE_FULL = 15;			// 服务器人数已满
const static unsigned int MSG_MAIN_TEST = 1;		// 测试消息
const static unsigned int MSG_MAIN_CONECT = 2;		// 连接测试

//接收线程参数
struct RecvThreadParam
{
	CTCPSocketManage* pThis;
	int index;
	RecvThreadParam()
	{
		pThis = NULL;
		index = 0;
	}
};

//////////////////////////////////////////////////////////////////////
CTCPSocketManage::CTCPSocketManage()
{
	m_listenerBase = NULL;
	m_pService = NULL;
	m_pSendDataLine = NULL;
	m_running = false;
	m_uMaxSocketSize = 0;
	m_uCurSocketSize = 0;
	m_iServiceType = 0;
	memset(m_bindIP, 0, sizeof(m_bindIP));
	m_port = 0;
	m_iFreeHead = 0;
	m_iFreeTail = 0;
	m_socketType = SOCKET_TYPE_TCP;
}

CTCPSocketManage::~CTCPSocketManage()
{

}

bool CTCPSocketManage::Init(IServerSocketService* pService, int maxCount, int port, const char* ip/* = NULL*/,SocketType socketType)
{
	INFO_LOG("service TCPSocketManage init begin...");

	if (!pService || maxCount <= 0 || port <= 1000)
	{
		ERROR_LOG("invalid params input pService=%p maxCount=%d port=%d", pService, maxCount, port);
		return false;
	}

	m_pService = pService;
	m_uMaxSocketSize = maxCount;
	if (ip && strlen(ip) < sizeof(m_bindIP))
	{
		strcpy(m_bindIP, ip);
	}
	m_port = port;
	m_socketType = socketType;

	m_workBaseVec.clear();
	m_heartBeatSocketSet.clear();

	// 初始化分配内存
	m_socketInfoVec.resize(m_uMaxSocketSize + 1);
	m_socketIndexVec.resize(m_uMaxSocketSize + 1);

	// 初始化空闲队列
	m_iFreeHead = 0;
	m_iFreeTail = 0;
	for (int i = 0; i < (int)m_socketIndexVec.size(); i++)
	{
		m_socketIndexVec[i] = i;
	}

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
	m_uCurSocketSize = 0;

	// 创建发送队列
	if (m_pSendDataLine == NULL)
	{
		m_pSendDataLine = new CDataLine;
	}

	// 创建发送线程
	pthread_t threadID = 0;
	int err = pthread_create(&threadID, NULL, ThreadSendMsg, (void*)this);
	if (err != 0)
	{
		SYS_ERROR_LOG("ThreadSendMsg failed");
		return false;
	}
	GameLogManage()->AddLogFile(threadID, THREAD_TYPE_SEND);

	// 创建连接线程
	err = pthread_create(&threadID, NULL, ThreadAccept, (void*)this);
	if (err != 0)
	{
		SYS_ERROR_LOG("ThreadAccept failed");
		return false;
	}
	GameLogManage()->AddLogFile(threadID, THREAD_TYPE_ACCEPT);

	INFO_LOG("service TCPSocketManage start end");

	return true;
}

bool CTCPSocketManage::Stop()
{
	INFO_LOG("service TCPSocketManage stop begin...");

	if (!m_running)
	{
		ERROR_LOG("TCPSocketManage is not running......");
		return false;
	}

	m_running = false;
	m_uCurSocketSize = 0;
	m_iFreeHead = 0;
	m_iFreeTail = 0;
	m_socketIndexVec.clear();

	event_base_loopbreak(m_listenerBase);
	for (size_t i = 0; i < m_workBaseVec.size(); i++)
	{
		event_base_loopbreak(m_workBaseVec[i].base);
	}

	for (size_t i = 0; i < m_socketInfoVec.size(); i++)
	{
		if (m_socketInfoVec[i].lock)
		{
			SafeDelete(m_socketInfoVec[i].lock);
		}
	}

	INFO_LOG("service TCPSocketManage stop end...");

	return true;
}

bool CTCPSocketManage::SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, void* pBufferevent, unsigned int uIdentification/* = 0*/)
{
	if (!IsConnected(index))
	{
		ERROR_LOG("socketIdx close, index=%d, mainID=%d assistID=%d", index, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(NetMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	// 整合一下数据
	char buf[sizeof(NetMessageHead) + size];

	// 拼接包头
	NetMessageHead* pHead = (NetMessageHead*)buf;
	pHead->uMainID = mainID;
	pHead->uAssistantID = assistID;
	pHead->uMessageSize = sizeof(NetMessageHead) + size;
	pHead->uHandleCode = handleCode;
	pHead->uIdentification = uIdentification;

	// 包体
	if (pData && size > 0)
	{
		memcpy(buf + sizeof(NetMessageHead), pData, size);
	}

	// 数据加密
	if (encrypted)
	{
		Xor::Encrypt((uint8_t*)(buf + sizeof(NetMessageHead)), size);
	}

	// 投递到发送队列
	if (m_pSendDataLine)
	{
		SendDataLineHead lineHead;
		lineHead.socketIndex = index;
		lineHead.pBufferevent = pBufferevent;
		unsigned int addBytes = m_pSendDataLine->AddData(&lineHead.dataLineHead, sizeof(lineHead), 0, buf, pHead->uMessageSize);

		if (addBytes == 0)
		{
			ERROR_LOG("投递消息失败,mainID=%d,assistID=%d", mainID, assistID);
			return false;
		}
	}

	return true;
}

bool CTCPSocketManage::CenterServerSendData(int index, UINT msgID, void* pData, int size, int mainID, int assistID, int handleCode, int userID, void* pBufferevent)
{
	if (!IsConnected(index))
	{
		ERROR_LOG("socketIdx close, index=%d, mainID=%d assistID=%d", index, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(PlatformMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	// 整合一下数据
	char buf[sizeof(PlatformMessageHead) + size];

	// 拼接包头
	PlatformMessageHead* pHead = (PlatformMessageHead*)buf;
	pHead->MainHead.uMainID = mainID;
	pHead->MainHead.uAssistantID = assistID;
	pHead->MainHead.uMessageSize = sizeof(PlatformMessageHead) + size;
	pHead->MainHead.uHandleCode = handleCode;
	pHead->AssHead.msgID = msgID;
	pHead->AssHead.userID = userID;

	// 包体
	if (pData && size > 0)
	{
		memcpy(buf + sizeof(PlatformMessageHead), pData, size);
	}

	// 投递到发送队列
	if (m_pSendDataLine)
	{
		SendDataLineHead lineHead;
		lineHead.socketIndex = index;
		lineHead.pBufferevent = pBufferevent;
		unsigned int addBytes = m_pSendDataLine->AddData(&lineHead.dataLineHead, sizeof(lineHead), 0, buf, pHead->MainHead.uMessageSize);

		if (addBytes == 0)
		{
			ERROR_LOG("投递消息失败,mainID=%d,assistID=%d", mainID, assistID);
			return false;
		}
	}

	return true;
}

bool CTCPSocketManage::CloseSocket(int index)
{
	RemoveTCPSocketInfo(index);

	return true;
}

CDataLine* CTCPSocketManage::GetRecvDataLine()
{
	if (m_pService)
	{
		return m_pService->GetDataLine();
	}

	return NULL;
}

CDataLine* CTCPSocketManage::GetSendDataLine()
{
	return m_pSendDataLine;
}

UINT CTCPSocketManage::GetCurSocketSize()
{
	return m_uCurSocketSize;
}

bool CTCPSocketManage::IsConnected(int index)
{
	if (index < 0 || index >= m_socketInfoVec.size())
	{
		return false;
	}

	return m_socketInfoVec[index].isConnect;
}

void CTCPSocketManage::GetSocketSet(std::vector<UINT>& vec)
{
	vec.clear();

	CSignedLockObject LockObject(&m_csSocketInfoLock, true);

	for (auto iter = m_heartBeatSocketSet.begin(); iter != m_heartBeatSocketSet.end(); iter++)
	{
		vec.push_back(*iter);
	}
}

const std::vector<TCPSocketInfo>& CTCPSocketManage::GetSocketVector()
{
	return m_socketInfoVec;
}

const char* CTCPSocketManage::GetSocketIP(int index)
{
	if (index < 0 || index >= m_socketInfoVec.size())
	{
		return NULL;
	}

	return m_socketInfoVec[index].ip;
}

const TCPSocketInfo* CTCPSocketManage::GetTCPSocketInfo(int index)
{
	if (index < 0 || index >= m_socketInfoVec.size())
	{
		return NULL;
	}

	return &m_socketInfoVec[index];
}

int CTCPSocketManage::GetFreeSocketIndex()
{
	CSignedLockObject LockObject(&m_csSocketInfoLock, true);

	//<	获取空闲对象空间。
	int	iFreeHead = (m_iFreeHead + 1) % m_socketIndexVec.size();
	if (iFreeHead == m_iFreeTail)
	{
		return -1;
	}

	m_iFreeHead = iFreeHead;

	return m_socketIndexVec[iFreeHead];
}

int CTCPSocketManage::FreeSocketIndex(int index, bool islock /*= true*/)
{
	if (index < 0 || index >= m_socketIndexVec.size())
	{
		return -1;
	}

	CSignedLockObject LockObject(&m_csSocketInfoLock, false);

	if (islock)
	{
		LockObject.Lock();
	}

	///	回收对象空间。
	if (m_iFreeHead == m_iFreeTail)
	{
		return -2;
	}

	m_socketIndexVec[m_iFreeTail] = index;
	m_iFreeTail = (m_iFreeTail + 1) % m_socketIndexVec.size();

	return 0;
}

void CTCPSocketManage::AddTCPSocketInfo(int threadIndex, PlatformSocketInfo* pTCPSocketInfo)
{
	struct event_base* base = m_workBaseVec[threadIndex].base;
	struct bufferevent* bev = NULL;
	int fd = pTCPSocketInfo->acceptFd;

	// 分配索引算法
	int index = GetFreeSocketIndex();
	if (index < 0)
	{
		ERROR_LOG("分配索引失败！！！fd=%d,ip=%s", fd, pTCPSocketInfo->ip);
		close(fd);
		return;
	}

	bev = bufferevent_socket_new(base, fd, /*BEV_OPT_CLOSE_ON_FREE |*/ BEV_OPT_THREADSAFE);
	if (!bev)
	{
		ERROR_LOG("Error constructing bufferevent!,fd=%d,ip=%s", fd, pTCPSocketInfo->ip);
		close(fd);
		FreeSocketIndex(index);
		return;
	}

	// 设置应用层收发数据包，单次大小
	SetMaxSingleReadAndWrite(bev, SOCKET_RECV_BUF_SIZE, SOCKET_SEND_BUF_SIZE);

	// 生成回调函数参数，调用bufferevent_free要释放内存，否则内存泄露
	RecvThreadParam* pRecvThreadParam = new RecvThreadParam;
	pRecvThreadParam->pThis = this;
	pRecvThreadParam->index = index;

	// 添加事件，并设置好回调函数
	bufferevent_setcb(bev, ReadCB, NULL, EventCB, (void*)pRecvThreadParam);
	if (bufferevent_enable(bev, EV_READ | EV_ET) < 0)
	{
		ERROR_LOG("add event fail!!!,fd=%d,ip=%s", fd, pTCPSocketInfo->ip);
		close(fd);
		bufferevent_free(bev);
		SafeDelete(pRecvThreadParam);
		FreeSocketIndex(index);
		return;
	}

	// 设置读超时，当做心跳。网关服务器才需要
	if (m_iServiceType == SERVICE_TYPE_LOGON)
	{
		timeval tvRead;
		tvRead.tv_sec = CHECK_HEAETBEAT_SECS * KEEP_ACTIVE_HEARTBEAT_COUNT;
		tvRead.tv_usec = 0;
		bufferevent_set_timeouts(bev, &tvRead, NULL);
	}

	// 保存信息
	TCPSocketInfo tcpInfo;
	memcpy(tcpInfo.ip, pTCPSocketInfo->ip, sizeof(tcpInfo.ip));
	tcpInfo.acceptFd = pTCPSocketInfo->acceptFd;
	tcpInfo.acceptMsgTime = pTCPSocketInfo->acceptMsgTime;
	tcpInfo.bev = bev;
	tcpInfo.isConnect = true;
	tcpInfo.port = pTCPSocketInfo->port;
	if (!tcpInfo.lock)
	{
		tcpInfo.lock = new CSignedLock;
	}

	CSignedLockObject LockObject(&m_csSocketInfoLock, false);
	LockObject.Lock();

	if (m_socketInfoVec[index].isConnect)
	{
		ERROR_LOG("分配索引失败,fd=%d,ip=%s", fd, pTCPSocketInfo->ip);
		close(fd);
		bufferevent_free(bev);
		SafeDelete(pRecvThreadParam);
		FreeSocketIndex(index, false);
		return;
	}

	m_socketInfoVec[index] = tcpInfo;
	m_heartBeatSocketSet.insert((UINT)index);

	m_uCurSocketSize++;

	LockObject.UnLock();

	// 发送连接成功消息
	if (m_iServiceType == SERVICE_TYPE_CENTER)
	{
		CenterServerSendData(index, 0, NULL, 0, MDM_CONNECT, ASS_CONNECT_SUCCESS, 0, 0, tcpInfo.bev);
	}
	else
	{
		SendData(index, NULL, 0, MDM_CONNECT, ASS_CONNECT_SUCCESS, 0, 0, tcpInfo.bev);
	}

	CON_INFO_LOG("TCP connect [ip=%s port=%d index=%d fd=%d bufferevent=%p]",
		tcpInfo.ip, tcpInfo.port, index, tcpInfo.acceptFd, tcpInfo.bev);
}

void CTCPSocketManage::RemoveTCPSocketInfo(int index, int closetype/* = 0*/)
{
	if (index < 0 || index >= m_socketInfoVec.size())
	{
		ERROR_LOG("index=%d 超出范围", index);
		return;
	}

	ULONG uAccessIP = 0;

	CSignedLockObject LockObject(&m_csSocketInfoLock, false);

	// 加锁
	LockObject.Lock();

	auto& tcpInfo = m_socketInfoVec[index];

	// 重复调用
	if (!tcpInfo.isConnect)
	{
		return;
	}

	// 如果锁没有分配内存，就分配
	if (!tcpInfo.lock)
	{
		tcpInfo.lock = new CSignedLock;
	}

	// 回收索引
	FreeSocketIndex(index, false);

	uAccessIP = inet_addr(tcpInfo.ip);
	m_uCurSocketSize--;
	m_heartBeatSocketSet.erase((UINT)index);

	// 释放参数内存
	RecvThreadParam* pRecvThreadParam = (RecvThreadParam*)0x01;
	bufferevent_getcb(tcpInfo.bev, NULL, NULL, NULL, (void**)&pRecvThreadParam);
	if (pRecvThreadParam)
	{
		SafeDelete(pRecvThreadParam);
	}

	//// 服务器主动发起FIN包
	//if (!isClientAutoClose)
	//{
	//	close(tcpInfo.acceptFd);
	//}

	// 和发送线程相关的锁
	CSignedLockObject LockSendMsgObject(tcpInfo.lock, false);

	LockSendMsgObject.Lock();

	tcpInfo.isConnect = false;
	bufferevent_free(tcpInfo.bev);
	tcpInfo.bev = nullptr;

	// 解锁发送线程
	LockSendMsgObject.UnLock();

	// 解锁多线程
	LockObject.UnLock();

	// 如果没有设置BEV_OPT_CLOSE_ON_FREE 选项，则关闭socket
	close(tcpInfo.acceptFd);

	//// 释放锁的内存
	//SAFE_DELETE(tcpInfo.lock);

	// 回调业务层
	if (m_pService)
	{
		m_pService->OnSocketCloseEvent(uAccessIP, index, (UINT)tcpInfo.acceptMsgTime, m_socketType);
	}

	CON_INFO_LOG("TCP close [ip=%s port=%d index=%d fd=%d closetype=%d acceptTime=%lld]",
		tcpInfo.ip, tcpInfo.port, index, tcpInfo.acceptFd, closetype, tcpInfo.acceptMsgTime);
}

bool CTCPSocketManage::DispatchPacket(void* pBufferevent, int index, NetMessageHead* pHead, void* pData, int size)
{
	if (!pHead)
	{
		return false;
	}

	if (pHead->uMainID == MSG_MAIN_TEST) //心跳包
	{
		return true;
	}

	if (pHead->uMainID == MSG_MAIN_CONECT) //测试连接包
	{
		return SendData(index, NULL, 0, pHead->uMainID, pHead->uAssistantID, pHead->uHandleCode, 0, pBufferevent, pHead->uIdentification);
	}

	CDataLine* pDataLine = GetRecvDataLine();
	if (!pDataLine)
	{
		return false;
	}

	SocketReadLine msg;

	msg.uHandleSize = size;
	msg.uIndex = index;
	msg.pBufferevent = pBufferevent;
	msg.uAccessIP = 0;
	msg.netMessageHead = *pHead;
	msg.socketType = m_socketType;

	unsigned int addBytes = pDataLine->AddData(&msg.LineHead, sizeof(SocketReadLine), HD_SOCKET_READ, pData, size);

	if (addBytes == 0)
	{
		return false;
	}

	return true;
}

bool CTCPSocketManage::RecvData(bufferevent* bev, int index)
{
	if (bev == NULL)
	{
		ERROR_LOG("RecvData error bev == NULL");
		return false;
	}

	struct evbuffer* input = bufferevent_get_input(bev);

	size_t maxSingleRead = Min_(evbuffer_get_length(input), SOCKET_RECV_BUF_SIZE);

	char buf[maxSingleRead];

	size_t realAllSize = evbuffer_copyout(input, buf, sizeof(buf));
	if (realAllSize <= 0)
	{
		return false;
	}

	// 剩余处理数据
	size_t handleRemainSize = realAllSize;

	// 解出包头
	NetMessageHead* pNetHead = (NetMessageHead*)buf;

	// 错误判断
	if (handleRemainSize >= sizeof(NetMessageHead) && pNetHead->uMessageSize > SOCKET_RECV_BUF_SIZE)
	{
		// 消息格式不正确
		CloseSocket(index);
		ERROR_LOG("close socket 数据包超过缓冲区最大限制,index=%d", index);
		return false;
	}

	// 粘包处理
	while (handleRemainSize >= sizeof(NetMessageHead) && handleRemainSize >= pNetHead->uMessageSize)
	{
		unsigned int messageSize = pNetHead->uMessageSize;
		if (messageSize > MAX_TEMP_SENDBUF_SIZE)
		{
			// 消息格式不正确
			CloseSocket(index);
			ERROR_LOG("close socket 超过单包数据最大值,index=%d,messageSize=%u", index, messageSize);
			return false;
		}

		int realSize = messageSize - sizeof(NetMessageHead);
		if (realSize < 0)
		{
			// 数据包不够包头
			CloseSocket(index);
			ERROR_LOG("close socket 数据包不够包头");
			return false;
		}

		void* pData = NULL;
		if (realSize > 0)
		{
			// 没数据就为NULL
			pData = (void*)(buf + realAllSize - handleRemainSize + sizeof(NetMessageHead));
		}

		// 派发数据
		DispatchPacket(bev, index, pNetHead, pData, realSize);

		handleRemainSize -= messageSize;

		pNetHead = (NetMessageHead*)(buf + realAllSize - handleRemainSize);
	}

	evbuffer_drain(input, realAllSize - handleRemainSize);

	return true;
}

void CTCPSocketManage::SetTcpRcvSndBUF(int fd, int rcvBufSize, int sndBufSize)
{
	int optval = 0;
	unsigned int optLen = sizeof(int);

	getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&optval, &optLen);
	if (optval < rcvBufSize * 2)
	{
		optval = rcvBufSize;
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval));
	}

	getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&optval, &optLen);
	if (optval < sndBufSize * 2)
	{
		optval = sndBufSize;
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));
	}
}

void CTCPSocketManage::SetMaxSingleReadAndWrite(bufferevent* bev, int rcvBufSize, int sndBufSize)
{
	if (bufferevent_get_max_single_read(bev) < rcvBufSize &&
		bufferevent_set_max_single_read(bev, rcvBufSize) < 0)
	{
		ERROR_LOG("bufferevent_set_max_single_read fail,bev=%p", bev);
	}

	/*if (bufferevent_set_max_single_write(bev, sndBufSize) < 0)
	{
		ERROR_LOG("bufferevent_set_max_single_write fail,fd=%d", fd);
	}*/
}

void* CTCPSocketManage::ThreadAccept(void* pThreadData)
{
	CTCPSocketManage* pThis = (CTCPSocketManage*)pThreadData;
	if (!pThis)
	{
		std::cout << "thread param is null" << "{func=" << __FUNCTION__ << " line=" << __LINE__ << "}\n";
		pthread_exit(NULL);
	}

	sleep(1);

	INFO_LOG("ThreadAccept thread begin...");

	// libevent服务器 
	struct evconnlistener* listener;
	struct sockaddr_in sin;

	pThis->m_listenerBase = event_base_new();
	if (!pThis->m_listenerBase)
	{
		CON_ERROR_LOG("TCP Could not initialize libevent!");
		exit(1);
	}

	// 开始监听
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(pThis->m_port);
	sin.sin_addr.s_addr = strlen(pThis->m_bindIP) == 0 ? INADDR_ANY : inet_addr(pThis->m_bindIP);

	listener = evconnlistener_new_bind(pThis->m_listenerBase, ListenerCB, (void*)pThis,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE, TCP_ACCEPT_LIST_COUNT,
		(struct sockaddr*) & sin,
		sizeof(sin));

	if (!listener) {
		printf("Could not create a listener! 尝试换个端口或者稍等一会。");
		exit(1);
	}

	evconnlistener_set_error_cb(listener, AcceptErrorCB);

	// 获取接收线程池数量
	const CommonConfig& commonConfig = ConfigManage()->GetCommonConfig();
	int workBaseCount = commonConfig.WorkThreadNumber;
	if (workBaseCount < 1)
	{
		workBaseCount = get_nprocs();
	}

	// 初始工作线程信息
	RecvThreadParam param[workBaseCount];
	int socketPairBufSize = sizeof(PlatformSocketInfo) * MAX_POST_CONNECTED_COUNT;
	for (int i = 0; i < workBaseCount; i++)
	{
		param[i].index = i;
		param[i].pThis = pThis;

		WorkThreadInfo workInfo;
		int fd[2];
		int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);
		if (ret == -1) {
			SYS_ERROR_LOG("socketpair");
			exit(1);
		}

		workInfo.read_fd = fd[0];
		workInfo.write_fd = fd[1];

		SetTcpRcvSndBUF(workInfo.read_fd, socketPairBufSize, socketPairBufSize);
		SetTcpRcvSndBUF(workInfo.write_fd, socketPairBufSize, socketPairBufSize);

		workInfo.base = event_base_new();
		if (!workInfo.base)
		{
			CON_ERROR_LOG("TCP Could not initialize libevent!");
			exit(1);
		}

		workInfo.event = event_new(workInfo.base, workInfo.read_fd, EV_READ, ThreadLibeventProcess, (void*)&param[i]);
		if (!workInfo.event)
		{
			CON_ERROR_LOG("TCP Could not create event!");
			exit(1);
		}

		if (event_add(workInfo.event, NULL) < 0)
		{
			CON_ERROR_LOG("TCP event_add ERROR");
			exit(1);
		}

		pThis->m_workBaseVec.push_back(workInfo);
	}

	// 开辟工作线程池
	for (int i = 0; i < workBaseCount; i++)
	{
		pthread_t threadID = 0;
		int err = pthread_create(&threadID, NULL, ThreadRSSocket, (void*)&param[i]);
		if (err != 0)
		{
			SYS_ERROR_LOG("ThreadRSSocket failed");
			exit(1);
		}

		GameLogManage()->AddLogFile(threadID, THREAD_TYPE_RECV);
	}

	event_base_dispatch(pThis->m_listenerBase);

	evconnlistener_free(listener);
	event_base_free(pThis->m_listenerBase);
	for (int i = 0; i < workBaseCount; i++)
	{
		event_base_free(pThis->m_workBaseVec[i].base);
		event_free(pThis->m_workBaseVec[i].event);
	}

	INFO_LOG("ThreadAccept thread exit.");

	pthread_exit(NULL);
}

void* CTCPSocketManage::ThreadRSSocket(void* pThreadData)
{
	RecvThreadParam* param = (RecvThreadParam*)pThreadData;
	if (!param)
	{
		std::cout << "thread param is null" << "{func=" << __FUNCTION__ << " line=" << __LINE__ << "}\n";
		pthread_exit(NULL);
	}

	// 处于监听状态
	event_base_dispatch(param->pThis->m_workBaseVec[param->index].base);

	pthread_exit(NULL);
}

void* CTCPSocketManage::ThreadSendMsg(void* pThreadData)
{
	CTCPSocketManage* pThis = (CTCPSocketManage*)pThreadData;
	if (!pThis)
	{
		std::cout << "thread param is null" << "{func=" << __FUNCTION__ << " line=" << __LINE__ << "}\n";
		pthread_exit(NULL);
	}

	CDataLine* pDataLine = pThis->GetSendDataLine();

	//数据缓存
	DataLineHead* pDataLineHead = NULL;

	sleep(3);

	INFO_LOG("CTCPSocketManage::ThreadSendMsg thread begin...");

	while (pThis->m_running)
	{
		//获取数据
		unsigned int bytes = pDataLine->GetData(&pDataLineHead);
		if (bytes == 0 || pDataLineHead == NULL)
		{
			continue;
		}

		//处理数据
		SendDataLineHead* pSocketSend = (SendDataLineHead*)pDataLineHead;
		void* pBuffer = (void*)(pSocketSend + 1);
		size_t size = pSocketSend->dataLineHead.uSize;
		int index = pSocketSend->socketIndex;

		if (size > sizeof(SendDataLineHead) && index >= 0 && index < pThis->m_socketInfoVec.size())
		{
			auto& tcpInfo = pThis->m_socketInfoVec[index];
			if (tcpInfo.lock)
			{
				CSignedLockObject LockSendMsgObject(tcpInfo.lock, false);
				LockSendMsgObject.Lock();

				if (tcpInfo.isConnect && tcpInfo.bev)
				{
					if (bufferevent_write(tcpInfo.bev, pBuffer, size - sizeof(SendDataLineHead)) < 0)
					{
						ERROR_LOG("发送数据失败，index=%d socketfd=%d bev=%p,", index, tcpInfo.acceptFd, tcpInfo.bev);
					}
				}

				// 解锁发送线程
				LockSendMsgObject.UnLock();
			}
		}
		else
		{
			ERROR_LOG("发送数据失败，index=%d 超出范围", index);
		}

		// 释放内存
		if (pDataLineHead)
		{
			free(pDataLineHead);
		}
	}

	INFO_LOG("CTCPSocketManage::ThreadSendMsg exit.");

	pthread_exit(NULL);
}

void CTCPSocketManage::ListenerCB(evconnlistener* listener, evutil_socket_t fd, sockaddr* sa, int socklen, void* data)
{
	CTCPSocketManage* pThis = (CTCPSocketManage*)data;

	// 获取连接信息
	struct sockaddr_in* addrClient = (struct sockaddr_in*)sa;
	PlatformSocketInfo tcpInfo;
	tcpInfo.acceptMsgTime = time(NULL);
	strcpy(tcpInfo.ip, inet_ntoa(addrClient->sin_addr));
	tcpInfo.port = ntohs(addrClient->sin_port);
	tcpInfo.acceptFd = fd;

	if (pThis->GetCurSocketSize() >= pThis->m_uMaxSocketSize)
	{
		ERROR_LOG("服务器已经满：fd=%d [ip:%s %d][人数：%u/%u]", fd, tcpInfo.ip, tcpInfo.port, pThis->GetCurSocketSize(), pThis->m_uMaxSocketSize);

		// 分配失败
		NetMessageHead netHead;

		netHead.uMainID = 100;
		netHead.uAssistantID = 3;
		netHead.uHandleCode = ERROR_SERVICE_FULL;//服务器人数已满
		netHead.uMessageSize = sizeof(NetMessageHead);

		sendto(fd, (char*)&netHead, sizeof(NetMessageHead), 0, (sockaddr*)&sa, sizeof(sockaddr_in));

		close(fd);

		return;
	}

	// 设置底层收发缓冲区
	SetTcpRcvSndBUF(fd, SOCKET_RECV_BUF_SIZE, SOCKET_SEND_BUF_SIZE);

	// memcached中线程负载均衡算法
	static int lastThreadIndex = 0;
	lastThreadIndex = lastThreadIndex % pThis->m_workBaseVec.size();

	// 投递到接收线程
	if (write(pThis->m_workBaseVec[lastThreadIndex].write_fd, &tcpInfo, sizeof(tcpInfo)) < sizeof(tcpInfo))
	{
		ERROR_LOG("投递连接消息失败,fd=%d", fd);
	}

	lastThreadIndex++;
}

void CTCPSocketManage::ReadCB(bufferevent* bev, void* data)
{
	RecvThreadParam* param = (RecvThreadParam*)data;
	CTCPSocketManage* pThis = param->pThis;
	int index = param->index;

	// 处理数据，包头解析
	pThis->RecvData(bev, index);
}

void CTCPSocketManage::EventCB(bufferevent* bev, short events, void* data)
{
	RecvThreadParam* param = (RecvThreadParam*)data;
	CTCPSocketManage* pThis = param->pThis;
	int index = param->index;
	int closetype = SOCKET_CLOSE_TYPE_CLIENT;

	if (events & BEV_EVENT_EOF)
	{
		// 正常结束
	}
	else if (events & BEV_EVENT_ERROR)
	{
		// windows正常结束
	}
	else if (events & BEV_EVENT_TIMEOUT) // 长时间没有收到，客户端发过来的数据，读取数据超时
	{
		INFO_LOG("心跳踢人 index=%d fd=%d", index, pThis->m_socketInfoVec[index].acceptFd);
		closetype = SOCKET_CLOSE_TYPE_HEART;
	}
	else
	{
		SYS_ERROR_LOG("Got an error on the connection,events=%d", events);
	}

	pThis->RemoveTCPSocketInfo(index, closetype);
}

void CTCPSocketManage::AcceptErrorCB(evconnlistener* listener, void* data)
{
	ERROR_LOG("accept error:%s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
}

void CTCPSocketManage::ThreadLibeventProcess(int readfd, short which, void* arg)
{
	RecvThreadParam* param = (RecvThreadParam*)arg;
	CTCPSocketManage* pThis = param->pThis;
	int threadIndex = param->index;
	if (threadIndex < 0 || threadIndex >= pThis->m_workBaseVec.size() || readfd != pThis->m_workBaseVec[threadIndex].read_fd)
	{
		ERROR_LOG("######  threadIndex = %d", threadIndex);
		exit(1);
	}

	char buf[sizeof(PlatformSocketInfo) * MAX_POST_CONNECTED_COUNT] = "";

	int realAllSize = read(readfd, buf, sizeof(buf));
	if (realAllSize < sizeof(PlatformSocketInfo) || realAllSize % sizeof(PlatformSocketInfo) != 0)
	{
		ERROR_LOG("#### ThreadLibeventProcess error size=%d,sizeof(PlatformSocketInfo)=%lld  ######", realAllSize, sizeof(PlatformSocketInfo));
		event_add(pThis->m_workBaseVec[threadIndex].event, NULL);
		return;
	}

	int countAcceptCount = realAllSize / sizeof(PlatformSocketInfo);
	for (int i = 0; i < countAcceptCount; i++)
	{
		PlatformSocketInfo* pTCPSocketInfo = (PlatformSocketInfo*)(buf + i * sizeof(PlatformSocketInfo));

		// 处理连接
		pThis->AddTCPSocketInfo(threadIndex, pTCPSocketInfo);
	}

	event_add(pThis->m_workBaseVec[threadIndex].event, NULL);
}
