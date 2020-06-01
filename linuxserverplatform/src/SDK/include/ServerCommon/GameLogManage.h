#pragma once

#include <map>
#include <string>
#include "Function.h"
#include "Lock.h"

struct LogFileElem
{
	pthread_t threadID;
	int serverType;

	LogFileElem()
	{
		threadID = 0;
		serverType = 0;
	}

	bool operator<(const LogFileElem& elem)const
	{
		if (threadID < elem.threadID)
		{
			return true;
		}
		else if (serverType == elem.serverType)
		{
			if (serverType < elem.serverType)
			{
				return true;
			}
		}
		return false;
	}
};

struct ThreadLogFiles
{
	std::string errorLog;
	std::string costLog;

	void Clear()
	{
		errorLog.clear();
		costLog.clear();
	}
};

// 游戏log管理
class CGameLogManage
{
private:
	CGameLogManage();
	~CGameLogManage();

public:
	static CGameLogManage* Instance();
	void Release();

	// 外部接口添加线程日志
	void AddLogFile(pthread_t threadID, int threadType, int roomID = 0);
	// 获取对应线程的errorlog
	std::string GetErrorLog(pthread_t threadID);
	// 获取对应线程的costLog
	std::string GetCostLog(pthread_t threadID);

	// 增加指定文件的fp
	bool AddLogFileFp(std::string strFile, FILE* fp);
	// 获取指定文件的fp
	FILE* GetLogFileFp(std::string&& strFile);

	// 设置进程日志目录
	void SetLogPath(const std::string& path);
	// 获取进程日志目录
	std::string GetLogPath();

private:
	// 日志文件map
	std::map<LogFileElem, ThreadLogFiles /*logFileName*/> m_LogFilesMap;

	// 文件描述符map
	std::unordered_map<std::string, FILE*> m_filesFpMap;

	// 进程日志目录
	std::string m_logPath;

	// 线程锁
	CSignedLock	m_csLock;
};

#define GameLogManage()		CGameLogManage::Instance()