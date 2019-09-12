#pragma once

#include "comstruct.h"
#include "Interface.h"
#include "Lock.h"
#include "DataLine.h"
#include "Exception.h"
#include "MysqlHelper.h"

#pragma warning(disable:4146)
#pragma warning(default:4146)

class CDataLine;
class CDataBaseManage;

#ifndef STRUCT_DATABASELINEHEAD
#define STRUCT_DATABASELINEHEAD

//数据库数据包头结构
struct DataBaseLineHead
{
	DataLineHead					DataLineHead;							///队列头
	UINT							uHandleKind;							///处理类型
	UINT							uIndex;									///对象索引
	DWORD							dwHandleID;								///对象标识
};
#endif

// 数据库处理类
class KERNEL_CLASS CDataBaseManage
{
public:
	KernelInfoStruct* m_pKernelInfo;	// 内核数据
	ManageInfoStruct* m_pInitInfo;		// 初始化数据指针
	CDataLine	m_DataLine;				// 数据队列
	CMysqlHelper * m_pMysqlHelper;		// 数据库模块
protected:
	HANDLE	m_hThread;			// 线程句柄
	HANDLE	m_hCompletePort;	// 完成端口
	bool	m_bInit;			// 初始化标志
	bool	m_bRun;				// 运行标志
	IDataBaseHandleService* m_pHandleService;	// 数据处理接口

public:
	CDataBaseManage();
	virtual ~CDataBaseManage();

public:
	//初始化函数
	bool Init(ManageInfoStruct* pInitInfo, KernelInfoStruct * pKernelInfo, IDataBaseHandleService * pHandleService, IAsynThreadResultService * pResultService);
	//取消初始化
	bool UnInit();
	//开始服务
	bool Start();
	//停止服务
	bool Stop();
	//加入处理队列
	bool PushLine(DataBaseLineHead* pData, UINT uSize, UINT uHandleKind, UINT uIndex, DWORD dwHandleID);

public:
	//检测数据连接
	bool CheckSQLConnect();
	//重联数据库
	bool SQLConnectReset();

	int		m_sqlClass;

	SHORT	m_nPort;
	BOOL    m_bsqlInit;

	char	m_host[128];
	char	m_passwd[48];
	char	m_user[48];
	char	m_name[48];

private:
	//数据库处理线程
	static unsigned __stdcall DataServiceThread(LPVOID pThreadData);
};

///***********************************************************************************************///
//数据库处理接口类
class KERNEL_CLASS CDataBaseHandle : public IDataBaseHandleService
{
public:
	CDataBaseHandle();
	virtual ~CDataBaseHandle();

protected:
	KernelInfoStruct* m_pKernelInfo;			//内核数据
	ManageInfoStruct* m_pInitInfo;				//初始化数据指针
	IAsynThreadResultService* m_pRusultService;	//结果处理接口
	CDataBaseManage*	m_pDataBaseManage;		//数据库对象

public:
	//设置参数
	virtual bool SetParameter(IAsynThreadResultService * pRusultService, CDataBaseManage * pDataBaseManage, ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);
};