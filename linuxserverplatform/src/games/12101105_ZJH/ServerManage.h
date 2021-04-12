#pragma once

#include "UpGradeLogic.h"

/*---------------------------------------------------------------------*/
#define TIME_SEND_CARD				30				//发牌定时器
#define TIME_BEGIN_PLAY				31				//发牌完成 开始下注
#define TIME_WAIT_ACTION			32				//等待操作
#define TIME_NOTICE_ACTION			33				//通知操作
#define TIME_GAME_FINISH			34				//游戏结束定时器
#define TIME_AUTO_FOLLOW			35				//自动跟注
#define TIME_AUTO_GIVEUP			36				//自动弃牌

#define TIME_GAME_TUOGUAN			41				//托管定时器
#define TIME_GAME_WAIT_ARGEE		43				//等待同意定时器
#define TIME_GAME_AUTO_BEGIN        42              //自动开始游戏
/*---------------------------------------------------------------------*/ 

struct	TGameBaseData
{
	//服务端配置文件配置参数
	BYTE				m_byThinkTime;			//游戏思考时间
	BYTE				m_byBeginTime;			//游戏开始时间	
	int					m_iGuoDi;				//锅底值	
	int					m_iDiZhu;				//底注     
	__int64				m_i64DingZhu;			//顶注--总下注数达到这个值 就强制开牌		
	BYTE				m_byCurrHandleDesk;		//当前操作的用户
	int					m_iBaseRatio;			//房间倍率

	//每局游戏结束需要重置的数据
	BYTE				m_byNtPeople;				//庄家位置
	bool				m_bCanLook[PLAY_COUNT];		//是否可以看牌	
	bool				m_bCanFollow;				//是否可以跟注	
	bool				m_bCanAdd[PLAY_COUNT][E_CHOUMA_COUNT];	//是否可以加注	
	bool				m_bCanOpen;					//是否可以比牌	
	bool				m_bCanGiveUp;				//是否可以弃牌	
	int					m_iUserState[PLAY_COUNT];	//记录用户状态	
	int					m_iFirstUserState[PLAY_COUNT];	//记录用户状态
	__int64				m_i64ZongXiaZhu;				//总注	
	__int64				m_i64XiaZhuData[PLAY_COUNT];	//玩家总下注数据
	BYTE				m_byUserCard[PLAY_COUNT][MAX_CARD_COUNT];		//用户手上的扑克
	BYTE				m_byUserCardCount[PLAY_COUNT];					//用户手上扑克数目
	bool				m_bFirstNote;									//是否为第一次下注
	int					m_iAddNoteTime;									//下注次数，用来给客户端判断是否可以比牌
	__int64				m_i64CurrZhuBase;								//当前加注的基数
	__int64				m_i64XiaZhuBase;								//当前下注的基数
	__int64				m_i64XiaZhuMaxBase;								//目前最大下注
	int					m_iChouMaCount[PLAY_COUNT][E_CHOUMA_COUNT];		//(每个玩家)每种筹码类型的数量

	TGameBaseData()
	{
		InitAllData();
	}
	void	InitAllData()
	{
		m_iBaseRatio = 1;
		m_byThinkTime	= 10;			
		m_byBeginTime	= 15;		
		m_iGuoDi	= 50;				
		m_iDiZhu	= 1;			
		m_i64DingZhu= 1000;					
		m_byNtPeople = 255;

		InitSomeData();
	}

	/*每局游戏结束/开始需要重置的数据*/
	void	InitSomeData()
	{
		m_byCurrHandleDesk	= 255;				
		m_bCanFollow= false;						
		m_bCanOpen	= false;				
		m_bCanGiveUp= false;
		memset(m_bCanLook,0,sizeof(m_bCanLook));
		memset(m_bCanAdd,0,sizeof(m_bCanAdd));
		memset(m_iUserState,STATE_ERR,sizeof(m_iUserState));
		m_i64ZongXiaZhu = 0;	
		memset(m_i64XiaZhuData,0,sizeof(m_i64XiaZhuData));

		memset(m_byUserCard,0,sizeof(m_byUserCard));
		memset(m_byUserCardCount,0,sizeof(m_byUserCardCount));

		m_bFirstNote	= true;
		m_iAddNoteTime	= 0;							
		m_i64CurrZhuBase= 0;
		m_i64XiaZhuBase=0;
		m_i64XiaZhuMaxBase=0;
		memset(m_iChouMaCount, 0, sizeof(m_iChouMaCount));
	}
};

//游戏桌类
class CServerGameDesk : public CGameDesk
{
private:
	CUpGradeGameLogic		m_Logic;     				//游戏逻辑
	TGameBaseData			m_TGameData;				//游戏数据
private:
	//奖池和超端控制参数
	int						m_iAIWantWinMoneyPercent;   //每张桌子机器人赢钱概率0~1000以内的值
	bool                    m_bSuperFlag[PLAY_COUNT];   //超端玩家结果，true为超端
	BYTE					m_bySuperMaxDesk;			//超端设置的最大玩家
	BYTE					m_bySuperMinDesk;			//超端设定的最小玩家
	BYTE					m_byRobot[PLAY_COUNT];		//所有机器人的座位号
	int						m_iRobotCount;				//机器人的位置
	BYTE					m_byHuman[PLAY_COUNT];		//所有真人的座位号
	int						m_iHumanCount;				//真人数量
	bool					m_bSystemWin;				//系统是否赢
private:
	BYTE			m_iTotalCard[52];			//总的牌
	int				m_iThisDiZhu;				//本局底注 
	__int64			m_i64ThisDingZhu;			//本局个人顶注 -总下注上限	达到上限 就要开牌	
public:
	int				m_OperateCount[PLAY_COUNT];			//用户本局操作的次数
	bool			m_bOpenRecord[PLAY_COUNT][PLAY_COUNT];
	bool			m_bWinPlayer[PLAY_COUNT];
	TZongResult		m_tZongResult;//总战绩
	int				m_iGameTurn;//第几轮下注
	BYTE			m_byBetStation;//当前下注玩家
	bool			m_bGuoZhuang; //是否过庄
	int				m_UserCardType[PLAY_COUNT];///牌型
	bool			m_bAuto[PLAY_COUNT];//托管
	bool			m_UserReady[PLAY_COUNT];//玩家是否准备了
	int				m_byGameAutoPlayTime;			//房卡场游戏托管时间
	int				m_ChouMaMoney[E_CHOUMA_COUNT];	//不同的积分场用不用的筹码值

	//断线重连需要用到的时间数据
	int				m_iLastTimerTime ;
	time_t			m_dwLastGameStartTime ;

	//规则
	bool    m_bBaoZiYouXi;//豹子有喜
	bool	m_bHuiErPai;//会儿牌
	bool    m_b235ShaBaoZi;//235杀豹子
	int		m_iFengDing;//封顶分数
	int     m_iAllTurnCount; //轮数 10，15,20轮
	int		m_CompareCardAdd;//比牌加倍 
	int		m_MenPai;////////焖牌 0 不焖，1 焖一轮，2 焖2轮 ,3 焖3轮
	int		m_TeSuPaiXing;////特殊牌型比较 1 顺子>同花 ;2 顺子<同花
	int     m_DiFen; /// 1分，2分，5分
	bool	m_GameAutoPlay;//游戏托管模式,超过5分钟房卡没反应就托管，弃牌

	// 记录
	bool    m_LastComparisonCard;//最后一组人比牌，不能弃牌
	bool	m_bAlreadSet;			//服务务端自用
public:
	//构造函数
	CServerGameDesk(); 
	//析构函数
	virtual ~CServerGameDesk();
public:
	//游戏开始
	virtual bool	GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool	GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//判断是否正在游戏
	virtual bool	IsPlayGame(BYTE bDeskStation);
	//游戏消息函数
	virtual bool	HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);
	//用户离开游戏桌
	virtual bool	UserLeftDesk(GameUserInfo* pUser);
	//用户断线
	virtual bool	UserNetCut(GameUserInfo *pUser);
	//初始化桌子
	virtual bool	InitDeskGameStation();
	//获取游戏状态信息
	virtual bool	OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool	ReSetGameState(BYTE bLastStation);
	//定时器消息
	virtual bool	OnTimer(UINT uTimerID);
	//大结算
	virtual void	OnDeskSuccessfulDissmiss(bool isDismissMidway = true);
	//初始化游戏数据，大结算初始化
	virtual void	InitDeskGameData();
	// 玩家请求托管
	virtual bool	OnHandleUserRequestAuto(BYTE deskStation);
	// 玩家取消托管
	virtual bool	OnHandleUserRequestCancelAuto(BYTE deskStation);
	// 动态加载配置文件数据
	virtual void	LoadDynamicConfig();

private:
	//响应超端设定
	void	OnHandleSuperSet(BYTE bDeskStation,C_S_SuperUserSet *pSuperUserSet);
	//收到客户端发来的玩家看牌消息
	void	OnHandleUserLookCard(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家跟注消息
	void	OnHandleUserFollow(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家加注消息
	void	OnHandleUserAddNote(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家弃牌消息
	void	OnHandleUserGiveUp(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家比牌消息
	void	OnHandleUserOpenCard(BYTE byDeskStation,void * pData);
public:
	//读取静态变量
	void	LoadIni();	
	//根据房间ID加载底注和上限
	BOOL	LoadExtIni(int iRoomID);
	//发送扑克给用户
	BOOL	SendCard();
	//游戏开始
	BOOL	BeginPlayUpGrade();
	//通知玩家下注
	void	NoticeUserAction();
	//通知玩家操作
	void	NoticeUserOperate(BYTE byDeskStation, BYTE type);
	///机器人输赢自动控制,
	void	AiWinAutoCtrl();

public:
	//自动弃牌
	void	AutoGiveUp(BYTE byDeskStation = 255);
	//自动比牌
	void	AutoBiPai();
	//自动跟注
	void	AutoFollow();
	//检测是否结束
	bool	CheckFinish();
	//获取最终胜利的玩家
	BYTE	GetFinalWiner();
	//统计游戏人数
	BYTE	CountPlayer();
	//得到下一个出牌玩家的位置
	BYTE	GetNextNoteStation(BYTE bCurDeskStation);
	//返回最小金币玩家的金币
	__int64		GetMinMoney();
	//获取剩余时间
	int GetGameRemainTime();
	//能否下为主
	bool	CanXiZhu();
	//获取玩家当前的金币
	__int64	GetUserMoney();
	//判断玩家身上资源是否只够比牌
	bool OnlyEnoughOpenCard();
private:
	//验证是否超端
	void	SpuerProof(BYTE byDeskStation);
	//执行超端设置
	void    OnSpuerResult();
	//交换两个人的牌
	void	Change2UserCard(BYTE byFirstDesk,BYTE bySecondDesk);
public:
	//房间规则解析
	void GetSpecialRule();
public:
	//值转换下标
	int ValueChangeIndex(int iValue);
private:
	//获取游戏底分
	int GetGameBasePoint();
	//配牌
	bool BrandCard();
};

/******************************************************************************************************/
