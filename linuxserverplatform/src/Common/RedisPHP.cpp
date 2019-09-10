#include "pch.h"
#include "RedisPHP.h"	
#include "InternalMessageDefine.h"
#include "log.h"
#include "Util.h"
#include "BillManage.h"

CRedisPHP::CRedisPHP()
{

}

CRedisPHP::~CRedisPHP()
{
}

bool CRedisPHP::Init()
{
	timeval tv = { 3, 0 };

	const RedisConfig& redisConfig = ConfigManage()->GetRedisConfig(REDIS_DATA_TYPE_PHP);

	m_pContext = redisConnectWithTimeout(redisConfig.ip, redisConfig.port, tv);
	if (!m_pContext)
	{
		return false;
	}

	if (m_pContext->err != 0)
	{
		return false;
	}

	bool ret = false;

	const char* passwd = redisConfig.passwd;
	if (passwd[0] != '\0')
	{
		ret = Auth(passwd);
		if (!ret)
		{
			ERROR_LOG("redis auth failed");
			return false;
		}
	}

	return true;
}

bool CRedisPHP::Stop()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}

	return true;
}

// 获取redis Context
redisContext* CRedisPHP::GetRedisContext()
{
	return m_pContext;
}

// 获取俱乐部成员
bool CRedisPHP::GetFGMember(int friendsGroupID, std::vector<int> &memberUserIDVec)
{
	memberUserIDVec.clear();

	std::string key = MakeKey(TBL_FG_TO_USER_SET, friendsGroupID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "ZRANGE %s 0 -1", key.c_str());
	if (!pReply)
	{
		ERROR_LOG("获取俱乐部成员失败,friendsGroupID=%d", friendsGroupID);
		return false;
	}

	// 如果某个记录不存在了，那么就是被删了
	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* value = pReply->element[i]->str;
		memberUserIDVec.push_back(atoi(value));
	}

	freeReplyObject(pReply);

	return true;
}

// 获取玩家俱乐部金币
bool CRedisPHP::GetUserFriendsGroupMoney(int friendsGroupID, int userID, long long &money)
{
	if (friendsGroupID <= 0 || userID <= 0)
	{
		ERROR_LOG("获取玩家火币失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	money = 0;

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d,%d carryFireCoin", TBL_FRIENDSGROUPTOUSER, friendsGroupID, userID);
	if (!pReply)
	{
		ERROR_LOG("获取玩家火币失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		money = _atoi64(pReply->str);
	}
	else
	{
		ERROR_LOG("获取玩家火币失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		freeReplyObject(pReply);
		return false;
	}

	freeReplyObject(pReply);

	return true;
}

// 设置玩家俱乐部金币
bool CRedisPHP::SetUserFriendsGroupMoney(int friendsGroupID, int userID, long long money, bool bAdd/* = true*/, int reason/* = 0*/, int roomID/* = 0*/, int rateFireCoin/* = 0*/)
{
	if (friendsGroupID <= 0 || userID <= 0)
	{
		ERROR_LOG("设置玩家火币失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	char combinationKey[128] = "";
	sprintf(combinationKey, "%s|%d,%d", TBL_FRIENDSGROUPTOUSER, friendsGroupID, userID);

	if (!IsKeyExists(combinationKey))
	{
		ERROR_LOG("设置玩家火币失败,key=%s", combinationKey);
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	long long changeValue = 0, afterValue = 0;

	if (bAdd)
	{
		if (money == 0) //不增不减
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s carryFireCoin %lld", combinationKey, money);
		changeValue = money;
	}
	else
	{
		sprintf(redisCmd, "HMSET %s carryFireCoin %lld", combinationKey, money);
		changeValue = afterValue = money;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	if (bAdd && pReply->type == REDIS_REPLY_INTEGER)
	{
		afterValue = pReply->integer;
	}

	freeReplyObject(pReply);

	// 账单 当前金币|此次变化|原因
	BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (userID,time,fireCoin,changeFireCoin,reason,roomID,friendsGroupID,rateFireCoin) VALUES(%d,%d,%lld,%lld,%d,%d,%d,%d)",
		TBL_STATI_USER_FIRECOIN_CHANGE, userID, (int)time(NULL), afterValue, changeValue, reason, roomID, friendsGroupID, rateFireCoin);

	return true;
}

// 获取玩家权限
int CRedisPHP::GetUserPower(int friendsGroupID, int userID)
{
	if (friendsGroupID <= 0 || userID <= 0)
	{
		ERROR_LOG("获取玩家权限失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d,%d power", TBL_USERTOFRIENDSGROUP, userID, friendsGroupID);
	if (!pReply)
	{
		ERROR_LOG("获取玩家权限失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	int ret = FRIENDSGROUP_POWER_TYPE_NO;

	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}
	else
	{
		ERROR_LOG("获取玩家权限失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
	}

	freeReplyObject(pReply);

	return ret;
}

// 获取俱乐部群主
int CRedisPHP::GetFriendsGroupMasterID(int friendsGroupID)
{
	if (friendsGroupID <= 0)
	{
		ERROR_LOG("获取俱乐部群主失败,friendsGroupID=%d", friendsGroupID);
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d masterID", TBL_FRIENDSGROUP, friendsGroupID);
	if (!pReply)
	{
		ERROR_LOG("获取俱乐部群主失败,friendsGroupID=%d", friendsGroupID);
		return false;
	}

	int ret = 0;

	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}
	else
	{
		ERROR_LOG("获取俱乐部群主失败,friendsGroupID=%d", friendsGroupID);
	}

	freeReplyObject(pReply);

	return ret;
}

// 判断是否可以加入俱乐部
bool CRedisPHP::IsCanJoinFriendsGroupRoom(int userID, int friendsGroupID)
{
	// 判断俱乐部是否存在
	if (IsKeyExists(TBL_FRIENDSGROUP, friendsGroupID) && IsKeyExists(TBL_FRIENDSGROUPTOUSER, friendsGroupID, userID))
	{
		return true;
	}

	return false;
}

//  设置玩家俱乐部资源数量
bool CRedisPHP::SetUserFriendsGroupResNums(int friendsGroupID, int userID, const char * resName, long long resNums, bool bAdd/* = false*/)
{
	if (friendsGroupID <= 0 || userID <= 0 || !resName)
	{
		ERROR_LOG("设置玩家火币失败,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	char combinationKey[128] = "";
	sprintf(combinationKey, "%s|%d,%d", TBL_FRIENDSGROUPTOUSER, friendsGroupID, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	if (bAdd)
	{
		if (resNums == 0) //不增不减
		{
			return true;
		}
		sprintf(redisCmd, "HINCRBY %s %s %lld", combinationKey, resName, resNums);
	}
	else
	{
		sprintf(redisCmd, "HMSET %s %s %lld", combinationKey, resName, resNums);
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

// 获取通知时间
bool CRedisPHP::GetFGNotifySendTime(long long llIndex, time_t &sendTime)
{
	if (llIndex <= 0)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%lld time", TBL_FG_NOTIFY, llIndex);
	if (!pReply)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		sendTime = _atoi64(pReply->str);
	}
	else
	{
		freeReplyObject(pReply);
		return false;
	}

	freeReplyObject(pReply);

	return true;
}

// 清理玩家俱乐部通知
void CRedisPHP::ClearUserFGNotifySet(int userID)
{
	std::string key = MakeKey(TBL_USER_FG_NOTIFY_SET, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "ZRANGE %s 0 -1", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECK(pReply, redisCmd);

	std::vector<long long> expireVec;

	// 如果某个记录不存在了，那么就是被删了
	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* value = pReply->element[i]->str;
		if (!IsKeyExists(TBL_FG_NOTIFY, value))
		{
			expireVec.push_back(_atoi64(value));
		}
		else
		{
			break;
		}
	}
	freeReplyObject(pReply);

	ZremMember(key.c_str(), expireVec);
}

//  清理所有通知
void CRedisPHP::ClearAllFGNotifyInfo()
{
	long long minIndex = GetKeyIndex(TBL_FG_NOTIFY_CLEAR_ID);
	long long maxIndex = GetKeyIndex(TBL_FG_NOTIFY_ID);
	int currTime = (int)time(NULL);

	while (minIndex <= maxIndex)
	{
		time_t sendTime = 0;
		if (!GetFGNotifySendTime(minIndex, sendTime))
		{
			minIndex++;
			continue;
		}

		if (currTime - sendTime <= GROUP_NOTIY_MSG_EXPIRE_TIME)
		{
			break;
		}

		std::string key = MakeKey(TBL_FG_NOTIFY, minIndex);
		DelKey(key.c_str());

		minIndex++;
	}

	SetKeyIndex(TBL_FG_NOTIFY_CLEAR_ID, minIndex <= maxIndex ? minIndex : maxIndex);
}

//获取俱乐部绑定的管理员ID
bool CRedisPHP::GetfriendsGroupToUser(int friendsGroupID, int userID, int &ChouShui, int&ManagerId)
{
	if (friendsGroupID <= 0 || userID <= 0)
	{
		ERROR_LOG("获取俱乐部绑定的管理员ID,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}


	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d,%d inviter_userid", TBL_FRIENDSGROUPTOUSER, friendsGroupID, userID);
	if (!pReply)
	{
		ERROR_LOG("获取俱乐部绑定的管理员ID,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		ManagerId = atoi(pReply->str);
	}
	else
	{
		ERROR_LOG("获取俱乐部绑定的管理员ID,friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		freeReplyObject(pReply);
		return false;
	}

	freeReplyObject(pReply);
	if (ManagerId > 0)
	{
		pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d,%d ratio", TBL_FRIENDSGROUPTOUSER, friendsGroupID, ManagerId);
		if (!pReply)
		{
			ERROR_LOG("获取俱乐部抽水失败,friendsGroupID=%d,userID=%d", friendsGroupID, ManagerId);
			return false;
		}

		if (pReply->type == REDIS_REPLY_STRING)
		{
			ChouShui = atoi(pReply->str);
		}
		else
		{
			ERROR_LOG("获取俱乐部抽水失败,friendsGroupID=%d,userID=%d", friendsGroupID, ManagerId);
			freeReplyObject(pReply);
			return false;
		}

		freeReplyObject(pReply);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CRedisPHP::GetEmailSendTime(long long llIndex, int &sendTime)
{
	if (llIndex <= 0)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%lld sendtime", TBL_EMAIL_DETAIL, llIndex);
	if (!pReply)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		sendTime = atoi(pReply->str);
	}
	else
	{
		freeReplyObject(pReply);
		return false;
	}

	freeReplyObject(pReply);

	return true;
}

//获取用户所有邮件id
bool CRedisPHP::GetUserAllEmailID(int userID, std::vector<EmailSimpleInfo> &simpleEmailID, bool Asc/* = true*/)
{
	if (userID <= 0)
	{
		ERROR_LOG("invalid emailID userID = %d", userID);
		return false;
	}

	std::string key = MakeKey(TBL_USER_EMAIL_SET, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	if (Asc)
	{
		sprintf(redisCmd, "ZRANGE %s 0 -1 WITHSCORES", key.c_str());
	}
	else
	{
		sprintf(redisCmd, "ZREVRANGE %s 0 -1 WITHSCORES", key.c_str());
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements == 0)
	{
		// 没有任何邮件正常
		freeReplyObject(pReply);
		return true;
	}
	if (elements % 2 != 0)
	{
		ERROR_LOG("获取用户所有邮件IDinvalid elements elements=%d, userID=%s", elements, userID);
		freeReplyObject(pReply);
		return false;
	}

	for (int i = 0; i < elements; i += 2)
	{
		const char* id = pReply->element[i]->str;
		const char* time = pReply->element[i + 1]->str;

		EmailSimpleInfo simpleEmail;

		simpleEmail.emailID = _atoi64(id);
		simpleEmail.sendtime = atoi(time);

		simpleEmailID.push_back(simpleEmail);
	}

	freeReplyObject(pReply);

	return true;
}

//删除用户的一封邮件
bool CRedisPHP::DelUserEmailInfo(int userID, long long emailID)
{
	if (emailID <= 0 || userID <= 0)
	{
		ERROR_LOG("invalid emailID userID = %d , emailID=%lld", userID, emailID);
		return false;
	}

	//移除集合
	std::string key = MakeKey(TBL_USER_EMAIL_SET, userID);
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "ZREM %s %lld", key.c_str(), emailID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	//先删信息
	char assistantKey[48] = "";
	sprintf(assistantKey, "%ld,%lld", userID, emailID);
	key = MakeKey(TBL_USER_EMAIL_DETAIL, assistantKey);
	DelKey(key.c_str());

	return true;
}

//清理某个用户的邮件集合
void CRedisPHP::ClearUserEmailSet(int userID)
{
	int currTime = (int)time(NULL);

	std::vector<long long> expireVec;
	std::vector<EmailSimpleInfo> simpleEmailID;
	GetUserAllEmailID(userID, simpleEmailID);

	for (size_t i = 0; i < simpleEmailID.size(); i++)
	{
		if (currTime - simpleEmailID[i].sendtime <= EMAIL_SIMPLE_EXPIRE_TIME)
		{
			break;
		}

		expireVec.push_back(simpleEmailID[i].emailID);
	}

	for (size_t i = 0; i < expireVec.size(); i++)
	{
		DelUserEmailInfo(userID, expireVec[i]);
	}
}

//清理过期邮件
void CRedisPHP::ClearAllEmailInfo()
{
	long long minIndex = GetKeyIndex(TBL_EMAIL_INFO_MIN_ID);
	long long maxIndex = GetKeyIndex(TBL_EMAIL_INFO_MAX_ID);
	int currTime = (int)time(NULL);

	while (minIndex <= maxIndex)
	{
		int sendTime = 0;
		if (!GetEmailSendTime(minIndex, sendTime))
		{
			minIndex++;
			continue;
		}

		if (currTime - sendTime <= EMAIL_EXPIRE_TIME)
		{
			break;
		}

		std::string key = MakeKey(TBL_EMAIL_DETAIL, minIndex);
		DelKey(key.c_str());

		minIndex++;
	}

	SetKeyIndex(TBL_EMAIL_INFO_MIN_ID, minIndex <= maxIndex ? minIndex : maxIndex);
}

//////////////////////////////////////////////////////////////////////////
// 增加资源值，只能设置数值类型
long long CRedisPHP::AddUserResNums(int userID, const char * resName, long long changeResNums)
{
	if (userID <= 0 || !resName)
	{
		return 0;
	}

	std::string key = MakeKey(TBL_WEB_USER, userID);
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HINCRBY %s %s %lld", key.c_str(), resName, changeResNums);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	long long ret = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = pReply->integer;
	}

	freeReplyObject(pReply);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool CRedisPHP::LoadAllMatchInfo(std::map<long long, MatchInfo> & matchInfoMap)
{
	matchInfoMap.clear();

	// 清除对应房间的玩家数据
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	sprintf(redisCmd, "KEYS %s|*", TBL_TIME_MATCH_INFO);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* key = pReply->element[i]->str;
		if (!key)
		{
			continue;
		}

		char tableName[64] = "";
		int matchID = 0;
		if (!ParseKey(key, tableName, matchID))
		{
			ERROR_LOG("解析比赛信息失败：key=%s", key);
			DelKey(key);
			continue;
		}

		MatchInfo matchInfo;
		if (!GetMatchInfo(matchID, matchInfo))
		{
			ERROR_LOG("获取比赛信息失败：key=%s", key);
			DelKey(key);
			continue;
		}

		if (matchInfo.matchStatus == MATCH_STATUS_SIGN_UP)
		{
			//加入到内存
			matchInfoMap.insert(std::make_pair(matchInfo.startTime, matchInfo));
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisPHP::GetMatchInfo(int matchID, MatchInfo& matchInfo)
{
	if (matchID <= 0)
	{
		ERROR_LOG("invalid userID matchID=%d", matchID);
		return false;
	}

	std::string key = MakeKey(TBL_TIME_MATCH_INFO, matchID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		ERROR_LOG("invalid elements elements=%d, matchID=%d", elements, matchID);
		freeReplyObject(pReply);
		return false;
	}

	// 可否做成配置，太烦了 TODO
	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "matchID"))
		{
			matchInfo.matchID = atoi(value);
		}
		else if (!strcmp(field, "gameID"))
		{
			matchInfo.gameID = atoi(value);
		}
		else if (!strcmp(field, "minPeople"))
		{
			matchInfo.minPeople = atoi(value);
		}
		else if (!strcmp(field, "costResType"))
		{
			matchInfo.costResType = atoi(value);
		}
		else if (!strcmp(field, "costResNums"))
		{
			matchInfo.costResNums = atoi(value);
		}
		else if (!strcmp(field, "startTime"))
		{
			matchInfo.startTime = _atoi64(value);
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisPHP::SetMatchStatus(int matchID, BYTE matchStatus)
{
	if (matchID <= 0)
	{
		ERROR_LOG("invalid userID matchID=%d", matchID);
		return false;
	}

	std::string key = MakeKey(TBL_TIME_MATCH_INFO, matchID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s matchStatus %d", key.c_str(), matchStatus);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}