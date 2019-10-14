#include "CommonHead.h"
#include "CenterServerModule.h"


CCenterServerModule::CCenterServerModule()
{

}

CCenterServerModule::~CCenterServerModule()
{

}

bool CCenterServerModule::InitService(ManageInfoStruct * pInitData)
{
	INFO_LOG("CenterServerModule InitService begin ...");
	if (!pInitData)
	{
		ERROR_LOG("invalid param input");
		return false;
	}

	bool ret = false;

	ret = m_CenterServerManage.Init(pInitData);
	if (!ret)
	{
		ERROR_LOG("CenterServerManage Init failed");
		return false;
	}

	INFO_LOG("CenterServerModule InitService end.");

	return true;
}

bool CCenterServerModule::UnInitService()
{
	try
	{
		return m_CenterServerManage.UnInit();
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

bool CCenterServerModule::StartService(UINT &errCode)
{
	INFO_LOG("CenterServerModule StartService begin...");

	errCode = 0;

	if (!m_CenterServerManage.Start())
	{
		return false;
	}

	INFO_LOG("CenterServerModule StartService end");

	return true;
}

bool CCenterServerModule::StoptService()
{
	INFO_LOG("CenterServerModule StoptService begin...");

	bool ret = m_CenterServerManage.Stop();
	if (!ret)
	{
		ERROR_LOG("LogonManage Stop failed");
		return false;
	}

	INFO_LOG("CenterServerModule StoptService end");

	return true;
}

bool CCenterServerModule::DeleteService()
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

bool CCenterServerModule::UpdateService()
{
	return true;
}