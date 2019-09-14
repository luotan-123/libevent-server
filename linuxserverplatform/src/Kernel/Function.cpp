#include "CommonHead.h"
#include "Function.h"
#include <sys/syscall.h>

// 获取系统时间
void GetLocalTime(SYSTEMTIME* sysTime)
{
	if (!system)
	{
		return;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t tt = tv.tv_sec;
	tt = tt + 8 * 3600;  // transform the time zone
	tm* t = gmtime(&tt);

	if (!t)
	{
		return;
	}

	sysTime->wYear = t->tm_year + 1900;
	sysTime->wMonth = t->tm_mon + 1;
	sysTime->wDayOfWeek = t->tm_wday;
	sysTime->wDay = t->tm_mday;
	sysTime->wHour = t->tm_hour;
	sysTime->wMinute = t->tm_min;
	sysTime->wSecond = t->tm_sec;
	sysTime->wMilliseconds = tv.tv_usec / 1000;
	sysTime->lMicroseconds = tv.tv_usec;
}

//获取线程pid
pthread_t GetCurrentThreadId()
{
	return pthread_self();
}

pthread_t GetCurrentSysThreadId()
{
	return syscall(SYS_gettid);
}