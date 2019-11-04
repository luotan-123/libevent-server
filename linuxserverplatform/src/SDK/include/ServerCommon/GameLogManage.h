#pragma once

#include <map>
#include <string>
#include "Function.h"
#include "Lock.h"

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
	void SetLogPath(const std::string &path);
private:
	// 添加中心服务器相关日志文件
	void AddCenterLogFile(pthread_t threadID, int threadType);

	// 添加大厅服务器相关日志文件
	void AddLogonLogFile(pthread_t threadID, int threadType);

	// 添加游戏服务器相关日志文件
	void AddLoaderLogFile(pthread_t threadID, int threadType, int roomID);

private:
	// 游戏日志文件map
	std::map<pthread_t /*threadID*/, ThreadLogFiles /*logFileName*/> m_loaderLogFilesMap;
	// 大厅日志文件map
	std::map<pthread_t /*threadID*/, ThreadLogFiles /*logFileName*/> m_logonLogFilesMap;
	// 中心服日志文件map
	std::map<pthread_t /*threadID*/, ThreadLogFiles /*logFileName*/> m_centerLogFilesMap;

	// 文件描述符map
	std::map<std::string, FILE*> m_filesFpMap;

	// 进程日志目录
	std::string m_logPath;

	// 线程锁
	CSignedLock	m_csLock;
};

#define GameLogManage()		CGameLogManage::Instance()