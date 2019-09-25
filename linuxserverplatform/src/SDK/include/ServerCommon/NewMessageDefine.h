#pragma once

#include "Define.h"

//////////////////////////////////////////////////////////////////////////
const unsigned int MSG_MAIN_TEST = 1;					// 测试消息

const unsigned int 	MSG_ASS_TEST = 1;					// 测试消息

//////////////////////////////////////////////////////////////////////////
const unsigned int MSG_MAIN_CONECT = 2;					// 连接测试

//////////////////////////////////////////////////////////////////////////

const unsigned int MSG_MAIN_USER_SOCKET_CLOSE = 4;		// 玩家掉线通知游戏服

// 消息定义文件
// 大厅消息大ID范围			100-499
// 游戏消息大ID范围			500-999


//////////////////////////////////////////////////////////////////////
// 大厅消息定义
//////////////////////////////////////////////////////////////////////

// 注册相关
const unsigned int MSG_MAIN_LOGON_REGISTER = 100;


// 登录相关
const unsigned int MSG_MAIN_LOGON_LOGON = 101;

const unsigned int MSG_ASS_LOGON_LOGON = 1; //账号登录
// 桌子相关
const unsigned int MSG_ASS_LOGON_DESK = 102;

const unsigned int MSG_ASS_LOGON_BUY_DESK = 1;			// 购买桌子
const unsigned int MSG_ASS_LOGON_ENTER_DESK = 2;		// 进入桌子


// 通知相关
const unsigned int MSG_MAIN_LOGON_NOTIFY = 103;

const unsigned int MSG_NTF_LOGON_USER_SQUEEZE = 1;			// 玩家被顶号（挤掉）
const unsigned int MSG_NTF_LOGON_NOTICE = 2;				// 公告
const unsigned int MSG_NTF_LOGON_HORN = 3;					// 喇叭
const unsigned int MSG_NTF_LOGON_USER_EXIST = 4;			// 玩家已登录
const unsigned int MSG_NTF_LOGON_RESOURCE_CHANGE = 5;		// 资源变化
const unsigned int MSG_NTF_LOGON_SUPPORT_INFO = 6;			// 破产补助信息（登录成功之后额外推送）
const unsigned int MSG_NTF_LOGON_BUYDESKINFO_CHANGE = 7;	// 通知开房列表信息变化
const unsigned int MSG_NTF_LOGON_FRIEND_NOTIFY = 8;			// 好友通知
const unsigned int MSG_NTF_LOGON_FRIEND_ADDOK = 9;			// 添加好友成功
const unsigned int MSG_NTF_LOGON_FRIEND_DELOK = 10;			// 删除好友成功
const unsigned int MSG_NTF_LOGON_FIREND_DELNOTIFY = 11;		// 删除好友通知
const unsigned int MSG_NTF_LOGON_RIEND_LOGINOROUT = 12;		// 通知好友登录或者登出
const unsigned int MSG_NTF_LOGON_FRIEND_REDSPOT = 13;		// 通知好友小红点
const unsigned int MSG_NTF_LOGON_CLOSE_SERVER = 14;			// 关服更新通知
const unsigned int MSG_NTF_LOGON_EMAIL_REDSPOT = 15;		// 邮件小红点
const unsigned int MSG_NTF_LOGON_NEW_EMAIL = 16;			// 新邮件通知
const unsigned int MSG_NTF_LOGON_REWARD_ACTIVITY = 17;		// 活动大奖公告通知

// 其他
const unsigned int MSG_MAIN_LOGON_OTHER = 104;

const unsigned int MSG_ASS_LOGON_OTHER_SEND_HORN = 1;				// 请求发送喇叭
const unsigned int MSG_ASS_LOGON_OTHER_SEND_GIFT = 2;				// 请求转赠
const unsigned int MSG_ASS_LOGON_OTHER_SEND_GRADE = 3;				// 请求战绩
const unsigned int MSG_ASS_LOGON_OTHER_SEND_ROOMOPENED = 4;			// 请求房主替他人开房信息
const unsigned int MSG_ASS_LOGON_OTHER_USERINFO_FLUSH = 5;			// 刷新个人信息
const unsigned int MSG_ASS_LOGON_OTHER_EMAIL_LIST = 6;				// 邮件列表信息
const unsigned int MSG_ASS_LOGON_OTHER_EMAIL_INFO = 7;				// 邮件内容信息
const unsigned int MSG_ASS_LOGON_OTHER_EMAIL_DELETE = 8;			// 删除邮件
const unsigned int MSG_ADD_LOGON_OTHER_REQ_SUPPORT = 9;				// 请求破产补助
const unsigned int MSG_ASS_LOGON_OTHER_ROBOT_TAKEMONEY = 10;		// 机器人取钱
const unsigned int MSG_ASS_LOGON_OTHER_REQ_SIMPLE_GRADELIST = 11;	// 请求战绩列表(大结算)
const unsigned int MSG_ASS_LOGON_OTHER_REQ_SIMPLE_GRADEINFO = 12;	// 请求战绩信息(大结算)
const unsigned int MSG_ASS_LOGON_OTHER_REQ_GRADELIST = 13;			// 请求单局游戏战绩列表
const unsigned int MSG_ASS_LOGON_OTHER_REQ_GRADEINFO = 14;			// 请求单局游戏信息
const unsigned int MSG_ASS_LOGON_OTHER_REQ_USERINFO = 15;			// 查询玩家信息
const unsigned int MSG_ASS_LOGON_OTHER_REQ_DISSMISS_DESK = 16;		// 请求解散房间
const unsigned int MSG_ASS_LOGON_OTHER_REQ_SAVE_POSITION = 17;		// 请求保存位置
const unsigned int MSG_ASS_LOGON_OTHER_REQ_ENTER_OR_LEAVE_THE_GAMERO = 18;	// 请求进入房间
const unsigned int MSG_ASS_LOGON_OTHER_JOIN_MATCH_SCENE = 19;		// 加入比赛场场景
const unsigned int MSG_ASS_LOGON_OTHER_EXIT_MATCH_SCENE = 20;		// 退出比赛场场景

// 银行
const unsigned int MSG_MAIN_LOGON_BANK = 105;

const unsigned int MSG_ASS_LOGON_BANK_SAVE_MONEY = 1;				// 存钱
const unsigned int MSG_ASS_LOGON_BANK_TAKE_MONEY = 2;				// 取钱
const unsigned int MSG_ASS_LOGON_BANK_TRANSFER = 3;					// 转账
const unsigned int MSG_ASS_LOGON_BANK_SETPASSWD = 4;				// 设置密码
const unsigned int MSG_ASS_LOGON_BANK_RESETPASSWD = 5;				// 修改密码
const unsigned int MSG_ASS_LOGON_BANK_RECORD = 6;					// 银行记录

// 好友
const unsigned int MSG_MAIN_LOGON_FRIEND = 106;

const unsigned int MSG_ASS_LOGON_FRIEND_LIST = 1;				// 好友信息
const unsigned int MSG_ASS_LOGON_FRIEND_ADD = 2;				// 添加好友
const unsigned int MSG_ASS_LOGON_FRIEND_ANSWER_ADD = 3;			// 回应添加好友
const unsigned int MSG_ASS_LOGON_FRIEND_DEL = 4;				// 删除好友
const unsigned int MSG_ASS_LOGON_FRIEND_REWARD = 5;				// 打赏
const unsigned int MSG_ASS_LOGON_FRIEND_TAKEREWARD = 6;			// 领取打赏
const unsigned int MSG_ASS_LOGON_FRIEND_SERACH = 7;				// 搜索玩家
const unsigned int MSG_ASS_LOGON_FRIEND_CHANGEBATCH = 8;		// 换一批
const unsigned int MSG_ASS_LOGON_FRIEND_NOTIFYLIST = 9;			// 查询通知列表
const unsigned int MSG_ASS_LOGON_FRIEND_DELNOTIFY = 10;			// 删除通知
const unsigned int MSG_ASS_LOGON_FRIEND_INVITEPLAY = 11;		// 邀请在线好友(玩游戏)
const unsigned int MSG_ASS_LOGON_FRIEND_DEL_REDSPOT = 12;		// 请求删除小红点

// 俱乐部
const unsigned int MSG_MAIN_LOGON_FRIENDSGROUP = 107;

const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_LIST = 1;				// 俱乐部列表
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_MEMBER = 2;			// 俱乐部成员
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_ALL_NOTIFY_MSG = 3;	// 玩家所有通知消息（某个时间以前的8k消息）
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_ALL_ROOM_MSG = 4;		// 俱乐部所有开房消息（某个时间以前的8k消息）
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_ALL_ACCO_MSG = 5;		// 俱乐部所有战绩消息（某个时间以前的8k消息）
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_CREATE = 6;			// 创建俱乐部
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_REQ_JOIN = 7;			// 申请加入俱乐部
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_INVITE = 8;			// 邀请好友加入俱乐部
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_ANSWER_OPER = 9;      // 回应通知消息
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_DEL_MEMBER = 10;		// 群主删除俱乐部成员
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_QUIT = 11;			// 退出俱乐部
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_DISMISS = 12;			// 解散俱乐部
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_CHANGE_NAME = 13;		// 改变俱乐部名字
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_DEL_NOTIFY_MSG = 14;	// 删除一条通知消息
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_SIMPLE_INFO = 15;		// 查询俱乐部简单信息
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_DEL_REDSPOT = 16;		// 请求删除小红点
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_SEND_NOTICE = 17;		// 请求发布公告
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_QUERY_ACCO = 18;		// 查询战绩统计
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_DESK_LIST_MSG = 19;	// 俱乐部桌子列表
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_DELETE_DESK_INFO = 20;// 删除俱乐部一个房间
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_COST_STATISTICS = 21; // 查询消耗统计
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_REQ_TRANSFER = 22;	// 请求转让俱乐部
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_REQ_AUTH_MEM = 23;	// 请求授权俱乐部成员或者取消用户身份
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_MODIFY_WECHAT = 24;	// 请求修改微信
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_MODIFY_POWER = 25;	// 请求修改管理员权限
const unsigned int MSG_ASS_LOGON_FRIENDSGROUP_CHANGE_FIRECOIN = 26;	// 请求充值或者兑换玩家火币

// 俱乐部通知
const unsigned int MSG_MAIN_FRIENDSGROUP_NOTIFY = 108;

const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_NOTIFY_MSG = 1;		// 服务器主动推送的一条通知消息
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_ROOM_MSG = 2;			// 服务器主动推送的俱乐部开房消息
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_ACCO_MSG = 3;			// 服务器主动推送的俱乐部战绩消息
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_REDSPOT = 4;			// 通知俱乐部小红点
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_MSG_HAVE_CHANGE = 5;	// 俱乐部聊天消息变化通知
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_CREATE_FG = 6;		// 服务器主动推送一个俱乐部
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_NAME_CHANGE = 7;		// 俱乐部名字变化通知
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_PEOPLE_CHANGE = 8;	// 俱乐部人变化通知
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_DISMISS = 9;			// 俱乐部被解散
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_NOTICE_CHANGE = 10;	// 俱乐部公告变化通知
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_NEW_DESK_MSG = 11;	// 牌桌消息(增加)
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_DESK_INFO_CHANGE = 12;// 牌桌变化通知（更新和删除）
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_STATUS_CHANGE = 13;	// 俱乐部身份变化通知
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_WECHAT_CHANGE = 14;	// 俱乐部微信变化通知
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_POWER_CHANGE = 15;	// 俱乐部玩家权限变更
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_NEW_VIPROOM_MSG = 16;	// 推送的俱乐部VIP房间(增加)
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_VIPROOM_CHANGE = 17;  // vip房间变化通知（更新和删除）
const unsigned int MSG_NTF_LOGON_FRIENDSGROUP_FIRECOIN_CHANGE = 18; // 玩家火币变化通知

// 比赛场通知
const unsigned int MSG_MAIN_MATCH_NOTIFY = 109;

const unsigned int MSG_NTF_LOGON_MATCH_NOTIFY_SIGNUP_CHANGE_PEOPLE = 1;	// 报名人数变化(实时赛)
const unsigned int MSG_NTF_LOGON_MATCH_NOTIFY_START_MATCH = 2;			// 通知开赛比赛
const unsigned int MSG_NTF_LOGON_MATCH_NOTIFY_SIGNUP_CHANGE_TIME = 3;	// 报名人数变化(定时赛)
const unsigned int MSG_NTF_LOGON_MATCH_NOTIFY_START_MATCH_FAIL = 4;		// 通知开赛失败
const unsigned int MSG_NTF_LOGON_MATCH_NOTIFY_START_MATCH_RIGHTAWAY = 5;// 通知比赛即将开始

//////////////////////////////////////////////////////////////////////////
#pragma pack(1)

//枚举定义

// 资源变化原因
enum ResourceChangeReason
{
	// 游戏服集群使用
	RESOURCE_CHANGE_REASON_DEFAULT = 0,
	RESOURCE_CHANGE_REASON_CREATE_ROOM,				// 创建房间消耗
	RESOURCE_CHANGE_REASON_GAME_BEGIN,				// 游戏开始
	RESOURCE_CHANGE_REASON_GAME_FINISHED,			// 游戏结束
	RESOURCE_CHANGE_REASON_GAME_SELLETE_ROLLBACK,	// 大结算没有生成战绩返还
	RESOURCE_CHANGE_REASON_GAME_SELLETE_NORAML,		// 大结算普通支付
	RESOURCE_CHANGE_REASON_GAME_SELLETE_AA,			// 游戏大结算AA支付
	RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP,			// 金币房抽水返还
	RESOURCE_CHANGE_REASON_ROOM_PUMP_CONSUME,		// 房间抽水扣除
	RESOURCE_CHANGE_REASON_SYSTEM_SUBSIDY,			// 系统补贴金币(原因是游戏扣除金币扣成了负数)
	RESOURCE_CHANGE_REASON_REGISTER,				// 注册
	RESOURCE_CHANGE_REASON_MAGIC_EXPRESS,			// 魔法表情

	// PHP服使用
	RESOURCE_CHANGE_REASON_BANK_SAVE = 1000, 		// 银行存钱
	RESOURCE_CHANGE_REASON_BANK_TAKE,           	// 银行取钱
	RESOURCE_CHANGE_REASON_BANK_TRAN,				// 银行转账
	RESOURCE_CHANGE_REASON_GIVE,		 			// 转赠
	RESOURCE_CHANGE_REASON_SUPPORT,			 		// 救济金
	RESOURCE_CHANGE_REASON_SIGN,			 		// 签到
	RESOURCE_CHANGE_REASON_BACK_RECHARE,			// 后台充值或者提现
	RESOURCE_CHANGE_REASON_PAY_RECHARE,			 	// 商城支付充值
	RESOURCE_CHANGE_REASON_AGENT_RECHARE,			// 代理充值
	RESOURCE_CHANGE_REASON_TURNTABLE,				// 转盘
	RESOURCE_CHANGE_REASON_SHARE,					// 分享
	RESOURCE_CHANGE_REASON_FRIEND_REWARD,			// 好友打赏
	RESOURCE_CHANGE_REASON_BIND_PHONE,				// 绑定手机
	RESOURCE_CHANGE_REASON_SEND_HORN,				// 世界广播
	RESOURCE_CHANGE_REASON_MAIL_REWARD,				// 邮件奖励
	RESOURCE_CHANGE_REASON_BIND_AGENT,				// 绑定代理
	RESOURCE_CHANGE_REASON_INVITE_ENTER,			// 邀请进入游戏
};

enum NOTIFY_MSG_TYPE
{
	SMT_CHAT = 1,				//聊天消息
	SMT_EJECT = 2,				//弹出消息
	SMT_GLOBAL = 4,				//全局消息
	SMT_PROMPT = 8,				//提示消息
	SMT_TABLE_ROLL = 16,		//滚动消息
};

///////////////////////////////////////////
// 结构体定义

// 请求注册
struct LogonRequestRegister
{
	BYTE		byFromPhone;			//0-从PC注册，1-从移动端注册
	BYTE		byFastRegister;			//0-快速注册，1-普通注册,2-微信注册，3-QQ注册，4-手机注册
	char		szAccount[64];			//用户名 微信昵称 手机号码
	char		szPswd[64];				//密码 微信、QQ登录标识
	BYTE		byStatus;			    //0 注册 1 登录 针对账号登录	
	BYTE		bySex;					//性别
	char		szHeadUrl[256];			//头像URL
	char		szAccountInfo[64];		//唯一标识

	// 选择性发送
	BYTE		byPlatformType;			//默认0   1是ios  2安卓   3、win32
	LogonRequestRegister()
	{
		memset(this, 0, sizeof(LogonRequestRegister));
	}
};

// 回应注册
struct LogonResponseRegister
{
	int  userID;			//用户ID
	char szToken[64];		//用户Token（sessioncode）
	char szAccountInfo[64];	//唯一标识
	BYTE byFastRegister;	//1-普通注册,2-微信注册，3-QQ注册

	LogonResponseRegister()
	{
		memset(this, 0, sizeof(LogonResponseRegister));
	}
};

// 请求登录大厅
struct LogonRequestLogon
{
	char		szMacAddr[64];						//mac地址
	int		    iUserID;							//用户ID登录，如果ID>0用ID登录
	char        szToken[64];                        //用户Token（sessioncode）
	char        szVersions[24];                     //版本号

	// 选择性发送
	BYTE		byPlatformType;						//默认0   1是ios  2安卓   3、win32
	LogonRequestLogon()
	{
		memset(this, 0, sizeof(LogonRequestLogon));
	}
};

// 登录大厅返回
struct LogonResponseLogon
{
	char	name[64];
	char	phone[24];
	BYTE	sex;
	long long money;
	int		jewels;					// 房卡数(钻石)
	int		roomID;					// 玩家所在的roomID
	int		deskIdx;				// 玩家所在的桌子索引
	char	logonIP[24];			// 玩家的登录IP
	char	headURL[256];			// 玩家头像
	long long bankMoney;			// 银行金币
	char	bankPasswd[20];			// 银行密码
	char	longitude[12];			// 经度
	char	latitude[12];			// 纬度
	char	address[64];			// 地址
	char	motto[128];				// 个性签名
	BYTE	visitor;				// 是否是游客
	long long combineMatchID;		// 报名的比赛id，gameID*MAX_GAME_MATCH_ID + id
	BYTE	matchStatus;			// 比赛状态 值为UserMatchStatus

	LogonResponseLogon()
	{
		memset(this, 0, sizeof(LogonResponseLogon));
	}
};

struct LogonResponseFail
{
	BYTE byType; //0 账号被封
	int iRemainTime;//剩余时间 ，-1永久封号
	LogonResponseFail()
	{
		memset(this, 0, sizeof(LogonResponseFail));
	}
};

// 请求购买桌子
struct LogonRequestBuyDesk
{
	int		userID;				// 用户ID
	int		roomType;			// 房间类型，1：积分，2：金币房，3：vip房间
	int		gameID;				// 游戏ID
	int		count;				// 购买局数
	BYTE	needExt;			// 是否自动扩展（0:不自动扩展，满局就解散 1:是）
	char	gameRules[256];		// 游戏规则。 json 格式，游戏动态库端和客户端自行定义 {"zhuang":"0"}
	BYTE	masterNotPlay;		// 是否替他人开房(1:Yes 0:No)
	int		friendsGroupID;     // 桌子的俱乐部ID
	int		friendsGroupDeskNumber;	// 号码，0：vip房间(vip房间的号码由系统自动分配)，大于0：牌桌

	LogonRequestBuyDesk()
	{
		memset(this, 0, sizeof(LogonRequestBuyDesk));
	}
};

// 购买桌子结果
struct LogonResponseBuyDesk
{
	int		userID;					// 用户ID
	int		roomID;					// 房间号
	int		deskIdx;				// 桌子号
	char	passwd[20];				// 桌子密码
	int		passwdLen;				// 密码长度
	int		jewels;					// 所消耗钻石
	BYTE	byMasterNotPlay;		// 是否替他人开房(1:Yes 0:No)

	LogonResponseBuyDesk()
	{
		memset(this, 0, sizeof(LogonResponseBuyDesk));
	}
};

//进入VIP桌子
struct LogonRequestEnterDesk
{
	int		userID;
	int		passwdLen;
	char	passwd[20];

	LogonRequestEnterDesk()
	{
		memset(this, 0, sizeof(LogonRequestEnterDesk));
	}
};

//进入VIP桌子结果
struct LogonResponseEnterDesk
{
	int		userID;
	int		roomID;
	int		deskIdx;

	LogonResponseEnterDesk()
	{
		memset(this, 0, sizeof(LogonResponseEnterDesk));
	}
};

// 通知公告
struct LogonNotifyNotice
{
	int		interval;		// 间隔
	int		times;			// 次数
	int		type;			// 类型(1:普通 2:特殊)
	char	tile[24];		// 标题
	int		sizeCount;		// 内容字节数量
	char	content[1024];	// 内容

	LogonNotifyNotice()
	{
		memset(this, 0, sizeof(LogonNotifyNotice));
	}
};

// 通知喇叭
struct LogonNotifyHorn
{
	int userID;
	char userName[64];
	int sizeCount;     //content字节数量
	char content[1024];

	LogonNotifyHorn()
	{
		memset(this, 0, sizeof(LogonNotifyHorn));
	}
};

// 爆奖获得
struct LogonNotifyActivity
{
	int		sizeCount;		// 内容字节数量
	char	content[1024];	// 内容

	LogonNotifyActivity()
	{
		memset(this, 0, sizeof(LogonNotifyActivity));
	}
};

struct LogonNotifyResourceChange
{
	int resourceType;
	long long value;
	int reason;				// 原因
	int param1;				// 参数
	long long changeValue;  // 变化量

	LogonNotifyResourceChange()
	{
		memset(this, 0, sizeof(LogonNotifyResourceChange));
	}
};

//个人信息刷新
struct LogonUserInfoFlush
{
	int iJewels;//房卡
	long long money; //金币

	LogonUserInfoFlush()
	{
		memset(this, 0, sizeof(LogonUserInfoFlush));
	}
};

// 通知开房列表信息变化
struct LogonNotifyBuyDeskInfoChange
{
	enum ChangeType
	{
		CHANGE_REASON_USERCOUNT_CHANGE = 1,		// 人数变化
		CHANGE_REASON_DEL = 2,					// 删除桌子（大结算）
		CHANGE_REASON_BEGIN = 3,				// 游戏开始（局数变化）
	};

	int changeType;		//对应ChangeType
	char passwd[20];
	int currUserCount;

	LogonNotifyBuyDeskInfoChange()
	{
		memset(this, 0, sizeof(LogonNotifyBuyDeskInfoChange));
	}
};

//机器人取钱
struct _LogonResponseRobotTakeMoney
{
	int iMoney;
	int iRoomID;
	_LogonResponseRobotTakeMoney()
	{
		memset(this, 0, sizeof(_LogonResponseRobotTakeMoney));
	}
};

struct _LogonRobotTakeMoneyRes
{
	int iMoney;
	_LogonRobotTakeMoneyRes()
	{
		memset(this, 0, sizeof(_LogonRobotTakeMoneyRes));
	}
};

struct LogonRequestUserInfo
{
	int userID;

	LogonRequestUserInfo()
	{
		memset(this, 0, sizeof(LogonRequestUserInfo));
	}
};

struct LogonResponseUserInfo
{
	char name[64];
	char headURL[256];
	BYTE sex;
	int gameCount;
	int winCount;
	BYTE onlineStatus;
	char longitude[12];
	char latitude[12];
	char address[64];
	int jewels;
	long long money;
	char ip[24];
	char motto[128];// 个性签名

	LogonResponseUserInfo()
	{
		memset(this, 0, sizeof(LogonResponseUserInfo));
	}
};

// 邮件小红点通知
struct LogonNotifyEmailRedSpot
{
	int notReadCount;	 //未读邮件数量
	int notReceivedCount;//未领取邮件数量
	LogonNotifyEmailRedSpot()
	{
		memset(this, 0, sizeof(LogonNotifyEmailRedSpot));
	}
};

// 通知好友小红点
struct LogonNotifyFriendRedSpot
{
	int friendListRedSpotCount;		// 好友列表红点数量
	int notifyListRedSpotCount;		// 通知列表红点数量
	LogonNotifyFriendRedSpot()
	{
		memset(this, 0, sizeof(LogonNotifyFriendRedSpot));
	}
};


//////////////////////////////////////////////////////////////////////////
//俱乐部相关协议

//玩家的俱乐部列表
struct OneFriendsGroupInfo
{
	int friendsGroupID;
	BYTE status;		//俱乐部的身份，1群主，0普通人员，2管理员
	int currOnlineCount;//当前在线人数
	int peopleCount;	//俱乐部人数
	int deskCount;		//已开牌桌数量
	int VIPRoomCount;	//已开VIP房间数量
	int frontMember[9];	//前面几个玩家userID
	int createTime;
	int masterID;
	char name[24];		//俱乐部名字
	char notice[128];	//俱乐部公告
	char wechat[48];	//微信群
	BYTE power;			//这个俱乐部的权限
	int carryFireCoin;	//在这个俱乐部携带的火币
	OneFriendsGroupInfo()
	{
		memset(this, 0, sizeof(OneFriendsGroupInfo));
	}
};

//请求俱乐部成员以及请求俱乐部简单信息
struct LogonRequestFriendsInfo
{
	int friendsGroupID;		// 俱乐部id
	int page;				// 第几页，第一次默认0
};
struct OneFriendsGroupUserInfo
{
	int userID;
	int joinTime;
	BYTE onlineStatus;		//在线状态 0:不在线 1：在线
	int score;
	long long money;
	int fireCoin;			//累计输赢的火币
	BYTE status;			//俱乐部的身份，1群主，0普通人员，2管理员
	int carryFireCoin;		//身上携带火币
	BYTE power;				//玩家的权限
	OneFriendsGroupUserInfo()
	{
		memset(this, 0, sizeof(OneFriendsGroupUserInfo));
	}
};
struct LogonResponseFriendsGroupMember
{
	int allPage;							//总共页
	int curPage;							//当前页
	int pageDataCount;						//当前页数据量
	OneFriendsGroupUserInfo userInfo[200];
	LogonResponseFriendsGroupMember()
	{
		memset(this, 0, sizeof(LogonResponseFriendsGroupMember));
	}
};
struct LogonResponseFriendsGroupSimple
{
	int friendsGroupID;
	char name[24];//俱乐部名字
	int masterID; //群主ID
	char notice[128]; //俱乐部公告
	LogonResponseFriendsGroupSimple()
	{
		memset(this, 0, sizeof(LogonResponseFriendsGroupSimple));
	}
};

//请求俱乐部的所有通知消息
struct LogonRequestFriendsGroupAllNotify
{
	int time; //时间点，time=0:请求最新的数据
};
struct OneFriendsGroupNotify
{
	int id; //消息id，唯一标识
	int userID;
	int targetFriendsGroupID;
	char name[24];
	BYTE type;//消息类型
	int time; //消息产生时间
	int param1;
	int param2;
	BYTE alreadyOper; //是否被其它玩家操作,0:否，1：是
	OneFriendsGroupNotify()
	{
		memset(this, 0, sizeof(OneFriendsGroupNotify));
	}
};

//请求俱乐部开房列表消息
struct LogonRequestFriendsGroupRoomMsg
{
	int friendsGroupID; //俱乐部id
	int time; //时间点，time=0:请求最新的数据
};
struct OneFriendsGroupRoomMsg //一条俱乐部开房消息
{
	int friendsGroupID;//俱乐部id
	int userID;		   //这条消息是谁发送的
	int id;            //消息ID,每个俱乐部消息id唯一
	int time;          //消息产生时间
	int roomID;		   //房间ID
	int roomCount;	   //更新开房数量，当前俱乐部所有的开房数量，提供给客户端更新开房数量
	int curPeopleCount;//当前人数
	int allPeopleCount;//总人数
	int playCount;     //游戏局数
	char passwd[20];   //桌子密码
	char gameRules[256];//游戏规则
	OneFriendsGroupRoomMsg()
	{
		memset(this, 0, sizeof(OneFriendsGroupRoomMsg));
	}
};

//请求俱乐部的战绩消息
struct LogonRequestFriendsGroupAccoMsg
{
	int friendsGroupID; //俱乐部id
	int firstTime;		//起始时间
	int endTime;		//截止时间。截止时间为0：表示至今
	BYTE roomType;		//0所有，1:积分房，2：金币房，3：VIP房
};
struct OneFriendsGroupAccountMsg //一条俱乐部结算消息
{
	int friendsGroupID;		//俱乐部id
	int userID;				//这条消息是谁发送的，userID=0，战绩默认userID=0
	int time;				//消息产生时间
	int roomID;				//房间ID
	int realPlayCount;		//真正游戏局数
	int playCount;			//最大游戏局数
	BYTE playMode;			//游戏模式，对应json["gameIdx"]，所有游戏必须统一
	char passwd[20];		//桌子密码
	char userInfoList[192];	//战绩列表
	BYTE roomType;			//房间类型
	BYTE srcType;			//桌号
	OneFriendsGroupAccountMsg()
	{
		memset(this, 0, sizeof(OneFriendsGroupAccountMsg));
	}
};

//请求创建俱乐部
struct LogonRequestCreateFriendsGroup
{
	char name[24];//俱乐部名字
};

//申请加入俱乐部
struct LogonRequestJoinFriendsGroup
{
	int friendsGroupID; //俱乐部id
};

//邀请好友加入俱乐部
struct LogonRequestAddFriendJoin
{
	int friendID;       //被邀请的好友id
	int friendsGroupID; //要加入的俱乐部id
};

//请求回应通知消息
struct LogonRequestAnswerOper
{
	int id;				//操作的消息的唯一id
	BYTE operType;		//操作类型,1同意，2拒绝
	LogonRequestAnswerOper()
	{
		memset(this, 0, sizeof(LogonRequestAnswerOper));
		operType = 1;
	}
};

//删除俱乐部成员
struct LogonRequestDelMember
{
	int friendsGroupID; //俱乐部id
	int targetID;		//要删除成员ID
};

//退出俱乐部
struct LogonRequestQuit
{
	int friendsGroupID; //俱乐部id
};

//解散俱乐部
struct LogonRequestDismiss
{
	int friendsGroupID; //俱乐部id
};

//改变俱乐部名字
struct LogonRequestChangeName
{
	int friendsGroupID; //俱乐部id
	char newName[24];//俱乐部名字
};

//删除一条消息通知
struct LogonRequestDelNotiy
{
	int id;
	BYTE reply;//是否需要回复，0：不回复，1：回复
};

// 请求删除小红点
struct LogonFriendsGroupRequestDelRedSpot
{
	enum RedSpotType
	{
		REDSPOT_TYPE_NOTIFYLIST = 1,	// 通知列表
		REDSPOT_TYPE_ROOM_MSG = 2,		// 开房消息
		REDSPOT_TYPE_ACCO_MSG = 3,		// 战绩消息
		REDSPOT_TYPE_DESK_MSG = 4,		// 牌桌消息
		REDSPOT_TYPE_VIP_ROOM_MSG = 5,	// VIP房间消息
	};

	int redSpotType;
	int friendsGroupID; //俱乐部id
};

// 请求发布俱乐部公告
struct LogonFriendsGroupRequestSendNotice
{
	int friendsGroupID; //俱乐部id
	char notice[128];   //通知
};

// 战绩统计
struct LogonFriendsGroupRequestQueryAcco
{
	int friendsGroupID; //俱乐部id
	int firstTime;		//起始时间
	int endTime;		//截止时间。截止时间为0：表示至今
};
struct OneSimlpeFriendsGroupAcco
{
	int userID;
	int score;
	long long money;
	int fireCoin;
};
struct LogonFriendsGroupRequestQueryAccoRsp
{
	int friendsGroupID; //俱乐部id
	int peopleCount;    //人数
	OneSimlpeFriendsGroupAcco accos[200];
	LogonFriendsGroupRequestQueryAccoRsp()
	{
		memset(this, 0, sizeof(LogonFriendsGroupRequestQueryAccoRsp));
	}
};

//请求俱乐部房间列表(包括VIP房间)
struct LogonFriendsGroupRequestDeskList
{
	int friendsGroupID; //俱乐部id
	BYTE type;			//0：请求牌桌，非0：请求vip房间列表
};
struct OneFriendsGroupDeskInfo //一条俱乐部牌桌（包括vip房间）
{
	int friendsGroupID; //俱乐部id
	int friendsGroupDeskNumber; //桌子id ,取值范围{1-(MAX_FRIENDSGROUP_DESK_LIST_COUNT+MAX_FRIENDSGROUP_VIP_ROOM_COUNT)}
	int time;          //桌子创建时间
	int roomID;		   //房间ID
	int gameID;		   //游戏ID
	int	roomType;	   //房间类型
	int curPeopleCount;//当前人数
	int allPeopleCount;//总人数
	int playCount;     //游戏局数
	BYTE gameStatus;   //游戏状态，0：未开始，1正在游戏中
	char passwd[20];   //桌子密码
	char gameRules[256];//游戏规则
	int userID[MAX_PLAYER_GRADE]; //玩家列表
	OneFriendsGroupDeskInfo()
	{
		Init();
	}
	void Init()
	{
		memset(this, 0, sizeof(OneFriendsGroupDeskInfo));
	}
};

//请求解散俱乐部一个牌桌（包括vip房间）
struct LogonFriendsGroupRequestDeleteDeskInfo
{
	int friendsGroupID; //俱乐部id
	int friendsGroupDeskNumber; //桌子号
};

// 消耗统计
struct LogonFriendsGroupRequestQueryDeskAcco
{
	int friendsGroupID; //俱乐部id
	int firstTime; //起始时间
	int endTime;   //截止时间。截止时间为0：表示至今
};
struct OneSimlpeFriendsGroupDeskAcco
{
	long long costMoney;			//消耗金币数量
	int costJewels;					//消耗钻石
	long long fireCoinRecharge;		//火币充值
	long long fireCoinExchange;		//火币兑换
	long long moneyPump;			//金币抽水
	long long fireCoinPump;			//火币抽水
	OneSimlpeFriendsGroupDeskAcco()
	{
		memset(this, 0, sizeof(OneSimlpeFriendsGroupDeskAcco));
	}
};
struct LogonFriendsGroupRequestQueryDeskAccoRsp
{
	int friendsGroupID;  //俱乐部id
	OneSimlpeFriendsGroupDeskAcco accos[1];
	LogonFriendsGroupRequestQueryDeskAccoRsp()
	{
		memset(this, 0, sizeof(LogonFriendsGroupRequestQueryDeskAccoRsp));
	}
};

// 请求转让俱乐部
struct LogonFriendsGroupRequestTransfer
{
	int friendsGroupID; //俱乐部id
	int tarUserID;
};

// 请求授权俱乐部成员
struct LogonFriendsGroupRequestAuthMembers
{
	int friendsGroupID; //俱乐部id
	int tarUserID;
	BYTE status; //0普通人员，2管理员
};

// 请求修改俱乐部微信
struct LogonFriendsGroupRequestModifyWechat
{
	int friendsGroupID; //俱乐部id
	char wechat[48];    //微信
};

// 请求修改某个玩家的权限
struct LogonFriendsGroupRequestModifyUserPower
{
	int friendsGroupID; //俱乐部id
	int targetUserID;
	BYTE newPower;		//新的权限
};

// 给玩家充值或者兑换火币
struct LogonFriendsGroupRequestModifyUserFireCoin
{
	int friendsGroupID; //俱乐部id
	int targetUserID;
	int changeFireCoint;//变化值：大于0增加火币，小于0减少火币
};


//////////////通知消息//////////////////////////////
//俱乐部聊天消息变化通知
struct LogonChatMsgChange
{
	int friendsGroupID; //俱乐部id
	BYTE operType; //0：更新，1：删除
	BYTE msgType; // 0开房消息，1：战绩消息
	int id; //消息ID,每个俱乐部消息id唯一
	int roomCount;	   //更新开房数量
	int curPeopleCount;//当前人数
	LogonChatMsgChange()
	{
		memset(this, 0, sizeof(LogonChatMsgChange));
	}
};

//俱乐部名字变化通知
struct LogonFriendsGroupNameChange
{
	int friendsGroupID; //俱乐部id
	char name[24];		//新的俱乐部名字
	LogonFriendsGroupNameChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupNameChange));
	}
};

//俱乐部人变化通知
struct LogonFriendsGroupPeopleChange
{
	int friendsGroupID; //俱乐部id
	BYTE type;  //0更新，1增加，2删除
	OneFriendsGroupUserInfo userInfo;
	LogonFriendsGroupPeopleChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupPeopleChange));
	}
};

//服务器推送的一条俱乐部通知
struct LogonFriendsGroupPushNotify
{
	OneFriendsGroupNotify notify;
};

//服务器推送的一条俱乐部开房消息
struct LogonFriendsGroupPushRoomMsg
{
	OneFriendsGroupRoomMsg notify;

};
//服务器推送的一条俱乐部战绩消息
struct LogonFriendsGroupPushAccoMsg
{
	OneFriendsGroupAccountMsg notify;
};

//服务器推送的一个俱乐部列表消息
struct LogonFriendsGroupPushFGMsg
{
	OneFriendsGroupInfo notify;
};

//服务器主动推送的俱乐部解散消息
struct LogonFriendsGroupPushDismiss
{
	int friendsGroupID; //俱乐部id
	LogonFriendsGroupPushDismiss()
	{
		memset(this, 0, sizeof(LogonFriendsGroupPushDismiss));
	}
};

//服务器推送的小红点数量
struct LogonFriendsGroupPushRedSpot
{
	int notifyListRedSpotCount;		//通知列表数量
	struct MsgRedSpot
	{
		int friendsGroupID;			//俱乐部id
		int deskMsgRedSpotCount;    //牌桌消息数量
		int VIPRoomMsgRedSpotCount;	//VIP房间消息数量
	};
	int friendsGroupCount;
	MsgRedSpot msgRedSpot[20];
	LogonFriendsGroupPushRedSpot()
	{
		memset(this, 0, sizeof(LogonFriendsGroupPushRedSpot));
	}
};

//俱乐部公告变化通知
struct LogonFriendsGroupNoticeChange
{
	int friendsGroupID; //俱乐部id
	char notice[128];	//新的俱乐部公告
	LogonFriendsGroupNoticeChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupNoticeChange));
	}
};

/////牌桌（包括vip房间）
//牌桌增加
struct LogonFriendsGroupAddDeskInfo
{
	OneFriendsGroupDeskInfo desk;
};
//牌桌变化
struct LogonFriendsGroupDeskInfoChange
{
	int friendsGroupID; //俱乐部id
	BYTE operType;		//0：更新，1：删除
	int friendsGroupDeskNumber; //桌子号码，大于MAX_FRIENDSGROUP_DESK_LIST_COUNT是vip房间
	int curPeopleCount;//当前人数
	BYTE gameStatus;   //游戏状态，0：未开始，1正在游戏中
	BYTE updateType;   //0：增加，1：减少，2：gameStatus状态变化
	int userID;		   //增加或减少玩家的ID
	LogonFriendsGroupDeskInfoChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupDeskInfoChange));
	}
};

//俱乐部玩家身份变化通知
struct LogonFriendsGroupUserStatusChange
{
	int friendsGroupID; //俱乐部id
	int peopleCount;

	struct FriendsGroupUserStatusChangeStruct
	{
		int userID;
		BYTE status; //俱乐部的身份，0普通人员 ,1群主，2管理员
		BYTE power;  //俱乐部权限
	};

	FriendsGroupUserStatusChangeStruct user[10];

	LogonFriendsGroupUserStatusChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupUserStatusChange));
	}
};

//俱乐部微信变化通知
struct LogonFriendsGroupWechatChange
{
	int friendsGroupID; //俱乐部id
	char wechat[48];	//新的俱乐部微信
	LogonFriendsGroupWechatChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupWechatChange));
	}
};

//俱乐部玩家的权限变化
struct LogonFriendsGroupUserPowerChange
{
	int userID;
	int friendsGroupID; //俱乐部id
	BYTE newPower;		//新的权限
	LogonFriendsGroupUserPowerChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupUserPowerChange));
	}
};

//俱乐部火币变化通知
struct LogonFriendsGroupUserFireCoinChange
{
	struct LogonFriendsGroupUserFireCoinChangeInfo
	{
		int userID;
		int newFireCoin;
	};
	int friendsGroupID; //俱乐部id
	LogonFriendsGroupUserFireCoinChangeInfo user[MAX_PLAYER_GRADE];
	LogonFriendsGroupUserFireCoinChange()
	{
		memset(this, 0, sizeof(LogonFriendsGroupUserFireCoinChange));
	}
};

//////////////////////////////////////////////////////////////////////////
//比赛场推送（实时赛）
struct LogonPeopleMatchSignUpPeopleChange
{
	int gameID;			//比赛的游戏
	int matchID;		//比赛游戏的编号
	int curSignUpCount;	//当前报名人数
	int peopleCount;	//满多少人开赛
	int allSignUpCount;	//累计报名人数

	LogonPeopleMatchSignUpPeopleChange()
	{
		memset(this, 0, sizeof(LogonPeopleMatchSignUpPeopleChange));
	}
};

struct LogonMatchJoinRoom
{
	int gameID;			//比赛的游戏
	BYTE matchType;		//比赛类型
	int matchID;		//比赛游戏的编号
	int roomID;

	LogonMatchJoinRoom()
	{
		memset(this, 0, sizeof(LogonMatchJoinRoom));
	}
};

//比赛场推送（定时赛）
struct LogonTimeMatchSignUpPeopleChange
{
	int matchID;		//比赛游戏的编号
	int curSignUpCount;	//当前报名人数

	LogonTimeMatchSignUpPeopleChange()
	{
		memset(this, 0, sizeof(LogonTimeMatchSignUpPeopleChange));
	}
};

//// 比赛失败原因
//enum MatchFailReason
//{
//	MATCH_FAIL_REASON_NOT_ENOUGH_PEOPLE = 1,	//人数不够
//	MATCH_FAIL_REASON_PLAYING = 2,				//游戏中
//	MATCH_FAIL_REASON_SYSTEM_ERROR = 3			//系统异常
//};
//通知比赛失败
struct LogonStartMatchFail
{
	int gameID;			//比赛的游戏
	BYTE matchType;		//比赛类型
	int matchID;		//比赛游戏的编号
	BYTE reason;		//比赛失败原因

	LogonStartMatchFail()
	{
		memset(this, 0, sizeof(LogonStartMatchFail));
	}
};

//通知比赛即将开始
struct LogonNotifyStartMatchRightAway
{
	int gameID;			//比赛的游戏
	int matchID;		//比赛游戏的编号
	long long startTime;//开始时间

	LogonNotifyStartMatchRightAway()
	{
		memset(this, 0, sizeof(LogonNotifyStartMatchRightAway));
	}
};

//================================================================================
#pragma pack()

//////////////////////////////////////////////////////////////////////
// 游戏消息定义
//////////////////////////////////////////////////////////////////////
// 登录相关
const unsigned int MSG_MAIN_LOADER_LOGON = 500;

const unsigned int MSG_ASS_LOADER_LOGON = 1;	// 登录
const unsigned int MSG_ADD_LOADER_LOGOUT = 2;	// 登出

// 动作相关
const unsigned int MSG_MAIN_LOADER_ACTION = 501;

const unsigned int MSG_ASS_LOADER_ACTION_SIT = 1;			// 坐下
const unsigned int MSG_ASS_LOADER_ACTION_STAND = 2;			// 站起来
const unsigned int MSG_ASS_LOADER_ACTION_MATCH_SIT = 3;		// 搓桌坐下
const unsigned int MSG_ASS_LOADER_ROBOT_TAKEMONEY = 4;		// 机器人被踢，重新取钱进桌
const unsigned int MSG_ASS_LOADER_GAME_BEGIN = 5;			// 请求游戏开始
const unsigned int MSG_ASS_LOADER_ACTION_COMBINE_SIT = 6;	// 请求组桌坐下
const unsigned int MSG_ASS_LOADER_ACTION_CANCEL_SIT = 7;	// 取消组桌坐下

// 框架类消息
const unsigned int MSG_MAIN_LOADER_FRAME = 502;

const unsigned int MSG_ASS_LOADER_DESK_INFO = 1;	// 请求桌子信息(桌子的基本信息，玩家信息)
const unsigned int MSG_ASS_LOADER_GAME_INFO = 2;	// 游戏信息(具体游戏)
const unsigned int MSG_ASS_LOADER_GAME_STATION = 3; // 游戏状态
const unsigned int MSG_ASS_LOADER_DESK_USERINFO = 4;// 桌子玩家信息
const unsigned int MSG_ASS_LOADER_DESK_ONE_USERINFO = 5;// 请求单个桌子玩家信息

// 游戏消息
const unsigned int MSG_MAIN_LOADER_GAME = 503;

const unsigned int MSG_ASS_LOADER_GAME_AGREE = 1;			// 玩家准备
const unsigned int MSG_ASS_LOADER_GAME_AUTO = 2;			// 托管
const unsigned int MSG_ASS_LOADER_GAME_CANCEL_AUTO = 3;		// 取消托管
const unsigned int MSG_ASS_LOADER_GAME_MAGICEXPRESS = 4;	// 魔法表情


//桌子解散消息
const unsigned int MSG_MAIN_LOADER_DESKDISSMISS = 504;

const unsigned int MSG_ASS_LOADER_REQ_DESKDISSMISS = 1;			// 请求解散桌子
const unsigned int MSG_ASS_LOADER_ANSWER_DESKDISMISS = 2;		// 同意解散桌子

// 通知类消息（桌子）
const unsigned int MSG_MAIN_LOADER_NOTIFY = 505;

const unsigned int MSG_NTF_LOADER_DESK_GAMEBEGIN = 1;			// 通知游戏开始
const unsigned int MSG_NTF_LOADER_DESK_BASEINFO = 2;			// 通知桌子信息
const unsigned int MSG_NTF_LOADER_DESK_DISMISS_OK = 3;			// 通知桌子成功解散
const unsigned int MSG_NTF_LOADER_DESK_DISMISS_FAILED = 4;		// 通知桌子解散失败
const unsigned int MSG_NTF_LOADER_DESK_ALL_USERINFO = 5;		// 发送所有玩家信息(某个桌子)
const unsigned int MSG_NTF_LOADER_DESK_DISMISS_INFO = 6;		// 通知同意桌子解散
const unsigned int MSG_NTF_LOADER_DESK_TALK = 7;				// 通知桌子聊天
const unsigned int MSG_NTF_LOADER_DESK_VOICE = 8;				// 通知桌子语音
const unsigned int MSG_NTF_LOADER_DESK_GAMEFINISH = 9;			// 通知游戏结束
const unsigned int MSG_NTF_LOADER_DESK_GRADE = 10;              // 通知当局游戏战绩
const unsigned int MSG_NTF_LOADER_DESK_MONEY = 11;				// 通知当局游戏金币结算
const unsigned int MSG_NTF_LOADER_DESK_LEFT_WAITAGREE_TIME = 12;// 通知剩余等待准备时间
const unsigned int MSG_NTF_LOADER_DESK_ISAUTO = 13;				// 通知玩家是否托管
const unsigned int MSG_NTF_LOADER_DESK_MAGICEXPRESS = 14;		// 通知魔法表情
const unsigned int MSG_NTF_LOADER_DESK_SITFULL = 15;			// 通知桌子玩家坐满了
const unsigned int MSG_NTF_LOADER_DESK_CANBEGIN = 16;			// 通知房主桌子能开始了
const unsigned int MSG_NTF_LOADER_DESK_STOP_JOIN = 17;			// 游戏已经开始，禁止加入
const unsigned int MSG_NTF_LOADER_RECHARGE = 18;				// 充值倒计时
const unsigned int MSG_NTF_LOADER_DESK_HUNDRED_ALL_USER = 19;	// 百人类游戏，发送玩家简单信息
const unsigned int MSG_NTF_LOADER_DESK_DISMISS_USERID = 20;		// 桌子成功解散，发送相应的座位号玩家id
const unsigned int MSG_NTF_LOADER_ERR_MSG = 21;					// 游戏过程中提示信息
const unsigned int MSG_NTF_LOADER_DESK_JEWELS = 22;				// 通知当局游戏钻石结算
const unsigned int MSG_NTF_LOADER_NO_USER = 23;					// 内存中不存在该玩家，前端直接返回大厅															
const unsigned int MSG_NTF_LOADER_DESK_MATCH_STATUS = 24;		// 通知比赛场桌子状态
const unsigned int MSG_NTF_LOADER_DESK_FINISH_MATCH = 25;		// 通知桌子完成比赛
const unsigned int MSG_NTF_LOADER_MATCH_RANK = 26;				// 通知玩家排名
const unsigned int MSG_NTF_LOADER_DESK_MATCH_GRADE = 27;        // 通知当局比赛场积分变化

// 通知某个玩家信息 
const unsigned int MSG_MAIN_LOADER_NOTIFY_USER = 506;

const unsigned int MSG_NTF_LOADER_DESK_USER_INFO = 1;			// 通知桌子(某个)玩家信息
const unsigned int MSG_NTF_LOADER_DESK_USER_SIT = 2;			// 坐下
const unsigned int MSG_NTF_LOADER_DESK_USER_AGREE = 3;			// 通知已经准备
const unsigned int MSG_NTF_LOADER_DESK_USER_LEFT = 4;			// 通知玩家离开
const unsigned int MSG_NTF_LOADER_DESK_USER_OFFLINE = 5;		// 通知玩家断线
const unsigned int MSG_NTF_LOADER_DESK_USER_BE_KICKOUT = 6;		// 通知玩家被T掉
const unsigned int MSG_NTF_LOADER_DESK_USER_NOFIND_DESK = 7;	// 没找到桌子

// 语音、聊天信息
const unsigned int MSG_MAIN_LOADER_VOICEANDTALK = 507;

const unsigned int MSG_ASS_LOADER_TALK = 1;			// 聊天
const unsigned int MSG_ASS_LOADER_VOICE = 2;		// 语音

// 资源变化相关
const unsigned int MSG_MAIN_LOADER_RESOURCE_CHANGE = 508; // (暂时没有小ID)

// 比赛场相关
const unsigned int MSG_MAIN_LOADER_MATCH = 509;

const unsigned int MSG_ASS_LOADER_MATCH_ALL_DESK_DATA = 1;		// 请求所有桌子状态
const unsigned int MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK = 2;	// 旁观其它桌子
const unsigned int MSG_ASS_LOADER_MATCH_QUIT_WATCH_DESK = 3;	// 退出旁观


// 协议最大主ID
const unsigned int MSG_MAIN_LOADER_MAX = 520;

// 结构体定义
#pragma pack(1)

//游戏房间登陆
struct LoaderRequestLogon
{
	int			roomID;					// 房间ID

	LoaderRequestLogon()
	{
		memset(this, 0, sizeof(LoaderRequestLogon));
	}
};

struct LoaderResponseLogon
{
	int	 deskIdx;
	int	 deskPasswdLen;
	char deskPasswd[20];
	bool isInDesk;
	int iRoomID;
	int playStatus;	// 玩家状态

	LoaderResponseLogon()
	{
		memset(this, 0, sizeof(LoaderResponseLogon));
	}
};

//用户请求坐下（金币场，坐桌）
struct LoaderRequestMatchSit
{
	int		deskIdx;			// 对于房卡场来说，发这个已经没有什么意义了

	LoaderRequestMatchSit()
	{
		memset(this, 0, sizeof(LoaderRequestMatchSit));
	}
};

//用户请求坐下(积分房和金币房)
struct LoaderRequestSit
{
	BYTE deskStation;   // 选择的座位号

	LoaderRequestSit()
	{
		deskStation = INVALID_DESKSTATION;
	}
};

struct LoaderNotifyWaitAgree
{
	int leftWaitAgreeSecs;		// 剩余准备时间
	int cfgWaitAgreeSecs;		// 配置的剩余准备时间

	LoaderNotifyWaitAgree()
	{
		memset(this, 0, sizeof(LoaderNotifyWaitAgree));
	}
};

///游戏信息
struct LoaderGameInfo
{
	BYTE bGameStation;	//游戏状态
	char gameRules[256];

	LoaderGameInfo()
	{
		memset(this, 0, sizeof(LoaderGameInfo));
	}
};

//机器人金币不足被踢，请求取钱
struct _LoaderRobotTakeMoney
{
	int iMoney;
	int iRoomID;
	_LoaderRobotTakeMoney()
	{
		memset(this, 0, sizeof(_LoaderRobotTakeMoney));
	}
};

//回复机器人
struct _LoaderRobotTakeMoneyRes
{
	BYTE byCode; //0成功
	int iMoney;
	_LoaderRobotTakeMoneyRes()
	{
		memset(this, 0, sizeof(_LoaderRobotTakeMoneyRes));
	}
};

//============================================================================================
// 玩家简单信息
struct UserSimpleInfo
{
	int		userID;
	char	name[64];
	char	headURL[256];
	bool	isOnline;
	BYTE	deskStation;
	int		playStatus;
	int		score;
	int		iDeskIndex;
	long long money;
	char	longitude[12];
	char	latitude[12];
	char	address[64];
	char	ip[24];
	BYTE	sex;
	bool	isAuto;
	int		jewels;
	char	motto[128];// 个性签名

	UserSimpleInfo()
	{
		Init();
	}

	void Init()
	{
		memset(this, 0, sizeof(UserSimpleInfo));
		deskStation = INVALID_DESKSTATION;
	}
};

// 通知房间信息(房间基本信息)
struct LoaderNotifyDeskInfo
{
	int		deskIdx;
	int		deskPasswdLen;
	char	deskPasswd[20];
	int		masterID;
	char	masterName[64];
	int		runGameCount;
	int		totalGameCount;
	bool	isDismissStatus;		// 是否处于解散状态

	LoaderNotifyDeskInfo()
	{
		memset(this, 0, sizeof(LoaderNotifyDeskInfo));
	}
};

// 通知桌子玩家信息
struct LoaderNotifyDeskUserInfo
{
	int		userCount;
	UserSimpleInfo userInfo[1];

	LoaderNotifyDeskUserInfo()
	{
		memset(this, 0, sizeof(LoaderNotifyDeskUserInfo));
	}
};

struct LoaderNotifyAgree
{
	BYTE deskStation;
	int userStatus;

	LoaderNotifyAgree()
	{
		memset(this, 0, sizeof(LoaderNotifyAgree));
	}
};

struct DeskDismissType
{
	BYTE deskStation;
	int dismissType;

	DeskDismissType()
	{
		deskStation = INVALID_DESKSTATION;
		dismissType = 0;
	}
};

// 通知解散
struct LoaderNotifyDismiss
{
	BYTE	deskStation;			// 申请解散者
	int		leftDismissTime;		// 请求解散的时间
	int		cfgWaitDismissTime;		// 等待解散时间
	int		deskUserCount;			// 房间玩家数量

	DeskDismissType dismissType[1];

	LoaderNotifyDismiss()
	{
		memset(this, 0, sizeof(LoaderNotifyDismiss));
	}
};

// 通知是否托管
struct LoaderNotifyIsAuto
{
	int userID;
	bool isAuto;

	LoaderNotifyIsAuto()
	{
		memset(this, 0, sizeof(LoaderNotifyIsAuto));
	}
};

struct LoaderRequestAnswerDismiss
{
	bool isAgree;

	LoaderRequestAnswerDismiss()
	{
		isAgree = false;
	}
};

// 请求聊天
struct LoaderRequestTalk
{
	int sizeCount;		//字节数量
	char words[1024];

	LoaderRequestTalk()
	{
		memset(this, 0, sizeof(LoaderRequestTalk));
	}
};

// 通知聊天
struct LoaderNotifyTalk
{
	int userID;
	int sizeCount;		//字节数量
	char words[1024];

	LoaderNotifyTalk()
	{
		memset(this, 0, sizeof(LoaderNotifyTalk));
	}
};

// 请求语音
struct LoaderRequestVoice
{
	int voiceID;

	LoaderRequestVoice()
	{
		voiceID = 0;
	}
};

struct LoaderNotifyVoice
{
	int userID;
	int voiceID;

	LoaderNotifyVoice()
	{
		memset(this, 0, sizeof(LoaderNotifyVoice));
	}
};

struct LoaderNotifyUserLeft
{
	int reason;
	UserSimpleInfo userInfo;

	LoaderNotifyUserLeft()
	{
		reason = 0;
	}
};

struct LoaderNotifyDismissSuccess
{
	bool isDismissMidway;			// 是否中途解散

	LoaderNotifyDismissSuccess()
	{
		memset(this, 0, sizeof(LoaderNotifyDismissSuccess));
	}
};

struct LoaderNotifyDismissSuccessData
{
	int	userID[MAX_PLAYER_GRADE];   // 每个座位号玩家id
	char name[MAX_PLAYER_GRADE][64];
	char headURL[MAX_PLAYER_GRADE][256];

	LoaderNotifyDismissSuccessData()
	{
		memset(this, 0, sizeof(LoaderNotifyDismissSuccessData));
	}
};

struct LoaderNotifyUserGrade
{
	int grade[MAX_PLAYER_GRADE];               //玩家战绩

	LoaderNotifyUserGrade()
	{
		memset(grade, 0, sizeof(grade));
	}
};

struct LoaderRequestMagicExpress
{
	int userID;
	int magicType;

	LoaderRequestMagicExpress()
	{
		userID = 0;
		magicType = 0;
	}
};

struct LoaderNotifyMagicExpress
{
	int srcUserID;
	int targetUserID;
	int magicType;

	LoaderNotifyMagicExpress()
	{
		memset(this, 0, sizeof(LoaderNotifyMagicExpress));
	}
};

struct LoaderNotifyResourceChange
{
	int userID;
	int resourceType;
	long long value;			// 总量
	long long changeValue;		// 变化量

	LoaderNotifyResourceChange()
	{
		memset(this, 0, sizeof(LoaderNotifyResourceChange));
	}
};

// 玩家简单信息
struct HundredGameUserSimpleInfo
{
	int  userID;
	bool isOnline;
	BYTE deskStation;
	int  userStatus;
	long long money;
	bool isAuto;

	HundredGameUserSimpleInfo()
	{
		Init();
	}

	void Init()
	{
		memset(this, 0, sizeof(HundredGameUserSimpleInfo));
		deskStation = INVALID_DESKSTATION;
	}
};

struct LoaderRequestOneUserSimpleInfo
{
	BYTE	deskStation;

	LoaderRequestOneUserSimpleInfo()
	{
		memset(this, 0, sizeof(LoaderRequestOneUserSimpleInfo));
	}
};

struct LoaderNotifyGameFinish
{
	int rechargeUserID[MAX_PLAYER_GRADE]; //金币不足，需要充值的玩家id

	LoaderNotifyGameFinish()
	{
		memset(rechargeUserID, 0, sizeof(rechargeUserID));
	}
};

struct LoaderNotifyRecharge
{
	int leftSecs;					// 剩余时间
	long long needRechargeMoney;	// 需要冲的钱

	LoaderNotifyRecharge()
	{
		leftSecs = 0;
		needRechargeMoney = 0;
	}
};

struct LoaderNotifyERRMsg
{
	int msgType;
	int sizeCount;
	char notify[1024];

	LoaderNotifyERRMsg()
	{
		memset(this, 0, sizeof(LoaderNotifyERRMsg));
	}
};

//////////////////////////////////////////////////////////////////////////
// 比赛场相关协议
struct LoaderNotifyDeskMatchStatus
{
	long long llPartOfMatchID;	//桌子属于的比赛id（小）
	int	iCurMatchRound;			//比赛进行的轮数
	int	iMaxMatchRound;			//最大比赛轮数
	BYTE status;				//0：比赛中，1：本桌比赛已经结束，2：本桌马上要开始比赛，remainTime是剩余时间
	int remainTime;				//剩余开始时间

	LoaderNotifyDeskMatchStatus()
	{
		memset(this, 0, sizeof(LoaderNotifyDeskMatchStatus));
	}
};

struct LoaderNotifyDeskFinishMatch
{
	int deskIdx; //桌子编号

	LoaderNotifyDeskFinishMatch()
	{
		memset(this, 0, sizeof(LoaderNotifyDeskFinishMatch));
	}
};

struct LoaderNotifyMatchRank
{
	struct LoaderNotifyMatchRankUser
	{
		int userID;
		int rank;

		LoaderNotifyMatchRankUser()
		{
			memset(this, 0, sizeof(LoaderNotifyMatchRankUser));
		}
	};

	int gameID;
	int gameMatchID;
	int	iCurMatchRound;		//比赛进行的轮数
	int	iMaxMatchRound;		//最大比赛轮数
	BYTE type;				//0：淘汰，1：晋级，2：最终决赛
	int rankMatch;			//排名

	int peopleCount;
	LoaderNotifyMatchRankUser user[MAX_MATCH_PEOPLE_COUNT];  //协议根据peopleCount数量解析

	LoaderNotifyMatchRank()
	{
		memset(this, 0, sizeof(LoaderNotifyMatchRank));
	}
};

//请求比赛场所有桌子战况
struct LoaderRequestMatchAllDeskDataInfo
{
	long long llPartOfMatchID;

	LoaderRequestMatchAllDeskDataInfo()
	{
		memset(this, 0, sizeof(LoaderRequestMatchAllDeskDataInfo));
	}
};

struct LoaderResponseMatchAllDeskDataInfo
{
	struct DeskInfo
	{
		int deskIdx;		//桌子编号
		BYTE status;		//0：比赛中，1：本桌比赛已经结束，2：本桌马上要开始比赛
	};

	int iCurPeopleCount;
	int iMaxPeopleCount;
	int	iCurMatchRound;		//比赛进行的轮数
	int	iMaxMatchRound;		//最大比赛轮数
	int deskCount;
	DeskInfo desk[256];		//最高256张桌子同时比赛

	LoaderResponseMatchAllDeskDataInfo()
	{
		memset(this, 0, sizeof(LoaderResponseMatchAllDeskDataInfo));
	}
};

// 旁观其它桌子
struct LoaderRequestMatchEnterWatch
{
	long long llPartOfMatchID;	//桌子属于的比赛id（小）
	int deskIdx;

	LoaderRequestMatchEnterWatch()
	{
		memset(this, 0, sizeof(LoaderRequestMatchEnterWatch));
	}
};
struct LoaderResponseMatchEnterWatch
{
	BYTE result;		//0：成功，1：空闲状态不能旁观，2：已经正在旁观，3：本桌游戏未结束，4：异常错误
	int peopleCount;
	UserSimpleInfo user[MAX_PLAYER_GRADE];

	LoaderResponseMatchEnterWatch()
	{
		memset(this, 0, sizeof(LoaderResponseMatchEnterWatch));
	}
};

// 退出旁观
struct LoaderMatchQuitWatch
{
	BYTE result;		//结果，0：默认  1：主动退出   2：游戏结束退出

	LoaderMatchQuitWatch()
	{
		memset(this, 0, sizeof(LoaderMatchQuitWatch));
	}
};

struct LoaderNotifyUserMatchRoomGrade
{
	int changeGrade[MAX_PLAYER_GRADE];	//变化值
	int grade[MAX_PLAYER_GRADE];		//变化之后的比赛积分

	LoaderNotifyUserMatchRoomGrade()
	{
		memset(this, 0, sizeof(LoaderNotifyUserMatchRoomGrade));
	}
};

#pragma pack()