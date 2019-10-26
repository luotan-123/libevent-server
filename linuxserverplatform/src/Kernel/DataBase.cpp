#include "CommonHead.h"
#include "configManage.h"
#include "log.h"
#include "DataBase.h"


CDataBaseManage::CDataBaseManage()
{
	m_bInit = false;
	m_bRun = false;
	m_hThread = 0;
	//m_hCompletePort = NULL;
	m_pInitInfo = NULL;
	m_pKernelInfo = NULL;
	m_pHandleService = NULL;
	m_pMysqlHelper = NULL;

	m_sqlClass = 0;
	m_nPort = 0;
	m_bsqlInit = false;

	m_host[0] = '\0';
	m_user[0] = '\0';
	m_name[0] = '\0';
	m_passwd[0] = '\0';
}

CDataBaseManage::~CDataBaseManage()
{
	m_bInit = false;
	m_pInitInfo = NULL;
	m_hThread = 0;
	//m_hCompletePort = NULL;
	m_pKernelInfo = NULL;
	m_pHandleService = NULL;
}

//开始服务
bool CDataBaseManage::Start()
{
	INFO_LOG("DataBaseManage start begin ...");

	if (m_bRun == true || m_bInit == false)
	{
		ERROR_LOG("DataBaseManage already running or not inited m_bRun=%d m_bInit=%d", m_bRun, m_bInit);
		return false;
	}

	m_bRun = true;

	SQLConnectReset();
	//SQLConnect();

	//建立数据处理线程
	pthread_t threadID = 0;
	int err = pthread_create(&threadID, NULL, DataServiceThread, (void*)this);
	if (err != 0)
	{
		SYS_ERROR_LOG("DataServiceThread failed");
		return false;
	}

	m_hThread = threadID;

	// 关联日志文件
	int roomID = 0;
	if (m_pInitInfo)
	{
		roomID = m_pInitInfo->uRoomID;
	}
	GameLogManage()->AddLogFile(threadID, THREAD_TYPE_ASYNC, roomID);

	INFO_LOG("DataBaseManage start end.");

	return true;
}

bool CDataBaseManage::Stop()
{
	INFO_LOG("DataBaseManage stop begin...");

	if (!m_bRun)
	{
		ERROR_LOG("DataBaseManage is not running...");
		return false;
	}

	m_bRun = false;


	//// 先关闭完成端口
	//if (m_hCompletePort)
	//{
	//	PostQueuedCompletionStatus(m_hCompletePort, 0, NULL, NULL);
	//	CloseHandle(m_hCompletePort);
	//	m_hCompletePort = NULL;
	//}

	// 清理dataline
	//m_DataLine.SetCompletionHandle(NULL);
	m_DataLine.CleanLineData();

	// 关闭数据库处理线程句柄
	if (m_hThread)
	{
		pthread_cancel(m_hThread);
		m_hThread = 0;
	}

	//关闭数据库连接
	if (m_pMysqlHelper)
	{
		m_pMysqlHelper->disconnect();
		delete m_pMysqlHelper;
		m_pMysqlHelper = NULL;
	}

	INFO_LOG("DataBaseManage stop end.");

	return true;
}

//取消初始化
bool CDataBaseManage::UnInit()
{
	return true;
}

//加入处理队列
bool CDataBaseManage::PushLine(DataBaseLineHead* pData, UINT uSize, UINT uHandleKind, UINT uIndex, UINT dwHandleID)
{
	//处理数据
	pData->dwHandleID = dwHandleID;
	pData->uIndex = uIndex;
	pData->uHandleKind = uHandleKind;
	return m_DataLine.AddData(&pData->dataLineHead, uSize, 0) != 0;
}

//数据库处理线程
void* CDataBaseManage::DataServiceThread(void* pThreadData)
{
	//数据定义
	CDataBaseManage* pDataManage =(CDataBaseManage*)pThreadData;//数据库管理指针
	CDataLine* pDataLine = &pDataManage->m_DataLine;			//数据队列指针
	IDataBaseHandleService* pHandleService = pDataManage->m_pHandleService;	//数据处理接口

	//数据缓存
	DataLineHead* pDataLineHead = NULL;

	sleep(1);

	INFO_LOG("DataServiceThread starting...");

	//cpu优化，合理使用cpu
	long long llLastTime = GetSysMilliseconds();
	long long llNowTime = 0, llDifTime = 0;

	while (pDataManage->m_bRun == true)
	{
		llNowTime = GetSysMilliseconds();
		llDifTime = THREAD_ONCE_DATABASE + llLastTime - llNowTime;
		if (llDifTime > THREAD_ONCE_DATABASE)
		{
			usleep(THREAD_ONCE_DATABASE * 1000);
		}
		else if (llDifTime > 0)
		{
			usleep((unsigned int)(llDifTime * 1000));
		}
		llLastTime = llNowTime;

		while (pDataLine->GetDataCount())
		{
			try
			{
				//获取数据
				unsigned int bytes = pDataLine->GetData(&pDataLineHead);
				if (bytes == 0 || pDataLineHead == NULL)
				{
					// 取出来的数据大小为0，不太可能
					ERROR_LOG("bytes == 0 || pDataLineHead == NULL");
					continue;
				}

				//处理数据
				pHandleService->HandleDataBase((DataBaseLineHead*)pDataLineHead);

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
	}

	INFO_LOG("DataServiceThread exit.");

	pthread_exit(NULL);
}

//重联数据库
bool CDataBaseManage::SQLConnectReset()
{
	if (m_bsqlInit == false)
	{
		const DBConfig& dbConfig = ConfigManage()->GetDBConfig();

		m_nPort = dbConfig.port;

		strcpy(m_host, dbConfig.ip);
		strcpy(m_user, dbConfig.user);
		strcpy(m_name, dbConfig.dbName);
		strcpy(m_passwd, dbConfig.passwd);

		m_bsqlInit = true;
	}

	if (m_pMysqlHelper)
	{
		delete m_pMysqlHelper;
	}

	m_pMysqlHelper = new CMysqlHelper;

	//初始化mysql对象并建立连接
	m_pMysqlHelper->init(m_host, m_user, m_passwd, m_name, "", m_nPort);
	try
	{
		m_pMysqlHelper->connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return false;
	}

	return true;
}

bool CDataBaseManage::CheckSQLConnect()
{
	if (!m_pMysqlHelper)
	{
		return false;
	}

	char buf[128] = "";
	sprintf(buf, "select * from %s LIMIT 1", TBL_BASE_GAME);
	bool bConect = false;

	try
	{
		m_pMysqlHelper->sqlExec(buf);
	}
	catch (...)
	{
		bConect = true;
	}

	if (bConect)
	{
		try
		{
			m_pMysqlHelper->connect();
		}
		catch (MysqlHelper_Exception& excep)
		{
			ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
			return false;
		}
	}

	return true;
}

//***********************************************************************************************//
CDataBaseHandle::CDataBaseHandle()
{
	m_pInitInfo = NULL;
	m_pKernelInfo = NULL;
	m_pRusultService = NULL;
	m_pDataBaseManage = NULL;
}

CDataBaseHandle::~CDataBaseHandle()
{
}

bool CDataBaseHandle::SetParameter(IAsynThreadResultService* pRusultService, CDataBaseManage* pDataBaseManage, ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData)
{
	m_pInitInfo = pInitData;
	m_pKernelInfo = pKernelData;
	m_pRusultService = pRusultService;
	m_pDataBaseManage = pDataBaseManage;

	return true;
}

//初始化函数
bool CDataBaseManage::Init(ManageInfoStruct* pInitInfo, KernelInfoStruct* pKernelInfo, IDataBaseHandleService* pHandleService, IAsynThreadResultService* pResultService)
{
	if (!pInitInfo || !pKernelInfo || !pHandleService || !pResultService)
	{
		throw new CException("CDataBaseManage::Init 参数错误!", (UINT)0x407, true);
	}

	//效验参数
	if (m_bInit == true || m_bRun == true)
	{
		throw new CException("CDataBaseManage::Init 状态效验失败", (UINT)0x408, true);
	}

	//设置数据
	m_pInitInfo = pInitInfo;
	m_pKernelInfo = pKernelInfo;
	m_pHandleService = pHandleService;
	m_DataLine.CleanLineData();

	//设置数据
	m_bInit = true;

	return true;
}
