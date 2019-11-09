#include "CommonHead.h"
#include "basemessage.h"
#include "log.h"
#include "configManage.h"
#include "DataLine.h"
#include "Xor.h"
#include "event2/thread.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "TCPSocket.h"


//通用变量的定义
const int ERROR_SERVICE_FULL = 15;			// 服务器人数已满
const unsigned int MSG_MAIN_TEST = 1;		// 测试消息
const unsigned int MSG_MAIN_CONECT = 2;		// 连接测试

//接收线程参数
struct RecvThreadParam
{
	CTCPSocketManage* pCTCPSocketManage;
	int index;
	RecvThreadParam()
	{
		pCTCPSocketManage = NULL;
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
}

CTCPSocketManage::~CTCPSocketManage()
{

}

bool CTCPSocketManage::Init(IServerSocketService* pService, int maxCount, int port, const char* ip/* = NULL*/)
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

	m_socketInfoMap.clear();
	m_workBaseVec.clear();

	// 设置libevent日志回调函数
	event_set_log_callback(EventLog);

	// 初始化线程
	if (evthread_use_pthreads() < 0)
	{
		CON_ERROR_LOG("evthread_use_pthreads fail");
		return false;
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

	event_base_loopbreak(m_listenerBase);
	for (size_t i = 0; i < m_workBaseVec.size(); i++)
	{
		event_base_loopbreak(m_workBaseVec[i].base);
	}

	INFO_LOG("service TCPSocketManage stop end...");

	return true;
}

bool CTCPSocketManage::SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, void* pBufferevent, unsigned int uIdentification/* = 0*/)
{
	if (index < 0)
	{
		ERROR_LOG("invalid socketIdx, index=%d, mainID=%d assistID=%d", index, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(NetMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	if (!pBufferevent)
	{
		ERROR_LOG("pBufferevent =NULL");
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

bool CTCPSocketManage::SendData(void* pBufferevent, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, unsigned int uIdentification)
{
	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(NetMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	if (!pBufferevent)
	{
		ERROR_LOG("pBufferevent =NULL");
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
		lineHead.socketIndex = -1;
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
	if (index < 0)
	{
		ERROR_LOG("invalid socketIdx, index=%d, mainID=%d assistID=%d", index, mainID, assistID);
		return false;
	}

	if (size < 0 || size > MAX_TEMP_SENDBUF_SIZE - sizeof(PlatformMessageHead))
	{
		ERROR_LOG("invalid message size size=%d", size);
		return false;
	}

	if (!pBufferevent)
	{
		ERROR_LOG("pBufferevent =NULL");
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
	RemoveTCPSocketStatus(index);

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

const std::unordered_map<int, TCPSocketInfo>& CTCPSocketManage::GetSocketMap()
{
	return m_socketInfoMap;
}

const char* CTCPSocketManage::GetSocketIP(int index)
{
	auto iter = m_socketInfoMap.find(index);
	if (iter == m_socketInfoMap.end())
	{
		return NULL;
	}

	return iter->second.ip;
}

bufferevent* CTCPSocketManage::GetTCPBufferEvent(int index)
{
	auto iter = m_socketInfoMap.find(index);
	if (iter == m_socketInfoMap.end())
	{
		return NULL;
	}

	if (iter->second.isConnect)
	{
		return iter->second.bev;
	}

	return NULL;
}

void CTCPSocketManage::AddTCPSocketInfo(int index, const TCPSocketInfo& info)
{
	// 发送连接成功消息
	if (m_iServiceType == SERVICE_TYPE_CENTER)
	{
		CenterServerSendData(index, 0, NULL, 0, MDM_CONNECT, ASS_CONNECT_SUCCESS, 0, 0, info.bev);
	}
	else
	{
		SendData(index, NULL, 0, MDM_CONNECT, ASS_CONNECT_SUCCESS, 0, 0, info.bev);
	}

	CON_INFO_LOG("client connect .ip=%s port=%d fd=%d bufferevent=%p", info.ip, info.port, index, info.bev);

	CSignedLockObject LockObject(&m_csSocketMapLock, true);

	m_socketInfoMap[index] = info;

	m_uCurSocketSize++;
}

bool CTCPSocketManage::SetTCPSocketRecvTime(int index, const time_t& llLastRecvMsgTime, const time_t& llLastSendMsgTime)
{
	auto iter = m_socketInfoMap.find(index);
	if (iter == m_socketInfoMap.end())
	{
		return false;
	}

	if (llLastRecvMsgTime)
	{
		iter->second.lastRecvMsgTime = llLastRecvMsgTime;
	}

	if (llLastSendMsgTime)
	{
		iter->second.lastSendMsgTime = llLastSendMsgTime;
	}

	return true;
}

void CTCPSocketManage::RemoveTCPSocketStatus(int index, bool isClientAutoClose/* = false*/)
{
	CSignedLockObject LockObject(&m_csSocketMapLock, false);

	ULONG uAccessIP = 0;

	// 加锁
	LockObject.Lock();

	auto iter = m_socketInfoMap.find(index);
	if (iter == m_socketInfoMap.end())
	{
		return;
	}

	// 重复调用
	if (!iter->second.isConnect)
	{
		return;
	}

	uAccessIP = inet_addr(iter->second.ip);
	iter->second.isConnect = false;
	m_uCurSocketSize--;

	int fd = bufferevent_getfd(iter->second.bev);
	if (fd == index)
	{
		// 服务器主动发起FIN包
		if (!isClientAutoClose)
		{
			close(fd);
			bufferevent_disable(iter->second.bev, EV_READ | EV_ET);
		}

		bufferevent_free(iter->second.bev);
	}
	else
	{
		CON_ERROR_LOG("######### 重要错误：index=%d,fd=%d ############", index, fd);
	}

	// 解锁
	LockObject.UnLock();

	// 回调业务层
	if (m_pService)
	{
		m_pService->OnSocketCloseEvent(uAccessIP, index, (UINT)iter->second.acceptMsgTime);
	}

	CON_INFO_LOG("close socket. ip=%s port=%d,fd=%d,bufferevent=%p,isClientAutoClose:%d,acceptTime=%lld",
		iter->second.ip, iter->second.port, fd, iter->second.bev, isClientAutoClose, iter->second.acceptMsgTime);
}

bool CTCPSocketManage::DispatchPacket(void* pBufferevent, int index, NetMessageHead* pHead, void* pData, int size)
{
	if (!pHead || !pBufferevent)
	{
		return false;
	}

	CDataLine* pDataLine = GetRecvDataLine();
	if (!pDataLine)
	{
		return false;
	}

	if (pHead->uMainID == MSG_MAIN_TEST) //心跳包
	{
		return true;
	}

	if (pHead->uMainID == MSG_MAIN_CONECT) //测试连接包
	{
		CDataLine* pSendDataLine = GetSendDataLine();

		// 投递到发送队列
		if (pSendDataLine)
		{
			SendDataLineHead lineHead;
			lineHead.socketIndex = index;
			lineHead.pBufferevent = pBufferevent;
			unsigned int addBytes = pSendDataLine->AddData(&lineHead.dataLineHead, sizeof(lineHead), 0, pHead, sizeof(NetMessageHead));

			if (addBytes == 0)
			{
				return false;
			}
		}

		return true;
	}

	SocketReadLine msg;

	msg.uHandleSize = size;
	msg.uIndex = index;
	msg.pBufferevent = pBufferevent;
	msg.uAccessIP = 0;
	msg.netMessageHead = *pHead;

	unsigned int addBytes = pDataLine->AddData(&msg.LineHead, sizeof(SocketReadLine), HD_SOCKET_READ, pData, size);

	if (addBytes == 0)
	{
		return false;
	}

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
		CON_ERROR_LOG("Could not initialize libevent!");
		exit(1);
	}

	// 开始监听
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(pThis->m_port);
	sin.sin_addr.s_addr = strlen(pThis->m_bindIP) == 0 ? INADDR_ANY : inet_addr(pThis->m_bindIP);

	listener = evconnlistener_new_bind(pThis->m_listenerBase, ListenerCB, (void*)pThis,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE, -1,
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
	if (workBaseCount <= 1)
	{
		workBaseCount = 4;
	}

	// 初始工作线程信息
	RecvThreadParam param[workBaseCount];
	int socketPairBufSize = sizeof(TCPSocketInfo) * MAX_POST_CONNECTED_COUNT;
	for (int i = 0; i < workBaseCount; i++)
	{
		param[i].index = i;
		param[i].pCTCPSocketManage = pThis;

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
			CON_ERROR_LOG("Could not initialize libevent!");
			exit(1);
		}

		workInfo.event = event_new(workInfo.base, workInfo.read_fd, EV_READ, ThreadLibeventProcess, (void*)&param[i]);
		if (!workInfo.event)
		{
			CON_ERROR_LOG("Could not create event!");
			exit(1);
		}

		if (event_add(workInfo.event, NULL) < 0)
		{
			CON_ERROR_LOG("event_add ERROR");
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
	event_base_dispatch(param->pCTCPSocketManage->m_workBaseVec[param->index].base);

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
		try
		{
			//获取数据
			unsigned int bytes = pDataLine->GetData(&pDataLineHead);
			if (bytes == 0 || pDataLineHead == NULL)
			{
				continue;
			}

			//处理数据
			SendDataLineHead* pSocketSend = (SendDataLineHead*)pDataLineHead;
			void* pBuffer = NULL;
			unsigned int size = pSocketSend->dataLineHead.uSize;

			if (size > sizeof(SendDataLineHead))
			{
				pBuffer = (void*)(pSocketSend + 1);			// 移动一个SendDataLineHead

				bufferevent* bev = (bufferevent*)pSocketSend->pBufferevent;
				if (bev)
				{
					struct evbuffer* output = bufferevent_get_output(bev);
					size_t outputLen = evbuffer_get_length(output);
					if (outputLen > MAX_EVBUFFER_WRITE_SIZE)
					{
						ERROR_LOG("发送数据失败，缓冲区太大，bev=%p,socketfd=%d,size=%lld", bev, pSocketSend->socketIndex, outputLen);
					}
					else
					{
						if (bufferevent_write(bev, pBuffer, size - sizeof(SendDataLineHead)) < 0)
						{
							ERROR_LOG("发送数据失败，bev=%p,socketfd=%d", bev, pSocketSend->socketIndex);
						}
						else
						{
							// 设置发送时间
							// time_t currTime = time(NULL);
							//pThis->SetTCPSocketRecvTime(pSocketSend->socketIndex, 0, currTime);
						}
					}
				}
				else
				{
					ERROR_LOG("发送数据失败，pBufferevent=NULL，或者连接关闭,socketfd=%d", pSocketSend->socketIndex);
				}

			}

			// 释放内存
			if (pDataLineHead)
			{
				free(pDataLineHead);
			}
		}
		catch (...)
		{
			ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
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
	TCPSocketInfo tcpInfo;
	tcpInfo.acceptMsgTime = tcpInfo.lastRecvMsgTime = tcpInfo.lastSendMsgTime = time(NULL);
	strcpy(tcpInfo.ip, inet_ntoa(addrClient->sin_addr));
	tcpInfo.port = ntohs(addrClient->sin_port);
	tcpInfo.isConnect = true;
	tcpInfo.acceptFd = fd;

	if (pThis->GetCurSocketSize() >= pThis->m_uMaxSocketSize)
	{
		ERROR_LOG("服务器已经满：fd=%d %u/%u", fd, pThis->GetCurSocketSize(), pThis->m_uMaxSocketSize);

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
	CTCPSocketManage* pThis = (CTCPSocketManage*)data;
	struct evbuffer* input = bufferevent_get_input(bev);
	int fd = bufferevent_getfd(bev);

	size_t maxSingleRead = Min_(evbuffer_get_length(input), SOCKET_RECV_BUF_SIZE);

	char buf[maxSingleRead];

	size_t realAllSize = evbuffer_copyout(input, buf, sizeof(buf));
	if (realAllSize <= 0)
	{
		return;
	}

	// 剩余处理数据
	size_t handleRemainSize = realAllSize;

	// 解出包头
	NetMessageHead* pNetHead = (NetMessageHead*)buf;

	while (handleRemainSize >= sizeof(NetMessageHead) && handleRemainSize >= pNetHead->uMessageSize)
	{
		unsigned int messageSize = pNetHead->uMessageSize;
		if (messageSize > MAX_TEMP_SENDBUF_SIZE)
		{
			// 消息格式不正确
			pThis->CloseSocket(fd);
			ERROR_LOG("消息格式不正确");
			return;
		}

		int realSize = messageSize - sizeof(NetMessageHead);
		if (realSize < 0)
		{
			// 数据包不够包头
			pThis->CloseSocket(fd);
			ERROR_LOG("数据包不够包头");
			return;
		}

		void* pData = NULL;
		if (realSize > 0)
		{
			// 没数据就为NULL
			pData = (void*)(buf + realAllSize - handleRemainSize + sizeof(NetMessageHead));
		}

		// 派发数据
		pThis->DispatchPacket(bev, fd, pNetHead, pData, realSize);

		handleRemainSize -= messageSize;

		pNetHead = (NetMessageHead*)(buf + realAllSize - handleRemainSize);
	}

	evbuffer_drain(input, realAllSize - handleRemainSize);
}

void CTCPSocketManage::EventCB(bufferevent* bev, short events, void* data)
{
	CTCPSocketManage* pThis = (CTCPSocketManage*)data;

	int fd = bufferevent_getfd(bev);

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
		CON_INFO_LOG("心跳踢人 fd=%d,bufferevent=%p", fd, bev);
	}
	else
	{
		SYS_ERROR_LOG("Got an error on the connection,events=%d", events);
	}

	pThis->RemoveTCPSocketStatus(fd, true);
}

void CTCPSocketManage::AcceptErrorCB(evconnlistener* listener, void* data)
{
	CON_ERROR_LOG("accept error:%s", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
}

void CTCPSocketManage::ThreadLibeventProcess(int readfd, short which, void* arg)
{
	RecvThreadParam* param = (RecvThreadParam*)arg;
	CTCPSocketManage* pThis = param->pCTCPSocketManage;
	int threadIndex = param->index;
	if (threadIndex < 0 || threadIndex >= pThis->m_workBaseVec.size() || readfd != pThis->m_workBaseVec[threadIndex].read_fd)
	{
		CON_ERROR_LOG("######  threadIndex = %d", threadIndex);
		exit(1);
	}

	char buf[sizeof(TCPSocketInfo) * MAX_POST_CONNECTED_COUNT] = "";

	int realAllSize = read(readfd, buf, sizeof(buf));
	if (realAllSize < sizeof(TCPSocketInfo) || realAllSize % sizeof(TCPSocketInfo) != 0)
	{
		CON_ERROR_LOG("#### ThreadLibeventProcess error size=%d,sizeof(TCPSocketInfo)=%lld  ######", realAllSize, sizeof(TCPSocketInfo));
		event_add(pThis->m_workBaseVec[threadIndex].event, NULL);
		return;
	}

	int countAcceptCount = realAllSize / sizeof(TCPSocketInfo);
	for (int i = 0; i < countAcceptCount; i++)
	{
		TCPSocketInfo* pTCPSocketInfo = (TCPSocketInfo*)(buf + i * sizeof(TCPSocketInfo));

		struct event_base* base = pThis->m_workBaseVec[threadIndex].base;
		struct bufferevent* bev = NULL;
		int fd = pTCPSocketInfo->acceptFd;

		bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
		if (!bev)
		{
			ERROR_LOG("Error constructing bufferevent!,fd=%d,ip=%s", fd, pTCPSocketInfo->ip);
			close(fd);
			continue;
		}

		// 设置应用层收发数据包，单次大小
		SetMaxSingleReadAndWrite(bev, SOCKET_RECV_BUF_SIZE, SOCKET_SEND_BUF_SIZE);

		// 添加事件，并设置好回调函数
		bufferevent_setcb(bev, ReadCB, NULL, EventCB, pThis);
		if (bufferevent_enable(bev, EV_READ | EV_ET) < 0)
		{
			ERROR_LOG("add event fail!!!,fd=%d,ip=%s", fd, pTCPSocketInfo->ip);
			pThis->CloseSocket(fd);
			continue;
		}

		// 设置读超时，当做心跳。网关服务器才需要
		if (pThis->m_iServiceType == SERVICE_TYPE_LOGON)
		{
			timeval tvRead;
			tvRead.tv_sec = CHECK_HEAETBEAT_SECS * KEEP_ACTIVE_HEARTBEAT_COUNT;
			tvRead.tv_usec = 0;
			bufferevent_set_timeouts(bev, &tvRead, NULL);
		}

		// 保存信息
		pTCPSocketInfo->bev = bev;
		pThis->AddTCPSocketInfo(fd, *pTCPSocketInfo);
	}

	event_add(pThis->m_workBaseVec[threadIndex].event, NULL);
}

void CTCPSocketManage::EventLog(int severity, const char* msg)
{
	switch (severity)
	{
	case EVENT_LOG_DEBUG:
		CON_INFO_LOG("libevent[debug]:%s", msg);
		break;
	case EVENT_LOG_MSG:
		CON_INFO_LOG("libevent[msg]:%s", msg);
		break;
	case EVENT_LOG_WARN:
		CON_INFO_LOG("libevent[warn]:%s", msg);
		break;
	case EVENT_LOG_ERR:
		CON_ERROR_LOG("########## libevent内核错误:%s ##########", msg);
		break;
	default:
		CON_INFO_LOG("libevent:%s", msg);
		break;
	}
}