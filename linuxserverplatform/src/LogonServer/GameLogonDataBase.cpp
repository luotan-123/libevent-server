#include "main.h"
#include "GameLogonDataBase.h"
#include "InternalMessageDefine.h"
#include "log.h"
#include "Util.h"
#include "configManage.h"
#include "LoaderAsyncEvent.h"
#include "MyCurl.h"
#include "Define.h"

CServiceDataBaseHandle::CServiceDataBaseHandle()
{
	m_lastCheckDBConnectionTime = 0;
}

CServiceDataBaseHandle::~CServiceDataBaseHandle()
{
}

UINT CServiceDataBaseHandle::HandleDataBase(DataBaseLineHead * pSourceData)
{
	if (!pSourceData)
	{
		ERROR_LOG("invalid pSourceData");
		return -1;
	}

	time_t currTime = time(NULL);
	if (m_pDataBaseManage && currTime - m_lastCheckDBConnectionTime >= 60)		// 60 TODO
	{
		m_lastCheckDBConnectionTime = currTime;
		m_pDataBaseManage->CheckSQLConnect();
	}

	switch (pSourceData->uHandleKind)
	{
	case DTK_GP_SQL_STATEMENT:			// 处理sql语句
	{
		return OnHandleExecuteSQLStatement(pSourceData);
	}
	case LOADER_ASYNC_EVENT_HTTP:
	{
		return OnHandleHTTP(pSourceData);
	}
	default:
		break;
	}

	return 0;
}

// 执行sql语句
int CServiceDataBaseHandle::OnHandleExecuteSQLStatement(DataBaseLineHead * pSourceData)
{
	AUTOCOST("执行SQL语句耗时");

	InternalSqlStatement* pMessage = (InternalSqlStatement*)pSourceData;
	if (!pMessage)
	{
		ERROR_LOG("pMessage is NULL");
		return -1;
	}

	if (!m_pDataBaseManage || !m_pDataBaseManage->m_pMysqlHelper)
	{
		ERROR_LOG("invalid hDatabase");
		return -2;
	}

	try
	{
		m_pDataBaseManage->m_pMysqlHelper->sqlExec(pMessage->sql, true);
	}
	catch(MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败==>>%s",excep.errorInfo) ;
		return -3;
	}
	
	return 0;
}

// HTTP请求
int CServiceDataBaseHandle::OnHandleHTTP(DataBaseLineHead * pSourceData)
{
	if (pSourceData->DataLineHead.uSize != sizeof(LoaderAsyncHTTP))
	{
		ERROR_LOG("size is not match realsize=%d expect=%d", pSourceData->DataLineHead.uSize, sizeof(LoaderAsyncHTTP));
		return -1;
	}

	LoaderAsyncHTTP* pAsyncMessage = (LoaderAsyncHTTP*)pSourceData;
	if (!pAsyncMessage)
	{
		ERROR_LOG("asyncMessage is NULL");
		return -2;
	}

	AUTOCOST("HTTP请求: userID=%d", pAsyncMessage->userID);

	//发送邮件接口
	MyCurl curl;
	std::vector<std::string> vUrlHeader;
	std::string postFields = "";
	std::string result = "";

	//填充固定头部
	//todo：根据具体业务可以改，也可以不填充头部
	if (pAsyncMessage->postType == HTTP_POST_TYPE_REQ_DATA)
	{
		vUrlHeader.push_back("Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
		vUrlHeader.push_back("Accept-Encoding:gzip, deflate, br");
		vUrlHeader.push_back("Accept-Language:zh-CN,zh;q=0.8");
		vUrlHeader.push_back("Cache-Control:max-age=0");
		vUrlHeader.push_back("Connection:keep-alive");
		vUrlHeader.push_back("Cookie:ccsalt=436fa91b51b76de36af8a7dc7002679c");
		vUrlHeader.push_back("Host:www.1392p.com");
		vUrlHeader.push_back("Upgrade-Insecure-Requests:1");
		vUrlHeader.push_back("User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36");
	}

	//组合生成URL
	std::string url = pAsyncMessage->url;

	curl.postUrlHttps(url, vUrlHeader, postFields, result);
	if (result.size() <= 0)
	{
		ERROR_LOG("HTTP请求失败：url=[%s],userID=[%d],ret=[%s]", pAsyncMessage->url, pAsyncMessage->userID, result.c_str());
		return -2;
	}
	
	if (result.size() < LD_MAX_PART - 1)
	{
		//返回结果
		char szBuffer[LD_MAX_PART] = "";
		memcpy(szBuffer, result.c_str(), min(result.size(), LD_MAX_PART - 1));

		szBuffer[LD_MAX_PART - 1] = 0;

		m_pRusultService->OnAsynThreadResultEvent(ANSY_THREAD_RESULT_TYPE_HTTP, 0, szBuffer,
			result.size() + 1, pAsyncMessage->postType, pAsyncMessage->userID);
	}
	else
	{
		ERROR_LOG("http请求返回数据过长 size=%d", result.size());
	}
	
	return 0;
}