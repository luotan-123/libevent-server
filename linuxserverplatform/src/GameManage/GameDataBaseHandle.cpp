#include "CommonHead.h"
#include "log.h"
#include "configManage.h"
#include "tableDefine.h"
#include "MyCurl.h"
#include "AsyncEventMsg.pb.h"
#include "GameDataBaseHandle.h"

using namespace AsyncEventMsg;

CGameDataBaseHandle::CGameDataBaseHandle()
{

}

CGameDataBaseHandle::~CGameDataBaseHandle()
{
}

UINT CGameDataBaseHandle::HandleDataBase(DataBaseLineHead* pSourceData)
{
	if (!pSourceData)
	{
		return 0;
	}

	switch (pSourceData->uHandleKind)
	{
	case ASYNC_EVENT_UPLOAD_VIDEO:
	{
		return OnUploadVideo(pSourceData);
	}
	case ASYNC_EVENT_SQL_STATEMENT:			// 处理sql语句
	{
		return OnHandleExecuteSQLStatement(pSourceData);
	}
	case ASYNC_EVENT_HTTP:
	{
		return OnHandleHTTP(pSourceData);
	}
	default:
		break;
	}

	return 0;
}

int CGameDataBaseHandle::OnUploadVideo(DataBaseLineHead* pSourceData)
{
	if (pSourceData->dataLineHead.uSize != sizeof(AsyncEventMsgUploadVideo))
	{
		ERROR_LOG("size is not match realsize=%d expect=%d", pSourceData->dataLineHead.uSize, sizeof(AsyncEventMsgUploadVideo));
		return -1;
	}

	AsyncEventMsgUploadVideo* pAsyncMessage = (AsyncEventMsgUploadVideo*)pSourceData;
	if (!pAsyncMessage)
	{
		ERROR_LOG("asyncMessage is NULL");
		return -2;
	}

	//AUTOCOST("上传录像json耗时");

	if (!m_pInitInfo)
	{
		ERROR_LOG("m_pInitInfo is NULL");
		return -3;
	}

	int gameID = m_pInitInfo->uNameID;

	MyCurl curl;
	curl.uploadUrl(pAsyncMessage->videoCode, gameID, 0, 3);

	return 0;
}

// 执行sql语句
int CGameDataBaseHandle::OnHandleExecuteSQLStatement(DataBaseLineHead* pSourceData)
{
	//AUTOCOST("执行SQL语句耗时");

	AsyncEventMsgSqlStatement* pMessage = (AsyncEventMsgSqlStatement*)pSourceData;
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

	unsigned int index = pMessage->dataLineHead.uIndex;

	if (index >= DB_TYPE_PHP)
	{
		ERROR_LOG("invalid hDatabase");
		return -3;
	}

	if (pMessage->bIsSelect)
	{
		CMysqlHelper::MysqlData dataSet;
		try
		{
			m_pDataBaseManage->m_pMysqlHelper[index].queryRecord(pMessage->sql, dataSet, true);
		}
		catch (MysqlHelper_Exception & excep)
		{
			ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
			return -4;
		}

		switch (pMessage->dataLineHead.uMsgID)
		{
		case ASYNC_MESSAGE_DATABASE_TEST:
		{
			AsyncEventMsg_Test allUser;
			for (size_t i = 0; i < dataSet.size(); i++)
			{
				AsyncEventMsg_Test_User* pUser = allUser.add_user(); // 添加repeated成员
				UserData userData;

				CConfigManage::sqlGetValue(dataSet[i], "userID", userData.userID);
				CConfigManage::sqlGetValue(dataSet[i], "name", userData.name, sizeof(userData.name));
				CConfigManage::sqlGetValue(dataSet[i], "headURL", userData.headURL, sizeof(userData.headURL));
				CConfigManage::sqlGetValue(dataSet[i], "money", userData.money);
				CConfigManage::sqlGetValue(dataSet[i], "winCount", userData.winCount);

				pUser->set_userid(userData.userID);
				pUser->set_name(userData.name);
				pUser->set_headurl(userData.headURL);
				pUser->set_money(userData.money);
				pUser->set_wincount(userData.winCount);
			}

			std::string result = "";
			allUser.SerializeToString(&result);

			//返回结果
			m_pRusultService->OnAsynThreadResultEvent(ASYNC_EVENT_SQL_STATEMENT, 0, result.c_str(),
				result.size(), pMessage->dataLineHead.uIndex, pMessage->dataLineHead.uMsgID);
		}
		break;
		}
	}
	else
	{
		try
		{
			m_pDataBaseManage->m_pMysqlHelper[index].sqlExec(pMessage->sql, true);
		}
		catch (MysqlHelper_Exception & excep)
		{
			ERROR_LOG("执行sql语句失败==>>%s", excep.errorInfo.c_str());
			return -3;
		}
	}

	return 0;
}

// HTTP请求
int CGameDataBaseHandle::OnHandleHTTP(DataBaseLineHead* pSourceData)
{
	if (pSourceData->dataLineHead.uSize != sizeof(AsyncEventMsgHTTP))
	{
		ERROR_LOG("size is not match realsize=%d expect=%d", pSourceData->dataLineHead.uSize, sizeof(AsyncEventMsgHTTP));
		return -1;
	}

	AsyncEventMsgHTTP* pAsyncMessage = (AsyncEventMsgHTTP*)pSourceData;
	if (!pAsyncMessage)
	{
		ERROR_LOG("asyncMessage is NULL");
		return -2;
	}

	//AUTOCOST("HTTP请求");

	//发送邮件接口
	MyCurl curl;
	std::vector<std::string> vUrlHeader;
	std::string postFields = "";
	std::string result = "";

	//填充固定头部
	//todo：根据具体业务可以改，也可以不填充头部
	if (pAsyncMessage->dataLineHead.uMsgID == HTTP_POST_TYPE_REQ_DATA)
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
		ERROR_LOG("HTTP请求失败：url=[%s],userID=[%d],ret=[%s]", pAsyncMessage->url, pAsyncMessage->dataLineHead.uIndex, result.c_str());
		return -2;
	}

	//返回结果
	m_pRusultService->OnAsynThreadResultEvent(ASYNC_EVENT_HTTP, 0, result.c_str(),
		result.size(), pAsyncMessage->dataLineHead.uIndex, pAsyncMessage->dataLineHead.uMsgID);

	return 0;
}