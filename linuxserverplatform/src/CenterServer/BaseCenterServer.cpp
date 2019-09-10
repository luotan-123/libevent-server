#include "main.h"
#include "BaseCenterServer.h"
#include "RedisLogon.h"
#include "log.h"
#include "InternalMessageDefine.h"
#include "Function.h"
#include "Util.h"
#include "PlatformMessage.h"

#pragma warning (disable: 4355)

//窗口线程启动结构
struct WindowThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	BOOL								bSuccess;					//启动成功标志
	CBaseCenterServer					* pMainManage;				//数据管理指针
};

//处理线程启动结构
struct HandleThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	HANDLE								hCompletionPort;			//完成端口
	CBaseCenterServer					* pMainManage;				//数据管理指针
};

/*****************************************************************************************************************/
CBaseCenterServer::CBaseCenterServer()
{
	m_bInit = false;
	m_bRun = false;
	m_hWindow = NULL;
	m_hHandleThread = NULL;
	m_hWindowThread = NULL;
	m_hCompletePort = NULL;
	::memset(&m_DllInfo, 0, sizeof(m_DllInfo));
	::memset(&m_InitData, 0, sizeof(m_InitData));
	::memset(&m_KernelData, 0, sizeof(m_KernelData));

	m_pRedis = NULL;
	m_pRedisPHP = NULL;
}

CBaseCenterServer::~CBaseCenterServer()
{
	SAFE_DELETE(m_pRedis);
	SAFE_DELETE(m_pRedisPHP);
}

//初始化函数 
bool CBaseCenterServer::Init(ManageInfoStruct * pInitData)
{
	INFO_LOG("BaseMainManageForC Init begin...");

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
	ret = m_TCPSocket.Init(this, m_InitData.uMaxPeople, m_InitData.uListenPort, m_InitData.iSocketSecretKey, m_InitData.szCenterIP);
	if (!ret)
	{
		ERROR_LOG("TCPSocket Init failed");
		return false;
	}

	ret = OnInit(&m_InitData, &m_KernelData);
	if (!ret)
	{
		ERROR_LOG("OnInit failed");
		return false;
	}

	m_bInit = true;

	INFO_LOG("BaseMainManageForC Init end");

	return true;
}

//取消初始化函数 
bool CBaseCenterServer::UnInit()
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
bool CBaseCenterServer::Start()
{
	INFO_LOG("BaseMainManageForC Start begin...");

	if (m_bInit == false || m_bRun == true)
	{
		ERROR_LOG("BaseMainManageForC already been inited or running");
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

	// 启动网络模块
	ret = m_TCPSocket.Start(SERVICE_TYPE_CENTER);
	if (!ret)
	{
		ERROR_LOG("TCPSocket Start failed");
		return false;
	}

	//调用接口
	ret = OnStart();
	if (!ret)
	{
		ERROR_LOG("OnStart failed");
		return false;
	}

	INFO_LOG("BaseMainManageForC Start end.");

	return true;
}

//停止服务
bool CBaseCenterServer::Stop()
{
	INFO_LOG("BaseMainManageForC Stop begin...");

	if (m_bRun == false)
	{
		ERROR_LOG("is not running...");
		return false;
	}

	m_bRun = false;

	m_DataLine.SetCompletionHandle(NULL);

	// 先关闭网络模块
	m_TCPSocket.Stop();

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

	// 上层接口
	OnStop();		// 主线程关闭并清理资源(这里需要依赖redis和db)

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

	INFO_LOG("BaseMainManageForC Stop end.");

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
bool CBaseCenterServer::OnSocketReadEvent(CTCPSocket * pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID)
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
bool CBaseCenterServer::WindowTimerMessage(UINT uTimerID)
{
	WindowTimerLine WindowTimer;
	WindowTimer.uTimerID = uTimerID;
	return (m_DataLine.AddData(&WindowTimer.LineHead, sizeof(WindowTimer), HD_TIMER_MESSAGE) != 0);
}

//设定定时器
bool CBaseCenterServer::SetTimer(UINT uTimerID, UINT uElapse)
{
	if ((m_hWindow != NULL) && (IsWindow(m_hWindow) == TRUE))
	{
		::SetTimer(m_hWindow, uTimerID, uElapse, NULL);
		return true;
	}
	return false;
}

//清除定时器
bool CBaseCenterServer::KillTimer(UINT uTimerID)
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		::KillTimer(m_hWindow, uTimerID);
		return true;
	}
	return false;
}

//创建窗口为了生成定时器
bool CBaseCenterServer::CreateWindowsForTimer()
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
unsigned __stdcall CBaseCenterServer::LineDataHandleThread(LPVOID pThreadData)
{
	// 给主线程一点时间
	Sleep(1);

	INFO_LOG("LineDataHandleThread start...");

	HandleThreadStartStruct* pData = (HandleThreadStartStruct *)pThreadData;
	if (!pData)
	{
		return -1;
	}

	CBaseCenterServer	* pThis = pData->pMainManage;
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

					//解析中心服务器包头
					CenterServerMessageHead * pCenterServerHead = (CenterServerMessageHead *)pBuffer;
					if (size < sizeof(CenterServerMessageHead))
					{
						ERROR_LOG("###### 包头大小错误 msgID=%d mainID=%d assistID=%d ######", pCenterServerHead->msgID,
							pSocketRead->NetMessageHead.uMainID, pSocketRead->NetMessageHead.uAssistantID);
						continue;
					}

					pBuffer = (void *)(pCenterServerHead + 1);
					size -= sizeof(CenterServerMessageHead);

					bool ret = pThis->OnSocketRead(&pSocketRead->NetMessageHead, pCenterServerHead, pBuffer, size, pSocketRead->uAccessIP, pSocketRead->uIndex, pSocketRead->dwHandleID);
					if (!ret)
					{
						ERROR_LOG("OnSocketRead failed msgID=%d mainID=%d assistID=%d", pCenterServerHead->msgID,
							pSocketRead->NetMessageHead.uMainID, pSocketRead->NetMessageHead.uAssistantID);
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
					WindowTimerLine * pTimerMessage = (WindowTimerLine *)pDataLineHead;
					pThis->OnTimerMessage(pTimerMessage->uTimerID);
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
					CWin32Exception::OutputWin32Exception("[ CenterServer 编号：0x%x ] [ 描述：%s] [ 源代码位置：0x%x ]", pEx->ExceptionRecord->ExceptionCode,
						CWin32Exception::GetDescByCode(pEx->ExceptionRecord->ExceptionCode), ((pEx)->ExceptionRecord)->ExceptionAddress);
				}
				continue;
			}

			catch (int iCode)
			{
				CWin32Exception::OutputWin32Exception("[ CenterServer 编号：%d ] [ 描述：如果有dump文件，请查看dump文件 ] [ 源代码位置：未知 ]", iCode);
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
unsigned __stdcall CBaseCenterServer::WindowMsgThread(LPVOID pThreadData)
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
LRESULT CALLBACK CBaseCenterServer::WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:		//窗口建立消息
	{
		DWORD iIndex = TlsAlloc();
		CBaseCenterServer * pMainManage = (CBaseCenterServer *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		TlsSetValue(iIndex, pMainManage);
		::SetWindowLong(hWnd, GWL_USERDATA, iIndex);
		break;
	}
	case WM_TIMER:		//定时器消息
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		CBaseCenterServer * pMainManage = (CBaseCenterServer *)::TlsGetValue(iIndex);
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
		CBaseCenterServer * pMainManage = (CBaseCenterServer *)::TlsGetValue(iIndex);
		if (pMainManage != NULL) pMainManage->m_hWindow = NULL;
		::TlsFree(iIndex);
		PostQuitMessage(0);
		break;
	}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
