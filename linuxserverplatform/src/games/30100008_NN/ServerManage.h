#pragma once
#include "UpGradeLogic.h"

#define SAFECHECK_STRUCT(A,StructType,pData,iSize)\
	if(sizeof(StructType) != iSize)			      \
	return true;							          \
	StructType *A = (StructType*)pData;		      \
	if(nullptr == A)							  \
	return true;							          \


//定时器 ID
#define TIME_CALL_NT				30				// 叫庄
#define TIME_NOTE                   31              // 下注计时器
#define TIME_NOTE_FINISH			32				// 下注完成定时器
#define TIME_SEND_CARD_FINISH		33				// 发牌定时器
#define TIME_AUTO_BULL				34				// 自动摆牛计时器
#define TIME_GAME_FINISH			35				// 游戏结束定时器
#define TIME_OUT_CARD               36              // 出牌
#define TIME_RAND_BANKER            37              // 随机选择庄
#define TIME_REMAIN_TIMER           40              // 剩余时间计时器
#define TIME_GAME_COUNT_FINISH		41              // 延时游戏结束
#define TIME_GAME_AUTO_BEGIN		42              // 自动开始游戏
#define TIME_GAME_TUOGUAN			44				// 托管定时器

//游戏桌类
class CServerGameDesk : public CGameDesk
{
private:
	/*---------------------------游戏可配置参数----------------------------------*/
	BYTE				m_byBeginTime;			//游戏开始时间
	BYTE				m_byThinkTime;			//游戏摆牛思考时间
	BYTE                m_byJiaoZhuangTime;     // 叫庄时间
	BYTE				m_byCallScoreTime;		//抢庄时间
	BYTE				m_byXiaZhuTime;			//下注时间
	BYTE                m_bySendCardTime;       // 发牌时间
	BYTE                m_byOutCardTime;        // 出牌时间
	BYTE                m_byRemainingTimer;     // 剩余时间
	BYTE				m_byTuoGuanTime;		// 托管时间
	// 游戏倍率
	BYTE				m_byCardShapeBase[MAX_SHAPE_COUNT];//0-10代表无牛到牛牛间的倍率
protected:
	/*---------------------------游戏基础数据----------------------------------*/
	CUpGradeGameLogic		m_Logic;				//游戏逻辑
	TRoomRulerStruct        m_tRoomRuler;           // 房间规则
	bool					m_bAlreadSet;			//服务务端自用
	int                     m_iOutCardPeople;       // 现在出牌的用户
	bool					m_bNetCut[PLAY_COUNT];
	BYTE					m_iAllCardCount;		//游戏所用的牌数
	BYTE					m_iTotalCard[52];		//总的牌
	int						m_bTurnRule;			//游戏顺序 0-顺时针 1-逆时针
	int						m_iGoodCard;			//发大牌机率牛七以上
	bool					m_bHaveKing;			//是否有王
	UINT					m_iCardShape;			//牌型

	/*---------------------------游戏基础数据每局需要重置的数据----------------*/
	BYTE					m_byUpGradePeople;					//庄家位置	
	BYTE					m_byCurrOperateUser;				//当前操作的玩家	
	BYTE					m_byUserStation[PLAY_COUNT];		//记录用户状态
	bool                    m_bUserReady[PLAY_COUNT];
	BYTE					m_byUserCallStation[PLAY_COUNT];
	bool					m_bTuoGuan[PLAY_COUNT];				//玩家是否托管

	/*---------------------------叫分数据相关----------------*/
	__int64                 m_i64CallValue[PLAY_COUNT][JIAO_FEN_COUNT];     // 每个玩家可以出现的叫分值
	__int64                 m_i64CallScoreMaxValue;                         // 叫分最大值
	int						m_iCallScore[PLAY_COUNT];				//玩家抢庄状态 -1表示还未操作，0表示不抢庄 1-表示抢庄
	__int64					m_i64UserCallScore[PLAY_COUNT];		    //保存每个玩家的叫分值
	__int64                 m_i64JiaoFenValue[JIAO_FEN_COUNT];      //叫分值
	bool					m_bCallScoreFinish;						//是否叫分完成

	/*---------------------------下注数据相关----------------*/
	__int64					m_i64PerJuTotalNote[PLAY_COUNT];					//用户每局压总注
	__int64					m_i64UserNoteLimite[PLAY_COUNT][BET_NUMBER];	    //玩家三个下注数字
	__int64					m_i64NoteValue[PLAY_COUNT][BET_NUMBER];	            //玩家能下注的筹码值
	__int64                 m_i64LastNoteValue[PLAY_COUNT];                     //上一轮玩家的下注值, 推注需要用到的数据
	BYTE                    m_byUserNoteStation[PLAY_COUNT];                    // 用户下注情况

	/*----------推注数据相关----------*/
	vector<__int64>			m_vWinMoney[PLAY_COUNT];            // 上一局闲家的输赢情况
	BYTE					m_byCurTurnNote[PLAY_COUNT];		// 本轮轮玩家下注值,取1,2,3
	BYTE					m_byLastTurnNote[PLAY_COUNT];       // 上一轮玩家下注值 取1,2,3
	BYTE                    m_byLastTurnBanker;                 // 上一轮的庄家位置
	int                     m_iShangJuQiangZhuang;              // 上一把抢庄倍数

	/*---------------------------摆牌数据相关----------------*/
	bool                    m_bReadyOpenCard[PLAY_COUNT];           // 已经摆牌的玩家个数
	BYTE					m_byOpenUnderCount[PLAY_COUNT];		//底牌张数
	BYTE					m_byOpenUnderCard[PLAY_COUNT][3];	//底牌的三张牌
	BYTE					m_byOpenUpCard[PLAY_COUNT][2];		//升起来的2张牌
	BYTE                    m_byBullCard[PLAY_COUNT][3];         //玩家摆的牛牌 
	BYTE					m_iLaserUserOpenShap[PLAY_COUNT];	//上一局牌型(用于轮庄模式)
	BYTE					m_byOpenShape[PLAY_COUNT];          // 玩家摆拍类型
	bool					m_bMingPai[PLAY_COUNT];             // 玩家是否摆过牌

	/*---------------------------结算数据相关----------------*/
	TZongResult				m_tZongResult;			//总战绩

	/*---------------------------扑克数据相关----------------*/
	BYTE					m_byUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE					m_byUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	BYTE                    m_byLastCard[PLAY_COUNT];   // 用户最后这一张手牌

	/*---------------------------超端控制用到的数据----------------*/
	bool					m_bSuperResult;
	BYTE					m_bySuperWinStation;
	BYTE					m_bySuperFailStation;
	bool                    m_bSuperFlag[PLAY_COUNT];          // 超端玩家结果
	BYTE					m_byRobot[PLAY_COUNT];		//所有机器人的座位号
	int						m_iRobotCount;				//机器人的位置
	BYTE					m_byHuman[PLAY_COUNT];		//所有真人的座位号
	int						m_iHumanCount;				//真人数量
	int						m_iAIWantWinMoneyPercent;   //每张桌子机器人赢钱概率，0~1000以内的值
	int						m_iHumanFailMoneyPercent;   //每张桌子人类输钱概率,0~1000以内的值
	int						m_iGiveHumanMoneyPercent;   //赠送玩家金币概率,0~1000以内的值
	int						m_iRobotGetMoneyPercent;	//机器人贪钱概率，0~1000
	bool					m_bSystemWin;				//系统是否赢
	BYTE					m_byRobotDesk;		//随便找一个机器人的位置
	BYTE					m_byUserDesk;		//随便找一个真实玩家的位置
	bool                    m_bIsRobot[PLAY_COUNT];     //是否是机器人

	/*---------------------------请求查看上局历史记录用到的数据----------------*/
	BYTE                    m_byLastPlayerCount;               // 上一局的游戏人数
	BYTE                    m_byLastUserCard[PLAY_COUNT][5];   // 上一局玩家牌的数据
	__int64                 m_i64LastTurePoint[PLAY_COUNT];
	BYTE                    m_byLastCallScore;                 // 上轮玩家最大叫庄喊的分数

	/*------------------------金币场用到的数据----------------------*/
	//;游戏模式
	int m_iGameModel;
	//确定玩家可下注的值
	int m_Difen;
	//	;牌型倍率设置
	int m_Guize;
	//	;是否有五花牛
	int m_Tesu_1;
	//	;是有炸弹牛
	int m_Tesu_2;
	//	;是否有五小牛
	int m_Tesu_3;
	//	;是否有顺子牛
	int m_Tesu_4;
	//	;是否有同花牛
	int m_Tesu_5;
	//	;是否有葫芦牛
	int m_Tesu_6;
	//	;闲家是否可以推注
	int m_Tz;
	//	;游戏后是否禁止加入
	int m_Stopjoin;
	//	;是否禁止搓牌
	int m_Jzcp;
	//	;庄家初始化分数
	int m_Shangzhuan;
	//	;抢庄个数
	int m_Qiangzhuan;

	//临时数据
	vector<int> m_vecIntTemp;
	//函数定义
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
	virtual bool GameBegin(BYTE bBeginFlag);

	//游戏结束
	virtual bool GameFinish(BYTE bDeskStation, BYTE bCloseFlag);

	//判断是否正在游戏
	virtual bool IsPlayGame(BYTE bDeskStation);

	//游戏数据包处理函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);

	//断线
	virtual bool UserNetCut(GameUserInfo *pUser);

	// 玩家离开桌子，直接返回大厅
	virtual bool UserLeftDesk(GameUserInfo* pUser);

	//获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);

	//重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);

	//定时器消息
	virtual bool OnTimer(UINT uTimerID);

	//桌子成功解散
	virtual void OnDeskSuccessfulDissmiss(bool isDismissMidway = true);

	//初始化游戏数据，大结算初始化
	virtual void InitDeskGameData();

	// 玩家请求托管
	virtual bool OnHandleUserRequestAuto(BYTE deskStation);

	// 玩家取消托管
	virtual bool OnHandleUserRequestCancelAuto(BYTE deskStation);

	// 动态加载配置文件数据
	virtual void LoadDynamicConfig();

	//处理函数
public:
	//读取静态变量
	int	LoadIni();

	//根据房间ID加载底注和上限
	int	LoadExtIni(int iRoomID);

	//统计游戏人数
	int	CountPlayer();

	//统计当前还未下注的玩家个数
	BYTE CountNoNotePeople();

	//统计当前还未下注的玩家个数
	BYTE CountNoCallScorePeople();

	//计算各家分数
	int	ComputePoint(BYTE DeskStation);

	//获取下一个玩家位置
	BYTE GetNextDeskStation(BYTE bDeskStation);

	//处理用户叫庄结果
	int	UserCallScoreResult(BYTE bDeskStation, BYTE byBetBtnNum);

	//叫分结束
	int	CallScoreFinish();

	//通知用户下注
	int	NoticeUserNote();

	//处理用户下注结果, byBetBtnNum从1开始
	int	UserNoteResult(BYTE bDeskStation, BYTE byBetBtnNum);

	//发送扑克给用户
	int	SendCard();

	//发最后一张牌
	bool SendLastCard();

	//发牌结束
	int	SendCardFinish();

	// 通知用户叫庄
	int NoticeUserScore();

	//开牌
	int	BeginOpenCard();

	//玩家摆牛
	bool UserOpenCard(BYTE byDeskStation, void * pData, int iSize);

	//检测是否结束
	int	CheckFinish();

	///清理所有计时器
	void KillAllTimer();

	//自动摆牛
	void AutoBull(BYTE byDeskStation);

	// 玩家请求当前牌型
	bool GetUserCardShape(BYTE byDeskStation);

	// 超端设置, 返回值1设置成功, 0表失败
	bool SuperSetResult(BYTE byStation);

	//解析规则
	void SetGameRuler();

	//配牌
	bool BrandCard();

	//获取推注值
	//iQiang抢庄，iBaseScore上一局下注底分，iCardShape上一局牌型，iReserve预留
	void GetTuiZhu(long long &iTuiZhuScore, long long &iTuiZhuScoreEx, int iQiang, long long iBaseScore, BYTE iCardShape, int iReserve = 0);

	/////////////////////////////////////////奖值/////////////////////////////////
	//交换两个人的牌
	void Change2UserCard(BYTE byFirstDesk, BYTE bySecondDesk);

	//改变两个玩家的手牌
	bool Change2UserCardEx(BYTE byFirstDesk, BYTE bySecondDesk);

	///机器人输赢自动控制,
	bool AiWinAutoCtrl();

	//抢庄模式下控制输赢，主要控制明牌抢庄
	void AiRobNtAutoCtrl(bool bMingPai);

	//回收赢金币数量很多的玩家
	void AutoRecoverMoney();

	//让符合一定条件的玩家赢钱
	void AutoGiveMoney();

	//这个玩家是否满足回收条件
	bool IsRecoverPlayer(BYTE byDeskStation);

	//这个玩家是否满足赢钱条件
	bool IsWinMoneyPlayer(BYTE byDeskStation);

	//所有牌中只找最大
	void FindMax(BYTE &byTmpMax);

	//在一组数据中，计算这组数据的牌的大小
	void FindAllSize(BYTE byDeak[], int iCount, int iAllSize[]);

	//从一组数据中计算庄家得分
	void CountLocal(BYTE byDeak[], int iCount, int &iWinScore);
	//////////////////////////////////////////////////////////////////////////
};
