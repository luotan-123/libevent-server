#pragma once

#include "CommonHead.h"
#include "RedisLoader.h"
#include "BaseMainManage.h"
#include "gameUserManage.h"
#include "GameDataBaseHandle.h"
#include "Exception.h"
#include "NewMessageDefine.h"
#include "ErrorCode.h"
#include "PlatformMessage.h"
#include "json/json.h"
#include "MD5.h"

using namespace std;

#define TIME_SPACE						100			//游戏 ID 间隔
#define TIME_START_ID					100			//定时器开始 ID

#define OFFLINE_CHANGE_AGREE_STATUS					//断线改变准备状态。注释掉断线不改变准备状态

#define G_CHANGEDESK_MAX_COUNT			12			//游戏换桌，最多搜索次数

// 定时器ID
enum LoaderServerTimerID
{
	LOADER_TIMER_BEGIN = 0,
	LOADER_TIMER_SAVE_ROOM_PEOPLE_COUNT,		// 写入当前房间人数
	LOADER_TIMER_CHECK_REDIS_CONNECTION,		// 检查redis连接性
	LOADER_TIMER_CHECK_INVALID_STATUS_USER,		// 检查无效状态的玩家
	LOADER_TIMER_CHECK_TIMEOUT_DESK,			// 检查超时的桌子
	LOADER_TIMER_COMMON_TIMER,					// 通用定时器
	LOADER_TIMER_HUNDRED_GAME_START,			// 百人类游戏启动通知事件(一次性)
	LOADER_TIMER_SCENE_GAME_START,				// 场景类游戏启动通知事件(一次性)
	LOADER_TIMER_COMBINE_DESK_GAME_BENGIN,		// 组桌游戏开始定时器
	LOADER_TIMER_END,
};

const int CHECK_SAVE_ROOM_PEOPLE_COUNT = 37;		// 定期保存游戏人数
const int CHECK_REDIS_CONNECTION_SECS = 307;		// 检查redis连接性时间
const int CHECK_INVALID_STATUS_USER_SECS = 67;		// 检查无效状态的玩家间隔
const int CHECK_TIMEOUT_DESK_SECS = 127;			// 检查超时桌子的时间
const int CHECK_COMMON_TIMER_SECS = 3;				// 通用定时器时间
const int CHECK_COMBINE_DESK_GAME_BENGIN_SECS = 3;	// 组桌游戏开始定时器（修改这个时间可以控制玩家匹配速度）

//基础数据管理类
class CGameDesk;
class CBaseMainManage;
class CGameUserManage;
class CGameMainManage : public CBaseMainManage
{
public:
	CGameMainManage();
	virtual ~CGameMainManage();
	CGameMainManage(CGameMainManage&) {}
	CGameMainManage& operator=(CGameMainManage&);

private:
	//数据管理模块初始化
	virtual bool OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);
	//数据管理模块卸载
	virtual bool OnUnInit();
	//数据管理模块启动
	virtual bool OnStart();
	//数据管理模块关闭
	virtual bool OnStop();
	//数据管理模块刷新
	virtual bool OnUpdate();

private:
	//服务扩展接口函数 （本处理线程调用）
	//SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, UINT dwHandleID);
	//SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime);
	//异步线程处理结果
	virtual bool OnAsynThreadResult(AsynThreadResultLine * pResultData, void * pData, UINT uSize);
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);

public:
	// 根据桌子id取桌子指针对象
	virtual CGameDesk* GetDeskObject(int deskIdx);

private:
	//创建游戏桌子信息
	virtual CGameDesk * CreateDeskObject(UINT uInitDeskCount, UINT & uDeskClassSize) = 0;
	//删除桌子信息
	virtual void DeleteDeskObject(CGameDesk** pCGameDesk) = 0;
	//删除所有定时器
	void KillAllTimer();
	//初始化游戏桌
	bool InitGameDesk(UINT uDeskCount, UINT	uDeskType);

private:
	//玩家掉线
	bool OnUserSocketClose(int userID, UINT uSocketIndex);

private:
	////////////////////////////////////////////////
	// 登录相关
	bool OnHandleLogonMessage(unsigned int assistID, void* pData, int size, unsigned int accessIP, unsigned int socketIdx, int userID);
	// 玩家登陆
	bool OnUserRequestLogon(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID);

	//房卡场登录逻
	bool OnPrivateLogonLogic(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID);
	//金币场登录逻辑
	bool OnMoneyLogonLogic(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID);
	//比赛场登陆逻辑
	bool OnMatchLogonLogic(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID);

	// 玩家登出
	bool OnUserRequestLogout(void* pData, UINT size, ULONG uAccessIP, UINT uIndex, int userID);

	//////////////////////////////////////////////
	// 动作相关
	bool OnHandleActionMessage(int userID, unsigned int assistID, void* pData, int size);
	// 玩家坐下
	bool OnHandleUserRequestSit(int userID, void* pData, int size);
	// 玩家搓桌坐下
	bool OnHandleUserRequestMatchSit(int userID, void* pData, int size);
	// 玩家站起
	bool OnHandleUserRequestStand(int userID, void* pData, int size);
	// 机器人请求取钱
	bool OnHandleRobotRequestTakeMoney(int userID, void* pData, int size);
	// 请求游戏开始
	bool OnHandleUserRequestGameBegin(int userID);
	// 玩家组桌坐下
	bool OnHandleUserRequestCombineSit(int userID, void* pData, int size);
	// 玩家取消组桌坐下
	bool OnHandleUserRequestCancelSit(int userID, void* pData, int size);
	///////////////////////////////////////////////
	// 框架消息
	bool OnHandleFrameMessage(int userID, unsigned int assistID, void* pData, int size);

	///////////////////////////////////////////////
	// 游戏消息
	bool OnHandleGameMessage(int userID, unsigned int assistID, void* pData, int size);
	// 解散消息
	bool OnHandleDismissMessage(int userID, unsigned int assistID, void* pData, int size);
	// 语音聊天相关
	bool OnHandleVoiceAndTalkMessage(int userID, unsigned int assistID, void* pData, int size);

	//////////////////////////////////////////////////////////////////////////
	//比赛场消息
	bool OnHandleMatchMessage(int userID, unsigned int assistID, void* pData, int size, long handleID);
	//请求所有桌子状态
	bool OnHandleMatchAllDeskStatusMessage(int userID, void* pData, int size, long handleID);
	//旁观其它桌子
	bool OnHandleMatchEnterWatchDeskMessage(int userID, void* pData, int size);
	//退出旁观
	bool OnHandleMatchQuitWatchDeskMessage(int userID, void* pData, int size);

public: //中心服发送消息
	bool SendMessageToCenterServer(UINT msgID, void* pData, UINT size, int userID = 0);
	// 通知大厅资源变化（金币和钻石）
	void SendResourcesChangeToLogonServer(int userID, int resourceType, long long value, int reason, long long changeValue);
	// 通知大厅资源变化（火币）
	void SendFireCoinChangeToLogonServer(int friendsGroupID, int userID, long long value, int reason, long long changeValue);
	// 通知全服大奖或者公告
	void SendRewardActivityNotify(const char * rewardMsg);

	//中心服消息
	virtual bool OnCenterServerMessage(UINT msgID, NetMessageHead * pNetHead, void* pData, UINT size, int userID);
	// 资源变化
	bool OnCenterMessageResourceChange(void* pData, int size, int userID);
	// 管理员解散牌桌
	bool OnCenterMessageFGDissmissDesk(void* pData, int size);
	// 开房列表解散桌子
	bool OnCenterMessageMasterDissmissDesk(void* pData, int size);
	// 关服，解散所有桌子
	void OnCenterCloseServerDissmissAllDesk(void* pData, int size);
	// 重新加载配置数据
	bool OnCenterMessageReloadGameConfig(void* pData, int size);
	// 开始比赛(实时赛)
	bool OnCenterMessageStartMatchPeople(void* pData, int size);
	// 开始比赛(定时赛)
	bool OnCenterMessageStartMatchTime(void* pData, int size);

	// 接口函数
public:
	void SendData(int userID, void* pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode);
	//发送通知消息
	bool SendErrNotifyMessage(int userID, const char* lpszMessage, int wType);
	// 获取玩家
	GameUserInfo* GetUser(int userID);
	// 玩家登出
	bool OnUserLogout(int userID);
	// 删除玩家
	void DelUser(int userID);
	// 获取roomID
	int GetRoomID();
	// 移除断线玩家
	bool RemoveOfflineUser(int userID);
	// 移除旁观玩家
	bool RemoveWatchUser(int userID);
	// 获取房间类型
	int GetRoomType();
	// 获取玩家状态
	int  GetUserPlayStatus(int userID);
	// 设置玩家状态
	bool SetUserPlayStatus(int userID, int status);
	// 获取在线玩家数量
	int GetOnlineUserCount();
	// 是否能旁观
	bool IsCanWatch();
	// 是否是组桌游戏
	bool IsCanCombineDesk();
	// 是否多人游戏公用一个游戏id
	bool IsMultiPeopleGame();
	// 通知玩家充钱
	void NotifyUserRechargeMoney(int userID, long long rechargeMoney, int leftSecs);
	// 判断某个玩家是否是游客
	bool IsVisitorUser(int userID);
	// 百人类预判是否还有桌子
	bool GoldRoomIsHaveDeskstation();
	// 加载奖池数据
	bool LoadPoolData();
	// 获取奖池配置，没有配置默认就是0
	int GetPoolConfigInfo(const char * fieldName);
	// 获取奖池配置，主要是获取数组类型
	bool GetPoolConfigInfoString(const char * fieldName, int * pArray, int size, int &iArrayCount);
	// 是否是1：1平台
	bool IsOneToOnePlatform();

	// 定时执行函数
private:
	// 检查无效状态的玩家
	void OnTimerCheckInvalidStatusUser();
	// 检查超时的桌子
	void CheckTimeOutDesk();
	// 通用定时器
	void OnCommonTimer();
	// 检查redis连接性
	void CheckRedisConnection();
	// 检查超时没有准备的玩家
	void CheckTimeoutNotAgreeUser();
	// 检查超时不操作玩家
	void CheckTimeoutNotOperateUser();
	// 百人类游戏启动通知
	void OnHundredGameStart();
	// 定期保存当前房间人数
	void OnSaveRoomPeopleCount();
	// 场景类游戏启动通知
	void OnSceneGameStart();
	// 为排队列表玩家匹配桌子，并开始游戏
	void OnCombineDeskGameBegin();

	// 组桌相关函数
private:
	// 添加玩家到组桌集合
	bool AddCombineDeskUser(int userID, BYTE isVirtual);
	// 集合中删除玩家
	bool DelCombineDeskUser(int userID, BYTE isVirtual);

	// 比赛场
public:
	// 为比赛分配一定数量的桌子
	bool GetMatchDesk(int needDeskCount, std::list<int> &listDesk);
	// 为比赛人员分配桌子，并坐下
	void AllocDeskStartMatch(const std::list<int> &matchDeskList, const std::vector<MatchUserInfo> &vecPeople);
	// 定时检查可以开始比赛的桌子
	void CheckDeskStartMatch();
	// 是否全部桌子都完成比赛
	bool IsAllDeskFinishMatch(long long llPartOfMatchID);
	// 某一张桌子游戏结束
	void MatchDeskFinish(long long llPartOfMatchID, int deskIdx);
	// 比赛进入下一轮
	void MatchNextRound(long long llPartOfMatchID, int iCurMatchRound, int iMaxMatchRound);
	// 比赛结束
	void MatchEnd(long long llPartOfMatchID, int iCurMatchRound, int iMaxMatchRound);
	// 根据积分，排序没有淘汰的玩家
	int MatchSortUser(std::vector<MatchUserInfo> &vecPeople, int iCurMatchRound);
	// 获取一个正确的机器人参加比赛
	int MatchGetRobotUserID();
	// 发送比赛邮件奖励
	void SendMatchGiftMail(int userID, int gameID, BYTE matchType, int gameMatchID, int ranking);
	// 发送比赛失败，退报名费，以及清理比赛状态
	void SendMatchFailMail(BYTE failReason, int userID, BYTE matchType, int gameMatchID);
	// 清理比赛状态
	void ClearMatchStatus(BYTE failReason, int userID, BYTE matchType, int gameMatchID);
	// 清理比赛玩家内存
	void ClearMatchUser(int gameMatchID, const std::vector<MatchUserInfo> &vecPeople);

private:
	//匹配坐桌相关定义
	std::set<int>		m_combineUserSet;					// 组桌玩家集合（快速查询）
	std::vector<int>	m_combineRealUserVec;				// 组桌玩家（真人）集合（匹配坐桌）
	std::vector<int>	m_combineRobotUserVec;				// 组桌玩家（机器人）集合（匹配坐桌）
	std::vector<int>	m_allCombineDeskUserVec;			// 所有人的集合

	//比赛场相关
public:
	std::map<long long, std::list<int> >				m_matchGameDeskMap;	// 每个比赛，包含的桌子索引
	std::map<long long, std::vector<MatchUserInfo> >	m_matchUserMap;		// 每个比赛，参赛的玩家
	std::map<long long, int>							m_matchMainIDMap;	// 每场比赛，所属的大ID
	std::map<long long, BYTE>							m_matchTypeMap;		// 每场比赛，比赛类型 MatchType

public:
	UINT				m_uNameID;							// 游戏名字 ID 号码
	RewardsPoolInfo		m_rewardsPoolInfo;					// 本场次奖池数据
protected:
	CGameUserManage*	m_pGameUserManage;					// 玩家的管理对象
	UINT				m_uDeskCount;						// 游戏桌数目
	CGameDesk			* * m_pDesk;						// 游戏桌指针
	CGameDesk			* m_pDeskArray;						// 游戏桌指针
};

//游戏数据管理类模板
template <class GameDeskClass, UINT uBasePoint, UINT uLessPointTimes> 
class CGameMainManageTemplate : public CGameMainManage
{
public:
	CGameMainManageTemplate() {}
	virtual ~CGameMainManageTemplate() 
	{
		SafeDeleteArray(m_pDesk);
		DeleteDeskObject(&m_pDeskArray);
	}

private:
	//获取信息函数 （必须重载）
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
	{
		//设置使用网络
		pKernelData->bStartTCPSocket = true;

		//设置数据库信息
		pKernelData->bLogonDataBase = true;
		pKernelData->bNativeDataBase = true;
		pKernelData->bStartSQLDataBase = true;

		//设置游戏信息
		pKernelData->uNameID = pInitData->uNameID;
		GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(pInitData->uNameID);
		pKernelData->uDeskPeople = pGameBaseInfo->deskPeople;

		return true;
	};

	//创建游戏桌子信息
	virtual CGameDesk* CreateDeskObject(UINT uInitDeskCount, UINT & uDeskClassSize)
	{
		uDeskClassSize = sizeof(GameDeskClass);
		return new GameDeskClass[uInitDeskCount];
	};

	//删除桌子信息
	virtual void DeleteDeskObject(CGameDesk** pCGameDesk)
	{
		GameDeskClass** pGameDeskClass = (GameDeskClass**)(pCGameDesk);
		SafeDeleteArray(*pGameDeskClass);
	}
};

/*******************************************************************************************************/
//游戏模块类模板
template <class GameDeskClass, UINT uBasePoint, UINT uLessPointTimes> 
class CGameModuleTemplate : public IModuleManageService
{
public:
	CGameDataBaseHandle														m_DataBaseHandle;	//数据库处理模块
	CGameMainManageTemplate<GameDeskClass, uBasePoint, uLessPointTimes>		m_GameMainManage;	//游戏数据管理

public:
	CGameModuleTemplate() {}
	virtual ~CGameModuleTemplate() {}

public:
	virtual bool InitService(ManageInfoStruct * pInitData)
	{
		KernelInfoStruct KernelData;
		return m_GameMainManage.Init(pInitData, &m_DataBaseHandle);
	}

	virtual bool UnInitService() { return m_GameMainManage.UnInit(); }
	virtual bool StartService(UINT &errCode) { return m_GameMainManage.Start(); }
	virtual bool StoptService() { return m_GameMainManage.Stop(); }
	virtual bool DeleteService() { delete this; return true; }
	virtual bool UpdateService() { return m_GameMainManage.Update(); }
};
