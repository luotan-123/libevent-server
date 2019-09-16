#include "CommonHead.h"
#include "RedisLoader.h"

CRedisLoader::CRedisLoader()
{
}

CRedisLoader::~CRedisLoader()
{
}

bool CRedisLoader::Init()
{
	timeval tv = { 3, 0 };

	const RedisConfig& redisConfig = ConfigManage()->GetRedisConfig(REDIS_DATA_TYPE_GAME);

	m_pContext = redisConnectWithTimeout(redisConfig.ip, redisConfig.port, tv);
	if (!m_pContext)
	{
		return false;
	}

	if (m_pContext->err != 0)
	{
		return false;
	}

	const char* passwd = redisConfig.passwd;
	if (passwd[0] != '\0')
	{
		bool ret = Auth(passwd);
		return ret;
	}

	return true;
}

bool CRedisLoader::Stop()
{
	return true;
}

bool CRedisLoader::SetPrivateDeskCurrUserCount(int deskMixID, int userCount)
{
	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s currDeskUserCount %d", key.c_str(), userCount);
	if (!pReply)
	{
		ERROR_LOG("设置人数失败key=%s", key.c_str());
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

bool CRedisLoader::SetPrivateDeskCurrWatchUserCount(int deskMixID, int watchUserCount)
{
	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s currWatchUserCount %d", key.c_str(), watchUserCount);
	if (!pReply)
	{
		ERROR_LOG("设置旁观人数失败key=%s", key.c_str());
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

bool CRedisLoader::DelPrivateDeskRecord(int deskMixID)
{
	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);

	// 取密码
	char passwd[128] = "";

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGET %s passwd", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	if (pReply->type == REDIS_REPLY_STRING)
	{
		strcpy(passwd, pReply->str);
	}

	freeReplyObject(pReply);

	// 删除privateDeskInfo的记录
	DelKey(key.c_str());

	// 删除passwd与deskMixID的映射
	key = MakeKey(TBL_CACHE_DESK_PASSWD, passwd);
	DelKey(key.c_str());

	return true;
}

bool CRedisLoader::CleanRoomAllData(int roomID)
{
	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
	if (!pRoomBaseInfo)
	{
		return false;
	}

	if (pRoomBaseInfo->type == ROOM_TYPE_GOLD || pRoomBaseInfo->type == ROOM_TYPE_MATCH)
	{
		return true;
	}

	//  将房间标记重置
	SetMarkDeskIndex(roomID, 0);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	// 清除对应房间的桌子数据
	sprintf(redisCmd, "KEYS %s|%d*", TBL_CACHE_DESK, roomID);

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
		int deskMixID = 0;
		if (!ParseKey(key, tableName, deskMixID))
		{
			DelKey(key);
			continue;
		}

		if (deskMixID / MAX_ROOM_HAVE_DESK_COUNT != roomID)
		{
			continue;
		}

		PrivateDeskInfo privateDeskInfo;
		if (!GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
		{
			DelKey(key);
			ERROR_LOG("GetPrivateDeskRecordInfo failed deskMixID(%d)", deskMixID);
			continue;
		}

		// 清除这个桌子数据
		DelPrivateDeskRecord(deskMixID);

		int masterID = privateDeskInfo.masterID;
		if (masterID <= 0)
		{
			ERROR_LOG("清理房间失败，masterID=%d", masterID);
			continue;
		}

		// 房主的开房记录
		DelUserBuyDeskInfoInSet(masterID, privateDeskInfo.passwd);

		UserData masterData;
		if (!GetUserData(masterID, masterData))
		{
			ERROR_LOG("GetUserData failed userID(%d)", masterID);
			continue;
		}

		// 房主的开房次数
		if (privateDeskInfo.friendsGroupID == 0)
		{
			SetUserBuyingDeskCount(masterID, -1, true);
		}

		// 删除牌桌
		if (privateDeskInfo.friendsGroupID > 0)
		{
			DelFGDeskRoom(privateDeskInfo.friendsGroupID, privateDeskInfo.friendsGroupDeskNumber);
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisLoader::GetAllDesk(int roomID, std::vector<int>& vecRooms)
{
	if (roomID <= 0)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "KEYS %s|%d*", TBL_CACHE_DESK, roomID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* key = pReply->element[i]->str;
		if (!key)
		{
			ERROR_LOG("key is NULL");
			continue;
		}

		char tableName[64] = "";
		int deskMixID = 0;

		if (!ParseKey(key, tableName, deskMixID))
		{
			ERROR_LOG("Pasekey failed key=%s", key);
			continue;
		}

		if (deskMixID / MAX_ROOM_HAVE_DESK_COUNT != roomID)
		{
			continue;
		}

		int deskIdx = deskMixID % MAX_ROOM_HAVE_DESK_COUNT;
		vecRooms.push_back(deskIdx);
	}

	freeReplyObject(pReply);

	return true;
}

void CRedisLoader::DelUserBuyDeskInfoInSet(int userID, const char* passwd)
{
	if (!passwd)
	{
		return;
	}

	std::string key = MakeKey(CACHE_USER_BUYDESK_SET, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "srem %s %s", key.c_str(), passwd);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECK(pReply, redisCmd);
}

long long CRedisLoader::GetRoomMaxGradeIndex(int roomID)
{
	std::string key = MakeKey(TBL_MAX_GRADEKEY, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "INCR %s", key.c_str());

	long long ret = -1;
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	if (!pReply)
	{
		ERROR_LOG("执行命令失败redisCmd=%s", redisCmd);
		return ret;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = pReply->integer;
		if (ret >= MAX_ROOM_GRADE_COUNT)
		{
			ret = pReply->integer % MAX_ROOM_GRADE_COUNT;
		}
	}
	freeReplyObject(pReply);

	return ret;
}

long long CRedisLoader::GetRoomMaxSimpleGradeIndex(int roomID)
{
	std::string key = MakeKey(TBL_MAX_GRADESIMPLEKEY, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "INCR %s", key.c_str());

	long long ret = -1;
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	if (!pReply)
	{
		ERROR_LOG("执行命令失败redisCmd=%s", redisCmd);
		return ret;
	}

	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = pReply->integer;
		if (ret >= MAX_ROOM_GRADE_COUNT)
		{
			ret = pReply->integer % MAX_ROOM_GRADE_COUNT;
		}
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisLoader::SetUserWinCount(int userID)
{
	if (userID <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_USER, userID);

	// 保存胜局用户到集合
	redisReply* pReply = NULL;
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	sprintf(redisCmd, "HINCRBY %s winCount %d", key.c_str(), 1);
	pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);
	long long winCount = pReply->integer;

	freeReplyObject(pReply);

	sprintf(redisCmd, "HINCRBY %s totalGameWinCount %d", key.c_str(), 1);
	pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	//添加到胜局集合
	AddKeyToZSet(TBL_RANKING_WINCOUNT, winCount, userID);

	return true;
}

bool CRedisLoader::SetUserTotalGameCount(int userID)
{
	if (userID <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	sprintf(redisCmd, "HINCRBY %s totalGameCount %d", key.c_str(), 1);
	long long llValue = 0;

	if (!InnerHINCRBYCommand(key.c_str(), redisCmd, llValue))
	{
		return false;
	}

	return true;
}

bool CRedisLoader::SetPrivateDeskGrade(GameGradeInfo& gameGradeInfo, long long& gradeID)
{
	if (gameGradeInfo.videoCode[0] == '\0')
	{
		strcpy(gameGradeInfo.videoCode, REDIS_STR_DEFAULT);
	}
	if (gameGradeInfo.gameData[0] == '\0')
	{
		strcpy(gameGradeInfo.gameData, REDIS_STR_DEFAULT);
	}

	long long roomMaxGradeIndex = GetRoomMaxGradeIndex(gameGradeInfo.roomID);
	if (roomMaxGradeIndex < 0)
	{
		ERROR_LOG(" GetRoomMaxGradeIndex failed roomID=%d", gameGradeInfo.roomID);
		return false;
	}

	gradeID = gameGradeInfo.roomID * MAX_ROOM_GRADE_COUNT + roomMaxGradeIndex;

	std::string key = MakeKey(TBL_GRADE_DETAIL, gradeID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s id %lld roomID %d passwd %d masterID %d inning %d time %d videoCode %s userInfoList %s gameData %s",
		key.c_str(), gradeID, gameGradeInfo.roomID, gameGradeInfo.deskPasswd, gameGradeInfo.masterID, gameGradeInfo.inning, (int)gameGradeInfo.currTime, gameGradeInfo.videoCode, gameGradeInfo.userInfoList, gameGradeInfo.gameData);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

bool CRedisLoader::SetPrivateDeskSimpleInfo(const std::vector<int>& userIDVec, PrivateDeskGradeSimpleInfo& simpleInfo, const std::vector<long long>& gradeIDVec)
{
	if (simpleInfo.gameRules[0] == '\0')
	{
		strcpy(simpleInfo.gameRules, REDIS_STR_DEFAULT);
	}

	int roomID = simpleInfo.roomID;
	long long maxSimpleGradeID = GetRoomMaxSimpleGradeIndex(roomID);
	if (maxSimpleGradeID < 0)
	{
		ERROR_LOG("GetRoomMaxSimpleGradeIndex failed roomID=%d", roomID);
		return false;
	}

	long long simpleGradeID = roomID * MAX_ROOM_GRADE_COUNT + maxSimpleGradeID;

	std::string key = MakeKey(TBL_GRADE_SIMPLE, simpleGradeID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s id %lld roomID %d passwd %d masterID %d time %d gameCount %d maxGameCount %d gameRules %s userInfoList %s",
		key.c_str(), simpleGradeID, roomID, simpleInfo.passwd, simpleInfo.masterID, simpleInfo.time, simpleInfo.gameCount, simpleInfo.maxGameCount, simpleInfo.gameRules, simpleInfo.userInfoList);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	// 将单局战绩的ID添加入集合
	key = MakeKey(TBL_GRADE_SIMPLE_SET, simpleGradeID);
	sprintf(redisCmd, "SADD %s ", key.c_str());

	for (size_t i = 0; i < gradeIDVec.size(); i++)
	{
		sprintf(redisCmd + strlen(redisCmd), "%lld ", gradeIDVec[i]);
	}

	pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	// 将大结算战绩添加到玩家的集合
	int currTime = (int)time(NULL);
	for (size_t i = 0; i < userIDVec.size(); i++)
	{
		int userID = userIDVec[i];
		key = MakeKey(TBL_USER_GRADE_SET, userID);

		sprintf(redisCmd, "ZADD %s %d %lld", key.c_str(), currTime, simpleGradeID);

		pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
		REDIS_CHECKF(pReply, redisCmd);

		freeReplyObject(pReply);
	}

	return true;
}

//机器人索引
int CRedisLoader::GetRobotInfoIndex()
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "INCR %s", TBL_ROBOT_INFO_INDEX);
	if (!pReply)
	{
		return 0;
	}

	int ret = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = (int)pReply->integer;
	}
	freeReplyObject(pReply);

	return ret;
}

// 设置超时数据
bool CRedisLoader::SetPrivateDeskCheckTime(int deskMixID, int checkTime)
{
	if (deskMixID <= 0)
	{
		ERROR_LOG("invalid deskMixID deskMixID=%d", deskMixID);
		return false;
	}

	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);
	if (!IsKeyExists(key.c_str()))
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HSET %s checkTime %d", key.c_str(), checkTime);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	bool ret = false;
	if (pReply->type != REDIS_REPLY_ERROR && pReply->integer == 1)
	{
		ret = true;
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisLoader::SetPrivateDeskGameCount(int deskMixID, int gameCount)
{
	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);

	if (!IsKeyExists(key.c_str()))
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s curGameCount %d", key.c_str(), gameCount);
	if (!pReply)
	{
		ERROR_LOG("设置游戏局数失败 deskMixID=%d", deskMixID);
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

//开房列表中，添加或者删除人
bool CRedisLoader::SetPrivateDeskUserID(int deskMixID, int userID, int type)//0新增，1删除
{
	std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGET %s arrUserID", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	char strUserID[128] = "";
	if (pReply->type == REDIS_REPLY_STRING)
	{
		strcpy(strUserID, pReply->str);
	}
	freeReplyObject(pReply);

	long long arrUserID[MAX_PLAYER_GRADE] = { 0 };
	int arrUserIDCount = 0;
	CUtil::StringToArray(strUserID, arrUserID, arrUserIDCount);

	int iIndex = -1;
	for (int i = 0; i < arrUserIDCount; i++)
	{
		if (arrUserID[i] == userID)
		{
			iIndex = i;
			break;
		}
	}

	if (type == 0) //新增
	{
		if (iIndex != -1)
		{
			ERROR_LOG("房间中已经存在 userID=%d", userID);
			return false;
		}

		arrUserID[arrUserIDCount++] = userID;
	}
	else //删除
	{
		if (iIndex == -1)
		{
			ERROR_LOG("房间中不存在 userID=%d", userID);
			return false;
		}

		arrUserID[iIndex] = 0;
	}

	memset(strUserID, 0, sizeof(strUserID));
	CUtil::ArrayToString(arrUserID, arrUserIDCount, strUserID);

	std::unordered_map<std::string, std::string> umap;
	umap["arrUserID"] = strUserID;
	if (!hmset(TBL_CACHE_DESK, deskMixID, umap))
	{
		ERROR_LOG("开房列表，添加或者新增人数失败 deskMixID= %d , userID=%d，type=%d", deskMixID, userID, type);
		return false;
	}

	return true;
}

//设置当前房间人数
bool CRedisLoader::SetRoomServerPeopleCount(int roomID, int peopleCount)
{
	if (roomID <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_BASE_ROOM, roomID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s currPeopleCount %d", key.c_str(), peopleCount);
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

//关服保存桌子
bool CRedisLoader::SaveFGDeskRoom(const PrivateDeskInfo &privateDeskInfo)
{
	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(privateDeskInfo.roomID);
	if (!pRoomBaseInfo)
	{
		return false;
	}

	//保存牌桌数据
	std::unordered_map<std::string, std::string> umap;
	umap["userID"] = CUtil::Tostring(privateDeskInfo.masterID);
	umap["friendsGroupID"] = CUtil::Tostring(privateDeskInfo.friendsGroupID);
	umap["friendsGroupDeskNumber"] = CUtil::Tostring(privateDeskInfo.friendsGroupDeskNumber);
	umap["roomType"] = CUtil::Tostring(pRoomBaseInfo->type);
	umap["gameID"] = CUtil::Tostring(pRoomBaseInfo->gameID);
	umap["maxCount"] = CUtil::Tostring(privateDeskInfo.buyGameCount);
	umap["gameRules"] = privateDeskInfo.gameRules;

	char assistantKey[64] = "";
	sprintf(assistantKey, "%d,%d,%d", privateDeskInfo.masterID, privateDeskInfo.friendsGroupID, privateDeskInfo.friendsGroupDeskNumber);

	if (hmset(TBL_FG_CLOSE_SAVE_DESK, assistantKey, umap) == false)
	{
		ERROR_LOG("关服保存牌桌失败::assistantKey = %s", assistantKey);
		return false;
	}

	return true;
}

bool CRedisLoader::GetUserControlParam(int userID, int &value)
{
	value = -1;

	if (userID <= 0)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGET %s|%d controlParam", TBL_USER, userID);
	if (!pReply)
	{
		return false;
	}

	if (pReply->type == REDIS_REPLY_STRING)
	{
		value = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return true;
}

//////////////////////////////////////////////////////////////////////////

//获取局部比赛索引
long long CRedisLoader::GetPartOfMatchIndex()
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "INCR %s", TBL_PART_OF_MATCH_INDEX);
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

bool CRedisLoader::DelFullPeopleMatchPeople(int gameID, int matchID, const std::vector<MatchUserInfo> &vecPeople)
{
	std::vector<int> userIDVec;
	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		userIDVec.push_back(vecPeople[i].userID);
	}

	char key[48] = "";
	sprintf(key, "%s|%d,%d", TBL_SIGN_UP_MATCH_PEOPLE, gameID, matchID);

	ZremMember(key, userIDVec);

	return true;
}

bool CRedisLoader::SetUserMatchStatus(int userID, BYTE matchType, int matchStatus)
{
	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s matchType %d matchStatus %d", key.c_str(), matchType, matchStatus);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;
}

bool CRedisLoader::SetUserMatchRank(int userID, long long combineMatchID, int curMatchRank)
{
	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s combineMatchID %lld curMatchRank %d", key.c_str(), combineMatchID, curMatchRank);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;
}

bool CRedisLoader::SetTimeMatchPeopleRank(int matchID, const std::vector<MatchUserInfo> &vecPeople)
{
	if (matchID <= 0 || vecPeople.size() <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_SIGN_UP_TIME_MATCH_PEOPLE, matchID);

	//先删掉整个集合
	DelKey(key.c_str());

	int size = vecPeople.size();
	if (size > 0)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

		int iCount_ = (size - 1) / 50;
		for (int index_ = 0; index_ <= iCount_; index_++)
		{
			sprintf(redisCmd, "ZADD %s ", key.c_str());

			for (int i = index_ * 50; i < size && i < (index_ + 1) * 50; i++)
			{
				sprintf(redisCmd + strlen(redisCmd), "%d %d ", i + 1, vecPeople[i].userID);
			}

			redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
			REDIS_CHECKF(pReply, redisCmd);

			freeReplyObject(pReply);
		}
	}

	return true;
}

//获取机器人userID
int CRedisLoader::GetRobotUserID()
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "INCR %s", TBL_MATCH_ROBOT_USERID_INDEX);
	if (!pReply)
	{
		return 0;
	}

	int ret = 0;
	if (pReply->type == REDIS_REPLY_INTEGER)
	{
		ret = (int)pReply->integer;
	}
	freeReplyObject(pReply);

	return (ret % (MAX_ROBOT_USERID - MIN_ROBOT_USERID + 1)) + MIN_ROBOT_USERID;
}
//////////////////////////////////////////////////////////////////////////