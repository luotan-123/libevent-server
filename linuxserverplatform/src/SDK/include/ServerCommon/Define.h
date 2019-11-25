#pragma once

#include "Function.h"

/***********************************************************************全局定义********************************************************************************/

//////////////////////////////////////////////////////////////////////////
#define TMLcopyright  "开发:深圳市科技有限公司"	///版权

//////////////////////////////////////////////////////////////////////////
#define SAFECHECK_MESSAGE(pMessage,MessageType,pData,iSize) \
if(sizeof(MessageType) != iSize)\
	return false;\
MessageType *pMessage = (MessageType *)pData;\
if(!pMessage)\
	return false;

//////////////////////////////////////////////////////////
// 游戏组件相关
#define INVALID_DESKIDX						(-1)			// 无效的桌号ID
#define INVALID_DESKSTATION					0xFF			// 无效的座位号
#define MAX_PLAYER_GRADE					9				// 战绩显示的最大人数
#define MAX_ROOM_HAVE_DESK_COUNT			100000			// 每个房间最多桌子数量
#define MAX_PEOPLE							180				// 最大游戏人数
#define MAX_BUY_DESK_JSON_LEN				256				// 购买桌子json最大长度

#define MAKE_DESKMIXID(roomID, deskIdx)		(roomID * MAX_ROOM_HAVE_DESK_COUNT + deskIdx)	// 计算redis中，桌子索引

#define PRIVATE_DESK_TIMEOUT_SECS			(10 * 60)		// 房卡场桌子超时时间(s) TODOs

#define MAX_ROOM_GRADE_COUNT				10000000		// 每个roomID最多战绩数量

#define GRADE_SIMPLE_EXPIRE_TIME			(7*24*60*60)	// 大结算战绩过期时间
#define GRADE_EXPIRE_TIME					(GRADE_SIMPLE_EXPIRE_TIME + (1*24*60*60))		// 单局战绩失效时间

#define SAVE_JSON_PATH						"/tmp/web-json/"	// 保存json的目录
#define SAVE_COREFILE_PATH					"/tmp/linuxserverplatform-corefile/"			// 保存corefile的目录

///开发库版本
#define DEV_LIB_VER							5									///开发库版本

///支持类型定义
#define SUP_NORMAL_GAME						0x01								///普通游戏
#define SUP_MATCH_GAME						0x02								///比赛游戏
#define SUP_MONEY_GAME						0x04								///金币游戏

////////////////////////////////////////////////////////////
// 俱乐部相关
#define GROUP_NOTIY_MSG_EXPIRE_TIME			(7*24*60*60)	// 俱乐部通知消息过期时间
#define MAX_FRIENDSGROUP_DESK_LIST_COUNT	20				// 最大牌桌数量
#define MAX_FRIENDSGROUP_VIP_ROOM_COUNT		20				// 最大VIP房间数量

//////////////////////////////////////////////////////////////////////////
// 邮件系统
#define EMAIL_SIMPLE_EXPIRE_TIME			(10*24*60*60)	// 每个用户的邮件id过期时间
#define EMAIL_EXPIRE_TIME					(11*24*60*60)	// 邮件详情过期时间

//////////////////////////////////////////////////////////////////////////
// 排行榜
#define MAX_RANK_COUNT						10				// 排行榜最多显示数量

//////////////////////////////////////////////////////////////////////////
// 比赛场相关
#define MAX_MATCH_ROUND							10						// 比赛最大进行轮数
#define MAX_GAME_MATCH_ID						10000000				// 最多举行的比赛数量
#define MAX_MATCH_PEOPLE_COUNT					512						// 最大参赛人数
#define MIN_ROBOT_USERID						117700					// 机器人最小id
#define MAX_ROBOT_USERID						117999					// 机器人最大id
#define MATCH_START_NOTIFY_TIME					180						// 开赛通知时间

// 比赛类型
enum MatchType
{
	MATCH_TYPE_NORMAL = 0,		//0默认值，没有报名比赛
	MATCH_TYPE_PEOPLE = 1,		//实时赛
	MATCH_TYPE_TIME = 2,		//定时赛
};

// 玩家比赛状态
enum UserMatchStatus
{
	USER_MATCH_STATUS_NORMAL = 0,		//没有报名，或者比赛结束
	USER_MATCH_STATUS_SIGN_UP = 1,		//已经报名，游戏未开始
	USER_MATCH_STATUS_AFTER_BEGIN = 2,	//已经报名，比赛即将开始
	USER_MATCH_STATUS_PLAYING = 3,		//比赛进行中
};

// 玩家比赛桌子状态
enum DeskMatchStatus
{
	DESK_MATCH_STATUS_NORMAL = 0,	//等待开赛，或者晋级
	DESK_MATCH_STATUS_FAIL = 1,		//已经被淘汰
};

// 比赛失败原因
enum MatchFailReason
{
	MATCH_FAIL_REASON_NOT_ENOUGH_PEOPLE = 1,	//人数不够
	MATCH_FAIL_REASON_PLAYING = 2,				//游戏中
	MATCH_FAIL_REASON_SYSTEM_ERROR = 3			//系统异常
};

//比赛状态  0：报名中，1：比赛中，2：比赛结束
enum MatchStatus
{
	MATCH_STATUS_SIGN_UP = 0,	//报名中
	MATCH_STATUS_PLAYING = 1,	//比赛中
	MATCH_STATUS_GAME_OVER = 2,	//比赛结束
};

//////////////////////////////////////////////////////////////////////////

/////////////////////////////////枚举模块/////////////////////////////////////////

// 踢掉(请离)原因
enum ReasonKickout
{
	REASON_KICKOUT_DEFAULT = 0,		// 默认
	REASON_KICKOUT_STAND,			// 站起来
	REASON_KICKOUT_STAND_MINLIMIT,	// 金币小于最低限制
	REASON_KICKOUT_STAND_MAXLIMIT,  // 金币大于最高限制
	REASON_KICKOUT_NOTAGREE,		// 长时间未准备
	REASON_KICKOUT_STAND_FIRECOIN_MINLIMIT,		 // 火币小于最低限制
	REASON_KICKOUT_STAND_FIRECOIN_MAXLIMIT,		 // 火币大于最高限制
	REASON_KICKOUT_LONG_TIME_NOOPERATION,		//长时间未操作
};

// 资源类型
enum ResourceType
{
	RESOURCE_TYPE_NONE = 0,
	RESOURCE_TYPE_MONEY,		// 金币
	RESOURCE_TYPE_JEWEL,		// 房卡
	RESOURCE_TYPE_BANKMONEY,	// 银行金币
	RESOURCE_TYPE_FIRECOIN,		// 俱乐部火币
	RESOURCE_TYPE_END,
};

// 登陆注册类型
enum LogonType
{
	LOGON_QUICK,	// 快速登录
	LOGON_NORMAL,	// 普通登录
	LOGON_WEICHAT,	// 微信登录
	LOGON_QQ,		// QQ登录
	LOGON_TEL_PHONE,// 手机登录
	LOGON_TEL_XIANLIAO,// 闲聊登录
	LOGON_VISITOR,	// 游客登录
	LOGON_NR		// 最大下标
};

// 操作状态
enum LogonStatus
{
	LOGON_REGISTER,  //注册
	LOGON_LOGIN,     //登录
};

// 支付方式
enum PayType
{
	PAY_TYPE_NORMAL = 1,	// 普通支付
	PAY_TYPE_AA = 2,		// AA支付
};

// 服务费类型
enum RoomTipType
{
	ROOM_TIP_TYPE_NO = 0,		//不收小费
	ROOM_TIP_TYPE_ALL_WIN = 1,	//收取所有的赢家
	ROOM_TIP_TYPE_MAX_WIN = 2,	//收取最大的大赢家
};

// redis分区类型
enum RedisDataType
{
	REDIS_DATA_TYPE_GAME = 1,	// 保存游戏数据
	REDIS_DATA_TYPE_PHP = 2,	// 保存PHP数据
};

// redis中存取的配置类型
enum SystemConfigType
{
	SYSTEM_CONFIG_TYPE_OTHER = 1,	// 其它配置
	SYSTEM_CONFIG_TYPE_BANK = 2,	// 银行
	SYSTEM_CONFIG_TYPE_SENDGIFT = 3,// 转增
	SYSTEM_CONFIG_TYPE_FG = 4,		// 俱乐部
	SYSTEM_CONFIG_TYPE_FTP = 5,		// ftp
};

// 登陆服socket类型
enum LogonServerSocketType
{
	LOGON_SERVER_SOCKET_TYPE_NO = 0,			//无类型
	LOGON_SERVER_SOCKET_TYPE_USER = 1,			//玩家
	LOGON_SERVER_SOCKET_TYPE_GAME_SERVER = 2,	//游戏服
};

//////////////////////////////////////////////////////////////////////////