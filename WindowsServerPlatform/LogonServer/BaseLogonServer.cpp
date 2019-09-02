#include "main.h"
#include "BaseLogonServer.h"
#include "Exception.h"
#include "RedisLogon.h"
#include "log.h"
#include "PlatformMessage.h"
#include "InternalMessageDefine.h"
#include "Function.h"
#include "Util.h"

#pragma warning (disable: 4355)

//窗口线程启动结构
struct WindowThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	BOOL								bSuccess;					//启动成功标志
	CBaseLogonServer				* pMainManage;				//数据管理指针
};

//处理线程启动结构
struct HandleThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	HANDLE								hCompletionPort;			//完成端口
	CBaseLogonServer				* pMainManage;				//数据管理指针
};

/*****************************************************************************************************************/
CBaseLogonServer::CBaseLogonServer()
{
	m_bInit = false;
	m_bRun = false;
	m_hWindow = NULL;
	m_hHandleThread = NULL;
	m_hWindowThread = NULL;
	m_hCompletePort = NULL;
	m_connectCServerHandle = NULL;
	::memset(&m_DllInfo, 0, sizeof(m_DllInfo));
	::memset(&m_InitData, 0, sizeof(m_InitData));
	::memset(&m_KernelData, 0, sizeof(m_KernelData));

	m_pRedis = NULL;
	m_pRedisPHP = NULL;
	m_pTcpConnect = NULL;
}

CBaseLogonServer::~CBaseLogonServer()
{
	SAFE_DELETE(m_pRedis);
	SAFE_DELETE(m_pRedisPHP);
	SAFE_DELETE(m_pTcpConnect);
}

//初始化函数 
bool CBaseLogonServer::Init(ManageInfoStruct * pInitData, IDataBaseHandleService * pDataHandleService)
{
	INFO_LOG("BaseMainManageForZ Init begin...");

	if (m_bInit == true)
	{
		ERROR_LOG("is already have been inited...");
		return false;
	}

	if (!pInitData || !pDataHandleService)
	{
		ERROR_LOG("invalid params input pInitData=%p pDataHandleService=%p", pInitData, pDataHandleService);
		return false;
	}

	if (m_bInit == true)
	{
		ERROR_LOG("is already been inited");
		return false;
	}

	srand((unsigned)time(NULL));

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
	m_pRedis = new CRedisLogon;
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

	// redis 相关
	m_pRedisPHP = new CRedisPHP;
	if (!m_pRedisPHP)
	{
		ERROR_LOG("create m_pRedisPHP object failed");
		return false;
	}

	ret = m_pRedisPHP->Init();
	if (!ret)
	{
		ERROR_LOG("连接PHP redis失败");
		return false;
	}

	// 初始化网络
	ret = m_TCPSocket.Init(this, m_InitData.uMaxPeople, m_InitData.uListenPort, m_InitData.iSocketSecretKey);
	if (!ret)
	{
		ERROR_LOG("TCPSocket Init failed");
		return false;
	}

	// DB相关
	ret = pDataHandleService->SetParameter(this, &m_SQLDataManage, &m_InitData, &m_KernelData);
	if (!ret)
	{
		ERROR_LOG("DataHandleService SetParameter failed");
		return false;
	}

	ret = m_SQLDataManage.Init(&m_InitData, &m_KernelData, pDataHandleService, this);
	if (!ret)
	{
		ERROR_LOG("SQLDataManage Init failed");
		return false;
	}

	// 中心服务器连接
	m_pTcpConnect = new CTcpConnect;
	if (!m_pTcpConnect)
	{
		throw new CException(TEXT("BaseMainManageForZ::Init new CTcpConnect failed"), 0x43A);
	}

	ret = OnInit(&m_InitData, &m_KernelData);
	if (!ret)
	{
		ERROR_LOG("OnInit failed");
		return false;
	}

	m_bInit = true;

	INFO_LOG("BaseMainManageForZ Init end");

	return true;
}

//取消初始化函数 
bool CBaseLogonServer::UnInit()
{
	////停止服务
	//if (m_bRun)
	//{
	//	Stop();
	//}

	////调用接口
	//OnUnInit();

	//m_bInit = false;
	//m_TCPSocket.UnInit();
	//m_SQLDataManage.UnInit();

	////设置数据
	//memset(&m_DllInfo, 0, sizeof(m_DllInfo));
	//memset(&m_InitData, 0, sizeof(m_InitData));
	//memset(&m_KernelData, 0, sizeof(m_KernelData));

	return true;
}

//启动函数
bool CBaseLogonServer::Start()
{
	INFO_LOG("BaseMainManageForZ Start begin...");

	if (m_bInit == false || m_bRun == true)
	{
		ERROR_LOG("BaseMainManageForZ already been inited or running");
		return false;
	}

	m_bRun = true;

	//建立事件
	HANDLE StartEvent = CreateEvent(FALSE, TRUE, NULL, NULL);

	//建立完成端口
	m_hCompletePort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (!m_hCompletePort)
	{
		ERROR_LOG("CreateIoCompletionPort failed err=%d", GetLastError());
		return false;
	}

	m_DataLine.SetCompletionHandle(m_hCompletePort);

	//启动处理线程
	UINT uThreadID = 0;

	HandleThreadStartStruct	ThreadStartData;

	ThreadStartData.pMainManage = this;
	ThreadStartData.hCompletionPort = m_hCompletePort;
	ThreadStartData.hEvent = StartEvent;

	m_hHandleThread = (HANDLE)_beginthreadex(NULL, 0, LineDataHandleThread, &ThreadStartData, 0, &uThreadID);
	if (!m_hHandleThread)
	{
		ERROR_LOG("begin LineDataHandleThread failed");
		return false;
	}

	// 关联大厅业务逻辑线程与对应日志文件
	GameLogManage()->AddLogFile(uThreadID, THREAD_TYPE_LOGIC);

	WaitForSingleObject(StartEvent, INFINITE);

	bool ret = false;

	//启动窗口模块
	ret = CreateWindowsForTimer();
	if (!ret)
	{
		ERROR_LOG("CreateWindowsForTimer failed");
		return false;
	}

	// 启动DB模块
	ret = m_SQLDataManage.Start();
	if (!ret)
	{
		ERROR_LOG("SQLDataManage start failed");
		return false;
	}

	// 启动网络模块
	ret = m_TCPSocket.Start(SERVICE_TYPE_LOGON);
	if (!ret)
	{
		ERROR_LOG("TCPSocket Start failed");
		return false;
	}

	const CenterServerConfig & centerServerConfig = ConfigManage()->GetCenterServerConfig();
	ret = m_pTcpConnect->Start(&m_DataLine, centerServerConfig.ip, centerServerConfig.port, SERVICE_TYPE_LOGON, ConfigManage()->GetLogonServerConfig().logonID);		// TODO	
	if (!ret)
	{
		throw new CException(TEXT("BaseMainManageForZ::m_TCPSocket.Start 连接模块启动失败"), 0x433);
	}

	m_connectCServerHandle = (HANDLE)_beginthreadex(NULL, 0, TcpConnectThread, this, 0, NULL);
	if (!m_connectCServerHandle)
	{
		throw new CException(TEXT("BaseMainManageForZ::m_TCPSocket.Start 连接线程函数启动失败"), 0x434);
	}

	//调用接口
	ret = OnStart();
	if (!ret)
	{
		ERROR_LOG("OnStart failed");
		return false;
	}

	INFO_LOG("BaseMainManageForZ Start end.");

	return true;
}

//停止服务
bool CBaseLogonServer::Stop()
{
	INFO_LOG("BaseMainManageForZ Stop begin...");

	if (m_bRun == false)
	{
		ERROR_LOG("is not running...");
		return false;
	}

	m_bRun = false;

	m_DataLine.SetCompletionHandle(NULL);

	// 先关闭网络模块
	m_TCPSocket.Stop();

	//关闭与中心服务器的连接
	m_pTcpConnect->Stop();

	//关闭完成端口
	if (m_hCompletePort)
	{
		PostQueuedCompletionStatus(m_hCompletePort, 0, NULL, NULL);
		CloseHandle(m_hCompletePort);
		m_hCompletePort = NULL;
	}

	// 关闭linedate线程句柄
	if (m_hHandleThread)
	{
		WaitForSingleObject(m_hHandleThread, INFINITE);
		CloseHandle(m_hHandleThread);
		m_hHandleThread = NULL;
	}

	// 关闭中心服连接线程
	if (m_connectCServerHandle)
	{
		//WaitForSingleObject(m_connectCServerHandle, INFINITE);
		CloseHandle(m_connectCServerHandle);
		m_connectCServerHandle = NULL;
	}

	// 上层接口
	OnStop();		// 主线程关闭并清理资源(这里需要依赖redis和db)

	// 关闭DB模块
	m_SQLDataManage.Stop();

	// 关闭redis
	m_pRedis->Stop();
	m_pRedisPHP->Stop();

	//关闭窗口
	if (m_hWindow != NULL && IsWindow(m_hWindow) == TRUE)
	{
		SendMessage(m_hWindow, WM_CLOSE, 0, 0);
	}

	// 等待窗口线程
	if (m_hWindowThread)
	{
		WaitForSingleObject(m_hWindowThread, INFINITE);
		CloseHandle(m_hWindowThread);
		m_hWindowThread = NULL;
	}

	INFO_LOG("BaseMainManageForZ Stop end.");

	return true;
}

//网络关闭处理
bool CBaseLogonServer::OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime)
{
	SocketCloseLine SocketClose;
	SocketClose.uConnectTime = uConnectTime;
	SocketClose.uIndex = uIndex;
	SocketClose.uAccessIP = uAccessIP;
	return (m_DataLine.AddData(&SocketClose.LineHead, sizeof(SocketClose), HD_SOCKET_CLOSE) != 0);
}

//网络消息处理
bool CBaseLogonServer::OnSocketReadEvent(CTCPSocket * pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID)
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
	SocketRead.NetMessageHead = *pNetHead;
	return m_DataLine.AddData(&SocketRead.LineHead, sizeof(SocketRead), HD_SOCKET_READ, pData, uSize) != 0;
}

//定时器通知消息
bool CBaseLogonServer::WindowTimerMessage(UINT uTimerID)
{
	WindowTimerLine WindowTimer;
	WindowTimer.uTimerID = uTimerID;
	return (m_DataLine.AddData(&WindowTimer.LineHead, sizeof(WindowTimer), HD_TIMER_MESSAGE) != 0);
}

//异步线程结果处理
bool CBaseLogonServer::OnAsynThreadResultEvent(UINT uHandleKind, UINT uHandleResult, void * pData, UINT uResultSize, UINT uDataType, UINT uHandleID)
{
	AsynThreadResultLine resultData;

	//包装数据
	resultData.LineHead.uSize = uResultSize;
	resultData.LineHead.uDataKind = uDataType;

	resultData.uHandleKind = uHandleKind;
	resultData.uHandleResult = uHandleResult;
	resultData.uHandleID = uHandleID;

	//加入队列
	return (m_DataLine.AddData(&resultData.LineHead, sizeof(resultData), HD_ASYN_THREAD_RESULT, pData, uResultSize) != 0);
}

//设定定时器
bool CBaseLogonServer::SetTimer(UINT uTimerID, UINT uElapse)
{
	if ((m_hWindow != NULL) && (IsWindow(m_hWindow) == TRUE))
	{
		::SetTimer(m_hWindow, uTimerID, uElapse, NULL);
		return true;
	}
	return false;
}

//清除定时器
bool CBaseLogonServer::KillTimer(UINT uTimerID)
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		::KillTimer(m_hWindow, uTimerID);
		return true;
	}
	return false;
}

//创建窗口为了生成定时器
bool CBaseLogonServer::CreateWindowsForTimer()
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		return false;
	}

	//建立事件
	HANDLE StartEvent = CreateEvent(FALSE, TRUE, NULL, NULL);

	//建立线程
	WindowThreadStartStruct ThreadData;

	ThreadData.bSuccess = FALSE;
	ThreadData.pMainManage = this;
	ThreadData.hEvent = StartEvent;

	UINT uThreadID = 0;
	m_hWindowThread = (HANDLE)::_beginthreadex(NULL, 0, WindowMsgThread, &ThreadData, 0, &uThreadID);
	if (m_hWindowThread == NULL)
	{
		ERROR_LOG("_beginthreadex WindowMsgThread failed");
		return false;
	}
	WaitForSingleObject(ThreadData.hEvent, INFINITE);

	if (ThreadData.bSuccess == FALSE)
	{
		ERROR_LOG("ThreadData.bSuccess == FALSE");
		return false;
	}

	return true;
}

//队列数据处理线程
unsigned __stdcall CBaseLogonServer::LineDataHandleThread(LPVOID pThreadData)
{
	// 给主线程一点时间
	Sleep(1);

	INFO_LOG("LineDataHandleThread start...");

	HandleThreadStartStruct* pData = (HandleThreadStartStruct *)pThreadData;
	if (!pData)
	{
		return -1;
	}

	CBaseLogonServer	* pThis = pData->pMainManage;
	if (!pThis)
	{
		return -2;
	}

	CDataLine* pDataLine = &pThis->m_DataLine;
	if (!pDataLine)
	{
		return -3;
	}

	//完成端口
	HANDLE hCompletionPort = pData->hCompletionPort;
	if (!hCompletionPort)
	{
		return -4;
	}

	//运行标志
	bool& bRun = pThis->m_bRun;

	::SetEvent(pData->hEvent);

	// 初始化随机数种子
	srand((unsigned)time(NULL));

	// 初始化异常函数
	CWin32Exception::SetWin32ExceptionFunc();

	//重叠数据
	DWORD dwThancferred = 0;
	ULONG dwCompleteKey = 0;
	LPOVERLAPPED OverData;

	//数据缓存
	BYTE szBuffer[LD_MAX_PART] = "";
	DataLineHead* pDataLineHead = (DataLineHead *)szBuffer;

	while (pThis->m_bRun)
	{
		BOOL bSuccess = ::GetQueuedCompletionStatus(hCompletionPort, &dwThancferred, &dwCompleteKey, (LPOVERLAPPED *)&OverData, INFINITE);
		if (bSuccess == FALSE || dwThancferred == 0)
		{
			//ERROR_LOG("GetQueuedCompletionStatus failed err = %d", GetLastError());
			continue;
		}

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

					if (!pThis->OnSocketRead(&pSocketRead->NetMessageHead, pBuffer, size, pSocketRead->uAccessIP, pSocketRead->uIndex, pSocketRead->dwHandleID))
					{
						ERROR_LOG("OnSocketRead failed mainID=%d assistID=%d", pSocketRead->NetMessageHead.uMainID, pSocketRead->NetMessageHead.uAssistantID);
						pThis->m_TCPSocket.OnSocketClose(pSocketRead->uIndex);
					}
					break;
				}
				case HD_SOCKET_CLOSE:		// socket 关闭
				{
					SocketCloseLine * pSocketClose = (SocketCloseLine *)pDataLineHead;
					pThis->OnSocketClose(pSocketClose->uAccessIP, pSocketClose->uIndex, pSocketClose->uConnectTime);
					break;
				}
				case HD_ASYN_THREAD_RESULT://异步线程处理结果
				{
					AsynThreadResultLine * pDataRead = (AsynThreadResultLine *)pDataLineHead;
					void* pBuffer = NULL;
					unsigned int size = pDataRead->LineHead.uSize;

					if (size < sizeof(AsynThreadResultLine))
					{
						ERROR_LOG("AsynThreadResultLine data error !!!");
						break;
					}

					if (size > sizeof(AsynThreadResultLine))
					{
						pBuffer = (void *)(pDataRead + 1);			// 移动一个SocketReadLine
					}

					pThis->OnAsynThreadResult(pDataRead, pBuffer, size - sizeof(AsynThreadResultLine));

					break;
				}
				case HD_TIMER_MESSAGE:		// 定时器消息
				{
					WindowTimerLine * pTimerMessage = (WindowTimerLine *)pDataLineHead;
					pThis->OnTimerMessage(pTimerMessage->uTimerID);
					break;
				}
				case HD_PLATFORM_SOCKET_READ:	// 中心服消息
				{
					PlatformDataLineHead* pPlaformMessageHead = (PlatformDataLineHead*)pDataLineHead;
					int size = pPlaformMessageHead->platformMessageHead.MainHead.uMessageSize - sizeof(PlatformMessageHead);
					UINT msgID = pPlaformMessageHead->platformMessageHead.AssHead.msgID;
					int userID = pPlaformMessageHead->platformMessageHead.AssHead.userID;
					void * pBuffer = NULL;
					if (size > 0)
					{
						pBuffer = (void*)(pPlaformMessageHead + 1);
					}

					if (!pThis->OnCenterServerMessage(msgID, &pPlaformMessageHead->platformMessageHead.MainHead, pBuffer, size, userID))
					{
						ERROR_LOG("处理中心服务器出错,msgID=%d", msgID);
					}

					break;
				}
				default:
					ERROR_LOG("HD_PLATFORM_SOCKET_READ invalid linedata type, type=%d", pDataLineHead->uDataKind);
					break;
				}

			}

			catch (CWin32Exception& Win32Ex)
			{
				PEXCEPTION_POINTERS pEx = Win32Ex.ExceptionInformation();
				if (pEx != NULL && pEx->ExceptionRecord != NULL)
				{
					CWin32Exception::OutputWin32Exception("[ LogonServer 编号：0x%x ] [ 描述：%s] [ 源代码位置：0x%x ]", pEx->ExceptionRecord->ExceptionCode,
						CWin32Exception::GetDescByCode(pEx->ExceptionRecord->ExceptionCode), ((pEx)->ExceptionRecord)->ExceptionAddress);
				}
				continue;
			}

			catch (int iCode)
			{
				CWin32Exception::OutputWin32Exception("[ LogonServer 编号：%d ] [ 描述：如果有dump文件，请查看dump文件 ] [ 源代码位置：未知 ]", iCode);
				continue;
			}

			catch (...)
			{
				CWin32Exception::OutputWin32Exception("#### 未知崩溃。####");
				continue;
			}
		}
	}

	INFO_LOG("LineDataHandleThread exit。");

	return 0;
}

//WINDOW 消息循环线程
unsigned __stdcall CBaseLogonServer::WindowMsgThread(LPVOID pThreadData)
{
	WindowThreadStartStruct * pStartData = (WindowThreadStartStruct *)pThreadData;

	try
	{
		//注册窗口类
		LOGBRUSH		LogBrush;
		WNDCLASS		WndClass;
		TCHAR			szClassName[] = TEXT("CMainManageWindow");

		LogBrush.lbColor = RGB(0, 0, 0);
		LogBrush.lbStyle = BS_SOLID;
		LogBrush.lbHatch = 0;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hCursor = NULL;
		WndClass.hIcon = NULL;
		WndClass.lpszMenuName = NULL;
		WndClass.lpfnWndProc = WindowProcFunc;
		WndClass.lpszClassName = szClassName;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;
		WndClass.hInstance = NULL;
		WndClass.hbrBackground = (HBRUSH)::CreateBrushIndirect(&LogBrush);
		::RegisterClass(&WndClass);

		//建立窗口
		pStartData->pMainManage->m_hWindow = ::CreateWindow(szClassName, NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, pStartData->pMainManage);
		if (pStartData->pMainManage->m_hWindow == NULL) throw TEXT("窗口建立失败");
	}
	catch (...)
	{
		ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
		//启动错误
		pStartData->bSuccess = FALSE;
		::SetEvent(pStartData->hEvent);
		_endthreadex(0);
	}

	//启动成功
	pStartData->bSuccess = TRUE;
	::SetEvent(pStartData->hEvent);

	//消息循环
	MSG	Message;
	while (::GetMessage(&Message, NULL, 0, 0))
	{
		if (!::TranslateAccelerator(Message.hwnd, NULL, &Message))
		{
			::TranslateMessage(&Message);
			::DispatchMessage(&Message);
		}
	}

	return 0;
}

//窗口回调函数
LRESULT CALLBACK CBaseLogonServer::WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:		//窗口建立消息
	{
		DWORD iIndex = TlsAlloc();
		CBaseLogonServer * pMainManage = (CBaseLogonServer *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		TlsSetValue(iIndex, pMainManage);
		::SetWindowLong(hWnd, GWL_USERDATA, iIndex);
		break;
	}
	case WM_TIMER:		//定时器消息
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		CBaseLogonServer * pMainManage = (CBaseLogonServer *)::TlsGetValue(iIndex);
		if ((pMainManage != NULL) && (pMainManage->WindowTimerMessage((UINT)wParam) == false)) ::KillTimer(hWnd, (UINT)wParam);
		break;
	}
	case WM_CLOSE:		//窗口关闭消息
	{
		DestroyWindow(hWnd);
		break;
	}
	case WM_DESTROY:	//窗口关闭消息
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		CBaseLogonServer * pMainManage = (CBaseLogonServer *)::TlsGetValue(iIndex);
		if (pMainManage != NULL) pMainManage->m_hWindow = NULL;
		::TlsFree(iIndex);
		PostQuitMessage(0);
		break;
	}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

unsigned CBaseLogonServer::TcpConnectThread(LPVOID pThreadData)
{
	CBaseLogonServer* pThis = (CBaseLogonServer*)pThreadData;
	if (!pThis)
	{
		return -1;
	}

	CTcpConnect* pTcpConnect = (CTcpConnect*)pThis->m_pTcpConnect;
	if (!pTcpConnect)
	{
		return -2;
	}

	while (pThis->m_bRun && pTcpConnect)
	{
		pTcpConnect->EventLoop();
		pTcpConnect->CheckConnection();
	}

	return 0;
}