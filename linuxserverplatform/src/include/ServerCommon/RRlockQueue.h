/**
*  @file    RRlockQueue.h
*  @brief   环形缓冲区，支持多线程写、单线程读
*  @author  luotan
*  @date    2020-08-08
*/

#pragma once

#include "Lock.h"
#include "InternalMessageDefine.h"
#include "UnlockQueue.h"

// 消息队列最大字节数量
#define MAX_UNLOCKQUEUE_LEN				(8 * 1024 * 1024)
// 消息队列最大单包长度
#define MAX_SINGLE_UNLOCKQUEUE_SIZE		(128 * 1024)

class RRlockQueue
{

public:
	RRlockQueue();
	virtual ~RRlockQueue();

public:
	//清理所有数据
	bool CleanLineData();
	//加入消息队列
	virtual UINT AddData(DataLineHead* pDataInfo, UINT uAddSize, UINT uDataKind, const void* pAppendData = NULL, UINT uAppendAddSize = 0);
	//提取消息数据
	virtual UINT GetData(DataLineHead** pDataBuffer);
	//获取队列已经存储的数据大小（字节数）
	UINT GetDataCount();

private:
	CSignedLock	m_csLock;
	UnlockQueue* m_pUnLockQueue;
};