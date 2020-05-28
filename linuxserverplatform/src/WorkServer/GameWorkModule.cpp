#include "CommonHead.h"
#include "GameWorkModule.h"


CGameWorkModule::CGameWorkModule()
{

}

CGameWorkModule::~CGameWorkModule()
{

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

	ret = m_WorkManage.Init(pInitData, &m_DataBaseHandle);
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
		return m_WorkManage.UnInit();
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

	if (!m_WorkManage.Start())
	{
		return false;
	}

	INFO_LOG("GameWorkModule StartService end");

	return true;
}

bool CGameWorkModule::StoptService()
{
	INFO_LOG("GameWorkModule StoptService begin...");

	bool ret = m_WorkManage.Stop();
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