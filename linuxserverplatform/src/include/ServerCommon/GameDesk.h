#pragma once

#include "GameMainManage.h"
#include "gameUserManage.h"


//定时器
#define IDT_AGREE_DISMISS_DESK		1		//同意解散桌子定时器
#define IDT_FRIEND_ROOM_GAMEBEGIN	2		//好友房游戏开始定时器

//定时器相关时间
const int CFG_DISMISS_DESK_WAIT_TIME = 120;						// 解散桌子等待时间
const int FRIEND_ROOM_WAIT_BEGIN_TIME = 61;						// 金币房、vip房充值时间
const int GOLD_DESK_TIMEOUT_UNAGREE_KICKOUT_SECS = 31;			// 金币场超时未准备T除玩家时间
static int HUUNDRED_GAME_TIMEOUT_OPERATE_KICKOUT_SECS = 120;	// 百人类游戏场景类游戏，长时间不操作踢人时间

//游戏结束标志
#define GFF_FORCE_FINISH			0			//强行解除
#define GFF_SAFE_FINISH				1			//解除游戏
#define GF_NORMAL					2			//游戏正常结束
#define GF_SALE						3			//游戏安全结束
#define GF_NET_CAUSE                4			//网络原因，游戏结束
#define GFF_DISSMISS_FINISH			5			//解散桌子结束

//开始模式
enum BeginMode
{
	FULL_BEGIN = 0,				// 满人才开始
	ALL_ARGEE = 1,				// 所有人同意就开始
};

enum DismissType
{
	DISMISS_TYPE_DEFAULT = 0,	// 默认
	DISMISS_TYPE_AGREE,			// 同意解散
	DISMISS_TYPE_DISAGREE,		// 不同意解散
};

// 桌子玩家信息
struct DeskUserInfo
{
	bool    bNetCut;			// 是否断线
	BYTE	deskStation;
	int		userID;
	int		dismissType;		// 解散类型
	time_t	lastWaitAgreeTime;	// 上次等待准备时间(包含第一次进入游戏, 游戏结束)仅实用于金币场
	bool	isAuto;				// 是否托管
	BYTE	isVirtual;			// 是否机器人
	int		leftRoomUser;		// 是否以前离开过桌子(私人房用)

	DeskUserInfo()
	{
		clear();
	}

	void clear()
	{
		deskStation = INVALID_DESKSTATION;
		userID = 0;
		dismissType = DISMISS_TYPE_DEFAULT;
		bNetCut = false;
		isAuto = false;
		lastWaitAgreeTime = 0;
		isVirtual = 0;
		leftRoomUser = 0;
	}
};

// 旁观者信息
struct WatchUserInfo
{
	int userID;
	BYTE deskStation;

	bool friend operator < (WatchUserInfo a, WatchUserInfo b)
	{
		return a.userID >= b.userID;
	}
};

class CGameMainManage;
class CGameDesk
{
public: //构造函数和析构函数
	explicit CGameDesk(BYTE byBeginMode);
	virtual ~CGameDesk();

	//虚函数，子类重载的函数，包括一些动作，属性判断等等
public:
	// 获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser) = 0;
	// 重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation) = 0;
	// 初始化游戏逻辑
	virtual bool InitDeskGameStation() { return true; }
	// 游戏开始
	virtual bool GameBegin(BYTE bBeginFlag);
	// 游戏结束
	virtual bool GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	// 定时器消息
	virtual bool OnTimer(UINT uTimerID);
	// 桌子成功解散 isDismissMidway=是否中途解散
	virtual void OnDeskSuccessfulDissmiss(bool isDismissMidway = true);
	// 初始化游戏数据，大结算初始化
	virtual void InitDeskGameData() { };
	// 判断游戏桌上的某个玩家是否开始游戏了
	virtual bool IsPlayGame(BYTE bDeskStation) { return m_bPlayGame; }
	// 是否可以开始游戏
	virtual bool CanBeginGame();
	// 用户同意开始
	virtual bool UserAgreeGame(BYTE bDeskStation);
	// 解散消息函数
	virtual bool HandleDissmissMessage(BYTE deskStation, unsigned int assistID, void* pData, int size);
	// 用户断线离开
	virtual bool UserNetCut(GameUserInfo *pUser);
	// 百人类游戏启动，百人类游戏必须重载
	virtual bool OnStart() { return true; }
	// 百人类游戏,判断这个玩家名字是否可变（包括机器人，以及上庄列表玩家）
	virtual bool HundredGameIsInfoChange(BYTE deskStation) { return false; }
	// 设置游戏状态
	virtual void SetGameStation(BYTE byGameStation) { m_byGameStation = byGameStation; }
	// 获得游戏状态
	virtual BYTE GetGameStation() { return m_byGameStation; }
	// 回放标识码
	virtual bool GetVideoCode(char *pCode, int iLen);
	// 框架消息
	virtual bool HandleFrameMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);
	// 游戏消息函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);
	// 玩家离开桌子，直接返回大厅
	virtual bool UserLeftDesk(GameUserInfo* pUser);
	// 玩家请求托管
	virtual bool OnHandleUserRequestAuto(BYTE deskStation);
	// 玩家取消托管
	virtual bool OnHandleUserRequestCancelAuto(BYTE deskStation);
	// 旁观者是否可以坐桌
	virtual bool WatchCanSit(GameUserInfo* pUser) { return true; }
	// 特殊游戏专用，比如德州扑克
	virtual void UserBeKicked(BYTE deskStation) {}
	// 检查玩家的金币能否继续玩游戏
	virtual int CheckUserMoney(int userID);
	// 动态加载配置文件数据
	virtual void LoadDynamicConfig() {}
	// 玩家坐桌通知游戏
	virtual bool UserSitDeskActionNotify(BYTE deskStation) { return true; };

	// 接口函数
public:
	// 初始化函数
	bool Init(int deskIdx, BYTE bMaxPeople, CGameMainManage * pDataManage);
	// 初始化购买桌子数据
	void InitBuyDeskData();
	// 初始化解散数据
	void InitDismissData();
	// 设置定时器，定时器默认只执行一次，可以修改timerType来实现永久定时器
	bool SetTimer(UINT uTimerID, int uElapse, BYTE timerType = SERVERTIMER_TYPE_SINGLE);
	// 删除定时器
	bool KillTimer(UINT uTimerID);
	// 获取房间类型
	int GetRoomType();
	// 获取房间种类（用于百人类）
	int GetRoomSort();
	// 获取房间等级（初级、中级、高级）
	int GetRoomLevel();
	// 加载桌子信息
	void LoadPrivateDeskInfo(const PrivateDeskInfo& privateDeskInfo);
	// 构造消息包
	void MakeAllUserInfo(char* pBuf, int size, int& realSize);
	// 百人类构造玩家信息
	void HundredMakeAllUserInfo(char* pBuf, int size, int& realSize);
	// 合成解散房间消息
	void MakeDismissData(char* buf, int& size);
	// 合成玩家简单信息
	bool MakeUserSimpleInfo(BYTE deskStation, UserSimpleInfo& userInfo);
	// 清除房间里面桌子玩家的信息，把管理器中玩家的deskIndex和deskStation置为无效
	void ClearAllData(const PrivateDeskInfo& privateDeskInfo);
	// 房卡场扣除房卡
	BuyGameDeskInfo ProcessCostJewels();
	// 需要购买的房间，添加战绩
	bool AddDeskGrade(const char *pVideoCode, const char * gameData, const char * userInfoList);
	// 非金币场踢人，对大厅战绩有影响，仅仅提供给德州扑克类游戏调用
	bool KickOutUser(BYTE deskStation, int ReaSon = REASON_KICKOUT_STAND);
	// 获取一个可以开始游戏的玩家
	void SetBeginUser();
	// 检查没有准备的玩家
	void CheckTimeoutNotAgreeUser(time_t currTime);
	// 检查没有操作的玩家
	void CheckTimeoutNotOperateUser(time_t currTime);
	// 通知资源变化
	void NotifyUserResourceChange(int userID, int resourceType, long long value, long long changeValue);

	// 发送数据相关
public:
	//发送游戏状态
	bool SendGameStation(BYTE deskStation, UINT socketID, bool bWatchUser, void * pData, UINT size);
	//发送数据函数
	bool SendGameData(BYTE deskStation, unsigned int mainID, unsigned int assistID, unsigned int handleCode);
	//发送数据函数 
	bool SendGameData(BYTE bDeskStation, void* pData, int  size, unsigned int mainID, unsigned int assistID, unsigned int handleCode);
	//发送旁观数据
	bool SendWatchData(void * pData, int uSize, int mainID, int assistID, int handleCode);
	//发送通知消息
	bool SendGameMessage(BYTE deskStation, const char * lpszMessage, int wType = SMT_EJECT);
	// 广播桌子信息
	void BroadcastDeskData(void *pData, int size, unsigned int mainID, unsigned int assistID, bool sendVirtual = true, unsigned int handleCode = 0);
	// 发送所有玩家信息
	void SendAllDeskUserInfo(BYTE deskStation);
	// 发送房间的基本信息
	void SendDeskBaseInfo(BYTE deskStation);
	// 广播房间的基本信息
	void BroadcastDeskBaseInfo();
	// 广播玩家坐下信息
	void BroadcastUserSitData(BYTE deskStation);
	// 广播玩家坐下信息（不广播自己）
	void BroadcastUserSitDataExceptMyself(BYTE deskStation);
	// 广播玩家简单信息
	void BroadcastUserSimpleInfo(BYTE deskStation);
	// 广播玩家离开消息
	bool BroadcastUserLeftData(BYTE deskStation, int reason);
	// 发送玩家离开消息
	void SendUserLeftData(BYTE deskStation, int reason);
	// 广播玩家同意消息
	void BroadcastUserAgreeData(BYTE deskStation);
	// 关闭玩家断线消息
	void BroadcastUserOfflineData(BYTE deskStation);
	// 广播玩家被T掉消息
	void BroadcastUserKickoutData(BYTE deskStation);
	// 发送等待准备时间信息
	void SendLeftWaitAgreeData(BYTE deskStation);
	// 通知桌子玩家解散桌子信息
	void BroadcastDismissData();
	// 发送解散信息(用于断线重连)
	void SendDismissData();
	// 广播消息，设定不广播特定玩家
	void BroadcastGameMessageExcept(BYTE deskStation, const char * lpszMessage, int wType = SMT_EJECT);

	// 结算函数
protected:
	// 私有房结算函数
	bool ChangeUserPointPrivate(long long *arPoint, bool *bCut, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL);
	// 私有房结算函数
	bool ChangeUserPointPrivate(int *arPoint, bool *bCut, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL);
	// 改变玩家积分，适用于金币场
	bool ChangeUserPoint(long long *arPoint, bool *bCut, long long * rateMoney = NULL);
	// 改变玩家积分，适用于金币场
	bool ChangeUserPoint(int *arPoint, bool *bCut, long long * rateMoney = NULL);
	// 改变玩家背包数据，暂时捕鱼使用
	bool ChangeUserBage(BYTE deskStation, char * resName, int changeNum = 1, bool add = true);
	// 改变玩家金币
	bool ChangeUserPoint(BYTE deskStation, long long point, long long rateMoney = 0, bool notify = true);
	// 改变玩家火币
	bool ChangeUserFireCoin(long long *arPoint, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL);
	// 改变玩家火币
	bool ChangeUserFireCoin(int *arPoint, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL);
	// 金币房结算函数
	bool ChangeUserPointGoldRoom(long long *arPoint, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL, long long * rateMoney = NULL);
	// 金币房结算函数
	bool ChangeUserPointGoldRoom(int *arPoint, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL, long long * rateMoney = NULL);
	// 钻石房结算函数
	bool ChangeUserPointJewelsRoom(long long *arPoint, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL, long long * rateJewels = NULL);
	// 钻石房结算函数
	bool ChangeUserPointJewelsRoom(int *arPoint, char *pVideoCode = NULL, bool bGrade = false, char * gameData = NULL, long long * rateJewels = NULL);

	// 属性判断函数
public:
	// 桌子是否有效
	bool IsEnable() { return m_enable; }
	// 玩家是否托管
	bool IsAuto(BYTE deskStation);
	// 判断玩家是否掉线
	bool IsNetCut(BYTE deskStation);
	// 判断玩家是否机器人
	bool IsVirtual(BYTE deskStation);
	// 判断是否是超端
	bool IsSuperUser(BYTE deskStation);
	// 判断是否是游客
	bool IsVisitorUser(BYTE deskStation);
	// 是否房卡场
	bool IsPrivateRoom() { return GetRoomType() == ROOM_TYPE_PRIVATE; }
	// 是否金币场
	bool IsGoldRoom() { return GetRoomType() == ROOM_TYPE_GOLD; }
	// 是否私人房
	bool IsFriendRoom() { return GetRoomType() == ROOM_TYPE_FRIEND; }
	// 是否俱乐部VIP房间
	bool IsFGVIPRoom() { return GetRoomType() == ROOM_TYPE_FG_VIP; }
	//是否可以坐桌
	bool IsCanSitDesk();
	// 是否旁观者
	bool IsWatcher(int userID);
	// 是否还有座位号
	bool IsHaveDeskStation(int userID, const PrivateDeskInfo &info);
	// 是否全部玩家掉线
	bool IsAllUserOffline();
	// 游戏判断是否需要踢一个机器人(主要是百人类使用)
	bool IsKickOutVirtual(BYTE deskStation);

	// 消息处理相关
public:
	// 处理玩家登录
	bool OnPrivateUserLogin(int userID, const PrivateDeskInfo& info);
	// 处理玩家请求桌子信息
	bool OnHandleUserRequestDeskInfo(BYTE deskStation);
	// 处理玩家请求游戏信息
	bool OnHandleUserRequestGameInfo(BYTE deskStation);
	// 处理玩家请求桌子玩家信息
	bool OnHandleUserRequestALLUserInfo(BYTE deskStation);
	// 处理请求单个玩家信息
	bool OnHandleUserRequestOneUserInfo(BYTE deskStation, void* pData, int size);
	// 处理玩家主动请求解散
	bool OnHandleUserRequestDissmiss(BYTE deskStation);
	// 处理玩家回应解散消息
	bool OnHandleUserAnswerDissmiss(BYTE deskStation, void* pData, int size);
	// 房间解散失败
	bool OnDeskDismissFailed();
	// 玩家同意解散
	bool OnUserAgreeDismiss(BYTE deskStation);
	// 处理玩家聊天消息
	bool OnHandleTalkMessage(BYTE deskStation, void* pData, int size);
	// 处理玩家语音消息
	bool OnHandleVoiceMessage(BYTE deskStation, void* pData, int size);
	// 处理玩家使用魔法表情
	bool OnHandleUserRequestMagicExpress(BYTE deskStation, void* pData, int size);
	// 请求游戏开始
	bool OnUserRequsetGameBegin(int userID);

	// 玩家数据相关
public:
	// 通过座位号获取玩家ID
	int GetUserIDByDeskStation(BYTE deskStation);
	// 通过玩家ID获取座位号
	BYTE GetDeskStationByUserID(int userID);
	// 通知桌子号获取玩家数据
	bool GetUserData(BYTE deskStation, UserData& userData);
	// 通知桌子号获取玩家数据
	bool GetUserData(BYTE deskStation, GameUserInfo& userData);
	// 获取玩家背包数据
	bool GetUserBagData(BYTE bDeskStation, UserBag & userBagData);
	// 通过key获取玩家某道具数量
	int GetUserBagDataByKey(BYTE bDeskStation, const char * resName);
	// 获取当前玩家的金币数或者火币数量
	bool GetRoomResNums(BYTE deskStation, long long &resNums);
	// 设置玩家积分
	bool SetUserScore(BYTE deskStation, long long score);
	// 获取玩家积分
	long long GetUserScore(BYTE deskStation);
	// 获取玩家控制参数
	int GetUserControlParam(BYTE deskStation);

	// 桌子数据相关
public:
	// 获取桌子游戏状态
	bool GetPlayGame() { return m_bPlayGame; }
	// 获取准备玩家数量
	int AgreePeople();
	// 获取真人玩家
	int GetRealPeople();
	// 获取机器人玩家
	UINT GetRobotPeople();
	// 获取底分(倍率)
	int GetBasePoint();
	// 获取当前玩家数量
	int GetUserCount();
	// 获取离线玩家数量
	int GetOfflineUserCount();
	// 获取已准备的玩家数量
	int GetAgreeUserCount();
	// 设置私人房踢人最低金币
	void SetRemovePlayerMinPoint(int point) { m_RemoveMinPoint = point; }
	// 百人类和场景类游戏，设置超时时间
	void SetOperationTimeout(int time) { HUUNDRED_GAME_TIMEOUT_OPERATE_KICKOUT_SECS = time; }

	// 玩家动作相关(坐下、离开和掉线)
public:
	// 玩家坐到桌子上
	bool UserSitDesk(GameUserInfo* pUser);
	// 房卡场坐桌逻辑
	bool PrivateSitDeskLogic(GameUserInfo* pUser);
	// 金币场坐桌逻辑
	bool MoneySitDeskLogic(GameUserInfo* pUser);
	// 房卡场离开桌子逻辑
	bool PrivateUserLeftDeskLogic(GameUserInfo * pUser);
	// 金币场离开桌子逻辑
	bool MoneyUserLeftDeskLogic(GameUserInfo * pUser);
	// 百人类游戏离开桌子逻辑
	bool HundredGameUserLeftLogic(GameUserInfo* pUser);
	// 百人类游戏断线逻辑
	bool HundredGameUserNetCutLogic(GameUserInfo* pUser);
	// 旁观者离开
	bool OnWatcherLeftDesk(GameUserInfo* pUser, const PrivateDeskInfo & deskInfo);
	// 桌子玩家离开
	bool OnDeskUserLeftDesk(GameUserInfo* pUser, const PrivateDeskInfo& deskInfo);
	// 处理玩家旁观
	bool ProcessUserWatch(GameUserInfo* pUser, const PrivateDeskInfo& privateDeskInfo);
	// 处理玩家坐下
	bool ProcessPrivateUserSitDesk(GameUserInfo* pUser, const PrivateDeskInfo& privateDeskInfo);
	// 旁观者掉线
	bool OnWatchUserOffline(int userID);

	// 游戏流程相关
protected:
	// 给旁观者分配座位
	BYTE AllocWatcherDeskStation();
	// 分配桌子的座位号
	BYTE AllocDeskStation(int userID);
	// 百人类游戏开始
	bool HundredGameBegin();
	// 百人类游戏结束
	bool HundredGameFinish();
	// 金币场开局扣金币
	void ProcessDeduceMoneyWhenGameBegin();
	// 平台游戏大结算发送相关数据
	void OnDeskDissmissFinishSendData();

	// 定时器相关
private:
	// 好友房定时开始
	bool OnTimerFriendRoomGameBegin();

	// 抽水函数
protected:
	// 获取系统抽水值
	double GetDeskPercentage();
	// 设置抽水之后的分数
	bool SetDeskPercentageScore(long long * arPoint, long long * ratePoint = NULL, bool arPointIsChange = true);
	// 设置抽水之后的分数
	bool SetDeskPercentageScore(int * arPoint, int * ratePoint = NULL, bool arPointIsChange = true);

	// 中心服相关
public:
	// 大厅解散房间消息，游戏已经开始就不解散
	void LogonDissmissDesk(int userID, bool bDelete);
	// 大厅解散房间消息(强行解散)
	void LogonDissmissDesk();
	// 通知大厅开房信息发生变化(人数变化)
	void NotifyLogonBuyDeskInfoChange(int masterID, int userCount, int userID, BYTE updateType, int deskMixID);
	// 大结算
	void NotifyLogonDeskDissmiss(const PrivateDeskInfo& privateDeskInfo);
	// 局数变化
	void NotifyLogonDeskStatusChange();

	// 平台参数和奖池数据
protected:
	// 设置系统奖池
	bool SetServerRoomPoolData(const char * fieldName, long long fieldValue, bool bAdd);
	// 设置系统奖池
	bool SetServerRoomPoolData(const char * fieldName, const char * fieldValue);
	// 获取房间配置数据
	bool GetRoomConfigInfo(char configInfo[2048], int size);
	// 是否是1：1平台
	bool IsOneToOnePlatform();
	// 平台倍率
	int GetPlatformMultiple();

	// 比赛场
public:
	// 初始化比赛场数据
	void InitDeskMatchData();
	// 比赛场游戏结束
	bool MatchRoomGameBegin();
	// 比赛场游戏结束
	bool MatchRoomGameFinish();
	// 发送当前桌子的比赛状态
	void SendMatchDeskStatus(int userID);
	// 比赛场坐桌逻辑
	bool MatchRoomSitDeskLogic(GameUserInfo* pUser);
	// 比赛场玩家离开桌子
	bool MatchRoomUserLeftDeskLogic(GameUserInfo * pUser);
	// 比赛场获取一个桌子所有玩家id
	int MatchGetDeskUserID(int arrUserID[MAX_PLAYER_GRADE]);
	// 比赛场进入本桌旁观
	bool MatchEnterMyDeskWatch(GameUserInfo * pUser, long long partOfMatchID);
	// 比赛场退出本桌旁观
	bool MatchQuitMyDeskWatch(GameUserInfo * pUser, int socketIdx, BYTE result);
	// 是否比赛房
	bool IsMatchRoom() { return GetRoomType() == ROOM_TYPE_MATCH; }
	// 比赛场结算函数
	bool ChangeUserPointMatchRoom(long long *arPoint);
	// 比赛场结算函数
	bool ChangeUserPointMatchRoom(int *arPoint);

public:
	// init 时候初始化的数据，生命周期从游戏启动到游戏结束
	CGameMainManage				* m_pDataManage;		// 管理器指针
	BYTE						m_byMaxPeople;			// 游戏人数
	BYTE						m_byBeginMode;			// 同意模式
	int							m_deskIdx;				// 桌子索引
	bool						m_needLoadConfig;		// 游戏中是否需要加载数据
	int							m_iRunGameCount;		// 游戏运行的局数
	std::string					m_ctrlParmRecordInfo;	// 控制信息，游戏控制的时候填入，后面会写到数据库

public:
	// 创建房间的时候需要设置的数据
	int							m_iVipGameCount;								// 购买桌子局数(可变)
	char						m_szGameRules[MAX_BUY_DESK_JSON_LEN];			// 游戏规则。json 格式，具体游戏定义
	char						m_szDeskPassWord[MAX_PRIVATE_DESK_PASSWD_LEN];	// 桌子密码
	int						m_masterID;										// 房主ID
	bool						m_isMasterNotPlay;                              // 是否替他人开房
	int							m_iConfigCount;				// 桌子最大人数（特殊配置）
	int							m_MinPoint;					// 入场限制（金币房和俱乐部vip房间）
	int							m_RemoveMinPoint;			// 踢人下限（金币房和俱乐部vip房间）
	int							m_basePoint;				// 底注（金币房）
	BYTE						m_roomTipType;				// 抽水方式
	BYTE						m_roomTipTypeNums;			// 抽水率
	BYTE						m_payType;					// 支付类型
	BYTE						m_playMode;					// 游戏玩法，如明牌抢庄
	PlatformFriendsGroupMsg		m_friendsGroupMsg;			// 桌子的俱乐部信息

private:
	// 申请解散相关数据
	bool						m_isDismissStatus;			// 是否处于解散状态(房主请求解散，但是还未真正解散)
	BYTE						m_reqDismissDeskStation;	// 请求解散的人
	time_t						m_reqDismissTime;			// 请求解散房间的时间

private:
	// 游戏运行中的数据
	bool						m_bPlayGame;				// 游戏是否开始标志,主要标识单局游戏
	BYTE						m_byGameStation;			// 游戏状态
	bool						m_enable;					// 是否有效
	time_t						m_beginTime;
	time_t						m_finishedTime;
	int							m_beginUserID;				// 开始游戏玩家

public:
	// 比赛场
	long long					m_llPartOfMatchID;			// 桌子属于的比赛id（小）
	int							m_iCurMatchRound;			// 比赛进行的轮数
	int							m_iMaxMatchRound;			// 最大比赛轮数
	time_t						m_llStartMatchTime;			// 比赛开始时间
	bool						m_bFinishMatch;				// 本桌是否完成比赛
	std::set<int>				m_matchWatchUserID;			// 本桌旁观者信息

private:
	// 其他数据
	std::vector<DeskUserInfo>	m_deskUserInfoVec;			// 玩家信息数组
	std::vector<long long>		m_gradeIDVec;				// 战绩列表
	std::set<WatchUserInfo>		m_watchUserInfoSet;			// 旁观者信息集合
	int							m_finishedGameCount;		// 完成的游戏局数
	bool						m_isBegin;					// 游戏是否开始, 房卡场第一局开始之后置为开始，持续到房间解散或者最后一局结束
	int							m_autoBeginMode;			// 自动开始模式(0：房主开始 N:满N人开始)
	bool						m_bGameStopJoin;			// 是否中途禁止加入
	int							m_iBuyGameCount;			// 购买桌子局数（创建房间固定，不可变）
	long long					m_uScore[MAX_PLAYER_GRADE];	// 桌子玩家积分
	int							m_gameWinCount[MAX_PLAYER_GRADE];	// 每个玩家的胜局数
};
