//#pragma once

#ifndef AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#define AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#pragma pack(1)
//文件名字定义

#define GAMENAME						"祁陽跑胡子"
#define NAME_ID							37460003						//名字 ID
#define PLAY_COUNT						4								//游戏人数
//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
#define GAME_TABLE_NAME					"MatchTable"
//游戏信息

#define GET_STRING(NUM)                 #NUM
#define INT_TO_STR(NUM)                 GET_STRING(NUM)
#define CLIENT_DLL_NAME                 INT_TO_STR(NAME_ID)##".ico"        // 客户端ico名字
#define SKIN_FOLDER                     INT_TO_STR(NAME_ID)                // 客户端目录
#define SKIN_CARD						"CardSkin"		                 // 共用图片文件夹

//游戏状态定义
#define GS_WAIT_SETGAME					0				//等待东家设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_PLAYGAME						23	//游戏状态
#define GS_WAIT_NEXT					24
#define GS_PIAO							25	//漂分状态

#define PIAO_COUNT					3	//漂分选择项个数

#define STATE_NULL					0x00			//表此位置无人
#define STATE_PLAY_GAME				0x01			//表此位置有人

//牌值
#define KINGVALUE					0x2f			//王牌(龙，癞子值)

//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))
typedef unsigned char BYTE;
/********************************************************************************/
//数值定义
#define MAX_WEAVE					7									//最大组合
#define MAX_INDEX					20									//最大索引
#define MAX_COUNT					21									//最大数目
#define	MASK_COLOR					0xF0								//花色掩码
#define	MASK_VALUE					0x0F								//数值掩码

enum HU_TYPE
{
	COMMON_HU				= 0x0001 , //平胡
	SHI_RED_HU_15			= 0x0002 , //小红胡		2番
	ONE_RED_HU_15			= 0x0004 , //一点红		3番
	DIAN_RED_HU_15			= 0x0008 , //大红胡		3番
	SHI_WU_RED_HU_15		= 0x0010 , //红乌		4番
	ALL_BLACK_HU_15			= 0x0020 , //全黑		4番
	//-----------------------------------------------------------
	RED_HU_6				= 0x0040 , //红胡(七张)		//六胡玩法
	ONE_RED_HU_6			= 0x0080 , //一个红
	ALL_RED_HU_6			= 0x0100 , //全红
	JIU_RED_HU_6			= 0x0200 , //9红胡
	ALL_BLACK_HU_6			= 0x0400 , //全黑
	//-----------------------------------------------------------
	WANG_DIAO				= 0x0800 , //王钓
	WANG_CHUANG				= 0x1000 , //王闯
	SWANG_CHUANG			= 0x2000 , //三王闯
	WANG_ZHA				= 0x4000 , //王炸
	ERROR_HU				= 0x8000   //错误牌型
};
//动作定义
#define ACK_NULL					0x0000								//空
#define ACK_TI						0x0001								//提
#define ACK_PAO						0x0002								//跑
#define ACK_WEI						0x0004								//偎
#define ACK_WEI_EX					0x0008								//嗅偎
#define ACK_CHI						0x0010								//吃
#define ACK_CHI_EX					0x0020								//吃
#define ACK_PENG					0x0040								//碰
#define ACK_CHIHU					0x0080								//胡
#define ACT_WANG_DIAO				0x0100								//王钓(只胡自摸)
#define ACT_WANG_CHUANG				0x0200								//王闯
#define ACT_SWANG_CHUANG			0x0400								//三王闯
#define ACT_WANG_ZHA				0x0800								//王炸
#define ACK_JIANGPAI				0x1000								//将牌

/*------------------------------------------------------------------------------*/
//吃牌类型
#define CK_NULL						0x00								//无效类型
#define CK_XXD						0x01								//小小大搭
#define CK_XDD						0x02								//小大大搭
#define CK_EQS						0x04								//二七十吃
#define CK_LEFT						0x10								//靠左对齐
#define CK_CENTER					0x20								//居中对齐
#define CK_RIGHT					0x40								//靠右对齐


#define OUT_CARD_NUM				23									//玩家出牌数	
//====================================================================================================================
//====================================================================================================================
//====================================================================================================================
//====================================================================================================================
//组合子项
struct tagWeaveItem
{
	int								iWeaveKind;							//组合类型
	BYTE							byCardCount;						//扑克数目
	BYTE							byCenterCard;						//中心扑克
	BYTE							byCardList[4];						//扑克列表
	bool							bIsJing[4];							//是否是精牌
	tagWeaveItem()
	{
		Init();
	}
	void Init()
	{
		iWeaveKind=ACK_NULL;
		memset(this,0,sizeof(tagWeaveItem));
	}
};
//标志出牌值
struct OutCard
{
	BYTE byOutCard;
	BYTE byChairID;
	OutCard(){ Clear();	}
	void Clear(){ memset(this,255,sizeof(OutCard)); }
};

//游戏规则
struct GameRuler
{
	BYTE byKingNum;			//王的个数
	BYTE byMinHuXi;			//胡牌时最小胡息 (9息:9  6息:6  3息:3)
	BYTE byUserCount;		//游戏人数
	bool bGenXing;			//true:跟醒 false:翻醒
	int  iMaxScore;			//积分上限
	bool iDianPao;			//能否点炮												DianPao

	BYTE tunXiRatio;		//囤息转换,1囤几息(1囤1息:传1    1囤3息:传3)			txr
	BYTE minHuXiTun;		//积分选择,起胡息数几囤 (例: 9息1囤:传1  9息3囤:传3)	mhxt
	BYTE biHuType;			//必胡类型(0:无 1:点炮必胡  2:有胡必胡)					bht
	BYTE playType;			//玩法(0:无  1:红黑点)									pt
	bool maoHu;				//能否毛胡 (0:否  非0:能)								mh
	bool b15Type;			//是否15张玩法 (0:否  非0:是)							b15
	BYTE piaoFen;			//漂分(0:不漂  1:漂1/2/3   2:漂2/3/5)					pf
	BYTE firstBanker;		//首局坐庄(0:随机  1:房主)								fb
	BYTE autoOverTime;		//超时托管时间(0:不托管  非0:超时分钟数)				aot
	BYTE autoCount;			//托管局数(0:整局托管 1:单局托管 3:3局托管)				ac
	bool bShuangXing;		//是否双醒												bsx
	bool bWeiVisible;		//偎牌是否可见											bwv
	bool bXianHuXianFan;	//是否限胡限番											bxhxf

	GameRuler() { Clear();}
	void Clear()
	{
		memset(this,0,sizeof(GameRuler));
	}
};
//吃牌数据
struct EatData
{
	BYTE byCard[3];
	EatData()
	{
		memset(this,0,sizeof(EatData));
	}
};

struct EatCardMemory
{
	EatData  WordArray;				   //首层
	EatData  SecWordArray[6] ;		   //第二层吃牌(6是怎么来的,吃头，吃中，吃尾,大大小，小小大，二七十)
	EatData  ThirdWordArray[6][6] ;    //第三层吃牌:(6同上）
	BYTE     SecCompareArrayCount ;	   //构成二层吃牌列表
	BYTE     ThirdCompareArrayCount ;  //构成第三层吃牌的数量
	EatCardMemory()
	{
		memset(this,0,sizeof(EatCardMemory));
	}
};

struct tagChiCardInfo
{
	BYTE byChiPai;
	EatCardMemory MemeoryEatCard[6];
	tagChiCardInfo()
	{
		Clear();
	}
	void Clear()
	{
		memset(this,0,sizeof(tagChiCardInfo));
	}
};

//胡牌信息
struct tagHuCardInfo
{
	BYTE							byHuXiCount;						//胡息数目
	BYTE							byXing;								//醒牌
	int								iXingNum;							//跟醒个数
	int								iSumDun;							//囤数
	int								iSumFan;							//总番数
	int								iAllDun;							//总囤数
	int								iHuType;							//胡牌类型
	bool							bZiMo;								//当前是否自摸
	BYTE							byWeaveCount;						//组合数目
	tagWeaveItem					tWeaveItemArray[MAX_WEAVE];			//组合扑克
	BYTE							byStation;							//胡牌玩家位置
	BYTE							byBeStation;						//被胡牌玩家位置
	bool							bWeaveReplace;						//偎牌替换
	tagHuCardInfo()
	{
		Clear();
	}
	void Clear()
	{
		memset(this,0,sizeof(tagHuCardInfo));
		byStation=255;
		byBeStation=255;
	}
};

//////////////////////////////////////////////////////////////////////////
//服务器命令结构
#define S_C_SEND_ALL_CARD			98									//发牌
#define S_C_GAME_BEGIN_PLAY			100									//游戏开始
#define S_C_USER_TI_CARD			101									//用户提牌
#define S_C_USER_PAO_CARD			102									//用户跑牌
#define S_C_USER_WEI_CARD			103									//用户偎牌
#define S_C_USER_PENG_CARD			104									//用户碰牌
#define S_C_USER_CHI_CARD			105									//用户吃牌
#define S_C_OPERATE_NOTIFY			106									//操作提示
#define S_C_OUT_CARD_NOTIFY			107									//出牌提示
#define S_C_OUT_CARD				108									//用户出牌
#define S_C_SEND_CARD				109									//发牌命令
#define S_C_GAME_END				110									//游戏结束
#define S_C_BIG_GAME_END			111									//大结算
#define S_C_UPDATA_HUXI				112									//更新胡息
#define S_C_WANGTAKEOUT				113									//王隐藏
#define S_C_ACTION_RESULT			114									//动作结果
#define S_C_FORBIDCARD				115									//通知死守
#define S_C_NOTICE_PIAO				116									//通知选漂分    S_C_NoticePiao
#define C_S_PIAO_REQ				117									//选漂分请求    C_S_PiaoReq
#define S_C_PIAO_RESP				118									//选漂分回复    S_C_PiaoResp

/*------------------------------------------超端------------------------------------*/
#define C_S_SUPER_REQUEST_LOOK							120				//超端看牌
#define S_C_SUPER_REQUEST_LOOK_RESULT					121				//超端看牌结果  CMD_S_SuperLookResult
#define C_S_SUPER_REQUEST_CRAD							122				//超端要牌		CMD_C_Super_Request_Card
#define S_C_SUPER_REQUEST_CRAD_RESULT					123				//超端要牌结果  CMD_S_Super_Request_Card_Result
#define C_S_SUPER_GET_WANG_REQ							124				//超端设置发牌时拿几个王请求	CMD_C_SuperGetWangReq
#define S_C_SUPER_GET_WANG_RESP							125				//超端设置发牌时拿几个王返回	CMD_S_SuperGetWangResp

//发牌时拿王请求
struct CMD_C_SuperGetWangReq
{
	BYTE wangNum;                //要几个王
	CMD_C_SuperGetWangReq()
	{
		wangNum = 0;
	}
};

//发牌时拿王返回
struct CMD_S_SuperGetWangResp
{
	BYTE	ECode;                 //错误码：0 成功  1 不是超端玩家  2 王数量不够  3 当前状态不对
	CMD_S_SuperGetWangResp()
	{
		ECode = 0;
	}
};

//返回牌堆数据
struct CMD_S_SuperLookResult
{
	BYTE    byRemainPaiDate[21];		//下标表示牌点数,下标20表示王,值表示剩余张数
	CMD_S_SuperLookResult()
	{
		memset(byRemainPaiDate, 0, sizeof(byRemainPaiDate));
	}
};

//要牌请求
struct CMD_C_Super_Request_Card
{
	BYTE card;                //牌数据
	CMD_C_Super_Request_Card()
	{
		card = 255;
	}
};

//要牌返回
struct CMD_S_Super_Request_Card_Result
{
	BYTE	ECode;                 //错误码：0 成功  1 不是超端玩家  2 没有这张牌
	CMD_S_Super_Request_Card_Result()
	{
		ECode = 0;
	}
};

//通知选漂分
struct S_C_NoticePiao
{
	BYTE						piaoList[PIAO_COUNT];			//漂分列表 (0:不显示 如果未选择,默认一个不漂选项和其他漂N选项)
	BYTE						byZuoXingUser;					//该小局坐醒玩家(4人玩法用)
	S_C_NoticePiao()
	{
		memset(this,0,sizeof(S_C_NoticePiao));
		byZuoXingUser = 255;
	}
};

//选漂分请求
struct C_S_PiaoReq
{
	BYTE						piaoV;							//选择值 (0:不漂)
	C_S_PiaoReq()
	{
		memset(this,0,sizeof(C_S_PiaoReq));
	}
};

//选漂分回复
struct S_C_PiaoResp
{
	BYTE						deskStation;					//座位号
	BYTE						piaoV;							//选择值 (0:不漂)
	S_C_PiaoResp()
	{
		memset(this,0,sizeof(S_C_PiaoResp));
	}
};

//游戏发牌
struct S_C_SendCardData
{
	BYTE						byBankerUser;						//庄家用户
	BYTE						byCurrentUser;						//当前用户
	BYTE						byCardData[PLAY_COUNT][21];			//扑克列表
	BYTE						byCardCount[PLAY_COUNT];			//玩家手牌张数(没有牌用0填充)
	BYTE						byLeftCardCount;
	BYTE						byZuoXingUser;						//该小局坐醒玩家(4人玩法用)
	S_C_SendCardData()
	{
		Init();
		memset(byCardCount,0,sizeof(byCardCount));
		byBankerUser = 255;
		byCurrentUser = 255;
		byZuoXingUser = 255;
	}
	void Init()
	{
		memset(byCardData,0,sizeof(byCardData));
	}
};

//展示一张牌
struct S_C_ShowCard
{
	BYTE byCard;
	BYTE byShowUser;
	BYTE byLeftCardCount;//余下数量
	S_C_ShowCard()
	{
		memset(this,255,sizeof(S_C_ShowCard));
		byLeftCardCount=0;
	}
};

//开始玩游戏
struct S_C_BeginPlay
{
	BYTE    byCard;
	BYTE	byCurrentUser;						//当前用户
	S_C_BeginPlay()
	{
		byCurrentUser = 255;					//当前用户
	}
};

//提牌命令
struct S_C_UserTiCard
{
	BYTE						byActionUser;						//动作用户
	BYTE						byActionCard;						//操作扑克
	BYTE						byRemoveCount;						//删除数目
	BYTE					    byLeftCardCount;					//余下数量
	S_C_UserTiCard()
	{
		memset(this,0,sizeof(S_C_UserTiCard));
	}
};

//跑牌命令
struct S_C_UserPaoCard
{
	BYTE						byActionUser;						//动作用户
	BYTE						byActionCard;						//操作扑克
	BYTE						byRemoveCount;						//删除数目
};

//偎牌命令
struct S_C_UserWeiCard
{
	BYTE						byActionUser;						//动作用户
	BYTE						byActionCard;						//操作扑克
	int							iWeiType;							//偎牌类型
	BYTE					    byLeftCardCount;					//余下数量
	S_C_UserWeiCard()
	{
		memset(this,0,sizeof(S_C_UserWeiCard));
	}
};

//碰牌命令
struct S_C_UserPengCard
{
	BYTE						byActionUser;						//动作用户
	BYTE						byActionCard;						//操作扑克
	S_C_UserPengCard()
	{
		byActionUser = 255;
		byActionCard = 0;
	}
};

//吃牌命令
struct S_C_UserChiCard
{
	BYTE						byActionUser;						//动作用户
	BYTE						byActionCard;						//操作扑克
	BYTE						byResultCount;						//结果数目
	BYTE						byCardData[3][3];					//吃牌组合
	S_C_UserChiCard()
	{
		byActionUser = 255;
		byActionCard = 0;
		byResultCount = 0;
		memset(byCardData,0,sizeof(byCardData));
	}
};

//操作提示
struct S_C_OperateNotify
{
	BYTE						byResumeUser;						//还原用户
	BYTE						byActionCard;						//操作扑克
	int							iOperateCode;						//操作代码
	tagChiCardInfo				tChiCardInfo;						//吃牌时的数据
	S_C_OperateNotify()
	{
		memset(this,0,sizeof(S_C_OperateNotify));
		byResumeUser = 255;
	}
};

//出牌提示
struct S_C_OutCardNotify
{
	bool						bOutCard;							//出牌标志
	BYTE						byCurrentUser;						//当前用户
	BYTE						byOutPai[MAX_INDEX];				//出牌列表
	BYTE						byOutPaiTing[MAX_INDEX];					//出牌后可听胡
	BYTE						byOutPaiTingCards[MAX_INDEX][MAX_INDEX];	//出哪张牌后听哪些牌
	S_C_OutCardNotify()
	{
		bOutCard = false;
		byCurrentUser = 255;
		memset(byOutPai,0,sizeof(byOutPai));
	}
};

//用户出牌结果
struct S_C_OutCard
{
	BYTE						byOutCardUser;						//出牌用户
	BYTE						byOutCardData;						//出牌扑克
	S_C_OutCard()
	{
		byOutCardUser = 255;
		byOutCardData = 255;
	}
};

//发牌命令
struct S_C_SendCard
{
	BYTE						byCardData;							//发牌扑克
	BYTE						byAttachUser;						//绑定用户(发给的当前用户)
	BYTE						byLeftCardCount;					//余下多少张牌
	int							iOperateCode;						//动作属性
	S_C_SendCard()
	{
		memset(this,0,sizeof(S_C_SendCard));
		byAttachUser = 255;
		iOperateCode=0;
	}
};

//更新胡息
struct S_C_UpdataHuXi
{
	BYTE byUserHuXi[PLAY_COUNT];
	S_C_UpdataHuXi()
	{
		memset(byUserHuXi,0,sizeof(byUserHuXi));
	}
};

//游戏结束
struct S_C_GameEnd
{
	//结束信息
	BYTE						byHuCard;							//胡牌扑克
	BYTE						byWinUser;							//胜利用户(无胜利用户则荒庄)

	BYTE						byHuXiCount;						//胡息数目
	int							iXingNum;							//跟醒个数
	int							iSumDun;							//囤数
	int							iSumFan;							//总番数
	int							iAllDun;							//总囤数
	int							iHuType;							//胡牌类型
//	bool						bZiMo;								//当前是否自摸
	int							huType;								//胡类型(0:自摸  1:底胡  2:点炮)

	BYTE						byKingReplaceCard[4];				//王牌代替的牌
	BYTE						byLeftCardData[50];					//余下的牌
	tagWeaveItem				tWeaveItemArray[MAX_WEAVE];			//胡牌数据
	BYTE						byHuXi[MAX_WEAVE];					//各列胡息
	__int64						i64GameScore[PLAY_COUNT];			//游戏积分
	BYTE						byXingPai;							//醒牌
	int							iBaseFen;							//底分
	int							iConSume;							//本局消耗
	BYTE						byCardData[PLAY_COUNT][MAX_COUNT];	//用户扑克
	int							piaoV[PLAY_COUNT];					//漂分值 (-1:不显示 0:不漂)	
	int							piaoWinLose[PLAY_COUNT];			//漂分输赢值
	int							huOperationType;					//胡操作类型(0:无  1:王钓  2:王钓王  3:王闯  4:王闯王  5:王炸)
	int							huOperationFan;						//胡操作番(0:无)
	S_C_GameEnd()
	{
		memset(this,0,sizeof(S_C_GameEnd));
		byWinUser = 255;
	}
};

//-------------------------------------------------------------------------------------
//总战绩
struct TZongResult	
{
	int		iEveryScore[PLAY_COUNT][16];		//每局得分
	BYTE	byWin[PLAY_COUNT];		//胜利局数
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
//王隐藏
struct S_C_WangTake
{
	bool bTake;//是否拿手里
	BYTE byDeskStation;//玩家位置
	S_C_WangTake()
	{
		bTake=true;
	}
};
//动作结果
struct S_C_ActionResult
{
	BYTE byDeskStation;//玩家位置
	int iOperateCode;//动作属性
	S_C_ActionResult()
	{
		byDeskStation=255;
		iOperateCode=0;
	}
};
//判断死守
struct S_C_NotifyForbidCard
{
	BYTE byDeskStation;//玩家位置
	S_C_NotifyForbidCard()
	{
		byDeskStation=255;
	}
};
//////////////////////////////////////////////////////////////////////////
//客户端命令结构
#define C_S_OUT_CARD				11									//出牌命令
#define C_S_OPERATE_CARD			12									//操作扑克
#define C_S_CONTINUE_CARD			13									//继续命令

//临时保保存吃牌数据
struct SaveChiData
{
	BYTE byData[3][3];		//吃牌数据列表
	SaveChiData()
	{
		memset(this,0,sizeof(SaveChiData));
	}
};

//出牌命令
struct C_S_OutCard
{
	BYTE							byCardData;							//扑克数据
	C_S_OutCard()
	{
		byCardData = 255;
	}
};

//操作命令
struct C_S_OperateCard
{
	int								iChiKind;							//吃牌类型
	int								iOperateCode;						//操作代码
	BYTE							byChiData[3][3];					//吃牌的数据
	C_S_OperateCard()
	{
		memset(this,0,sizeof(C_S_OperateCard));
	}
};

//----------------------------------------------------------------------------------
//游戏断线重连结构体

//等待同意
struct GameStation_WaiteAgree
{
	int							iCellScore;							//基础积分
	BYTE						byBeginTime;						//游戏开始时间
	BYTE						byThinkTime;						//思考时间
	BYTE						byBankerUser;						//庄家用户
	GameRuler					tGameRule;							//游戏规则
	BYTE						byPiaoTime;							//漂分阶段时间
	bool						bSuper;								//是否是超端
	GameStation_WaiteAgree()
	{
		memset(this,0,sizeof(GameStation_WaiteAgree));
		byBankerUser = 255;
	}
};

//漂分状态
struct GameStation_PiaoFen
{
	int							iCellScore;							//基础积分
	BYTE						byBeginTime;						//游戏开始时间
	BYTE						byThinkTime;						//思考时间
	BYTE						byBankerUser;						//庄家用户
	GameRuler					tGameRule;							//游戏规则
	BYTE						byPiaoTime;							//漂分阶段时间
	int							piaoV[PLAY_COUNT];					//漂分值 (-1:未选择 0:不漂)
	BYTE						piaoList[PIAO_COUNT];				//漂分列表 (0:不显示 如果未选择,默认一个不漂选项和其他漂N选项)
	BYTE						byZuoXingUser;						//该小局坐醒玩家(4人玩法用)
	bool						bSuper;								//是否是超端
	GameStation_PiaoFen()
	{
		memset(this,0,sizeof(GameStation_PiaoFen));
		byBankerUser = 255;
		byZuoXingUser = 255;
	}
};

//游戏状态
struct GameStation_PlayGame
{
	//游戏信息
	int								iCellScore;									//基础积分
	BYTE							byBeginTime;								//游戏开始时间
	BYTE							byThinkTime;								//思考时间
	BYTE							byRemainingTime;							//余下时间
	BYTE							byBankerUser;								//庄家用户
	BYTE							byCurrentUser;								//当前用户
	//出牌信息
	bool							bOutCard;									//出牌标志
	BYTE							byOutCardUser;								//出牌用户
	BYTE							byOutCardData;								//出牌扑克
	BYTE							byUserOutCard[PLAY_COUNT][OUT_CARD_NUM];	//玩家弃牌数据(出在界面的牌)
	//扑克信息
	BYTE							byLeftCardCount;							//剩余数目
	BYTE							byCardData[MAX_COUNT];						//用户扑克
	BYTE							byUserCardCount[PLAY_COUNT];				//扑克数目
	//组合信息
	BYTE							byCPGTWPCount[PLAY_COUNT];					//组合数目
	tagWeaveItem					tCPGTWPArray[PLAY_COUNT][7];				//组合扑克
	BYTE							byUserHuXiCount[PLAY_COUNT];				//胡息
	//动作信息
	bool							bResponse;									//响应标志
	int								iUserAction;								//用户动作
	tagChiCardInfo					tChiCardInfo;								//吃牌时的数据
	GameRuler						tGameRule;									//游戏规则
	BYTE							byPiaoTime;									//漂分阶段时间
	int								piaoV[PLAY_COUNT];							//漂分值 (-1:不显示 0:不漂)
	BYTE							byOutPaiTing[MAX_INDEX];					//出牌后可听胡
	BYTE							byOutPaiTingCards[MAX_INDEX][MAX_INDEX];	//出哪张牌后听哪些牌
	BYTE							byZuoXingUser;						//该小局坐醒玩家(4人玩法用)
	bool							bSuper;								//是否是超端
	BYTE							byHuPai[MAX_INDEX];					//可胡的牌
	GameStation_PlayGame()
	{
		memset(this,0,sizeof(GameStation_PlayGame));
		byZuoXingUser = 255;
	}
};

//分析子项
struct tagAnalyseItem
{
	BYTE							byHuXiCount;						//胡息数目
	BYTE							byXing;								//醒牌
	int								iXingNum;							//跟醒个数
	int								iSumDun;							//囤数
	int								iSumFan;							//总番数
	int								iAllDun;							//总囤数
	int								iHuType;							//胡牌类型
	bool							bZiMo;								//当前是否自摸
	BYTE							byWeaveCount;						//组合数目
	tagWeaveItem					tWeaveItemArray[MAX_WEAVE];	
	BYTE							byStation;							//胡牌玩家位置
	BYTE							byBeStation;						//被胡牌玩家位置
	bool							bWeaveReplace;						//偎牌替换
	void Init()
	{
		memset(this,0,sizeof(tWeaveItemArray));
		byStation=255;
		byBeStation=255;
		for(int i=0;i<MAX_WEAVE;i++)//++
		{
			tWeaveItemArray[i].Init();
		}
	}
	void AddData(int iType,BYTE byCenterCard,int iCardNum,BYTE byPai[],bool bJing[])
	{
		if(nullptr != byPai && nullptr != bJing)
		{
			for(int i = 0;i < MAX_WEAVE;i ++)
			{
				if(0 == tWeaveItemArray[i].iWeaveKind)
				{
					tWeaveItemArray[i].iWeaveKind = iType;
					tWeaveItemArray[i].byCardCount = iCardNum;
					tWeaveItemArray[i].byCenterCard = byCenterCard;
					memcpy(tWeaveItemArray[i].byCardList,byPai,sizeof(tWeaveItemArray[i].byCardList));
					memcpy(tWeaveItemArray[i].bIsJing,bJing,sizeof(tWeaveItemArray[i].bIsJing));
					byWeaveCount ++;
					break;
				}
			}
		}
	}
	void DeleteData(int iType,BYTE byPai[],bool bJing[])//删除元素
	{
		if(nullptr != byPai && nullptr != bJing)
		{
			for(int i = 0;i < MAX_WEAVE;i ++)
			{
				if(tWeaveItemArray[i].iWeaveKind == iType)
				{
					if(tWeaveItemArray[i].byCardList[0] == byPai[0] && tWeaveItemArray[i].byCardList[1] == byPai[1] &&
						tWeaveItemArray[i].byCardList[2] == byPai[2] && tWeaveItemArray[i].byCardList[3] == byPai[3] &&
						tWeaveItemArray[i].bIsJing[0] == bJing[0] && tWeaveItemArray[i].bIsJing[1] == bJing[1] &&
						tWeaveItemArray[i].bIsJing[2] == bJing[2] && tWeaveItemArray[i].bIsJing[3] == bJing[3])
					{
						byWeaveCount --;
						memset(&tWeaveItemArray[i],0,sizeof(tWeaveItemArray[i]));
						break;
					}
				}
			}
		}
	}
};

#pragma pack()
/********************************************************************************/
#endif
