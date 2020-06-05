#include "CommonHead.h"
#include "GameLogManage.h"
#include "configManage.h"
#include <sys/stat.h>

static unsigned long long int g_fileErrLogCount = 0;
static unsigned long long int g_fileCostLogCount = 0;

CGameLogManage::CGameLogManage()
{
	m_logPath = "";
}

CGameLogManage::~CGameLogManage()
{
	//Release();
}

CGameLogManage* CGameLogManage::Instance()
{
	static CGameLogManage g_mgr;
	return &g_mgr;
}

void CGameLogManage::Release()
{
	m_LogFilesMap.clear();

	for (auto iter = m_filesFpMap.begin(); iter != m_filesFpMap.end(); ++iter)
	{
		FILE* fp = iter->second;
		if (fp)
		{
			fclose(fp);
			fp = NULL;
		}
	}

	m_filesFpMap.clear();
}

void CGameLogManage::AddLogFile(pthread_t threadID, int threadType, int roomID/* = 0*/)
{
	CSignedLockObject LockObject(&m_csLock, true);

	int serviceType = ConfigManage()->m_serviceType;
	if (serviceType <= SERVICE_TYPE_BEGIN || serviceType >= SERVICE_TYPE_END)
	{
		return;
	}

	LogFileElem elem;
	elem.threadID = threadID;
	elem.serverType = serviceType;
	std::string servername = ConfigManage()->GetServerNameByType(serviceType);

	if (serviceType == SERVICE_TYPE_LOADER)
	{
		if (threadID <= 0)
		{
			return;
		}

		if (threadType == THREAD_TYPE_LOGIC && roomID <= 0)
		{
			return;
		}

		std::string errfileName = m_logPath;
		std::string costFileName = m_logPath;

		if (threadType == THREAD_TYPE_MAIN)		// 主线程
		{
			errfileName += servername + "_main";
			costFileName += servername + "_main_cost";
		}
		else if (threadType == THREAD_TYPE_LOGIC)	// 逻辑线程(每个游戏都是单独的)
		{
			RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
			if (!pRoomBaseInfo)
			{
				return;
			}

			errfileName += pRoomBaseInfo->name;
			errfileName += "_logic";

			costFileName += pRoomBaseInfo->name;
			costFileName += "_logic_cost";
		}
		else if (threadType == THREAD_TYPE_ASYNC)
		{
			RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
			if (!pRoomBaseInfo)
			{
				return;
			}

			errfileName += pRoomBaseInfo->name;
			errfileName += "_async";

			costFileName += pRoomBaseInfo->name;
			costFileName += "_async_cost";
		}
		else if (threadType == THREAD_TYPE_ACCEPT)
		{
			RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
			if (!pRoomBaseInfo)
			{
				return;
			}

			errfileName += pRoomBaseInfo->name;
			errfileName += "_accept";

			costFileName += pRoomBaseInfo->name;
			costFileName += "_accept_cost";
		}
		else if (threadType == THREAD_TYPE_RECV)
		{
			RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
			if (!pRoomBaseInfo)
			{
				return;
			}

			errfileName += pRoomBaseInfo->name;
			errfileName += "_recv";

			costFileName += pRoomBaseInfo->name;
			costFileName += "_recv_cost";
		}
		else if (threadType == THREAD_TYPE_SEND)
		{
			RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
			if (!pRoomBaseInfo)
			{
				return;
			}

			errfileName += pRoomBaseInfo->name;
			errfileName += "_send";

			costFileName += pRoomBaseInfo->name;
			costFileName += "_send_cost";
		}
		else
		{
			return;
		}

		ThreadLogFiles logs;
		logs.errorLog = errfileName;
		logs.costLog = costFileName;

		m_LogFilesMap.emplace(elem, logs);
	}
	else
	{
		if (threadID <= 0)
		{
			return;
		}

		std::string errfileName = m_logPath;
		std::string costFileName = m_logPath;

		if (threadType == THREAD_TYPE_MAIN)		// 主线程
		{
			errfileName += servername + "_main";
			costFileName += servername + "_main_cost";
		}
		else if (threadType == THREAD_TYPE_LOGIC)
		{
			errfileName += servername + "_logic";
			costFileName += servername + "_logic_cost";
		}
		else if (threadType == THREAD_TYPE_ASYNC)
		{
			errfileName += servername + "_async";
			costFileName += servername + "_async_cost";
		}
		else if (threadType == THREAD_TYPE_ACCEPT)
		{
			errfileName += servername + "_accept";
			costFileName += servername + "_accept_cost";
		}
		else if (threadType == THREAD_TYPE_RECV)
		{
			errfileName += servername + "_recv";
			costFileName += servername + "_recv_cost";
		}
		else if (threadType == THREAD_TYPE_SEND)
		{
			errfileName += servername + "_send";
			costFileName += servername + "_send_cost";
		}
		else
		{
			return;
		}

		ThreadLogFiles logs;
		logs.errorLog = errfileName;
		logs.costLog = costFileName;

		m_LogFilesMap.emplace(elem, logs);
	}
}

std::string CGameLogManage::GetErrorLog(pthread_t threadID)
{
	int serviceType = ConfigManage()->m_serviceType;
	if (serviceType <= SERVICE_TYPE_BEGIN || serviceType >= SERVICE_TYPE_END)
	{
		return "";
	}

	std::string str = "";
	LogFileElem elem;
	elem.threadID = threadID;
	elem.serverType = serviceType;

	auto iter = m_LogFilesMap.find(elem);
	if (iter != m_LogFilesMap.end())
	{
		const ThreadLogFiles& logs = iter->second;
		str = logs.errorLog;
	}


	// 根据日期生成最终的文件
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	char buf[128] = "";
	sprintf(buf, "_%04d-%02d-%02d.log", sysTime.wYear, sysTime.wMonth, sysTime.wDay);

	if (str == "")
	{
		str = m_logPath + "other_thread_err";
	}

	str += buf;

	// 判断当前文件大小，超过重新生成文件
	g_fileErrLogCount++;
	struct stat statbuf;
	if (g_fileErrLogCount % 5 == 1 && stat(str.c_str(), &statbuf) == 0 && statbuf.st_size > MAX_LOG_FILE_SIZE)
	{
		sprintf(buf, "%02d-%02d-%02d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
		iter->second.errorLog += buf;
	}

	return str;
}

std::string CGameLogManage::GetCostLog(pthread_t threadID)
{
	int serviceType = ConfigManage()->m_serviceType;
	if (serviceType <= SERVICE_TYPE_BEGIN || serviceType >= SERVICE_TYPE_END)
	{
		return "";
	}

	std::string str = "";
	LogFileElem elem;
	elem.threadID = threadID;
	elem.serverType = serviceType;

	auto iter = m_LogFilesMap.find(elem);
	if (iter != m_LogFilesMap.end())
	{
		const ThreadLogFiles& logs = iter->second;
		str = logs.costLog;
	}

	if (str == "")
	{
		str = m_logPath + "other_thread_cost.log";
	}
	else
	{
		str += ".log";
	}

	//// 根据日期生成最终的文件
	//SYSTEMTIME sysTime;
	//GetLocalTime(&sysTime);

	//char buf[128] = "";
	//sprintf(buf, "_%04d-%02d-%02d.log", sysTime.wYear, sysTime.wMonth, sysTime.wDay);

	//str += buf;

	//// 判断当前文件大小，超过重新生成文件
	//g_fileCostLogCount++;
	//struct stat statbuf;
	//if (g_fileCostLogCount % 5 == 1 && stat(str.c_str(), &statbuf) == 0 && statbuf.st_size > MAX_LOG_FILE_SIZE)
	//{
	//	sprintf(buf, "%02d-%02d-%02d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	//	iter->second.costLog += buf;
	//}

	return str;
}

bool CGameLogManage::AddLogFileFp(std::string strFile, FILE* fp)
{
	CSignedLockObject LockObject(&m_csLock, true);

	if (strFile == "" || !fp)
	{
		return false;
	}

	if (m_filesFpMap.find(strFile) != m_filesFpMap.end())
	{
		return false;
	}

	m_filesFpMap.emplace(strFile, fp);

	return true;
}

FILE* CGameLogManage::GetLogFileFp(std::string&& strFile)
{
	if (strFile == "")
	{
		return NULL;
	}

	auto iter = m_filesFpMap.find(strFile);
	if (iter != m_filesFpMap.end())
	{
		return iter->second;
	}

	return NULL;
}

// 设置进程日志目录
void CGameLogManage::SetLogPath(const std::string& path)
{
	m_logPath = path;
}

// 获取进程日志目录
std::string CGameLogManage::GetLogPath()
{
	return m_logPath;
}