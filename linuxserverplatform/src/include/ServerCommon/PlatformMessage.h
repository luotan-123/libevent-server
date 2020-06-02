#pragma once

#include "DataLine.h"
#include "Define.h"

/*
推送消息分3大类:
	1：向单个人推送
	2：向俱乐部所有在线玩家推送
	3：向登陆服所有在线玩家推送
*/

#pragma pack(1)

/////////////////////////////// PHP -----> 中心服 ///////////////////////////////////////////
// 和PHP通信的消息定义 10001-20000
const int PLATFORM_MESSAGE_BEGIN = 10000;

const int PLATFORM_MESSAGE_NOTICE = 10001;					// 公告
const int PLATFORM_MESSAGE_REQ_ALL_USER_MAIL = 10002;		// 请求全服邮件通知
const int PLATFORM_MESSAGE_CLOSE_SERVER = 10003;			// 关服，保存数据
const int PLATFORM_MESSAGE_OPEN_SERVER = 10004;				// 开服，恢复数据
const int PLATFORM_MESSAGE_SEND_HORN = 10005;				// 发送喇叭
const int PLATFORM_MESSAGE_MASTER_DISSMISS_DESK = 10006;	// 房主解散房间
const int PLATFORM_MESSAGE_FG_DISSMISS_DESK = 10007;		// 俱乐部牌桌解散房间
const int PLATFORM_MESSAGE_RESOURCE_CHANGE = 10008;			// （通知某个人）资源变化
const int PLATFORM_MESSAGE_NOTIFY_USERID = 10009;			// 向一个玩家推送消息
const int PLATFORM_MESSAGE_NOTIFY_FG = 10010;				// 向俱乐部所有玩家推送消息
const int PLATFORM_MESSAGE_IDENTUSER = 10011;				// 设置用户身份
const int PLATFORM_MESSAGE_RED_SPOT = 10012;				// 请求通知小红点
const int PLATFORM_MESSAGE_RED_FG_SPOT = 10013;				// 请求通知俱乐部小红点
const int PLATFORM_MESSAGE_RELOAD_GAME_CONFIG = 10014;		// 重新加载游戏配置
const int PLATFORM_MESSAGE_PHONE_INFO = 10015;				// 手机验证成功，注册账号
const int PLATFORM_MESSAGE_OPEN_ROOM_NOTICE = 10016;		// 推送一个新的开房信息
const int PLATFORM_MESSAGE_NOTIFY_MUCH_USERID = 10017;		// 向多个玩家推送消息
const int PLATFORM_MESSAGE_SIGN_UP_MATCH_PEOPLE = 10018;	// 有人报名或者退出比赛（实时赛）
const int PLATFORM_MESSAGE_START_MATCH_PEOPLE = 10019;		// 请求开始比赛(实时赛)
const int PLATFORM_MESSAGE_MODIFY_TIME_MATCH = 10020;		// 创建、修改、删除，一个定时赛
const int PLATFORM_MESSAGE_SIGN_UP_MATCH_TIME = 10021;		// 有人报名或者退出比赛（定时赛）

const int PLATFORM_MESSAGE_END = 20000;

enum PlatformRetCode
{
	PF_SUCCESS = 0,                 // 操作成功
	PF_NOUSER = 1,					// 用户不存在
	PF_SIZEERROR = 2,               // 数据长度不对
	PF_DATA_NULL = 3,               // 数据包为空
	PF_NOTICE_NUM_NULL = 4,         // 公告数值小于0
	PF_REDIS_NULL = 5,				// redis为空
	PF_CLOSE_STATUS_ERR = 6,		// 发送的停服状态不正确
	PF_DISSMISS_DESK_ERR = 7,       // 桌子索引小于0
	PF_SEND_DATA_ERR = 8,			// 发送数据失败
	PF_REDSPOT_NOT_EXIST = 9,		// 没有记录玩家小红点数据
};

// 发送公告消息
struct PlatformNoticeMessage
{
	char	tile[24];		// 标题
	char	content[1024];	// 内容
	int		interval;		// 间隔
	int		times;			// 次数
	int		type;			// 类型 (1:普通 2：特殊 3:弹出)

	PlatformNoticeMessage()
	{
		memset(this, 0, sizeof(PlatformNoticeMessage));
	}
};

struct PlatformCloseServerMessage //关服
{
	int status;	 // 1:关服，2：测试
	PlatformCloseServerMessage()
	{
		memset(this, 0, sizeof(PlatformCloseServerMessage));
	}
};

// 发送喇叭
struct PlatformHorn
{
	int userID;
	char userName[64];
	char content[1024];

	PlatformHorn()
	{
		memset(this, 0, sizeof(PlatformHorn));
	}
};

struct PlatformDissmissDesk
{
	int userID;
	int deskMixID;
	bool bDelete;
	int roomID;

	PlatformDissmissDesk()
	{
		memset(this, 0, sizeof(PlatformDissmissDesk));
	}
};

struct PlatformIdentUser
{
	BYTE type;			// 0 取消 1 设置
	BYTE statusValue;	// 1 超端 2 赢玩家 4 输玩家 8 封号
	int otherValue;     // 封号功能，在这填写封号时间（单位秒），-1永久封号

	PlatformIdentUser()
	{
		memset(this, 0, sizeof(PlatformIdentUser));
	}
};

struct PlatformPHPRedSpotNotify //通知小红点
{
	enum PHPRedSpotType
	{
		PHP_REDSPOT_TYPE_EMAIL = 0,			//邮件小红点
		PHP_REDSPOT_TYPE_FIREND = 1,		//好友小红点
		PHP_REDSPOT_TYPE_FG = 2,			//俱乐部小红点
	};
	BYTE redspotType;

	PlatformPHPRedSpotNotify()
	{
		memset(this, 0, sizeof(PlatformPHPRedSpotNotify));
	}
};

struct PlatformPHPReloadGameConfig //加载游戏配置
{
	int roomID;

	PlatformPHPReloadGameConfig()
	{
		memset(this, 0, sizeof(PlatformPHPReloadGameConfig));
	}
};

//手机注册信息
struct PlatformPhoneInfo
{
	char phone[24];
	char passwd[64];
	char name[64];
	char headURL[256];			// 玩家头像
	char sex;

	PlatformPhoneInfo()
	{
		memset(this, 0, sizeof(PlatformPhoneInfo));
	}
};

//报名或者退出比赛（实时赛）
struct PlatformPHPSignUpMatchPeople
{
	int gameID;			//比赛的游戏
	int matchID;		//比赛游戏的编号
	int curSignUpCount;	//当前报名人数
	int peopleCount;	//满多少人开赛
	int allSignUpCount;	//累计报名人数

	PlatformPHPSignUpMatchPeople()
	{
		memset(this, 0, sizeof(PlatformPHPSignUpMatchPeople));
	}
};

//开赛(实时赛)
struct PlatformPHPReqStartMatchPeople
{
	int gameID;			//比赛的游戏
	int matchID;		//比赛游戏的编号
	int matchRound;		//比赛轮数
	int peopleCount;	//满多少人开

	PlatformPHPReqStartMatchPeople()
	{
		memset(this, 0, sizeof(PlatformPHPReqStartMatchPeople));
	}
};

//创建、修改、删除，一个定时赛
struct PlatformPHPModifyTimeMatch
{
	BYTE type;			//0：创建，1：删除，2：修改
	int matchID;		//比赛游戏的编号
	int gameID;			//游戏id
	int minPeople;		//比赛所需最小人数
	BYTE costResType;	//消耗资源类型
	int costResNums;	//消息资源数量
	long long startTime;//比赛开始时间

	PlatformPHPModifyTimeMatch()
	{
		memset(this, 0, sizeof(PlatformPHPModifyTimeMatch));
	}
};

//报名或者退出比赛（定时赛）
struct PlatformPHPSignUpMatchTime
{
	int matchID;		//比赛游戏的编号
	int curSignUpCount;	//当前报名人数

	PlatformPHPSignUpMatchTime()
	{
		memset(this, 0, sizeof(PlatformPHPSignUpMatchTime));
	}
};

///////////////////////////////// 【中心服 -----> 登陆服】  【中心服 -----> 游戏服】 ///////////////////////////////////
const int CENTER_MESSAGE_COMMON_BEGIN = 20000;

// 中心服发送，小于20100定义在TcpConnect.h中
const int CENTER_MESSAGE_COMMON_REPEAT_ID = 20101;			// 服务器唯一id重复
const int CENTER_MESSAGE_COMMON_LOGON_GROUP_INFO = 20102;	// 登陆服集群信息
const int CENTER_MESSAGE_COMMON_RESOURCE_CHANGE = 20103;	// （通知某个人）资源变化
const int CENTER_MESSAGE_COMMON_AUTO_CREATEROOM = 20104;	// 通知登陆服务器，自动开房
const int CENTER_MESSAGE_COMMON_START_MATCH_TIME = 20105;	// 通知游戏服开始定时赛

const int CENTER_MESSAGE_COMMON_END = 30000;

struct PlatformResourceChange //通知(游戏服和大厅服)玩家资源变化
{
	int resourceType;
	long long value;		// 全量
	long long changeValue;	// 增量
	int reason;				// 原因
	int reserveData;		// 预留字段，主要保存俱乐部id
	BYTE isNotifyRoom;		// 是否通知到游戏

	PlatformResourceChange()
	{
		memset(this, 0, sizeof(PlatformResourceChange));
	}
};

struct PlatformDistributedSystemInfo
{
	UINT logonGroupIndex;		//登陆服集群索引
	UINT logonGroupCount;		//登陆服集群数量
	UINT mainLogonGroupIndex;	//当前主要登陆服集群索引（相比其它服务器集群处理更多数据）

	PlatformDistributedSystemInfo()
	{
		memset(this, 0, sizeof(PlatformDistributedSystemInfo));
	}
};

struct PlatformRepeatUserLogon
{
	int userID;

	PlatformRepeatUserLogon()
	{
		memset(this, 0, sizeof(PlatformRepeatUserLogon));
	}
};

struct PlatformAutoCreateRoom
{
	int masterID;
	int	friendsGroupID;			// 桌子的俱乐部ID
	int friendsGroupDeskNumber;	// 俱乐部桌子号码，<=0非桌子列表，>0俱乐部桌子id
	int roomType;				//房间类型
	int gameID;			//游戏ID
	int	maxCount;		//最大游戏局数
	char gameRules[256];//游戏规则

	PlatformAutoCreateRoom()
	{
		memset(this, 0, sizeof(PlatformAutoCreateRoom));
	}
};

//开赛(定时赛)
struct PlatformReqStartMatchTime
{
	int gameID;			//游戏id
	int matchID;		//比赛游戏的编号
	int minPeople;		//比赛所需最小人数
	BYTE costResType;	//消耗资源类型
	int costResNums;	//消息资源数量

	PlatformReqStartMatchTime()
	{
		memset(this, 0, sizeof(PlatformReqStartMatchTime));
	}
};

//////////////////////////// 登陆服 -----> 中心服 ////////////////////////////////////
const int CENTER_MESSAGE_LOGON_BEGIN = 30000;

const int CENTER_MESSAGE_LOGON_RESOURCE_CHANGE = 30001;			// 资源变化
const int CENTER_MESSAGE_LOGON_USER_LOGON_OUT = 30002;			// 玩家登录，或者掉线
const int CENTER_MESSAGE_LOGON_REPEAT_USER_LOGON = 30003;		// 玩家重复登录
const int CENTER_MESSAGE_LOGON_RELAY_USER_MSG = 30004;			// 转发消息给个人
const int CENTER_MESSAGE_LOGON_RELAY_FG_MSG = 30005;			// 转发消息给俱乐部
const int CENTER_MESSAGE_LOGON_REQ_FG_REDSPOT_MSG = 30006;		// 请求给俱乐部所有玩家发送小红点
const int CENTER_MESSAGE_LOGON_MASTER_DISSMISS_DESK = 30007;	// 请求解散房间

const int CENTER_MESSAGE_LOGON_END = 40000;


struct PlatformUserLogonLogout
{
	int userID;
	BYTE type; //类型说明 0:新增 1：移除
	BYTE isVirtual;

	PlatformUserLogonLogout()
	{
		memset(this, 0, sizeof(PlatformUserLogonLogout));
	}
};

//服务器推送的小红点数量
struct PlatformFriendsGroupPushRedSpot
{
	int friendsGroupID;			//俱乐部id
	int deskMsgRedSpotCount;    //牌桌消息数量
	int VIPRoomMsgRedSpotCount;	//VIP房间消息数量

	PlatformFriendsGroupPushRedSpot()
	{
		memset(this, 0, sizeof(PlatformFriendsGroupPushRedSpot));
	}
};

////////////////////////////游戏服 -----> 中心服////////////////////////////////////
const int CENTER_MESSAGE_LOADER_BEGIN = 40000;

const int CENTER_MESSAGE_LOADER_RESOURCE_CHANGE = 40001;			// 资源变化(金币和钻石变化)
const int CENTER_MESSAGE_LOADER_BUYDESKINFO_CHANGE = 40002;			// 开房信息发生变化(人数变化)
const int CENTER_MESSAGE_LOADER_DESK_DISSMISS = 40003;				// 桌子解散
const int CENTER_MESSAGE_LOADER_DESK_STATUS_CHANGE = 40004;			// 桌子局数变化
const int CENTER_MESSAGE_LOADER_FIRECOIN_CHANGE = 40005;			// 资源变化(火币变化)
const int CENTER_MESSAGE_LOADER_REWARD_ACTIVITY = 40006;			// 活动大奖通知
const int CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH = 40007;			// 通知前端进入游戏开始比赛
const int CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH_FAIL = 40008;	// 通知前端比赛开始失败

const int CENTER_MESSAGE_LOADER_END = 50000;

//俱乐部消息结构体
struct PlatformFriendsGroupMsg
{
	int	friendsGroupID;			// 桌子的俱乐部ID
	int friendsGroupDeskNumber;	// 俱乐部桌子号码，<=0非桌子列表，>0俱乐部桌子id
	bool bDeleteDesk;			// 俱乐部房间是否继续开房

	PlatformFriendsGroupMsg()
	{
		Init();
	}
	void Init()
	{
		memset(this, 0, sizeof(PlatformFriendsGroupMsg));
	}
};

// 人数变化
struct PlatformDeskPeopleCountChange
{
	int	friendsGroupID;			// 桌子的俱乐部ID
	int friendsGroupDeskNumber;	// 俱乐部桌子号码，<=0非桌子列表，>0俱乐部桌子id
	int masterID;
	char roomPasswd[20];
	int currUserCount;
	BYTE updateType;			// 0:增加，1：减少
	int userID;					// 增加或减少玩家的ID
	PlatformDeskPeopleCountChange()
	{
		memset(this, 0, sizeof(PlatformDeskPeopleCountChange));
	}
};

// 游戏大结算
struct PlatformDeskDismiss
{
	int masterID;
	int	friendsGroupID;			// 桌子的俱乐部ID
	int friendsGroupDeskNumber;	// 俱乐部桌子号码，<=0非桌子列表，>0俱乐部桌子id
	char passwd[20];			// 房间号
	BYTE bDeleteDesk;			// 俱乐部房间是否继续开房

	// 自动开房使用
	int roomType;				//房间类型
	int gameID;					//游戏ID
	int	maxCount;				//最大游戏局数
	char gameRules[256];		//游戏规则

	PlatformDeskDismiss()
	{
		memset(this, 0, sizeof(PlatformDeskDismiss));
	}
};

// 局数变化
struct PlatformDeskGameCountChange
{
	int	friendsGroupID;			// 桌子的俱乐部ID
	int friendsGroupDeskNumber;	// 俱乐部桌子号码，<=0非桌子列表，>0俱乐部桌子id
	BYTE gameStatus;			// 游戏状态，0：未开始，1正在游戏中
	PlatformDeskGameCountChange()
	{
		memset(this, 0, sizeof(PlatformDeskGameCountChange));
	}
};

// 通知开赛
struct PlatformLoaderNotifyStartMatch
{
	int gameID;			//比赛的游戏
	BYTE matchType;		//比赛类型
	int matchID;		//比赛游戏的编号
	int roomID;
	int peopleCount;	//参赛人数
	int userID[MAX_MATCH_PEOPLE_COUNT];

	PlatformLoaderNotifyStartMatch()
	{
		memset(this, 0, sizeof(PlatformLoaderNotifyStartMatch));
	}
};

// 通知开赛失败
struct PlatformLoaderNotifyStartMatchFail
{
	int gameID;			//比赛的游戏
	BYTE matchType;		//比赛类型
	int matchID;		//比赛游戏的编号
	BYTE reason;		//比赛失败原因

	PlatformLoaderNotifyStartMatchFail()
	{
		memset(this, 0, sizeof(PlatformLoaderNotifyStartMatchFail));
	}
};

#pragma pack()
