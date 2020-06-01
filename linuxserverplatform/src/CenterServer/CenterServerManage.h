#pragma once

#include "BaseCenterServer.h"

// 定时器ID
enum CenterServerTimerID
{
	CENTER_TIMER_BEGIN = 0,
	CENTER_TIMER_CHECK_REDIS_CONNECTION,			// redis连接性
	CENTER_TIMER_NORMAL,							// 通用定时器(s)
	CENTER_TIMER_ROUTINE_CHECK_UNBINDID_SOCKET,		// 清理无绑定连接
	CENTER_TIMER_LOAD_MATCH_INFO,					// 加载比赛信息
	CENTER_TIMER_END,
};

const int CHECK_REDIS_CONNECTION_SECS = 307;		// 定期检查redis连接(s)
const int ROUTINE_CHECK_UNBINDID_SOCKET = 59;		// 定期清理连接(s)
const int NORMAL_TIMER_SECS = 3;					// 通用定时器(s)
const int CHECK_REDIS_LOAD_MATCH_INFO = 1200;		// 加载比赛信息(s)

//非定时器时间
const int CONNECT_TIME_SECS = 20;					// 每个连接最多存在时间

// 中心服务器玩家结构
struct CenterUserInfo
{
	int userID;
	UINT socketIdx; //登录服索引
	BYTE isVirtual;
	void* pBufferevent;

	CenterUserInfo()
	{
		userID = 0;
		socketIdx = -1;
		isVirtual = 0;
		pBufferevent = nullptr;
	}
};

struct ServerBaseInfo
{
	int serverType; // 服务器类型   SERVICE_TYPE_LOGON//大厅    SERVICE_TYPE_LOADER//游戏
	int serverID;	// 大厅服是logonID，游戏服是roomID
	ServerBaseInfo()
	{
		serverType = SERVICE_TYPE_BEGIN;
		serverID = 0;
	}
	bool operator<(const ServerBaseInfo &server)const
	{
		if (serverType < server.serverType)
		{
			return true;
		}
		else if (serverType == server.serverType)
		{
			if (serverID < server.serverID)
			{
				return true;
			}
		}
		return false;
	}
};

class CCenterServerManage : public CBaseCenterServer
{
public:
	UINT						m_nPort;				//登陆服务器端口
	UINT						m_uMaxPeople;			//支持最大分布式连接数量

private:
	std::unordered_map<UINT, ServerBaseInfo>		m_socketToServerMap;	// socketIdx到服务器ID的映射(key=socketIdx,value=ServerBaseInfo)
	std::map<ServerBaseInfo, SocketSimpleInfo>		m_serverToSocketMap;	// 服务器到socketIdx的映射  (key=ServerBaseInfo,value=socketIdx)
	std::vector<SocketSimpleInfo>					m_logonGroupSocket;		// 网关服集群(value=socketIdx)
	std::vector<SocketSimpleInfo>					m_workGroupSocket;		// 逻辑服集群(value=socketIdx)
	std::unordered_map<int, CenterUserInfo>			m_centerUserInfoMap;	// 集群系统在线玩家(只统计登录服的)
	std::vector<int>								m_memberUserIDVec;		// 临时保存的俱乐部成员
	time_t											m_lastNormalTimerTime;
	std::map<long long, MatchInfo>					m_timeMatchInfoMap;		// 定时赛信息，根据比赛开始时间排序。tips：所有定时赛比赛开始时间不能相同
	std::vector<UINT>								m_socketIndexVec;		// socket索引

public:
	CCenterServerManage();
	CCenterServerManage(CCenterServerManage&);
	CCenterServerManage & operator = (CCenterServerManage &);
	virtual ~CCenterServerManage();

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
	virtual bool OnSocketRead(NetMessageHead * pNetHead, CenterServerMessageHead * pCenterHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	//SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime, BYTE socketType);
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);

public:///////////////////////////////////发送数据相关///////////////////////////////////////
	// 给玩家(登录服)发送数据
	bool SendData(int userID, UINT msgID, void* pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode);
	// 向服务器发送数据
	bool SendData(ServerBaseInfo * pServer, UINT msgID, void* pData, UINT size, int userID = 0, UINT mainID = 0, UINT assistID = 0, UINT handleCode = 0);
	// 向服务器发送数据
	bool SendData(UINT uScoketIndex, UINT msgID, void* pData, UINT size, UINT handleCode, void* pBufferevent);
	// 通知资源变化
	void NotifyResourceChange(int userID, int resourceType, long long value, long long changeValue, int reason, int reserveData = 0, BYTE isNotifyRoom = 0);
	// 给登录服服务器集群发送当前集群信息
	void SendDistributedSystemInfo(ServiceType type);
	// 比赛即将开始，给所有报名玩家发送消息通知
	void SendNotifyMatchStart(const MatchInfo &matchInfo);
private: /////////////////////////内部系统////////////////////////
	// 初始化需要定期检查的事件
	void InitRounteCheckEvent();
	// 检查redis连接性
	void CheckRedisConnection();
	// 通用定时器
	void OnNormalTimer();
	// 清理连接
	void RoutineCheckUnbindIDSocket();
	// 跨天
	void OnServerCrossDay();
	// 跨周
	void OnServerCrossWeek();
	// 玩家是否在线
	bool IsUserOnline(int userID);
	// 检查定时赛
	void CheckTimeMatch(const time_t &currTime);

private:
	////////////////////////////////处理PHP服消息//////////////////////////////////////////
	bool OnHandlePHPMessage(NetMessageHead * pNetHead, CenterServerMessageHead * pCenterHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	// 平台公告相关
	bool OnHandlePHPNoticeMessage(UINT socketIdx, void* pData, int size);
	// 全服邮件通知
	bool OnHandlePHPAllUserMailMessage(UINT socketIdx, void* pData, int size);
	// 关服
	bool OnHandlePHPCloseServerMessage(UINT socketIdx, void*pData, int size);
	// 开服
	bool OnHandlePHPOpenServerMessage(UINT socketIdx, void*pData, int size);
	// 发送喇叭
	bool OnHandlePHPHornMessage(UINT socketIdx, void*pData, int size);
	// 解散桌子
	bool OnHandlePHPDissmissDeskMessage(UINT socketIdx, UINT msgID, void*pData, int size);
	// 通知某个人资源变化
	bool OnHandlePHPNotifyResChangeMessage(UINT socketIdx, int userID, void*pData, int size);
	// 向某个玩家推送消息
	bool OnHandlePHPNotifyOneUserMessage(UINT socketIdx, int userID, void*pData, int size, unsigned int mainID, unsigned int assistID);
	// 向俱乐部玩家推送消息
	bool OnHandlePHPNotifyFGMessage(UINT socketIdx, int friendsGroupID, void*pData, int size, unsigned int mainID, unsigned int assistID);
	// 设置玩家身份
	bool OnHandlePHPSetUserMessage(UINT socketIdx, int userID, void* pData, int size);
	// 通知小红点
	bool OnHandlePHPNotifyUserRedspotMessage(UINT socketIdx, int userID, void* pData, int size);
	// 通知俱乐部小红点
	bool OnHandlePHPNotifyUserRedFGspotMessage(UINT socketIdx, int friendsGroupID, void* pData, int size);
	// 加载游戏配置
	bool OnHandlePHPReloadGameConfigMessage(UINT socketIdx, void* pData, int size);
	// 手机注册验证成功，注册账号
	bool OnHandlePHPPhoneInfoMessage(UINT socketIdx, void* pData, int size);
	// 有人报名或者退出比赛（实时赛）
	bool OnHandlePHPSignUpMatchMessage(UINT socketIdx, void*pData, int size);
	// 请求开始比赛
	bool OnHandlePHPReqStartMatchMessage(UINT socketIdx, void*pData, int size);
	// 创建、修改、删除，一个定时赛
	bool OnHandlePHPReqModifyTimeMatchMessage(UINT socketIdx, void*pData, int size);
	// 玩家报名或者退出报名（定时赛）
	bool OnHandlePHPTimeMatchPeopleChangeMessage(UINT socketIdx, void*pData, int size);

	////////////////////////////////处理通用(网关和游戏，不包含PHP)消息//////////////////////////////////////////
	bool OnHandleCommonMessage(NetMessageHead * pNetHead, CenterServerMessageHead * pCenterHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	// 为每个服务器绑定socketIdx
	bool OnHandleCommonServerVerifyMessage(void* pData, UINT size, ULONG uAccessIP, UINT uIndex, void* pBufferevent);


	////////////////////////////////处理大厅服消息//////////////////////////////////////////
	bool OnHandleLogonMessage(NetMessageHead * pNetHead, CenterServerMessageHead * pCenterHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	// 登陆服资源变化
	bool OnHandleLogonResChangeMessage(int userID, void* pData, UINT size);
	// 玩家上下线
	bool OnHandleLogonUserStatusMessage(void* pData, UINT size, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	// 给其它登录服发送数据，踢掉旧服务器玩家
	bool OnHandleLogonRepeatUserMessage(CenterServerMessageHead * pCenterHead, void* pData, UINT size, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	// 转发消息给某个人
	bool OnHandleLogonRelayUserMessage(NetMessageHead * pNetHead, void* pData, UINT size, int userID);
	// 转发消息给俱乐部
	bool OnHandleLogonRelayFGMessage(NetMessageHead * pNetHead, void* pData, UINT size, int friendsGroupID);
	// 请求给俱乐部所有玩家发送小红点
	bool OnHandleLogonReqFGRedSpotMessage(void* pData, UINT size, int friendsGroupID);
	// 请求解散房间
	bool OnHandleLogonReqDissmissDeskessage(void* pData, UINT size);


	////////////////////////////////处理游戏服消息//////////////////////////////////////////
	bool OnHandleLoaderMessage(NetMessageHead * pNetHead, CenterServerMessageHead * pCenterHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent);
	// 游戏服资源变化（金币和钻石）
	bool OnHandleLoaderResChangeMessage(int userID, void* pData, UINT size);
	// 开房列表信息变化(人数变化)
	bool OnHandleLoaderBuyDeskInfoChangeMessage(void* pData, UINT size);
	// 游戏大结算
	bool OnHandleLoaderDeskDissmissMessage(void* pData, UINT size);
	// 局数变化
	bool OnHandleLoaderDeskStatusChangeMessage(void* pData, UINT size);
	// 火币变化通知
	bool OnHandleLoaderFireCoinChangeMessage(int userID, void* pData, UINT size);
	// 大奖通知
	bool OnHandleLoaderRewardMessage(void* pData, UINT size);
	// 开始比赛
	bool OnHandleLoaderNotifyStartMatchMessage(void* pData, UINT size);
	// 开始比赛失败
	bool OnHandleLoaderNotifyStartMatchFailMessage(int userID, void* pData, UINT size);
};
