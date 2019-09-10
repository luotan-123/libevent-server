#pragma once

#include "DataBase.h"

// 游戏服异步事件  2001-3000

//////////////////////////////////////////////////////////////////////
#define	LOADER_ASYNC_EVENT_UPLOAD_VIDEO		2001		// 上传录像
#define	LOADER_ASYNC_EVENT_HTTP				2002		// http请求

//////////////////////////////////////////////////////////////////////
#pragma pack(1)
// 上传录像
struct LoaderAsyncUploadVideo
{
	DataBaseLineHead dataBaseHead;	//数据包头
	char videoCode[20];

	LoaderAsyncUploadVideo()
	{
		memset(this, 0, sizeof(LoaderAsyncUploadVideo));
	}
};

// HTTP 请求
struct LoaderAsyncHTTP
{
	DataBaseLineHead dataBaseHead;	//数据包头
	char url[1024];
	int userID;
	BYTE postType;

	LoaderAsyncHTTP()
	{
		memset(this, 0, sizeof(LoaderAsyncHTTP));
	}
};

#pragma pack()
//////////////////////////////////////////////////////////////////////