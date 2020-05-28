#pragma once

#include "GameWorkManage.h"
#include "GameWorkDataBase.h"

//游戏登陆模块
class CGameWorkModule : public IModuleManageService
{
public:
	CGameWorkManage		m_WorkManage;				//登陆管理
	CServiceDataBaseHandle	m_DataBaseHandle;			//数据库数据处理模块

public:
	CGameWorkModule();
	virtual ~CGameWorkModule();

public:
	virtual bool InitService(ManageInfoStruct * pInitData);
	virtual bool UnInitService();
	virtual bool StartService(UINT &errCode);
	virtual bool StoptService();
	virtual bool DeleteService();
	virtual bool UpdateService();
};
