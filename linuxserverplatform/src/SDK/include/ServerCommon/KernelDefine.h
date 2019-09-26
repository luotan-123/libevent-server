#pragma once

/***********************************************************************内核定义********************************************************************************/

const int MIN_USER_ACCOUNT_LEN = 4;			// 玩家账号最小长度
const int MAX_USER_ACCOUNT_LEN = 48;		// 玩家账号最大长度

const int MAX_USER_NAME_LEN = 64;			// 玩家昵称最大长度
const int MAX_USER_HEADURL_LEN = 256;		// 玩家头像最大长度

const int USER_MD5_PASSWD_LEN = 32;				// 密码长度(都是MD5)
const int MAX_USER_MAD_PASSWD_SIZE = 48;		// 最大密码缓冲区大小

const int MAX_PRIVATE_DESK_PASSWD_LEN = 20;		// 私有房最大密码长度
const int PRIVATE_DESK_PASSWD_LEN = 6;			// 私有房密码长度

const int MAX_TEMP_SENDBUF_SIZE = 8192;			// 临时发送缓冲区大小
const int MAX_REDIS_COMMAND_SIZE = 2048;		// redis命令缓冲区大小
const int MAX_SQL_STATEMENT_SIZE = 2048;		// sql语句缓冲区大小

const int MAX_IP_ADDR_SIZE = 128;		// 最大IP地址长度
const int MAX_DB_USER_SIZE = 48;		// 最大db_user缓冲区大小
const int MAX_DB_PASSWD_SIZE = 48;		// 最大db_passwd缓冲区大小
const int MAX_DB_NAME_SIZE = 48;		// 最大db_name缓冲区大小

const int MAX_LOG_BUF_SIZE = 2048;		// log缓冲区大小
const int MAX_FILE_NAME_SIZE = 256;		// 最大文件名长度
const int MAX_FUNC_NAME_SIZE = 128;		// 最大函数名长度
const int MAX_LOG_FILE_SIZE = 1024 * 1024 * 32;  //日子文件的最大的长度，超过长度会生成新的文件

const int MAX_BANK_PASSWD_MIN_LEN = 6;	// 银行密码最小长度

//////////////////////////////////////////////////////////////////////////
//数据队列最大长度（包括队列头部），要大于 MAX_TEMP_SENDBUF_SIZE=8192
#define LD_MAX_PART						8320

/////////////////////////////////////////////////////////
//web 相关
#define  WEB_ADDRESS	"headimgqn.szysx8.cn"
const int MIN_MAN_HEADURL_ID = 1001;
const int MAX_MAN_HEADURL_ID = 3287;
const int MIN_WOMAN_HEADURL_ID = 1000001;
const int MAX_WOMAN_HEADURL_ID = 1003803;

/////////////////////////////////////////////////////////
// 心跳、socket相关
#define KEEP_ACTIVE_HEARTBEAT_COUNT				3		// 前端和服务器的心跳
#define BINDID_SOCKET_USERID_TIME				30		// 每个连接socket，绑定玩家时间，单位秒
#define MAX_LOGON_SERVER_COUNT					48		// 最多登陆服数量
#define INVALID_SOCKET							-1		// 无效的socket值

/////////////////////////////////////////////////////////
// 耗时统计
#define MIN_STATISTICS_FUNC_COST_TIME			20000	// 函数耗时最小统计时间(us)

/////////////////////////////////////////////////////////
// 线程处理时间间隔 单位微妙
#define THREAD_ONCE_DATABASE					50000   // 数据处理线程间隔
#define THREAD_ONCE_HANDLE_MSG					20000   // 消息处理线程间隔
#define MAX_TIMER_THRED_NUMS					4		// 定时器最大线程数量

/////////////////////////////////枚举模块/////////////////////////////////////////

// 玩家性别
enum UserSex
{
	USER_DEX_FEMALE = 0,//女性
	USER_SEX_MALE = 1,  //男性
};

// 服务器类型
enum ServiceType
{
	SERVICE_TYPE_BEGIN = 0,
	SERVICE_TYPE_LOGON,		// 大厅
	SERVICE_TYPE_LOADER,	// 游戏
	SERVICE_TYPE_CENTER,	// 中心服
	SERVICE_TYPE_PHP,		// PHP服务器
	SERVICE_TYPE_END,
};

// 线程类型
enum ThreadType
{
	THREAD_TYPE_MAIN = 1,	// 主线程
	THREAD_TYPE_LOGIC = 2,	// 逻辑线程
	THREAD_TYPE_ASYNC = 3,	// 异步线程
	THREAD_TYPE_ACCEPT = 4,	// 连接线程
	THREAD_TYPE_RECV = 5,	// 接收线程
};

// 日志等级
enum LogLevel
{
	LOG_LEVEL_INFO = 0,						//info
	LOG_LEVEL_WARNNING,						//warnning
	LOG_LEVEL_ERROR,						//error
	LOG_LEVEL_INFO_CONSOLE,					//info Console
	LOG_LEVEL_ERROR_CONSOLE,				//error Console
	LOG_LEVEL_ERROR_SYS,					//error system
	LOG_LEVEL_END,
};

//////////////////////////////////////////////////////////////////////////

//时间参数
typedef struct _SYSTEMTIME {
	int wYear;
	int wMonth;
	int wDayOfWeek;
	int wDay;
	int wHour;
	int wMinute;
	int wSecond;
	int wMilliseconds;
	long lMicroseconds;
} SYSTEMTIME, * PSYSTEMTIME, * LPSYSTEMTIME;

//////////////////////////////////////////////////////////////////////////