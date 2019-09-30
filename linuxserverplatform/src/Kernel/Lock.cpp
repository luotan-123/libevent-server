#include "CommonHead.h"
#include <fcntl.h>
#include <sys/stat.h>
#include "Lock.h"

CSignedLockObject::CSignedLockObject(CSignedLock* pLockObject, bool bAutoLock)
{
	m_iLockCount = 0;

	if (pLockObject)
	{
		m_pLockObject = pLockObject;
	}

	if (bAutoLock)
	{
		Lock();
	}
}

CSignedLockObject::~CSignedLockObject()
{
	while (m_iLockCount > 0)
	{
		UnLock();
	}
}

inline void CSignedLockObject::Lock()
{
	if (m_pLockObject)
	{
		m_iLockCount++;
		m_pLockObject->Lock();
	}
}

inline void CSignedLockObject::UnLock()
{
	if (m_pLockObject)
	{
		m_iLockCount--;
		m_pLockObject->UnLock();
	}
}

// 初始化属性
CSignedLock::CSignedLock()
{
	pthread_mutexattr_init(&m_attr);

	// 设置互斥锁属性
	pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE_NP);

	pthread_mutex_init(&m_csLock, &m_attr);
}

CSignedLock::~CSignedLock()
{
	pthread_mutex_destroy(&m_csLock);

	pthread_mutexattr_destroy(&m_attr);
}

////////////////////////////////////////////////////////////////////////////////
CFIFOEvent::CFIFOEvent(const char* filename)
{
	m_fifoName = filename;

	int res = 0;

	if (access(m_fifoName, F_OK) == -1)
	{
		res = mkfifo(m_fifoName, 0777);
		if (res != 0)
		{
			printf("could not create fifo:%s\n", strerror(errno));
			m_fifoName = NULL;
		}
	}
}

void CFIFOEvent::WaitForEvent()
{
	if (!m_fifoName)
	{
		return;
	}

	int fifoFd = open(m_fifoName, O_RDONLY);
	int res = 0;

	if (fifoFd != -1)
	{
		char chTemp;
		res = read(fifoFd, &chTemp, sizeof(chTemp));
		if (res == -1)
		{
			printf("read error:%s\n", strerror(errno));
		}

		close(fifoFd);
	}
	else
	{
		printf("open error:%s\n", strerror(errno));
	}
}

void CFIFOEvent::SetEvent()
{
	if (!m_fifoName)
	{
		return;
	}

	int fifoFd = open(m_fifoName, O_WRONLY);
	int res = 0;

	if (fifoFd != -1)
	{
		char chTemp = 123;
		res = write(fifoFd, &chTemp, sizeof(chTemp));
		if (res == -1)
		{
			printf("write error:%s\n", strerror(errno));
		}

		close(fifoFd);
	}
	else
	{
		printf("open error:%s\n", strerror(errno));
	}
}