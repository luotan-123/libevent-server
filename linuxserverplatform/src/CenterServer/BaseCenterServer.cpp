#include "CommonHead.h"
#include "BaseCenterServer.h"


//处理线程启动结构
struct HandleThreadStartStruct
{
	//变量定义
	CFIFOEvent*							pFIFO;						//启动事件
	CBaseCenterServer					* pMainManage;				//数据管理指针
};

/*****************************************************************************************************************/
CBaseCenterServer::CBaseCenterServer()
{
	m_bInit = false;
	m_bRun = false;
	m_hHandleThread = 0;
	::memset(&m_DllInfo, 0, sizeof(m_DllInfo));
	::memset(&m_InitData, 0, sizeof(m_InitData));
	::memset(&m_KernelData, 0, sizeof(m_KernelData));

	m_pRedis = NULL;
	m_pRedisPHP = NULL;
	m_pServerTimer = NULL;
}

CBaseCenterServer::~CBaseCenterServer()
{
	SAFE_DELETE(m_pRedis);
	SAFE_DELETE(m_pRedisPHP);
	SafeDeleteArray(m_pServerTimer);
}

//初始化函数 
bool CBaseCenterServer::Init(ManageInfoStruct * pInitData)
{
	INFO_LOG("CBaseCenterServer Init begin...");

	if (m_bInit == true)
	{
		ERROR_LOG("is already have been inited...");
		return false;
	}

	if (!pInitData)
	{
		ERROR_LOG("invalid params input pInitData=%p", pInitData);
		return false;
	}

	if (m_bInit == true)
	{
		ERROR_LOG("is already been inited");
		return false;
	}

	//设置数据
	m_InitData = *pInitData;

	bool ret = false;

	// 加载参数
	ret = PreInitParameter(&m_InitData, &m_KernelData);
	if (!ret)
	{
		ERROR_LOG("PreInitParameter failed");
		return false;
	}

	// redis 相关
	m_pRedis = new CRedisCenter;
	if (!m_pRedis)
	{
		ERROR_LOG("create redis object failed");
		return false;
	}

	ret = m_pRedis->Init();
	if (!ret)
	{
		ERROR_LOG("连接redis失败");
		return false;
	}

	m_pRedisPHP = new CRedisPHP;
	if (!m_pRedisPHP)
	{
		ERROR_LOG("create redis object failed");
		return false;
	}

	ret = m_pRedisPHP->Init();
	if (!ret)
	{
		ERROR_LOG("连接PHPredis失败");
		return false;
	}

	// 初始化网络
	ret = m_TCPSocket.Init(this, m_InitData.uMaxPeople, m_InitData.uListenPort, 0, m_InitData.szCenterIP);
	if (!ret)
	{
		ERROR_LOG("TCPSocket Init failed");
		return false;
	}

	// 初始化定时器
	int iServerTimerNums = Min_(MAX_TIMER_THRED_NUMS, ConfigManage()->GetCommonConfig().TimerThreadNumber);
	iServerTimerNums = iServerTimerNums <= 0 ? 1 : iServerTimerNums;
	m_pServerTimer = new CServerTimer[iServerTimerNums];
	if (!m_pServerTimer)
	{
		ERROR_LOG("CServerTimer Init failed");
		return false;
	}

	ret = OnInit(&m_InitData, &m_KernelData);
	if (!ret)
	{
		ERROR_LOG("OnInit failed");
		return false;
	}

	m_bInit = true;

	INFO_LOG("CBaseCenterServer Init end");

	return true;
}

//取消初始化函数 
bool CBaseCenterServer::UnInit()
{
	//停止服务
	if (m_bRun)
	{
		Stop();
	}

	m_bInit = false;
	m_TCPSocket.UnInit();

	//设置数据
	memset(&m_DllInfo, 0, sizeof(m_DllInfo));
	memset(&m_InitData, 0, sizeof(m_InitData));
	memset(&m_KernelData, 0, sizeof(m_KernelData));

	//释放redis
	m_pRedis->Stop();
	SAFE_DELETE(m_pRedis);
	m_pRedisPHP->Stop();
	SAFE_DELETE(m_pRedisPHP);
	
	//删除定时器
	SafeDeleteArray(m_pServerTimer);

	//调用接口
	OnUnInit();

	return true;
}

//启动函数
bool CBaseCenterServer::Start()
{
	INFO_LOG("CBaseCenterServer Start begin...");

	if (m_bInit == false || m_bRun == true)
	{
		ERROR_LOG("CBaseCenterServer already been inited or running");
		return false;
	}

	m_bRun = true;
	bool ret = false;

	// 创建管道
	CFIFOEvent fifo("/tmp/CBaseCenterServer-Start-fifo");

	// 启动网络模块
	ret = m_TCPSocket.Start(SERVICE_TYPE_CENTER);
	if (!ret)
	{
		ERROR_LOG("TCPSocket Start failed");
		return false;
	}

	// 启动定时器
	for (int i = 0; i < GetNewArraySize(m_pServerTimer); i++)
	{
		if (!m_pServerTimer[i].Start(&m_DataLine))
		{
			ERROR_LOG("CBaseMainManage::m_pServerTimer.Start 定时器启动失败");
			return false;
		}
	}

	//启动处理线程
	HandleThreadStartStruct	ThreadStartData;
	ThreadStartData.pMainManage = this;
	ThreadStartData.pFIFO = &fifo;
	int err = pthread_create(&m_hHandleThread, NULL, LineDataHandleThread, (void*)&ThreadStartData);
	if (err != 0)
	{
		SYS_ERROR_LOG("pthread_create LineDataHandleThread failed");
		return false;
	}

	// 关联大厅业务逻辑线程与对应日志文件
	GameLogManage()->AddLogFile(m_hHandleThread, THREAD_TYPE_LOGIC);

	//调用接口
	ret = OnStart();
	if (!ret)
	{
		ERROR_LOG("OnStart failed");
		return false;
	}

	// 等待子线程读取线程参数
	fifo.WaitForEvent();

	INFO_LOG("CBaseCenterServer Start end.");

	return true;
}

//停止服务
bool CBaseCenterServer::Stop()
{
	INFO_LOG("CBaseCenterServer Stop begin...");

	if (m_bRun == false)
	{
		ERROR_LOG("is not running...");
		return false;
	}

	m_bRun = false;

	// 先关闭网络模块
	m_TCPSocket.Stop();

	//退出处理线程
	if (m_hHandleThread)
	{
		pthread_cancel(m_hHandleThread);
		m_hHandleThread = 0;
	}

	//关闭定时器
	for (int i = 0; i < GetNewArraySize(m_pServerTimer); i++)
	{
		m_pServerTimer[i].Stop();
	}

	//清理队列数据
	m_DataLine.CleanLineData();

	// 上层接口
	OnStop();		// 主线程关闭并清理资源(这里需要依赖redis和db)

	INFO_LOG("CBaseCenterServer Stop end.");

	return true;
}

//网络关闭处理
bool CBaseCenterServer::OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime)
{
	SocketCloseLine SocketClose;
	SocketClose.uConnectTime = uConnectTime;
	SocketClose.uIndex = uIndex;
	SocketClose.uAccessIP = uAccessIP;
	return (m_DataLine.AddData(&SocketClose.LineHead, sizeof(SocketClose), HD_SOCKET_CLOSE) != 0);
}

//网络消息处理
bool CBaseCenterServer::OnSocketReadEvent(CTCPSocket * pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, UINT dwHandleID)
{
	if (!pNetHead)
	{
		return false;
	}

	SocketReadLine SocketRead;

	SocketRead.uHandleSize = uSize;
	SocketRead.uIndex = uIndex;
	SocketRead.dwHandleID = dwHandleID;
	SocketRead.uAccessIP = 0;		//TODO
	SocketRead.netMessageHead = *pNetHead;
	return m_DataLine.AddData(&SocketRead.LineHead, sizeof(SocketRead), HD_SOCKET_READ, pData, uSize) != 0;
}

//设定定时器
bool CBaseCenterServer::SetTimer(UINT uTimerID, UINT uElapse, BYTE timerType/* = SERVERTIMER_TYPE_PERISIST*/)
{
	if (!m_pServerTimer)
	{
		ERROR_LOG("no timer run");
		return false;
	}

	int iTimerCount = GetNewArraySize(m_pServerTimer);
	if (iTimerCount <= 0 || iTimerCount > MAX_TIMER_THRED_NUMS)
	{
		ERROR_LOG("timer error");
		return false;
	}

	m_pServerTimer[uTimerID % iTimerCount].SetTimer(uTimerID, uElapse, timerType);

	return true;
}

//清除定时器
bool CBaseCenterServer::KillTimer(UINT uTimerID)
{
	if (!m_pServerTimer)
	{
		ERROR_LOG("no timer run");
		return false;
	}

	int iTimerCount = GetNewArraySize(m_pServerTimer);
	if (iTimerCount <= 0 || iTimerCount > MAX_TIMER_THRED_NUMS)
	{
		ERROR_LOG("timer error");
		return false;
	}

	m_pServerTimer[uTimerID % iTimerCount].KillTimer(uTimerID);

	return true;
}

//队列数据处理线程
void* CBaseCenterServer::LineDataHandleThread(void* pThreadData)
{
	INFO_LOG("LineDataHandleThread start...");

	HandleThreadStartStruct* pData = (HandleThreadStartStruct *)pThreadData;
	CBaseCenterServer	* pThis = pData->pMainManage;
	CDataLine* pDataLine = &pThis->m_DataLine;
	CFIFOEvent* pCFIFOEvent = pData->pFIFO;
	
	//线程数据读取完成
	pCFIFOEvent->SetEvent();

	//数据缓存
	BYTE szBuffer[MAX_DATALINE_SIZE] = "";
	DataLineHead* pDataLineHead = (DataLineHead *)szBuffer;

	while (pThis->m_bRun)
	{
		//BOOL bSuccess = ::GetQueuedCompletionStatus(hCompletionPort, &dwThancferred, &dwCompleteKey, (LPOVERLAPPED *)&OverData, INFINITE);
		//if (bSuccess == FALSE || dwThancferred == 0)
		//{
		//	//ERROR_LOG("GetQueuedCompletionStatus failed err = %d", GetLastError());
		//	continue;
		//}

		usleep(THREAD_ONCE_HANDLE_MSG);

		while (pDataLine->GetDataCount())
		{
			try
			{
				unsigned int bytes = pDataLine->GetData(pDataLineHead, sizeof(szBuffer));
				if (bytes == 0)
				{
					// 取出来的数据大小为0，不太可能
					ERROR_LOG("GetDataCount data size = 0");
					continue;
				}

				// 置零末尾
				szBuffer[bytes] = 0;

				switch (pDataLineHead->uDataKind)
				{
				case HD_SOCKET_READ:		// socket 数据读取
				{
					SocketReadLine * pSocketRead = (SocketReadLine *)pDataLineHead;
					void* pBuffer = NULL;
					unsigned int size = pSocketRead->uHandleSize;
					if (size > 0)
					{
						pBuffer = (void *)(pSocketRead + 1);			// 移动一个SocketReadLine
					}

					//解析中心服务器包头
					CenterServerMessageHead * pCenterServerHead = (CenterServerMessageHead *)pBuffer;
					if (size < sizeof(CenterServerMessageHead))
					{
						ERROR_LOG("###### 包头大小错误 msgID=%d mainID=%d assistID=%d ######", pCenterServerHead->msgID,
							pSocketRead->netMessageHead.uMainID, pSocketRead->netMessageHead.uAssistantID);
						continue;
					}

					pBuffer = (void *)(pCenterServerHead + 1);
					size -= sizeof(CenterServerMessageHead);

					bool ret = pThis->OnSocketRead(&pSocketRead->netMessageHead, pCenterServerHead, pBuffer, size, pSocketRead->uAccessIP, pSocketRead->uIndex, pSocketRead->dwHandleID);
					if (!ret)
					{
						ERROR_LOG("OnSocketRead failed msgID=%d mainID=%d assistID=%d", pCenterServerHead->msgID,
							pSocketRead->netMessageHead.uMainID, pSocketRead->netMessageHead.uAssistantID);
						continue;
					}
					break;
				}
				case HD_SOCKET_CLOSE:		// socket 关闭
				{
					SocketCloseLine * pSocketClose = (SocketCloseLine *)pDataLineHead;
					pThis->OnSocketClose(pSocketClose->uAccessIP, pSocketClose->uIndex, pSocketClose->uConnectTime);
					break;
				}
				case HD_TIMER_MESSAGE:		// 定时器消息
				{
					ServerTimerLine* pTimerMessage = (ServerTimerLine*)pDataLineHead;
					pThis->OnTimerMessage(pTimerMessage->uTimerID);
					break;
				}
				default:
					ERROR_LOG("HD_PLATFORM_SOCKET_READ invalid linedata type, type=%d", pDataLineHead->uDataKind);
					break;
				}

			}

			catch (int iCode)
			{
				CON_ERROR_LOG("[ CenterServer 编号：%d ] [ 描述：如果有dump文件，请查看dump文件 ] [ 源代码位置：未知 ]", iCode);
				continue;
			}

			catch (...)
			{
				CON_ERROR_LOG("#### 未知崩溃。####");
				continue;
			}
		}
	}

	pthread_exit(NULL);
}