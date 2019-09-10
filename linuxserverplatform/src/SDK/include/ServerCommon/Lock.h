#pragma once

class CSignedLock;
class CSignedLockObject
{
private:
	int			m_iLockCount;		//锁定计数
	CSignedLock* m_pLockObject;		//锁定对象

public:
	CSignedLockObject(CSignedLock * pLockObject, bool bAutoLock);
	~CSignedLockObject();

public:
	inline void Lock();
	inline void UnLock();
};

class CSignedLock
{
	friend class CSignedLockObject;

private:
	pthread_mutex_t	m_csLock;

public:
	inline CSignedLock() {  pthread_mutex_init(&m_csLock, NULL); }
	inline ~CSignedLock() { pthread_mutex_destroy(&m_csLock); }

private:
	inline void Lock() { pthread_mutex_lock(&m_csLock); }
	inline void UnLock() { pthread_mutex_unlock(&m_csLock); }
};