#pragma once

#include "UpgradeMessage.h"
#include "UpGradeLogic.h"
#include <queue>
#include <array>

using namespace DTProto;

class CNewServerManage : public CGameDesk
{
public:
	CNewServerManage();
	virtual ~CNewServerManage();

public:
	// 重载一些虚函数
	virtual bool InitDeskGameStation();
	virtual bool OnStart();
	virtual bool OnGetGameStation(BYTE deskStation, UINT uSocketID, bool bWatchUser);
	virtual bool HandleNotifyMessage(BYTE deskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);
	virtual bool OnTimer(unsigned int timerID);
	virtual bool ReSetGameState(BYTE bLastStation);
	virtual bool UserLeftDesk(GameUserInfo* pUser);
	virtual bool UserNetCut(GameUserInfo* pUser);
	virtual bool IsPlayGame(BYTE deskStation);
	virtual bool IsHundredGameSystemDealer();	//百人类游戏,判断是否系统做庄家
	virtual bool HundredGameIsInfoChange(BYTE deskStation);
	virtual void UserBeKicked(BYTE deskStation);
	virtual void LoadDynamicConfig();
	// 消息处理函数
private:
	// 请求上庄
	bool OnHandleUserRequestShangZhuang(BYTE deskStation, void* pData, int size);
	// 请求下庄
	bool OnHandleUserRequestXiaZhuang(BYTE deskStation);
	// 请求下注
	bool OnHandleUserReuqestNote(BYTE deskStation, void* pData, int size);
	// 请求续压
	bool OnHandleRequestContinueNote(BYTE deskStation);
	// 请求走势
	bool OnHandleRequestAreaTrend(BYTE deskStation);
	// 请求超端设置
	bool OnHandleRequestSuperSet(BYTE deskStation, void* pData, int size);
	// 请求是否继续坐庄
	bool OnHandleRequestContinueZhuang(BYTE deskStation, void* pData, int size);
	// 请求坐下
	bool OnHandleRequestSit(BYTE deskStation, void* pData, int size);

	// 定时器相关
private:
	void OnTimerWaitNext();
	void OnTimerSendCard();
	void OnTimerNote();
	void OnTimerSendNoteInfo();
	void OnTimerCompare();
	void OnTimerSettle();

private:
	long long GetNoteByIndex(int index);
	BYTE PopCardFromDesk();
	bool SendAreaCards(BYTE* pCardList);
	void KillAllTimer();
	bool ConfirmDealer();
	time_t GetNextStageLeftSecs();
	void SetResult(int result);

private:
	// 等待开始阶段
	void TransToWaitNextStage();
	// 发牌阶段
	void TransToSendCardStage();
	// 下注阶段
	void TransToNoteStage();
	// 比牌阶段
	void TransToCompareStage();
	// 结束阶段
	void TransToSettleStage();

private:
	// 通知游戏状态
	void BroadcastGameStatus();
	// 通知庄信息
	void BroadcastZhuangInfo();
	// 通知筹码(下注)信息
	void BrodcastNoteInfo(BYTE deskStation=255);
	// 填充庄信息(庄家信息+上庄列表)
	bool MakeZhuangInfo(NewGameMessageNtfZhuangInfo& msg);
	// 填充发牌信息
	bool MakeSendCardInfo(NewGameMessageNtfSendCard& msg);
	// 计算区域输赢信息
	void CalcAreaRate();
	// 更换系统名字资源
	void ChangeSystemZhuangInfo();
public:
	/////////////////////////////////////////奖值/////////////////////////////////
	///机器人输赢自动控制，true：控制了，false：没有控制
	void AiWinAutoCtrl();
	//获取本局控制输还是控制赢，1：控制赢，-1：控制输，0：不控制
	int GetCtrlRet();
	//计算当前系统赢钱
	long long CalcSystemWinMoney(int iRateValue);
	//执行超端设置
	void SuperSetChange();
	//机器人控制接口
private:
	// 判断机器人是否可以上下庄  0：上庄   1：下庄
	bool IsVirtualCanShangXiazZhuang(BYTE deskStation, BYTE type);
	// 控制下注
	bool IsVirtualCanNote(BYTE deskStation, long long note);
	// 判断机器人是否可以上坐下站起  0：坐下   1：站起
	bool IsVirtualCanSit(BYTE deskStation, BYTE type);
	//座位相关
private:
	bool IsDeskUser(BYTE deskStation);
	bool DelDeskUser(BYTE deskStation);
	bool SetDeskUser(BYTE deskStation, BYTE byIndex);
	bool IsDeskHaveUser(BYTE byIndex);
	void ClearDeskUser();
	//每局可以上座的机器人数
	void UpSitPlayerCount();
	std::vector<BYTE> m_gameDesk;

private:
	GameConfig m_gameConfig;				// 游戏配置

private:
	CPokerGameLogic m_logic;
	std::vector<DTUserInfo> m_userInfoVec;				// 玩家信息数组(不包含庄家)
	DealerInfo m_dealerInfo;							// 庄家信息
	std::vector<DealerInfo> m_dealerInfoVec;			// 上庄信息
	std::array<AreaInfo, MAX_AREA_COUNT> m_areaInfoArr;	// 区域信息
	std::queue<BYTE> m_deskCardQueue;					// 桌上的牌
	int m_gameStatus;									// 游戏状态
	BYTE m_dealerDeskStation;							// 庄家位置(254为系统)
	BYTE m_LastDealerDeskStation;                       // 上局庄家位置
	long long m_LastShangzhuangMoney;                   // 上局结束后的庄家携带金币数
	int m_currDealerGameCount;							// 当前庄家进行的局数(仅对玩家有效)
	bool m_xiaZhuangRightNow;							// 是否立刻下庄
	time_t m_startTime;									// "开始"时间(等待开始阶段开始计时)
	bool					m_bIsSuper[PLAY_COUNT];			// 是否是超端
	BYTE					m_bySuperSetType;				// 超端设置类型
	int m_currWinArea;    //当前开赢的区域
	std::list<TrendData> m_resultList;// 区域结果
	std::list<int>		 m_resultSummaryList;
	
	ChouMaInfo m_tempChouMaData[MAX_CHOU_MA_COUNT];		// 筹码统计
	int m_tempChouMaDataCount;							// 筹码数量
	int m_areaChouMaCount[MAX_AREA_COUNT][MAX_NOTE_SORT];	// 每个区域筹码数量

	int m_allCount;
	int m_longWinCount;
	int m_iContinueZhuang;	//是否继续坐庄，-1：默认值，0：下庄，1：继续坐庄

	//////////////////////////////////奖值、超端数据////////////////////////////////////////
	//RewardsPoolInfo			m_pooInfo;
	std::vector<BYTE>		m_countResultCardVec;
	std::vector<CtrlSystemParam>	m_ctrlParam;		// 一组控制参数值
	int m_curCtrlWinCount;
	int m_runCtrlCount;
	int m_ctrlParamIndex;
	char m_szLastCtrlParam[256];
};