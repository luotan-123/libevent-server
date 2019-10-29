#pragma once

#include "DataBase.h"

// 游戏服异步事件  2001-3000

//////////////////////////////////////////////////////////////////////
#define	ASYNC_EVENT_SQL_STATEMENT	2001		// 执行sql语句
#define	ASYNC_EVENT_UPLOAD_VIDEO	2002		// 上传录像
#define	ASYNC_EVENT_HTTP			2003		// http请求
#define	ASYNC_EVENT_LOG				2004		// 写入日志

//////////////////////////////////////////////////////////////////////
#pragma pack(1)

// 执行数据库语句
struct AsyncEventMsgSqlStatement
{
	DataBaseLineHead	dataLineHead;
	bool	bIsSelect;	// 默认0：非查询性语句，错误返回字符串，1：查询语句，需要返回结果集合
	char	sql[MAX_SQL_STATEMENT_SIZE];

	AsyncEventMsgSqlStatement()
	{
		memset(this, 0, sizeof(AsyncEventMsgSqlStatement));
	}
};

// 上传录像
struct AsyncEventMsgUploadVideo
{
	DataBaseLineHead dataLineHead;	//数据包头
	char videoCode[48];

	AsyncEventMsgUploadVideo()
	{
		memset(this, 0, sizeof(AsyncEventMsgUploadVideo));
	}
};

// HTTP 请求
struct AsyncEventMsgHTTP
{
	DataBaseLineHead dataLineHead;	//数据包头
	char url[1024];

	AsyncEventMsgHTTP()
	{
		memset(this, 0, sizeof(AsyncEventMsgHTTP));
	}
};

#pragma pack()
//////////////////////////////////////////////////////////////////////
// 数据库消息id
const int ASYNC_MESSAGE_DATABASE_NO = 0;
const int ASYNC_MESSAGE_DATABASE_TEST = 1;


//////////////////////////////////////////////////////////////////////
// http 请求id
const int	HTTP_POST_TYPE_REGISTER = 1;	//注册
const int	HTTP_POST_TYPE_LOGON = 2;		//登陆
const int	HTTP_POST_TYPE_LOGOUT = 3;		//登出
const int	HTTP_POST_TYPE_REQ_DATA = 4;	//获取页面数据
const int	HTTP_POST_TYPE_MATCH_GIFT = 5;	//比赛奖励
const int	HTTP_POST_TYPE_MATCH_FAIL = 6;	//比赛失败

//////////////////////////////////////////////////////////////////////