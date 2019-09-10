#pragma once

#include "GameLogonManage.h"
#include "GameLogonDataBase.h"

//游戏登陆模块
class CGameLogonModule : public IModuleManageService
{
public:
	CGameLogonManage		m_LogonManage;				//登陆管理
	CServiceDataBaseHandle	m_DataBaseHandle;			//数据库数据处理模块

public:
	CGameLogonModule();
	virtual ~CGameLogonModule();

public:
	virtual bool InitService(ManageInfoStruct * pInitData);
	virtual bool UnInitService();
	virtual bool StartService(UINT &errCode);
	virtual bool StoptService();
	virtual bool DeleteService();
	virtual bool UpdateService();
};
