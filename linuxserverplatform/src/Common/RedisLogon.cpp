#include "CommonHead.h"
#include "RedisLogon.h"	

CRedisLogon::CRedisLogon()
{
	m_uLogonGroupIndex = 0;
	m_uLogonGroupCount = 1;
	m_uMainLogonGroupIndex = 0;
}

CRedisLogon::~CRedisLogon()
{
}

bool CRedisLogon::Init()
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

bool CRedisLogon::Stop()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}

	return true;
}

// 获取redis Context
redisContext* CRedisLogon::GetRedisContext()
{
	return m_pContext;
}

int CRedisLogon::Register(const UserData& userData, BYTE registerType)
{
	//新玩家ID
	int newUserID = GetCurMaxUserID() + 1;

	std::string key = MakeKey(TBL_USER, newUserID);

	// 同步数据
	CRedisLock redisLock(m_pContext, key.c_str(), 5);

	if (IsKeyExists(key.c_str()))
	{
		SetCurMaxUserID(newUserID + 1);
		ERROR_LOG("用户已经存在:%s", key.c_str());
		return -1;
	}

	std::string strAccount = userData.account;
	std::string strPwd = userData.passwd;
	char temp[64] = { 0 };
	if (registerType == LOGON_WEICHAT || registerType == LOGON_QQ)
	{
		sprintf(temp, "Phone%d", newUserID);
		strAccount = temp;
	}
	else if (registerType == LOGON_TEL_PHONE)  //手机注册
	{
		sprintf(temp, "%s", userData.phone);
		strAccount = temp;
	}
	else if (registerType == LOGON_VISITOR)    //游客注册
	{
		// 填充账号
		sprintf(temp, "Visitor%d", newUserID);
		strAccount = temp;

		// 填充名字
		sprintf(temp, "游客%d", newUserID);
	}

	std::unordered_map<std::string, std::string> umap;

	umap["userID"] = CUtil::Tostring(newUserID);
	umap["account"] = strAccount;
	umap["passwd"] = userData.passwd;
	umap["name"] = registerType == LOGON_VISITOR ? temp : userData.name;
	umap["phone"] = userData.phone;
	umap["sex"] = CUtil::Tostring(userData.sex);
	umap["mail"] = userData.mail;
	umap["money"] = CUtil::Tostring(userData.money);
	umap["bankMoney"] = CUtil::Tostring(userData.bankMoney);
	umap["bankPasswd"] = userData.bankPasswd;
	umap["jewels"] = CUtil::Tostring(userData.jewels);
	umap["roomID"] = CUtil::Tostring(userData.roomID);
	umap["deskIdx"] = CUtil::Tostring(userData.deskIdx);
	umap["logonIP"] = userData.logonIP;
	umap["headURL"] = userData.headURL;
	umap["winCount"] = CUtil::Tostring(userData.winCount);
	umap["macAddr"] = userData.macAddr;
	umap["token"] = userData.token;
	umap["isVirtual"] = CUtil::Tostring(userData.isVirtual);
	umap["status"] = CUtil::Tostring(userData.status);
	umap["reqSupportTimes"] = CUtil::Tostring(userData.reqSupportTimes);
	umap["lastReqSupportDate"] = CUtil::Tostring(userData.lastReqSupportDate);
	umap["registerTime"] = CUtil::Tostring(userData.registerTime);
	umap["registerIP"] = userData.registerIP;
	umap["registerType"] = CUtil::Tostring(userData.registerType);
	umap["buyingDeskCount"] = CUtil::Tostring(userData.buyingDeskCount);
	umap["lastCrossDayTime"] = CUtil::Tostring(userData.lastCrossDayTime);
	umap["totalGameCount"] = CUtil::Tostring(userData.totalGameCount);
	umap["sealFinishTime"] = CUtil::Tostring(userData.sealFinishTime);
	umap["wechat"] = userData.wechat;
	umap["phonePasswd"] = userData.phonePasswd;
	umap["accountInfo"] = userData.accountInfo;
	umap["totalGameWinCount"] = CUtil::Tostring(userData.totalGameWinCount);
	umap["Lng"] = userData.Lng;
	umap["Lat"] = userData.Lat;
	umap["address"] = userData.address;
	umap["lastCalcOnlineToTime"] = CUtil::Tostring(userData.lastCalcOnlineToTime);
	umap["allOnlineToTime"] = CUtil::Tostring(userData.allOnlineToTime);
	umap["IsOnline"] = CUtil::Tostring(userData.IsOnline);
	umap["motto"] = userData.motto;
	umap["xianliao"] = userData.xianliao;
	umap["controlParam"] = CUtil::Tostring(userData.controlParam);
	umap["ModifyInformationCount"] = CUtil::Tostring(userData.ModifyInformationCount);
	umap["matchType"] = CUtil::Tostring(userData.matchType);
	umap["combineMatchID"] = CUtil::Tostring(userData.combineMatchID);
	umap["matchStatus"] = CUtil::Tostring(userData.matchStatus);
	umap["curMatchRank"] = CUtil::Tostring(userData.curMatchRank);

	if (!hmset(TBL_USER, newUserID, umap))
	{
		ERROR_LOG("hmset userinfo user(%d) failed", newUserID);
		return -2;
	}

	//建立背包表
	std::unordered_map<std::string, std::string> mmap;
	mmap["userID"] = CUtil::Tostring(newUserID);
	mmap["skillFrozen"] = CUtil::Tostring(0);
	mmap["skillLocking"] = CUtil::Tostring(0);
	mmap["redBag"] = CUtil::Tostring(0);
	mmap["phoneBillCard1"] = CUtil::Tostring(0);
	mmap["phoneBillCard5"] = CUtil::Tostring(0);
	mmap["goldenArmor"] = CUtil::Tostring(0);
	mmap["mechatroPioneer"] = CUtil::Tostring(0);
	mmap["deepSeaArtillery"] = CUtil::Tostring(0);
	mmap["octopusCannon"] = CUtil::Tostring(0);
	mmap["goldenDragon"] = CUtil::Tostring(0);
	mmap["lavaArmor"] = CUtil::Tostring(0);

	if (!hmset(TBL_USER_BAG, newUserID, mmap))
	{
		ERROR_LOG("hmset userbag user(%d) failed", newUserID);
		return -2;
	}
	redisLock.Unlock();

	FixAccountIndexInfo(strAccount.c_str(), userData.passwd, newUserID, userData.registerType);

	return newUserID;
}

int CRedisLogon::GetRelevanceTrdUid(std::string triID)
{
	std::string key = MakeKey(TBL_TRDUSERID, triID.c_str());

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "GET %s", key.c_str());

	redisReply * pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int userID = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		userID = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return userID;
}

int CRedisLogon::GetVisitorID(std::string triID)
{
	std::string key = MakeKey(TBL_VISITOR_TOUSERID, triID.c_str());

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "GET %s", key.c_str());

	redisReply * pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int userID = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		userID = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return userID;
}

int CRedisLogon::GetDeskMixIDByPasswd(const char* passwd)
{
	if (!passwd)
	{
		return 0;
	}

	std::string key = MakeKey(TBL_CACHE_DESK_PASSWD, passwd);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "GET %s", key.c_str());

	redisReply * pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int ret = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return ret;
}

std::string CRedisLogon::CreatePrivateDeskRecord(int userID, int roomID, BYTE masterNotPlay, int maxDeskCount, int buyGameCount, const char* pGameRules, int maxDeskUserCount, int payType, int maxWatchUserCount, int friendsGroupID, int friendsGroupDeskNumber)
{
	if (!pGameRules)
	{
		return "";
	}

	int loop = maxDeskCount;
	int deskIndex = GetMarkDeskIndex(roomID);

	while (loop >= 0)
	{
		loop--;
		deskIndex++;

		if (deskIndex >= maxDeskCount)
		{
			deskIndex = 0;
		}

		int deskMixID = roomID * MAX_ROOM_HAVE_DESK_COUNT + deskIndex;
		std::string key = MakeKey(TBL_CACHE_DESK, deskMixID);
		time_t currTime = time(NULL);

		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

		// 同步数据
		CRedisLock redisLock(m_pContext, key.c_str(), 10);

		if (IsKeyExists(key.c_str()))
		{
			continue;
		}

		// 随机生成6位的密码
		std::string passwd = GetRandUnRepeatedDeskPasswd();
		if (passwd == "")
		{
			return "";
		}

		sprintf(redisCmd, "HMSET %s roomID %d deskIdx %d masterID %d passwd %s buyGameCount %d gameRules %s createTime %lld masterNotPlay %d payType %d currDeskUserCount 0 maxDeskUserCount %d currWatchUserCount 0 maxWatchUserCount %d checkTime %lld friendsGroupID %d friendsGroupDeskNumber %d curGameCount 0",
			key.c_str(), roomID, deskIndex, userID, passwd.c_str(), buyGameCount, pGameRules[0] != '\0' ? pGameRules : REDIS_STR_DEFAULT, currTime, masterNotPlay, payType, maxDeskUserCount, maxWatchUserCount, currTime, friendsGroupID, friendsGroupDeskNumber);

		// 创建一条记录
		redisReply* pReply = (redisReply *)redisCommand(m_pContext, redisCmd);
		REDIS_CHECKF(pReply, redisCmd);
		freeReplyObject(pReply);

		// 设置索引
		SetMarkDeskIndex(roomID, deskIndex);

		// 解锁
		redisLock.Unlock();

		// 构建deskPasswd和deskMixID的映射
		std::string deskPasswdKey = MakeKey(TBL_CACHE_DESK_PASSWD, passwd.c_str());
		sprintf(redisCmd, "SET %s %d", deskPasswdKey.c_str(), deskMixID);

		pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
		REDIS_CHECKF(pReply, redisCmd);
		freeReplyObject(pReply);

		// 加入玩家开房列表
		AddUserBuyDeskSet(userID, passwd);

		return passwd;
	}

	return "";
}

int CRedisLogon::GetMarkDeskIndex(int roomID)
{
	std::string key = MakeKey(TBL_MARK_DESK_INDEX, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "GET %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int ret = 0;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	return ret;
}

bool CRedisLogon::SetUserInfo(int userID, std::string strLine)
{
	if (strLine.empty())
	{
		return false;
	}

	std::string key = MakeKey(TBL_USER, userID);

	std::string redisCmd = "HMSET " + key;
	redisCmd += strLine;
	if (!InnerHMSetCommand(key.c_str(), redisCmd.c_str(), REDIS_EXTEND_MODE_UPDATE))
	{
		return false;
	}

	return true;
}

void CRedisLogon::RountineSaveRedisDataToDB(bool updateAll)
{
	if (!m_pDBManage)
	{
		return;
	}

	const int MAX_SAVE_DATA_COUNT = 72;
	int iSaveCount = 0;

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "SMEMBERS %s", CACHE_UPDATE_SET);
	if (pReply == NULL)
	{
		return;
	}

	for (size_t i = 0; i < pReply->elements; i++)
	{
		if (!updateAll && iSaveCount >= MAX_SAVE_DATA_COUNT) //每次最多处理数据量
		{
			break;
		}

		const char* key = pReply->element[i]->str;
		if (!key)
		{
			continue;
		}

		//分布式处理
		if (!IsDistributedSystemCalculate(CUtil::BKDRHash(key)))
		{
			continue;
		}

		char tableName[64] = "";
		int id = 0;

		// 是哈希的key必然有tablename和id
		if (!ParseKey(key, tableName, id))
		{
			ERROR_LOG("解析key失败::key=%s", key);
			SremMember(CACHE_UPDATE_SET, key);
			continue;
		}

		int extendMode = GetExtendMode(key);
		if (extendMode <= 0)
		{
			SremMember(CACHE_UPDATE_SET, key);
			continue;
		}

		if (!SaveRedisDataToDB(key, tableName, id, extendMode))
		{
			ERROR_LOG("定时保存数据失败:key=%s", key);
		}

		// 将处理的玩家从集合中删除TODO
		SremMember(CACHE_UPDATE_SET, key);

		iSaveCount++;
	}

	if (pReply->elements >= 1000)
	{
		INFO_LOG("总数据:%d，共保存数据:%d", pReply->elements, iSaveCount);
	}

	freeReplyObject(pReply);
}

bool CRedisLogon::SaveRedisDataToDB(const char* key, const char* tableName, int id, int mode)
{
	if (!key || !tableName || id <= 0)
	{
		ERROR_LOG("invalid inputparam key is NULL or tableName is NULL or id <= 0, key=%s tableName=%s id=%d", key, tableName, id);
		return false;
	}

	if (mode <= REDIS_EXTEND_MODE_DEFAULT || mode >= REDIS_EXTEND_MODE_END)
	{
		ERROR_LOG("invalid mode: mode=%d", mode);
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HGETALL %s", key);
	if (!pReply)
	{
		ERROR_LOG("获取redis数据失败,key=%s", key);
		return false;
	}

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements <= 0)
	{
		ERROR_LOG("invalid elements: elements=%d", elements);
		freeReplyObject(pReply);
		return false;
	}

	char sql[MAX_SQL_STATEMENT_SIZE] = "";
	const char* dbKey = ConfigManage()->GetTablePriamryKey(tableName);
	if (!dbKey)
	{
		ERROR_LOG("dbKey Error");
		freeReplyObject(pReply);
		return false;
	}

	if (mode == REDIS_EXTEND_MODE_UPDATE)
	{
		sprintf(sql, "UPDATE %s SET ", tableName);
	}
	else if (mode == REDIS_EXTEND_MODE_INSERT)
	{
		sprintf(sql, "INSERT INTO %s ", tableName);
	}

	m_vecFields.clear();
	char account[129] = ""; // 账号
	char name[129] = ""; // 特殊字符转义保存到数据库，注意：一张表中不能有两个name

	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		// 根据tableName和filed获取value的类型
		int valueType = ConfigManage()->GetFieldType(tableName, field);
		if (valueType <= FIELD_VALUE_TYPE_NONE || valueType >= FIELD_VALUE_TYPE_END)
		{
			ERROR_LOG("redis数据字段错误 tableName=%s, field=%s, key=%s", tableName, field, key);
			continue;
		}

		// 特殊字段跳过
		if (!strcmp(field, "extendMode"))
		{
			continue;
		}
		else if (!strcmp(field, "motto")) // UTF-8字符串跳过
		{
			continue;
		}

		FieldRealInfo fieldInfo;

		fieldInfo.field = field;
		fieldInfo.type = valueType;

		if (!strcmp(field, "name")) //转义插入，有个性签名的补上个性签名
		{
			memcpy(name, value, Min_(strlen(value), sizeof(name)));

			CUtil::TransString(name, sizeof(name), strlen(name));
			fieldInfo.value = name;
		}
		else if (!strcmp(field, "account")) //转义插入
		{
			memcpy(account, value, Min_(strlen(value), sizeof(account)));

			CUtil::TransString(account, sizeof(account), strlen(account));
			fieldInfo.value = account;
		}
		else
		{
			fieldInfo.value = value;
		}

		m_vecFields.push_back(fieldInfo);
	}

	if (mode == REDIS_EXTEND_MODE_UPDATE)
	{
		for (size_t i = 0; i < m_vecFields.size(); i++)
		{
			FieldRealInfo& fieldInfo = m_vecFields[i];

			sprintf(sql + strlen(sql), " %s=", fieldInfo.field);

			if (fieldInfo.type == FIELD_VALUE_TYPE_INT || fieldInfo.type == FIELD_VALUE_TYPE_CHAR)
			{
				sprintf(sql + strlen(sql), "%d, ", atoi(fieldInfo.value));
			}
			else if (fieldInfo.type == FIELD_VALUE_TYPE_LONGLONG)
			{
				sprintf(sql + strlen(sql), "%lld, ", atoll(fieldInfo.value));
			}
			else if (fieldInfo.type == FIELD_VALUE_TYPE_STR)
			{
				sprintf(sql + strlen(sql), "'%s', ", fieldInfo.value);
			}
		}

		// 最后的那个逗号
		int pos = strlen(sql) - 2;
		if (sql[pos] == ',')
		{
			sql[pos] = ' ';
		}

		sprintf(sql + strlen(sql), "WHERE %s=%d", dbKey, id);
	}
	else if (mode == REDIS_EXTEND_MODE_INSERT)
	{
		sprintf(sql + strlen(sql), "%s", "( ");

		for (int i = 0; i < (int)m_vecFields.size(); i++)
		{
			FieldRealInfo& fieldInfo = m_vecFields[i];

			sprintf(sql + strlen(sql), "%s, ", fieldInfo.field);
		}

		int pos = strlen(sql) - 2;
		if (sql[pos] == ',')
		{
			sql[pos] = ' ';
		}

		sprintf(sql + strlen(sql), "%s", ") VALUES( ");

		for (int i = 0; i < (int)m_vecFields.size(); i++)
		{
			FieldRealInfo& fieldInfo = m_vecFields[i];

			if (fieldInfo.type == FIELD_VALUE_TYPE_INT || fieldInfo.type == FIELD_VALUE_TYPE_CHAR)
			{
				sprintf(sql + strlen(sql), "%d, ", atoi(fieldInfo.value));
			}
			else if (fieldInfo.type == FIELD_VALUE_TYPE_LONGLONG)
			{
				sprintf(sql + strlen(sql), "%lld, ", atoll(fieldInfo.value));
			}
			else if (fieldInfo.type == FIELD_VALUE_TYPE_STR)
			{
				sprintf(sql + strlen(sql), "'%s', ", fieldInfo.value);
			}
		}

		pos = strlen(sql) - 2;
		if (sql[pos] == ',')
		{
			sql[pos] = ' ';
		}

		sprintf(sql + strlen(sql), "%s", ") ");
	}

	// 将sql语句push到异步处理线程
	InternalSqlStatement msg;

	memcpy(msg.sql, sql, sizeof(sql));

	m_pDBManage->PushLine(&msg.head, sizeof(InternalSqlStatement), DTK_GP_SQL_STATEMENT, 0, 0);

	freeReplyObject(pReply);

	return true;
}

void CRedisLogon::ClearGradeInfo()
{
	std::vector<int> vecRoomID;
	auto iter = ConfigManage()->m_roomBaseInfoMap.begin();
	for (; iter != ConfigManage()->m_roomBaseInfoMap.end(); iter++)
	{
		const RoomBaseInfo& roomBaseInfo = iter->second;
		if (roomBaseInfo.type == ROOM_TYPE_GOLD || roomBaseInfo.type == ROOM_TYPE_MATCH)
		{
			continue;
		}
		vecRoomID.push_back(roomBaseInfo.roomID);
	}

	//分布式处理
	for (size_t i = 0; i < vecRoomID.size(); i++)
	{
		if (!IsDistributedSystemCalculate(i))
		{
			continue;
		}
		ClearGradeSimpleInfoByRoomID(vecRoomID[i]);
		ClearGradeDetailInfoByRoomID(vecRoomID[i]);
	}
}

std::string CRedisLogon::GetRandUnRepeatedDeskPasswd()
{
	const int loop = 50;			// 最大循环次数

	for (int i = 0; i < loop; i++)
	{
		int randNum = GetRandDigit(6);
		char buf[10] = "";
		sprintf(buf, "%d", randNum);

		std::string key = MakeKey(TBL_CACHE_DESK_PASSWD, buf);
		if (!IsKeyExists(key.c_str()))
		{
			return std::string(buf);
		}
	}

	return "";
}

void CRedisLogon::ClearGradeSimpleInfoByRoomID(int roomID)
{
	// 获取战绩清理最小id
	std::string clearKey = MakeKey(TBL_GRADE_SIMPLE_MIN_ID, roomID);
	long long minClearIndex = GetKeyIndex(clearKey.c_str());
	if (minClearIndex >= MAX_ROOM_GRADE_COUNT)
	{
		minClearIndex = minClearIndex % MAX_ROOM_GRADE_COUNT;
	}

	std::vector<long long> expireVec;
	time_t currTime = time(NULL);
	while (true)
	{
		long long gradeID = roomID * MAX_ROOM_GRADE_COUNT + minClearIndex;

		PrivateDeskGradeSimpleInfo info;
		if (!GetGradeSimpleInfo(gradeID, info))
		{
			break;
		}

		if (currTime - info.time <= GRADE_SIMPLE_EXPIRE_TIME)
		{
			break;
		}

		expireVec.push_back(gradeID);

		minClearIndex++;
		if (minClearIndex >= MAX_ROOM_GRADE_COUNT)
		{
			minClearIndex = minClearIndex % MAX_ROOM_GRADE_COUNT;
		}
	}

	SetKeyIndex(clearKey.c_str(), minClearIndex);

	for (size_t i = 0; i < expireVec.size(); i++)
	{
		//删除大结算战绩
		std::string key = MakeKey(TBL_GRADE_SIMPLE, expireVec[i]);
		DelKey(key.c_str());

		//删除大结算集合
		key = MakeKey(TBL_GRADE_SIMPLE_SET, expireVec[i]);
		DelKey(key.c_str());
	}
}

void CRedisLogon::ClearGradeDetailInfoByRoomID(int roomID)
{
	// 获取战绩清理最小id
	std::string clearKey = MakeKey(TBL_GRADE_DETAIL_MIN_ID, roomID);
	long long minClearIndex = GetKeyIndex(clearKey.c_str());
	if (minClearIndex >= MAX_ROOM_GRADE_COUNT)
	{
		minClearIndex = minClearIndex % MAX_ROOM_GRADE_COUNT;
	}

	std::vector<long long> expireVec;
	time_t currTime = time(NULL);
	while (true)
	{
		long long gradeID = roomID * MAX_ROOM_GRADE_COUNT + minClearIndex;

		GameGradeInfo info;
		if (!GetGradeDetailInfo(gradeID, info))
		{
			break;
		}

		if (currTime - info.currTime <= GRADE_EXPIRE_TIME)
		{
			break;
		}

		expireVec.push_back(gradeID);

		minClearIndex++;
		if (minClearIndex >= MAX_ROOM_GRADE_COUNT)
		{
			minClearIndex = minClearIndex % MAX_ROOM_GRADE_COUNT;
		}
	}

	SetKeyIndex(clearKey.c_str(), minClearIndex);

	for (size_t i = 0; i < expireVec.size(); i++)
	{
		//删除小结算战绩
		std::string key = MakeKey(TBL_GRADE_DETAIL, expireVec[i]);
		DelKey(key.c_str());
	}
}

void CRedisLogon::ClearUserGradeInfo(int userID)
{
	std::string key = MakeKey(TBL_USER_GRADE_SET, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "ZRANGE %s 0 -1", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECK(pReply, redisCmd);

	std::vector<long long> expireVec;

	// 如果某个记录不存在了，那么就是被删了
	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* value = pReply->element[i]->str;
		if (!IsKeyExists(TBL_GRADE_SIMPLE, value))
		{
			expireVec.push_back(atoll(value));
		}
		else
		{
			break;
		}
	}
	freeReplyObject(pReply);

	ZremMember(key.c_str(), expireVec);
}

bool CRedisLogon::GetGradeSimpleInfo(long long id, PrivateDeskGradeSimpleInfo& simpleInfo)
{
	std::string key = MakeKey(TBL_GRADE_SIMPLE, id);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	size_t elements = pReply->elements;
	if (elements == 0 || elements % 2 != 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	for (size_t i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "id"))
		{
			simpleInfo.id = atoll(value);
		}
		else if (!strcmp(field, "roomID"))
		{
			simpleInfo.roomID = atoi(value);
		}
		else if (!strcmp(field, "passwd"))
		{
			simpleInfo.passwd = atoi(value);
		}
		else if (!strcmp(field, "masterID"))
		{
			simpleInfo.masterID = atoi(value);
		}
		else if (!strcmp(field, "gameRules"))
		{
			memcpy(simpleInfo.gameRules, value, Min_(strlen(value), sizeof(simpleInfo.gameRules)));
		}
		else if (!strcmp(field, "time"))
		{
			simpleInfo.time = atoi(value);
		}
		else if (!strcmp(field, "gameCount"))
		{
			simpleInfo.gameCount = atoi(value);
		}
		else if (!strcmp(field, "maxGameCount"))
		{
			simpleInfo.maxGameCount = atoi(value);
		}
		else if (!strcmp(field, "userInfoList"))
		{
			memcpy(simpleInfo.userInfoList, value, Min_(strlen(value), sizeof(simpleInfo.userInfoList)));
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisLogon::GetGradeDetailInfo(long long id, GameGradeInfo& gameGradeInfo)
{
	std::string key = MakeKey(TBL_GRADE_DETAIL, id);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	if (pReply->elements == 0 || pReply->elements % 2 != 0)
	{
		freeReplyObject(pReply);
		return false;
	}

	for (size_t i = 0; i < pReply->elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "id"))
		{
			gameGradeInfo.id = atoll(value);
		}
		else if (!strcmp(field, "inning"))
		{
			gameGradeInfo.inning = atoi(value);
		}
		else if (!strcmp(field, "time"))
		{
			gameGradeInfo.currTime = atoi(value);
		}
		else if (!strcmp(field, "videoCode"))
		{
			strcpy(gameGradeInfo.videoCode, value);
		}
		else if (!strcmp(field, "userInfoList"))
		{
			memcpy(gameGradeInfo.userInfoList, value, Min_(strlen(value), sizeof(gameGradeInfo.userInfoList)));
		}
		else if (!strcmp(field, "gameData"))
		{
			memcpy(gameGradeInfo.gameData, value, Min_(strlen(value), sizeof(gameGradeInfo.gameData)));
		}
	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisLogon::ClearAllUserWinCount()
{
	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "ZRANGE %s 0 -1", TBL_RANKING_WINCOUNT);
	if (pReply == NULL)
	{
		ERROR_LOG("清理每周胜局数失败");
		return false;
	}

	char cmd[MAX_REDIS_COMMAND_SIZE] = "";
	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* key = pReply->element[i]->str;
		if (!key)
		{
			continue;
		}

		int userID = atoi(key);
		if (userID <= 0)
		{
			continue;
		}

		sprintf(cmd, "HMSET %s|%d winCount 0", TBL_USER, userID);

		if (!InnerHMSetCommand("", cmd))
		{
			ERROR_LOG("每周清理胜局数失败::key=%s", key);
		}
	}

	freeReplyObject(pReply);

	// 清理胜局
	DelKey(TBL_RANKING_WINCOUNT);

	return true;
}

void CRedisLogon::AddUserBuyDeskSet(int userID, const std::string& passwd)
{
	std::string key = MakeKey(CACHE_USER_BUYDESK_SET, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "sadd %s %s", key.c_str(), passwd.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECK(pReply, redisCmd);
}

bool CRedisLogon::ClearOneDayWinMoney()
{
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "KEYS %s|*", TBL_REWARDS_POOL);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* key = pReply->element[i]->str;
		if (!key)
		{
			continue;
		}

		char szTable[64] = "";
		int roomID = 0;
		if (!ParseKey(key, szTable, roomID))
		{
			continue;
		}

		// 分布式处理
		if (!IsDistributedSystemCalculate(roomID))
		{
			continue;
		}

		RewardsPoolInfo poolInfo;
		if (!GetRewardsPoolInfo(roomID, poolInfo))
		{
			continue;
		}

		char cmd[MAX_REDIS_COMMAND_SIZE] = "";
		sprintf(cmd, "HMSET %s allGameWinMoney %lld allPercentageWinMoney %lld allOtherWinMoney %lld",
			key, poolInfo.allGameWinMoney + poolInfo.gameWinMoney, poolInfo.allPercentageWinMoney + poolInfo.percentageWinMoney, poolInfo.allOtherWinMoney + poolInfo.otherWinMoney);

		if (!InnerHMSetCommand(key, cmd))
		{
			return false;
		}

		if (!SetRoomPoolData(roomID, "gameWinMoney", -poolInfo.gameWinMoney, true))
		{
			ERROR_LOG("清理奖池失败");
		}

		if (!SetRoomPoolData(roomID, "percentageWinMoney", -poolInfo.percentageWinMoney, true))
		{
			ERROR_LOG("清理奖池失败");
		}

		if (!SetRoomPoolData(roomID, "otherWinMoney", -poolInfo.otherWinMoney, true))
		{
			ERROR_LOG("清理奖池失败");
		}

	}

	freeReplyObject(pReply);

	return true;
}

bool CRedisLogon::CountOneDayPoolInfo()
{
	time_t currTime = time(NULL);

	for (auto itr = ConfigManage()->m_roomBaseInfoMap.begin(); itr != ConfigManage()->m_roomBaseInfoMap.end(); itr++)
	{
		int roomID = itr->second.roomID;

		// 分布式处理
		if (!IsDistributedSystemCalculate(roomID))
		{
			continue;
		}

		RewardsPoolInfo poolInfo;
		if (!GetRewardsPoolInfo(roomID, poolInfo))
		{
			continue;
		}

		if (roomID <= 0)
		{
			continue;
		}

		// 生成每天的奖池记录
		BillManage()->WriteBill(m_pDBManage, "INSERT INTO %s (time,roomID,poolMoney,gameWinMoney,percentageWinMoney,otherWinMoney) VALUES(%lld,%d,%lld,%lld,%lld,%lld)",
			TBL_STATI_REWARDS_POOL, currTime, roomID, poolInfo.poolMoney, poolInfo.gameWinMoney, poolInfo.percentageWinMoney, poolInfo.otherWinMoney);
	}

	return true;
}

// 通过手机号获取用户id
int CRedisLogon::GetUserIDByPhone(const char* phone)
{
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	std::string key = MakeKey(TBL_PHONE_TOUSERID, phone);

	sprintf(redisCmd, "GET %s", key.c_str());

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

int  CRedisLogon::GetCurMaxUserID()
{
	std::string key = TBL_CURMAX_USERID;

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "GET %s", key.c_str());

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int ret = 118000;
	if (pReply->type == REDIS_REPLY_STRING)
	{
		ret = atoi(pReply->str);
	}

	freeReplyObject(pReply);

	SetCurMaxUserID(ret + 1 + (CUtil::GetRandNum() % 10));

	return ret;
}

// 闲聊登陆
int CRedisLogon::GetUserIDByXianLiao(const char* xianliao)
{
	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	std::string key = MakeKey(TBL_XIANLIAO_TOUSERID, xianliao);

	sprintf(redisCmd, "GET %s", key.c_str());

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

//判断是否是当前主要集群系统
bool CRedisLogon::IsMainDistributedSystem()
{
	if (m_uLogonGroupIndex == m_uMainLogonGroupIndex)
	{
		return true;
	}

	return false;
}

//这个id是否当前系统计算
bool CRedisLogon::IsDistributedSystemCalculate(long long calcID)
{
	if (m_uLogonGroupCount <= 0)
	{
		ERROR_LOG("########### 重大错误m_uLogonGroupCount=%d##############", m_uLogonGroupCount);
		return false;
	}

	if (calcID % m_uLogonGroupCount == m_uLogonGroupIndex)
	{
		return true;
	}

	return false;
}

// 设置登录服人数
bool CRedisLogon::SetLogonServerCurrPeopleCount(int logonID, int peopleCount)
{
	if (logonID <= 0)
	{
		return false;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "HSET %s|%d curPeople %d", TBL_BASE_LOGON, logonID, peopleCount);
	if (!pReply)
	{
		return false;
	}

	freeReplyObject(pReply);

	return true;
}

int CRedisLogon::IsCanCreateFriendsGroupRoom(int userID, int friendsGroupID, BYTE userPower, int friendsGroupDeskNumber)
{
	BYTE managerPower = friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT ? FRIENDSGROUP_POWER_TYPE_VIPROOM : FRIENDSGROUP_POWER_TYPE_DESK;
	// 判断权限
	if (managerPower != (userPower & managerPower))
	{
		return 1;
	}

	if (friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT + MAX_FRIENDSGROUP_VIP_ROOM_COUNT || friendsGroupDeskNumber <= 0)
	{
		return 2; //牌桌号码错误
	}
	else
	{
		char combinationKey[128] = "";
		sprintf(combinationKey, "%s|%d,%d", TBL_FG_ROOM_INFO, friendsGroupID, friendsGroupDeskNumber);
		int iDeskIndex = (int)GetKeyIndex(combinationKey);

		if (iDeskIndex >= MAX_ROOM_HAVE_DESK_COUNT)
		{
			if (IsKeyExists(TBL_CACHE_DESK, iDeskIndex)) //牌桌已经存在不能创建
			{
				return 2;
			}
			else
			{
				DelFGDeskRoom(friendsGroupID, friendsGroupDeskNumber);
			}
		}
	}

	return 0;
}

bool CRedisLogon::CreateFriendsGroupDeskInfo(int friendsGroupID, int friendsGroupDeskNumber, const std::string &deskPasswd,
	int gameID, int roomType, OneFriendsGroupDeskInfo &deskInfo, bool &bHaveRedSpot)
{
	// 根据唯一的密码查询私有桌子
	int deskMixID = GetDeskMixIDByPasswd(deskPasswd.c_str());
	if (deskMixID <= 0)
	{
		ERROR_LOG("创建牌桌失败 failed passwd=%s", deskPasswd.c_str());
		return false;
	}

	// 根据deskMixID取出桌子的数据
	PrivateDeskInfo privateDeskInfo;
	if (!GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		ERROR_LOG("创建牌桌失败 GetPrivateDeskRecordInfo failed deskMixID %d", deskMixID);
		return false;
	}

	char combinationKey[128] = "";
	sprintf(combinationKey, "%s|%d,%d", TBL_FG_ROOM_INFO, friendsGroupID, friendsGroupDeskNumber);

	if (!IsKeyExists(combinationKey))
	{
		bHaveRedSpot = true;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "SET %s %d", combinationKey, deskMixID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	deskInfo.friendsGroupID = friendsGroupID;
	deskInfo.friendsGroupDeskNumber = friendsGroupDeskNumber;
	deskInfo.time = (int)time(NULL);
	deskInfo.roomID = privateDeskInfo.roomID;
	deskInfo.gameID = gameID;
	deskInfo.roomType = roomType;
	deskInfo.curPeopleCount = 0;
	deskInfo.allPeopleCount = privateDeskInfo.maxDeskUserCount;
	deskInfo.playCount = privateDeskInfo.buyGameCount;
	deskInfo.gameStatus = 0;
	memcpy(deskInfo.passwd, privateDeskInfo.passwd, sizeof(privateDeskInfo.passwd));
	memcpy(deskInfo.gameRules, privateDeskInfo.gameRules, sizeof(deskInfo.gameRules));

	return true;
}

bool CRedisLogon::GetTempFgDesk(const char * asskey, SaveRedisFriendsGroupDesk & desk)
{
	if (asskey == NULL || strlen(asskey) == 0)
	{
		return false;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HGETALL %s", asskey);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	int elements = pReply->elements;
	if (elements % 2 != 0 || elements == 0)
	{
		ERROR_LOG("获取关服redis数据出错，键:%s", asskey);
		freeReplyObject(pReply);
		return false;
	}

	for (int i = 0; i < elements; i += 2)
	{
		const char* field = pReply->element[i]->str;
		const char* value = pReply->element[i + 1]->str;

		if (!strcmp(field, "userID"))
		{
			desk.userID = atoi(value);
		}
		else if (!strcmp(field, "friendsGroupID"))
		{
			desk.friendsGroupID = atoi(value);
		}
		else if (!strcmp(field, "friendsGroupDeskNumber"))
		{
			desk.friendsGroupDeskNumber = atoi(value);
		}
		else if (!strcmp(field, "roomType"))
		{
			desk.roomType = atoi(value);
		}
		else if (!strcmp(field, "gameID"))
		{
			desk.gameID = atoi(value);
		}
		else if (!strcmp(field, "maxCount"))
		{
			desk.maxCount = atoi(value);
		}
		else if (!strcmp(field, "gameRules"))
		{
			memcpy(desk.gameRules, value, Min_(sizeof(desk.gameRules), strlen(value)));
		}
	}

	freeReplyObject(pReply);

	return true;
}

int CRedisLogon::GetAllTempFgDesk(std::vector<SaveRedisFriendsGroupDesk>& vecFGDesk)
{
	if (!IsMainDistributedSystem())
	{
		return 0;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "KEYS %s|*", TBL_FG_CLOSE_SAVE_DESK);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	for (size_t i = 0; i < pReply->elements; i++)
	{
		const char* key = pReply->element[i]->str;

		if (!key)
		{
			continue;
		}

		SaveRedisFriendsGroupDesk desk;
		if (GetTempFgDesk(key, desk))
		{
			vecFGDesk.push_back(desk);
		}
		else
		{
			ERROR_LOG("获取关服保存牌桌数据失败，key=%s", key);
		}

		//删除数据
		DelKey(key);
	}

	int iAllCount = pReply->elements;

	freeReplyObject(pReply);

	return iAllCount;
}

int CRedisLogon::GetFGDeskMixID(const char * asskey)
{
	redisReply* pReply = (redisReply *)redisCommand(m_pContext, "GET %s", asskey);
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

bool CRedisLogon::ClearAllUserRanking(const char* ranking)
{
	if (!ranking)
	{
		return false;
	}

	int iCount = GetZSetSize(ranking);

	if (iCount <= MAX_RANK_COUNT || MAX_RANK_COUNT <= 0)
	{
		return true;
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, "ZREMRANGEBYRANK %s 0 %d", ranking, iCount - MAX_RANK_COUNT - 1);
	if (!pReply)
	{
		return false;
	}

	freeReplyObject(pReply);

	return true;
}