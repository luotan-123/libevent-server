#include "CommonHead.h"
#include "GameWorkManage.h"
#include "GameWorkDataBase.h"
#include "GameWorkModule.h"

CGameWorkModule* CGameWorkModule::g_pGameWorkModule = nullptr;

CGameWorkModule::CGameWorkModule()
{
	m_pWorkManage = new CGameWorkManage();
	m_pDataBaseHandle = new CServiceDataBaseHandle();
}

CGameWorkModule* CGameWorkModule::Instance()
{
	if (g_pGameWorkModule)
	{
		return g_pGameWorkModule;
	}

	g_pGameWorkModule = new CGameWorkModule();

	return g_pGameWorkModule;
}

CGameWorkModule::~CGameWorkModule()
{
	SafeDelete(m_pWorkManage);
	SafeDelete(m_pDataBaseHandle);
}

bool CGameWorkModule::InitService(ManageInfoStruct * pInitData)
{
	INFO_LOG("GameWorkModule InitService begin ...");
	if (!pInitData)
	{
		ERROR_LOG("invalid param input");
		return false;
	}

	bool ret = false;

	ret = m_pWorkManage->Init(pInitData, m_pDataBaseHandle);
	if (!ret)
	{
		ERROR_LOG("WorkManage Init failed");
		return false;
	}

	INFO_LOG("GameWorkModule InitService end.");

	return true;
}

bool CGameWorkModule::UnInitService()
{
	try
	{
		return m_pWorkManage->UnInit();
	}
	catch (CException * pException) 
	{
		pException->Delete();
		ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__); 
	}
	catch (...) 
	{
		ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
	}

	return false;
}

bool CGameWorkModule::StartService(UINT &errCode)
{
	INFO_LOG("GameWorkModule StartService begin...");
	
	errCode = 0;

	if (!m_pWorkManage->Start())
	{
		return false;
	}

	INFO_LOG("GameWorkModule StartService end");

	return true;
}

bool CGameWorkModule::StoptService()
{
	INFO_LOG("GameWorkModule StoptService begin...");

	bool ret = m_pWorkManage->Stop();
	if (!ret)
	{
		ERROR_LOG("WorkManage Stop failed");
		return false;
	}

	INFO_LOG("GameWorkModule StoptService end");

	return true;
}

bool CGameWorkModule::DeleteService()
{
	try
	{
		delete this;
	}
	catch (...)
	{
		ERROR_LOG("CATCH:%s with %s\n", __FILE__, __FUNCTION__);
	}

	return true;
}

bool CGameWorkModule::UpdateService()
{
	return true;
}