#pragma once


class CGameWorkManage;
class CServiceDataBaseHandle;

//逻辑服务器模块，单例模式
class CGameWorkModule : public IModuleManageService
{
public:
	CGameWorkManage* m_pWorkManage;				//登陆管理
	CServiceDataBaseHandle* m_pDataBaseHandle;	//数据库数据处理模块
	static CGameWorkModule* g_pGameWorkModule;  //全局唯一指针

private:
	CGameWorkModule();
public:
	static CGameWorkModule* Instance();
	virtual ~CGameWorkModule();

public:
	virtual bool InitService(ManageInfoStruct * pInitData);
	virtual bool UnInitService();
	virtual bool StartService(UINT &errCode);
	virtual bool StoptService();
	virtual bool DeleteService();
	virtual bool UpdateService();
};

#define WorkServerModule()		CGameWorkModule::Instance()