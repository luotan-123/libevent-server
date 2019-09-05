#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

// 定时器精度，单位毫秒
#define SERVER_TIME_ONCE	100

struct ServerTimerInfo
{
	unsigned int elapse;  // 转换之后的时间间隔
	long long startslice; // 起始时间
	ServerTimerInfo()
	{
		elapse = 10;
		startslice = 0;
	}
};

class CDataLine;
class CSignedLock;
class KERNEL_CLASS CServerTimer
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
	// 定时器线程函数
	static unsigned __stdcall ThreadCheckTimer(LPVOID pThreadData);

private:
	volatile bool m_bRun;
	volatile long long m_llTimeslice;// 时间片
	std::unordered_map<unsigned int, ServerTimerInfo> m_timerMap;
	CDataLine* m_pDataLine;	// 共享的dataline对象
	CSignedLock* m_pLock; // 线程锁
};