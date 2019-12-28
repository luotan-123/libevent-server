#pragma once

#include "BaseLogonServer.h"
#include "LogonUserManage.h"
#include "LogonGServerManage.h"

// 定时器ID
enum LogonServerTimerID
{
	LOGON_TIMER_BEGIN = 0,
	LOGON_TIMER_CHECK_HEARTBEAT,				// 心跳定时器
	LOGON_TIMER_CHECK_REDIS_CONNECTION,			// redis连接性
	LOGON_TIMER_ROUTINE_CHECK_UNBINDID_SOCKET,	// 定期检查没有绑定玩家ID的无效连接
	LOGON_TIMER_ROUTINE_SAVE_REDIS,				// 定期存储redis数据
	LOGON_TIMER_SAVE_SOCKET_COUNT,				// 定期保存网关socket数量
	LOGON_TIMER_NORMAL,							// 通用定时器(s)
	LOGON_TIMER_END,
};

const int CHECK_REDIS_SAVE_DB = 61;					// 定期存储redis数据(s)
const int CHECK_REDIS_CONNECTION_SECS = 307;		// 定期检查redis连接(s)
const int ROUTINE_CHECK_UNBINDID_SOCKET = 67;		// 定期检查未登录的连接(s)
const int CHECK_SAVE_SOCKET_COUNT = 17;				// 定期保存网关socket数量(s)
const int NORMAL_TIMER_SECS = 3;					// 通用定时器(s)

// 登陆服tcpsocket
struct LogonServerSocket
{
	BYTE type;			// 两种类型的socket 1：玩家的socket，2：游戏服的socket
	int identityID;		// 玩家id或者roomID

	LogonServerSocket()
	{
		type = LOGON_SERVER_SOCKET_TYPE_NO;
		identityID = 0;
	}

	LogonServerSocket(BYTE type, int identityID)
	{
		this->type = type;
		this->identityID = identityID;
	}

	void Init()
	{
		type = LOGON_SERVER_SOCKET_TYPE_NO;
		identityID = 0;
	}
};

class CGameLogonManage : public CBaseLogonServer
{
public:
	UINT						m_uMaxPeople;			//支持最大人数（包括gserver数量） tcp
	UINT						m_uMaxWebPeople;		//支持最大人数 websocket

private:
	CLogonUserManage*			m_pUserManage;			// 玩家管理器
	CLogonGServerManage*		m_pGServerManage;		// 游戏服管理器
	std::vector<LogonServerSocket> m_socketInfoMap;		// socket索引和identityID的映射表 TCP
	std::vector<LogonServerSocket> m_webSocketInfoMap;	// socket索引和userID的映射表 websocket
	std::vector<int>			m_buyRoomVec;
	time_t						m_lastNormalTimerTime;
	std::vector<UINT>			m_socketIndexVec;		// socket索引，遍历在线tcpsocket需要

private:
	std::set<UINT>				m_socketMatch;			// 在比赛场相关页面的玩家

public:
	CGameLogonManage();
	virtual ~CGameLogonManage();

public:
	//数据管理模块启动
	virtual bool OnStart();
	//数据管理模块关闭
	virtual bool OnStop();

	//服务扩展接口函数
private:
	//获取信息函数
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);
	//SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, BYTE socketType, UINT uIndex, void* pBufferevent);
	//SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime, BYTE socketType);
	//异步线程处理结果
	virtual bool OnAsynThreadResult(AsynThreadResultLine * pResultData, void * pData, UINT uSize);
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);

	// 登陆通知
private:
	void NotifyUserInfo(const UserData &userData);
private:
	// 玩家注册相关
	bool OnHandleUserRegister(unsigned int assistID, void* pData, int size, BYTE socketType, unsigned int socketIdx, void* pBufferevent);

	// 玩家登陆相关
	bool OnHandleUserLogonMessage(int assistID, void* pData, int size, BYTE socketType, unsigned int socketIdx, void* pBufferevent);

	bool OnHanleUserLogon(void* pData, int size, BYTE socketType, unsigned int socketIdx, void* pBufferevent);

	// 桌子相关
	bool OnHandleGameDeskMessage(int assistID, void* pData, int size, BYTE socketType, unsigned int socketIdx, void* pBufferevent);

	bool OnHandleUserBuyDesk(int userID, void* pData, int size);
	bool OnHandleUserEnterDesk(int userID, void* pData, int size);

	// 其他相关
	bool OnHandleOtherMessage(int assistID, void* pData, int size, BYTE socketType, unsigned int socketIdx, void* pBufferevent);

	//刷新个人信息
	bool OnHandleUserInfoFlush(int userID, void* pData, int size);
	//机器人取钱
	bool OnHandleRobotTakeMoney(int userID, void* pData, int size);
	// 请求玩家信息
	bool OnHandleReqUserInfo(int userID, void* pData, int size);
	// 请求进入比赛场页面
	bool OnHandleJoinMatchScene(UINT index);
	// 请求退出比赛场页面
	bool OnHandleExitMatchScene(UINT index);

	//////////////////////////////游戏服相关////////////////////////////////////////////
	// 认证
	bool OnHandleGServerVerifyMessage(void* pData, int size, unsigned int socketIdx, void* pBufferevent);
	// 前端 ----> 游戏服
	bool OnHandleGServerToGameMessage(int userID, NetMessageHead * pNetHead, void * pData, UINT uSize, BYTE socketType, UINT uIndex, void* pBufferevent);
	// 游戏服 ----> 前端
	bool OnHandleGServerToUserMessage(int roomID, NetMessageHead * pNetHead, void * pData, UINT uSize, BYTE socketType, UINT uIndex, void* pBufferevent);

public:
	// 通过socketIdx获取socket信息
	LogonServerSocket GetIdentityIDBySocketIdx(int socketIdx, BYTE socketType);
	// 删除socketIdx索引
	void DelSocketIdx(int socketIdx, BYTE socketType);
	// 给玩家发送数据
	bool SendData(int userID, void* pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode, unsigned int uIdentification = 0);
	// 通过索引发送数据
	bool SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, int encrypted, BYTE socketType, unsigned int uIdentification = 0);
	// 广播全服数据，toGServer=true广播到游戏服务器
	bool SendDataBatch(void* pData, UINT uSize, UINT uMainID, UINT bAssistantID, UINT uHandleCode, bool toGServer);
	// 通知资源变化, value为总值，不是变化值
	void NotifyResourceChange(int userID, int resourceType, long long value, int reason, long long changeValue);

public:
	// 向中心服务器发送消息
	bool SendMessageToCenterServer(UINT msgID, void* pData, UINT size, int userID = 0, UINT mainID = 0, UINT assistID = 0, UINT handleCode = 0);

public:
	// 判断roomID的服务器是否存在
	bool IsRoomIDServerExists(int roomID);
	// 删除程序生成的无效文件
	void DeleteExpireFile();
	// 玩家是否在线
	bool IsUserOnline(int userID);
	// 自动开房
	bool AutoCreateRoom(const SaveRedisFriendsGroupDesk &deskInfo);
	// 判断此ip是否可以注册
	bool IsIpRegister(const OtherConfig &otherConfig, const char * ip);
	// 判断某个玩家是否是游客
	bool IsVisitorUser(int userID);
	// 获取随机头像
	std::string GetRandHeadURLBySex(BYTE sex);
private:
	// 初始化需要定期检查的事件
	void InitRounteCheckEvent();
	// 检查redis连接性
	void CheckRedisConnection();
	// 检查心跳
	void CheckHeartBeat();
	// 定时存储redis数据到DB updateAll=是否全部更新
	void RountineSaveRedisDataToDB(bool updateAll);
	// 定期检查没有绑定玩家ID的链接
	void RoutineCheckUnbindIDSocket();
	// 通用定时器
	void OnNormalTimer();
	// 跨天
	void OnServerCrossDay();
	// 凌晨12点
	void OnServerCrossDay12Hour();
	// 跨周
	void OnServerCrossWeek();
	// 清除胜局榜
	void CleanAllUserWinCount();
	// 清理金币钻石排行榜
	void CleanAllUserMoneyJewelsRank();
	// 处理玩家登录
	void OnUserLogon(const UserData &userData);
	// 处理玩家登出
	void OnUserLogout(int userID);
	// 处理玩家注册
	void OnUserRegister(const UserData &userData);
	// 玩家跨天
	void OnUserCrossDay(int userID, int time);
	// 清理数据库过期数据
	void ClearDataBase();
	// 生成新表记录账单数据
	void AutoCreateNewTable(bool start);

	// 中心服消息相关
private:
	// 处理中心服务器的消息
	virtual bool OnCenterServerMessage(UINT msgID, NetMessageHead * pNetHead, void* pData, UINT size, int userID);
	// 服务器id重复处理
	bool OnCenterRepeatIDMessage(void* pData, int size);
	// 分布式系统信息
	bool OnCenterDistributedSystemInfoMessage(void* pData, int size);
	// 踢掉本服旧玩家（不同设备同时登陆使用）
	bool OnCenterKickOldUserMessage(void* pData, UINT size);
	// 踢人掉线消息(封号使用)
	bool OnCenterKickUserMessage(void* pData, int size, int userID);
	// 发布公告
	bool OnCenterNoticeMessage(void* pData, int size);
	// 全服邮件通知
	bool OnCenterAllUserMailMessage(void* pData, int size);
	// 关服处理
	bool OnCenterCloseServerMessage(void* pData, int size);
	// 开服处理
	bool OnCenterOpenServerMessage(void* pData, int size);
	// 玩家发送喇叭
	bool OnCenterSendHornMessage(void* pData, UINT size);
	// 向某个玩家推送消息
	bool OnCenterNotifyUserMessage(NetMessageHead * pNetHead, void* pData, int size, int userID);
	// 中心服自动开房
	bool OnCenterAutoCreateRoomMessage(void* pData, UINT size);
	// 手机注册
	bool OnCenterPhoneInfoMessage(void* pData, int size);
	// 奖励全服通知
	bool OnCenterActivityRewardsMessage(void* pData, int size);
	// 有人报名或者退出比赛（实时赛）
	bool OnCenterSignUpMatchMessage(void* pData, UINT size);
	// 比赛场通知玩家进入比赛
	bool OnCenterLoaderStartMatchMessage(void* pData, UINT size);
	// 有人报名或者退出比赛(定时赛)
	bool OnCenterTimeMatchPeopleChangeMessage(void* pData, UINT size);

	//向PHP请求接口相关
private:
	// 给php发送消息接口
	void SendHTTPMessage(int userID, const std::string &url, BYTE postType);
	// 玩家注册发送http请求
	bool SendHTTPUserRegisterMessage(int userID);
	// 玩家登陆和登出通知 type类型：0登陆，1登出
	bool SendHTTPUserLogonLogout(int userID, BYTE type);
};
