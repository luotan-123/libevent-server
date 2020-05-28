#include "CommonHead.h"
#include "log.h"
#include <stdarg.h>

std::unordered_map<std::string, AutoCostInfo> CAutoLogCost::g_costMap;

CLog::CLog()
{
}

CLog::~CLog()
{

}

void CLog::Write(const char* pLogfile, int level, const char* pFile, int line, const char* pFuncName, const char* pBuf, ...)
{
	if (!pLogfile || !pFile || !pFuncName || !pBuf)
	{
		return;
	}

	if (level >= (int)levelNames.size())
	{
		return;
	}

	const char* levelName = levelNames[level];
	if (!levelName)
	{
		return;
	}

	char buf[MAX_LOG_BUF_SIZE] = "";

	// 线程ID和level
	pthread_t threadID = GetCurrentSysThreadId();
	sprintf(buf, "%lu %s ", threadID, levelName);

	// 时间
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	int millisecs = sysTime.wSecond * 1000 + sysTime.wMilliseconds;

	sprintf(buf + strlen(buf), "%04d-%02d-%02d %02d:%02d:%05d ", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, millisecs);

	// 参数
	va_list args;
	va_start(args, pBuf);

	vsprintf(buf + strlen(buf), pBuf, args);
	va_end(args);

	// 输出到控制台
	if (level == LOG_LEVEL_INFO_CONSOLE)
	{
		std::cout << buf << std::endl;
	}
	else if (level == LOG_LEVEL_ERROR_CONSOLE)
	{
		std::cout << buf << "{func=" << pFuncName << " line=" << line << "}\n";
	}

	sprintf(buf + strlen(buf), "{%s %s %d}\n", pFile, pFuncName, line);

	std::string strFile = pLogfile;
	FILE* fp = GameLogManage()->GetLogFileFp(std::move(strFile));
	if (!fp)
	{
		fp = fopen(pLogfile, "a+");
		if (!fp)
		{
			return;
		}
		GameLogManage()->AddLogFileFp(pLogfile, fp);
	}

	fputs(buf, fp);
	fflush(fp);
}

void CLog::Write(const char* pLogFile, const char* pFuncName, const char* pFormat, ...)
{
	if (!pLogFile || !pFuncName || !pFormat)
	{
		return;
	}

	char buf[MAX_LOG_BUF_SIZE] = "";

	// 线程ID和level
	pthread_t threadID = GetCurrentSysThreadId();
	const char* levelName = levelNames[LOG_LEVEL_INFO];
	sprintf(buf, "%lu %s ", threadID, levelName);

	// 时间
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	int millisecs = sysTime.wSecond * 1000 + sysTime.wMilliseconds;

	sprintf(buf + strlen(buf), "%04d-%02d-%02d %02d:%02d:%05d ", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, millisecs);

	// 参数
	va_list args;
	va_start(args, pFormat);

	vsprintf(buf + strlen(buf), pFormat, args);
	va_end(args);

	sprintf(buf + strlen(buf), " %s\n", pFuncName);

	std::string strFile = pLogFile;
	FILE* fp = GameLogManage()->GetLogFileFp(std::move(strFile));
	if (!fp)
	{
		fp = fopen(pLogFile, "a+");
		if (!fp)
		{
			return;
		}
		GameLogManage()->AddLogFileFp(pLogFile, fp);
	}

	fputs(buf, fp);
	fflush(fp);
}

void CLog::Write(const char* pLogFile, const char* buf)
{
	if (!pLogFile || !buf)
	{
		return;
	}

	std::string strFile = pLogFile;
	FILE* fp = GameLogManage()->GetLogFileFp(std::move(strFile));
	if (!fp)
	{
		fp = fopen(pLogFile, "a+");
		if (!fp)
		{
			return;
		}
		GameLogManage()->AddLogFileFp(pLogFile, fp);
	}

	fputs(buf, fp);
	fflush(fp);
}

void CLog::WriteSysErr(const char* pLogfile, int level, const char* pFile, int line, const char* pFuncName, const char* err, const char* pBuf, ...)
{
	if (!pLogfile || !pFile || !pFuncName || !pBuf)
	{
		return;
	}

	if (level >= (int)levelNames.size())
	{
		return;
	}

	const char* levelName = levelNames[level];
	if (!levelName)
	{
		return;
	}

	char buf[MAX_LOG_BUF_SIZE] = "";

	// 线程ID和level
	pthread_t threadID = GetCurrentSysThreadId();
	sprintf(buf, "%lu %s ", threadID, levelName);

	// 时间
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	int millisecs = sysTime.wSecond * 1000 + sysTime.wMilliseconds;

	sprintf(buf + strlen(buf), "%04d-%02d-%02d %02d:%02d:%05d ", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, millisecs);

	// 参数
	va_list args;
	va_start(args, pBuf);

	vsprintf(buf + strlen(buf), pBuf, args);
	va_end(args);

	// 输出到控制台
	std::cout << buf << " {err=[" << err << "] func=" << pFuncName << " line=" << line << "}\n";

	sprintf(buf + strlen(buf), " {%s} ", err);

	sprintf(buf + strlen(buf), "{%s %s %d}\n", pFile, pFuncName, line);

	std::string strFile = pLogfile;
	FILE* fp = GameLogManage()->GetLogFileFp(std::move(strFile));
	if (!fp)
	{
		fp = fopen(pLogfile, "a+");
		if (!fp)
		{
			return;
		}
		GameLogManage()->AddLogFileFp(pLogfile, fp);
	}

	fputs(buf, fp);
	fflush(fp);
}

CAutoLog::CAutoLog(const char* pLogFile, const char* pFileName, const char* pFuncName, int line)
{
	memset(m_logFile, 0, sizeof(m_logFile));
	memset(m_fileName, 0, sizeof(m_fileName));
	memset(m_funcName, 0, sizeof(m_funcName));

	if (pLogFile && pFileName && pFuncName)
	{
		strcpy(m_logFile, pLogFile);
		strcpy(m_fileName, pFileName);
		strcpy(m_funcName, pFuncName);
	}

	m_line = line;

	// 打印开始信息
	CLog::Write(m_logFile, LOG_LEVEL_INFO, pFileName, line, pFuncName, "AUTOLOG Begin:");
}

CAutoLog::~CAutoLog()
{
	CLog::Write(m_logFile, LOG_LEVEL_INFO, m_fileName, m_line, m_funcName, "AUTOLOG End。");
}

/////////////////////////////////////////////////////////////////////////////////////////////

CAutoLogCost::CAutoLogCost(const char* plogFile, const char* pFuncName, int microSecs, bool once, const char* pFormat, ...)
{
	m_logFile[0] = 0;
	m_buf[0] = 0;
	m_key[0] = 0;
	m_microSecs = microSecs;
	m_bIsOnce = once;

	// 时间
	struct timeval tv;
	gettimeofday(&tv, NULL);

	// 记录开始时间
	m_beginTime = tv.tv_sec * 1000000 + tv.tv_usec;

	// 选择log文件
	if (plogFile)
	{
		strcpy(m_logFile, plogFile);
	}
	else
	{
		strcpy(m_logFile, "cost.log");
	}

	// 时间搓和函数名
	sprintf(m_buf + strlen(m_buf), "%ld func=%s ", tv.tv_sec, pFuncName);

	// pid
	sprintf(m_key, "pid=%ld ", GetCurrentSysThreadId());

	// 变长参数
	va_list args;
	va_start(args, pFormat);

	vsprintf(m_key + strlen(m_key), pFormat, args);

	va_end(args);

	sprintf(m_buf + strlen(m_buf), " %s", m_key);
}

CAutoLogCost::~CAutoLogCost()
{
	if (m_key[0] == 0)
	{
		return;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);

	// 记录结束时间
	long long endTime = tv.tv_sec * 1000000 + tv.tv_usec;
	long long costTime = endTime - m_beginTime;

	if (m_bIsOnce)
	{
		if (costTime >= m_microSecs)
		{
			sprintf(m_buf + strlen(m_buf), " costTime:[%lldus]\n", costTime);
			CLog::Write(m_logFile, m_buf);
		}
		return;
	}

	auto iter = g_costMap.find(m_key);
	if (iter == g_costMap.end())
	{
		AutoCostInfo info;

		info.curCount++;
		info.allCount++;
		info.curCostTime += costTime;
		info.allCostTime += costTime;

		g_costMap.emplace(m_key, info);

		iter = g_costMap.find(m_key);
		if (iter == g_costMap.end())
		{
			return;
		}
	}
	else
	{
		iter->second.curCount++;
		iter->second.allCount++;
		iter->second.curCostTime += costTime;
		iter->second.allCostTime += costTime;
	}

	if (iter->second.curCostTime >= m_microSecs)
	{
		sprintf(m_buf + strlen(m_buf), " ave=[call:%llu cost_time:%llu] all=[all_call:%llu all_cost_time:%llu]\n",
			iter->second.curCount, iter->second.curCostTime, iter->second.allCount, iter->second.allCostTime);
		CLog::Write(m_logFile, m_buf);

		iter->second.curCount = 0;
		iter->second.curCostTime = 0;
	}
}
