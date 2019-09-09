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