#include "pch.h"
#include "BaseMainManage.h"
#include "Function.h"
#include "Exception.h"
#include "commonuse.h"
#include "GameLogManage.h"
#include "InternalMessageDefine.h"
#include "log.h"
#include "PlatformMessage.h"
#include "ConfigManage.h"
#include "Util.h"

#pragma warning (disable: 4355)

//窗口线程启动结构
struct WindowThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	BOOL								bSuccess;					//启动成功标志
	CBaseMainManage						* pMainManage;				//数据管理指针
};

//处理线程启动结构
struct HandleThreadStartStruct
{
	//变量定义
	HANDLE								hEvent;						//启动事件
	HANDLE								hCompletionPort;			//完成端口
	CBaseMainManage						* pMainManage;				//数据管理指针
};

//构造函数
CBaseMainManage::CBaseMainManage()
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
	m_pGServerConnect = NULL;
}

CBaseMainManage::~CBaseMainManage()
{
	SAFE_DELETE(m_pRedis);
	SAFE_DELETE(m_pRedisPHP);
	SAFE_DELETE(m_pTcpConnect);
	SAFE_DELETE(m_pGServerConnect);
}

bool CBaseMainManage::Init(ManageInfoStruct * pInitData, IDataBaseHandleService * pDataHandleService)
{
	if (!pInitData || !pDataHandleService)
	{
		ERROR_LOG("invalid input params pInitData=%p pDataHandleService=%p", pInitData, pDataHandleService);
		return false;
	}

	if (m_bInit)
	{
		ERROR_LOG("already inited");
		return false;
	}

	m_InitData = *pInitData;

	bool ret = false;

	ret = PreInitParameter(&m_InitData, &m_KernelData);
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::Init PreInitParameter 参数调节错误"), 0x41A);
	}

	m_pRedis = new CRedisLoader;
	if (!m_pRedis)
	{
		return false;
	}

	ret = m_pRedis->Init();
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::Init redis初始化失败（可能是redis服务器未启动）"), 0x401);
	}

	//vip房间才需要连接php redis
	if (m_InitData.iRoomType != ROOM_TYPE_GOLD)
	{
		m_pRedisPHP = new CRedisPHP;
		if (!m_pRedisPHP)
		{
			return false;
		}

		ret = m_pRedisPHP->Init();
		if (!ret)
		{
			throw new CException(TEXT("CBaseMainManage::Init redisPHP初始化失败（可能是redis PHP服务器未启动）"), 0x402);
		}
	}

	ret = pDataHandleService->SetParameter(this, &m_SQLDataManage, &m_InitData, &m_KernelData);
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::Init pDataHandleService->SetParameter失败"), 0x41C);
	}

	ret = m_SQLDataManage.Init(&m_InitData, &m_KernelData, pDataHandleService, this);
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::Init m_SQLDataManage 初始化失败"), 0x41D);
	}

	m_pTcpConnect = new CTcpConnect;
	if (!m_pTcpConnect)
	{
		throw new CException(TEXT("CBaseMainManage::Init new CTcpConnect failed"), 0x43A);
	}

	m_pGServerConnect = new CGServerConnect;
	if (!m_pGServerConnect)
	{
		throw new CException(TEXT("CBaseMainManage::Init new CGServerConnect failed"), 0x43A);
	}

	ret = OnInit(&m_InitData, &m_KernelData);
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::Init OnInit 函数错误"), 0x41B);
	}

	m_bInit = true;

	return true;
}

//取消初始化函数 
bool CBaseMainManage::UnInit()
{
	//停止服务
	if (m_bRun)
	{
		Stop();
	}

	//调用接口
	OnUnInit();

	//取消初始化
	m_bInit = false;
	m_SQLDataManage.UnInit();

	//设置数据
	memset(&m_DllInfo, 0, sizeof(m_DllInfo));
	memset(&m_InitData, 0, sizeof(m_InitData));
	memset(&m_KernelData, 0, sizeof(m_KernelData));

	//释放redis
	m_pRedis->Stop();
	SAFE_DELETE(m_pRedis);
	if (m_pRedisPHP)
	{
		m_pRedisPHP->Stop();
		SAFE_DELETE(m_pRedisPHP);
	}

	return true;
}

//启动函数
bool CBaseMainManage::Start()
{
	if (!m_bInit || m_bRun)
	{
		return false;
	}

	m_bRun = true;

	//建立事件
	HANDLE StartEvent = CreateEvent(FALSE, TRUE, NULL, NULL);

	//建立完成端口
	m_hCompletePort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hCompletePort == NULL)
	{
		throw new CException(TEXT("CBaseMainManage::Start m_hCompletePort 建立失败"), 0x41D);
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
		throw new CException(TEXT("CBaseMainManage::Start LineDataHandleThread 线程启动失败"), 0x41E);
	}
	// 关联游戏业务逻辑线程与对应日志文件
	GameLogManage()->AddLogFile(uThreadID, THREAD_TYPE_LOGIC, m_InitData.uRoomID);

	WaitForSingleObject(StartEvent, INFINITE);

	//创建窗口为了生成定时器
	bool ret = CreateWindowsForTimer();
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::CreateWindowsForTimer 组件启动失败"), 0x41F);
	}

	ret = m_SQLDataManage.Start();
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::m_SQLDataManage.Start 数据库模块启动失败"), 0x420);
	}

	//////////////////////////////////建立与中心服的连接////////////////////////////////////////
	const CenterServerConfig & centerServerConfig = ConfigManage()->GetCenterServerConfig();
	ret = m_pTcpConnect->Start(&m_DataLine, centerServerConfig.ip, centerServerConfig.port, SERVICE_TYPE_LOADER, m_InitData.uRoomID);
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::m_pTcpConnect.Start 连接模块启动失败"), 0x433);
	}

	m_connectCServerHandle = (HANDLE)_beginthreadex(NULL, 0, TcpConnectThread, this, 0, NULL);
	if (!m_connectCServerHandle)
	{
		throw new CException(TEXT("CBaseMainManage::m_pTcpConnect.Start 连接线程函数启动失败"), 0x434);
	}

	//////////////////////////////////建立与登录服的连接////////////////////////////////////////
	ret = m_pGServerConnect->Start(&m_DataLine, m_InitData.uRoomID);
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::m_pGServerConnect.Start 连接模块启动失败"), 0x433);
	}

	//////////////////////////////////////////////////////////////////////////

	ret = OnStart();
	if (!ret)
	{
		throw new CException(TEXT("CBaseMainManage::Start OnStart 函数错误"), 0x422);
	}

	return true;
}

//停止服务
bool CBaseMainManage::Stop()
{
	if (m_bRun == false)
	{
		return true;
	}

	OnStop();

	m_bRun = false;

	m_DataLine.SetCompletionHandle(NULL);

	m_SQLDataManage.Stop();

	if (m_pTcpConnect)
	{
		m_pTcpConnect->Stop();
	}

	if (m_pGServerConnect)
	{
		m_pGServerConnect->Stop();
	}

	//关闭窗口
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		::SendMessage(m_hWindow, WM_CLOSE, 0, 0);
	}

	//关闭完成端口
	if (m_hCompletePort != NULL)
	{
		::PostQueuedCompletionStatus(m_hCompletePort, 0, NULL, NULL);
		::CloseHandle(m_hCompletePort);
		m_hCompletePort = NULL;
	}

	// 关闭中心服连接线程
	if ((m_connectCServerHandle != NULL) && (::WaitForSingleObject(m_connectCServerHandle, 50) == WAIT_TIMEOUT))
	{
		TerminateThread(m_connectCServerHandle, 0);
		CloseHandle(m_connectCServerHandle);
		m_connectCServerHandle = NULL;
	}

	//退出窗口线程
	if ((m_hWindowThread != NULL) && (::WaitForSingleObject(m_hWindowThread, 3000) == WAIT_TIMEOUT))
	{
		TerminateThread(m_hWindowThread, 0);
		CloseHandle(m_hWindowThread);
		m_hWindowThread = NULL;
	}

	//退出处理线程
	if ((m_hHandleThread != NULL) && (::WaitForSingleObject(m_hHandleThread, 3000) == WAIT_TIMEOUT))
	{
		TerminateThread(m_hHandleThread, 0);
		CloseHandle(m_hHandleThread);
		m_hHandleThread = NULL;
	}

	return true;
}

//刷新服务
bool CBaseMainManage::Update()
{
	return OnUpdate();
}

//网络关闭处理
bool CBaseMainManage::OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime)
{
	SocketCloseLine SocketClose;
	SocketClose.uConnectTime = uConnectTime;
	SocketClose.uIndex = uIndex;
	SocketClose.uAccessIP = uAccessIP;
	return (m_DataLine.AddData(&SocketClose.LineHead, sizeof(SocketClose), HD_SOCKET_CLOSE) != 0);
}

//网络消息处理
bool CBaseMainManage::OnSocketReadEvent(CTCPSocket* pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID)
{
	SocketReadLine SocketRead;
	SocketRead.uHandleSize = uSize;
	SocketRead.uIndex = uIndex;
	SocketRead.dwHandleID = dwHandleID;
	SocketRead.uAccessIP = 0;		//todo
	SocketRead.NetMessageHead = *pNetHead;
	return (m_DataLine.AddData(&SocketRead.LineHead, sizeof(SocketRead), HD_SOCKET_READ, pData, uSize) != 0);
}

//定时器通知消息
bool CBaseMainManage::WindowTimerMessage(UINT uTimerID)
{
	WindowTimerLine WindowTimer;
	WindowTimer.uTimerID = uTimerID;
	return (m_DataLine.AddData(&WindowTimer.LineHead, sizeof(WindowTimer), HD_TIMER_MESSAGE) != 0);
}

//异步线程结果处理
bool CBaseMainManage::OnAsynThreadResultEvent(UINT uHandleKind, UINT uHandleResult, void * pData, UINT uResultSize, UINT uDataType, UINT uHandleID)
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
bool CBaseMainManage::SetTimer(UINT uTimerID, UINT uElapse)
{
	if ((m_hWindow != NULL) && (IsWindow(m_hWindow) == TRUE))
	{
		::SetTimer(m_hWindow, uTimerID, uElapse, NULL);
		return true;
	}

	return false;
}

//清除定时器
bool CBaseMainManage::KillTimer(UINT uTimerID)
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE))
	{
		::KillTimer(m_hWindow, uTimerID);
		return true;
	}
	return false;
}

//创建窗口为了生成定时器
bool CBaseMainManage::CreateWindowsForTimer()
{
	if ((m_hWindow != NULL) && (::IsWindow(m_hWindow) == TRUE)) return false;
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
		throw new CException(TEXT("CBaseMainManage::CreateWindowsForTimer WindowMsgThread 线程建立失败"), 0x421);
	}

	::WaitForSingleObject(ThreadData.hEvent, INFINITE);
	if (ThreadData.bSuccess == FALSE)
	{
		throw new CException(TEXT("CBaseMainManage::CreateWindowsForTimer WindowMsgThread 线程建立失败"), 0x422);
	}

	return true;
}

//队列数据处理线程
unsigned __stdcall CBaseMainManage::LineDataHandleThread(LPVOID pThreadData)
{
	// 初始化随机数种子
	srand((unsigned)time(NULL));

	//数据定义
	HandleThreadStartStruct		* pData = (HandleThreadStartStruct *)pThreadData;		//线程启动数据指针
	CBaseMainManage				* pMainManage = pData->pMainManage;						//数据管理指针
	CDataLine					* m_pDataLine = &pMainManage->m_DataLine;				//数据队列指针
	HANDLE						hCompletionPort = pData->hCompletionPort;				//完成端口
	bool						& bRun = pMainManage->m_bRun;							//运行标志

	//线程数据读取完成
	::SetEvent(pData->hEvent);

	//重叠数据
	void						* pIOData = NULL;										//数据
	DWORD						dwThancferred = 0;										//接收数量
	ULONG						dwCompleteKey = 0L;										//重叠 IO 临时数据
	LPOVERLAPPED				OverData;												//重叠 IO 临时数据
	//数据缓存
	BOOL						bSuccess = FALSE;
	BYTE						szBuffer[LD_MAX_PART];
	DataLineHead				* pDataLineHead = (DataLineHead *)szBuffer;

	// 初始化异常函数
	CWin32Exception::SetWin32ExceptionFunc();

	while (pMainManage->m_bRun == true)
	{
		//等待完成端口
		bSuccess = ::GetQueuedCompletionStatus(hCompletionPort, &dwThancferred, &dwCompleteKey, (LPOVERLAPPED *)&OverData, INFINITE);
		if (bSuccess == FALSE || dwThancferred == 0)
		{
			continue;
		}

		while (m_pDataLine->GetDataCount())
		{
			try
			{
				unsigned int size = m_pDataLine->GetData(pDataLineHead, sizeof(szBuffer));
				if (size == 0)
				{
					continue;
				}

				switch (pDataLineHead->uDataKind)
				{
				case HD_SOCKET_READ://SOCKET数据读取
				{
					SocketReadLine * pSocketRead = (SocketReadLine *)pDataLineHead;
					if (pMainManage->OnSocketRead(&pSocketRead->NetMessageHead,
						pSocketRead->uHandleSize ? pSocketRead + 1 : NULL,
						pSocketRead->uHandleSize, pSocketRead->uAccessIP,
						pSocketRead->uIndex, pSocketRead->dwHandleID) == false)
					{
						//ERROR_LOG("OnSocketRead failed mainID=%d assistID=%d", pSocketRead->NetMessageHead.uMainID, pSocketRead->NetMessageHead.uAssistantID);
					}
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

					pMainManage->OnAsynThreadResult(pDataRead, pBuffer, size - sizeof(AsynThreadResultLine));

					break;
				}
				case HD_TIMER_MESSAGE://定时器消息
				{
					WindowTimerLine * pTimerMessage = (WindowTimerLine *)pDataLineHead;
					pMainManage->OnTimerMessage(pTimerMessage->uTimerID);
					break;
				}
				case HD_PLATFORM_SOCKET_READ:	// 中心服务器发过来的消息
				{
					PlatformDataLineHead* pPlaformMessageHead = (PlatformDataLineHead*)pDataLineHead;
					int sizeCenterMsg = pPlaformMessageHead->platformMessageHead.MainHead.uMessageSize - sizeof(PlatformMessageHead);
					UINT msgID = pPlaformMessageHead->platformMessageHead.AssHead.msgID;
					int userID = pPlaformMessageHead->platformMessageHead.AssHead.userID;
					void * pBuffer = NULL;
					if (sizeCenterMsg > 0)
					{
						pBuffer = (void*)(pPlaformMessageHead + 1);
					}

					pMainManage->OnCenterServerMessage(msgID, &pPlaformMessageHead->platformMessageHead.MainHead, pBuffer, sizeCenterMsg, userID);
					break;
				}
				default:
					break;
				}
			}

			catch (CWin32Exception& Win32Ex)
			{
				PEXCEPTION_POINTERS pEx = Win32Ex.ExceptionInformation();
				if (pEx != NULL && pEx->ExceptionRecord != NULL)
				{
					CWin32Exception::OutputWin32Exception("[ LoaderServer 编号：0x%x ] [ 描述：%s] [ 源代码位置：0x%x ]", pEx->ExceptionRecord->ExceptionCode,
						CWin32Exception::GetDescByCode(pEx->ExceptionRecord->ExceptionCode), ((pEx)->ExceptionRecord)->ExceptionAddress);
				}
				continue;
			}

			catch (int iCode)
			{
				CWin32Exception::OutputWin32Exception("[ LoaderServer 编号：%d ] [ 描述：如果有dump文件，请查看dump文件 ] [ 源代码位置：未知 ]", iCode);
				continue;
			}

			catch (...)
			{
				CWin32Exception::OutputWin32Exception("#### 未知崩溃。####");
				continue;
			}
		}
	}

	//INFO_LOG("THREAD::thread LineDataHandleThread exit!!!");

	return 0;
}

//WINDOW消息循环线程
unsigned __stdcall CBaseMainManage::WindowMsgThread(LPVOID pThreadData)
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
	while (GetMessage(&Message, NULL, 0, 0))
	{
		if (!TranslateAccelerator(Message.hwnd, NULL, &Message))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	//INFO_LOG("THREAD::thread WindowMsgThread exit!!!");

	return 0;
}

//窗口回调函数
LRESULT CALLBACK CBaseMainManage::WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:		//窗口建立消息
	{
		DWORD iIndex = TlsAlloc();
		CBaseMainManage * pMainManage = (CBaseMainManage *)(((CREATESTRUCT *)lParam)->lpCreateParams);
		TlsSetValue(iIndex, pMainManage);
		::SetWindowLong(hWnd, GWL_USERDATA, iIndex);
		break;
	}
	case WM_TIMER:		//定时器消息
	{
		DWORD iIndex = ::GetWindowLong(hWnd, GWL_USERDATA);
		CBaseMainManage * pMainManage = (CBaseMainManage *)::TlsGetValue(iIndex);
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
		CBaseMainManage * pMainManage = (CBaseMainManage *)::TlsGetValue(iIndex);
		if (pMainManage != NULL) pMainManage->m_hWindow = NULL;
		::TlsFree(iIndex);
		PostQuitMessage(0);
		break;
	}
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
// 中心服连接线程
unsigned CBaseMainManage::TcpConnectThread(LPVOID pThreadData)
{
	CBaseMainManage* pThis = (CBaseMainManage*)pThreadData;
	if (!pThis)
	{
		return -1;
	}

	CTcpConnect* pTcpConnect = (CTcpConnect*)pThis->m_pTcpConnect;
	if (!pTcpConnect)
	{
		return -2;
	}

	while (pThis->m_bRun && pThis->m_pTcpConnect)
	{
		pTcpConnect->EventLoop();
		pTcpConnect->CheckConnection();
	}

	return 0;
}