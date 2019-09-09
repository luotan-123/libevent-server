#pragma once
//运营版经过编译优化，无法调试但是运行速度更快
#pragma pack(1)
//文件名字定义
#define GAMENAME						TEXT("潮汕斗地主")
#define NAME_ID							36610103								//名字 ID

//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
//游戏内部开发版本号
#define DEV_HEIGHT_VERSION				2				//内部开发高版本号(每修正一次LOW_VERSIONG+1)
#define DEV_LOW_VERSION					0				//内部开发低版本号

//游戏信息
#define PLAY_COUNT						3								//游戏人数
#define SKIN_CARD						TEXT(".\\image\\cardskin")		//共用图片文件夹
static TCHAR szTempStr[MAX_PATH] = { 0 };//暂存字符串，用来获取 dll_name 等的时候使用
#define GET_CLIENT_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".ico"))
#define GET_SERVER_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".dll"))
#define GET_CLIENT_BCF_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".bcf"))

//游戏状态定义
#define GS_WAIT_SETGAME					0				//等待东家设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_SEND_CARD					20				//发牌状态
#define GS_WAIT_BACK					21				//等待扣压底牌
#define GS_PLAY_GAME					22				//游戏中状态.
#define GS_WAIT_NEXT					23				//等待下一盘开始 

#define GS_FLAG_NORMAL					0				//正常情况
#define GS_FLAG_CALL_SCORE				1				//叫分
#define GS_FLAG_ROB_NT					2				//抢地主
#define GS_FLAG_ADD_DOUBLE				3				//加倍
#define GS_FLAG_SHOW_CARD				4				//亮牌
#define GS_FLAG_PLAY_GAME               5 

//-------------------------------------------------------------------------------
/********************************************************************************/
// 数据包处理辅助标识
//warning:79号ID预留给超级客户端,永不使用
/********************************************************************************/
#define ASS_GAME_BEGIN					51				//游戏开始
#define ASS_SEND_CARD					53				//发牌信息
#define ASS_SEND_CARD_MSG				54				//发牌过程中处理消息
#define ASS_SEND_ALL_CARD				55				//发送所有牌(一下发放全部)
#define ASS_SEND_FINISH					56				//发牌完成
#define ASS_CALL_SCORE					57				//叫分
#define ASS_CALL_SCORE_RESULT			58				//叫分结果
#define ASS_CALL_SCORE_FINISH			59				//叫分结束

#define ASS_ROB_NT						61				//抢地主
#define ASS_ROB_NT_RESULT				62				//抢地主结果
#define ASS_GAME_MULTIPLE				64				//游戏倍数(抢地主后会加倍)
#define ASS_ROB_NT_FINISH				65				//抢地主结果

#define ASS_BACK_CARD					66				//底牌数据
#define ASS_BACK_CARD_EX				67				//扩展底牌数据(其他玩家可见)
#define ASS_ADD_DOUBLE					68				//通知加倍
#define ASS_ADD_DOUBLE_RESULT			69				//加倍结果
#define ASS_ADD_DOUBLE_FINISH			71				//加倍结束
#define ASS_SHOW_CARD					72				//亮牌
#define ASS_SHOW_CARD_RESULT			73				//亮牌结果
#define ASS_SHOW_CARD_FINISH			75				//亮牌结束
#define ASS_GAME_PLAY					76				//开始游戏
#define ASS_OUT_CARD					77				//用户出牌
#define ASS_OUT_CARD_RESULT				78				//出牌結果
#define	ASS_SUPER_USER					79				//超端消息
#define ASS_REPLACE_OUT_CARD			80				//代替出牌(79留给超级客户端发牌器)
#define ASS_ONE_TURN_OVER				81				//一轮完成(使客户端上一轮可用)
#define ASS_NEW_TURN					82				//新一轮开始
#define ASS_AWARD_POINT					83				//奖分(炸弹火箭)
#define ASS_CONTINUE_END				84				//游戏结束
#define ASS_NO_CONTINUE_END				85				//游戏结束
#define ASS_NO_CALL_SCORE_END			86				//无人叫分
#define ASS_CUT_END						87				//用户强行离开
#define ASS_SAFE_END					88				//游戏安全结束
#define ASS_TERMINATE_END				89				//意外结束
#define ASS_AHEAD_END					90				//提前结束
#define ASS_AUTO						91				//用户托管
#define ASS_HAVE_THING					92				//有事
#define ASS_LEFT_RESULT					93				//有事离开结果
#define ASS_AI_STATION					95				//机器人托管(断线户用)
#define ASS_USER_LEFTDESK               97              //玩家离开桌子或断线

#define S_C_UPDATE_CALCULATE_BOARD_SIG	98				//更新结算榜
#define S_C_UPDATE_REMAIN_JUSHU_SIG		99				//更新剩余局数

#define S_C_ROBOT_CARD					100				//机器人专属消息
#define S_C_OUT_CARD_RESULT				109             //出牌结果(主要出错)

const  int  MAX_TASK_TYPE = 4;  ///任务最大种类 
const  int  MAX_CARD_SHAPE = 8;  ///牌型最大种类
const  int  MAX_CARD_TYPE = 15; ///牌种类

///底牌类型
enum BackCardType
{
	TYPE_NONE = 0,//什么牌型都不是
	TYPE_DOUBLE_CARD = 10,
	TYPE_SAME_HUA = 11,
	TYPE_SMALL_KING = 12,
	TYPE_BIG_KING = 13, //大王
	TYPE_TRIPLE_CARD = 14,
	TYPE_STRAIT = 15,
	TYPE_ROCKET = 16
};

///任务类型
enum LastTaskType
{
	TYPE_LAST_NONE = 0,    ///无任何牌型 
	TYPE_HAVE_A_CARD = 100,  ///有某张牌
	TYPE_SOME_SHAPE = 101,  ///有某种牌型
	TYPE_SINGLE_SOME_CARD = 102,  ///打的某张牌
	TYPE_DOUBLE_SOME_CARD = 103   ///打的一对某种牌
};

///游戏任务结构
struct GameTaskStruct
{
	BYTE     byBackCardType;
	BYTE     byTaskType;
	BYTE     bySpecifyShape;
	BYTE     bySpecifyCard;
	BYTE     byBackCardMutiple;  ///底牌倍数

	GameTaskStruct()
	{
		memset(this, 0, sizeof(GameTaskStruct));
	}
	void IniData(void)
	{
		memset(this, 0, sizeof(GameTaskStruct));
	}
};

///游戏中的倍数
struct  GameMutipleStruct
{
	int       sBaseMutiple;                 ///游戏中的基本底注
	int       sBackCardMutiple;             ///底牌倍数
	int       sBombCount;                   ///炸弹倍数
	int       sSprintMutiple;               ///春天
	int       sCardShapeMutiple;           ///牌型倍数（一般指任务中的倍数）
	BYTE      sAddGameMutiple[PLAY_COUNT];  /// 游戏中加倍
	BYTE      sRobNtMutiple[PLAY_COUNT];    ///抢地主倍数
	BYTE      sMingPaiMutiple[PLAY_COUNT];  ///每个人的明牌倍数
	GameMutipleStruct()
	{
		sBaseMutiple = 1;
		sBackCardMutiple = 1;
		sBombCount = 0;
		sSprintMutiple = 1;
		sCardShapeMutiple = 100;

		memset(sAddGameMutiple, 0, sizeof(sAddGameMutiple));
		memset(sRobNtMutiple, 0, sizeof(sRobNtMutiple));
		memset(sMingPaiMutiple, 0, sizeof(sMingPaiMutiple));
	};
	void  IniData(int  iBaseMutiple)
	{
		sBaseMutiple = iBaseMutiple;
		sBombCount = 0;
		sBackCardMutiple = 1;
		sSprintMutiple = 1;
		sCardShapeMutiple = 100;

		memset(sAddGameMutiple, 0, sizeof(sAddGameMutiple));
		memset(sRobNtMutiple, 0, sizeof(sRobNtMutiple));
		memset(sMingPaiMutiple, 0, sizeof(sMingPaiMutiple));
	}
	///获取明牌最大倍数
	int  GetMingMaxMutiple(void)
	{
		int iMingMutiple = max(max(sMingPaiMutiple[0], sMingPaiMutiple[1]), sMingPaiMutiple[2]);

		return (iMingMutiple > 0 ? iMingMutiple : 1);
	}
	///获取器抢地主倍数
	int  GetRobNtMutiple(void)
	{
		int  iRobMutiple = 0;
		int  iRobCount = 0;

		for (int i = 0; i < PLAY_COUNT; i++)
		{
			iRobCount += sRobNtMutiple[i];
		}

		iRobMutiple = pow(2.0, iRobCount);

		return iRobMutiple;
	}
	///获取公共倍数
	int  GetPublicMutiple(void)
	{
		int  iBombMutiple = pow(2.0, sBombCount);
		sBackCardMutiple = 1;
		int  iGameMutiple = sBaseMutiple*sBackCardMutiple*iBombMutiple*sSprintMutiple*sCardShapeMutiple*GetRobNtMutiple()*GetMingMaxMutiple() / 100;
		return iGameMutiple;
	}
	//获取炸弹倍数
	int GetBombMutiple()
	{
		int  iBombMutiple = pow(2.0, sBombCount);
		int  iGameMutiple = sBaseMutiple*iBombMutiple;
		return iGameMutiple;
	}
};

/********************************************************************************/
//游戏数据包
/********************************************************************************/
//游戏状态数据包	（ 等待其他玩家开始 ）
struct GameStation_2
{
	BYTE				iVersion;						//游戏版本号
	BYTE				iVersion2;						//游戏版本号
	bool                bUserReady[PLAY_COUNT];        ///玩家是否已准备

	BYTE				iBeginTime;						//开始准备时间
	BYTE				iThinkTime;						//出牌思考时间
	BYTE				iCallScoreTime;					//叫分计时
	BYTE                iRobNTTime;                    //抢地主时间
	BYTE				iAddDoubleTime;					//加倍时间

	int                 iGameMutiple;             ///游戏中的倍数
	int                 iAddDoubleLimit;             //加倍限制
	int                 iGameMaxLimit;              ///游戏最大输赢

	DWORD				iCardShape;						//牌型设置
	//游戏倍数
	UINT				iDeskBasePoint;					//桌面基础分
	UINT				iRoomBasePoint;					//房间倍数
	__int64				iRunPublish;					//逃跑扣分

	int					iFengDing;//封顶倍数（炸弹）
	int                 iRunTime;//单位秒
	int                 iRemainRunTime; //单位秒

	GameStation_2()
	{
		memset(this, 0, sizeof(GameStation_2));
	}
};

//游戏状态数据包	（ 等待扣押底牌状态 ）
struct GameStation_3
{
	BYTE				iVersion;						//游戏版本号
	BYTE				iVersion2;						//游戏版本号

	BYTE				iBackCount;						//底牌数
	BYTE				iBeginTime;						//开始准备时间
	BYTE				iThinkTime;						//出牌思考时间
	BYTE                iRobNTTime;                    //抢地主时间
	BYTE				iCallScoreTime;					//叫分计时
	BYTE				iAddDoubleTime;					//加倍时间

	BYTE				iCallScorePeople;				//当前叫分人
	BYTE				iGameFlag;						//叫分标记
	BYTE				iCallScoreResult;				//所叫的分

	int					iUpGradePeople;					//庄家位置
	int                 iCurOperator;                 ///当前操作的人

	UINT				iDeskBasePoint;					//桌面基础分
	UINT				iRoomBasePoint;					//房间倍数

	DWORD				iCardShape;						//牌型设置
	int                 iGameMutiple;             ///游戏中的倍数
	int                 iAddDoubleLimit;             //加倍限制
	int                 iGameMaxLimit;              ///游戏最大输赢

	bool				bAuto[PLAY_COUNT];				//托管情况
	bool				bCanleave[PLAY_COUNT];			//能否点退出
	int					iCallScore[PLAY_COUNT];			//几家叫分情况
	int 				iRobNT[PLAY_COUNT];				//抢地主情况
	int                 iUserDoubleValue[PLAY_COUNT];  ///玩家加倍情况
	BYTE				iUserCardCount[PLAY_COUNT];		//用户手上扑克数目
	BYTE				iUserCardList[195];				//用户手上的扑克
	BYTE                iGameBackCard[12];                 ///底牌
	BYTE 				iBackCardCount;						//底牌数量
	__int64				iRunPublish;					//逃跑扣分

	GameMutipleStruct   gameMutiple;                      ///游戏倍数  
	GameTaskStruct      gameTask;                         ///游戏任务

	int					iFengDing;//封顶倍数（炸弹）
	int                 iRunTime;//单位秒
	int                 iRemainRunTime; //单位秒

	BYTE                byJiaoFenSFTime;                //叫分剩余时间
	bool				bIsCall[3];						//标记1,2,3分是否被叫
	GameStation_3()
	{
		memset(this, 0, sizeof(GameStation_3));
	}

};

//游戏状态数据包	（ 游戏中状态 ）
struct GameStation_4
{
	bool				bIsLastCard;						//是否有上轮数据

	BYTE				iVersion;							//游戏版本号
	BYTE				iVersion2;							//游戏版本号
	BYTE				iBackCount;							//底牌数

	BYTE				iBeginTime;							//开始准备时间
	BYTE				iThinkTime;							//出牌思考时间
	BYTE                iRobNTTime;                    //抢地主时间
	BYTE				iCallScoreTime;						//叫分计时
	BYTE				iAddDoubleTime;						//加倍时间

	BYTE				bIsPass;							//是否不出

	int					iRunPublish;						//逃跑扣分	
	int					iBase;								//当前炸弹个数
	int					iUpGradePeople;						//庄家位置

	int 				iCallScoreResult[PLAY_COUNT];		//叫分结果
	int					iOutCardPeople;						//现在出牌用户
	int					iFirstOutPeople;					//先出牌的用户
	int					iBigOutPeople;						//先出牌的用户

	UINT				iDeskBasePoint;						//桌面基础分
	UINT				iRoomBasePoint;						//房间倍数

	int                 iGameMutiple;             ///游戏中的倍数
	DWORD				iCardShape;							//牌型设置
	int                 iAddDoubleLimit;             //加倍限制
	int                 iGameMaxLimit;              ///游戏最大输赢

	BYTE				iAwardPoint[PLAY_COUNT];			//奖分
	BYTE				iPeopleBase[PLAY_COUNT];			//加倍
	BYTE				iRobNT[PLAY_COUNT];					//抢地主


	bool				bAuto[PLAY_COUNT];					//托管情况
	bool				bCanleave[PLAY_COUNT];				//能否点退出

	BYTE				iUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE				iUserCardList[195];					//用户手上的扑克

	BYTE				iBaseOutCount;						//出牌的数目
	BYTE                iBaseCardList[45];                //桌面上的牌

	BYTE				iDeskCardCount[PLAY_COUNT];			//桌面扑克的数目
	BYTE                iDeskCardList[PLAY_COUNT][45];    ///桌面上的牌

	BYTE				iLastCardCount[PLAY_COUNT];			//上轮扑克的数目
	BYTE				iLastOutCard[PLAY_COUNT][45];		//上轮的扑克

	bool                bPass[PLAY_COUNT];                //不出
	bool                bLastTurnPass[PLAY_COUNT];         //上一轮不出

	BYTE                iGameBackCard[12];                 ///底牌
	BYTE 				iBackCardCount;						//底牌数量

	GameMutipleStruct   gameMutiple;                      ///游戏倍数  
	GameTaskStruct      gameTask;                         ///游戏任务

	int					iFengDing;//封顶倍数（炸弹）
	int                 iRunTime;//单位秒
	int                 iRemainRunTime; //单位秒

	BYTE                byOutCardSFTime;                //出牌剩余时间
	GameStation_4()
	{
		memset(this, 0, sizeof(GameStation_4));
	}
};

//用户出牌数据包 （发向服务器）
struct CMD_C_OutCard
{
	int					iCardCount;						//扑克数目
	BYTE				iCardList[45];					//扑克信息
	CMD_C_OutCard()
	{
		memset(this, 0, sizeof(CMD_C_OutCard));
	}
};

//出牌结果
struct CMD_S_OutCard
{
	BYTE byCurOutCardStation;   //当前出牌玩家
	BYTE byCardCount;			//出扑克数目
	BYTE byCardList[45];		//扑克信息
	BYTE byHandCardCount;       //手牌数量
	BYTE byHandCard[45];		//出牌之后玩家手牌
	CMD_S_OutCard()
	{
		byCurOutCardStation = 255;
		byCardCount = 0;
		byHandCardCount = 0;
		memset(byCardList, 0, sizeof(byCardList));
		memset(byHandCard, 0, sizeof(byHandCard));
	}
};

//通知出牌
struct CMD_S_NotifyOutCard
{
	BYTE byNextDeskStation;//下一出牌者
	CMD_S_NotifyOutCard()
	{
		byNextDeskStation = 255;
	}
};

//新一轮
struct NewTurnStruct
{
	BYTE				bDeskStation;					//坐号
	BYTE				bReserve;						//保留
};

///明牌开始
struct  UserMingStruct
{
	bool    bStart;       ///是否为开始
	bool    bMing;         ///玩家是否明牌
	BYTE    bDeskStaion;   ///玩家的位置
	BYTE    byCardCount;   ///明牌时玩家扑克的数量
};

//游戏开始
struct	GameBeginStruct
{
	BYTE				iPlayLimit;							//游戏总局数
	BYTE				iBeenPlayGame;						//已经玩了多少局
	BYTE                byUserMingBase[PLAY_COUNT];        ///玩家明牌倍数情况
	DWORD				iCardShape;							//牌型设置

	GameBeginStruct()
	{
		memset(this, 0, sizeof(GameBeginStruct));
	}
};

//发牌数据包
struct	SendCardStruct
{
	BYTE				bDeskStation;							//玩家
	BYTE				bCard;									//牌标号
};

//发送所有牌数据
struct	SendAllStruct
{
	BYTE             iUserCardCount[PLAY_COUNT];		//发牌数量
	BYTE             iUserCardList[108];				//发牌队例

	SendAllStruct()
	{
		memset(this, 0, sizeof(SendAllStruct));
	}
};

//发牌结束
struct	SendCardFinishStruct
{
	BYTE		bReserve;
	BYTE		byBackCardList[12];						//底牌数据
	BYTE		byUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE		byUserCard[PLAY_COUNT][45];				//用户手上的扑克
	SendCardFinishStruct()
	{
		memset(this, 0, sizeof(SendCardFinishStruct));
	}
};

//底牌数据包
struct BackCardExStruct
{
	BYTE				iGiveBackPeople;				//底牌玩家
	BYTE				iBackCardCount;					//扑克数目
	BYTE				iBackCard[12];					//底牌数据

	GameTaskStruct      gameTask;                      //游戏任务  
	BackCardExStruct()
	{
		memset(this, 0, sizeof(BackCardExStruct));
	}
};

//游戏开始数据包
struct BeginPlayStruct
{
	BYTE				iOutDeskStation;				//出牌的位置
};

//叫分数据包
struct CallScoreStruct
{
	BYTE				bDeskStation;		//当前叫分者
	int					iValue;				//叫分类型
	bool 				bCallScoreflag;		//叫分标记			
	BYTE                byFirstCard;        //叫分牌
	bool                bIsCall[3];			//标记1,2,3分是否被叫
	CallScoreStruct()
	{
		bDeskStation = 255;
		iValue = -1;
		bCallScoreflag = false;
		byFirstCard = 0;
		memset(bIsCall, 0, sizeof(bIsCall));
	}
};

//抢地主
struct	RobNTStruct
{
	BYTE   byDeskStation;		//抢地主坐号
	BYTE   byRobCount;         //玩家抢地主次数
	int    iValue;				//抢地主情况(0-叫地主状态 1-抢地主状态)
	BYTE   byFirstCard;         //叫分牌
	RobNTStruct()
	{
		byDeskStation = 255;
		byRobCount = 0;
		iValue = 0;
		byFirstCard = 0;
	}
};

//明牌
struct ShowCardStruct
{
	BYTE bDeskStation;										//坐号
	BYTE iCardList[54];					                   //扑克信息
	BYTE iCardCount;						              //扑克数目
	int iValue;											  //保留
	int iBase;											  //倍数 20081204
};

//加倍
struct AddDoubleStruct
{
	BYTE bDeskStation;										//加倍位置
	int iValue;												//加倍情况
};

//奖分
struct AwardPointStruct
{
	BYTE	iAwardPoint;									//奖分
	BYTE	bDeskStation;									//坐号
	int		iBase;											//倍数
};

//托管数据结构
struct AutoStruct
{
	BYTE bDeskStation;
	bool bAuto;
};

//机器人托管
struct UseAIStation
{
	BYTE bDeskStation;					//坐号
	BOOL bState;						//状态
};

//游戏结束统计数据包
struct GameEndStruct
{
	bool bChunTian;	//是否春天
	int iBaseFen;	//底分
	int iBombCount;	//炸弹数量
	int iAllDouble[PLAY_COUNT];//每个玩家的总倍数
	bool bAddDouble[PLAY_COUNT];//每个玩家是否加倍
	int iZongFen;				//总分
	BYTE	iUserCard[PLAY_COUNT][45];			//用户手上的扑克
	BYTE	iUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	int		iUserScore[PLAY_COUNT];				//各个玩家的得分
	int     iVipGameCount;                      //局数
	GameEndStruct()
	{
		memset(this, 0, sizeof(GameEndStruct));
	}
};

//游戏结束统计数据包
struct GameCutStruct
{
	int					iRoomBasePoint;					//倍数
	int					iDeskBasePoint;					//桌面倍数
	int					iHumanBasePoint;				//人头倍数
	int					bDeskStation;					//退出位置
	__int64				iChangeMoney[8];				//玩家金币
	__int64				iTurePoint[PLAY_COUNT];			//庄家得分

};

struct HaveThingStruct
{
	BYTE pos;
	char szMessage[61];
};

struct LeaveResultStruct
{
	BYTE bDeskStation;
	int bArgeeLeave;
};

struct UserleftDesk
{
	BYTE bDeskStation;                //断线玩家
};

//超端信息结构体
struct	SuperUserMsg
{
	BYTE	byDeskStation;
	bool	bIsSuper;
	SuperUserMsg()
	{
		memset(this, 0, sizeof(SuperUserMsg));
	}
};

//大结算
struct	TZongResult
{
	bool	bWinner[PLAY_COUNT];				//大赢家
	int		iBombCount[PLAY_COUNT];				//炸弹次数
	int		iMaxWinMoney[PLAY_COUNT];			//最多赢钱数目
	int		iMaxContinueCount[PLAY_COUNT];		//最高连胜
	int		iWinCount[PLAY_COUNT];				//胜利局数
	__int64	i64WinMoney[PLAY_COUNT];			//输赢金币
	__int64	i64UserMoney[PLAY_COUNT];			//玩家金币
	BYTE    byGameEndType[PLAY_COUNT];          //大结算结束类型，0：最后一局正常，1：最后一局无人叫分
	TZongResult()
	{
		Init();
	}
	void Init()
	{
		memset(this, 0, sizeof(TZongResult));
		memset(bWinner, false, sizeof(bWinner));
		memset(iBombCount, 0, sizeof(iBombCount));
		memset(iMaxWinMoney, 0, sizeof(iMaxWinMoney));
		memset(iMaxContinueCount, 0, sizeof(iMaxContinueCount));
		memset(iWinCount, 0, sizeof(iWinCount));
		memset(i64WinMoney, 0, sizeof(i64WinMoney));
		memset(i64UserMoney, 0, sizeof(i64UserMoney));
		memset(byGameEndType, 0, sizeof(byGameEndType));
	}
};

//出牌结果
struct CMD_S_OutCard_Result
{
	bool bResult;        //出牌结果，成功true，失败false
	//错误码，1：协议大小不对，2：非游戏状态，3:不是出牌玩家，4：没有这张手牌，5：服务器错误 ,6：牌型错误
	BYTE byErrorCode;

	BYTE byHandCardCount;	//手牌数量
	BYTE byHandCard[45];	//玩家手牌
	CMD_S_OutCard_Result()
	{
		bResult = false;
		byErrorCode = 0;
		byHandCardCount = 0;
		memset(byHandCard, 0, sizeof(byHandCard));
	}
};

#pragma pack()
