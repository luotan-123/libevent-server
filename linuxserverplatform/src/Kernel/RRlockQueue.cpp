/**
*  @file    RRlockQueue.cpp
*  @brief   环形缓冲区，支持多线程写、单线程读
*  @author  luotan
*  @date    2020-08-06
*/

#include "CommonHead.h"
#include "log.h"
#include "RRlockQueue.h"
#include <assert.h>


RRlockQueue::RRlockQueue(bool bAutoLock, QueueType qType, unsigned int nTimeOnce)
{
	m_pUnLockQueue = nullptr;
	m_pUnLockQueue = new UnlockQueue(MAX_UNLOCKQUEUE_LEN, qType, nTimeOnce);

	assert(m_pUnLockQueue != nullptr);
	assert(m_pUnLockQueue->Initialize());

	m_bAutoLock = bAutoLock;
}

RRlockQueue::~RRlockQueue()
{
	SAFE_DELETE(m_pUnLockQueue);
}

//加入消息队列
/*
Function		:AddData
Memo			:将数据压入到队列当中
Parameter		:
[IN]		pDataInfo	:要压入队列的数据指针
[IN]		uAddSize	:数据大小
[IN]		uDataKind	:数据类型
[IN]		pAppendData	:附加数据，可能是空的
[IN]		pAppendAddSize	:附加数据大小，可以为0，此时实体数据为空
Return			:指压入队列的大小
*/
UINT RRlockQueue::AddData(DataLineHead* pDataInfo, UINT uAddSize, UINT uDataKind, const void* pAppendData, UINT uAppendAddSize)
{
	if (!pDataInfo || uAddSize == 0)
	{
		return 0;
	}

	pDataInfo->uDataKind = uDataKind;
	pDataInfo->uSize = uAddSize;

	if (pAppendData)//如果有附加数据
	{
		pDataInfo->uSize += uAppendAddSize;
	}

	if (pDataInfo->uSize > MAX_SINGLE_UNLOCKQUEUE_SIZE)
	{
		ERROR_LOG("队列中加入数据失败，添加的数据太长 DataSize=%u", pDataInfo->uSize);
		return 0;
	}

	// 加锁
	if (m_bAutoLock)
	{
		CSignedLockObject LockObject(&m_csLock);
	}

	// 拷贝数据
	if (m_pUnLockQueue->Put((const unsigned char*)pDataInfo, uAddSize, (const unsigned char*)pAppendData, uAppendAddSize) == 0)
	{
		ERROR_LOG("队列中加入数据失败，队列空间不足 AllSize=%u DataLength=%u",
			m_pUnLockQueue->GetSize(), m_pUnLockQueue->GetDataLen());
		return 0;
	}

	//返回大小
	return pDataInfo->uSize;
}

//提取消息数据
/*
Function		:GetData
Memo			:从队列中取出数据
Parameter		:
[OUT]			pDataBuffer	:取出数据的缓存
Return			:取出数据的实际大小
*/
UINT RRlockQueue::GetData(DataLineHead** pDataBuffer)
{
	UINT uDataSize = 0;

	UINT uRealSize = m_pUnLockQueue->Get((unsigned char*)&uDataSize, sizeof(uDataSize));

	if (uRealSize == sizeof(uDataSize))
	{
		*((UINT*)(*pDataBuffer)) = uDataSize;
		uRealSize = m_pUnLockQueue->Get((unsigned char*)(*pDataBuffer) + sizeof(uDataSize), uDataSize - sizeof(uDataSize));

		if (uRealSize != uDataSize - sizeof(uDataSize))
		{
			ERROR_LOG("队列中获取数据失败 uRealSize=%u  NeedSize=%u", uRealSize, uDataSize - sizeof(uDataSize));
			return 0;
		}

		return uDataSize;
	}
	else if (uRealSize > 0)
	{
		ERROR_LOG("队列中获取数据失败 uRealSize=%u  NeedSize=%u", uRealSize, sizeof(uDataSize));
		return 0;
	}

	return uRealSize;
}

//清理所有数据
bool RRlockQueue::CleanLineData()
{
	m_pUnLockQueue->Clean();

	return true;
}

// 获取队列数据数量
UINT RRlockQueue::GetDataCount()
{
	return m_pUnLockQueue->GetDataLen();
}
