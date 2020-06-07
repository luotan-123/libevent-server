#pragma once

#include "Playback.h"

#define SAFECHECK_STRUCT(A,StructType,pData,iSize)\
	if(sizeof(StructType) != iSize)			      \
	return true;							      \
	StructType *A = (StructType*)pData;		      \
	if(nullptr == A)							  \
	return true;							      \

//定时器 ID
#define TIME_GAME_BEING_PLAY				31				//开始玩游戏
#define TIME_OUTCARD_OUTTIME				32				//出牌超时
#define TIME_SEND_NEXT_INTEVAL				33				//定时发送下一张牌
#define TIME_REMAINING						34				//余下时间
#define TIME_FINISH							35
#define TIME_PAO							36              //跑牌
#define TIME_PIAO							37              //选漂分定时器
#define TIME_AUTO_AGREE						38              //自动准备定时器
#define TIME_OPER_OUTTIMER					39				//操作超时(定时器范围 [39,39+PLAY_COUNT))
#define TIME_WEI_TI_DELAY_HU				43				//偎或提后可胡,延迟发胡(定时器范围 [43,43+PLAY_COUNT)

const BYTE INVALID_TABLE = 255;
const BYTE INVALID_CHAIR = 255;

//游戏桌类
class CServerGameDesk : public CGameDesk
{
private:
	CGameLogic						m_GameLogic;			//游戏逻辑
	/*---------------------------游戏可配置参数----------------------------------*/
	bool							m_cfgbTurnRule;			//游戏顺序 0-顺时针 1-逆时针
	BYTE							m_cfgbyBeginTime;		//游戏开始时间
	BYTE							m_cfgbyThinkTime;		//游戏摆牛思考时间
	BYTE							m_cfgbySendCardTime;	//发牌时间-控制客户端发一张牌的速度
	BYTE							m_byRemainingTime;		//余下时间
	BYTE							m_cfgPiaoTime;			//选漂分时间
//[游戏变量]
public:
	BYTE							m_byBankerUser;							//庄家用户
	BYTE							m_byCurrentUser;						//当前玩家
	BYTE							m_byTiOrPaoPaiUser;						//提或跑牌玩家
	BYTE							m_byUserHuXiCount[PLAY_COUNT];			//胡息数目
	BYTE							m_byCardIndex[PLAY_COUNT][MAX_INDEX];	//玩家手牌(转换成索引值)
	BYTE							m_byCardIndexJing[PLAY_COUNT];			//保存金牌的数量
	BYTE							m_byCardCount[PLAY_COUNT];				//牌总数量
	BYTE							m_byUserStation[PLAY_COUNT];			//记录用户状态
	BYTE							m_byHuUser;								//保存胡牌玩家
	int								m_iHuOperation;							//当前胡牌操作
	int								m_iHuRealOperation;						//当前胡牌实际操作(这个有点搓,但是不想改了)
	BYTE							m_byUserOutCard[PLAY_COUNT][OUT_CARD_NUM];//玩家出牌数据（弃牌数）
	int								m_iOutCardCount[PLAY_COUNT];			  //玩家出牌个数
	BYTE							m_byCurrentCatchUser;					  //当前摸牌玩家
	BYTE							m_byZuoXingUser;						  //坐醒玩家(4人玩法用)

	OutCard							m_tOutCard;								//出牌数据或弃牌据
	int								m_iRoomID;
//[标志变量]
protected:
	bool							m_bOutCard;								//出牌标志
	bool							m_bEstimate;							//判断标志
	bool							m_bDispatch;							//派发标志
	int                             m_choushui;

//[提偎变量]
protected:
	bool							m_bTiCard;								//提牌标志
	bool							m_bSendNotify;							//发送提示

//[辅助变量]
protected:
	BYTE							m_byResumeUser;							//还原用户
	BYTE							m_byOutCardUser;						//出牌用户
	BYTE							m_byOutCardData;						//出牌扑克
	BYTE							m_byCatchCardUser;						//抓牌用户
	BYTE							m_byCatchCardData;						//抓牌扑克
//[发牌信息]
protected:
	BYTE							m_byLeftCardCount;						//剩余数目
	BYTE							m_byRepertoryCard[84];					//库存扑克

	//用户状态
public:
	bool							m_bResponse[PLAY_COUNT];				//响应标志
	int								m_iUserAction[PLAY_COUNT];				//用户动作
	BYTE							m_byChiCardKind[PLAY_COUNT];			//吃牌类型
	int								m_iPerformAction[PLAY_COUNT];			//执行动作
	SaveChiData						m_tChiData[PLAY_COUNT];					//吃牌的数据

	tagChiCardInfo					m_tChiCardInfo[PLAY_COUNT];				//有吃牌时玩家能吃的牌
//[限制状态]
public:
	bool							m_bMingPao[PLAY_COUNT];					//明跑标志
	bool							m_bAbandonCard[PLAY_COUNT][MAX_INDEX];	//放弃扑克(吃)
	bool							m_bAbandonCard1[PLAY_COUNT][MAX_INDEX];	//放弃扑克(碰)

//[组合扑克](主要保存吃碰杠偎提)
protected:
	GameData						m_GameData;								//数据管理
	BYTE							m_byXingPai;							//当前醒牌
protected:
	tagHuCardInfo					m_tHuCardInfo[PLAY_COUNT][5];			//第0个放普通胡牌类型,1:放王钓胡牌数据,2:放王闯,3:三王闯,4:放王炸
//[游戏规则与大结算]
public:
	TZongResult						m_tZongResult;			//总战绩
	GameRuler						m_tGameRules;			//游戏规则
//回放
private:
	ofstream						m_fVideoFile;
	int								m_iVideoIndex;
	Json::Value						m_element;
	Json::Value						m_root;

private:
	BYTE							m_piaoList[PIAO_COUNT];	//漂分列表
	int								m_piaoV[PLAY_COUNT];	//漂分值(-1:未选  0:不漂)
	int								m_haveAutoCount[PLAY_COUNT];	//已托管局数

	BYTE							m_superWangNum;		//超端发牌时能拿几个王
	BYTE							m_superSetCard;		//超端设置要的牌
	BYTE							m_superSetChairID;	//拿到超端设置牌的位置
	BYTE							m_tmpCard;			//中间变量

//[函数定义]
public:
	//构造函数
	CServerGameDesk(); 
	//析构函数
	virtual ~CServerGameDesk();
	//重载函数
public:
	//初始化游戏逻辑
	virtual bool InitDeskGameStation();
	//游戏开始
	virtual bool	GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool	GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//判断是否正在游戏
	virtual bool IsPlayGame(BYTE bDeskStation);
	//游戏数据包处理函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);
	//玩家断线
	virtual bool UserNetCut(GameUserInfo *pUser);
	//玩家离开
	virtual bool UserLeftDesk(GameUserInfo* pUser);
	//桌子成功解散
	virtual void OnDeskSuccessfulDissmiss(bool isDismissMidway = true);
	// 动态加载配置文件数据
	virtual void LoadDynamicConfig();
//[重载纯虚函数
public:
	//获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation){ return true;};
	//定时器消息
	virtual bool OnTimer(UINT uTimerID);
	// 玩家请求托管
	virtual bool OnHandleUserRequestAuto(BYTE deskStation);
	// 玩家取消托管
	virtual bool OnHandleUserRequestCancelAuto(BYTE deskStation);
//[计配置函数]
public:
	//读取静态变量
	BOOL	LoadIni();	
	//根据房间ID加载底注和上限
	BOOL	LoadExtIni(int iRoomID);	
//[常用函数]
public:
	//获取下一个玩家位置
	BYTE	GetNextDeskStation(BYTE bDeskStation, bool realNext = false);
	//设置游戏规则
	void	SetGameRuler();
	//得置游戏数据
	bool	ReSetGameState();
	//统计游戏人数
	BYTE	CountPlayer();
	///清理所有计时器
	void	KillAllTimer();	
	//第一局选择庄家
	BYTE	GetFirstBanker();
//[游戏事件]
protected:
	//用户出牌
	bool OnUserOutCard(BYTE byChairID, BYTE byCardData);
	//用户操作
	bool OnUserOperateCard(BYTE byChairID, int iOperateCode, BYTE byChiKind,BYTE byData[3][3]);
	//继续命令
	bool OnUserContinueCard(BYTE byChairID);
//辅助函数
protected:
	//派发扑克
	bool DispatchCardData(BYTE byCurrentUser);
	//出牌提示
	bool SendOutCardNotify(BYTE byCurrentUser, bool bOutCard);
	//响应判断
	bool EstimateUserRespond(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch);
	//检测动作
public:
	//总动作检测
	bool CheckAction(BYTE byStation,BYTE byCard,BYTE byType, bool bWeiTi = false);
	//检测当前是否能提牌
	bool CheckTiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bTiPai);
	//检测当前是否能偎牌
	bool CheckWeiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bWeiPai);
	//检测能否胡牌
	bool CheckHuPaiAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch,bool &bPerformAction,bool bWeiPai,bool bPao,bool bChiPeng=false);
	//检测能否跑牌
	bool CheckPaoPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction);
	//检测能否吃碰胡
	bool CheckCPPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction);
	//撇跑胡
	bool CheckPiePaoHuAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch);
	//是否有王钓，王闯，三王闯、王炸胡法
	bool CheckSpecialHuAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch,bool &bPerformAction,bool bWeiPai,bool bPao);
public:
	//设置胡牌相关信息
	bool SetHuInfor(S_C_GameEnd &tGameEnd);
	//统计醒牌个数
	int  CountCPTWPXingNumber(BYTE byChairID);
	//检测龙个数
	void CheckLongNum();
	//替换一张龙牌
	void SwapOneLongCard(BYTE chairID, int LongCardIndex);
//[功能函数]
protected:
	//出牌判断
	bool IsAllowOutCard(BYTE byChairID,bool bHand=false);
	//获取提跑数量
	int GetTiPaoNum(BYTE byChairID);
	//是否有跑牌
	bool IsPaoPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,BYTE &byPaoUser);
	//检测当前是否能提牌
	bool IsTiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bTiPai);
	//检测当前是否能偎牌
	bool IsWeiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bWeiPai);
	//撇跑胡
	bool IsPiePaoHuAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch);
	//更新胡息
	bool UpdateUserHuXiCount(BYTE byChairID);
	//增加出牌数据
	void AddOutCard(BYTE byChairID,BYTE byCard);
	//删除牌
	void DeleteOutCard();
//[超时处理]
protected:
	//出牌超时
	void  AutoOutCard();
	//玩家操作超时
	void  OperOutTimer(BYTE chairID);
	//开始通玩牌  
	void  NoticBeginPlayGame();
	//设置醒牌
	void SetXingPai(BYTE byCard);
	//获取一张牌
	BYTE GetPai(BYTE byStation);
	//是否有出牌
	bool HaveOutCard(BYTE byStation);
	//胡牌优先级
	void HuPriority(BYTE byHuStation,int iUserAction[],BYTE byPri[]);
	//json存储
	void OnJson(void *pData,int iType);
	//配牌
	BYTE HandleCard();

	//写死部分规则值
	void SetFixedRule();
	//通知选漂分
	void NoticeSelectPiao();
	//选漂分请求
	void OnUserPiao(BYTE byStation, int piaoV);
	//选漂分回复
	void SendPiaoResp(BYTE byStation, int piaoV);
	//检查是否是有效漂分选项
	bool IsValidPiaoV(int piaoV);
	//检查是否所有人都已选漂分
	bool IsAllSelectPiao();
	//通知发牌
	void NoticeSendCard();
	//获取听牌提示数据
	void GetTingData(BYTE byStation, BYTE byOutPaiTing[MAX_INDEX], BYTE byOutPaiTingCards[MAX_INDEX][MAX_INDEX]);
	//获取胡牌数据
	void GetHuPaiData(BYTE byStation, BYTE byHuPai[MAX_INDEX]);
	//王隐藏
	void NoticeWangTakeOut(BYTE byStation, bool bTake, int iType = 13);
	//发送操作选项
	void SendOperateNotify(BYTE byStation, BYTE byResumeUser, BYTE byActionCard, int iOperateCode, const tagChiCardInfo &tChiCardInfo);
	//设置定时器
	bool MySetTimer(UINT uTimerID, int uElapse);
	//是否有胡操作
	bool HaveHuAction(int iUserAction);
	//胡家,坐醒玩家分醒钱逻辑(4人玩法)
	void DivideXingScore(S_C_GameEnd &tGameEnd, int xingScore);

	//超端玩家请求看门牌
	bool SuperUserLookMenPai(BYTE byDeskStation);
	//发送牌堆数据
	void SendLeftCards(BYTE byDeskStation);
	//牌堆是否有该牌
	bool IsHaveOneLeftCard(BYTE card);

	//超端玩家请求要牌
	bool SuperUserChangePai(BYTE byDeskStation, void *pData, int size);
	//超端要牌返回
	void SuperChangePaiResp(BYTE byDeskStation, int errCode);
	//超端设置发牌时拿几个王请求
	bool SuperUserGetWangReq(BYTE byDeskStation, void *pData, int size);
	//超端设置发牌时拿几个王返回
	void SuperUserGetWangResp(BYTE byDeskStation, int errCode);
	//超端发牌拿王逻辑
	void SuperGetWangLogic();

	//是否是机器人
	bool IsVirtual(BYTE byDeskStation);

};

/******************************************************************************************************/
