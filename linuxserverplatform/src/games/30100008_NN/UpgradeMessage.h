#pragma once
#pragma pack(1)
//文件名字定义

#define GAMENAME						"牛将军"
#define NAME_ID                         30100008
#define PLAY_COUNT                      8

//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
#define GAME_TABLE_NAME					"MatchTable"

//游戏开发版本
#define DEV_HEIGHT_VERSION				3								//内部开发高版本号
#define DEV_LOW_VERSION					10 								//内部开发低版本号

//游戏状态定义
#define GS_WAIT_SETGAME					0		//等待东家设置状态
#define GS_WAIT_ARGEE					1		//等待同意设置
#define GS_ROB_NT						20      //抢庄状态
#define GS_NOTE                         21		//下底注状态
#define GS_SEND_CARD					22		//发牌状态
#define GS_OPEN_CARD					23		//摆牛状态
#define GS_WAIT_NEXT					24		//等待下一盘开始 

// 游戏模式
#define GM_NIUNIU_NT                    1          // 牛牛上庄
#define GM_PERMANENT_NT                 2          // 固定庄家
#define GM_FREE_NT                      3          // 自由庄家
#define GM_SHOW_NT                      4          // 明牌抢庄
#define GM_ONE_WIN                      5          // 通比牛牛

//按注类型
#define STATE_NULL					0x00			//无状态-错误状态-
#define STATE_PLAY_GAME				0x01			//正在游戏中
#define STATE_HAVE_NOTE				0x02			//已经下注状态
#define STATE_OPEN_CARD				0x03			//已经摆牛了
#define STATE_CALL_SCORE			0x04			//记录玩家叫分

#define MAX_SHAPE_COUNT                 18              //所有牌型种类
#define SH_USER_CARD					5               //牌的张数
#define BET_NUMBER						3				//筹码数量
#define JIAO_FEN_COUNT                  5               // 叫分数(抢庄时需要)

//=================================================================================================
/********************************************************************************/
// 数据包处理辅助标识
/********************************************************************************/
#define S_C_GAME_PLAY					130				//开始游戏
#define S_C_CALL_BANKER					131				//通知叫庄
#define S_C_NOTIC_BET					132				//通知下注
#define S_C_SEND_CARD					133				//发牌
#define S_C_NOTIC_OPER					134				//通知开牌(只要有这步，就含有搓牌)
#define S_C_SEND_LAST_CARD				136				//发最后一张牌
#define S_C_GAME_FINISH					137				//游戏结束
#define S_C_NOTIC_RAND_SELECT_BANKER	138				//通知随机叫庄
#define S_C_NOTICE_SUPER_USER_SET       139             //通知超端
#define S_C_RAND_SELECT_BANKER_RESULT   140             //随机叫庄结果

#define SUB_S_GAME_END_ALL				163				//大结算

#define C_S_CALL_BANKER					50				//叫庄
#define S_C_CALL_BANKER_RESULT			150				//叫庄结果

#define C_S_USER_BET					51				//玩家下注
#define S_C_USER_BET_RESULT				151				//玩家下注结果

#define C_S_USER_OPER					52				//玩家操作牌
#define S_C_USER_OPER_RESULT			152				//玩家操作结果

#define C_S_SUPER_USER                     53           // 超端
#define S_C_SUPER_USER_RESULT              153          // 超端操作结果

#define C_S_REQUEST_HISTORY                54           // 请求上一局的历史记录
#define S_C_RESPONSE_HISTORY_RESULT        154          // 上一局的历史记录结果 

#define C_S_CARDSHAPE                      55           // 请求牌型
#define S_C_CARDSHAPE_RESULT              155           // 得到牌型结果
#define S_C_NOTIFY_ROBOT				  156           // 通知机器人
//===================================================================================================
struct S_C_NoticeSuperUser           // 通知超端
{
	BYTE   byDeskStation;             // 超端位置
	S_C_NoticeSuperUser()
	{
		byDeskStation = 255;
	}
};

struct C_S_SuperUserState
{
	BYTE    byDeskStation;           // 超端位置
	BYTE    byOperatorValue[PLAY_COUNT];              // 1表示让该位置赢, 0表示让该位置输, 不操作2
	C_S_SuperUserState()
	{
		byDeskStation = 255;
		memset(byOperatorValue, 2, sizeof(byOperatorValue));
	}
};

struct S_C_SuperUserResult
{
	BYTE    byDeskStation;            // 目标玩家位置
	BYTE    byOperator[PLAY_COUNT];   // 数组值: 2表示不改变该玩家的数据, 0表示让该玩家输, 1表示让该玩家赢
	BYTE    byOperatorResult;         // 数据说明
	// 1表示设置成功: 通比牛牛或明牌抢庄, 等待用户同意或者是等待下一局设置时生效, 其他状态下一局生效.
	// 2表示设置失败: 固定庄, 牛牛上庄, 自由抢庄)在下注状态设置时生效, 其他状态下一局生效
	// 0表示设置失败, 玩家没有做任何操作
	S_C_SuperUserResult()
	{
		byDeskStation = 255;
		memset(byOperator, 2, sizeof(byOperator));
		byOperatorResult = 0;
	}
};

/*----------------------------------------------------------*/
// 房间设置包
struct TRoomRulerStruct
{
	int iGameModel;        // 房间模式
	int iBaseScore;		   // 游戏底分
	BYTE byBaseScoreLow;   // 底分1---最小下注值
	BYTE byBaseScoreUpper; // 底分2---最大下注值
	int iBecomeNt;         // 成为庄家
	bool bSmallNiu;        // 五小牛
	bool bBombNiu;         // 炸弹牛
	bool bHuaNiu;          // 五花牛
	bool bShunNiu;        // 顺子牛
	bool bTongHuaNiu;         // 同花牛
	bool bHuLuNiu;          // 葫芦牛
	int iGuize;            // 规则
	bool bTuiZhu;          // 推注
	bool bStopJoin;        // 游戏后静止加入
	bool bStopShuffle;     // 禁止搓牌
	int iMaxQiangZhuang;   // 最大抢庄倍数
	int iShangZhuangFen;   // 上庄分数
	BYTE byFangZhu;        // 房主
	BYTE byZhiFuFangShi;   // 支付方式
	TRoomRulerStruct()
	{
		Init();
	}
	void Init()
	{
		iGameModel = 3;
		iBaseScore = 1;
		byBaseScoreLow = 1;   // 底分1---最小下注值
		byBaseScoreUpper = 2; // 底分2---最大下注值
		iBecomeNt = -1;         // 成为庄家
		bSmallNiu = true;        // 五小牛
		bBombNiu = true;         // 炸弹牛
		bHuaNiu = true;          // 五花牛
		bShunNiu = true;				 // 顺子牛
		bTongHuaNiu = true;			   // 同花牛
		bHuLuNiu = true;				  // 葫芦牛
		iGuize = 3;            // 规则
		bTuiZhu = true;          // 推注
		bStopJoin = false;       // 游戏后静止加入
		bStopShuffle = false;     // 禁止搓牌
		if (iGameModel = 4)
		{
			iMaxQiangZhuang = 4;   // 最大抢庄倍数
		}
		else 
		{
			iMaxQiangZhuang = 1;   // 最大抢庄倍数
		}
		iShangZhuangFen = 0;   // 上庄分数
		byFangZhu = 1;        // 房主
		byZhiFuFangShi = 1;   // 支付方式
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 等待东家设置状态 ）
struct GameStation_Base
{
	//游戏版本
	BYTE				iVersion;						//游戏版本号
	BYTE				iVersion2;						//游戏版本号

	BYTE				byGameStation;					//游戏状态

	//游戏信息
	BYTE				byThinkTime;					//思考时间就是出牌时间
	BYTE				byBeginTime;					//准备时间
	BYTE                byCuoPaiTime;                   //搓牌时间
	BYTE				bySendCardTime;		 			//发牌时间
	BYTE				byCallScoreTime;				//叫庄时间
	BYTE				byXiaZhuTime;					//下注时间
	BYTE				byAllCardCount;					//扑克数目
	int					iRoomBasePoint;					//房间倍数
	int					iBaseNote;						//底注
	UINT				iCardShape;						//牌型设置
	BYTE				byGameRuler;
	TRoomRulerStruct    tRoomRuler;                     // 游戏规则
	bool                bIsSuperUser[PLAY_COUNT];       // 是否是超端用户
	BYTE				byRemainingTimer;				// 余下时间

	GameStation_Base()
	{
		memset(this,0,sizeof(GameStation_Base));
	}
};
/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 等待其他玩家开始 ）
struct GameStation_WaiteAgree : public GameStation_Base
{
	bool                bUserReady[PLAY_COUNT] ;        //玩家是否已准备

	GameStation_WaiteAgree()
	{
		memset(this,false,sizeof(GameStation_WaiteAgree));
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 叫庄状态 ）
struct GameStation_RobNt : public GameStation_Base
{
	BYTE			byUserCallStation[PLAY_COUNT];		//各玩家抢庄情况	255-表示还没操作 0-表示不抢 1-表示已经抢了
	BYTE			byUserStation[PLAY_COUNT];			//各玩家状态 标记是否中途加入的
	BYTE            byUserCard[PLAY_COUNT][SH_USER_CARD];     // 叫分状态时的牌数据
	BYTE            byUserCardCount[PLAY_COUNT];
	__int64         i64UserCallScore[PLAY_COUNT];        // 玩家叫分值, 255未操作, 不存在的玩家叫-1分

	GameStation_RobNt()
	{
		memset(this,0,sizeof(GameStation_RobNt));
		memset(byUserCallStation,255,sizeof(byUserCallStation));
		memset(byUserStation,0,sizeof(byUserStation));
		memset(byUserCard, 0, sizeof(byUserCard));
		memset(byUserCardCount, 0, sizeof(byUserCardCount));
		memset(i64UserCallScore, 255, sizeof(i64UserCallScore));
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 下注状态 ）
struct GameStation_Note : public GameStation_Base
{
	BYTE			byNtStation;							//庄家位置
	int			    iUserStation[PLAY_COUNT];				//座位上是否有人, 0表示没人, 1表示有人
	__int64			i64UserNoteLimite[PLAY_COUNT][BET_NUMBER];  //玩家三个下注数字
	__int64         i64PerJuTotalNote[PLAY_COUNT];            // 玩家下注的值
	BYTE            byUserCard[PLAY_COUNT][SH_USER_CARD];     // 下注状态时的牌数据
	BYTE            byUserCardCount[PLAY_COUNT];              
	BYTE			byUserNoteStation[PLAY_COUNT];		//各玩家下注情况	0-表示未操作 1表示下注
	BYTE            byBankerCallValue;                  // 庄家叫分值      

	GameStation_Note()
	{
		byNtStation = 255;
		memset(this,0,sizeof(GameStation_Note));
		memset(iUserStation,255,sizeof(iUserStation));
		memset(i64UserNoteLimite, 0, sizeof(i64UserNoteLimite));
		memset(i64PerJuTotalNote, 0, sizeof(i64PerJuTotalNote));
		memset(byUserCardCount, 0, sizeof(byUserCardCount));
		memset(byUserCard, 0, sizeof(byUserCard));
		memset(byUserNoteStation, 0, sizeof(byUserNoteStation));
		byBankerCallValue = 0;
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 发牌状态 ）
struct GameStation_SendCard : public GameStation_Base
{
	BYTE			byNtStation;							//庄家位置
	BYTE			byUserCardCount[PLAY_COUNT];				//用户手上扑克数目
	BYTE			byUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	int				iUserStation[PLAY_COUNT];				//各玩家状态 标记是否中途加入的
	__int64			i64PerJuTotalNote[PLAY_COUNT];			//用户每局压总注
	BYTE            byBankerCallValue;                  // 庄家叫分值

	GameStation_SendCard()
	{
		memset(this,0,sizeof(GameStation_SendCard));
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 开牌状态 ）
struct GameStation_OpenCard : public GameStation_Base
{
	BYTE			byNtStation;							//庄家位置
	int				iUserStation[PLAY_COUNT];				//各玩家下注状态 -1-表示还没操作 0-表示不抢 1-表示已经抢了
	BYTE			byUserCardCount[PLAY_COUNT];				//用户手上扑克数目
	BYTE			byUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	BYTE			byOpenUnderCount[PLAY_COUNT];			//底牌张数
	BYTE			byOpenUnderCard[PLAY_COUNT][3];			//底牌的三张牌
	BYTE			byOpenUpCard[PLAY_COUNT][2];			//升起来的2张牌
	int				iOpenShape[PLAY_COUNT];					//摆牛牌型
	bool            bMingPai[PLAY_COUNT];                   //玩家是否点击了亮牌按钮
	__int64			i64PerJuTotalNote[PLAY_COUNT];			//用户每局压总注
	BYTE            byBankerCallValue;                 
	bool            bReadyOpenCard[PLAY_COUNT];	            // 已经摆牌玩家
	GameStation_OpenCard()
	{
		memset(this,0,sizeof(GameStation_OpenCard));
	}
};

//=================================================================================================
// 通知搓牌
struct S_C_CUOPAI
{
	BYTE     byDeskStation;     // 玩家搓牌位置
	bool     bCuoPaiType;       // 搓牌类型   默认为1.
	BYTE     byUserCard[SH_USER_CARD]; // 搓牌数据就是用户手牌数据
	S_C_CUOPAI()
	{
		byDeskStation = 255;
		bCuoPaiType = true;
		memset(byUserCard,0,sizeof(byUserCard));
	}
};

/*------------------------------------------------------------------------------*/
//游戏结束统计数据包
struct GameCutStruct
{
	int					bDeskStation;					//退出位置
	__int64				i64TurePoint[PLAY_COUNT];			//庄家得分
	__int64				i64ChangeMoney[PLAY_COUNT];
	BYTE                byQuitType;                      //退出类型
	GameCutStruct()
	{
		memset(this,0,sizeof(GameCutStruct));
	}
};

//
/*------------------------------------------------------------------------------*/
//游戏开始数据包
struct S_C_BeginPlayStruct
{
	BYTE		iOutDeskStation;					//出牌的位置
	bool        bIsSuperUser[PLAY_COUNT];           // 是否是超端用户
	S_C_BeginPlayStruct()
	{
		iOutDeskStation = 255;
		memset(bIsSuperUser, false, sizeof(bIsSuperUser));
	}
};

//通知叫庄
struct S_C_NoticCallBanker
{
	BYTE byCallUser;
	S_C_NoticCallBanker()
	{
		byCallUser = 255;
	}
};
/*------------------------------------------------------------------------------*/
//通知下注
struct S_C_NoticBet
{
	BYTE byBanker;						//当前庄家
	__int64  i64CanBetValue[BET_NUMBER];//能下注的筹码值,如果为0,则表法当前无此筹码,不用显示此按钮
	BYTE byBankerCallValue;             // 庄家叫分值
	S_C_NoticBet()
	{
		byBanker = 255;
		memset(i64CanBetValue,0,sizeof(i64CanBetValue));
	}
};

/*------------------------------------------------------------------------------*/
//发牌
struct S_C_SendCard
{
	BYTE byStartPos;								//发牌起始位置
	BYTE byCardData[PLAY_COUNT][SH_USER_CARD];
	BYTE byCardCount[PLAY_COUNT];	
	S_C_SendCard()
	{
		byStartPos = 0;
		memset(byCardData,0,sizeof(byCardData));
		memset(byCardCount,0,sizeof(byCardCount));
	}
};

/*------------------------------------------------------------------------------*/
//通知摆牌
struct S_C_NotOpenCard
{
	BYTE byOperUser;			//此参数不用
	S_C_NotOpenCard()
	{
		byOperUser = 255;
	}
};

//发最后一张牌
struct S_C_SendLastCard
{
	BYTE byPoint;			//此参数不用
	BYTE byCardData;
	BYTE byCardCount[PLAY_COUNT];
	S_C_SendLastCard()
	{
		byPoint = 255;
		byCardData = 255;
		memset(byCardCount,0,sizeof(byCardCount));
	}
};

//通知随机选择庄
struct S_C_NoticRandSelectBanker
{
	BYTE byBanker;
	bool bRandSelectUser[PLAY_COUNT];
	S_C_NoticRandSelectBanker()
	{
		byBanker = 255;
		memset(bRandSelectUser,false,sizeof(bRandSelectUser));
	}
};

struct S_C_GameFinish
{
	BYTE          byDownData[PLAY_COUNT][3];			//下敦
	BYTE          byUpData[PLAY_COUNT][2];				//上敦
	BYTE          byCardData[PLAY_COUNT][SH_USER_CARD];	//牌数据
	BYTE          byCardShap[PLAY_COUNT];				//牌型
	__int64       i64Money[PLAY_COUNT];				    //积分
	BYTE          byUpGradeStation;                     // 庄家位置
	BYTE          byUserState[PLAY_COUNT];              // 玩家状态
	__int64       i64TurePoint[PLAY_COUNT];               // 每个玩家与庄家的输赢值
	__int64		  i64ChangeMoney[PLAY_COUNT];
	S_C_GameFinish()
	{
		memset(this, 0, sizeof(S_C_GameFinish));
	}
};

//玩家叫庄
struct C_S_UserCallBanker
{
	BYTE  byValue;						//叫庄时点的是第几个按钮, 默认从0开始, 有5个值					
	C_S_UserCallBanker()
	{
		byValue = 255;
	}
};

//叫庄结果
struct S_C_UserCallBankerResult
{
	BYTE byValue;					//叫庄时点的是第几个按钮, 默认从0开始, 有5个值
	BYTE byCallBankerUser;					//叫庄玩家
	S_C_UserCallBankerResult()
	{
		byValue = 255;
		byCallBankerUser = 255;
	}
};

//玩家下注
struct C_S_UserBetValue
{
	BYTE byBetBtnNum;							//下注时点的是第几个按扭
	C_S_UserBetValue()
	{
		byBetBtnNum = 0;
	}
};

//玩家下注结果
struct S_C_UserBetValueResult
{
	__int64 i64BetValue;						
	BYTE byBetUser;
	S_C_UserBetValueResult()
	{
		i64BetValue = 0;
		byBetUser = 255;
	}
};

//玩家摆牌
struct C_S_UserOpenCard
{			
	BYTE byType; //预留字节
	C_S_UserOpenCard()
	{
		memset(this,0,sizeof(C_S_UserOpenCard));
	}
};

//玩家摆牌结果
struct S_C_UserOpenCardReslut
{
	BYTE byDeskStation;					//摆牌玩家				
	BYTE byDown[3];
	BYTE byUp[2];
	BYTE byDownCount;
	BYTE byTanPaiCard[SH_USER_CARD];     // 摊牌时用来保存手牌的数据
	BYTE byCardShap;
	bool bReadyOpenCard[PLAY_COUNT];	 // 已经摆牌玩家
	BYTE byBanker;

	S_C_UserOpenCardReslut()
	{
		memset(this,0,sizeof(S_C_UserOpenCardReslut));
	}
};
//-------------------------------------------------------------------------------
//总战绩
struct TZongResult	
{
	long long iBestWin[PLAY_COUNT];
	long long iMoney[PLAY_COUNT];
	TZongResult()
	{
		Init();
	}
	void Init()
	{
		memset(this,0,sizeof(TZongResult));
	}
};

// 玩家请求当前牌型
struct C_S_GetCardShape
{
	BYTE    byDeskStation;              // 请求得到牌型的玩家位置
	C_S_GetCardShape()
	{
		byDeskStation = 255;
	}
};

struct S_C_GetCardShapeResult
{
	BYTE byDeskStation;           // 请求得到牌型的玩家位置
	BYTE byCardShape;             // 返回玩家得到的牌型
	BYTE byCard3[3];
	S_C_GetCardShapeResult()
	{
		byDeskStation = 255;
		byCardShape = 255;
		memset(byCard3,0,sizeof(byCard3));
	}
};

struct S_C_Notify_Robot
{
	BYTE byDeskStation;           // 请求得到牌型的玩家位置
	S_C_Notify_Robot()
	{
		byDeskStation = 255;
	}
};
#pragma pack()

