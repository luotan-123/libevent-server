#pragma once

#include "RedisBase.h"

// redis 公共类
class CRedisCommon : public CRedisBase
{
public:
	CRedisCommon();
	virtual ~CRedisCommon();

public:
	//////////////////////////////////购买桌子数据////////////////////////////////////////
	// 获取桌子数据
	bool GetPrivateDeskRecordInfo(int deskMixID, PrivateDeskInfo& deskRecordInfo);
	// 设置最大购买桌子索引
	bool SetMarkDeskIndex(int roomID, int iMarkIndex);
	// 删除俱乐部牌桌
	bool DelFGDeskRoom(int friendsGroupID, int friendsGroupDeskNumber);

	//////////////////////////////////背包数据////////////////////////////////////////
	//请求玩家背包信息
	bool GetUserBag(int userID, UserBag& userBagInfo);
	//获取指定背包道具数量
	int GetUserBagCount(int userID, const char * resName);
	//设置玩家背包
	bool SetUserBag(int userID, const char * resName, int changeResNums, bool bAdd);

	/////////////////////////////////用户数据/////////////////////////////////////////
	// 玩家账号是否存在
	bool IsAccountExists(const char* account);
	// 通过玩家账号获取ID
	int GetUserIDByAccount(const char* account);
	// 获取玩家数据
	bool GetUserData(int userID, UserData& userData);
	// 设置玩家的roomID
	bool SetUserRoomID(int userID, int roomID);
	// 设置玩家deskIdx
	bool SetUserDeskIdx(int userID, int deskIdx);
	// 设置玩家金币
	bool SetUserMoney(int userID, long long money);
	// 设置玩家金币，新方法
	bool SetUserMoneyEx(int userID, long long money, bool bAdd = true, int reason = 0, int roomID = 0, long long rateMoney = 0, BYTE isVirtual = 0, int friendsGroupID = 0, int roomType = -1);
	// 设置玩家房卡
	bool SetUserJewels(int userID, int jewels);
	// 新的设置玩家房卡
	bool SetUserJewelsEx(int userID, int jewels, bool bAdd = true, int reason = 0, int roomID = 0, int rateJewels = 0, BYTE isVirtual = 0, int friendsGroupID = 0, int roomType = -1);
	// 获取玩家属性值，只能获取数值类型
	long long GetUserResNums(int userID, const char * resName);
	// 设置玩家属性值，只能设置数值类型
	bool SetUserResNums(int userID, const char * resName, long long resNums);
	// 增加资源值，只能设置数值类型
	long long AddUserResNums(int userID, const char * resName, long long changeResNums);
	// 设置玩家购买中的桌子次数
	bool SetUserBuyingDeskCount(int userID, int count, bool bAdd = true);
	// 设置玩家的跨天时间
	bool SetUserCrossDayTime(int useID, int time);
	// 设置玩家token
	bool SetUserToken(int userID, const char* token);
	// 获取玩家小红点信息
	bool GetUserRedSpot(int userID, UserRedSpot &userRedSpot);
	// 设置当前最大用户id
	bool SetCurMaxUserID(int iUserID);
	// 建立key-userID索引
	bool FixAccountIndexInfo(const char* account, const char* passwd, int userID, int registerType);

	/////////////////////////////////奖池数据/////////////////////////////////////////
	bool GetRewardsPoolInfo(int roomID, RewardsPoolInfo& poolInfo);
	bool SetRoomPoolMoney(int roomID, long long money, bool bAdd = false);
	bool SetRoomGameWinMoney(int roomID, long long money, bool bAdd = false);
	bool SetRoomPercentageWinMoney(int roomID, long long money, bool bAdd = false);
	bool SetRoomPoolData(int roomID, const char * fieldName, long long money, bool bAdd = false);
	long long GetRoomPoolData(int roomID, const char * fieldName);

	/////////////////////////////////服务器配置数据/////////////////////////////////////////
	// 获取服务器状态
	bool GetServerStatus(int &status);
	// 设置服务器状态
	bool SetServerStatus(int status);
	// 获取OtherConfig
	bool GetOtherConfig(OtherConfig &config);
	// 获取roomBaseInfo
	bool GetRoomBaseInfo(int roomID, RoomBaseInfo &room);
	// 获取购买房卡配置 privateDeskConfig
	bool GetBuyGameDeskInfo(const BuyGameDeskInfoKey &buyKey, BuyGameDeskInfo &buyInfo);

	////////////////////////////////比赛场//////////////////////////////////////////
	// 获取满人开比赛玩家
	bool GetFullPeopleMatchPeople(int gameID, int matchID, int peopleCount, std::vector<MatchUserInfo> &vecPeople);
	// 获取定时赛报名玩家
	bool GetTimeMatchPeople(int matchID, std::vector<MatchUserInfo> &vecPeople);

protected:
	// 需要保存到db指令接口，目前只有玩家数据 这个函数目前不能使用redis lock 使用redis lock的时机由调用者决定
	bool InnerHINCRBYCommand(const char* key, const char* redisCmd, long long &retValue, int mode = REDIS_EXTEND_MODE_DEFAULT);	// 只能针对HINCRBY指令，只能数值类型
	bool InnerHMSetCommand(const char* key, const char* redisCmd, int mode = REDIS_EXTEND_MODE_DEFAULT);						// 只能针对HMSET指令

public:
	//玩家在线维护
	bool IsUserOnline(int userID);
	bool RemoveOnlineUser(int userID, BYTE isVirtual);
	bool AddOnlineUser(int userID, BYTE isVirtual);
	bool ClearOnlineUser();

	// 集合操作
	bool AddKeyToSet(const char* key, const char * assKey);
	bool AddKeyToZSet(const char* key, long long socre, long long value);
	bool GetZSetSocreByRank(const char* key, int rank, bool order, long long &socre); //true：从大到小
public:
	// 获取指定位数的随机数，首位不为0
	static int GetRandDigit(int digit);
};