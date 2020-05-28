#pragma once

#include "GameLogManage.h"
#include "configManage.h"
#include <string>
#include <array>

const std::array<const char*, LOG_LEVEL_END> levelNames = { "[INFO]", "[WARNNING]", "[ERROR]", "[INFO]","[ERROR]", "[SYS_ERR]", };

//// 日志类
class CLog
{
public:
	CLog();
	~CLog();

public:
	static void Write(const char* pLogFile, int level, const char* pFile, int line, const char* pFuncName, const char* pFormat, ...);

	static void Write(const char* pLogFile, const char* pFuncName, const char* pFormat, ...);

	// 把buf中的内容写入logFile
	static void Write(const char* pLogFile, const char* buf);

	static void WriteSysErr(const char* pLogFile, int level, const char* pFile, int line, const char* pFuncName, const char* err, const char* pFormat, ...);
};

// 检测函数运行
class CAutoLog
{
public:
	CAutoLog(const char* pLogFile, const char* pFileName, const char* pFuncName, int line);
	~CAutoLog();

private:
	char m_logFile[MAX_FILE_NAME_SIZE];
	char m_fileName[MAX_FILE_NAME_SIZE];
	char m_funcName[MAX_FUNC_NAME_SIZE];
	int	 m_line;
};

// 统计单位
struct AutoCostInfo
{
	unsigned long long curCount;
	unsigned long long allCount;
	unsigned long long curCostTime;
	unsigned long long allCostTime;

	AutoCostInfo()
	{
		curCount = 0;
		allCount = 0;
		allCostTime = 0;
		curCostTime = 0;
	}
};

// 统计函数耗时
class CAutoLogCost
{
public:
	CAutoLogCost(const char* pLogFile, const char* pFuncName, int microSecs, bool once, const char* format, ...);
	~CAutoLogCost();

private:
	int m_microSecs;
	long long m_beginTime;
	bool m_bIsOnce;

	char m_logFile[MAX_FILE_NAME_SIZE];
	char m_buf[MAX_LOG_BUF_SIZE];
	char m_key[MAX_COST_BUF_SIZE];

	static std::unordered_map<std::string, AutoCostInfo> g_costMap;
};

// 性能统计
#define AUTOCOSTONCE(...)	CAutoLogCost logCost(GameLogManage()->GetCostLog(GetCurrentThreadId()).c_str(), __FUNCTION__, MIN_STATISTICS_FUNC_COST_TIME, true, __VA_ARGS__);
#define AUTOCOST(...)		CAutoLogCost logCost(GameLogManage()->GetCostLog(GetCurrentThreadId()).c_str(), __FUNCTION__, ALL_STATISTICS_FUNC_COST_TIME, false, __VA_ARGS__);

// 输出错误消息 【只输出到文件系统】
#define ERROR_LOG(...)	{ CLog::Write(GameLogManage()->GetErrorLog(GetCurrentThreadId()).c_str(), LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }

// 输出常规信息 【只输出到文件系统】
#define INFO_LOG(...)	{ CLog::Write(GameLogManage()->GetErrorLog(GetCurrentThreadId()).c_str(), LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }

// 输出重要警告信息 【只输出到文件系统】
#define WARNNING_LOG(...)	{ CLog::Write(GameLogManage()->GetErrorLog(GetCurrentThreadId()).c_str(), LOG_LEVEL_WARNNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }

// 输出错误消息 【输出到文件系统和控制台】
#define CON_ERROR_LOG(...)	{ CLog::Write(GameLogManage()->GetErrorLog(GetCurrentThreadId()).c_str(), LOG_LEVEL_ERROR_CONSOLE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }

// 输出常规信息 【输出到文件系统和控制台】
#define CON_INFO_LOG(...)	{ CLog::Write(GameLogManage()->GetErrorLog(GetCurrentThreadId()).c_str(), LOG_LEVEL_INFO_CONSOLE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }

// 输出系统错误信息，strerror(errno)函数全局的，调用SYS_ERROR_LOG之前，不能调用其它系统函数
#define SYS_ERROR_LOG(...)	{ CLog::WriteSysErr(GameLogManage()->GetErrorLog(GetCurrentThreadId()).c_str(), LOG_LEVEL_ERROR_SYS, __FILE__, __LINE__, __FUNCTION__,strerror(errno), __VA_ARGS__); }