#include "CommonHead.h"
#include "configManage.h"
#include "INIFile.h"
#include "Exception.h"
#include "curl.h"
#include "KernelDefine.h"
#include "log.h"
#include "MysqlHelper.h"

CConfigManage::CConfigManage()
{
	m_pMysqlHelper = new CMysqlHelper;
}

CConfigManage::~CConfigManage()
{
}

CConfigManage* CConfigManage::Instance()
{
	static CConfigManage mgr;
	return &mgr;
}

void CConfigManage::Release()
{
	m_tableFieldDescMap.clear();
	m_gameBaseInfoMap.clear();
	m_roomBaseInfoMap.clear();
	m_buyGameDeskInfoMap.clear();
	m_buyRoomInfoMap.clear();
	m_logonBaseInfoMap.clear();

	curl_global_cleanup();

	if (m_pMysqlHelper)
	{
		delete m_pMysqlHelper;
		m_pMysqlHelper = NULL;
	}

}

bool CConfigManage::Init()
{
	AUTOCOST("CConfigManage::Init()");
	INFO_LOG("configManage Init begin...");

	// 初始化curl
	curl_global_init(CURL_GLOBAL_ALL);

	bool ret = false;

	//// 请求认证
	//if (!RequestAuth())
	//{
	//	ERROR_LOG("request auth failed");
	//	AfxMessageBox("服务器认证失败", MB_ICONSTOP);
	//	exit(0);
	//}

	// 加载db配置
	ret = LoadDBConfig();
	if (!ret)
	{
		ERROR_LOG("LoadDBConfig failed");
		return false;
	}

	// 加载中心服务器配置
	ret = LoadCenterServerConfig();
	if (!ret)
	{
		ERROR_LOG("LoadCenterServerConfig failed");
		return false;
	}

	// 加载大厅服务器配置
	ret = LoadLogonServerConfig();
	if (!ret)
	{
		ERROR_LOG("LoadLogonServerConfig failed");
		return false;
	}

	// 加载游戏服配置
	ret = LoadLoaderServerConfig();
	if (!ret)
	{
		ERROR_LOG("LoadLoaderServerConfig failed");
		return false;
	}

	// 加载公共配置
	ret = LoadCommonConfig();
	if (!ret)
	{
		ERROR_LOG("LoadCommonConfig failed");
		return true;
	}

	// 测试数据库连接
	ret = ConnectToDatabase();
	if (!ret)
	{
		ERROR_LOG("ConnectToDatabase failed");
		return false;
	}

	// 加载动态表的字段描述
	ret = LoadTableFiledConfig();
	if (!ret)
	{
		ERROR_LOG("LoadTableFiledConfig failed");
		return false;
	}

	// 加载表的主键
	ret = LoadTablesPrimaryKey();
	if (!ret)
	{
		ERROR_LOG("LoadTablesPrimaryKey failed");
		return false;
	}

	// 加载基础配置
	ret = LoadBaseConfig();
	if (!ret)
	{
		ERROR_LOG("LoadBaseConfig failed");
		return false;
	}

	//断开与数据库的连接
	m_pMysqlHelper->disconnect();

	INFO_LOG("configManage Init end.");

	return true;
}

bool CConfigManage::LoadDBConfig()
{
	string path = CINIFile::GetAppPath();
	CINIFile file(path + "config.ini");

	string key = "DB";
	string ret;

	ret = file.GetKeyVal(key, "ip", "127.0.0.1");
	strncpy(m_dbConfig.ip, ret.c_str(), sizeof(m_dbConfig.ip) - 1);

	ret = file.GetKeyVal(key, "user", "sa");
	strncpy(m_dbConfig.user, ret.c_str(), sizeof(m_dbConfig.user) - 1);

	ret = file.GetKeyVal(key, "passwd", "123456");
	strncpy(m_dbConfig.passwd, ret.c_str(), sizeof(m_dbConfig.passwd) - 1);

	ret = file.GetKeyVal(key, "dbName", "HM");
	strncpy(m_dbConfig.dbName, ret.c_str(), sizeof(m_dbConfig.dbName) - 1);

	m_dbConfig.port = file.GetKeyVal(key, "port", 1433);

	return true;
}

bool CConfigManage::LoadRedisConfig()
{
	char sql[128] = "";
	sprintf(sql, "select * from %s", TBL_BASE_REDIS_CONFIG);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_pMysqlHelper->queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		RedisConfig config;

		sqlGetValue(dataSet[i], "redisTypeID", config.redisTypeID);
		sqlGetValue(dataSet[i], "ip", config.ip, sizeof(config.ip));
		sqlGetValue(dataSet[i], "port", config.port);
		sqlGetValue(dataSet[i], "passwd", config.passwd, sizeof(config.passwd));

		m_redisConfigMap.insert(std::make_pair(config.redisTypeID, config));
	}

	return true;
}

bool CConfigManage::LoadLoaderServerConfig()
{
	string path = CINIFile::GetAppPath();
	CINIFile file(path + "config.ini");

	string key = "LOADERSERVER";
	string ret;

	ret = file.GetKeyVal(key, "serviceName", "hm");
	strncpy(m_loaderServerConfig.serviceName, ret.c_str(), sizeof(m_loaderServerConfig.serviceName) - 1);

	ret = file.GetKeyVal(key, "logonserverPasswd", "e10adc3949ba59abbe56e057f20f883e");
	strncpy(m_loaderServerConfig.logonserverPasswd, ret.c_str(), sizeof(m_loaderServerConfig.logonserverPasswd) - 1);

	ret = file.GetKeyVal(key, "recvThreadNumber", "4");
	m_loaderServerConfig.recvThreadNumber = atoi(ret.c_str());

	return true;
}

bool CConfigManage::LoadCommonConfig()
{
	string path = CINIFile::GetAppPath();
	CINIFile file(path + "config.ini");

	const char* pKey = "COMMON";

	m_commonConfig.logLevel = file.GetKeyVal(pKey, "logLevel", LOG_LEVEL_INFO);
	m_commonConfig.IOCPWorkThreadNumber = file.GetKeyVal(pKey, "IOCPWorkThreadNumber", 4);

	return true;
}

bool CConfigManage::LoadTableFiledConfig()
{
	m_tableFieldDescMap.clear();

	for (size_t i = 0; i < dynamicTbls.size(); i++)
	{
		const char* tblName = dynamicTbls[i];
		if (!tblName)
		{
			continue;
		}

		char buf[MAX_SQL_STATEMENT_SIZE] = "";
		sprintf(buf, "show full fields from %s", tblName);

		CMysqlHelper::MysqlData dataSet;
		try
		{
			m_pMysqlHelper->queryRecord(buf, dataSet);
		}
		catch (MysqlHelper_Exception& excep)
		{
			ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
			return false;
		}

		std::vector<FieldDescriptor> vec;

		for (size_t j = 0; j < dataSet.size(); j++)
		{
			string szType = "";
			int type = FIELD_VALUE_TYPE_NONE;

			szType = dataSet[j]["Type"];

			if (szType.find("int") != string::npos)
			{
				if (szType.find("bigint") != string::npos)
				{
					type = FIELD_VALUE_TYPE_LONGLONG;
				}
				else if (szType.find("tinyint") != string::npos)
				{
					type = FIELD_VALUE_TYPE_CHAR;
				}
				else
				{
					type = FIELD_VALUE_TYPE_INT;
				}
			}
			else if (szType.find("varchar") != string::npos || szType.find("text") != string::npos)
			{
				type = FIELD_VALUE_TYPE_STR;
			}
			else if (szType.find("double") != string::npos)
			{
				type = FIELD_VALUE_TYPE_DOUBLE;
			}

			if (type == FIELD_VALUE_TYPE_NONE)
			{
				// 存在不支持的数据类型
				ERROR_LOG("have unsupported type szType=%s", szType.c_str());
				return false;
			}

			FieldDescriptor fieldDesc;

			sqlGetValue(dataSet[j], "Field", fieldDesc.fieldName, sizeof(fieldDesc.fieldName));
			fieldDesc.valueType = type;

			vec.push_back(fieldDesc);
		}

		if (vec.size() == 0)
		{
			ERROR_LOG("FieldDescriptor vec is 0 table=%s", tblName);
			return false;
		}

		m_tableFieldDescMap.insert(std::make_pair(std::string(tblName), vec));
	}

	return true;
}

bool CConfigManage::LoadBaseConfig()
{
	AUTOCOST("LoadBaseConfig");

	// 加载redis配置
	bool ret = LoadRedisConfig();
	if (!ret)
	{
		ERROR_LOG("LoadRedisConfig failed");
		return false;
	}

	ret = LoadGameBaseConfig();
	if (!ret)
	{
		ERROR_LOG("LoadGameBaseConfig failed");
		return false;
	}

	ret = LoadLogonBaseConfig();
	if (!ret)
	{
		ERROR_LOG("LoadLogonBaseConfig failed");
		return false;
	}

	ret = LoadRoomBaseConfig();
	if (!ret)
	{
		ERROR_LOG("LoadRoomBaseConfig failed");
		return false;
	}

	ret = LoadBuyGameDeskConfig();
	if (!ret)
	{
		ERROR_LOG("LoadBuyGameDeskConfig failed");
		return false;
	}

	ret = LoadOtherConfig();
	if (!ret)
	{
		ERROR_LOG("LoadOtherConfig failed");
		return false;
	}

	if (m_serviceType == SERVICE_TYPE_LOADER)
	{
		ret = LoadRobotPositionConfig();
		if (!ret)
		{
			ERROR_LOG("LoadRobotPositionConfig failed");
			return false;
		}
	}

	ret = LoadDirtyWordsConfig();
	if (!ret)
	{
		ERROR_LOG("LoadDirtyWordsConfig failed");
		return false;
	}

	return true;
}

bool CConfigManage::LoadOtherConfig()
{
	char sql[128] = "";
	sprintf(sql, "select * from %s", TBL_BASE_OTHER_CONFIG);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_pMysqlHelper->queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		string strKey = "", strValue = "";

		strKey = dataSet[i]["keyConfig"];
		strValue = dataSet[i]["valueConfig"];

		GetOtherConfigKeyValue(strKey, strValue);
	}

	return true;
}

void CConfigManage::SetServiceType(int type)
{
	m_serviceType = type;
}

bool CConfigManage::ConnectToDatabase()
{
	m_pMysqlHelper->init(m_dbConfig.ip, m_dbConfig.user, m_dbConfig.passwd, m_dbConfig.dbName, "", m_dbConfig.port);
	try
	{
		m_pMysqlHelper->connect();
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("连接数据库失败:%s", excep.errorInfo.c_str());
		return false;
	}

	return true;
}

bool CConfigManage::LoadGameBaseConfig()
{
	char sql[128] = "";
	sprintf(sql, "select * from %s", TBL_BASE_GAME);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_pMysqlHelper->queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		GameBaseInfo gameInfo;

		sqlGetValue(dataSet[i], "gameID", gameInfo.gameID);
		sqlGetValue(dataSet[i], "name", gameInfo.name, sizeof(gameInfo.name));
		sqlGetValue(dataSet[i], "deskPeople", gameInfo.deskPeople);
		sqlGetValue(dataSet[i], "dllName", gameInfo.dllName, sizeof(gameInfo.dllName));
		sqlGetValue(dataSet[i], "watcherCount", gameInfo.watcherCount);
		sqlGetValue(dataSet[i], "canWatch", gameInfo.canWatch);
		sqlGetValue(dataSet[i], "canCombineDesk", gameInfo.canCombineDesk);
		sqlGetValue(dataSet[i], "multiPeopleGame", gameInfo.multiPeopleGame);

		m_gameBaseInfoMap.insert(std::make_pair(gameInfo.gameID, gameInfo));

	}

	return true;
}

bool CConfigManage::LoadBuyGameDeskConfig()
{
	char sql[128] = "";
	sprintf(sql, "select * from %s", TBL_BASE_BUY_DESK);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_pMysqlHelper->queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		BuyGameDeskInfo buyDeskInfo;

		sqlGetValue(dataSet[i], "gameID", buyDeskInfo.gameID);
		sqlGetValue(dataSet[i], "count", buyDeskInfo.count);
		sqlGetValue(dataSet[i], "roomType", buyDeskInfo.roomType);
		sqlGetValue(dataSet[i], "costResType", buyDeskInfo.costResType);
		sqlGetValue(dataSet[i], "costNums", buyDeskInfo.costNums);
		sqlGetValue(dataSet[i], "AAcostNums", buyDeskInfo.AAcostNums);
		sqlGetValue(dataSet[i], "otherCostNums", buyDeskInfo.otherCostNums);
		sqlGetValue(dataSet[i], "peopleCount", buyDeskInfo.peopleCount);

		m_buyGameDeskInfoMap[BuyGameDeskInfoKey(buyDeskInfo.gameID, buyDeskInfo.count, buyDeskInfo.roomType)] = buyDeskInfo;
	}

	return true;
}

bool CConfigManage::LoadRoomBaseConfig()
{
	char sql[128] = "";
	if (m_serviceType == SERVICE_TYPE_LOGON || m_serviceType == SERVICE_TYPE_CENTER)
	{
		sprintf(sql, "select * from %s", TBL_BASE_ROOM);
	}
	else
	{
		sprintf(sql, "select * from %s where serviceName = '%s'", TBL_BASE_ROOM, m_loaderServerConfig.serviceName);
	}

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_pMysqlHelper->queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		RoomBaseInfo roomBaseInfo;

		sqlGetValue(dataSet[i], "roomID", roomBaseInfo.roomID);
		sqlGetValue(dataSet[i], "gameID", roomBaseInfo.gameID);
		sqlGetValue(dataSet[i], "name", roomBaseInfo.name, sizeof(roomBaseInfo.name));
		sqlGetValue(dataSet[i], "serviceName", roomBaseInfo.serviceName, sizeof(roomBaseInfo.serviceName));
		sqlGetValue(dataSet[i], "type", roomBaseInfo.type);
		sqlGetValue(dataSet[i], "sort", roomBaseInfo.sort);
		sqlGetValue(dataSet[i], "deskCount", roomBaseInfo.deskCount);
		sqlGetValue(dataSet[i], "minPoint", roomBaseInfo.minPoint);
		sqlGetValue(dataSet[i], "maxPoint", roomBaseInfo.maxPoint);
		sqlGetValue(dataSet[i], "basePoint", roomBaseInfo.basePoint);
		sqlGetValue(dataSet[i], "gameBeginCostMoney", roomBaseInfo.gameBeginCostMoney);
		sqlGetValue(dataSet[i], "describe", roomBaseInfo.describe, sizeof(roomBaseInfo.describe));
		sqlGetValue(dataSet[i], "roomSign", roomBaseInfo.roomSign);
		sqlGetValue(dataSet[i], "robotCount", roomBaseInfo.robotCount);
		sqlGetValue(dataSet[i], "level", roomBaseInfo.level);
		sqlGetValue(dataSet[i], "configInfo", roomBaseInfo.configInfo, sizeof(roomBaseInfo.configInfo));

		m_roomBaseInfoMap.insert(std::make_pair(roomBaseInfo.roomID, roomBaseInfo));

		m_buyRoomInfoMap[BuyRoomInfoKey(roomBaseInfo.gameID, roomBaseInfo.type)].push_back(roomBaseInfo.roomID);
	}

	return true;
}

bool CConfigManage::LoadRobotPositionConfig()
{
	AUTOCOST("LoadRobotPositionConfig");

	FILE* fp = fopen("robotPosition.txt", "r+");
	if (!fp)
	{
		return true;
	}

	int iManHeadIndex = MIN_MAN_HEADURL_ID, iWoManHeadIndex = MIN_WOMAN_HEADURL_ID;
	char bufHead[256] = "";

	while (!feof(fp))
	{
		char szBuf[2048] = "";
		fgets(szBuf, sizeof(szBuf), fp);

		std::string src(szBuf);
		RobotPositionInfo info;

		info.ip = ParseJsonValue(src, "ip");
		info.address = ParseJsonValue(src, "address");
		info.latitude = ParseJsonValue(src, "latitude");
		info.longitude = ParseJsonValue(src, "longitude");
		std::string strSex = ParseJsonValue(src, "sex");

		if (strSex == "man")
		{
			info.sex = USER_SEX_MALE;
			sprintf(bufHead, "http://%s/%d.jpg", WEB_ADDRESS, iManHeadIndex);
			iManHeadIndex++;
			if (iManHeadIndex > MAX_MAN_HEADURL_ID)
			{
				iManHeadIndex = MIN_MAN_HEADURL_ID;
			}
		}
		else if (strSex == "woman")
		{
			info.sex = USER_DEX_FEMALE;
			sprintf(bufHead, "http://%s/%d.jpg", WEB_ADDRESS, iWoManHeadIndex);
			iWoManHeadIndex++;
			if (iWoManHeadIndex > MAX_WOMAN_HEADURL_ID)
			{
				iWoManHeadIndex = MIN_WOMAN_HEADURL_ID;
			}
		}
		else
		{
			continue;
		}
		info.headUrl = bufHead;

		std::string name = ParseJsonValue(src, "realName");
		std::string nickName = ParseJsonValue(src, "nickName");
		if (nickName == "")
		{
			nickName = "sdfsdfe";
		}
		int iCount = m_nickName[nickName];
		if (iCount >= 2)
		{
			info.name = name;
		}
		else
		{
			info.name = nickName;
			m_nickName[nickName] ++;
		}

		if (info.ip.size() >= 24 || info.address.size() >= 64 || info.latitude.size() >= 12 || info.longitude.size() >= 12 || info.name.size() >= MAX_USER_NAME_LEN || info.headUrl.size() >= 256)
		{
			ERROR_LOG("invalid robot element ipSize=%d, addressSize=%d latitudeSize=%d longitudeSize=%d", info.ip.size(), info.address.size(), info.latitude.size(), info.longitude.size());
			continue;
		}

		m_robotPositionInfoVec.push_back(info);
	}

	return true;
}

bool CConfigManage::LoadDirtyWordsConfig()
{
	AUTOCOST("LoadDirtyWordsConfig");

	FILE* fp = fopen("dirtywords.txt", "r+");
	if (!fp)
	{
		return true;
	}

	const int dirtyBufSize = 10 * 1024 * 1024;
	char* pBuf = new char[dirtyBufSize];
	if (!pBuf)
	{
		return true;
	}
	memset(pBuf, 0, dirtyBufSize);

	fgets(pBuf, dirtyBufSize, fp);

	int lastSpacePos = -1;
	int begin = -1;
	int end = -1;

	for (size_t i = 0; i < strlen(pBuf); i++)
	{
		if (pBuf[i] == ' ')
		{
			if (lastSpacePos == -1)
			{
				begin = 0;
			}
			else
			{
				begin = lastSpacePos + 1;
			}

			end = i;

			if (end >= begin)
			{
				std::string word = std::string(pBuf + begin, end - begin);
				m_dirtyWordsVec.push_back(word);
			}
			lastSpacePos = i;
		}
	}

	auto iter = m_dirtyWordsVec.begin();
	for (; iter != m_dirtyWordsVec.end();)
	{
		if (*iter == "")
		{
			iter = m_dirtyWordsVec.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	delete[] pBuf;

	return true;
}

bool CConfigManage::LoadTablesPrimaryKey()
{
	for (size_t i = 0; i < dynamicTbls.size(); i++)
	{
		const char* tblName = dynamicTbls[i];
		if (!tblName)
		{
			continue;
		}

		char buf[MAX_SQL_STATEMENT_SIZE] = "";
		sprintf(buf, "SELECT column_name FROM INFORMATION_SCHEMA.`KEY_COLUMN_USAGE` WHERE table_name='%s' AND CONSTRAINT_SCHEMA='%s' AND constraint_name='PRIMARY'", tblName, m_dbConfig.dbName);

		CMysqlHelper::MysqlData dataSet;
		try
		{
			m_pMysqlHelper->queryRecord(buf, dataSet);
		}
		catch (MysqlHelper_Exception& excep)
		{
			ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
			return false;
		}

		if (dataSet.size() > 0)
		{
			char keyName[64] = "";
			sqlGetValue(dataSet[0], "column_name", keyName, sizeof(keyName));
			m_tablePrimaryKeyMap[tblName] = keyName;
		}
	}

	return true;
}

const DBConfig& CConfigManage::GetDBConfig()
{
	return m_dbConfig;
}

const RedisConfig& CConfigManage::GetRedisConfig(int redisTypeID)
{
	return m_redisConfigMap[redisTypeID];
}

const CommonConfig& CConfigManage::GetCommonConfig()
{
	return m_commonConfig;
}

const FtpConfig& CConfigManage::GetFtpConfig()
{
	return m_ftpConfig;
}

int CConfigManage::GetFieldType(const char* tableName, const char* filedName)
{
	if (!tableName || !filedName)
	{
		return FIELD_VALUE_TYPE_NONE;
	}

	// 特殊字段，所有hash表共有
	if (!strcmp(filedName, "extendMode"))
	{
		return FIELD_VALUE_TYPE_INT;
	}

	//std::string str(tableName);
	auto iter = m_tableFieldDescMap.find(tableName);
	if (iter == m_tableFieldDescMap.end())
	{
		return -1;
	}

	const auto& vec = iter->second;
	for (int i = 0; i < (int)vec.size(); i++)
	{
		const std::string& field = vec[i].fieldName;
		if (!strcmp(filedName, field.c_str()))
		{
			return vec[i].valueType;
		}
	}

	return FIELD_VALUE_TYPE_NONE;
}

const char* CConfigManage::GetDllFileName(int gameID)
{
	if (gameID <= 0)
	{
		return NULL;
	}

	auto iter = m_gameBaseInfoMap.find(gameID);
	if (iter != m_gameBaseInfoMap.end())
	{
		return iter->second.dllName;
	}

	return NULL;
}

RoomBaseInfo* CConfigManage::GetRoomBaseInfo(int roomID)
{
	if (roomID <= 0)
	{
		return NULL;
	}

	auto iter = m_roomBaseInfoMap.find(roomID);
	if (iter != m_roomBaseInfoMap.end())
	{
		return &iter->second;
	}

	return NULL;
}

bool CConfigManage::GetPrivateRoomIDByGameID(int gameID, std::vector<int>& roomID)
{
	auto iter = m_roomBaseInfoMap.begin();
	for (; iter != m_roomBaseInfoMap.end(); iter++)
	{
		auto info = iter->second;
		if (info.gameID == gameID && (info.type != ROOM_TYPE_GOLD))
		{
			roomID.push_back(info.roomID);
		}
	}

	return true;
}

GameBaseInfo* CConfigManage::GetGameBaseInfo(int gameID)
{
	if (gameID <= 0)
	{
		return NULL;
	}

	auto iter = m_gameBaseInfoMap.find(gameID);
	if (iter != m_gameBaseInfoMap.end())
	{
		return &iter->second;
	}

	return NULL;
}

BuyGameDeskInfo* CConfigManage::GetBuyGameDeskInfo(const BuyGameDeskInfoKey& buyKey)
{
	auto iter = m_buyGameDeskInfoMap.find(buyKey);
	if (iter != m_buyGameDeskInfoMap.end())
	{
		return &iter->second;
	}

	return NULL;
}

const OtherConfig& CConfigManage::GetOtherConfig()
{
	return m_otherConfig;
}

const SendGiftConfig& CConfigManage::GetSendGiftConfig()
{
	return m_sendGiftConfig;
}

const BankConfig& CConfigManage::GetBankConfig()
{
	return m_bankConfig;
}

const FriendsGroupConfig& CConfigManage::GetFriendsGroupConfig()
{
	return m_friendsGroupConfig;
}

bool CConfigManage::GetRobotPositionInfo(int robotID, RobotPositionInfo& info)
{
	if (m_robotPositionInfoVec.size() <= 0)
	{
		// 没有配置文件
		return true;
	}

	int idx = robotID % m_robotPositionInfoVec.size();
	info = m_robotPositionInfoVec[idx];

	return true;
}

const std::vector<FieldDescriptor>& CConfigManage::GetTableFiledDescVec(const std::string& tableName)
{
	static std::vector<FieldDescriptor> vec;

	auto iter = m_tableFieldDescMap.find(tableName);
	if (iter != m_tableFieldDescMap.end())
	{
		return iter->second;
	}

	return vec;
}

const char* CConfigManage::GetTablePriamryKey(const std::string& tableName)
{
	auto iter = m_tablePrimaryKeyMap.find(tableName);
	if (iter == m_tablePrimaryKeyMap.end())
	{
		return NULL;
	}

	return iter->second.c_str();
}

std::string CConfigManage::ParseJsonValue(const std::string& src, const char* key)
{
	if (src == "")
	{
		return "";
	}

	char strKey[128] = "";
	sprintf(strKey, "\"%s\":", key);

	int pos = src.find(strKey);
	int begin = pos + strlen(strKey);
	std::string subStr = src.substr(begin);

	int realBegin = -1;
	int realEnd = -1;
	for (size_t i = 0; i < subStr.length(); i++)
	{
		char c = subStr[i];
		if (c == '"')
		{
			if (realBegin == -1)
			{
				realBegin = i;
			}
			else if (realEnd == -1)
			{
				realEnd = i;
				break;
			}
		}
	}

	std::string value(subStr.c_str() + realBegin + 1, subStr.c_str() + realEnd);
	return value;
}

void CConfigManage::GetOtherConfigKeyValue(std::string& strKey, std::string& strValue)
{
	//// 补助相关
	if (strKey == "supportTimesEveryDay")
	{
		m_otherConfig.supportTimesEveryDay = atoi(strValue.c_str());
	}
	else if (strKey == "supportMinLimitMoney")
	{
		m_otherConfig.supportMinLimitMoney = atoi(strValue.c_str());
	}
	else if (strKey == "supportMoneyCount")
	{
		m_otherConfig.supportMoneyCount = atoi(strValue.c_str());
	}

	//// 注册赠送相关
	else if (strKey == "registerGiveMoney")
	{
		m_otherConfig.registerGiveMoney = atoi(strValue.c_str());
	}
	else if (strKey == "registerGiveJewels")
	{
		m_otherConfig.registerGiveJewels = atoi(strValue.c_str());
	}

	//// 每天登录赠送金币相关
	else if (strKey == "logonGiveMoneyEveryDay")
	{
		m_otherConfig.logonGiveMoneyEveryDay = atoi(strValue.c_str());
	}

	//// 世界广播相关
	else if (strKey == "sendHornCostJewels")
	{
		m_otherConfig.sendHornCostJewels = atoi(strValue.c_str());
	}

	//// 魔法表情相关
	else if (strKey == "useMagicExpressCostMoney")
	{
		m_otherConfig.useMagicExpressCostMoney = atoi(strValue.c_str());
	}

	//// 好友打赏
	else if (strKey == "friendRewardMoney")
	{
		m_otherConfig.friendRewardMoney = atoi(strValue.c_str());
	}
	else if (strKey == "friendRewardMoneyCount")
	{
		m_otherConfig.friendRewardMoneyCount = atoi(strValue.c_str());
	}
	else if (strKey == "friendTakeRewardMoneyCount")
	{
		m_otherConfig.friendTakeRewardMoneyCount = atoi(strValue.c_str());
	}

	//// 房间列表数量
	else if (strKey == "buyingDeskCount")
	{
		m_otherConfig.buyingDeskCount = atoi(strValue.c_str());
	}

	//// 转赠相关
	else if (strKey == "sendGiftMyLimitMoney")
	{
		m_sendGiftConfig.myLimitMoney = atoll(strValue.c_str());
	}
	else if (strKey == "sendGiftMyLimitJewels")
	{
		m_sendGiftConfig.myLimitJewels = atoi(strValue.c_str());
	}
	else if (strKey == "sendGiftMinMoney")
	{
		m_sendGiftConfig.sendMinMoney = atoll(strValue.c_str());
	}
	else if (strKey == "sendGiftMinJewels")
	{
		m_sendGiftConfig.sendMinJewels = atoi(strValue.c_str());
	}
	else if (strKey == "sendGiftRate")
	{
		m_sendGiftConfig.rate = atof(strValue.c_str());
	}

	//// 银行相关
	else if (strKey == "bankMinSaveMoney")
	{
		m_bankConfig.minSaveMoney = atoll(strValue.c_str());
	}
	else if (strKey == "bankSaveMoneyMuti")
	{
		m_bankConfig.saveMoneyMuti = atoi(strValue.c_str());
	}
	else if (strKey == "bankMinTakeMoney")
	{
		m_bankConfig.minTakeMoney = atoll(strValue.c_str());
	}
	else if (strKey == "bankTakeMoneyMuti")
	{
		m_bankConfig.takeMoneyMuti = atoi(strValue.c_str());
	}
	else if (strKey == "bankMinTransfer")
	{
		m_bankConfig.minTransferMoney = atoll(strValue.c_str());
	}
	else if (strKey == "bankTransferMuti")
	{
		m_bankConfig.transferMoneyMuti = atoi(strValue.c_str());
	}

	//// 俱乐部相关
	else if (strKey == "groupCreateCount")
	{
		m_friendsGroupConfig.groupCreateCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupMemberCount")
	{
		m_friendsGroupConfig.groupMemberCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupJoinCount")
	{
		m_friendsGroupConfig.groupJoinCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupManageRoomCount")
	{
		m_friendsGroupConfig.groupManageRoomCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupRoomCount")
	{
		m_friendsGroupConfig.groupRoomCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupAllAlterNameCount")
	{
		m_friendsGroupConfig.groupAllAlterNameCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupEveAlterNameCount")
	{
		m_friendsGroupConfig.groupEveAlterNameCount = atoi(strValue.c_str());
	}
	else if (strKey == "groupTransferCount")
	{
		m_friendsGroupConfig.groupTransferCount = atoi(strValue.c_str());
	}

	////ftp服务器相关
	else if (strKey == "ftpIP")
	{
		memcpy(m_ftpConfig.ftpIP, strValue.c_str(), min(strlen(strValue.c_str()), sizeof(m_ftpConfig.ftpIP) - 1));
	}
	else if (strKey == "ftpUser")
	{
		memcpy(m_ftpConfig.ftpUser, strValue.c_str(), min(strlen(strValue.c_str()), sizeof(m_ftpConfig.ftpUser) - 1));
	}
	else if (strKey == "ftpPasswd")
	{
		memcpy(m_ftpConfig.ftpPasswd, strValue.c_str(), min(strlen(strValue.c_str()), sizeof(m_ftpConfig.ftpPasswd) - 1));
	}

	//// ip注册限制
	else if (strKey == "byIsIPRegisterLimit")
	{
		m_otherConfig.byIsIPRegisterLimit = atoi(strValue.c_str());
	}
	else if (strKey == "IPRegisterLimitCount")
	{
		m_otherConfig.IPRegisterLimitCount = atoi(strValue.c_str());
	}

	//// 分布式配置
	else if (strKey == "byIsDistributedTable")
	{
		m_otherConfig.byIsDistributedTable = atoi(strValue.c_str());
	}

	//// http请求
	else if (strKey == "http")
	{
		memcpy(m_otherConfig.http, strValue.c_str(), min(strlen(strValue.c_str()), sizeof(m_otherConfig.http) - 1));
	}

	//// 控制1：1平台
	else if (strKey == "byIsOneToOne")
	{
		m_otherConfig.byIsOneToOne = atoi(strValue.c_str());
	}
}

bool CConfigManage::LoadLogonServerConfig()
{
	string path = CINIFile::GetAppPath();
	CINIFile file(path + "config.ini");

	string key = "LOGONSERVER";

	m_logonServerConfig.logonID = file.GetKeyVal(key, "logonID", 1);

	return true;
}

bool CConfigManage::LoadCenterServerConfig()
{
	string path = CINIFile::GetAppPath();
	CINIFile file(path + "config.ini");

	string key = "CENTERSERVER";
	string ret;

	ret = file.GetKeyVal(key, "ip", "127.0.0.1");
	strncpy(m_centerServerConfig.ip, ret.c_str(), sizeof(m_centerServerConfig.ip) - 1);

	m_centerServerConfig.port = file.GetKeyVal(key, "port", 3016);
	m_centerServerConfig.maxSocketCount = file.GetKeyVal(key, "maxSocketCount", 200);

	return true;
}

const LogonServerConfig& CConfigManage::GetLogonServerConfig()
{
	return m_logonServerConfig;
}

const CenterServerConfig& CConfigManage::GetCenterServerConfig()
{
	return m_centerServerConfig;
}

// 大厅服基本配置
bool CConfigManage::LoadLogonBaseConfig()
{
	char sql[128] = "";
	sprintf(sql, "select * from %s", TBL_BASE_LOGON);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_pMysqlHelper->queryRecord(sql, dataSet, true);
	}
	catch (MysqlHelper_Exception& excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return false;
	}

	for (size_t i = 0; i < dataSet.size(); i++)
	{
		LogonBaseInfo logonBaseInfo;

		sqlGetValue(dataSet[i], "logonID", logonBaseInfo.logonID);
		sqlGetValue(dataSet[i], "ip", logonBaseInfo.ip, sizeof(logonBaseInfo.ip));
		sqlGetValue(dataSet[i], "intranetIP", logonBaseInfo.intranetIP, sizeof(logonBaseInfo.intranetIP));
		sqlGetValue(dataSet[i], "port", logonBaseInfo.port);
		sqlGetValue(dataSet[i], "maxPeople", logonBaseInfo.maxPeople);

		ServerConfigInfo configInfo(logonBaseInfo.port, logonBaseInfo.ip);

		if (m_logonPortSet.count(configInfo) > 0)
		{
			ERROR_LOG("登陆服端口或IP重复");
			return false;
		}

		m_logonPortSet.insert(configInfo);
		m_logonBaseInfoMap.insert(std::make_pair(logonBaseInfo.logonID, logonBaseInfo));
	}

	if (m_logonBaseInfoMap.size() > MAX_LOGON_SERVER_COUNT)
	{
		ERROR_LOG("配置的登陆服数量过多。【%d/%d】", m_logonBaseInfoMap.size(), MAX_LOGON_SERVER_COUNT);
		return false;
	}

	if (m_logonBaseInfoMap.size() == 0)
	{
		ERROR_LOG("没有配置登陆服。【%d/%d】", m_logonBaseInfoMap.size(), MAX_LOGON_SERVER_COUNT);
		return false;
	}

	return true;
}

// 获取大厅基本配置
LogonBaseInfo* CConfigManage::GetLogonBaseInfo(int logonID)
{
	if (logonID <= 0)
	{
		return NULL;
	}

	auto iter = m_logonBaseInfoMap.find(logonID);
	if (iter != m_logonBaseInfoMap.end())
	{
		return &iter->second;
	}

	return NULL;
}

// 获取购买roomID信息
void CConfigManage::GetBuyRoomInfo(int gameID, int roomType, std::vector<int>& roomIDVec)
{
	roomIDVec.clear();

	auto iter = m_buyRoomInfoMap.find(BuyRoomInfoKey(gameID, roomType));
	if (iter != m_buyRoomInfoMap.end())
	{
		roomIDVec = iter->second;
	}
}

// 获得分表的表名
bool CConfigManage::GetTableNameByDate(const char* name, char* dateName, size_t size)
{
	if (name == NULL || dateName == NULL || size <= 1)
	{
		return false;
	}

	if (!m_otherConfig.byIsDistributedTable)
	{
		sprintf(dateName, "%s", name);

		return true;
	}

	SYSTEMTIME sys;
	GetLocalTime(&sys);

	sprintf(dateName, "%s_%d%02d", name, sys.wYear, sys.wMonth);

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, int& iValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	iValue = atoi(data[szFieldName].c_str());

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, UINT& uValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	uValue = atoi(data[szFieldName].c_str());

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, long& lValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	lValue = atol(data[szFieldName].c_str());

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, long long& llValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	llValue = atoll(data[szFieldName].c_str());

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, double& dValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	dValue = atof(data[szFieldName].c_str());

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, bool& bValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	bValue = atoi(data[szFieldName].c_str()) != 0;

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, BYTE& dValue)
{
	if (!szFieldName || data.size() <= 0)
	{
		return false;
	}

	dValue = (BYTE)atoi(data[szFieldName].c_str());

	return true;
}

bool CConfigManage::sqlGetValue(std::map<string, string>& data, const char* szFieldName, char szBuffer[], UINT uSize)
{
	if (!szFieldName || !szBuffer || data.size() <= 0 || uSize <= 1)
	{
		return false;
	}

	memcpy(szBuffer, data[szFieldName].c_str(), min(data[szFieldName].size(), uSize - 1));

	return true;
}