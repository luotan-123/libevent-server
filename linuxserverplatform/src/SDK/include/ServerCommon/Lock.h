#pragma once

class CSignedLock;
class CSignedLockObject
{
private:
	int			m_iLockCount;		//锁定计数
	CSignedLock* m_pLockObject;		//锁定对象

public:
	CSignedLockObject(CSignedLock* pLockObject, bool bAutoLock);
	~CSignedLockObject();

public:
	inline void Lock();
	inline void UnLock();
};

// pthread_mutex_init  pthread_mutexattr_t 的第二个参数详细解释 
// PTHREAD_MUTEX_TIMED_NP，这是缺省值，也就是普通锁。当一个线程加锁以后，其余请求锁的线程将形成一个等待队列，并在解锁后按优先级获得锁。这种锁策略保证了资源分配的公平性。
// PTHREAD_MUTEX_RECURSIVE_NP，嵌套锁，允许同一个线程对同一个锁成功获得多次，并通过多次unlock解锁。如果是不同线程请求，则在加锁线程解锁时重新竞争。
// PTHREAD_MUTEX_ERRORCHECK_NP，检错锁，如果同一个线程请求同一个锁，则返回EDEADLK，否则与PTHREAD_MUTEX_TIMED_NP类型动作相同。这样保证当不允许多次加锁时不出现最简单情况下的死锁。
// PTHREAD_MUTEX_ADAPTIVE_NP，适应锁，动作最简单的锁类型，仅等待解锁后重新竞争。

class CSignedLock
{
	friend class CSignedLockObject;

private:
	pthread_mutex_t	m_csLock;
	pthread_mutexattr_t m_attr;

public:
	CSignedLock();
	~CSignedLock();

private:
	inline void Lock() { pthread_mutex_lock(&m_csLock); }
	inline void UnLock() { pthread_mutex_unlock(&m_csLock); }
};