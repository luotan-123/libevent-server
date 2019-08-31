#pragma once

#include "DataBase.h"

//游戏数据库处理
class KERNEL_CLASS CGameDataBaseHandle : public CDataBaseHandle
{
public:
	CGameDataBaseHandle();
	virtual ~CGameDataBaseHandle();

private:
	virtual UINT HandleDataBase(DataBaseLineHead * pSourceData);

private:
	// 上传录像
	int OnUploadVideo(DataBaseLineHead* pSourceData);
	// 执行sql语句
	int OnHandleExecuteSQLStatement(DataBaseLineHead * pSourceData);
	// HTTP请求
	int OnHandleHTTP(DataBaseLineHead * pSourceData);

public:
	int m_ErrorSQLCount;
	time_t m_lastCheckDBConnectionTime;
};