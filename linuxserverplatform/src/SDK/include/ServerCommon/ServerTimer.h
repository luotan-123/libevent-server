#pragma once

#include <event2/event.h>
#include <event2/event_struct.h>


#define MAX_TIMER_THRED_NUMS	4		// 定时器最大线程数量

enum SERVERTIMER_TYPE
{
	SERVERTIMER_TYPE_PERISIST = 0,		// 持久定时器
	SERVERTIMER_TYPE_SINGLE,			// 一次性定时器
};

struct ServerTimerInfo
{
	unsigned int elapse;	// 定时器间隔（单位毫秒）
	long long starttime;	// 起始时间（单位毫秒）
	BYTE timertype;			// 定时器类型 SERVERTIMER_TYPE
	ServerTimerInfo()
	{
		elapse = 10;
		starttime = 0;
		timertype = SERVERTIMER_TYPE_PERISIST;
	}
};

class CDataLine;
class CSignedLock;
class CServerTimer
{
public:
	CServerTimer();
	~CServerTimer();
	bool Start(CDataLine* pDataLine);
	bool Stop();
	bool SetTimer(unsigned int uTimerID, unsigned int uElapse, BYTE timerType = SERVERTIMER_TYPE_PERISIST); //uElapse是毫秒单位，大于100ms
	bool KillTimer(unsigned int uTimerID);
	bool ExistsTimer(unsigned int uTimerID);

private:
	// 定时器执行函数
	static void TimeoutCB(evutil_socket_t fd, short event, void* arg);
	// 定时器线程函数
	static void* ThreadCheckTimer(void* pThreadData);
private:
	volatile bool m_bRun;
	std::unordered_map<unsigned int, ServerTimerInfo> m_timerMap;
	CDataLine* m_pDataLine;	// 共享的dataline对象
	CSignedLock* m_pLock; // 线程锁
};