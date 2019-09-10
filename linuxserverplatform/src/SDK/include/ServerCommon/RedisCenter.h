#pragma once

#include "RedisCommon.h"

class KERNEL_CLASS CRedisCenter : public CRedisCommon
{
public:
	CRedisCenter();
	virtual ~CRedisCenter();

public:
	// 初始化
	virtual bool Init();
	// 关闭
	virtual bool Stop();

public:
	// bUnBind表示是否解绑
	bool SetUserPhone(int userID, const char* phone, bool bUnBind = false);

	// 闲聊绑定
	bool SetUserXianLiao(int userID, const char* xianliao);

	// 设置游戏服状态
	bool SetRoomServerStatus(int roomID, int status);

	// 设置登陆服状态
	bool SetLogonServerStatus(int logonID, int status);

	// 设置封号时间
	bool SetUserSealTime(int userID, int time);

private:
	int NeedLoadAllUserData();		//判断是否需要加载数据库中所有用户数据到redis，0：不需要，1：需要，>1：异常错误
	bool LoadAllUserData();			//加载所有用户
	bool LoadAllRewardsPoolData();	//加载所有游戏奖池
	bool LoadAllAgentUser();		//加载所有代理
	bool LoadAllConfig();			//加载系统所有配置到redis
	bool LoadAllUserBag();			//加载用户背包数据

};