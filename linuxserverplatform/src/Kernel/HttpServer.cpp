#include "CommonHead.h"
#include "configManage.h"
#include "log.h"
#include "HttpServer.h"


CHttpServer::CHttpServer()
{
	

}

CHttpServer::~CHttpServer()
{
	
}

//开始服务
bool CHttpServer::Start()
{
	INFO_LOG("HttpServer start begin ...");

	if (m_bRun == true)
	{
		ERROR_LOG("HttpServer already running or not inited m_bRun=%d", m_bRun);
		return false;
	}

	m_bRun = true;

	

	INFO_LOG("HttpServer start end.");

	return true;
}

bool CHttpServer::Stop()
{
	INFO_LOG("HttpServer stop begin...");

	if (!m_bRun)
	{
		ERROR_LOG("HttpServer is not running...");
		return false;
	}

	m_bRun = false;

	// 清理dataline
	m_DataLine.CleanLineData();

	



	INFO_LOG("HttpServer stop end.");

	return true;
}



