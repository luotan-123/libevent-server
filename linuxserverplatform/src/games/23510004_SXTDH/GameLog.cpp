//////////////////////////////////////////////////////////////////////////
/// 调试信息、出错信息输出函数定义
/// 其中，GameLog用于调试跟踪时输出信息，发行时可以修改函数体，不写文件直接返回
#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "../GameMessage/upgrademessage.h"

//打印日志
void GameLog::OutputFile(const char *strOutputString, ...)
{
#ifdef _RELEASEEX
	//运营版禁止调用OutputFile函数，因为该函数耗时约1.5毫秒
	return;
#endif // _RELEASEEX
	char szFilename[256];
	CString strPath = CINIFile::GetAppPath() + GAMENAME + "_log\\";
	SHCreateDirectoryEx(NULL, strPath, NULL);
	CTime time = CTime::GetCurrentTime();
	sprintf(szFilename, "%s%d__%d%02d%02d_log.txt", strPath.GetBuffer(), NAME_ID, time.GetYear(), time.GetMonth(), time.GetDay());
	FILE *fp = fopen(szFilename, "a");
	if (NULL == fp)
	{
		return;
	}
	//插入时间
	char cTime[30];
	::memset(cTime, 0, sizeof(cTime));
	sprintf(cTime, "[%d:%d:%d] ", time.GetHour(), time.GetMinute(), time.GetSecond());
	fprintf(fp, cTime);
	va_list arg;
	va_start(arg, strOutputString);
	vfprintf(fp, strOutputString, arg);
	fprintf(fp, "\n");
	fclose(fp);
}

void GameLog::SFile(const char * strOutputString, ...)
{
	char szFilename[256];
	CString strPath = CINIFile::GetAppPath() + GAMENAME + "_log\\";
	SHCreateDirectoryEx(NULL, strPath, NULL);
	CTime time = CTime::GetCurrentTime();
	sprintf(szFilename, "%s%d__%d%02d%02d_特殊日志.txt", strPath.GetBuffer(), NAME_ID, time.GetYear(), time.GetMonth(), time.GetDay());
	FILE *fp = fopen(szFilename, "a");
	if (NULL == fp)
	{
		return;
	}
	//插入时间
	char cTime[30];
	::memset(cTime, 0, sizeof(cTime));
	sprintf(cTime, "[%d:%d:%d] ", time.GetHour(), time.GetMinute(), time.GetSecond());
	fprintf(fp, cTime);
	va_list arg;
	va_start(arg, strOutputString);
	vfprintf(fp, strOutputString, arg);
	fprintf(fp, "\n");
	fclose(fp);
}

void GameLog::OutputString(const char * strOutputString, ...)
{
#ifdef _RELEASEEX
	//运营版禁止调用OutputDebugString函数，因为该函数耗时严重
	return;
#endif // _RELEASEEX
	//插入时间
	CTime time = CTime::GetCurrentTime();
	char cTime[30];
	::memset(cTime, 0, sizeof(cTime));
	sprintf(cTime, "\n[%d:%d:%d] ", time.GetHour(), time.GetMinute(), time.GetSecond());
	OutputDebugString(cTime);
	char strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	_vsnprintf(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
	va_end(vlArgs);
	OutputDebugString(strBuffer);
	OutputDebugString("\n");
}