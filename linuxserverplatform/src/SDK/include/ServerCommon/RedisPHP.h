#pragma once

#include "RedisBase.h"

class CRedisPHP : public CRedisBase
{
public:
	CRedisPHP();
	virtual ~CRedisPHP();

public:
	// 初始化
	virtual bool Init();
	// 关闭
	virtual bool Stop();
	// 获取redis Context
	redisContext* GetRedisContext();
public:
	/////////////////////////////////俱乐部相关/////////////////////////////////////////
	bool GetFGMember(int friendsGroupID, std::vector<int> &memberUserIDVec);
	// 获取玩家俱乐部金币
	bool GetUserFriendsGroupMoney(int friendsGroupID, int userID, long long &money);
	// 设置玩家俱乐部金币
	bool SetUserFriendsGroupMoney(int friendsGroupID, int userID, long long money, bool bAdd = true, int reason = 0, int roomID = 0, int rateFireCoin = 0);
	// 获取玩家权限
	int GetUserPower(int friendsGroupID, int userID);
	// 获取俱乐部群主
	int GetFriendsGroupMasterID(int friendsGroupID);
	// 是否可以加入俱乐部房间
	bool IsCanJoinFriendsGroupRoom(int userID, int friendsGroupID);
	// 设置玩家俱乐部资源数量
	bool SetUserFriendsGroupResNums(int friendsGroupID, int userID, const char * resName, long long resNums, bool bAdd = false);
	// 获取通知时间
	bool GetFGNotifySendTime(long long llIndex, time_t &sendTime);
	// 清理玩家俱乐部通知
	void ClearUserFGNotifySet(int userID);
	//  清理所有通知
	void ClearAllFGNotifyInfo();
	//获取俱乐部绑定的管理员ID
	bool GetfriendsGroupToUser(int friendsGroupID, int userID, int &ChouShui, int&ManagerId);


	/////////////////////////////////邮件模块/////////////////////////////////////////
	bool GetEmailSendTime(long long llIndex, int &sendTime);
	bool GetUserAllEmailID(int userID, std::vector<EmailSimpleInfo> &simpleEmailID, bool Asc = true);
	bool DelUserEmailInfo(int userID, long long emailID);
	void ClearUserEmailSet(int userID);
	void ClearAllEmailInfo();

	//////////////////////////////////数据统计////////////////////////////////////////
	long long AddUserResNums(int userID, const char * resName, long long changeResNums);

	//////////////////////////////////比赛场////////////////////////////////////////
	// 将redis比赛信息，加载到内存
	bool LoadAllMatchInfo(std::map<long long, MatchInfo> & matchInfoMap);
	// 获取比赛信息
	bool GetMatchInfo(int matchID, MatchInfo& matchInfo);
	// 设置比赛状态
	bool SetMatchStatus(int matchID, BYTE matchStatus);
};