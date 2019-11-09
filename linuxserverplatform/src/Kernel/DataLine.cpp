#include "CommonHead.h"
#include "DataLine.h"
#include "log.h"

CDataLine::CDataLine()
{
	m_DataListSize = 0;
	m_DataList.clear();
}

CDataLine::~CDataLine()
{
	ListItemData* pListItem = NULL;
	while (m_DataList.size() > 0)
	{
		pListItem = m_DataList.front();
		m_DataList.pop_front();
		free(pListItem->pData);
		delete pListItem;
	}
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
UINT CDataLine::AddData(DataLineHead* pDataInfo, UINT uAddSize, UINT uDataKind, const void* pAppendData, UINT uAppendAddSize)
{
	if (!pDataInfo || uAddSize == 0)
	{
		return 0;
	}

	if (m_DataListSize > MAX_DATALINE_LEN)
	{
		ERROR_LOG("队列已满(%d)\n", MAX_DATALINE_LEN);
		return 0;
	}

	CSignedLockObject LockObject(&m_csLock, false);

	ListItemData* pListItem = new ListItemData;			//创建一个队列项

	pListItem->pData = NULL;							//先设为0，以保证后续不出错
	pListItem->stDataHead.uSize = uAddSize;				//数据大小
	pListItem->stDataHead.uDataKind = uDataKind;		//数据类型
	if (pAppendData)									//如果有附加数据
	{
		pListItem->stDataHead.uSize += uAppendAddSize;
	}

	pListItem->pData = (BYTE*)malloc(pListItem->stDataHead.uSize + 1);	//申请数据项内存
	pListItem->pData[pListItem->stDataHead.uSize] = 0;					//初始化末尾

	pDataInfo->uDataKind = uDataKind;
	pDataInfo->uSize = pListItem->stDataHead.uSize;

	memcpy(pListItem->pData, pDataInfo, uAddSize);					//复制实体数据
	if (pAppendData != NULL)										//如果有附加数据，复制在实体数据后面
	{
		memcpy(pListItem->pData + uAddSize, pAppendData, uAppendAddSize);
	}

	// 加锁
	LockObject.Lock();

	m_DataList.push_back(pListItem);								//加到队列尾部

	m_DataListSize++;

	LockObject.UnLock();

	// 通知条件变量
	m_csLock.Notify();

	return pListItem->stDataHead.uSize;		//返回大小
}

//提取消息数据
/*
Function		:GetData
Memo			:从队列中取出数据
Parameter		:
[OUT]		pDataBuffer	:取出数据的缓存
Return			:取出数据的实际大小
*/
UINT CDataLine::GetData(DataLineHead** pDataBuffer)
{
	*pDataBuffer = NULL;

	CSignedLockObject LockObject(&m_csLock, false);

	LockObject.Lock();

	//进入挂起状态
	if (m_DataListSize <= 0)
	{
		m_csLock.Wait();
	}

	//如果队列是空的，直接返回
	if (m_DataListSize <= 0)
	{
		return 0;
	}

	//取数据
	ListItemData* pListItem = m_DataList.front();
	m_DataList.pop_front();

	m_DataListSize--;

	LockObject.UnLock();

	UINT uDataSize = pListItem->stDataHead.uSize;

	//投递数据，外部一定要释放内存，否则内存泄漏
	*pDataBuffer = (DataLineHead*)pListItem->pData;

	//删除队列中的数据
	delete pListItem;

	return uDataSize;
}

//清理所有数据
bool CDataLine::CleanLineData()
{
	CSignedLockObject LockObject(&m_csLock, true);

	ListItemData* pListItem = NULL;

	while (m_DataList.size() > 0)
	{
		pListItem = m_DataList.front();
		m_DataList.pop_front();
		free(pListItem->pData);
		delete pListItem;
	}
	m_DataListSize = 0;

	return true;
}

// 获取队列数据数量
size_t CDataLine::GetDataCount()
{
	return m_DataListSize;
}
