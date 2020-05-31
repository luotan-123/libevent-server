#pragma once

#include <map>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include "Function.h"

//////////////////////////////////////////////////////////
// 基础配置表(先尝试读取redis，redis中没有读取内存的)
#define TBL_BASE_GAME			"gameBaseInfo"
#define TBL_BASE_ROOM			"roomBaseInfo"
#define TBL_BASE_BUY_DESK		"privateDeskConfig"
#define TBL_BASE_OTHER_CONFIG	"otherConfig"
#define TBL_BASE_REDIS_CONFIG	"redisBaseInfo"
#define TBL_BASE_LOGON			"logonBaseInfo"
#define TBL_BASE_WORK			"workBaseInfo"

/////////////////////////////////////////////////////////
// 自动生成sql语句，将redis数据保存到数据库
const std::array<const char*, 3> dynamicTbls =
{
	"userInfo",
	"rewardsPool",
	"userBag",
};

// 数据库的字段类型定义
enum FieldValueType
{
	FIELD_VALUE_TYPE_NONE = 0,
	FIELD_VALUE_TYPE_CHAR,
	FIELD_VALUE_TYPE_INT,
	FIELD_VALUE_TYPE_LONGLONG,
	FIELD_VALUE_TYPE_STR,
	FIELD_VALUE_TYPE_DOUBLE,
	FIELD_VALUE_TYPE_END,
};

struct FieldDescriptor
{
	char fieldName[48];
	int valueType;

	FieldDescriptor()
	{
		memset(this, 0, sizeof(FieldDescriptor));
	}
};

// TODO, 可以生成一份配置文件
//////////////////////////////////////////////////////////////////////
// 基础信息表结构

// 游戏信息结构
struct GameBaseInfo
{
	int	gameID;
	char name[64];
	int	deskPeople;
	char dllName[24];
	int	watcherCount;
	int canWatch;
	BYTE canCombineDesk;	//0：匹配桌子模式，1：组桌模式
	BYTE multiPeopleGame;	//0：默认，1：标识2,3,4人是否公用同一个游戏id

	GameBaseInfo()
	{
		memset(this, 0, sizeof(GameBaseInfo));
	}
};
// 房间基础信息表 roomBaseInfo
enum RoomType
{
	ROOM_TYPE_GOLD = 0,	// 金币场
	ROOM_TYPE_PRIVATE,	// 积分房
	ROOM_TYPE_FRIEND,	// 金币房
	ROOM_TYPE_FG_VIP,	// 俱乐部vip房
	ROOM_TYPE_MATCH,	// 比赛场
};

enum RoomSort
{
	ROOM_SORT_NORMAL = 0,	// 普通
	ROOM_SORT_HUNDRED,		// 百人类
	ROOM_SORT_SCENE,		// 场景类
};

enum ServerPlatfromStatus
{
	SERVER_PLATFROM_STATUS_NORMAL = 0,	//正常状态	
	SERVER_PLATFROM_STATUS_CLOSE = 1,	//关服
	SERVER_PLATFROM_STATUS_TEST = 2,	//测试
};

// 游戏服配置表
struct RoomBaseInfo
{
	int		roomID;
	int		gameID;
	char	name[48];
	char	serviceName[48];	// 服务器名，注意长度超出！
	int		type;				// 房间类型(0: 金币场 1: 房卡场)
	int		sort;				// 房间种类(0: 普通 1: 百人类)
	int		deskCount;			// 桌子数量
	int		minPoint;			// 最小积分(只适用于金币场)
	int		maxPoint;			// 最大积分(只适用于金币场)
	int		basePoint;			// 底分
	int		gameBeginCostMoney; // 金币场开局消耗金币
	char	describe[64];		// 描述
	int     roomSign;           // 房间类型标记
	UINT	robotCount;			// 每桌机器人数量
	BYTE	status;				// 启动状态
	int		currPeopleCount;	// 当前房间人数
	BYTE	level;				// 等级，初中高特
	char	configInfo[2048];	// 房间配置信息（json格式），包括玩法，可与ini文件同时并存

	RoomBaseInfo()
	{
		memset(this, 0, sizeof(RoomBaseInfo));
	}
};

// 网关服配置表
struct LogonBaseInfo
{
	int		logonID;
	char	ip[MAX_NUM_IP_ADDR_SIZE];
	char	intranetIP[MAX_NUM_IP_ADDR_SIZE];
	int		port;
	int		maxPeople;
	int		curPeople;
	BYTE	status;
	int		socketCount;
	int		webSocketPort;
	int		maxWebSocketPeople;
	int		curWebSocketPeople;
	int		webSocketCount;

	LogonBaseInfo()
	{
		memset(this, 0, sizeof(LogonBaseInfo));
	}
};

// 数据库配置结构
struct DBConfig
{
	char ip[MAX_IP_ADDR_SIZE];
	char user[MAX_DB_USER_SIZE];
	char passwd[MAX_DB_PASSWD_SIZE];
	char dbName[MAX_DB_NAME_SIZE];
	int port;

	DBConfig()
	{
		memset(this, 0, sizeof(DBConfig));
	}
};

// redis配置结构
struct RedisConfig
{
	int redisTypeID;
	char ip[128];
	int port;
	char passwd[128];

	RedisConfig()
	{
		memset(this, 0, sizeof(RedisConfig));
	}
};

// 中心服务器配置
struct CenterServerConfig
{
	char ip[MAX_IP_ADDR_SIZE];
	int port;
	int maxSocketCount;

	CenterServerConfig()
	{
		memset(this, 0, sizeof(CenterServerConfig));
	}
};

// 网关服务器配置
struct LogonServerConfig
{
	int logonID;
	BYTE webSocket;

	LogonServerConfig()
	{
		memset(this, 0, sizeof(LogonServerConfig));
	}
};

// 逻辑服务器配置
struct WorkServerConfig
{
	int workID;
	int status;
	int gateconnected;

	WorkServerConfig()
	{
		memset(this, 0, sizeof(WorkServerConfig));
	}
};

// 游戏服务器配置
struct LoaderServerConfig
{
	char serviceName[64];
	char logonserverPasswd[128];
	int  recvThreadNumber;

	LoaderServerConfig()
	{
		memset(this, 0, sizeof(LoaderServerConfig));
	}
};

// 通用配置
struct CommonConfig
{
	std::string logPath;
	int WorkThreadNumber;
	int TimerThreadNumber;

	CommonConfig()
	{
		WorkThreadNumber = 4;
		TimerThreadNumber = 1;
	}
};

struct BuyGameDeskInfoKey
{
	int gameID;			//	游戏ID
	int count;			//	局数
	int roomType;		//  房间类型
	BuyGameDeskInfoKey(int gameID, int count, int roomType)
	{
		this->gameID = gameID;
		this->count = count;
		this->roomType = roomType;
	}

	bool operator < (const BuyGameDeskInfoKey& key_) const
	{
		if (gameID < key_.gameID)
		{
			return true;
		}

		if (gameID == key_.gameID && count < key_.count)
		{
			return true;
		}

		if (gameID == key_.gameID && count == key_.count && roomType < key_.roomType)
		{
			return true;
		}

		return false;
	}
};

struct BuyRoomInfoKey
{
	int gameID;			//	游戏ID
	int roomType;		//  房间类型
	BuyRoomInfoKey(int gameID, int roomType)
	{
		this->gameID = gameID;
		this->roomType = roomType;
	}

	bool operator < (const BuyRoomInfoKey& key_) const
	{
		if (gameID < key_.gameID)
		{
			return true;
		}

		if (gameID == key_.gameID && roomType < key_.roomType)
		{
			return true;
		}

		return false;
	}
};

struct BuyGameDeskInfo
{
	int gameID;			//	游戏ID
	int count;			//	局数
	int roomType;		//  房间类型
	int costResType;	//  RESOURCE_TYPE_MONEY=1:金币   RESOURCE_TYPE_JEWEL=2:房卡
	int costNums;		//	普通支付扣数量
	int AAcostNums;		//  AA支付数量
	int otherCostNums;	//  其它支付扣数量
	int peopleCount;	//	人数

	BuyGameDeskInfo()
	{
		memset(this, 0, sizeof(BuyGameDeskInfo));
	}
};

struct OtherConfig
{
	int supportTimesEveryDay;
	int supportMinLimitMoney;
	int supportMoneyCount;
	int registerGiveMoney;
	int registerGiveJewels;
	int logonGiveMoneyEveryDay;

	int sendHornCostJewels;			// 发送世界广播消耗的房卡

	int useMagicExpressCostMoney;	// 使用魔法表情花费

	int buyingDeskCount;

	int friendRewardMoney;			// 好友打赏获得金币
	int friendRewardMoneyCount;     // 每天能领取的次数
	int friendTakeRewardMoneyCount; // 每天能领取的次数

	BYTE byIsIPRegisterLimit;		// 注册ip限制
	int IPRegisterLimitCount;		// 每个ip注册数量

	BYTE byIsDistributedTable;		// 是否分布式记录表

	char http[128];					// http请求的域名或者ip

	BYTE byIsOneToOne;				// 是否是1:1平台

	OtherConfig()
	{
		supportTimesEveryDay = 2;
		supportMinLimitMoney = 2000;
		supportMoneyCount = 4000;
		registerGiveMoney = 100000;
		registerGiveJewels = 20;
		logonGiveMoneyEveryDay = 0;
		sendHornCostJewels = 5;
		useMagicExpressCostMoney = 300;
		buyingDeskCount = 5;
		friendRewardMoney = 100;
		friendRewardMoneyCount = 5;
		friendTakeRewardMoneyCount = 5;

		byIsIPRegisterLimit = 0;
		IPRegisterLimitCount = 20;

		byIsDistributedTable = 0;

		memset(http, 0, sizeof(http));

		byIsOneToOne = 0;
	}
};

struct BankConfig
{
	long long minSaveMoney;
	int saveMoneyMuti;
	long long minTakeMoney;
	int takeMoneyMuti;
	long long minTransferMoney;
	int transferMoneyMuti;

	BankConfig()
	{
		minSaveMoney = 10000;
		saveMoneyMuti = 1000;
		minTakeMoney = 10000;
		takeMoneyMuti = 1000;
		minTransferMoney = 10000;
		transferMoneyMuti = 1000;
	}
};

//  转赠相关配置
struct SendGiftConfig
{
	long long myLimitMoney;
	int myLimitJewels;

	long long sendMinMoney;
	int sendMinJewels;

	double rate;		//税率 

	SendGiftConfig()
	{
		myLimitMoney = 500000;
		myLimitJewels = 50;

		sendMinMoney = 10000;
		sendMinJewels = 10;

		rate = 0.02;
	}
};

//  俱乐部相关配置
struct FriendsGroupConfig
{
	int groupCreateCount; //每个人最多创建俱乐部时间
	int groupMemberCount; //每个俱乐部成员最多数量
	int groupJoinCount;   //每个人最多加入俱乐部数量（包括自己创建和加入的）
	int groupManageRoomCount;//在俱乐部中，群主最多创建房间数量
	int groupRoomCount;		//在俱乐部中，普通成员最多创建房间数量
	int groupAllAlterNameCount; //俱乐部总共最多改名数量
	int groupEveAlterNameCount; //俱乐部每日最多改名数量
	int groupTransferCount;		//俱乐部最多能授权的管理员数量

	FriendsGroupConfig()
	{
		groupCreateCount = 5;
		groupMemberCount = 150;
		groupJoinCount = 10;
		groupManageRoomCount = 5;
		groupRoomCount = 1;
		groupAllAlterNameCount = 3;
		groupEveAlterNameCount = 1;
		groupTransferCount = 3;
	}
};

// ftp配置
struct FtpConfig
{
	char ftpIP[24];
	char ftpUser[64];
	char ftpPasswd[64];

	FtpConfig()
	{
		memset(this, 0, sizeof(FtpConfig));
	}
};

struct RobotPositionInfo
{
	std::string ip;
	std::string longitude;
	std::string latitude;
	std::string address;
	std::string name;
	std::string headUrl;
	BYTE sex;

	RobotPositionInfo()
	{
		ip = "127.0.0.1";
		address = "default";
		latitude = "0.0";
		longitude = "0.0";
		name = "default";
		headUrl = "default";//"http://39.108.166.3/head/man/1001.jpg";
		sex = 0;
	}
};

struct ServerConfigInfo
{
	int port;
	std::string ip;

	ServerConfigInfo(int port, std::string ip)
	{
		this->port = port;
		this->ip = ip;
	}

	bool operator < (const ServerConfigInfo& info) const
	{
		if (port < info.port)
		{
			return true;
		}

		if (port == info.port && ip < info.ip)
		{
			return true;
		}

		return false;
	}
};

class CMysqlHelper;
// 配置管理, 管理数据中或者是通用配置文件的配置
class CConfigManage
{
private:
	CConfigManage();
	~CConfigManage();

public:
	static CConfigManage* Instance();
	void Release();

public:
	bool Init();
	// 加载DB配置
	bool LoadDBConfig();
	// 加载中心服务器配置
	bool LoadCenterServerConfig();
	// 加载本地网关服务器配置
	bool LoadLogonServerConfig();
	// 加载本地逻辑服务器配置
	bool LoadWorkServerConfig();
	// 加载游戏服务器配置
	bool LoadLoaderServerConfig();
	// 加载通用配置
	bool LoadCommonConfig();
	// 加载DB表字段描述配置
	bool LoadTableFiledConfig();
	// 加载基础配置
	bool LoadBaseConfig();
	// 加载其他配置
	bool LoadOtherConfig();
	// 设置服务器类型
	void SetServiceType(int type);
	// 解析json数据
	static std::string ParseJsonValue(const std::string& src, const char* key);
	// libEvent日志回调函数
	static void EventLog(int severity, const char* msg);

private:
	// 测试数据库连接
	bool ConnectToDatabase();
	// 加载游戏基础配置
	bool LoadGameBaseConfig();
	// 加载购买桌子配置
	bool LoadBuyGameDeskConfig();
	// 加载房间基本配置
	bool LoadRoomBaseConfig();
	// 加载机器人位置配置
	bool LoadRobotPositionConfig();
	// 加载脏字库
	bool LoadDirtyWordsConfig();
	// 加载db表的主键
	bool LoadTablesPrimaryKey();
	// 大厅服基本配置
	bool LoadLogonBaseConfig();
	// 加载逻辑服务器配置
	bool LoadWorkBaseConfig();
	// 加载redis基本配置
	bool LoadRedisConfig();
public:
	// 获取DB配置
	const DBConfig& GetDBConfig(int dbType);
	// 获取centerserver配置
	const CenterServerConfig& GetCenterServerConfig();
	// 获取logonserver配置
	const LogonServerConfig& GetLogonServerConfig();
	// 获取logonserver配置
	const WorkServerConfig& GetWorkServerConfig();
	// 获取通用配置
	const CommonConfig& GetCommonConfig();
	// 获取ftp配置
	const FtpConfig& GetFtpConfig();
	// 获取表中某个字段的类型
	int GetFieldType(const char* tableName, const char* filedName);
	// 获取dll文件名
	const char* GetDllFileName(int gameID);
	// 获取房间基本配置
	RoomBaseInfo* GetRoomBaseInfo(int roomID);
	// 通过gameID获取私有房的roomID
	bool GetPrivateRoomIDByGameID(int gameID, std::vector<int>& roomID);
	// 获取游戏基本配置
	GameBaseInfo* GetGameBaseInfo(int GameID);
	// 获取购买桌子配置
	BuyGameDeskInfo* GetBuyGameDeskInfo(const BuyGameDeskInfoKey& buyKey);
	// 获取其他配置信息
	const OtherConfig& GetOtherConfig();
	// 获取转赠配置
	const SendGiftConfig& GetSendGiftConfig();
	// 获取银行配置
	const BankConfig& GetBankConfig();
	// 获取俱乐部配置
	const FriendsGroupConfig& GetFriendsGroupConfig();
	// 获取大厅基本配置
	LogonBaseInfo* GetLogonBaseInfo(int logonID);
	// 获取大厅基本配置
	WorkServerConfig* GetWorkBaseInfo(int workID);
	// 获取redis配置
	const RedisConfig& GetRedisConfig(int redisTypeID);
	// 获取购买roomID信息
	void GetBuyRoomInfo(int gameID, int roomType, std::vector<int>& roomIDVec);
	// 获得分表的表名
	bool GetTableNameByDate(const char* name, char* dateName, size_t size);
	// 根据服务器类型获取名字
	std::string GetServerNameByType(int type);

public:
	// 通过机器人的ID获取信息
	bool GetRobotPositionInfo(int robotID, RobotPositionInfo& info);
	// 获取数据表字段以及类型信息
	const std::vector<FieldDescriptor>& GetTableFiledDescVec(const std::string& tableName);
	// 获取表的主键字段名
	const char* GetTablePriamryKey(const std::string& tableName);
public:
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, int& iValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, UINT& uValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, long& lValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, long long& llValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, double& dValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, bool& bValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, BYTE& dValue);
	static bool sqlGetValue(std::map<std::string, std::string>& data, const char* szFieldName, char szBuffer[], UINT uSize);
public:
	// DB配置
	DBConfig m_dbConfig[DBType::DB_TYPE_MAX];
	// 中心服务器配置
	CenterServerConfig m_centerServerConfig;
	// 网关服务器配置
	LogonServerConfig m_logonServerConfig;
	// 逻辑服务器配置
	WorkServerConfig m_workServerConfig;
	// 游戏服务器配置
	LoaderServerConfig m_loaderServerConfig;
	// 通用配置
	CommonConfig m_commonConfig;
	// ftp配置
	FtpConfig m_ftpConfig;
	// 其他配置
	OtherConfig m_otherConfig;
	// 转赠配置
	SendGiftConfig m_sendGiftConfig;
	// 银行配置
	BankConfig m_bankConfig;
	// 俱乐部相关配置
	FriendsGroupConfig m_friendsGroupConfig;
	// 服务器类型
	int m_serviceType;

public:
	// 游戏基础信息																	
	std::unordered_map<int, GameBaseInfo> m_gameBaseInfoMap;
	// 房间配置
	std::map<int, RoomBaseInfo> m_roomBaseInfoMap;
	// 购买游戏桌子消耗信息(房卡配置)
	std::map<BuyGameDeskInfoKey, BuyGameDeskInfo> m_buyGameDeskInfoMap;
	// 购买游戏桌子roomID信息(roomID配置)
	std::map<BuyRoomInfoKey, std::vector<int> > m_buyRoomInfoMap;
	// redis数据分区
	std::map<int, RedisConfig> m_redisConfigMap;
	// 网关服配置
	std::map<int, LogonBaseInfo> m_logonBaseInfoMap;
	// 逻辑服配置
	std::map<int, WorkServerConfig> m_workBaseInfoMap;
	// 数据表字段相关信息(基础配置表不需要)
	std::unordered_map<std::string, std::vector<FieldDescriptor> > m_tableFieldDescMap;

public:
	std::vector<RobotPositionInfo> m_robotPositionInfoVec;
	std::vector<std::string> m_dirtyWordsVec;
	std::unordered_map<std::string, std::string> m_tablePrimaryKeyMap;
	std::unordered_map<std::string, int> m_nickName;
private:
	CMysqlHelper* m_pMysqlHelper;		// 数据库模块
private:
	std::set<ServerConfigInfo>  m_logonPortSet;
public:
	void GetOtherConfigKeyValue(std::string& strKey, std::string& strValue);
};

#define ConfigManage()		CConfigManage::Instance()