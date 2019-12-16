#include "CommonHead.h"
#include "MysqlHelper.h"
#include "Define.h"
#include "Util.h"
#include "log.h"
#include <stdarg.h>
#include "DataLine.h"
#include "AsyncEventMsg.h"
#include "BillManage.h"


CBillManage::CBillManage()
{
}

CBillManage::~CBillManage()
{
}

CBillManage* CBillManage::Instance()
{
	static CBillManage manage;
	return &manage;
}

void CBillManage::Release()
{
}

void CBillManage::WriteBill(CDataBaseManage* m_pDBManage, const char* pFormat, ...)
{
	if (m_pDBManage == NULL)
	{
		ERROR_LOG("数据句柄m_pDBManage=NULL");
		return;
	}

	char sql[MAX_SQL_STATEMENT_SIZE] = "";

	va_list args;
	va_start(args, pFormat);

	vsprintf(sql, pFormat, args);

	va_end(args);

	AsyncEventMsgSqlStatement msg;
	memcpy(msg.sql, sql, sizeof(sql));

	if (!m_pDBManage->PushLine(&msg.dataLineHead, sizeof(msg) - sizeof(msg.sql) + strlen(msg.sql), ASYNC_EVENT_SQL_STATEMENT, DB_TYPE_LOG, 0))
	{
		ERROR_LOG("投递队列失败：%s", sql);
	}
}