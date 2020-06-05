#pragma once

#include "Define.h"
#include "basemessage.h"

////////////////////////redis和数据库都有的表//////////////////////////////////
// 动态表(运行中数据会发生改变，会备份到数据库)
#define TBL_USER				"userInfo"
#define	TBL_REWARDS_POOL		"rewardsPool"
#define TBL_USER_BAG			"userBag"

//////////////////////////////////账号相关redis键值(都是hash类型,只存在缓存)////////////////////////////////////////
//普通注册账号
#define TBL_NORMAL_TO_USERID	"accountToUserID"
//第三方账号与UID的映射表
#define TBL_TRDUSERID			"trdToUserID"                          
//手机登陆映射表
#define TBL_PHONE_TOUSERID		"phoneToUserID"
//闲聊登陆映射表
#define TBL_XIANLIAO_TOUSERID	"xianliaoToUserID"
//游客登录映射表
#define TBL_VISITOR_TOUSERID	"visitorToUserID"

//////////////////////////////////////////////////////////
// 只保存在缓存中的数据表，根据功能模块划分

// 购买桌子模块
#define TBL_CACHE_DESK			"privateDeskInfo"			// 字段信息详见PrivateDeskInfo结构体 【hash】
#define TBL_FG_CLOSE_SAVE_DESK  "FGCloseSaveDesk"			// 临时保存在数据库中的牌桌 【hash】
#define TBL_CACHE_DESK_PASSWD	"privateDeskPasswd"			// redis key， string类型 保存了deskpasswd和deskMixID的映射 【string】
#define TBL_USER_BUY_DESK		"sameUserBuyDesk"			// 同步购买桌子数据 【string】
#define TBL_FG_BUY_DESK			"sameFGBuyDesk"				// 同步俱乐部购买桌子数据 【string】
#define TBL_MARK_DESK_INDEX		"BuyDeskMarkIndex"			// 购买桌子需要用到的索引 【string】
#define CACHE_USER_BUYDESK_SET	"cacheUserBuyDeskSet"		// 玩家开房列表 【set】

// 战绩模块
#define TBL_GRADE_DETAIL		"gradeDetailInfo"			// 单局战绩详情 【hash】
#define TBL_GRADE_SIMPLE		"gradeSimpleInfo"			// 大结算战绩信息 【hash】
#define TBL_MAX_GRADEKEY		"roomMaxGradeKey"			// 房间最大的战绩ID(战绩详情 单局战绩) 【string】
#define TBL_MAX_GRADESIMPLEKEY	"roomMaxGradeSimpleKey"		// 房间最大的战绩ID(战绩简介 大结算) 【string】
#define TBL_GRADE_DETAIL_MIN_ID	"clearGradeDetailMinID"		// 未清理的单局战绩详情最小id 【string】
#define TBL_GRADE_SIMPLE_MIN_ID	"clearGradeSimpleMinID"		// 未清理的大结算战绩信息最小id 【string】
#define TBL_GRADE_SIMPLE_SET	"gradeSimpleSet"			// 大结算战绩集合(包含了所有小结算的ID) 【set】
#define TBL_USER_GRADE_SET		"userGradeSet"				// 玩家的大结算战绩集合  【zSet】

// 用户数据模块
#define TBL_USER_REDSPOT		"userRedSpotCount"			// 玩家小红点哈希 【hash】
#define TBL_WEB_USER			"webUserInfo"				// php相关用户数据 【hash】【php redis】
#define TBL_ONLINE_USER_SET		"allServerOnlineUserSet"	// 玩家在线集合 【set】
#define TBL_ONLINE_REALUSER_SET "allServerOnlineRealUserSet"// 玩家在线集合(不包含机器人) 【set】
#define TBL_WEB_AGENTMEMBER		"web_agent_member"			// 存放代理的集合 【set】
#define TBL_ROBOT_INFO_INDEX	"robotInfoIndex"			// 机器人信息索引 【string】
#define TBL_CURMAX_USERID		"curSystemMaxUserID"		// 当前系统最大的userID 【string】

// 俱乐部模块
#define TBL_FG_ROOM_INFO		"FGRoomInfo"				// 俱乐部房间（牌桌）						【string】
#define TBL_FRIENDSGROUP		"friendsGroup"				// 俱乐部信息								【hash】【php redis】
#define TBL_FRIENDSGROUPTOUSER	"friendsGroupToUser"		// 俱乐部成员（获取设置火币）				【hash】【php redis】
#define TBL_USERTOFRIENDSGROUP	"userToFriendsGroup"		// 玩家对俱乐部哈希表（获取玩家权限）		【hash】【php redis】
#define TBL_FG_NOTIFY			"friendsGroupNotify"		// 俱乐部通知								【hash】【php redis】
#define TBL_FG_TO_USER_SET		"friendsGroupToUserSet"		// 俱乐部成员（获取俱乐部玩家）				【zSet】【php redis】
#define	TBL_USER_FG_NOTIFY_SET	"userToFriendsGroupNotifySet"// 俱乐部通知集合							【zSet】【php redis】
#define	TBL_FG_NOTIFY_ID		"friendsGroupNotifyID"		// 俱乐部自增id								【string】【php redis】
#define	TBL_FG_NOTIFY_CLEAR_ID	"friendsGroupNotifyClearID" // 俱乐部清理通知id							【string】【php redis】

// 邮件模块 
#define TBL_EMAIL_DETAIL		"emailDetailInfo"			// 每封邮件详情								【hash】【php redis】
#define TBL_USER_EMAIL_DETAIL	"userToEmailDetailInfo"		// 玩家的邮件详情(是否已读，是否已经领取)	【hash】【php redis】
#define TBL_EMAIL_INFO_MIN_ID	"clearEmailInfoMinID"		// 未清理的邮件最小id						【string】【php redis】
#define TBL_EMAIL_INFO_MAX_ID	"emailInfoMaxIDSet"			// 邮件最大id								【string】【php redis】
#define TBL_USER_EMAIL_SET		"userEmailSet"				// 玩家的邮件集合							【zSet】【php redis】

// 排行榜模块
#define TBL_RANKING_MONEY		"rankingUserMoney"			// 金币排行榜   【zSet】  [ zadd  rankingUserMoney  money  userID ]
#define TBL_RANKING_WINCOUNT	"rankingUserWinCount"		// 胜局排行榜   【zSet】  [ zadd  rankingUserWinCount  12  userID ]
#define TBL_RANKING_JEWELS		"rankingUserJewels"			// 钻石排行榜   【zSet】  [ zadd  rankingUserJewels  12  userID ]

// 平台管理模块
#define TBL_SERVER_STATUS		"ServerPlatfromStatus"		// 服务器状态	【string】
#define CACHE_UPDATE_SET		"cacheUpdateSet"			// 需要更新的数据集合 【set】

// 比赛场
#define TBL_TIME_MATCH_INFO		"timeMatchInfo"				// 定时赛比赛信息 【hash】【php redis】
#define TBL_PART_OF_MATCH_INDEX	"partOfMatchIndex"			// 局部比赛自增id 【string】
#define TBL_MATCH_ROBOT_USERID_INDEX	"matchRobotUserIDIndex"		// 比赛场机器人ID索引【string】
#define TBL_SIGN_UP_MATCH_PEOPLE		"curSignUpMatchPeople"		// （实时赛）当前报名人数	curSignUpMatchPeople|gameID,id	【zSet】  按照报名时间排序
#define TBL_SIGN_UP_TIME_MATCH_PEOPLE	"curSignUpTimeMatchPeople"	// （定时赛）当前报名人数	curSignUpTimeMatchPeople|id		【zSet】  按照报名时间排序

// 其它
#define TBL_SAVE_DATA_SECOND 	"save_data_second"			// 里面保存的是sql语句 list

////////////////////////////常量定义//////////////////////////////////////////
#define REDIS_STR_DEFAULT		"aa"	// redis中字符串默认值
#define	USER_IDENTITY_TYPE_SUPER	1	// 超端
#define	USER_IDENTITY_TYPE_WIN		2	// 内定赢钱玩家
#define	USER_IDENTITY_TYPE_FAIL		4	// 内定输钱玩家
#define USER_IDENTITY_TYPE_SEAL		8	// 封号标识

/////////////////////////////////////////////////////////////////////////
// 动态表

// 玩家表
struct UserData
{
	int		userID;
	char	account[64];
	char	passwd[64];
	char	name[64];
	char	phone[24];
	BYTE	sex;
	char	mail[64];
	long long money;
	long long bankMoney;
	char	bankPasswd[20];
	int		jewels;					// 房卡数(钻石)
	int		roomID;					// 玩家所在的roomID
	int		deskIdx;				// 玩家所在的桌子索引
	char	logonIP[24];			// 玩家的登录IP
	int		winCount;
	char	headURL[256];			// 玩家头像
	char	macAddr[64];			// mac地址字符串形式 
	char    token[64];				// GUID
	BYTE	isVirtual;				// 是否机器人
	int		status;					// 用户身份
	int		reqSupportTimes;		// 请求破产补助的次数
	int		lastReqSupportDate;		// 上次请求破产补助的日期
	int		registerTime;			// 注册时间
	char	registerIP[24];			// 注册IP
	BYTE	registerType;			// 注册类型
	int		buyingDeskCount;		// 已经买了但是还未解散的桌子数
	int		lastCrossDayTime;		// 上次登陆时间时间
	int		totalGameCount;			// 总局数
	int		sealFinishTime;			// 封号结束时间，-1 永久封号，0无封号，大于0截止时间
	char	wechat[24];				// 微信号
	char	phonePasswd[64];		// 手机登录密码
	char	accountInfo[64];		// 唯一标识
	int		totalGameWinCount;		// 总胜局数
	char    Lng[12];				// 经度
	char	Lat[12];				// 纬度
	char	address[64];			// 地址
	long long lastCalcOnlineToTime; // 上次计算在线时长时间
	long long allOnlineToTime;		// 累计在线时长时间
	BYTE	IsOnline;				// 是否在线
	char	motto[128];				// 个性签名
	char	xianliao[64];			// 闲聊唯一信息
	int		controlParam;			// 个人控制权重，部分游戏使用
	int		ModifyInformationCount; // 修改信息次数
	BYTE	matchType;				// 比赛类型，MatchType类型
	long long combineMatchID;		// 报名的比赛id，gameID*MAX_GAME_MATCH_ID + id
	BYTE	matchStatus;			// 比赛状态 值为UserMatchStatus
	int		curMatchRank;			// 当前参加的比赛排名

	UserData()
	{
		memset(this, 0, sizeof(UserData));
		deskIdx = INVALID_DESKIDX;
		controlParam = -1;
	}
};

////////////////////////////////////////////////////////////////////////
// redis 中保存的信息(部分)
struct PrivateDeskInfo
{
	int	roomID;
	int	deskIdx;
	int masterID;
	char passwd[MAX_PRIVATE_DESK_PASSWD_LEN];
	int	buyGameCount;
	char gameRules[256];
	time_t createTime;
	time_t checkTime;
	BYTE masterNotPlay;
	BYTE payType;
	int	currDeskUserCount;
	int	maxDeskUserCount;
	int	currWatchUserCount;
	int	maxWatchUserCount;
	int	friendsGroupID;				// 桌子的俱乐部ID
	int friendsGroupDeskNumber;		// 俱乐部桌子号码，<=0非桌子列表，>0俱乐部桌子id
	char arrUserID[128];			// 桌子里面的玩家
	int curGameCount;				// 当前游戏局数 0：未开始，>=1正在游戏中

	PrivateDeskInfo()
	{
		memset(this, 0, sizeof(PrivateDeskInfo));
		memcpy(passwd, REDIS_STR_DEFAULT, strlen(REDIS_STR_DEFAULT));
		memcpy(gameRules, REDIS_STR_DEFAULT, strlen(REDIS_STR_DEFAULT));
	}
};

// 一局游戏战绩
struct GameGradeInfo
{
	long long id;
	int roomID;
	int deskPasswd;
	int masterID;
	time_t currTime;
	int inning;
	char videoCode[20];
	char userInfoList[192];
	char gameData[1024];

	GameGradeInfo()
	{
		memset(this, 0, sizeof(GameGradeInfo));
		memcpy(gameData, REDIS_STR_DEFAULT, strlen(REDIS_STR_DEFAULT) + 1);
	}
};

// 一个房间(桌子)战绩的简单信息(对应大结算的数据)
struct PrivateDeskGradeSimpleInfo
{
	long long id;
	int roomID;
	int passwd;
	int masterID;
	int time;
	int gameCount;
	int maxGameCount;
	char gameRules[256];
	char userInfoList[192];

	PrivateDeskGradeSimpleInfo()
	{
		memset(this, 0, sizeof(PrivateDeskGradeSimpleInfo));
	}
};

/*
json字段定义:

下注限制
noteRate        机器人总下注占比(占庄家上庄金币) 1-100值

机器人更新频率
updateRate	机器人更换频率 1-100的值

上庄机器人
zMinC   上庄机器人最小数量
zMaxC   上庄机器人最大数量

坐桌机器人
sMinC   坐桌机器人最小数量
sMaxC   坐桌机器人最大数量

机器人
minC   机器人最小数量
maxC   机器人最大数量
minM   机器人最小金币
maxM   机器人最大金币
*/
//奖池表
struct RewardsPoolInfo
{
	int roomID;						//房间id
	long long poolMoney;			//每个房间的奖池
	long long gameWinMoney;			//每个房间系统的输赢钱
	long long percentageWinMoney;	//游戏抽水获得的钱
	long long otherWinMoney;		//其它类型赢钱
	long long allGameWinMoney;		//总共：每个房间系统的输赢钱
	long long allPercentageWinMoney;//总共：游戏抽水获得的钱
	long long allOtherWinMoney;		//总共：其它类型赢钱
	BYTE platformCtrlType;			//平台控制输赢类型，0：根据platformCtrlPercent
	int platformCtrlPercent;		//点型控制输赢百分比，范围0-1000
	int realPeopleFailPercent;		//符合条件的真人输概率，范围0-1000
	int realPeopleWinPercent;		//符合条件的真人赢概率，范围0-1000
	char detailInfo[1024];          //游戏各区域控制概率 feng

	RewardsPoolInfo()
	{
		memset(this, 0, sizeof(RewardsPoolInfo));
	}
};

//小红点
struct UserRedSpot
{
	int notEMRead;			//未读邮件数量
	int notEMReceived;		//未领取邮件数量

	int friendList;			//好友列表红点数量
	int friendNotifyList;	//通知列表红点数量

	int FGNotifyList;		//俱乐部通知列表数量

	UserRedSpot()
	{
		memset(this, 0, sizeof(UserRedSpot));
	}
};

//牌桌
struct SaveRedisFriendsGroupDesk
{
	int userID;			//群主id
	int friendsGroupID;	//俱乐部id
	int friendsGroupDeskNumber; //桌子id(1-9)
	int roomType;		//房间类型
	int gameID;			//游戏ID
	int	maxCount;		//最大游戏局数
	char gameRules[256];//游戏规则
	SaveRedisFriendsGroupDesk()
	{
		memset(this, 0, sizeof(SaveRedisFriendsGroupDesk));
	}
};

//俱乐部权限
enum FriendsGroupPowerType
{
	FRIENDSGROUP_POWER_TYPE_NO = 0,			// 无权限

	FRIENDSGROUP_POWER_TYPE_DEL = 1,		// 删除成员
	FRIENDSGROUP_POWER_TYPE_DESK = 2,		// 创建和解散牌桌
	FRIENDSGROUP_POWER_TYPE_VIPROOM = 4,	// 创建和解散VIP房
	FRIENDSGROUP_POWER_TYPE_FIRE_COIN = 8,	// 兑换及充值火币
	FRIENDSGROUP_POWER_TYPE_SET = 16,		// 设置俱乐部微信或者公告或者改名

	FRIENDSGROUP_POWER_TYPE_ALL = 31,		// 全部权限，所有权限或运算得到
};

//简单邮件详情
struct EmailSimpleInfo
{
	long long emailID;	//邮件的唯一id
	int sendtime;		//发送时间
	EmailSimpleInfo()
	{
		memset(this, 0, sizeof(EmailSimpleInfo));
	}
};

struct FieldRealInfo
{
	const char* field;
	const char* value;
	int	type;
};

//用户背包
struct UserBag
{
	int		userID;				//用户ID
	int		skillFrozen;		//冰冻技能数量
	int		skillLocking;		//锁定目标技能
	int		redBag;				//红包数量
	int		phoneBillCard1;		//话费卡1元
	int		phoneBillCard5;		//话费卡5元
	int		goldenArmor;		//黄金战甲炮台（买一次永久）
	int		mechatroPioneer;	//机甲先锋（买一次永久）
	int		deepSeaArtillery;	//深海大炮（买一次永久）
	int		octopusCannon;		//章鱼大炮（买一次永久）
	int		goldenDragon;		//黄金圣龙（买一次永久）
	int		lavaArmor;			//熔岩战甲（买一次永久）

	UserBag()
	{
		memset(this, 0, sizeof(UserBag));
	}
};


//比赛场
struct MatchUserInfo
{
	int userID;
	BYTE byMatchStatus;		//比赛状态  枚举DeskMatchStatus
	time_t signUpTime;		//报名比赛时间

	MatchUserInfo()
	{
		memset(this, 0, sizeof(MatchUserInfo));
	}
};

//比赛信息
struct MatchInfo
{
	int matchID;		//比赛游戏的编号
	int gameID;			//游戏id
	int minPeople;		//比赛所需最小人数
	BYTE costResType;	//消耗资源类型
	int costResNums;	//消息资源数量
	long long startTime;//比赛开始时间
	BYTE matchStatus;	//比赛状态  MatchStatus类型 （0：报名中，1：比赛中，2：比赛结束）
	BYTE isNotified;	//是否通知过比赛即将开始

	MatchInfo()
	{
		memset(this, 0, sizeof(MatchInfo));
	}
};