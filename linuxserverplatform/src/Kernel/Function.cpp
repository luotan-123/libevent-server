#include "CommonHead.h"
#include <sys/syscall.h>
#include "Function.h"

int GetNewArraySize(void* pArray)
{
	if (pArray == NULL)
	{
		return 0;
	}

	int* p = (int*)pArray;
	int iSize = *(p - 2);

	if (iSize < 0 || iSize > INT32_MAX)
	{
		return 0;
	}

	return iSize;
}

void GetLocalTime(SYSTEMTIME* sysTime)
{
	if (!sysTime)
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

long long GetSysMilliseconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

pthread_t GetCurrentThreadId()
{
	return pthread_self();
}

pthread_t GetCurrentSysThreadId()
{
	return syscall(SYS_gettid);
}

pid_t GetProcessPidByName(const char* procName)
{
	FILE* fp = NULL;
	char buf[NAME_MAX] = "";
	char cmd[NAME_MAX] = "";
	pid_t pid = -1;
	sprintf(cmd, "pidof %s", procName);

	if ((fp = popen(cmd, "r")) != NULL)
	{
		if (fgets(buf, 255, fp) != NULL)
		{
			pid = atoi(buf);
		}
	}

	pclose(fp);

	return pid;
}