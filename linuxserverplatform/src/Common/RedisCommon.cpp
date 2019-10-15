#include "CommonHead.h"
#include "RedisCommon.h"

CRedisCommon::CRedisCommon()
{
}

CRedisCommon::~CRedisCommon()
{
}

bool CRedisCommon::IsAccountExists(const char* account)
{
	if (!account)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "EXISTS %s|%s", TBL_NORMAL_TO_USERID, account);

	redisReply* pReply = (redisReply *)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

int CRedisCommon::GetUserIDByAccount(const char * account)
{
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "GET %s|%s", TBL_NORMAL_TO_USERID, account);

	redisReply* pReply = (redisReply *)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int userID = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		userID = atoi(pReply->str);
	}
	freeReplyObject(pReply);

	return userID;
}

bool CRedisCommon::GetUserData(int userID, UserData& userData)
{
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID userID=%d", userID);
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGETALL %s|%d", TBL_USER, userID);
	if (!pReply)
	{
		ERROR_LOG("获取玩家数据失败 userID=%d", userID);
		return false;
	}

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		ERROR_LOG("invalid elements elements=%d, userID=%d", elements, userID);
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "userID"))
		{
			userData.userID = atoi(value);
		}
		else if (!strcmp(field, "account"))
		{
			memcpy(userData.account, value, Min_(strlen(value), sizeof(userData.account) - 1));
		}
		else if (!strcmp(field, "passwd"))
		{
			memcpy(userData.passwd, value, Min_(strlen(value), sizeof(userData.passwd)));
		}
		else if (!strcmp(field, "name"))
		{
			memcpy(userData.name, value, Min_(strlen(value), sizeof(userData.name)));
		}
		else if (!strcmp(field, "sex"))
		{
			userData.sex = atoi(value);
		}
		/*else if (!strcmp(field, "mail"))
		{
			memcpy(userData.mail, value, strlen(value));
		}*/
		else if (!strcmp(field, "phone"))
		{
			memcpy(userData.phone, value, Min_(strlen(value), sizeof(userData.phone)));
		}
		else if (!strcmp(field, "money"))
		{
			userData.money = atoll(value);
		}
		else if (!strcmp(field, "bankMoney"))
		{
			userData.bankMoney = atoll(value);
		}
		else if (!strcmp(field, "bankPasswd"))
		{
			memcpy(userData.bankPasswd, value, Min_(strlen(value), sizeof(userData.bankPasswd)));
		}
		else if (!strcmp(field, "jewels"))
		{
			userData.jewels = atoi(value);
		}
		else if (!strcmp(field, "roomID"))
		{
			userData.roomID = atoi(value);
		}
		else if (!strcmp(field, "deskIdx"))
		{
			userData.deskIdx = atoi(value);
		}
		else if (!strcmp(field, "logonIP"))
		{
			memcpy(userData.logonIP, value, Min_(strlen(value), sizeof(userData.logonIP)));
		}
		else if (!strcmp(field, "headURL"))
		{
			memcpy(userData.headURL, value, Min_(strlen(value), sizeof(userData.headURL)));
		}
		else if (!strcmp(field, "macAddr"))
		{
			memcpy(userData.macAddr, value, Min_(strlen(value), sizeof(userData.macAddr)));
		}
		else if (!strcmp(field, "token"))
		{
			memcpy(userData.token, value, Min_(strlen(value), sizeof(userData.token)));
		}
		else if (!strcmp(field, "isVirtual"))
		{
			userData.isVirtual = atoi(value);
		}
		else if (!strcmp(field, "status"))
		{
			userData.status = atoi(value);
		}
		/*else if (!strcmp(field, "reqSupportTimes"))
		{
			userData.reqSupportTimes = atoi(value);
		}
		else if (!strcmp(field, "lastReqSupportDate"))
		{
			userData.lastReqSupportDate = atoi(value);
		}*/
		else if (!strcmp(field, "winCount"))
		{
			userData.winCount = atoi(value);
		}
		else if (!strcmp(field, "registerTime"))
		{
			userData.registerTime = atoi(value);
		}
		else if (!strcmp(field, "registerIP"))
		{
			memcpy(userData.registerIP, value, Min_(strlen(value), sizeof(userData.registerIP)));
		}
		else if (!strcmp(field, "registerType"))
		{
			userData.registerType = atoi(value);
		}
		else if (!strcmp(field, "buyingDeskCount"))
		{
			userData.buyingDeskCount = atoi(value);
		}
		else if (!strcmp(field, "lastCrossDayTime"))
		{
			userData.lastCrossDayTime = atoi(value);
		}
		else if (!strcmp(field, "totalGameCount"))
		{
			userData.totalGameCount = atoi(value);
		}
		else if (!strcmp(field, "sealFinishTime"))
		{
			userData.sealFinishTime = atoi(value);
		}
		/*else if (!strcmp(field, "wechat"))
		{
			memcpy(userData.wechat, value, strlen(value));
		}*/
		else if (!strcmp(field, "phonePasswd"))
		{
			memcpy(userData.phonePasswd, value, Min_(strlen(value), sizeof(userData.phonePasswd)));
		}
		else if (!strcmp(field, "accountInfo"))
		{
			memcpy(userData.accountInfo, value, Min_(strlen(value), sizeof(userData.accountInfo)));
		}
		else if (!strcmp(field, "totalGameWinCount"))
		{
			userData.totalGameWinCount = atoi(value);
		}
		else if (!strcmp(field, "Lng"))
		{
			memcpy(userData.Lng, value, Min_(strlen(value), sizeof(userData.Lng)));
		}
		else if (!strcmp(field, "Lat"))
		{
			memcpy(userData.Lat, value, Min_(strlen(value), sizeof(userData.Lat)));
		}
		else if (!strcmp(field, "address"))
		{
			memcpy(userData.address, value, Min_(strlen(value), sizeof(userData.address)));
		}
		else if (!strcmp(field, "lastCalcOnlineToTime"))
		{
			userData.lastCalcOnlineToTime = atoll(value);
		}
		else if (!strcmp(field, "allOnlineToTime"))
		{
			userData.allOnlineToTime = atoll(value);
		}
		else if (!strcmp(field, "IsOnline"))
		{
			userData.IsOnline = atoi(value);
		}
		else if (!strcmp(field, "motto"))
		{
			memcpy(userData.motto, value, Min_(strlen(value), sizeof(userData.motto)));
		}
		else if (!strcmp(field, "xianliao"))
		{
			memcpy(userData.xianliao, value, Min_(strlen(value), sizeof(userData.xianliao)));
		}
		else if (!strcmp(field, "controlParam"))
		{
			userData.controlParam = atoi(value);
		}
		else if (!strcmp(field, "matchType"))
		{
			userData.matchType = atoi(value);
		}
		else if (!strcmp(field, "combineMatchID"))
		{
			userData.combineMatchID = atoll(value);
		}
		else if (!strcmp(field, "matchStatus"))
		{
			userData.matchStatus = atoi(value);
		}
		else if (!strcmp(field, "curMatchRank"))
		{
			userData.curMatchRank = atoi(value);
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::GetPrivateDeskRecordInfo(int deskMixID, PrivateDeskInfo & deskRecordInfo)
{
	if (deskMixID <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements <= 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "roomID"))
		{
			deskRecordInfo.roomID = atoi(value);
		}
		else if (!strcmp(field, "deskIdx"))
		{
			deskRecordInfo.deskIdx = atoi(value);
		}
		else if (!strcmp(field, "masterID"))
		{
			deskRecordInfo.masterID = atoi(value);
		}
		else if (!strcmp(field, "passwd"))
		{
			memcpy(deskRecordInfo.passwd, value, strlen(value));
		}
		else if (!strcmp(field, "buyGameCount"))
		{
			deskRecordInfo.buyGameCount = atoi(value);
		}
		else if (!strcmp(field, "gameRules"))
		{
			memcpy(deskRecordInfo.gameRules, value, Min_(sizeof(deskRecordInfo.gameRules), strlen(value)));
		}
		else if (!strcmp(field, "createTime"))
		{
			deskRecordInfo.createTime = atoll(value);
		}
		else if (!strcmp(field, "checkTime"))
		{
			deskRecordInfo.checkTime = atoll(value);
		}
		else if (!strcmp(field, "masterNotPlay"))
		{
			deskRecordInfo.masterNotPlay = atoi(value);
		}
		else if (!strcmp(field, "payType"))
		{
			deskRecordInfo.payType = atoi(value);
		}
		else if (!strcmp(field, "currDeskUserCount"))
		{
			deskRecordInfo.currDeskUserCount = atoi(value);
		}
		else if (!strcmp(field, "maxDeskUserCount"))
		{
			deskRecordInfo.maxDeskUserCount = atoi(value);
		}
		else if (!strcmp(field, "currWatchUserCount"))
		{
			deskRecordInfo.currWatchUserCount = atoi(value);
		}
		else if (!strcmp(field, "maxWatchUserCount"))
		{
			deskRecordInfo.maxWatchUserCount = atoi(value);
		}
		else if (!strcmp(field, "friendsGroupID"))
		{
			deskRecordInfo.friendsGroupID = atoi(value);
		}
		else if (!strcmp(field, "curGameCount"))
		{
			deskRecordInfo.curGameCount = atoi(value);
		}
		else if (!strcmp(field, "friendsGroupDeskNumber"))
		{
			deskRecordInfo.friendsGroupDeskNumber = atoi(value);
		}
		else if (!strcmp(field, "arrUserID"))
		{
			memcpy(deskRecordInfo.arrUserID, value, Min_(sizeof(deskRecordInfo.arrUserID), strlen(value)));
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::SetUserRoomID(int userID, int roomID)
{
	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s roomID %d", key.c_str(), roomID);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;
}

bool CRedisCommon::SetUserDeskIdx(int userID, int deskIdx)
{
	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s deskIdx %d", key.c_str(), deskIdx);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;
}

// 机器人使用
bool CRedisCommon::SetUserMoney(int userID, long long money)
{
	if (money < 0)
	{
		money = 0;
	}

	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s money %lld", key.c_str(), money);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	// 添加进排行榜
	AddKeyToZSet(TBL_RANKING_MONEY, money, userID);

	return true;
}

// 设置玩家金币，新方法
bool CRedisCommon::SetUserMoneyEx(int userID, long long money, bool bAdd /*= true*/, int reason /*= 0*/, int roomID/* = 0*/, long long rateMoney /*= 0*/, BYTE isVirtual/* = 0*/, int friendsGroupID /*= 0*/, int roomType/* = -1*/)
{
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	std::string key = MakeKey(TBL_USER, userID);

	if (bAdd)
	{
		if (money == 0) //不增不减
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s money %lld", key.c_str(), money);
	}
	else
	{
		//请使用 SetUserMoney
		return true;
	}

	long long llRet = 0;
	if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llRet, isVirtual == 0 ? REDIS_EXTEND_MODE_UPDATE : REDIS_EXTEND_MODE_DEFAULT))
	{
		return false;
	}

	bool bSubsidy = false;
	if (bAdd && llRet < 0)
	{
		SetUserResNums(userID, "money", 0);
		bSubsidy = true;
	}

	// 添加进排行榜
	AddKeyToZSet(TBL_RANKING_MONEY, llRet, userID);

	if (!isVirtual)
	{
		time_t currTime = time(NULL);
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_USER_MONEY_CHANGE, tableName, sizeof(tableName));

		// 账单 当前金币|此次变化|原因
		BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,money,changeMoney,reason,roomID,friendsGroupID,rateMoney) VALUES(%d,%lld,%lld,%lld,%d,%d,%d,%lld)",
			tableName, userID, currTime, llRet, money, reason, roomID, friendsGroupID, rateMoney);
		if (bSubsidy)
		{
			// 账单 当前金币|此次变化|原因
			BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,money,changeMoney,reason,roomID,friendsGroupID,rateMoney) VALUES(%d,%lld,%lld,%lld,%d,%d,%d,%lld)",
				tableName, userID, currTime, (long long)0, llabs(llRet), RESOURCE_CHANGE_REASON_SYSTEM_SUBSIDY, roomID, friendsGroupID, rateMoney);
		}

		// 业绩
		if (roomType == ROOM_TYPE_GOLD)
		{
			ConfigManage()->GetTableNameByDate(TBL_WEB_AGENT_PUMP_MONEY, tableName, sizeof(tableName));
			BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,money,changeMoney,reason,roomID,friendsGroupID,rateMoney) VALUES(%d,%lld,%lld,%lld,%d,%d,%d,%lld)",
				tableName, userID, currTime, llRet, money, reason, roomID, friendsGroupID, rateMoney);
		}
	}

	return true;
}

// 获取玩家属性值，只能获取数值类型
long long CRedisCommon::GetUserResNums(int userID, const char * resName)
{
	if (userID <= 0 || !resName)
	{
		return 0;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d %s", TBL_USER, userID, resName);
	if (!pReply)
	{
		return 0;
	}

	long long ret = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoll(pReply->str);
	}

	freeReplyObject(pReply);

	return ret;
}

// 设置玩家属性值，只能设置数值类型
bool CRedisCommon::SetUserResNums(int userID, const char * resName, long long resNums)
{
	if (userID <= 0 || !resName)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s|%d %s %lld", TBL_USER, userID, resName, resNums);
	if (!pReply)
	{
		return false;
	}

	bool ret = false;
	if (pReply->type != REDIS_REPLY_ERROR && pReply->integer == 0)
	{
		ret = true;
	}
	else
	{
		INFO_LOG("userInfo添加了域值 :%s", resName);
	}

	freeReplyObject(pReply);

	return ret;
}

// 增加资源值，只能设置数值类型
long long CRedisCommon::AddUserResNums(int userID, const char * resName, long long changeResNums)
{
	if (userID <= 0 || !resName)
	{
		return 0;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HINCRBY %s|%d %s %lld", TBL_USER, userID, resName, changeResNums);
	if (!pReply)
	{
		return 0;
	}

	long long ret = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = pReply->integer;
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisCommon::SetUserJewels(int userID, int jewels)
{
	if (jewels < 0)
	{
		jewels = 0;
	}

	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s jewels %d", key.c_str(), jewels);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	// 添加进排行榜
	AddKeyToZSet(TBL_RANKING_JEWELS, jewels, userID);

	return true;
}

// 新的设置玩家房卡
bool CRedisCommon::SetUserJewelsEx(int userID, int jewels, bool bAdd /*= true*/, int reason /*= 0*/, int roomID/* = 0*/, int rateJewels/* = 0*/, BYTE isVirtual/* = 0*/, int friendsGroupID/* = 0*/, int roomType /*= -1*/)
{
	std::string key = MakeKey(TBL_USER, userID);
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	if (bAdd)
	{
		if (jewels == 0) //不增不减
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s jewels %d", key.c_str(), jewels);
	}
	else
	{
		// 请使用 SetUserJewels
		return true;
	}

	long long llRet = 0;
	if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llRet, isVirtual == 0 ? REDIS_EXTEND_MODE_UPDATE : REDIS_EXTEND_MODE_DEFAULT))
	{
		return false;
	}

	bool bSubsidy = false;
	if (bAdd && llRet < 0)
	{
		SetUserResNums(userID, "jewels", 0);
		bSubsidy = true;
	}

	// 添加进排行榜
	AddKeyToZSet(TBL_RANKING_JEWELS, llRet, userID);

	if (!isVirtual)
	{
		time_t currTime = time(NULL);
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_USER_JEWELS_CHANGE, tableName, sizeof(tableName));

		// 账单 当前金币|此次变化|原因
		BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,jewels,changeJewels,reason,roomID,friendsGroupID,rateJewels) VALUES(%d,%lld,%d,%d,%d,%d,%d,%d)",
			tableName, userID, currTime, (int)llRet, jewels, reason, roomID, friendsGroupID, rateJewels);
		if (bSubsidy)
		{
			// 账单 当前金币|此次变化|原因
			BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,jewels,changeJewels,reason,roomID,friendsGroupID,rateJewels) VALUES(%d,%lld,%d,%d,%d,%d,%d,%d)",
				tableName, userID, currTime, 0, (int)abs((int)llRet), RESOURCE_CHANGE_REASON_SYSTEM_SUBSIDY, roomID, friendsGroupID, rateJewels);
		}

		// 业绩
		ConfigManage()->GetTableNameByDate(TBL_WEB_AGENT_PUMP_JEWELS, tableName, sizeof(tableName));
		BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,jewels,changeJewels,reason,roomID,friendsGroupID,rateJewels) VALUES(%d,%lld,%d,%d,%d,%d,%d,%d)",
			tableName, userID, currTime, (int)llRet, jewels, reason, roomID, friendsGroupID, rateJewels);
	}

	return true;
}

bool CRedisCommon::SetUserBuyingDeskCount(int userID, int count, bool bAdd/* = true*/)
{
	std::string key = MakeKey(TBL_USER, userID);
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	long long llValue = 0;

	if (bAdd)
	{
		if (count == 0)
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s buyingDeskCount %d", key.c_str(), count);
		if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llValue))
		{
			return false;
		}
	}
	else
	{
		if (count < 0)
		{
			count = 0;
		}
		sprintf(redisCmd, "HMSET %s buyingDeskCount %d", key.c_str(), count);
		if (!InnerHMSetCommand(key.c_str(), redisCmd))
		{
			return false;
		}
	}

	if (bAdd && count < 0 && llValue < 0)
	{
		SetUserResNums(userID, "buyingDeskCount", 0);
	}

	return true;
}

bool CRedisCommon::SetUserCrossDayTime(int userID, int time)
{
	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s lastCrossDayTime %d", key.c_str(), time);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;
}

bool CRedisCommon::SetUserToken(int userID, const char* token)
{
	if (!token)
	{
		return false;
	}

	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s token %s", key.c_str(), token);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;

}

// 获取平台控制数据
bool CRedisCommon::GetRewardsPoolInfo(int roomID, RewardsPoolInfo& poolInfo)
{
	if (roomID <= 0)
	{
		ERROR_LOG("invalid roomID roomID=%d", roomID);
		return false;
	}

	std::string key = MakeKey(TBL_REWARDS_POOL, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		ERROR_LOG("invalid elements elements=%d, roomID=%d", elements, roomID);
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "roomID"))
		{
			poolInfo.roomID = atoi(value);
		}
		else if (!strcmp(field, "poolMoney"))
		{
			poolInfo.poolMoney = atoll(value);
		}
		else if (!strcmp(field, "gameWinMoney"))
		{
			poolInfo.gameWinMoney = atoll(value);
		}
		else if (!strcmp(field, "percentageWinMoney"))
		{
			poolInfo.percentageWinMoney = atoll(value);
		}
		else if (!strcmp(field, "otherWinMoney"))
		{
			poolInfo.otherWinMoney = atoll(value);
		}
		else if (!strcmp(field, "allGameWinMoney"))
		{
			poolInfo.allGameWinMoney = atoll(value);
		}
		else if (!strcmp(field, "allPercentageWinMoney"))
		{
			poolInfo.allPercentageWinMoney = atoll(value);
		}
		else if (!strcmp(field, "allOtherWinMoney"))
		{
			poolInfo.allOtherWinMoney = atoll(value);
		}
		else if (!strcmp(field, "platformCtrlType"))
		{
			poolInfo.platformCtrlType = atoi(value);
		}
		else if (!strcmp(field, "platformCtrlPercent"))
		{
			poolInfo.platformCtrlPercent = atoi(value);
		}
		else if (!strcmp(field, "realPeopleFailPercent"))
		{
			poolInfo.realPeopleFailPercent = atoi(value);
		}
		else if (!strcmp(field, "realPeopleWinPercent"))
		{
			poolInfo.realPeopleWinPercent = atoi(value);
		}
		else if (!strcmp(field, "detailInfo"))
		{
			memcpy(poolInfo.detailInfo, value, Min_(strlen(value), sizeof(poolInfo.detailInfo)));
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::InnerHINCRBYCommand(const char* key, const char* redisCmd, long long &retValue, int mode/* = REDIS_EXTEND_MODE_DEFAULT*/)
{
	if (!key || !redisCmd)
	{
		return false;
	}

	if (mode != REDIS_EXTEND_MODE_DEFAULT)
	{
		SetExtendMode(key, mode);
		AddKeyToSet(CACHE_UPDATE_SET, key);
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		retValue = pReply->integer;
		ret = true;
	}

	if (!ret)
	{
		ERROR_LOG("命令执行失败:[ %s ],errstr=%s", redisCmd, m_pContext->errstr);
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisCommon::InnerHMSetCommand(const char* key, const char* redisCmd, int mode/* = REDIS_EXTEND_MODE_DEFAULT*/)
{
	if (!key || !redisCmd)
	{
		return false;
	}

	if (mode != REDIS_EXTEND_MODE_DEFAULT)
	{
		SetExtendMode(key, mode);
		AddKeyToSet(CACHE_UPDATE_SET, key);
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;

	if (pReply->type == REDIS_REPLY_STATUS)
	{
		if (!strcmp(pReply->str, "OK"))
		{
			ret = true;
		}
	}

	if (!ret)
	{
		ERROR_LOG("命令执行失败:[ %s ],errstr=%s", redisCmd, m_pContext->errstr);
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisCommon::AddKeyToSet(const char * key, const char * assKey)
{
	if (!key || !assKey)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SADD %s %s", key, assKey);
	if (!pReply)
	{
		return false;
	}

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);
	return ret;
}

bool CRedisCommon::AddKeyToZSet(const char* key, long long socre, long long value)
{
	if (!key)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "ZADD %s %lld %lld", key, socre, value);
	if (!pReply)
	{
		return false;
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::GetZSetSocreByRank(const char* key, int rank, bool order, long long &socre)
{
	if (!key)
	{
		return false;
	}

	socre = 0;

	if (rank < 0)
	{
		return true;
	}

	redisReply* pReply = NULL;
	if (order)
	{
		pReply = (redisReply*)redisCommand(m_pContext, "ZREVRANGE %s %d %d WITHSCORES", key, rank, rank);
		if (!pReply)
		{
			return false;
		}
	}
	else
	{
		pReply = (redisReply*)redisCommand(m_pContext, "ZRANGE %s %d %d WITHSCORES", key, rank, rank);
		if (!pReply)
		{
			return false;
		}
	}

	if (pReply->elements == 0)
	{
		freeReplyObject(pReply);
		return true;
	}

	if (pReply->elements != 2)
	{
		freeReplyObject(pReply);
		ERROR_LOG("获取score失败:key=%s", key);
		return false;
	}

	for (size_t i = 0; i < pReply->elements; i += 2)
	{
		const char* pSocre = pReply->element[i + 1]->str;
		if (pSocre)
		{
			socre = atoll(pSocre);
			break;
		}
	}

	freeReplyObject(pReply);

	return true;
}

// 设置奖池
bool CRedisCommon::SetRoomPoolMoney(int roomID, long long money, bool bAdd)
{
	if (roomID <= 0)
	{
		return false;
	}
	std::string key = MakeKey(TBL_REWARDS_POOL, roomID);
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	long long llValue = 0;

	if (bAdd)
	{
		if (money == 0)
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s poolMoney %lld", key.c_str(), money);
		if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llValue))
		{
			return false;
		}
	}
	else
	{
		/*sprintf(redisCmd, "HMSET %s poolMoney %lld", key.c_str(), money);
		if (!InnerHMSetCommand(key.c_str(), redisCmd))
		{
			return false;
		}*/

		INFO_LOG("不支持直接设置奖池");
		return true;
	}

	return true;
}

//奖池
bool CRedisCommon::SetRoomGameWinMoney(int roomID, long long money, bool bAdd)
{
	if (roomID <= 0)
	{
		return false;
	}
	std::string key = MakeKey(TBL_REWARDS_POOL, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	long long llValue = 0;
	if (bAdd)
	{
		if (money == 0)
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s gameWinMoney %lld", key.c_str(), money);
		if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llValue, REDIS_EXTEND_MODE_UPDATE))
		{
			return false;
		}
	}
	else
	{
		/*sprintf(redisCmd, "HMSET %s gameWinMoney %lld", key.c_str(), money);
		if (!InnerHMSetCommand(key.c_str(), redisCmd, REDIS_EXTEND_MODE_UPDATE))
		{
			return false;
		}*/

		INFO_LOG("不支持直接设置奖池");
		return true;
	}

	return true;
}

bool CRedisCommon::SetRoomPercentageWinMoney(int roomID, long long money, bool bAdd)
{
	if (roomID <= 0)
	{
		return false;
	}
	std::string key = MakeKey(TBL_REWARDS_POOL, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	long long llValue = 0;
	if (bAdd)
	{
		if (money == 0)
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s percentageWinMoney %lld", key.c_str(), money);
		if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llValue, REDIS_EXTEND_MODE_UPDATE))
		{
			return false;
		}
	}
	else
	{
		/*sprintf(redisCmd, "HMSET %s percentageWinMoney %lld", key.c_str(), money);
		if (!InnerHMSetCommand(key.c_str(), redisCmd, REDIS_EXTEND_MODE_UPDATE))
		{
		return false;
		}*/

		INFO_LOG("不支持直接设置奖池");
		return true;
	}

	return true;
}

bool CRedisCommon::SetRoomPoolData(int roomID, const char * fieldName, long long money, bool bAdd/* = false*/)
{
	if (roomID <= 0 || !fieldName)
	{
		return false;
	}
	std::string key = MakeKey(TBL_REWARDS_POOL, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	long long llValue = 0;

	if (bAdd)
	{
		if (money == 0)
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s %s %lld", key.c_str(), fieldName, money);
		if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llValue, REDIS_EXTEND_MODE_UPDATE))
		{
			return false;
		}
	}
	else
	{
		/*sprintf(redisCmd, "HMSET %s %s %lld", key.c_str(), fieldName, money);
		if (!InnerHMSetCommand(key.c_str(), redisCmd, REDIS_EXTEND_MODE_UPDATE))
		{
		return false;
		}*/

		INFO_LOG("不支持直接设置奖池");
		return true;
	}

	return true;
}

long long CRedisCommon::GetRoomPoolData(int roomID, const char * fieldName)
{
	long long ret = 0;

	if (roomID <= 0 || !fieldName || strlen(fieldName) <= 0)
	{
		return ret;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d %s", TBL_REWARDS_POOL, roomID, fieldName);
	if (!pReply)
	{
		return ret;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoll(pReply->str);
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisCommon::GetServerStatus(int &status)
{
	status = SERVER_PLATFROM_STATUS_NORMAL;

	char redisCmd[128] = "";

	std::string key = TBL_SERVER_STATUS;

	sprintf(redisCmd, "GET %s", key.c_str());
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	if (pReply->str == NULL || pReply->len == 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	status = atoi(pReply->str);

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::SetServerStatus(int status)
{
	std::string key = TBL_SERVER_STATUS;

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "SET %s %d", key.c_str(), status);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::SetCurMaxUserID(int iUserID)
{
	if (iUserID <= 0)
	{
		return false;
	}

	std::string key = TBL_CURMAX_USERID;

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "SET %s %d", key.c_str(), iUserID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::FixAccountIndexInfo(const char * account, const char* passwd, int userID, int registerType)
{
	if (!account || userID <= 0)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	if (registerType == LOGON_NORMAL || registerType == LOGON_QUICK)	// 普通用户
	{
		sprintf(redisCmd, "SET %s|%s %d", TBL_NORMAL_TO_USERID, account, userID);
	}
	else if (registerType == LOGON_TEL_PHONE)
	{
		sprintf(redisCmd, "SET %s|%s %d", TBL_PHONE_TOUSERID, account, userID);
	}
	else if (registerType == LOGON_VISITOR)
	{
		sprintf(redisCmd, "SET %s|%s %d", TBL_VISITOR_TOUSERID, passwd, userID);
	}
	else	// 第三方用户
	{
		sprintf(redisCmd, "SET %s|%s %d", TBL_TRDUSERID, passwd, userID);
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

//////////////////////////////////玩家在线维护////////////////////////////////////////
bool CRedisCommon::IsUserOnline(int userID)
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SISMEMBER %s %d", TBL_ONLINE_USER_SET, userID);
	if (!pReply)
	{
		ERROR_LOG("判断在线玩家出错,userID=%d", userID);
		return false;
	}

	bool ret = false;
	if (pReply->type == REDIS_REPLY_INTEGER && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisCommon::RemoveOnlineUser(int userID, BYTE isVirtual)
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SREM %s %d", TBL_ONLINE_USER_SET, userID);
	if (!pReply)
	{
		ERROR_LOG("删除在线玩家出错,userID=%d", userID);
		return false;
	}

	freeReplyObject(pReply);

	if (!isVirtual)
	{
		pReply = (redisReply*)redisCommand(m_pContext, "SREM %s %d", TBL_ONLINE_REALUSER_SET, userID);
		if (!pReply)
		{
			ERROR_LOG("删除在线玩家出错,userID=%d", userID);
			return false;
		}

		freeReplyObject(pReply);
	}

	return true;
}

bool CRedisCommon::AddOnlineUser(int userID, BYTE isVirtual)
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SADD %s %d", TBL_ONLINE_USER_SET, userID);
	if (!pReply)
	{
		ERROR_LOG("添加在线玩家出错,userID=%d", userID);
		return false;
	}

	freeReplyObject(pReply);

	if (!isVirtual)
	{
		pReply = (redisReply*)redisCommand(m_pContext, "SADD %s %d", TBL_ONLINE_REALUSER_SET, userID);
		if (!pReply)
		{
			ERROR_LOG("添加在线玩家出错,userID=%d", userID);
			return false;
		}

		freeReplyObject(pReply);
	}

	return true;
}

bool CRedisCommon::ClearOnlineUser()
{
	DelKey(TBL_ONLINE_USER_SET);
	DelKey(TBL_ONLINE_REALUSER_SET);

	return true;
}

int CRedisCommon::GetRandDigit(int digit)
{
	if (digit <= 0)
	{
		return 0;
	}

	// 头
	int head = rand() % 9 + 1;		// 取1-9中的一个值

	int count = digit - 1;
	if (count == 0)
	{
		return head;
	}

	// 剩余的部分
	int tailSum = 1;
	for (int i = 0; i < count; i++)
	{
		tailSum *= 10;
	}

	int tail = rand() % tailSum;

	int ret = head * tailSum + tail;

	return ret;
}

/////////////////////////////////获取系统配置/////////////////////////////////////////
// 获取OtherConfig
bool CRedisCommon::GetOtherConfig(OtherConfig &config)
{
	std::string key = MakeKey(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_OTHER);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		ERROR_LOG("invalid elements elements=%d", elements);
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "supportTimesEveryDay"))
		{
			config.supportTimesEveryDay = atoi(value);
		}
		else if (!strcmp(field, "supportMinLimitMoney"))
		{
			config.supportMinLimitMoney = atoi(value);
		}
		else if (!strcmp(field, "supportMoneyCount"))
		{
			config.supportMoneyCount = atoi(value);
		}
		else if (!strcmp(field, "registerGiveMoney"))
		{
			config.registerGiveMoney = atoi(value);
		}
		else if (!strcmp(field, "registerGiveJewels"))
		{
			config.registerGiveJewels = atoi(value);
		}
		else if (!strcmp(field, "logonGiveMoneyEveryDay"))
		{
			config.logonGiveMoneyEveryDay = atoi(value);
		}
		else if (!strcmp(field, "sendHornCostJewels"))
		{
			config.sendHornCostJewels = atoi(value);
		}
		else if (!strcmp(field, "useMagicExpressCostMoney"))
		{
			config.useMagicExpressCostMoney = atoi(value);
		}
		else if (!strcmp(field, "buyingDeskCount"))
		{
			config.buyingDeskCount = atoi(value);
		}
		else if (!strcmp(field, "friendRewardMoney"))
		{
			config.friendRewardMoney = atoi(value);
		}
		else if (!strcmp(field, "friendRewardMoneyCount"))
		{
			config.friendRewardMoneyCount = atoi(value);
		}
		else if (!strcmp(field, "friendTakeRewardMoneyCount"))
		{
			config.friendTakeRewardMoneyCount = atoi(value);
		}
		else if (!strcmp(field, "byIsIPRegisterLimit"))
		{
			config.byIsIPRegisterLimit = atoi(value);
		}
		else if (!strcmp(field, "IPRegisterLimitCount"))
		{
			config.IPRegisterLimitCount = atoi(value);
		}
		else if (!strcmp(field, "byIsDistributedTable"))
		{
			config.byIsDistributedTable = atoi(value);
		}
		else if (!strcmp(field, "http"))
		{
			memcpy(config.http, value, Min_(strlen(value), sizeof(config.http)));
		}
		else if (!strcmp(field, "byIsOneToOne"))
		{
			config.byIsOneToOne = atoi(value);
		}
	}

	freeReplyObject(pReply);

	return true;
}

// 获取roomBaseInfo
bool CRedisCommon::GetRoomBaseInfo(int roomID, RoomBaseInfo &room)
{
	std::string key = MakeKey(TBL_BASE_ROOM, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "roomID"))
		{
			room.roomID = atoi(value);
		}
		else if (!strcmp(field, "gameID"))
		{
			room.gameID = atoi(value);
		}
		else if (!strcmp(field, "type"))
		{
			room.type = atoi(value);
		}
		else if (!strcmp(field, "sort"))
		{
			room.sort = atoi(value);
		}
		else if (!strcmp(field, "deskCount"))
		{
			room.deskCount = atoi(value);
		}
		else if (!strcmp(field, "minPoint"))
		{
			room.minPoint = atoi(value);
		}
		else if (!strcmp(field, "maxPoint"))
		{
			room.maxPoint = atoi(value);
		}
		else if (!strcmp(field, "basePoint"))
		{
			room.basePoint = atoi(value);
		}
		else if (!strcmp(field, "gameBeginCostMoney"))
		{
			room.gameBeginCostMoney = atoi(value);
		}
		else if (!strcmp(field, "roomSign"))
		{
			room.roomSign = atoi(value);
		}
		else if (!strcmp(field, "robotCount"))
		{
			room.robotCount = atoi(value);
		}
		else if (!strcmp(field, "currPeopleCount"))
		{
			room.currPeopleCount = atoi(value);
		}
		else if (!strcmp(field, "level"))
		{
			room.level = atoi(value);
		}
		else if (!strcmp(field, "configInfo"))
		{
			memcpy(room.configInfo, value, Min_(strlen(value), sizeof(room.configInfo) - 1));
		}
	}

	freeReplyObject(pReply);

	return true;
}

// 获取购买房卡配置 privateDeskConfig
bool CRedisCommon::GetBuyGameDeskInfo(const BuyGameDeskInfoKey &buyKey, BuyGameDeskInfo &buyInfo)
{
	char key[64] = "";
	sprintf(key, "%s|%d,%d,%d", TBL_BASE_BUY_DESK, buyKey.gameID, buyKey.count, buyKey.roomType);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		ERROR_LOG("invalid elements elements=%d", elements);
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "gameID"))
		{
			buyInfo.gameID = atoi(value);
		}
		else if (!strcmp(field, "count"))
		{
			buyInfo.count = atoi(value);
		}
		else if (!strcmp(field, "roomType"))
		{
			buyInfo.roomType = atoi(value);
		}
		else if (!strcmp(field, "costResType"))
		{
			buyInfo.costResType = atoi(value);
		}
		else if (!strcmp(field, "costNums"))
		{
			buyInfo.costNums = atoi(value);
		}
		else if (!strcmp(field, "AAcostNums"))
		{
			buyInfo.AAcostNums = atoi(value);
		}
		else if (!strcmp(field, "otherCostNums"))
		{
			buyInfo.otherCostNums = atoi(value);
		}
		else if (!strcmp(field, "peopleCount"))
		{
			buyInfo.peopleCount = atoi(value);
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::SetMarkDeskIndex(int roomID, int iMarkIndex)
{
	if (iMarkIndex < 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_MARK_DESK_INDEX, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "SET %s %d", key.c_str(), iMarkIndex);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

// 删除牌桌
bool CRedisCommon::DelFGDeskRoom(int friendsGroupID, int friendsGroupDeskNumber)
{
	if (friendsGroupID <= 0)
	{
		return false;
	}

	// 删除redis中的牌桌
	char combinationKey[64] = "";
	sprintf(combinationKey, "%s|%d,%d", TBL_FG_ROOM_INFO, friendsGroupID, friendsGroupDeskNumber);

	return DelKey(combinationKey);
}

// 获取玩家小红点信息
bool CRedisCommon::GetUserRedSpot(int userID, UserRedSpot &userRedSpot)
{
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID userID=%d", userID);
		return false;
	}

	std::string key = MakeKey(TBL_USER_REDSPOT, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "notEMRead"))
		{
			userRedSpot.notEMRead = atoi(value);
		}
		else if (!strcmp(field, "notEMReceived"))
		{
			userRedSpot.notEMReceived = atoi(value);
		}
		else if (!strcmp(field, "friendList"))
		{
			userRedSpot.friendList = atoi(value);
		}
		else if (!strcmp(field, "friendNotifyList"))
		{
			userRedSpot.friendNotifyList = atoi(value);
		}
		else if (!strcmp(field, "FGNotifyList"))
		{
			userRedSpot.FGNotifyList = atoi(value);
		}
	}

	freeReplyObject(pReply);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 背包系统
bool CRedisCommon::GetUserBag(int userID, UserBag& userBagInfo)
{
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID=%d", userID);
		return false;
	}

	std::string key = MakeKey(TBL_USER_BAG, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "skillFrozen"))
		{
			userBagInfo.skillFrozen = atoi(value);
		}
		else if (!strcmp(field, "skillLocking"))
		{
			userBagInfo.skillLocking = atoi(value);
		}
		else if (!strcmp(field, "redBag"))
		{
			userBagInfo.redBag = atoi(value);
		}
		else if (!strcmp(field, "phoneBillCard1"))
		{
			userBagInfo.phoneBillCard1 = atoi(value);
		}
		else if (!strcmp(field, "phoneBillCard5"))
		{
			userBagInfo.phoneBillCard5 = atoi(value);
		}
		else if (!strcmp(field, "goldenArmor"))
		{
			userBagInfo.goldenArmor = atoi(value);
		}
		else if (!strcmp(field, "mechatroPioneer"))
		{
			userBagInfo.mechatroPioneer = atoi(value);
		}
		else if (!strcmp(field, "deepSeaArtillery"))
		{
			userBagInfo.deepSeaArtillery = atoi(value);
		}
		else if (!strcmp(field, "octopusCannon"))
		{
			userBagInfo.octopusCannon = atoi(value);
		}
		else if (!strcmp(field, "goldenDragon"))
		{
			userBagInfo.goldenDragon = atoi(value);
		}
		else if (!strcmp(field, "lavaArmor"))
		{
			userBagInfo.lavaArmor = atoi(value);
		}
	}

	freeReplyObject(pReply);

	return true;
}

int CRedisCommon::GetUserBagCount(int userID, const char * resName)
{
	if (userID <= 0 || !resName)
	{
		return 0;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d %s", TBL_USER_BAG, userID, resName);
	if (!pReply)
	{
		return 0;
	}

	int ret = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisCommon::SetUserBag(int userID, const char * resName, int changeResNums, bool bAdd)
{
	if (userID <= 0 || !resName)
	{
		ERROR_LOG("设置背包失败，userID=%d", userID);
		return false;
	}

	std::string key = MakeKey(TBL_USER_BAG, userID);
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	if (bAdd)
	{
		sprintf(redisCmd, "HINCRBY %s %s %d", key.c_str(), resName, changeResNums);

		long long llRet = 0;
		if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llRet, REDIS_EXTEND_MODE_UPDATE))
		{
			return false;
		}

		if (llRet < 0)
		{
			ERROR_LOG("######  修改资源数量超过限制 userID=%d,resName=%s,changeResNums=%d  #######", userID, resName, changeResNums);
			return false;
		}
	}
	else
	{
		sprintf(redisCmd, "HMSET %s %s %d", key.c_str(), resName, changeResNums);

		if (!InnerHMSetCommand(key.c_str(), redisCmd, REDIS_EXTEND_MODE_UPDATE))
		{
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CRedisCommon::GetFullPeopleMatchPeople(int gameID, int matchID, int peopleCount, std::vector<MatchUserInfo> &vecPeople)
{
	vecPeople.clear();

	if (gameID <= 0 || matchID <= 0 || peopleCount <= 0)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "ZRANGE %s|%d,%d 0 -1 WITHSCORES", TBL_SIGN_UP_MATCH_PEOPLE, gameID, matchID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements == 0 || elements % 2 != 0 || elements / 2 != peopleCount)
	{
		freeReplyObject(pReply);
		ERROR_LOG("人数错误：%d", elements / 2);
		return false;
	}

	for (int i = 0; i < elements; i += 2)
	{
		const char* id = pReply->element[i]->str;
		const char* time = pReply->element[i + 1]->str;

		MatchUserInfo user;

		user.userID = atoi(id);
		user.signUpTime = atoll(time);

		vecPeople.push_back(user);
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisCommon::GetTimeMatchPeople(int matchID, std::vector<MatchUserInfo> &vecPeople)
{
	vecPeople.clear();

	if (matchID <= 0)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "ZRANGE %s|%d 0 -1 WITHSCORES", TBL_SIGN_UP_TIME_MATCH_PEOPLE, matchID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements == 0 || elements % 2 != 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	for (int i = 0; i < elements; i += 2)
	{
		const char* id = pReply->element[i]->str;
		const char* time = pReply->element[i + 1]->str;

		MatchUserInfo user;

		user.userID = atoi(id);
		user.signUpTime = atoll(time);

		vecPeople.push_back(user);
	}

	freeReplyObject(pReply);

	return true;
}