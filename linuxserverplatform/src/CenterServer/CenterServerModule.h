#pragma once

#include "CenterServerManage.h"

//游戏登陆模块
class CCenterServerModule : public IModuleManageService
{
public:
	CCenterServerManage		m_CenterServerManage;				//登陆管理

public:
	CCenterServerModule();
	virtual ~CCenterServerModule();

public:
	virtual bool InitService(ManageInfoStruct* pInitData);
	virtual bool UnInitService();
	virtual bool StartService(UINT& errCode);
	virtual bool StoptService();
	virtual bool DeleteService();
	virtual bool UpdateService();
};
