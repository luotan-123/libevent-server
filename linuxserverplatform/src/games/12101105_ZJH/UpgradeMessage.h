#pragma once

//文件名字定义
#define GAMENAME						"5人扎金花"
#define NAME_ID							12101105								//名字 ID

//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
//游戏信息
#define PLAY_COUNT						5								//游戏人数
#define MAX_CARD_COUNT                  3
#define SKIN_CARD						"cardskin"		//共用图片文件夹

#define GET_CLIENT_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".ico"))
#define GET_SERVER_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".dll"))
#define GET_SKIN_FOLDER(A)              (sprintf(A,"%d",NAME_ID),strcat(A,""))     //获取图片文件夹
#define GET_KEY_NAME(A, B, C, D)		(sprintf(A,"%s%d%s", B, C, D),strcat(A,""))
#define GET_CLIENT_BCF_NAME(A)          (sprintf(A,"%d",NAME_ID),strcat(A,".bcf"))    

//游戏状态定义
#define GS_WAIT_SETGAME					0				//设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_SEND_CARD					20				//发牌状态
#define GS_PLAY_GAME					21				//游戏中状态
#define GS_WAIT_NEXT					23				//等待下一盘开始 

//按注类型
#define STATE_ERR						0x00			//错误状态
#define STATE_NORMAL					0x01			//正常状态
#define STATE_LOOK						0x02			//看牌了状态
#define STATE_GIVE_UP					0x04			//弃牌状态
#define STATE_FAIL						0x05			//比牌失败

#define ACTION_LOOK						0x00			//看牌
#define ACTION_FOLLOW					0x01			//跟注
#define ACTION_ADD						0x02			//加注
#define ACTION_OPEN						0x03			//比牌
#define ACTION_GIVEUP					0x04			//弃牌
#define ACTION_WIN						0x05			//赢牌
#define ACTION_LOST						0x06			//输牌
#define ACTION_NO						0x07			//无

#define E_CHOUMA_COUNT 4
static int G_iChouMaMoney[E_CHOUMA_COUNT] = { 1,2,4,8};

#pragma pack(1)

/********************************************************************************/
// 数据包处理辅助标识
/********************************************************************************/
#define S_C_GAME_BEGIN					50				//游戏开始		
#define S_C_SEND_CARD					51				//开始发牌
#define S_C_GAME_PLAY					52				//开始游戏		
#define S_C_CALL_ACTION					53				//通知玩家操作	
#define S_C_GAME_END					54				//游戏结束

#define S_C_SUPER_PROOF					70				//超端校验
#define C_S_SUPER_SET					71				//超端设置
#define S_C_SUPER_SET_RESULT			72				//超端设置结果

#define C_S_LOOK_CARD					80				//玩家看牌
#define S_C_LOOK_CARD_RESULT			81				//玩家看牌结果
#define C_S_GEN_ZHU						82				//玩家跟注
#define S_C_GEN_ZHU_RESULT				83				//玩家跟注结果
#define C_S_JIA_ZHU						84				//加注
#define S_C_JIA_ZHU_RESULT				85				//加注结果
#define C_S_BI_PAI						86				//玩家比牌
#define S_C_BI_PAI_RESULT				87				//玩家比牌结果
#define C_S_GIVE_UP						88				//玩家弃牌
#define S_C_GIVE_UP_RESULT				89				//玩家弃牌结果
#define SUB_S_GAME_END_ALL				90				//总结算
#define S_C_JIA_ZHU_FAIL				91				//加注失败
#define S_C_NOTIFY_OPER					92				//提示操作

//断线重连游戏状态数据包（ 等待其他玩家开始 ）
struct S_C_GameStation_WaiteAgree
{
	BYTE		byThinkTime;			//游戏思考时间
	BYTE		byBeginTime;			//游戏开始时间
	bool		bAgree[PLAY_COUNT];		//玩家准备状态
	int			iGuoDi;					//锅底值	
	int			iDiZhu;					//底注      
	__int64		i64ShangXian;			//个人下注上限	明注
	__int64		i64DingZhu;				//顶注

	//游戏规则
	bool		bBaoZiYouXi;			//豹子有喜
	bool		bHuiErPai;				//会儿牌
	bool		b235ShaBaoZi;			//235杀豹子
	int			iRuleDingZhu;			//游戏顶注
	int			iAllTurnCount;          //总轮数
	int			playMode;				//游戏模式，0：经典模式，1：疯狂模式

	S_C_GameStation_WaiteAgree()
	{
		memset(this,0,sizeof(S_C_GameStation_WaiteAgree));
	}
};

/*-----------------------------------------------------------------------------*/
//断线重连游戏状态数据包	（ 游戏正在发牌状态 ）
struct S_C_GameStation_SendCard
{
	BYTE		byThinkTime;			//游戏思考时间
	BYTE		byBeginTime;			//游戏开始时间
	BYTE		byNtStation;					//庄家位置
	BYTE		byUserCardCount[PLAY_COUNT];	//用户手上的牌数
	BYTE        byHuiPai;
	int			iUserState[PLAY_COUNT];		//玩家状态
	int			iGuoDi;						//锅底值	
	int			iDiZhu;						//底注      
	__int64		i64DingZhu;					//顶注
	__int64		i64AllZongZhu;				//总注
	__int64		i64XiaZhuData[PLAY_COUNT];	//用户当前下注信息

	//游戏规则
	bool		bBaoZiYouXi;			//豹子有喜
	bool		bHuiErPai;				//会儿牌
	bool		b235ShaBaoZi;			//235杀豹子
	int			iRuleDingZhu;			//游戏顶注
	int			iAllTurnCount;          //总轮数
	int			playMode;				//游戏模式

	S_C_GameStation_SendCard()
	{
		memset(this,0,sizeof(S_C_GameStation_SendCard));
		byHuiPai=255;
	}
};

/*-----------------------------------------------------------------------------*/
//断线重连游戏状态数据包	（ 游戏中状态 ）
struct S_C_GameStation_PlayGame 
{
	bool		bCanLook[PLAY_COUNT];				//可否看牌
	bool		bCanFollow;				//可否跟注
	bool		bCanAdd[E_CHOUMA_COUNT];				//可否加注
	bool		bCanOpen;				//可否比牌
	bool		bCanGiveUp;				//可否弃牌
	BYTE		byThinkTime;			//游戏思考时间
	BYTE		byBeginTime;			//游戏开始时间
	BYTE		byNtStation;					//庄家位置
	BYTE		byCurrHandleDesk;				//当前操作的玩家
	BYTE		byUserCardCount[PLAY_COUNT];				//用户手上的牌数
	BYTE		byUserCard[PLAY_COUNT][MAX_CARD_COUNT];		//用户手上的牌
	BYTE        byHuiPai;
	int			iUserState[PLAY_COUNT];		//玩家状态
	int			iRemainderTime;				//剩余时间
	int			iGuoDi;						//锅底值	
	int			iDiZhu;						//底注  
	int			iTurn;						//第几轮
	__int64		i64ShangXian;				//个人下注上限	明注
	__int64		i64DingZhu;					//顶注
	__int64		i64AllZongZhu;				//总注
	__int64		i64XiaZhuData[PLAY_COUNT];	//用户当前下注信息

	//游戏规则
	bool		bBaoZiYouXi;			//豹子有喜
	bool		bHuiErPai;				//会儿牌
	bool		b235ShaBaoZi;			//235杀豹子
	int			iRuleDingZhu;			//游戏顶注
	int			iChouMaCount[PLAY_COUNT][E_CHOUMA_COUNT];		//每个筹码类型的数量
	int			iAllTurnCount;          //总轮数
	int			iCardType;				///牌型
	int			playMode;				//游戏模式

	S_C_GameStation_PlayGame()
	{
		memset(this,0,sizeof(S_C_GameStation_PlayGame));
		byHuiPai=255;
	}
};

/*-----------------------------------------------------------------------------*/
//游戏开始
struct	S_C_GameBegin
{
	BYTE                byNtStation;                      //庄家 
	int                 iGuoDi;                         //本局锅底值
	int					iDiZhu;							//本局底注
	int					iUserState[PLAY_COUNT];			//玩家状态
	int					iUserBetValue[E_CHOUMA_COUNT];	//用户下注按钮值
	__int64				i64DingZhu;						//本局顶注 -总下注上限	达到上限 就要开牌	
	__int64				i64ZongXiaZhu;					//当前总下注;
	__int64				i64XiaZhuData[PLAY_COUNT];		//玩家下注数据

	S_C_GameBegin()
	{
		memset(this,0,sizeof(S_C_GameBegin));
	}
};

/*-----------------------------------------------------------------------------*/


//托管数据结构
struct AutoStruct
{
	BYTE bDeskStation;
	bool bAuto;
};

//发牌数据包
struct S_C_SendCard
{
	BYTE		byCard[PLAY_COUNT][MAX_CARD_COUNT];				//牌数据
	BYTE		byCardCount[PLAY_COUNT];                        //牌张数
	BYTE		bySendCardTurn[PLAY_COUNT*MAX_CARD_COUNT];		//发牌顺序--记录的是发牌的位置  255表示不需要发牌
	BYTE		byHuiPai;										//癞子牌
	S_C_SendCard()
	{
		memset(this,0,sizeof(S_C_SendCard));
		memset(bySendCardTurn,255,sizeof(bySendCardTurn));
		byHuiPai=255;
	}

};

/*-----------------------------------------------------------------------------*/
///游戏开始数据包
struct S_C_BeginPlay
{
	BYTE		byCurrHandleStation;			//出牌的位置
	bool		bLook[PLAY_COUNT];		//是否可看牌
	bool		bFollow;	//是否可跟注
	bool		bAdd[E_CHOUMA_COUNT];		//是否可加注
	bool		bOpen;		//是否可比牌
	bool		bGiveUp;	//是否可弃牌
	int			iTurn;		//第几轮下注	
	S_C_BeginPlay()
	{
		memset(this,0,sizeof(S_C_BeginPlay));
	}
};

/*-----------------------------------------------------------------------------*/
//通知操作
struct S_C_NoticeAction
{
	BYTE				byCurrHandleStation;	//出牌的位置
	bool				bCanLook[PLAY_COUNT];				//是否可以看牌	
	bool				bCanFollow;				//是否可以跟注	
	bool				bCanAdd[E_CHOUMA_COUNT];
	bool				bCanOpen;				//是否可以比牌	
	bool				bCanGiveUp;				//是否可以弃牌	
	int					iTurn;					//第几轮下注
	S_C_NoticeAction()
	{
		memset(this,0,sizeof(S_C_NoticeAction));
	}
};

/*-----------------------------------------------------------------------------*/
//用户看牌
struct C_S_LookCard
{
	BYTE	byDeskStation;	
	C_S_LookCard()
	{
		memset(this,255,sizeof(C_S_LookCard));
	}
};

//看牌数据
struct S_C_LookCardResult
{	
	bool			bCanAdd[E_CHOUMA_COUNT];
	BYTE            byDeskStation;				//玩家位置
	BYTE			byUserCardCount;			//用户手上的牌数
	BYTE			byUserCard[MAX_CARD_COUNT];	//用户手上的牌
	int				iUserState;					//玩家状态
	int				iCardType;					//牌型
	S_C_LookCardResult()
	{
		memset(this,-1,sizeof(S_C_LookCardResult));
		byUserCardCount = 0;
	}
};

/*-----------------------------------------------------------------------------*/
//用户跟注
struct C_S_UserGenZhu
{
	BYTE	byDeskStation;		//玩家位置
};

/*-----------------------------------------------------------------------------*/
//用户跟注结果
struct S_C_UserGenZhuResult
{
	BYTE		byDeskStation;		//玩家位置
	int			iUserState;			//该位置玩家的状态
	__int64		i64FollowNum;		//跟注的大小
	__int64		i64UserZongZhu;		//玩家的总下注
	__int64		i64AllZongZhu;		//所有总下注
	S_C_UserGenZhuResult()
	{
		memset(this,0,sizeof(S_C_UserGenZhuResult));
	}
};

/*-----------------------------------------------------------------------------*/
//用户加注
struct C_S_JiaZhu
{
	BYTE		byDeskStation;	
	int			iAddType;			//加注类型
	C_S_JiaZhu()
	{
		memset(this,0,sizeof(C_S_JiaZhu));
	}
};

/*-----------------------------------------------------------------------------*/
//用户加注结果
struct S_C_JiaZhuResult
{
	bool		bDing;				//是否下顶注
	BYTE		byDeskStation;	
	int			iUserState;			//该位置玩家的状态
	__int64		i64AddNum;			//跟注的大小
	__int64		i64UserZongZhu;		//玩家的总下注
	__int64		i64AllZongZhu;		//所有总下注
	S_C_JiaZhuResult()
	{
		memset(this,0,sizeof(S_C_JiaZhuResult));
	}
};

/*-----------------------------------------------------------------------------*/
//用户弃牌、弃牌结果
struct C_S_UserGiveUp
{
	BYTE	byDeskStation;	
};

//弃牌结果
struct S_C_UserGiveUpResult
{
	BYTE	byDeskStation;	
	int		iUserState;					//玩家状态
};

/*-----------------------------------------------------------------------------*/
//用户比牌
struct C_S_UserBiPai
{
	BYTE	byDeskStation;		//主动比牌的玩家
	BYTE	byBeDeskStation;	//被动比牌玩家
	C_S_UserBiPai()
	{
		memset(this, 255, sizeof(C_S_UserBiPai));
	}
};
/*-----------------------------------------------------------------------------*/
//用户比牌
struct S_C_UserBiPaiResult
{
	BYTE	byDeskStation;		//主动比牌的玩家
	BYTE	byBeDeskStation;	//被动比牌玩家
	BYTE	byWinDesk;			//比牌赢的玩家
	BYTE	byLostDesk;			//比牌输的玩家
	int		iUserStation[PLAY_COUNT];			//记录用户状态
	__int64		i64AddNum;			//比牌需要下注的数
	__int64		i64UserZongZhu;		//玩家的总下注
	__int64		i64AllZongZhu;		//所有总下注
	BYTE		byFinished;			// 是否结束(0：没结束 1:比牌之后游戏结束)
	BYTE		byWinnerCard[MAX_CARD_COUNT]; // 最终比牌赢家的牌
	BYTE		byWinCardType;//赢家牌型
	S_C_UserBiPaiResult()
	{
		memset(this,0,sizeof(S_C_UserBiPaiResult));
	}
};


/*-----------------------------------------------------------------------------*/
//游戏结束统计数据包
struct S_C_GameEnd
{
	BYTE				byWiner;							//赢家
	BYTE				byCard[PLAY_COUNT][MAX_CARD_COUNT];	//牌数据
	BYTE				byCardShape[PLAY_COUNT];			//牌型
	int					iOperateCount[PLAY_COUNT];			//操作次数
	int					iUserScore[PLAY_COUNT];			//各玩家得分-扣税前
	int					iChangeMoney[PLAY_COUNT];			//各玩家金币变化-扣税后
	BYTE				iUserSatus[PLAY_COUNT];				//每个玩家的状态
	S_C_GameEnd()
	{
		memset(this,0,sizeof(S_C_GameEnd));
	}
};
/*-----------------------------------------------------------------------------*/
//游戏结束统计数据包
struct GameCutStruct
{
	BYTE					byDeskStation;					//退出位置
	__int64					iTurePoint[PLAY_COUNT];					//庄家得分
	__int64					iChangeMoney[PLAY_COUNT];
	GameCutStruct()
	{
		memset(this,0,sizeof(GameCutStruct));
	}
};
//-------------------------------------------------------------------------------
//超端信息结构体
struct	S_C_SuperUserProof
{
	BYTE	byDeskStation;	
	bool	bIsSuper;
	S_C_SuperUserProof()
	{
		memset(this,0,sizeof(S_C_SuperUserProof));
		byDeskStation = 255;
	}
};
//超端信息结构体
struct	C_S_SuperUserSet
{
	BYTE	byDeskStation;	
	BYTE	byMaxDesk;		//设定最大的玩家
	BYTE	byMinDesk;		//设定最小的玩家
	C_S_SuperUserSet()
	{
		memset(this,255,sizeof(C_S_SuperUserSet));
	}
};
//超端设置结果
struct	S_C_SuperUserSetResult
{
	BYTE	byCode;			//错误码，0：成功，1：发送数据错误，2：已经看牌不能设置，3：玩家状态异常，无法设置，4：不能同时设置一个玩家最大又最小，5：非超端玩家不能设置
	BYTE	byDeskStation;	//设置座位号
	BYTE	byMaxDesk;		//设定最大的玩家
	BYTE	byMinDesk;		//设定最小的玩家
	S_C_SuperUserSetResult()
	{
		memset(this,255,sizeof(S_C_SuperUserSetResult));
		byCode = 0;
	}
};

////大结算
struct TZongResult
{
	int			iGameScore[PLAY_COUNT];			//累积得分
	BYTE		byWinCount[PLAY_COUNT];			//赢钱次数
	BYTE		byLostCount[PLAY_COUNT];		//输钱次数

	int			iCardTypeCount[PLAY_COUNT][8];	//各种牌型次数 1~7
	int			iXiQian[PLAY_COUNT];			//喜钱
	int			iOneMaxScore[PLAY_COUNT];		//单局最高分
	BYTE		iMaxCardType[PLAY_COUNT];		//最大牌型
	BYTE		iWinCount[PLAY_COUNT];			//赢的局数
	BYTE        iLoseCount[PLAY_COUNT];			//输的局数
	TZongResult()
	{
		Init();
	}
	void Init()
	{
		memset(this,0,sizeof(TZongResult));
	}
};

//提示玩家进行某种操作
struct S_C_Notify_Oper
{
	BYTE type;		//1：提示充值，2：提示比牌。其它不用管
	int  time;		//定时器时间
	S_C_Notify_Oper()
	{
		memset(this, 0, sizeof(S_C_Notify_Oper));
	}
};
/*-----------------------------------------------------------------------------*/
#pragma pack()
