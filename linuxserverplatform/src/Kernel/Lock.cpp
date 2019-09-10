#include "CommonHead.h"
#include "Lock.h"

CSignedLockObject::CSignedLockObject(CSignedLock * pLockObject, bool bAutoLock)
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

void CSignedLockObject::Lock()
{
	if (m_pLockObject)
	{
		m_iLockCount++;
		m_pLockObject->Lock();
	}
}

void CSignedLockObject::UnLock()
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