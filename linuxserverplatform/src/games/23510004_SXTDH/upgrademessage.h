#pragma once
#include "../Server/MJ/mj.h"

/*------------------------------------------------------------------------------------------------------*/
//支持类型
///支持类型定义
#define SUP_NORMAL_GAME			0x01								///普通游戏
#define SUP_MATCH_GAME			0x02								///比赛游戏
#define SUP_MONEY_GAME			0x04								///金币游戏
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
/*------------------------------------------------------------------------------------------------------*/
#define MAIN_VERSION					710		// 主版本号,?年?月
#define ASS_VERSION						15		// 辅助版本号,?号
/*------------------------------------------------------------------------------------------------------*/
//文件名字定义
#define GAMENAME						TEXT("山西推到胡")
#define NAME_ID						    23510004  		// 名字 ID

/*------------------------------------------------------------------------------------------------------*/
static TCHAR szTempStr[MAX_PATH] =		{0};/** 暂存字符串，用来获取 dll_name 等的时候使用 */
#define GET_CLIENT_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".ico"))
#define GET_SERVER_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".dll"))
#define GET_SKIN_FOLDER(A)				(sprintf(A,"%d",NAME_ID),strcat(A,""))
/*------------------------------------------------------------------------------------------------------*/
#define GET_STRING(NUM)                 #NUM
#define INT_TO_STR(NUM)                 GET_STRING(NUM)
#define SKIN_FOLDER                     TEXT(INT_TO_STR(NAME_ID))          // 服务器目录
/*------------------------------------------------------------------------------------------------------*/
// 版本定义
#define GAME_MAX_VER					2									// 现在最高版本
#define GAME_LESS_VER					2									// 现在最低版本
#define GAME_CHANGE_VER					0									// 修改版本		
#define PLAY_COUNT						4									// 游戏人数	
// 游戏状态定义
#define GS_STATUS_FREE					0				//空闲状态
#define GS_STATUS_PLAYING				1				//游戏中状态
///各种牌的数量
#define HAND_CARD_NUM			14				//最大手牌张数
#define OUT_CARD_NUM			40				//出牌张数
#define MAX_REMAIN_MEN_PAI_NUM  92              //最多剩余门牌数量
#define HUA_CARD_NUM			8				//花牌张数
#define MEN_CARD_NUM			34				//牌墙张数(各自在门前码成17墩牌)
#define MAX_HUPAI_TYPE			4				//糊牌类型最大的组合
#define MAX_CANHU_CARD_NUM      13              //一副牌最多能胡的牌数量
#define MAX_HU_FEN_KIND         2				//胡分种类
#define MAX_MJ_PAI				38				//最大牌值
/***********************算分使用******************/
#define MAX_COUNT_GANG_FEN       13              //杠分产生的最多列表项
#define MAX_COUNT_HORSE_FEN      8               //买马分生成的最多列表项，最多买两2马的时候，最多产生8项
#define MAX_COUNT_GEN_ZHUANG_FEN 1               //最大跟庄分项
/***********************************/

//麻将牌面风格定义
#define MJTABLE_CARDTYPE0				0									//牌样式1
#define MJTABLE_CARDTYPE1				1									//牌样式2
#pragma pack(1)
//服务器命令结构
#define S_C_GAME_START			100									//游戏开始
#define S_C_OUT_CARD			101									//出牌命令
#define S_C_SEND_CARD			102									//发送扑克
#define S_C_OPERATE_NOTIFY		103									//操作提示
#define S_C_OPERATE_RESULT		104									//操作命令
#define S_C_GAME_END			105									//游戏结束
#define S_C_TRUSTEE				106									//用户托管
#define S_C_GAME_END_ALL		107								    //总结算
#define S_C_QIHU_NOTIFY		    108								    //弃胡通知
#define S_C_OUT_CARD_RESULT		109								    //出牌结果


/////////////////////////////////服务器基础玩法宏定义////////////////////////
//定义玩法规则必须RULE_开头
#define RULE_HAVE_TING_PAI_NOTIFY //听牌提示 
#define RULE_HAVE_HU_PAI_NOTIFY   //胡牌提示
#define DELAY_CHECK_ACTION   //延时检测动作
/////////////////////////////////////////////////////////////////////////////


//麻将动作
enum MJ_ACTION
{
	ACTION_NO=0,//没有动作
	ACTION_CHI,//吃牌动作
	ACTION_CHI_FRONT,//吃头
	ACTION_CHI_MID,//吃中
	ACTION_CHI_BACK,//吃尾
	ACTION_PENG,//碰牌动作
	ACTION_KAN,//坎牌牌动作
	ACTION_GANG,//杠
	ACTION_AN_GANG,//暗杠动作
	ACTION_BU_GANG,//补杠动作
	ACTION_MING_GANG,//明杠动作
	ACTION_CHI_TNG,//抢吃（吃后听牌）
	ACTION_TING_PAI,//听动作
	ACTION_HU,//糊动作
	ACTION_PASS,//过
};

//麻将算分位置
enum MJ_STATION
{
	MJ_STATION_NO = 0,//无类型
	MJ_STATION_BEN_JIA = 1,//本家
	MJ_STATION_XIA_JIA = 2,//下家
	MJ_STATION_DUI_JIA = 4,//对家
	MJ_STATION_SHANG_JIA = 8,//上家
	MJ_STATION_SHANG_XIA_JIA = MJ_STATION_SHANG_JIA | MJ_STATION_XIA_JIA,//上家和下家
	MJ_STATION_SHANG_DUI_JIA = MJ_STATION_SHANG_JIA | MJ_STATION_DUI_JIA,//上家和对家
	MJ_STATION_DUI_XIA_JIA = MJ_STATION_DUI_JIA | MJ_STATION_XIA_JIA,//对家和下家
	MJ_STATION_ALL_JIA = MJ_STATION_SHANG_JIA | MJ_STATION_XIA_JIA | MJ_STATION_DUI_JIA,//三家（上家，对家和下家），3人麻将中叫另外2家，2人麻将另外1家
};

//麻将算分杠类型
//根据不同的麻将可以有不同的叫法
enum MJ_GANG_FEN_TYPE
{
	MJ_GANG_FEN_TYPE_NO = 0,//无类型
	MJ_GANG_FEN_TYPE_AN_GANG ,//暗杠
	MJ_GANG_FEN_TYPE_MING_GANG,//明杠
	MJ_GANG_FEN_TYPE_BU_GANG,//明杠(碰)
	MJ_GANG_FEN_TYPE_BEI_AN_GANG,//被暗杠
	MJ_GANG_FEN_TYPE_FANG_GANG,//点杠
	MJ_GANG_FEN_TYPE_BEI_BU_GANG,//被明杠（碰） ----补杠引起
	MJ_GANG_FEN_TYPE_BEI_MING_GANG,//被明杠
};

//麻将胡牌名字
//不同的地方麻将可以有不同的叫法
enum MJ_HU_FEN_TYPE
{
	MJ_HU_FEN_TYPE_NO = 0,//无类型
	MJ_HU_FEN_TYPE_ZI_MO,//自摸
	MJ_HU_FEN_TYPE_BEI_ZI_MO,//被自摸
	MJ_HU_FEN_TYPE_DIAN_PAO,//点炮（放炮）
	MJ_HU_FEN_TYPE_DIAN_PAO_HU,//吃胡（点炮胡）
};

//糊牌类型
enum HUPAI_TYPE_MJ//麻将胡牌类型枚举结构
{
	MJ_HUPAI_TYPE_None					=255,	//默认填充值	
	MJ_HUPAI_TYPE_PingHu				=3,		//平胡
	MJ_HUPAI_TYPE_QiangGang				=46,	//抢杠胡
	MJ_HUPAI_TYPE_QiDui					=58,	//七小对
	MJ_HUPAI_TYPE_QingYiSe				=61,	//清一色
	MJ_HUPAI_TYPE_ShiSanYao				=84,	//十三幺
	MJ_HUPAI_TYPE_LongQiDui				=88,	//豪华七对
	MJ_HUPAI_TYPE_YI_TIAO_LONG			=89,	//一条龙
};

//胡分计算
struct CountHuFenStruct
{
	BYTE byNameType[3];					//相应的名字， 枚举类型MJ_HU_FEN_TYPE
	BYTE byXiangYingStation[3];			//相应玩家，枚举类型MJ_STATION
	BYTE byXiangYingStationEx[3];		//相应玩家，枚举类型MJ_STATION
	int iAllFen[3];						//输赢的胡分
	BYTE byNums;
	CountHuFenStruct()
	{
		Init();
	}
	void Init()
	{
		memset(byNameType,MJ_HU_FEN_TYPE_NO,sizeof(byNameType));
		memset(byXiangYingStation,MJ_STATION_NO,sizeof(byXiangYingStation));
		memset(byXiangYingStationEx,MJ_STATION_NO,sizeof(byXiangYingStationEx));
		memset(iAllFen,0,sizeof(iAllFen));
		byNums = 0;
	}
};

//杠分结算
struct CountGangFenStruct
{
	BYTE byGangType[MAX_COUNT_GANG_FEN];		//杠类型 ，枚举类型MJ_GANG_FEN_TYPE
	BYTE byXiangYingStation[MAX_COUNT_GANG_FEN];//相应玩家，枚举类型MJ_STATION
	bool bBaoTing[MAX_COUNT_GANG_FEN];			//这个杠是否报听(相应玩家是否报听),true报听杠
	BYTE byGangNums[MAX_COUNT_GANG_FEN];		//相同杠的数量：：指杠类型相同，相应玩家位置相同
	int iAllFen[MAX_COUNT_GANG_FEN];//总分
	BYTE byNums;
	CountGangFenStruct()
	{
		Init();
	}
	void Init()
	{
		memset(byGangType,MJ_GANG_FEN_TYPE_NO,sizeof(byGangType));
		memset(byXiangYingStation,MJ_STATION_NO,sizeof(byXiangYingStation));
		memset(byGangNums,0,sizeof(byGangNums));
		memset(iAllFen,0,sizeof(iAllFen));
		byNums = 0;
	}
};

///吃碰杠牌数据结构
struct TCPGStruct
{
	BYTE  byType;    //吃碰杠类型
	BYTE  iStation;  //吃碰杠玩家位置
	BYTE  iBeStation;//被吃碰杠玩家位置
	BYTE  iOutpai;   //别人出的牌
	BYTE  byData[4]; //吃碰杠牌数据
	TCPGStruct()
	{
		Init();
	}
	void Init()
	{
		memset(byData,255,sizeof(byData));
		byType = ACTION_NO;    //吃碰杠类型
		iOutpai = 255;   //别人出的牌
		iStation = 255;  //吃碰杠玩家位置
		iBeStation = 255;//被吃碰杠玩家位置
	};
};

//此处定义创建房间的规则，游戏规则
struct tagStructGameRuler
{
	bool bDaHu;                  //是否大胡，bDaHu=true是大胡玩法，bDaHu=false是平胡玩法
	bool bBaoTing;               //报听
	int  iZiMoScore;			 //自摸分数
	bool bZhiZiMo;               //只自摸
	bool bFenPai;                //是否带风牌
	bool bAlreadSet;			 //服务务端自用
	tagStructGameRuler()
	{
		Init();
	}
	void Init()
	{
		bZhiZiMo = false;
		bDaHu = true;
		bFenPai = false;
		bBaoTing = true;
		iZiMoScore = 3;
		bAlreadSet = 0;
	}
};

//空闲状态
struct CMD_S_StatusFree
{
	bool bAgree[PLAY_COUNT];//准备玩家
	BYTE byOutTime;			//出牌时间		
	BYTE byBlockTime;		//拦牌思考时间
	BYTE byMJCardType;		//麻将牌样式
	int iPlayingCount;		//进行局数
	tagStructGameRuler tGameRuler;		//游戏规则
	bool bSuperFlag;		//超端玩家
	CMD_S_StatusFree()
	{
		Init();
	}
	void Init()
	{
		bSuperFlag = false;
		byOutTime=15;
		byBlockTime=15;
		iPlayingCount=0;
		memset(bAgree,false,sizeof(bAgree));
		byMJCardType=MJTABLE_CARDTYPE0;
	}
};

//游戏中状态
struct CMD_S_StatusPlay
{
	bool				bTing[PLAY_COUNT];	//听牌
	//bool				bTinging;			//是否正在进行听牌操作
	bool				bTuoGuan[PLAY_COUNT];//托管
	BYTE				byOutTime;			//出牌时间		
	BYTE				byBlockTime;		//拦牌思考时间	
	//手牌信息
	BYTE				byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌不超过14张 	 
	BYTE				byArHandPaiCount[PLAY_COUNT];//手牌张数	 
	//出牌信息
	BYTE				byArOutPai[PLAY_COUNT][OUT_CARD_NUM];//出牌不超过40张
	BYTE				byArOutPaiCount[PLAY_COUNT];//出牌的张数
	BYTE				byMJCardType;//麻将牌样式
	//出牌玩家
	BYTE				byNowOutStation;//出牌位置
	//BYTE				byTingCanOut[HAND_CARD_NUM];//听牌能打的牌
	BYTE				byNtStation;
	BYTE				iMenNums;//门牌数量
	int					iPlayingCount;//进行局数
	//吃碰杠信息
	TCPGStruct			UserGCPData[PLAY_COUNT][5];//吃碰杠
	tagStructGameRuler  tGameRuler;		//游戏规则
	bool                bQiHu;          //是否正处于弃胡状态
	BYTE				byTingOutCardIndex[PLAY_COUNT];//出牌列表中，听牌倒扣的索引牌
	bool				bSuperFlag;	//超端玩家
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
	//胡牌信息
	BYTE                byOutCanHuCard[HAND_CARD_NUM];  //出牌前，出这张牌能听
	BYTE				byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];    //出牌前，对应索引能胡的牌
	BYTE				byCardRemainNum[MAX_MJ_PAI];		//操作玩家显示剩余牌数量
#endif
#ifdef RULE_HAVE_HU_PAI_NOTIFY
	BYTE                byOutAfterHuCard[MAX_CANHU_CARD_NUM];   //出牌后，胡牌数据
#endif
	CMD_S_StatusPlay()
	{
		Init();
	}
	void Init()
	{
		bSuperFlag = false;
		memset(bTing,false,sizeof(bTing));
		//bTinging=false;
		byOutTime=20;
		memset(byArHandPai,255,sizeof(byArHandPai));
		memset(byArHandPaiCount,0,sizeof(byArHandPaiCount));
		memset(byArOutPai,255,sizeof(byArOutPai));
		memset(byArOutPaiCount,0,sizeof(byArOutPaiCount));
		byNowOutStation=255;
		iMenNums=0;
		//memset(byTingCanOut,255,sizeof(byTingCanOut));
		memset(bTuoGuan,false,sizeof(bTuoGuan));
		iPlayingCount=0;
		byNtStation=255;
		byMJCardType=MJTABLE_CARDTYPE0;
		//iBaseFen=1;
		bQiHu = false;
		memset(byTingOutCardIndex,255,sizeof(byTingOutCardIndex));
#ifdef RULE_HAVE_TING_PAI_NOTIFY
		memset(byOutCanHuCard,255,sizeof(byOutCanHuCard));
		memset(byHuCard,255,sizeof(byHuCard));
		memset(byCardRemainNum,0,sizeof(byCardRemainNum));
#endif
#ifdef RULE_HAVE_HU_PAI_NOTIFY
		memset(byOutAfterHuCard,255,sizeof(byOutAfterHuCard));
#endif
	}
};

//游戏开始
struct CMD_S_GameStart
{
	BYTE							byNtStation;								//庄家
	BYTE							byCard[PLAY_COUNT][HAND_CARD_NUM];			//牌数据
	BYTE							byCardCount[PLAY_COUNT];					//牌张数
	BYTE							bySezi0;									//色子0数据
	BYTE							bySezi1;									//色子1数据
	BYTE							byGetPaiDir;								//本局起牌方向
	BYTE							byGetPai;									//起牌敦数
	BYTE							byMJCardType;	                            //麻将牌样式
	BYTE                            byNtCard;                                   //庄家起手抓到的牌
	BYTE							iMenNums;									//门牌数量
	int								iPlayingCount;								//局数
	bool							bSuperFlag[PLAY_COUNT];						//超端玩家
#ifdef RULE_HAVE_TING_PAI_NOTIFY
	BYTE                            byOutCanHuCard[HAND_CARD_NUM];  //出这张牌能听
	BYTE							byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];    //对应索引能胡的牌   
	BYTE							byCardRemainNum[MAX_MJ_PAI];		//操作玩家显示剩余牌数量
#endif
	CMD_S_GameStart()
	{
		memset(bSuperFlag,false,sizeof(bSuperFlag));
		byNtStation=255;
		memset(byCard,255,sizeof(byCard));
		memset(byCardCount,0,sizeof(byCardCount));
#ifdef RULE_HAVE_TING_PAI_NOTIFY
		memset(byOutCanHuCard,255,sizeof(byOutCanHuCard));
		memset(byHuCard,255,sizeof(byHuCard));
		memset(byCardRemainNum,0,sizeof(byCardRemainNum));
#endif
		iMenNums=0;
		iPlayingCount=0;
		bySezi0=255;
		bySezi1=255;
		byGetPaiDir=255;
		byGetPai=255;
		byMJCardType=MJTABLE_CARDTYPE0;
		/*byBaoGen = 255;
		byBaoCard=255;
		iGameBase = 1;*/
	}
};

//出牌
struct CMD_S_OutCard
{
	bool							bTing;
	BYTE							byOutCardUser;						//出牌用户
	BYTE							byOutCardData;						//出牌扑克	
	BYTE                            byCardData[HAND_CARD_NUM];          //牌数据
	BYTE                            byCardCount;                        //牌数目
#ifdef RULE_HAVE_HU_PAI_NOTIFY //是否要做胡牌提示
	BYTE							byHuCard[MAX_CANHU_CARD_NUM];	    //能胡的牌
#endif
	BYTE							byOutCardDataIndex;                 //出牌索引
	CMD_S_OutCard()
	{
		bTing=false;
		byOutCardUser=255;
		byOutCardData=255;
		memset(byCardData,255,sizeof(byCardData));
#ifdef RULE_HAVE_HU_PAI_NOTIFY //是否要做胡牌提示
		memset(byHuCard,255,sizeof(byHuCard));
#endif
		byCardCount=0;
		byOutCardDataIndex=255;
	}
};

//出牌结果
struct CMD_S_OutCard_Result
{
	bool							bResult;        //出牌结果，成功true，失败false
	//错误码，1：非游戏状态，2:不是出牌玩家，3：没有这张手牌，4：牌已经出完，5：已经听牌，6：有吃碰杠胡动作不能出牌
	BYTE							byErrorCode;	
	CMD_S_OutCard_Result()
	{
		bResult = false;
		byErrorCode = 0;
	}
};

//抓牌
struct CMD_S_SendCard
{
	BYTE							byCardData;							//扑克数据
	BYTE							byCurrentUser;						//当前用户
	BYTE                            byCard[HAND_CARD_NUM];				//牌数据
	BYTE                            byCardCount;                        //牌数目	
	BYTE							iMenNums;							//门牌数量
#ifdef RULE_HAVE_TING_PAI_NOTIFY
	BYTE                            byOutCanHuCard[HAND_CARD_NUM];  //出这张牌能听
	BYTE							byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];    //对应索引能胡的牌
	BYTE							byCardRemainNum[MAX_MJ_PAI];		//操作玩家显示剩余牌数量
#endif
	CMD_S_SendCard()
	{
		byCardData=255;
		byCurrentUser=255;
		memset(byCard,255,sizeof(byCard));
		byCardCount=0;
		iMenNums=0;
#ifdef RULE_HAVE_TING_PAI_NOTIFY
		memset(byOutCanHuCard,255,sizeof(byOutCanHuCard));
		memset(byHuCard,255,sizeof(byHuCard));
		memset(byCardRemainNum,0,sizeof(byCardRemainNum));
#endif
	}
};

//吃碰杠胡操作提示
struct CMD_S_OperateNotify
{
	bool							bChi;							
	bool							bPeng;
	bool							bGang;
	bool							bHu;
	bool							bTing;
	BYTE							byUsr;								//操作玩家
	BYTE							byChi[3][3];						//能吃的牌
	BYTE							byPeng;								//能碰的牌
	BYTE							byGangData[4][2];					//能杠的牌	
	BYTE							byTingCanOut[HAND_CARD_NUM];		//听牌能打的牌
	BYTE							byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];  //听牌，打那张牌胡那张牌
	BYTE							byCardRemainNum[MAX_MJ_PAI]; //每张牌的剩余数量
	CMD_S_OperateNotify()
	{
		Init();
	}
	void Init()
	{
		bChi=false;
		bPeng=false;
		bGang=false;
		bHu=false;
		bTing=false;
		byUsr=255;
		memset(byChi,255,sizeof(byChi));
		memset(byGangData,255,sizeof(byGangData));
		memset(byTingCanOut,255,sizeof(byTingCanOut));
		byPeng=255;
	}
};

//吃碰杠胡操作结果
struct CMD_S_OperateResult
{
	BYTE  byType;    //吃碰杠类型
	BYTE  iStation;  //吃碰杠玩家位置
	BYTE  iBeStation;//被吃碰杠玩家位置
	BYTE  iOutpai;   //别人出的牌
	BYTE  byData[4]; //吃碰杠牌数据
	BYTE  byCard[HAND_CARD_NUM];//手牌数据
	BYTE  byCardCount;//手牌数量
#ifdef RULE_HAVE_TING_PAI_NOTIFY
	BYTE  byOutCanHuCard[HAND_CARD_NUM];                 //出这张牌能听
	BYTE  byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];    //对应索引能胡的牌
	BYTE  byCardRemainNum[MAX_MJ_PAI];		//操作玩家显示剩余牌数量
#endif
	CMD_S_OperateResult()
	{
		byType=255;
		iStation=255;
		iBeStation=255;
		iOutpai=255;
		byCardCount=0;
		memset(byData,255,sizeof(byData));
		memset(byCard,255,sizeof(byCard));
#ifdef RULE_HAVE_TING_PAI_NOTIFY
		memset(byOutCanHuCard,255,sizeof(byOutCanHuCard));
		memset(byHuCard,255,sizeof(byHuCard));
		memset(byCardRemainNum,0,sizeof(byCardRemainNum));
#endif
	}
};

//游戏结束
struct CMD_S_GameEnd
{
	bool							bZimo;						//是否自摸
	bool							bIsLiuJu;					//是否流局
	bool                            bIsHu[PLAY_COUNT];		//哪些玩家胡牌了
	BYTE							byFangPao;				//放炮玩家位置
	BYTE							byPs;						//胡的牌 
	BYTE							byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE							byArHandPaiCount[PLAY_COUNT];//手牌张数
	///////////////////其它数据//////////////////
	//BYTE							byBaoCard;					//癞子牌
	TCPGStruct						UserGCPData[PLAY_COUNT][5];	//吃碰杠
	//BYTE							byArOutPai[PLAY_COUNT][OUT_CARD_NUM]; //出牌数据
	//BYTE							byArOutPaiCount[PLAY_COUNT];	 //出牌的张数
	BYTE							byRemainMenPai[MAX_REMAIN_MEN_PAI_NUM];
	BYTE							byRemainMenPaiCount;//剩余门牌数量
	BYTE							byHuType[PLAY_COUNT][MAX_HUPAI_TYPE];//胡牌类型 ，255默认填充值
	int								iFanCount[PLAY_COUNT];				 //胡牌类型的番数，比如大四喜88番，清一色16番
	bool							bTing[PLAY_COUNT];					 //玩家是否报听
	CountHuFenStruct                countHuFen[PLAY_COUNT];     //胡分列表
	CountGangFenStruct              countGangFen[PLAY_COUNT];   //杠分列表
	long long						llGameScore[PLAY_COUNT];	//总得分
	int								iWinFailDouble[PLAY_COUNT]; //金币场输赢多少倍
	CMD_S_GameEnd()
	{
		Init();
	}
	void Init()
	{
		bZimo = false;		//是否自摸
		byFangPao=255;
		byPs		= 255;	//胡的牌值
		bIsLiuJu	= true;	//是否流局
		memset(bIsHu,false,sizeof(bIsHu));
		memset(byArHandPai, 255, sizeof(byArHandPai));//手牌数据	
		memset(byArHandPaiCount, 0, sizeof(byArHandPaiCount));
		memset(llGameScore,0,sizeof(llGameScore));
		memset(iFanCount,0,sizeof(iFanCount));
		memset(byHuType,255,sizeof(byHuType));
		memset(bTing,false,sizeof(bTing));
		//byBaoCard=255;
		/*memset(byArOutPai,255,sizeof(byArOutPai));
		memset(byArOutPaiCount,0,sizeof(byArOutPaiCount));*/
		memset(byRemainMenPai,255,sizeof(byRemainMenPai));
		byRemainMenPaiCount = 0;
		memset(iWinFailDouble,0,sizeof(iWinFailDouble));
	}
};
struct CMD_S_ZongResult
{
	int								iZimo[PLAY_COUNT];				//自摸次数
	int								iJiePao[PLAY_COUNT];			//接炮次数
	int								iDianPao[PLAY_COUNT];			//点炮次数
	int								iDianGang[PLAY_COUNT];			//点杠次数
	int								iGang[PLAY_COUNT];			    //杠牌次数
	long long						llGameScore[PLAY_COUNT];		//累积得分
	CMD_S_ZongResult()
	{
		Init();
	}
	void Init()
	{
		memset(iZimo,0,sizeof(iZimo));
		memset(iJiePao,0,sizeof(iJiePao));
		memset(iDianPao,0,sizeof(iDianPao));
		memset(iDianGang,0,sizeof(iDianGang));
		memset(iGang,0,sizeof(iGang));
		memset(llGameScore,0,sizeof(llGameScore));
	}
};
//用户托管
struct CMD_S_Trustee
{
	bool							bTrustee;					//是否托管
	BYTE							byDeskStation;				//托管座位号
	CMD_S_Trustee()
	{
		bTrustee=false;
		byDeskStation=255;
	}
};


////////////////////客户端发送给服务器
/////////////////////////////////////////////////////////////////////////////////////////////
//客户端命令结构
#define C_S_OUT_CARD				50									//出牌命令
#define C_S_OPERATE_CARD			51									//操作扑克
#define C_S_TRUSTEE				    52									//用户托管

/*------------------------------------------超端------------------------------------*/
#define C_S_SUPER_REQUEST_LOOK							120				//超端看牌
#define S_C_SUPER_REQUEST_LOOK_RESULT					121				//超端看牌结果
#define C_S_SUPER_REQUEST_CRAD							122				//超端要牌
#define S_C_SUPER_REQUEST_CRAD_RESULT					123				//超端要牌结果


//请求记牌器
struct CMD_C_SuperLook
{
};

//请求记牌器结果
struct CMD_S_SuperLookResult
{
	BYTE    byRemainPaiDate[38];		//剩余牌数据	下标索引为牌值	
	CMD_S_SuperLookResult()
	{
		memset(byRemainPaiDate,0,sizeof(byRemainPaiDate));
	}
};

//请求要牌
struct CMD_C_Super_Request_Card
{
	BYTE CardDate;                //牌数据
	CMD_C_Super_Request_Card()
	{
		CardDate = 255;
	}
};

//请求要牌结果
struct CMD_S_Super_Request_Card_Result
{
	BYTE	ECode;                 //错误码：0 成功  1 不是超端玩家  2 没有这张牌
	CMD_S_Super_Request_Card_Result()
	{
		ECode = 0;
	}
};

//出牌
struct CMD_C_OutCard
{
	bool							bTing;								//听牌
	BYTE							byOutCardData;						//出牌数据
	CMD_C_OutCard()
	{
		bTing=false;
		byOutCardData=255;
	}
};
//吃碰杠胡操作
struct CMD_C_OperateCard
{
	BYTE							byOperateType;						//操作类型
	BYTE							byActionData[4];					//吃碰杠数据
	CMD_C_OperateCard()
	{
		memset(byActionData,255,sizeof(byActionData));
		byOperateType=ACTION_NO;
	}
};

//额外，加底分
struct CMD_C_AddScore
{
	int							    iNums;			//分数
	CMD_C_AddScore()
	{
		iNums = 0;
	}
};

//用户托管
struct CMD_C_Trustee
{
	bool							bTrustee;							//是否托管	
	CMD_C_Trustee()
	{
		bTrustee=false;
	}
};
#pragma pack()