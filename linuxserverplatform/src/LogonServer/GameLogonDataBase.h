#pragma once

#include "DataBase.h"

//数据库处理者
class CServiceDataBaseHandle : public CDataBaseHandle
{
public:
	CServiceDataBaseHandle();
	virtual ~CServiceDataBaseHandle();

private:
	virtual UINT HandleDataBase(DataBaseLineHead * pSourceData);

private:
	// 执行sql语句
	int OnHandleExecuteSQLStatement(DataBaseLineHead * pSourceData);
	// HTTP请求
	int OnHandleHTTP(DataBaseLineHead * pSourceData);

private:
	time_t m_lastCheckDBConnectionTime;
};