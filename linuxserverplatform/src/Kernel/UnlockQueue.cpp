/**
*  @file    UnlockQueue.cpp
*  @brief   并发无锁队列。只适用于：单线程读、单线程写
*  @author  luotan
*  @date    2020-08-06
*/

#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include "UnlockQueue.h"

#define QUEUE_TIME_ONCE		2000000	// 单位：纳秒

UnlockQueue::UnlockQueue(unsigned int nSize, QueueType qType)
	: m_pBuffer(NULL)
	, m_nSize(nSize)
	, m_nIn(0)
	, m_nOut(0)
	, m_qType(qType)
{
	//round up to the next power of 2
	if (!is_power_of_2(nSize))
	{
		m_nSize = (unsigned int)roundup_power_of_two(nSize);
	}

	if (qType == QUEUE_TYPE_COND)
	{
		// 初始化锁
		pthread_mutex_init(&m_csLock, NULL);

		// 初始化条件变量
		pthread_cond_init(&m_cond, NULL);
	}
}

UnlockQueue::~UnlockQueue()
{
	if (NULL != m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

	if (m_qType == QUEUE_TYPE_COND)
	{
		pthread_mutex_destroy(&m_csLock);

		pthread_cond_destroy(&m_cond);
	}
}

bool UnlockQueue::Initialize()
{
	m_pBuffer = new unsigned char[m_nSize];
	if (!m_pBuffer)
	{
		return false;
	}

	m_nIn = m_nOut = 0;

	return true;
}

unsigned long UnlockQueue::roundup_power_of_two(unsigned long val)
{
	if ((val & (val - 1)) == 0)
		return val;

	unsigned long maxulong = (unsigned long)((unsigned long)~0);
	unsigned long andv = ~(maxulong & (maxulong >> 1));
	while ((andv & val) == 0)
		andv = andv >> 1;

	return andv << 1;
}


unsigned int UnlockQueue::Put(const unsigned char* buffer, unsigned int len)
{
	unsigned int l = 0;

	// 超过剩余空间返回失败0
	if (buffer == NULL || len == 0 || len > m_nSize - m_nIn + m_nOut)
	{
		return 0;
	}
	//len = std::min(len, m_nSize - m_nIn + m_nOut);

	/*
	* Ensure that we sample the m_nOut index -before- we
	* start putting bytes into the UnlockQueue.
   */
	__sync_synchronize();

	/* first put the data starting from fifo->in to buffer end */
	l = std::min(len, m_nSize - (m_nIn & (m_nSize - 1)));
	memcpy(m_pBuffer + (m_nIn & (m_nSize - 1)), buffer, l);

	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(m_pBuffer, buffer + l, len - l);

	/*
	* Ensure that we add the bytes to the kfifo -before-
	* we update the fifo->in index.
	*/
	__sync_synchronize();

	m_nIn += len;

	// 通知条件变量，一般这个时候，读线程还不能够立马获取到值
	// 不建议使用volatile，频繁地使用volatile很可能会增加代码尺寸和降低性能,因此要合理的使用volatile
	if (m_qType == QUEUE_TYPE_COND)
	{
		pthread_cond_signal(&m_cond);
	}

	return len;
}

unsigned int UnlockQueue::Get(unsigned char* buffer, unsigned int len)
{
	unsigned int l = 0;

	if (buffer == NULL || len == 0)
	{
		return 0;
	}

	len = std::min(len, m_nIn - m_nOut);

	if (len == 0)
	{
		if (m_qType == QUEUE_TYPE_COND)
		{
			pthread_cond_wait(&m_cond, &m_csLock);
		}
		else if (m_qType == QUEUE_TYPE_SLEEP)
		{
			struct timespec slptm;
			slptm.tv_sec = 0;
			slptm.tv_nsec = QUEUE_TIME_ONCE;      //1000 ns = 1 us
			nanosleep(&slptm, NULL);
		}
		return 0;
	}

	/*
	* Ensure that we sample the fifo->in index -before- we
	* start removing bytes from the kfifo.
	*/
	__sync_synchronize();

	/* first get the data from fifo->out until the end of the buffer */
	l = std::min(len, m_nSize - (m_nOut & (m_nSize - 1)));
	memcpy(buffer, m_pBuffer + (m_nOut & (m_nSize - 1)), l);

	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buffer + l, m_pBuffer, len - l);

	/*
	* Ensure that we remove the bytes from the kfifo -before-
	* we update the fifo->out index.
	*/
	__sync_synchronize();

	m_nOut += len;

	return len;
}