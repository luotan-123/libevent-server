#include "CommonHead.h"
#include "RedisCenter.h"	
#include "InternalMessageDefine.h"
#include "log.h"
#include "Util.h"
#include "BillManage.h"
#include <tuple>
#include "MysqlHelper.h"

CRedisCenter::CRedisCenter()
{

}

CRedisCenter::~CRedisCenter()
{
}

bool CRedisCenter::Init()
{
	AUTOCOST("CRedisCenter::Init() 耗时");

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
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig();

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo);
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
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo);
		return 2;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
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
	AUTOCOST("LoadAllUserData 耗时");

	int currMaxUserID = -1;
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig();

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo);
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
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo);
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
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

		umap["userID"] = CUtil::Tostring(userData.userID);
		umap["account"] = userData.account;
		umap["passwd"] = userData.passwd;
		umap["name"] = userData.name;
		umap["phone"] = userData.phone;
		umap["sex"] = CUtil::Tostring(userData.sex);
		umap["mail"] = userData.mail;
		umap["money"] = CUtil::Tostring(userData.money);
		umap["bankMoney"] = CUtil::Tostring(userData.bankMoney);
		umap["bankPasswd"] = userData.bankPasswd;
		umap["jewels"] = CUtil::Tostring(userData.jewels);
		umap["roomID"] = "0";//CUtil::Tostring(userData.roomID);
		umap["deskIdx"] = CUtil::Tostring(INVALID_DESKIDX);//userData.deskIdx);
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
		umap["phonePasswd"] = userData.phonePasswd;
		umap["wechat"] = userData.wechat;
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
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig();

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo);
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
			ERROR_LOG("执行sql语句失败:%s", excep.errorInfo);
			continue;
		}
	}

	// 加载所有的玩家数据
	sprintf(buf, "select * from %s", TBL_REWARDS_POOL);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(buf, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo);
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
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

		umap["roomID"] = CUtil::Tostring(poolData.roomID);
		umap["poolMoney"] = CUtil::Tostring(poolData.poolMoney);
		umap["gameWinMoney"] = CUtil::Tostring(poolData.gameWinMoney);
		umap["percentageWinMoney"] = CUtil::Tostring(poolData.percentageWinMoney);
		umap["otherWinMoney"] = CUtil::Tostring(poolData.otherWinMoney);
		umap["allGameWinMoney"] = CUtil::Tostring(poolData.allGameWinMoney);
		umap["allPercentageWinMoney"] = CUtil::Tostring(poolData.allPercentageWinMoney);
		umap["allOtherWinMoney"] = CUtil::Tostring(poolData.allOtherWinMoney);
		umap["platformCtrlType"] = CUtil::Tostring(poolData.platformCtrlType);
		umap["platformCtrlPercent"] = CUtil::Tostring(poolData.platformCtrlPercent);
		umap["realPeopleFailPercent"] = CUtil::Tostring(poolData.realPeopleFailPercent);
		umap["realPeopleWinPercent"] = CUtil::Tostring(poolData.realPeopleWinPercent);
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
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig();

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo);
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
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo);
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		int userID = 0;

		CConfigManage::sqlGetValue(dataSet[i], "userid", userID);

		if (userID > 0)
		{
			// 写入redis中
			AddKeyToSet(TBL_WEB_AGENTMEMBER, CUtil::Tostring(userID).c_str());
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

		umap["logonID"] = CUtil::Tostring(itr->second.logonID);
		umap["ip"] = itr->second.ip;
		umap["intranetIP"] = itr->second.intranetIP;
		umap["port"] = CUtil::Tostring(itr->second.port);
		umap["maxPeople"] = CUtil::Tostring(itr->second.maxPeople);
		umap["curPeople"] = "0";

		hmset(TBL_BASE_LOGON, itr->first, umap);
	}

	// SystemConfig表加载到redis
	const OtherConfig& otherConfig = ConfigManage()->GetOtherConfig();
	const BankConfig& bankConfig = ConfigManage()->GetBankConfig();
	const SendGiftConfig& sendGiftConfig = ConfigManage()->GetSendGiftConfig();
	const FriendsGroupConfig& fgConfig = ConfigManage()->GetFriendsGroupConfig();
	const FtpConfig ftpConfig = ConfigManage()->GetFtpConfig();

	std::unordered_map<std::string, std::string> umapOtherConfig;
	umapOtherConfig["supportTimesEveryDay"] = CUtil::Tostring(otherConfig.supportTimesEveryDay);
	umapOtherConfig["supportMinLimitMoney"] = CUtil::Tostring(otherConfig.supportMinLimitMoney);
	umapOtherConfig["supportMoneyCount"] = CUtil::Tostring(otherConfig.supportMoneyCount);
	umapOtherConfig["registerGiveMoney"] = CUtil::Tostring(otherConfig.registerGiveMoney);
	umapOtherConfig["registerGiveJewels"] = CUtil::Tostring(otherConfig.registerGiveJewels);
	umapOtherConfig["logonGiveMoneyEveryDay"] = CUtil::Tostring(otherConfig.logonGiveMoneyEveryDay);
	umapOtherConfig["sendHornCostJewels"] = CUtil::Tostring(otherConfig.sendHornCostJewels);
	umapOtherConfig["useMagicExpressCostMoney"] = CUtil::Tostring(otherConfig.useMagicExpressCostMoney);
	umapOtherConfig["buyingDeskCount"] = CUtil::Tostring(otherConfig.buyingDeskCount);
	umapOtherConfig["friendRewardMoney"] = CUtil::Tostring(otherConfig.friendRewardMoney);
	umapOtherConfig["friendRewardMoneyCount"] = CUtil::Tostring(otherConfig.friendRewardMoneyCount);
	umapOtherConfig["friendTakeRewardMoneyCount"] = CUtil::Tostring(otherConfig.friendTakeRewardMoneyCount);
	umapOtherConfig["ftpIP"] = ftpConfig.ftpIP;
	umapOtherConfig["ftpUser"] = ftpConfig.ftpUser;
	umapOtherConfig["ftpPasswd"] = ftpConfig.ftpPasswd;
	umapOtherConfig["byIsIPRegisterLimit"] = CUtil::Tostring(otherConfig.byIsIPRegisterLimit);
	umapOtherConfig["IPRegisterLimitCount"] = CUtil::Tostring(otherConfig.IPRegisterLimitCount);
	umapOtherConfig["byIsDistributedTable"] = CUtil::Tostring(otherConfig.byIsDistributedTable);
	umapOtherConfig["http"] = otherConfig.http;
	umapOtherConfig["byIsOneToOne"] = CUtil::Tostring(otherConfig.byIsOneToOne);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_OTHER, umapOtherConfig);

	std::unordered_map<std::string, std::string> umapBankConfig;
	umapBankConfig["bankMinSaveMoney"] = CUtil::Tostring(bankConfig.minSaveMoney);
	umapBankConfig["bankSaveMoneyMuti"] = CUtil::Tostring(bankConfig.saveMoneyMuti);
	umapBankConfig["bankMinTakeMoney"] = CUtil::Tostring(bankConfig.minTakeMoney);
	umapBankConfig["bankTakeMoneyMuti"] = CUtil::Tostring(bankConfig.takeMoneyMuti);
	umapBankConfig["bankMinTransfer"] = CUtil::Tostring(bankConfig.minTransferMoney);
	umapBankConfig["bankTransferMuti"] = CUtil::Tostring(bankConfig.transferMoneyMuti);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_BANK, umapBankConfig);

	std::unordered_map<std::string, std::string> umapSendGiftConfig;
	umapSendGiftConfig["sendGiftMyLimitMoney"] = CUtil::Tostring(sendGiftConfig.myLimitMoney);
	umapSendGiftConfig["sendGiftMyLimitJewels"] = CUtil::Tostring(sendGiftConfig.myLimitJewels);
	umapSendGiftConfig["sendGiftMinMoney"] = CUtil::Tostring(sendGiftConfig.sendMinMoney);
	umapSendGiftConfig["sendGiftMinJewels"] = CUtil::Tostring(sendGiftConfig.sendMinJewels);
	umapSendGiftConfig["sendGiftRate"] = CUtil::Tostring(sendGiftConfig.rate);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_SENDGIFT, umapSendGiftConfig);

	std::unordered_map<std::string, std::string> umapFriendsGroupConfig;
	umapFriendsGroupConfig["groupCreateCount"] = CUtil::Tostring(fgConfig.groupCreateCount);
	umapFriendsGroupConfig["groupMemberCount"] = CUtil::Tostring(fgConfig.groupMemberCount);
	umapFriendsGroupConfig["groupJoinCount"] = CUtil::Tostring(fgConfig.groupJoinCount);
	umapFriendsGroupConfig["groupManageRoomCount"] = CUtil::Tostring(fgConfig.groupManageRoomCount);
	umapFriendsGroupConfig["groupRoomCount"] = CUtil::Tostring(fgConfig.groupRoomCount);
	umapFriendsGroupConfig["groupAllAlterNameCount"] = CUtil::Tostring(fgConfig.groupAllAlterNameCount);
	umapFriendsGroupConfig["groupEveAlterNameCount"] = CUtil::Tostring(fgConfig.groupEveAlterNameCount);
	umapFriendsGroupConfig["groupTransferCount"] = CUtil::Tostring(fgConfig.groupTransferCount);
	hmset(TBL_BASE_OTHER_CONFIG, SYSTEM_CONFIG_TYPE_FG, umapFriendsGroupConfig);

	// 把gameBaseInfo 加载到内存
	for (auto itr = ConfigManage()->m_gameBaseInfoMap.begin(); itr != ConfigManage()->m_gameBaseInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["gameID"] = CUtil::Tostring(itr->second.gameID);
		umap["name"] = itr->second.name;
		umap["deskPeople"] = CUtil::Tostring(itr->second.deskPeople);
		umap["dllName"] = itr->second.dllName;
		umap["watcherCount"] = CUtil::Tostring(itr->second.watcherCount);
		umap["canWatch"] = CUtil::Tostring(itr->second.canWatch);
		umap["canCombineDesk"] = CUtil::Tostring(itr->second.canCombineDesk);
		umap["multiPeopleGame"] = CUtil::Tostring(itr->second.multiPeopleGame);

		hmset(TBL_BASE_GAME, itr->first, umap);
	}

	// 把privateDeskConfig 加载到内存
	for (auto itr = ConfigManage()->m_buyGameDeskInfoMap.begin(); itr != ConfigManage()->m_buyGameDeskInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["gameID"] = CUtil::Tostring(itr->second.gameID);
		umap["count"] = CUtil::Tostring(itr->second.count);
		umap["roomType"] = CUtil::Tostring(itr->second.roomType);
		umap["costResType"] = CUtil::Tostring(itr->second.costResType);
		umap["costNums"] = CUtil::Tostring(itr->second.costNums);
		umap["AAcostNums"] = CUtil::Tostring(itr->second.AAcostNums);
		umap["otherCostNums"] = CUtil::Tostring(itr->second.otherCostNums);
		umap["peopleCount"] = CUtil::Tostring(itr->second.peopleCount);

		// 组合键值
		char key[64] = "";
		sprintf(key, "%d,%d,%d", itr->first.gameID, itr->first.count, itr->first.roomType);
		hmset(TBL_BASE_BUY_DESK, key, umap);
	}

	// 把roomBaseInfo 加载到内存
	for (auto itr = ConfigManage()->m_roomBaseInfoMap.begin(); itr != ConfigManage()->m_roomBaseInfoMap.end(); itr++)
	{
		std::unordered_map<std::string, std::string> umap;

		umap["roomID"] = CUtil::Tostring(itr->second.roomID);
		umap["gameID"] = CUtil::Tostring(itr->second.gameID);
		umap["name"] = itr->second.name;
		umap["serviceName"] = itr->second.serviceName;
		umap["type"] = CUtil::Tostring(itr->second.type);
		umap["sort"] = CUtil::Tostring(itr->second.sort);
		umap["deskCount"] = CUtil::Tostring(itr->second.deskCount);
		umap["minPoint"] = CUtil::Tostring(itr->second.minPoint);
		umap["maxPoint"] = CUtil::Tostring(itr->second.maxPoint);
		umap["basePoint"] = CUtil::Tostring(itr->second.basePoint);
		umap["gameBeginCostMoney"] = CUtil::Tostring(itr->second.gameBeginCostMoney);
		umap["describe"] = itr->second.describe;
		umap["roomSign"] = CUtil::Tostring(itr->second.roomSign);
		umap["robotCount"] = CUtil::Tostring(itr->second.robotCount);
		umap["currPeopleCount"] = "0";
		umap["level"] = CUtil::Tostring(itr->second.level);
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

	std::string key = MakeKey(TBL_PHONE_TOUSERID, phone);

	if (bUnBind)
	{
		DelKey(key.c_str());
	}
	else
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";

		sprintf(redisCmd, "SET %s %ld", key.c_str(), userID);

		redisReply* pReply = (redisReply*)redisCommand(m_pContext, redisCmd);
		REDIS_CHECKF(pReply, redisCmd);

		freeReplyObject(pReply);
	}

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

	sprintf(redisCmd, "SET %s|%s %ld", TBL_XIANLIAO_TOUSERID, xianliao, userID);

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
	const DBConfig& dbConfig = ConfigManage()->GetDBConfig();

	//初始化mysql对象并建立连接
	CMysqlHelper mysqlHelper;
	mysqlHelper.init(dbConfig.ip, dbConfig.user, dbConfig.passwd, dbConfig.dbName, "", dbConfig.port);
	try
	{
		mysqlHelper.connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo);
		return false;
	}

	char buf[MAX_SQL_STATEMENT_SIZE] = "";


	// 加载所有的玩家数据
	sprintf(buf, "select * from %s", TBL_USER_BAG);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		mysqlHelper.queryRecord(buf, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo);
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		char redisCmd[MAX_REDIS_COMMAND_SIZE] = "";
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

		umap["userID"] = CUtil::Tostring(userbag.userID);
		umap["skillFrozen"] = CUtil::Tostring(userbag.skillFrozen);
		umap["skillLocking"] = CUtil::Tostring(userbag.skillLocking);
		umap["redBag"] = CUtil::Tostring(userbag.redBag);
		umap["phoneBillCard1"] = CUtil::Tostring(userbag.phoneBillCard1);
		umap["phoneBillCard5"] = CUtil::Tostring(userbag.phoneBillCard5);
		umap["goldenArmor"] = CUtil::Tostring(userbag.goldenArmor);
		umap["mechatroPioneer"] = CUtil::Tostring(userbag.mechatroPioneer);
		umap["deepSeaArtillery"] = CUtil::Tostring(userbag.deepSeaArtillery);
		umap["octopusCannon"] = CUtil::Tostring(userbag.octopusCannon);
		umap["goldenDragon"] = CUtil::Tostring(userbag.goldenDragon);
		umap["lavaArmor"] = CUtil::Tostring(userbag.lavaArmor);

		if (!hmset(TBL_REWARDS_POOL, userbag.userID, umap))
		{
			ERROR_LOG("加载背包失败:userID=%d", userbag.userID);
		}
	}

	return true;
}
