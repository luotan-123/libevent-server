#pragma once

#include <event2/event.h>
#include <event2/event_struct.h>


struct ServerTimerInfo
{
	unsigned int elapse;	// 定时器间隔（单位毫秒）
	long long starttime;	// 起始时间（单位毫秒）
	ServerTimerInfo()
	{
		elapse = 10;
		starttime = 0;
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
	bool SetTimer(unsigned int uTimerID, unsigned int uElapse); //uElapse是毫秒单位
	bool KillTimer(unsigned int uTimerID);
	bool ExistsTimer(unsigned int uTimerID);

private:
	// 定时器执行函数
	static void TimeoutCB(evutil_socket_t fd, short event, void* arg);
	// 定时器线程函数
	static void * ThreadCheckTimer(void* pThreadData);
private:
	volatile bool m_bRun;
	std::unordered_map<unsigned int, ServerTimerInfo> m_timerMap;
	CDataLine* m_pDataLine;	// 共享的dataline对象
	CSignedLock* m_pLock; // 线程锁
};