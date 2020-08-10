/**
*  @file    UnlockQueue.h
*  @brief   并发无锁队列。只适用于：单线程读、单线程写
*  @author  luotan
*  @date    2020-08-06
*/
#ifndef _UNLOCK_QUEUE_H
#define _UNLOCK_QUEUE_H

#include <pthread.h>

enum QueueType
{
	QUEUE_TYPE_NO = 0,		// 无类型			cpu使用率200%
	QUEUE_TYPE_COND = 1,	// 条件变量通知		cpu使用率190%
	QUEUE_TYPE_SLEEP = 2,   // 线程休眠			cpu使用率200%
};

class UnlockQueue
{
public:
	UnlockQueue(unsigned int nSize, QueueType qType = QUEUE_TYPE_COND);
	virtual ~UnlockQueue();

	bool Initialize();

	unsigned int Put(const unsigned char* pBuffer, unsigned int nLen);
	unsigned int Get(unsigned char* pBuffer, unsigned int nLen);

	inline void Clean() { m_nIn = m_nOut = 0; }
	inline unsigned int GetDataLen() const { return  m_nIn - m_nOut; }
	inline unsigned int GetSize() { return m_nSize; }
	inline unsigned int GetRemainDataLen() { return m_nSize - m_nIn + m_nOut; }

private:
	inline bool is_power_of_2(unsigned long n) { return (n != 0 && ((n & (n - 1)) == 0)); };
	inline unsigned long roundup_power_of_two(unsigned long val);

private:
	unsigned char* m_pBuffer;     /* the buffer holding the data */
	unsigned int	m_nSize;      /* the size of the allocated buffer */
	unsigned int	m_nIn;        /* data is added at offset (in % size) */
	unsigned int	m_nOut;       /* data is extracted from off. (out % size) */
	QueueType		m_qType;      /* 队列类型 */
	pthread_cond_t	m_cond;		  /* 条件变量 */
	pthread_mutex_t	m_csLock;     /* 互斥锁，配合条件变量使用 */
};

#endif