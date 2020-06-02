#pragma once

#include "comstruct.h"
#include "Interface.h"
#include "Lock.h"
#include "DataLine.h"
#include "Exception.h"

// 数据库处理类
class CHttpServer
{
public:
	KernelInfoStruct* m_pKernelInfo;// 内核数据
	ManageInfoStruct* m_pInitInfo;	// 初始化数据指针
	CDataLine	m_DataLine;			// 数据队列
	
protected:
	pthread_t	m_hThread;		// 线程句柄
	bool		m_bRun;			// 运行标志
	IDataBaseHandleService* m_pHandleService;	// 数据处理接口

public:
	CHttpServer();
	virtual ~CHttpServer();

public:
	//开始服务
	bool Start();
	//停止服务
	bool Stop();
};