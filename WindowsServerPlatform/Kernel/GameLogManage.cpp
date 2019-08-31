#include "pch.h"
#include "GameLogManage.h"
#include "configManage.h"
#include "KernelDefine.h"

CGameLogManage::CGameLogManage()
{

}

CGameLogManage::~CGameLogManage()
{
	//Release();
}

CGameLogManage* CGameLogManage::Instance()
{
	static CGameLogManage mgr;
	return &mgr;
}

void CGameLogManage::Release()
{
	m_centerLogFilesMap.clear();
	m_logonLogFilesMap.clear();
	m_loaderLogFilesMap.clear();

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

void CGameLogManage::AddCenterLogFile(int threadID, int threadType)
{
	if (threadID <= 0)
	{
		return;
	}

	auto iter = m_centerLogFilesMap.find(threadID);
	if (iter != m_centerLogFilesMap.end())
	{
		return;
	}

	std::string errfileName;
	std::string costFileName;

	if (threadType == THREAD_TYPE_MAIN)		// 主线程
	{
		errfileName = "log/centerserver_sys";
		costFileName = "log/centerserver_sys_cost";
	}
	else if (threadType == THREAD_TYPE_LOGIC)
	{
		errfileName = "log/centerserver_err";
		costFileName = "log/centerserver_cost";
	}
	else if (threadType == THREAD_TYPE_ASYNC)
	{
		errfileName = "log/centerserver_async_err";
		costFileName = "log/centerserver_async_cost";
	}
	else if (threadType == THREAD_TYPE_ACCEPT)
	{
		errfileName = "log/centerserver_accept_err";
		costFileName = "log/centerserver_accept_cost";
	}
	else if (threadType == THREAD_TYPE_RECV)
	{
		errfileName = "log/centerserver_recv_err";
		costFileName = "log/centerserver_recv_cost";
	}
	else
	{
		return;
	}

	ThreadLogFiles logs;
	logs.errorLog = errfileName;
	logs.costLog = costFileName;

	m_centerLogFilesMap.insert(std::make_pair(threadID, logs));
}

void CGameLogManage::AddLogonLogFile(int threadID, int threadType)
{
	if (threadID <= 0)
	{
		return;
	}

	auto iter = m_logonLogFilesMap.find(threadID);
	if (iter != m_logonLogFilesMap.end())
	{
		return;
	}

	std::string errfileName;
	std::string costFileName;

	if (threadType == THREAD_TYPE_MAIN)		// 主线程
	{
		errfileName = "log/logonserver_sys";
		costFileName = "log/logonserver_sys_cost";
	}
	else if (threadType == THREAD_TYPE_LOGIC)
	{
		errfileName = "log/logonserver_err";
		costFileName = "log/logonserver_cost";
	}
	else if (threadType == THREAD_TYPE_ASYNC)
	{
		errfileName = "log/logonserver_async_err";
		costFileName = "log/logonserver_async_cost";
	}
	else if (threadType == THREAD_TYPE_ACCEPT)
	{
		errfileName = "log/logonserver_accept_err";
		costFileName = "log/logonserver_accept_cost";
	}
	else if (threadType == THREAD_TYPE_RECV)
	{
		errfileName = "log/logonserver_recv_err";
		costFileName = "log/logonserver_recv_cost";
	}
	else
	{
		return;
	}

	ThreadLogFiles logs;
	logs.errorLog = errfileName;
	logs.costLog = costFileName;

	m_logonLogFilesMap.insert(std::make_pair(threadID, logs));
}

void CGameLogManage::AddLoaderLogFile(int threadID, int threadType, int roomID)
{
	if (threadID <= 0)
	{
		return;
	}

	if (threadType == THREAD_TYPE_LOGIC && roomID <= 0)
	{
		return;
	}

	std::string errfileName;
	std::string costFileName;

	if (threadType == THREAD_TYPE_MAIN)		// 主线程
	{
		errfileName = "log/loaderserver_sys";
		costFileName = "log/loaderserver_sys_cost";
	}
	else if (threadType == THREAD_TYPE_LOGIC)	// 逻辑线程(每个游戏都是单独的)
	{
		RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
		if (!pRoomBaseInfo)
		{
			return;
		}

		errfileName += "log/";
		errfileName += pRoomBaseInfo->name;
		errfileName += "_err";

		costFileName += "log/";
		costFileName += pRoomBaseInfo->name;
		costFileName += "_cost";
	}
	else if (threadType == THREAD_TYPE_ASYNC)
	{
		RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
		if (!pRoomBaseInfo)
		{
			return;
		}

		errfileName += "log/";
		errfileName += pRoomBaseInfo->name;
		errfileName += "_async_err";

		costFileName += "log/";
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

		errfileName += "log/";
		errfileName += pRoomBaseInfo->name;
		errfileName += "_accept_err";

		costFileName += "log/";
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

		errfileName += "log/";
		errfileName += pRoomBaseInfo->name;
		errfileName += "_recv_err";

		costFileName += "log/";
		costFileName += pRoomBaseInfo->name;
		costFileName += "_recv_cost";
	}
	else
	{
		return;
	}

	ThreadLogFiles logs;
	logs.errorLog = errfileName;
	logs.costLog = costFileName;

	m_loaderLogFilesMap.insert(std::make_pair(threadID, logs));
}

void CGameLogManage::AddLogFile(int threadID, int threadType, int roomID/* = 0*/)
{
	int serviceType = ConfigManage()->m_serviceType;
	if (serviceType <= SERVICE_TYPE_BEGIN || serviceType >= SERVICE_TYPE_END)
	{
		return;
	}

	if (serviceType == SERVICE_TYPE_LOGON)
	{
		AddLogonLogFile(threadID, threadType);
	}
	else if (serviceType == SERVICE_TYPE_LOADER)
	{
		AddLoaderLogFile(threadID, threadType, roomID);
	}
	else if (serviceType == SERVICE_TYPE_CENTER)
	{
		AddCenterLogFile(threadID, threadType);
	}
}

std::string CGameLogManage::GetErrorLog(int threadID)
{
	int serviceType = ConfigManage()->m_serviceType;
	if (serviceType <= SERVICE_TYPE_BEGIN || serviceType >= SERVICE_TYPE_END)
	{
		return "";
	}

	std::string str = "";

	if (serviceType == SERVICE_TYPE_LOGON)
	{
		auto iter = m_logonLogFilesMap.find(threadID);
		if (iter != m_logonLogFilesMap.end())
		{
			const ThreadLogFiles& logs = iter->second;
			str = logs.errorLog;
		}
	}
	else if (serviceType == SERVICE_TYPE_LOADER)
	{
		auto iter = m_loaderLogFilesMap.find(threadID);
		if (iter != m_loaderLogFilesMap.end())
		{
			const ThreadLogFiles& logs = iter->second;
			str = logs.errorLog;
		}
	}
	else if (serviceType == SERVICE_TYPE_CENTER)
	{
		auto iter = m_centerLogFilesMap.find(threadID);
		if (iter != m_centerLogFilesMap.end())
		{
			const ThreadLogFiles& logs = iter->second;
			str = logs.errorLog;
		}
	}

	// 根据日期生成最终的文件
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	char buf[128] = "";
	sprintf(buf, " %04d-%02d-%02d.log", sysTime.wYear, sysTime.wMonth, sysTime.wDay);

	if (str == "")
	{
		str = "log/other err";
	}

	str += buf;

	return str;
}

std::string CGameLogManage::GetCostLog(int threadID)
{
	int serviceType = ConfigManage()->m_serviceType;
	if (serviceType <= SERVICE_TYPE_BEGIN || serviceType >= SERVICE_TYPE_END)
	{
		return "";
	}

	std::string str = "";

	if (serviceType == SERVICE_TYPE_LOGON)
	{
		auto iter = m_logonLogFilesMap.find(threadID);
		if (iter != m_logonLogFilesMap.end())
		{
			const ThreadLogFiles& logs = iter->second;
			str = logs.costLog;
		}
	}
	else if (serviceType == SERVICE_TYPE_LOADER)
	{
		auto iter = m_loaderLogFilesMap.find(threadID);
		if (iter != m_loaderLogFilesMap.end())
		{
			const ThreadLogFiles& logs = iter->second;
			str = logs.costLog;
		}
	}
	else if (serviceType == SERVICE_TYPE_CENTER)
	{
		auto iter = m_centerLogFilesMap.find(threadID);
		if (iter != m_centerLogFilesMap.end())
		{
			const ThreadLogFiles& logs = iter->second;
			str = logs.costLog;
		}
	}

	// 根据日期生成最终的文件
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	char buf[128] = "";
	sprintf(buf, " %04d-%02d-%02d.log", sysTime.wYear, sysTime.wMonth, sysTime.wDay);

	if (str == "")
	{
		str = "log/cost";
	}

	str += buf;

	return str;
}

bool CGameLogManage::AddLogFileFp(std::string strFile, FILE * fp)
{
	if (strFile == "" || !fp)
	{
		return false;
	}

	if (m_filesFpMap.find(strFile) != m_filesFpMap.end())
	{
		return false;
	}

	m_filesFpMap.insert(std::make_pair(strFile, fp));

	return true;
}

FILE * CGameLogManage::GetLogFileFp(std::string&& strFile)
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
