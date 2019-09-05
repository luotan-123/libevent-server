#include "pch.h"
#include "ServerTimer.h"
#include "log.h"
#include "Lock.h"
#include "InternalMessageDefine.h"

CServerTimer::CServerTimer()
{
	m_bRun = false;
	m_pDataLine = NULL;
	m_llTimeslice = 0;
	m_pLock = new CSignedLock;
}

CServerTimer::~CServerTimer()
{
	m_timerMap.clear();
	delete m_pLock;
	m_pLock = NULL;
}

bool CServerTimer::Start(CDataLine* pDataLine)
{
	if (!pDataLine)
	{
		CON_ERROR_LOG("pDataLine == NULL");
		return false;
	}

	m_pDataLine = pDataLine;
	m_bRun = true;

	// ¿ª±ÙÏß³Ì
	unsigned int connectThreadID = 0;
	_beginthreadex(NULL, 0, ThreadCheckTimer, this, 0, &connectThreadID);

	return true;
}
bool CServerTimer::Stop()
{
	m_bRun = false;
	m_timerMap.clear();

	return true;
}
bool CServerTimer::SetTimer(unsigned int uTimerID, unsigned int uElapse)
{
	if (uElapse < SERVER_TIME_ONCE)
	{
		uElapse = SERVER_TIME_ONCE;
	}

	CSignedLockObject lock(m_pLock, true);

	ServerTimerInfo info;
	info.elapse = uElapse / SERVER_TIME_ONCE;
	info.startslice = m_llTimeslice + info.elapse;

	m_timerMap[uTimerID] = info;

	return true;
}

bool CServerTimer::KillTimer(unsigned int uTimerID)
{
	CSignedLockObject lock(m_pLock, true);

	auto iter = m_timerMap.find(uTimerID);
	if (iter == m_timerMap.end())
	{
		return false;
	}

	m_timerMap.erase(iter);

	return true;
}

bool CServerTimer::ExistsTimer(unsigned int uTimerID)
{
	auto iter = m_timerMap.find(uTimerID);
	if (iter == m_timerMap.end())
	{
		return false;
	}

	return true;
}

unsigned __stdcall CServerTimer::ThreadCheckTimer(LPVOID pThreadData)
{
	INFO_LOG("CServerTimer thread begin...");

	CServerTimer* pThis = (CServerTimer*)pThreadData;
	if (!pThis)
	{
		return -1;
	}

	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = SERVER_TIME_ONCE * 1000;

	while (pThis->m_bRun)
	{
		Sleep(SERVER_TIME_ONCE);
		pThis->m_llTimeslice++;
		
		// lock
		CSignedLockObject lock(pThis->m_pLock, false);
		lock.Lock();

		for (auto iter = pThis->m_timerMap.begin(); iter != pThis->m_timerMap.end(); iter++)
		{
			if ((pThis->m_llTimeslice - iter->second.startslice) % iter->second.elapse == 0 && pThis->m_pDataLine)
			{
				WindowTimerLine WindowTimer;
				WindowTimer.uTimerID = iter->first;
				pThis->m_pDataLine->AddData(&WindowTimer.LineHead, sizeof(WindowTimer), HD_TIMER_MESSAGE);
				printf("%lld\n", pThis->m_llTimeslice);
			}
		}

		lock.UnLock();
	}

	INFO_LOG("CServerTimer thread exit.");

	return 0;
}