#include "CommonHead.h"
#include <sys/syscall.h>
#include "Function.h"
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

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

unsigned long GetTickCount()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
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

#define MAX 64
void SetDaemon()
{
	//创建子进程，父进程退出，功能都在子进程
	pid_t pid = fork();
	if (pid > 0)
	{
		printf("父进程退出\n");
		exit(0);
	}
	else if (pid < 0)
	{
		printf("创建守护进程失败\n");
		exit(0);
	}

	//创建新回话，让进程摆脱原会话的控制，让进程摆脱原进程组的控制，让进程摆脱原控制终端的控制
	setsid();

	//修改当前工作路径到根目录下
	//由于在进程运行中，当前目录所在的文件是不能卸载的，这对以后的使用会造成很多的不便。
	//chdir("/");

	//重设文件掩码，Deamon创建文件不会有太大麻烦
	umask(0);

	//尽可能关闭文件描述符
	//新进程会从父进程那里继承一些已经打开了的文件。这些被打开的文件可能永远不会被守护进程读写，
	//而它们一直消耗系统资源。另外守护进程已经与所属的终端失去联系，那么从终端输入的字符不可能到达守护进程，
	//守护进程中常规方法（如printf）输出的字符也不可能在终端上显示。所以通常关闭从0到MAXFILE的所有文件描述符。
	/*for (auto i = 0; i < MAX; i++)
	{
		close(i);
	}*/

	/********************************/
	/***到这里守护进程已经创建完成***/
	/********************************/
}