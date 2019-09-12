#pragma once

#include <list>
#include "Lock.h"

#ifndef STRUCT_DATALINEHEAD
#define STRUCT_DATALINEHEAD
//数据队列信息头
struct DataLineHead
{
	UINT						uSize;								//数据大小
	UINT						uDataKind;							//数据类型
};
#endif //STRUCT_DATALINEHEAD
#ifndef STRUCT_DATABASERESULTLINE
#define STRUCT_DATABASERESULTLINE
///异步线程结果消息结构定义
struct AsynThreadResultLine
{
	DataLineHead						LineHead;					///队列头
	UINT								uHandleResult;				///结果结果
	UINT								uHandleKind;				///处理类型
	UINT								uHandleID;					///对象标识
};
#endif //STRUCT_DATABASERESULTLINE

/*
Struct		:ListItemData
Memo		:链表项数据结构
Author		:Fred Huang
Add Data	:2008-3-4
Modify Data	:none
Parameter	:
	stDataHead	:数据包头
	pData		:每个链表项数据的指针，使用用new方式申请的内存，注意，在出队列时，要显式的delete 该内存
*/
struct ListItemData
{
	DataLineHead stDataHead;
	BYTE* pData;
};

//数据队列类
class CDataLine
{
private:
	std::list <ListItemData*> m_DataList;
	CSignedLock		m_csLock;				//同步锁

	//HANDLE			m_hCompletionPort
public:
	CDataLine();
	virtual ~CDataLine();

public:
	//获取锁
	CSignedLock* GetLock() { return &m_csLock; }
	//清理所有数据
	bool CleanLineData();
	//加入消息队列
	virtual UINT AddData(DataLineHead* pDataInfo, UINT uAddSize, UINT uDataKind, void* pAppendData = NULL, UINT uAppendAddSize = 0);
	//提取消息数据
	virtual UINT GetData(DataLineHead* pDataBuffer, UINT uBufferSize);

public:
	size_t GetDataCount();
};

