#include "main.h"
#include "GameLogonModule.h"


CGameLogonModule::CGameLogonModule()
{

}

CGameLogonModule::~CGameLogonModule()
{

}

bool CGameLogonModule::InitService(ManageInfoStruct * pInitData)
{
	INFO_LOG("GameLogonModule InitService begin ...");
	if (!pInitData)
	{
		ERROR_LOG("invalid param input");
		return false;
	}

	bool ret = false;

	ret = m_LogonManage.Init(pInitData, &m_DataBaseHandle);
	if (!ret)
	{
		ERROR_LOG("LogonManage Init failed");
		return false;
	}

	INFO_LOG("GameLogonModule InitService end.");

	return true;
}

bool CGameLogonModule::UnInitService()
{
	try
	{
		return m_LogonManage.UnInit();
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

bool CGameLogonModule::StartService(UINT &errCode)
{
	INFO_LOG("GameLogonModule StartService begin...");
	
	errCode = 0;

	if (!m_LogonManage.Start())
	{
		return false;
	}

	INFO_LOG("GameLogonModule StartService end");

	return true;
}

bool CGameLogonModule::StoptService()
{
	INFO_LOG("GameLogonModule StoptService begin...");

	bool ret = m_LogonManage.Stop();
	if (!ret)
	{
		ERROR_LOG("LogonManage Stop failed");
		return false;
	}

	INFO_LOG("GameLogonModule StoptService end");

	return true;
}

bool CGameLogonModule::DeleteService()
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

bool CGameLogonModule::UpdateService()
{
	return true;
}