#include "CommonHead.h"
#include "RedisCenter.h"	

CRedisCenter::CRedisCenter()
{

}

CRedisCenter::~CRedisCenter()
{
}

bool CRedisCenter::Init()
{
	AUTOCOSTONCE("CRedisCenter::Init() 耗时");

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

	int iRetCode = NeedLoadAllUserData();
	if (iRetCode == 1)
	{
		INFO_LOG("加载全部用户数据。。。");
		ret = LoadAllUserData();
		if (!ret)
		{
			ERROR_LOG("LoadAllUserData failed");
			return false;
		}

		INFO_LOG("加载所有代理。。。");
		ret = LoadAllAgentUser();
		if (!ret)
		{
			ERROR_LOG("LoadAllAgentUser failed");
			return false;
		}

		INFO_LOG("加载全部背包数据。。。");
		ret = LoadAllUserBag();
		if (!ret)
		{
			ERROR_LOG("LoadAllUserBag failed");
			return false;
		}
	}
	else if (iRetCode > 1)
	{
		ERROR_LOG("NeedLoadAllUserData failed");
		return false;
	}
	else
	{
		INFO_LOG("===当前系统不需要加载，全部用户数据和代理数据===");
	}

	ret = LoadAllRewardsPoolData();
	if (!ret)
	{
		ERROR_LOG("LoadAllRewardsPoolData failed");
		return false;
	}

	ret = LoadAllConfig();
	if (!ret)
	{
		ERROR_LOG("LoadAllConfig failed");
		return false;
	}

	return true;
}

bool CRedisCenter::Stop()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}

	return true;
}

int CRedisCenter::NeedLoadAllUserData()	//判断是否需要加载数据库中所有用户数据到redis
{
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig(DB_TYPE_COMMON);

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return 2;
	}

	char buf[128] = "";

	// 加载所有的玩家数据
	sprintf(buf, "select * from %s limit 5", TBL_USER);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(buf, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return 2;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		UserData userData;

		CConfigManage::sqlGetValue(dataSet[i], "userID", userData.userID);

		// 保存到redis
		std::string key = MakeKey(TBL_USER, userData.userID);

		// redis中已经存在了就不加载
		if (!IsKeyExists(key.c_str()))
		{
			return 1;
		}
	}

	return 0;
}

bool CRedisCenter::LoadAllUserData()
{
	AUTOCOSTONCE("LoadAllUserData 耗时");

	int currMaxUserID = -1;
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig(DB_TYPE_COMMON);

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return false;
	}

	char buf[MAX_SQL_STATEMENT_SIZE] = "";

	// 加载所有的玩家数据
	sprintf(buf, "select * from %s", TBL_USER);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(buf, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		UserData userData;

		CConfigManage::sqlGetValue(dataSet[i], "userID", userData.userID);
		CConfigManage::sqlGetValue(dataSet[i], "account", userData.account, sizeof(userData.account));
		CConfigManage::sqlGetValue(dataSet[i], "passwd", userData.passwd, sizeof(userData.passwd));
		CConfigManage::sqlGetValue(dataSet[i], "name", userData.name, sizeof(userData.name));
		CConfigManage::sqlGetValue(dataSet[i], "sex", userData.sex);
		CConfigManage::sqlGetValue(dataSet[i], "mail", userData.mail, sizeof(userData.mail));
		CConfigManage::sqlGetValue(dataSet[i], "phone", userData.phone, sizeof(userData.phone));
		CConfigManage::sqlGetValue(dataSet[i], "money", userData.money);
		CConfigManage::sqlGetValue(dataSet[i], "bankMoney", userData.bankMoney);
		CConfigManage::sqlGetValue(dataSet[i], "bankPasswd", userData.bankPasswd, sizeof(userData.bankPasswd));
		CConfigManage::sqlGetValue(dataSet[i], "jewels", userData.jewels);
		CConfigManage::sqlGetValue(dataSet[i], "roomID", userData.roomID);
		CConfigManage::sqlGetValue(dataSet[i], "deskIdx", userData.deskIdx);
		CConfigManage::sqlGetValue(dataSet[i], "logonIP", userData.logonIP, sizeof(userData.logonIP));
		CConfigManage::sqlGetValue(dataSet[i], "headURL", userData.headURL, sizeof(userData.headURL));
		CConfigManage::sqlGetValue(dataSet[i], "macAddr", userData.macAddr, sizeof(userData.macAddr));
		CConfigManage::sqlGetValue(dataSet[i], "token", userData.token, sizeof(userData.token));
		CConfigManage::sqlGetValue(dataSet[i], "isVirtual", userData.isVirtual);
		CConfigManage::sqlGetValue(dataSet[i], "status", userData.status);
		CConfigManage::sqlGetValue(dataSet[i], "reqSupportTimes", userData.reqSupportTimes);
		CConfigManage::sqlGetValue(dataSet[i], "lastReqSupportDate", userData.lastReqSupportDate);
		CConfigManage::sqlGetValue(dataSet[i], "winCount", userData.winCount);
		CConfigManage::sqlGetValue(dataSet[i], "registerTime", userData.registerTime);
		CConfigManage::sqlGetValue(dataSet[i], "registerIP", userData.registerIP, sizeof(userData.registerIP));
		CConfigManage::sqlGetValue(dataSet[i], "registerType", userData.registerType);
		CConfigManage::sqlGetValue(dataSet[i], "buyingDeskCount", userData.buyingDeskCount);
		CConfigManage::sqlGetValue(dataSet[i], "lastCrossDayTime", userData.lastCrossDayTime);
		CConfigManage::sqlGetValue(dataSet[i], "totalGameCount", userData.totalGameCount);
		CConfigManage::sqlGetValue(dataSet[i], "sealFinishTime", userData.sealFinishTime);
		CConfigManage::sqlGetValue(dataSet[i], "phonePasswd", userData.phonePasswd, sizeof(userData.phonePasswd));
		CConfigManage::sqlGetValue(dataSet[i], "wechat", userData.wechat, sizeof(userData.wechat));
		CConfigManage::sqlGetValue(dataSet[i], "accountInfo", userData.accountInfo, sizeof(userData.accountInfo));
		CConfigManage::sqlGetValue(dataSet[i], "totalGameWinCount", userData.totalGameWinCount);
		CConfigManage::sqlGetValue(dataSet[i], "Lng", userData.Lng, sizeof(userData.Lng));
		CConfigManage::sqlGetValue(dataSet[i], "Lat", userData.Lat, sizeof(userData.Lat));
		CConfigManage::sqlGetValue(dataSet[i], "address", userData.address, sizeof(userData.address));
		CConfigManage::sqlGetValue(dataSet[i], "lastCalcOnlineToTime", userData.lastCalcOnlineToTime);
		CConfigManage::sqlGetValue(dataSet[i], "allOnlineToTime", userData.allOnlineToTime);
		CConfigManage::sqlGetValue(dataSet[i], "IsOnline", userData.IsOnline);
		CConfigManage::sqlGetValue(dataSet[i], "motto", userData.motto, sizeof(userData.motto));
		CConfigManage::sqlGetValue(dataSet[i], "xianliao", userData.xianliao, sizeof(userData.xianliao));
		CConfigManage::sqlGetValue(dataSet[i], "controlParam", userData.controlParam);
		CConfigManage::sqlGetValue(dataSet[i], "ModifyInformationCount", userData.ModifyInformationCount);
		CConfigManage::sqlGetValue(dataSet[i], "matchType", userData.matchType);
		CConfigManage::sqlGetValue(dataSet[i], "combineMatchID", userData.combineMatchID);
		CConfigManage::sqlGetValue(dataSet[i], "matchStatus", userData.matchStatus);
		CConfigManage::sqlGetValue(dataSet[i], "curMatchRank", userData.curMatchRank);

		// 保存到redis
		std::string key = MakeKey(TBL_USER, userData.userID);

		FixAccountIndexInfo(userData.account, userData.passwd, userData.userID, userData.registerType);

		//绑定手机号，绑定手机号才需要此功能，手机注册功能不需要
		if (!userData.isVirtual)
		{
			SetUserPhone(userData.userID, userData.phone);
		}

		//绑定闲聊号
		if (!userData.isVirtual)
		{
			SetUserXianLiao(userData.userID, userData.xianliao);
		}

		if (userData.winCount > 0)
		{
			AddKeyToZSet(TBL_RANKING_WINCOUNT, userData.winCount, userData.userID);
		}

		// redis中已经存在了就不加载
		if (IsKeyExists(key.c_str()))
		{
			continue;
		}

		std::unordered_map<std::string, std::string> umap;

		umap["userID"] = std::to_string(userData.userID);
		umap["account"] = userData.account;
		umap["passwd"] = userData.passwd;
		umap["name"] = userData.name;
		umap["phone"] = userData.phone;
		umap["sex"] = std::to_string(userData.sex);
		umap["mail"] = userData.mail;
		umap["money"] = std::to_string(userData.money);
		umap["bankMoney"] = std::to_string(userData.bankMoney);
		umap["bankPasswd"] = userData.bankPasswd;
		umap["jewels"] = std::to_string(userData.jewels);
		umap["roomID"] = "0";//std::to_string(userData.roomID);
		umap["deskIdx"] = std::to_string(INVALID_DESKIDX);//userData.deskIdx);
		umap["logonIP"] = userData.logonIP;
		umap["headURL"] = userData.headURL;
		umap["winCount"] = std::to_string(userData.winCount);
		umap["macAddr"] = userData.macAddr;
		umap["token"] = userData.token;
		umap["isVirtual"] = std::to_string(userData.isVirtual);
		umap["status"] = std::to_string(userData.status);
		umap["reqSupportTimes"] = std::to_string(userData.reqSupportTimes);
		umap["lastReqSupportDate"] = std::to_string(userData.lastReqSupportDate);
		umap["registerTime"] = std::to_string(userData.registerTime);
		umap["registerIP"] = userData.registerIP;
		umap["registerType"] = std::to_string(userData.registerType);
		umap["buyingDeskCount"] = std::to_string(userData.buyingDeskCount);
		umap["lastCrossDayTime"] = std::to_string(userData.lastCrossDayTime);
		umap["totalGameCount"] = std::to_string(userData.totalGameCount);
		umap["sealFinishTime"] = std::to_string(userData.sealFinishTime);
		umap["phonePasswd"] = userData.phonePasswd;
		umap["wechat"] = userData.wechat;
		umap["accountInfo"] = userData.accountInfo;
		umap["totalGameWinCount"] = std::to_string(userData.totalGameWinCount);
		umap["Lng"] = userData.Lng;
		umap["Lat"] = userData.Lat;
		umap["address"] = userData.address;
		umap["lastCalcOnlineToTime"] = std::to_string(userData.lastCalcOnlineToTime);
		umap["allOnlineToTime"] = std::to_string(userData.allOnlineToTime);
		umap["IsOnline"] = std::to_string(userData.IsOnline);
		umap["motto"] = userData.motto;
		umap["xianliao"] = userData.xianliao;
		umap["controlParam"] = std::to_string(userData.controlParam);
		umap["ModifyInformationCount"] = std::to_string(userData.ModifyInformationCount);
		umap["matchType"] = std::to_string(userData.matchType);
		umap["combineMatchID"] = std::to_string(userData.combineMatchID);
		umap["matchStatus"] = std::to_string(userData.matchStatus);
		umap["curMatchRank"] = std::to_string(userData.curMatchRank);

		if (hmset(TBL_USER, userData.userID, umap) == false)
		{
			ERROR_LOG("初始化失败 hmset userinfo to redis failed user(%d)", userData.userID);
			return false;
		}

		if (userData.userID > currMaxUserID)
		{
			currMaxUserID = userData.userID;
		}
	}

	if (currMaxUserID > 0 && currMaxUserID < 118000)
	{
		currMaxUserID = 118000;
	}

	SetCurMaxUserID(currMaxUserID);

	INFO_LOG("load all userData finished maxUserID=%d", currMaxUserID);

	return true;
}

bool CRedisCenter::LoadAllRewardsPoolData()
{
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig(DB_TYPE_COMMON);

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return false;
	}

	char buf[MAX_SQL_STATEMENT_SIZE] = "";

	for (auto itr = ConfigManage()->m_roomBaseInfoMap.begin(); itr != ConfigManage()->m_roomBaseInfoMap.end(); itr++)
	{
		sprintf(buf, "INSERT INTO %s(roomID) SELECT %d FROM dual WHERE not exists (select * from %s where %s.roomID = %d);",
			TBL_REWARDS_POOL, itr->first, TBL_REWARDS_POOL, TBL_REWARDS_POOL, itr->first);
		try
		{
			mysqlHelper.sqlExec(buf);
		}
		catch (MysqlHelper_Exception& excep)
		{
			ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
			continue;
		}
	}

	// 加载所有的玩家数据
	sprintf(buf, "select * from %s", TBL_REWARDS_POOL);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(buf, dataSet);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		RewardsPoolInfo poolData;

		CConfigManage::sqlGetValue(dataSet[i], "roomID", poolData.roomID);
		CConfigManage::sqlGetValue(dataSet[i], "poolMoney", poolData.poolMoney);
		CConfigManage::sqlGetValue(dataSet[i], "gameWinMoney", poolData.gameWinMoney);
		CConfigManage::sqlGetValue(dataSet[i], "percentageWinMoney", poolData.percentageWinMoney);
		CConfigManage::sqlGetValue(dataSet[i], "otherWinMoney", poolData.otherWinMoney);
		CConfigManage::sqlGetValue(dataSet[i], "allGameWinMoney", poolData.allGameWinMoney);
		CConfigManage::sqlGetValue(dataSet[i], "allPercentageWinMoney", poolData.allPercentageWinMoney);
		CConfigManage::sqlGetValue(dataSet[i], "allOtherWinMoney", poolData.allOtherWinMoney);
		CConfigManage::sqlGetValue(dataSet[i], "platformCtrlType", poolData.platformCtrlType);
		CConfigManage::sqlGetValue(dataSet[i], "platformCtrlPercent", poolData.platformCtrlPercent);
		CConfigManage::sqlGetValue(dataSet[i], "realPeopleFailPercent", poolData.realPeopleFailPercent);
		CConfigManage::sqlGetValue(dataSet[i], "realPeopleWinPercent", poolData.realPeopleWinPercent);
		CConfigManage::sqlGetValue(dataSet[i], "detailInfo", poolData.detailInfo, sizeof(poolData.detailInfo));

		// redis中已经存在了就不加载
		if (IsKeyExists(TBL_REWARDS_POOL, poolData.roomID))
		{
			continue;
		}

		std::unordered_map<std::string, std::string> umap;

		umap["roomID"] = std::to_string(poolData.roomID);
		umap["poolMoney"] = std::to_string(poolData.poolMoney);
		umap["gameWinMoney"] = std::to_string(poolData.gameWinMoney);
		umap["percentageWinMoney"] = std::to_string(poolData.percentageWinMoney);
		umap["otherWinMoney"] = std::to_string(poolData.otherWinMoney);
		umap["allGameWinMoney"] = std::to_string(poolData.allGameWinMoney);
		umap["allPercentageWinMoney"] = std::to_string(poolData.allPercentageWinMoney);
		umap["allOtherWinMoney"] = std::to_string(poolData.allOtherWinMoney);
		umap["platformCtrlType"] = std::to_string(poolData.platformCtrlType);
		umap["platformCtrlPercent"] = std::to_string(poolData.platformCtrlPercent);
		umap["realPeopleFailPercent"] = std::to_string(poolData.realPeopleFailPercent);
		umap["realPeopleWinPercent"] = std::to_string(poolData.realPeopleWinPercent);
		umap["detailInfo"] = poolData.detailInfo;

		if (!hmset(TBL_REWARDS_POOL, poolData.roomID, umap))
		{
			ERROR_LOG("加载奖池失败:roomID=%d", poolData.roomID);
		}
	}

	return true;
}

bool CRedisCenter::LoadAllAgentUser()		//加载所有代理
{
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig(DB_TYPE_PHP);

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return false;
	}

	char sql[128] = "";
	sprintf(sql, "select * from %s", TBL_WEB_AGENTMEMBER);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		int userID = 0;

		CConfigManage::sqlGetValue(dataSet[i], "userid", userID);

		if (userID > 0)
		{
			// 写入redis中
			AddKeyToSet(TBL_WEB_AGENTMEMBER, std::to_string(userID).c_str());
		}
	}

	return true;
}

bool CRedisCenter::LoadAllConfig()		//加载所有配置
{
	// 把logonBaseInfo加载到redis
	for (auto itr = ConfigManage()->m_logonBaseInfoMap.begin(); itr != ConfigManage()->m_logonBaseInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["logonID"] = std::to_string(itr->second.logonID);
		umap["ip"] = itr->second.ip;
		umap["intranetIP"] = itr->second.intranetIP;
		umap["port"] = std::to_string(itr->second.port);
		umap["maxPeople"] = std::to_string(itr->second.maxPeople);
		umap["curPeople"] = "0";
		umap["socketCount"] = std::to_string(itr->second.socketCount);
		umap["webSocketPort"] = std::to_string(itr->second.webSocketPort);
		umap["maxWebSocketPeople"] = std::to_string(itr->second.maxWebSocketPeople);
		umap["curWebSocketPeople"] = std::to_string(itr->second.curWebSocketPeople);
		umap["webSocketCount"] = std::to_string(itr->second.webSocketCount);

		hmset(TBL_BASE_LOGON, itr->first, umap);
	}

	// SystemConfig表加载到redis
	const OtherConfig& otherConfig = ConfigManage()->GetOtherConfig();
	const BankConfig& bankConfig = ConfigManage()->GetBankConfig();
	const SendGiftConfig& sendGiftConfig = ConfigManage()->GetSendGiftConfig();
	const FriendsGroupConfig& fgConfig = ConfigManage()->GetFriendsGroupConfig();
	const FtpConfig ftpConfig = ConfigManage()->GetFtpConfig();

	std::unordered_map<std::string, std::string> umapOtherConfig;
	umapOtherConfig["supportTimesEveryDay"] = std::to_string(otherConfig.supportTimesEveryDay);
	umapOtherConfig["supportMinLimitMoney"] = std::to_string(otherConfig.supportMinLimitMoney);
	umapOtherConfig["supportMoneyCount"] = std::to_string(otherConfig.supportMoneyCount);
	umapOtherConfig["registerGiveMoney"] = std::to_string(otherConfig.registerGiveMoney);
	umapOtherConfig["registerGiveJewels"] = std::to_string(otherConfig.registerGiveJewels);
	umapOtherConfig["logonGiveMoneyEveryDay"] = std::to_string(otherConfig.logonGiveMoneyEveryDay);
	umapOtherConfig["sendHornCostJewels"] = std::to_string(otherConfig.sendHornCostJewels);
	umapOtherConfig["useMagicExpressCostMoney"] = std::to_string(otherConfig.useMagicExpressCostMoney);
	umapOtherConfig["buyingDeskCount"] = std::to_string(otherConfig.buyingDeskCount);
	umapOtherConfig["friendRewardMoney"] = std::to_string(otherConfig.friendRewardMoney);
	umapOtherConfig["friendRewardMoneyCount"] = std::to_string(otherConfig.friendRewardMoneyCount);
	umapOtherConfig["friendTakeRewardMoneyCount"] = std::to_string(otherConfig.friendTakeRewardMoneyCount);
	umapOtherConfig["ftpIP"] = ftpConfig.ftpIP;
	umapOtherConfig["ftpUser"] = ftpConfig.ftpUser;
	umapOtherConfig["ftpPasswd"] = ftpConfig.ftpPasswd;
	umapOtherConfig["byIsIPRegisterLimit"] = std::to_string(otherConfig.byIsIPRegisterLimit);
	umapOtherConfig["IPRegisterLimitCount"] = std::to_string(otherConfig.IPRegisterLimitCount);
	umapOtherConfig["byIsDistributedTable"] = std::to_string(otherConfig.byIsDistributedTable);
	umapOtherConfig["http"] = otherConfig.http;
	umapOtherConfig["byIsOneToOne"] = std::to_string(otherConfig.byIsOneToOne);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_OTHER, umapOtherConfig);

	std::unordered_map<std::string, std::string> umapBankConfig;
	umapBankConfig["bankMinSaveMoney"] = std::to_string(bankConfig.minSaveMoney);
	umapBankConfig["bankSaveMoneyMuti"] = std::to_string(bankConfig.saveMoneyMuti);
	umapBankConfig["bankMinTakeMoney"] = std::to_string(bankConfig.minTakeMoney);
	umapBankConfig["bankTakeMoneyMuti"] = std::to_string(bankConfig.takeMoneyMuti);
	umapBankConfig["bankMinTransfer"] = std::to_string(bankConfig.minTransferMoney);
	umapBankConfig["bankTransferMuti"] = std::to_string(bankConfig.transferMoneyMuti);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_BANK, umapBankConfig);

	std::unordered_map<std::string, std::string> umapSendGiftConfig;
	umapSendGiftConfig["sendGiftMyLimitMoney"] = std::to_string(sendGiftConfig.myLimitMoney);
	umapSendGiftConfig["sendGiftMyLimitJewels"] = std::to_string(sendGiftConfig.myLimitJewels);
	umapSendGiftConfig["sendGiftMinMoney"] = std::to_string(sendGiftConfig.sendMinMoney);
	umapSendGiftConfig["sendGiftMinJewels"] = std::to_string(sendGiftConfig.sendMinJewels);
	umapSendGiftConfig["sendGiftRate"] = std::to_string(sendGiftConfig.rate);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_SENDGIFT, umapSendGiftConfig);

	std::unordered_map<std::string, std::string> umapFriendsGroupConfig;
	umapFriendsGroupConfig["groupCreateCount"] = std::to_string(fgConfig.groupCreateCount);
	umapFriendsGroupConfig["groupMemberCount"] = std::to_string(fgConfig.groupMemberCount);
	umapFriendsGroupConfig["groupJoinCount"] = std::to_string(fgConfig.groupJoinCount);
	umapFriendsGroupConfig["groupManageRoomCount"] = std::to_string(fgConfig.groupManageRoomCount);
	umapFriendsGroupConfig["groupRoomCount"] = std::to_string(fgConfig.groupRoomCount);
	umapFriendsGroupConfig["groupAllAlterNameCount"] = std::to_string(fgConfig.groupAllAlterNameCount);
	umapFriendsGroupConfig["groupEveAlterNameCount"] = std::to_string(fgConfig.groupEveAlterNameCount);
	umapFriendsGroupConfig["groupTransferCount"] = std::to_string(fgConfig.groupTransferCount);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_FG, umapFriendsGroupConfig);

	// 把gameBaseInfo 加载到内存
	for (auto itr = ConfigManage()->m_gameBaseInfoMap.begin(); itr != ConfigManage()->m_gameBaseInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["gameID"] = std::to_string(itr->second.gameID);
		umap["name"] = itr->second.name;
		umap["deskPeople"] = std::to_string(itr->second.deskPeople);
		umap["dllName"] = itr->second.dllName;
		umap["watcherCount"] = std::to_string(itr->second.watcherCount);
		umap["canWatch"] = std::to_string(itr->second.canWatch);
		umap["canCombineDesk"] = std::to_string(itr->second.canCombineDesk);
		umap["multiPeopleGame"] = std::to_string(itr->second.multiPeopleGame);

		hmset(TBL_BASE_GAME, itr->first, umap);
	}

	// 把privateDeskConfig 加载到内存
	for (auto itr = ConfigManage()->m_buyGameDeskInfoMap.begin(); itr != ConfigManage()->m_buyGameDeskInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["gameID"] = std::to_string(itr->second.gameID);
		umap["count"] = std::to_string(itr->second.count);
		umap["roomType"] = std::to_string(itr->second.roomType);
		umap["costResType"] = std::to_string(itr->second.costResType);
		umap["costNums"] = std::to_string(itr->second.costNums);
		umap["AAcostNums"] = std::to_string(itr->second.AAcostNums);
		umap["otherCostNums"] = std::to_string(itr->second.otherCostNums);
		umap["peopleCount"] = std::to_string(itr->second.peopleCount);

		// 组合键值
		char key[64] = "";
		sprintf(key, "%d,%d,%d", itr->first.gameID, itr->first.count, itr->first.roomType);
		hmset(TBL_BASE_BUY_DESK, key, umap);
	}

	// 把roomBaseInfo 加载到内存
	for (auto itr = ConfigManage()->m_roomBaseInfoMap.begin(); itr != ConfigManage()->m_roomBaseInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["roomID"] = std::to_string(itr->second.roomID);
		umap["gameID"] = std::to_string(itr->second.gameID);
		umap["name"] = itr->second.name;
		umap["serviceName"] = itr->second.serviceName;
		umap["type"] = std::to_string(itr->second.type);
		umap["sort"] = std::to_string(itr->second.sort);
		umap["deskCount"] = std::to_string(itr->second.deskCount);
		umap["minPoint"] = std::to_string(itr->second.minPoint);
		umap["maxPoint"] = std::to_string(itr->second.maxPoint);
		umap["basePoint"] = std::to_string(itr->second.basePoint);
		umap["gameBeginCostMoney"] = std::to_string(itr->second.gameBeginCostMoney);
		umap["describe"] = itr->second.describe;
		umap["roomSign"] = std::to_string(itr->second.roomSign);
		umap["robotCount"] = std::to_string(itr->second.robotCount);
		umap["currPeopleCount"] = "0";
		umap["level"] = std::to_string(itr->second.level);
		umap["configInfo"] = itr->second.configInfo;

		hmset(TBL_BASE_ROOM, itr->first, umap);
	}

	return true;
}

bool CRedisCenter::SetUserPhone(int userID, const char* phone, bool bUnBind /*= false*/)
{
	if (!phone || userID <= 0)
	{
		return false;
	}

	if (strlen(phone) <= 0)
	{
		return true;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	if (bUnBind)
	{
		sprintf(redisCmd, "HDEL %s %s", TBL_PHONE_TOUSERID, phone);
	}
	else
	{
		sprintf(redisCmd, "HSET %s %s %d", TBL_PHONE_TOUSERID, phone, userID);
	}

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

// 闲聊绑定
bool CRedisCenter::SetUserXianLiao(int userID, const char* xianliao)
{
	if (!xianliao || userID <= 0)
	{
		return false;
	}

	if (strlen(xianliao) <= 0)
	{
		return true;
	}

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

	sprintf(redisCmd, "HSET %s %s %d", TBL_XIANLIAO_TOUSERID, xianliao, userID);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

// 设置游戏服状态
bool CRedisCenter::SetRoomServerStatus(int roomID, int status)
{
	if (roomID <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_BASE_ROOM, roomID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s status %d", key.c_str(), status);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

// 设置登陆服状态
bool CRedisCenter::SetLogonServerStatus(int logonID, int status)
{
	if (logonID <= 0)
	{
		return false;
	}

	std::string key = MakeKey(TBL_BASE_LOGON, logonID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s status %d", key.c_str(), status);

	redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
	REDIS_CHECKF(pReply, redisCmd);

	freeReplyObject(pReply);

	return true;
}

bool CRedisCenter::SetUserSealTime(int userID, int time)
{
	std::string key = MakeKey(TBL_USER, userID);

	char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
	sprintf(redisCmd, "HMSET %s sealFinishTime %d", key.c_str(), time);

	if (!InnerHMSetCommand(key.c_str(), redisCmd))
	{
		return false;
	}

	return true;
}

bool CRedisCenter::LoadAllUserBag()
{
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig(DB_TYPE_COMMON);

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return false;
	}

	char buf[MAX_SQL_STATEMENT_SIZE] = "";


	// 加载所有的玩家数据
	sprintf(buf, "select * from %s", TBL_USER_BAG);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(buf, dataSet);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		UserBag userbag;

		CConfigManage::sqlGetValue(dataSet[i], "userID", userbag.userID);
		CConfigManage::sqlGetValue(dataSet[i], "skillFrozen", userbag.skillFrozen);
		CConfigManage::sqlGetValue(dataSet[i], "skillLocking", userbag.skillLocking);
		CConfigManage::sqlGetValue(dataSet[i], "redBag", userbag.redBag);
		CConfigManage::sqlGetValue(dataSet[i], "phoneBillCard1", userbag.phoneBillCard1);
		CConfigManage::sqlGetValue(dataSet[i], "phoneBillCard5", userbag.phoneBillCard5);
		CConfigManage::sqlGetValue(dataSet[i], "goldenArmor", userbag.goldenArmor);
		CConfigManage::sqlGetValue(dataSet[i], "mechatroPioneer", userbag.mechatroPioneer);
		CConfigManage::sqlGetValue(dataSet[i], "deepSeaArtillery", userbag.deepSeaArtillery);
		CConfigManage::sqlGetValue(dataSet[i], "octopusCannon", userbag.octopusCannon);
		CConfigManage::sqlGetValue(dataSet[i], "goldenDragon", userbag.goldenDragon);
		CConfigManage::sqlGetValue(dataSet[i], "lavaArmor", userbag.lavaArmor);

		// redis中已经存在了就不加载
		if (IsKeyExists(TBL_REWARDS_POOL, userbag.userID))
		{
			continue;
		}

		std::unordered_map<std::string, std::string> umap;

		umap["userID"] = std::to_string(userbag.userID);
		umap["skillFrozen"] = std::to_string(userbag.skillFrozen);
		umap["skillLocking"] = std::to_string(userbag.skillLocking);
		umap["redBag"] = std::to_string(userbag.redBag);
		umap["phoneBillCard1"] = std::to_string(userbag.phoneBillCard1);
		umap["phoneBillCard5"] = std::to_string(userbag.phoneBillCard5);
		umap["goldenArmor"] = std::to_string(userbag.goldenArmor);
		umap["mechatroPioneer"] = std::to_string(userbag.mechatroPioneer);
		umap["deepSeaArtillery"] = std::to_string(userbag.deepSeaArtillery);
		umap["octopusCannon"] = std::to_string(userbag.octopusCannon);
		umap["goldenDragon"] = std::to_string(userbag.goldenDragon);
		umap["lavaArmor"] = std::to_string(userbag.lavaArmor);

		if (!hmset(TBL_REWARDS_POOL, userbag.userID, umap))
		{
			ERROR_LOG("加载背包失败:userID=%d", userbag.userID);
		}
	}

	return true;
}
