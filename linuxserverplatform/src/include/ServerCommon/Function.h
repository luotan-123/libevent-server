#pragma once

#include "KernelDefine.h"

//时间参数
typedef struct _SYSTEMTIME {
	int wYear;
	int wMonth;
	int wDayOfWeek;
	int wDay;
	int wHour;
	int wMinute;
	int wSecond;
	int wMilliseconds;
	long lMicroseconds;
} SYSTEMTIME, * PSYSTEMTIME, * LPSYSTEMTIME;


////////////////////////////////////系统函数实现/////////////////////////////////////////////

//计算数组维数
#define  CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//安全删除指针
#define  SafeDelete(pData) { try { delete pData; } catch (...) { ERROR_LOG("CATCH:%s with %s\n",__FILE__,__FUNCTION__);} pData = NULL; } 

//安全删除指针
#define  SafeDeleteArray(pData)	{ if(pData){ try { delete [] pData; } catch (...) { ERROR_LOG("CATCH:%s with %s\n",__FILE__,__FUNCTION__);} pData = NULL;} } 

//安全删除指针
#define  SAFE_DELETE(pData) { if(pData){ try{ delete pData; } catch(...){ ERROR_LOG("CATCH: *** SAFE_DELETE(%s) crash! *** %s %d\n",#pData,__FILE__, __LINE__); } pData = nullptr; } }

//获取动态数组指针大小，使用jemalloc，只有非标准类型才能使用
extern int GetNewArraySize(void* pArray);

//判断大小函数
#define Min_(x,y) ((x)>(y)?(y):(x))
#define Max_(x,y) ((x)>(y)?(x):(y))

//获取系统时间
extern void GetLocalTime(SYSTEMTIME* sysTime);

//获取时间戳（单位：毫秒）
extern long long GetSysMilliseconds();

//获取自系统开机以来的毫秒数
extern unsigned long GetTickCount();

//获取线程pid,线程内部id  pthread库函数
extern pthread_t GetCurrentThreadId();

//获取线程pid,内核线程id
extern pthread_t GetCurrentSysThreadId();

//根据进程名字获取进程id
extern pid_t GetProcessPidByName(const char* procName);

//设置守护进程
void SetDaemon();