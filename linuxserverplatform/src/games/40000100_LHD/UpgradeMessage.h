#pragma once

#include "GameDefine.h"

namespace DTProto
{
#pragma pack(1)

	// 游戏名称信息
#define GAMENAME						"龙虎斗"									// 游戏名称
#define GAME_DLL_AUTHOR                 "深圳火妹网络科技"						 //
#define GAME_DLL_NOTE                   "百人类 -- 游戏组件"

#define NAME_ID							40000100								 // 名字 ID

	// 版本定义
#define GAME_MAX_VER					1								        // 现在最高版本
#define GAME_LESS_VER					1								        // 现在最低版本
#define GAME_CHANGE_VER					0								        // 修改版本

	// 支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME

#define PLAY_COUNT						180							        // 游戏人数

	//游戏内部开发版本号
#define DEV_HIGH_VERSION				2				                        // 内部开发高版本号(每修正一次LOW_VERSIONG+1)
#define DEV_LOW_VERSION					1				                        // 内部开发低版本号


	// 新消息定义
#define		MSG_GAME_REQ_SHANGZHUANG		201		// 请求上庄(无结构)
#define		MSG_GAME_RSP_SHANGZHUANG		202		// 回应上庄(无结构)
#define		MSG_GAME_REQ_XIAZHUANG			203		// 请求下庄(无结构)
#define		MSG_GAME_RSP_XIAZHUANG			204		// 回应下庄(无结构)
#define		MSG_GAME_REQ_NOTE				205		// 请求下注 NewGameMessageReqNote
#define		MSG_GAME_RSP_NOTE				206		// 回应下注
#define		MSG_GAME_REQ_CONTINUE_NOTE		207		// 请求续压(无结构)
#define		MSG_GAME_RSP_CONTINUE_NOTE		208		// 回应续压NewGameMessageRspContinueNote
#define		MSG_GAME_REQ_AREA_TREND			209		// 请求走势(无结构)
#define		MSG_GAME_RSP_AREA_TREND			210		// 回应走势NewGameMessageRspAreaTrend
#define		MSG_GAME_REQ_SUPER_SET			211		// 请求超端设置
#define		MSG_GAME_REQ_CONTINUE_ZHUANG	212		// 请求是否继续上庄
#define		MSG_GAME_REQ_SIT				213		// 请求坐下或者站起
#define		MSG_GAME_RSP_SIT				214		// 回应坐下或者站起


#define		MSG_GAME_NTF_GAMESTATUS			220		// 通知游戏状态
#define		MSG_GAME_NTF_ZHUANGINFO			221		// 通知庄信息(包含庄家信息和上庄列表)
#define		MSG_GAME_NTF_NOTEINFO			222		// 通知筹码信息
#define		MSG_GAME_NTF_SENDCARD			223		// 通知发牌信息
#define		MSG_GAME_NTF_SETTLE				224		// 通知结算信息
#define		MSG_GAME_NTF_SIT				225		// 通知玩家坐下或者站起
#define		MSG_GAME_NIF_XUYA				226		// 通知能否续压

	enum ShangZhuangResult
	{
		SHANGZHUANG_RESULT_OK = 0,
		SHANGZHUANG_RESULT_LIST_FULL,	// 上庄列表已满
		SHANGZHUANG_RESULT_MONEY_LESS   //上庄携带金币数不足
	};

	enum NoteResult
	{
		NOTE_RESULT_OK = 0,
	    NOTE_RESULT_OVERRANGING = 1,  //下注的筹码超出庄家的上庄携带金币
		NOTE_RESULT_MONEY_LESS = 2   //玩家身上的金币不足下该注筹码
	};

	struct UserInfo
	{
		int userID;
		long long money;	
		char name[64];
		char headURL[256];//保持用户头像信息
		long long shangzhuangMoney;	//上庄携带金币数
		
		UserInfo()
		{
			memset(this, 0, sizeof(UserInfo));
		}
	};

	struct ChouMaInfo
	{
		BYTE index;			//筹码索引
		BYTE area;			//属于区域
		long long nums;		//筹码值
		BYTE deskStation;	//属于的玩家

		ChouMaInfo()
		{
			index = 255;
			area = 255;
			nums = 0;
			deskStation = 255;
		}
	};

	//请求上庄  MSG_GAME_REQ_SHANGZHUANG
	struct NewGameMessageReqShangZhuang
	{
		long long shangzhuangMoney;	//上庄携带金币数
		NewGameMessageReqShangZhuang()
		{
			shangzhuangMoney = 0;
		}
	};

	//请求上庄响应  MSG_GAME_RSP_SHANGZHUANG
	struct NewGameMessageRspShangZhuang
	{
		int result;

		NewGameMessageRspShangZhuang()
		{
			result = 0;
		}
	};

	// 请求下注 MSG_GAME_REQ_NOTE
	struct NewGameMessageReqNote
	{
		int area;//区域索引
		int noteIndex;//下注的数目

		NewGameMessageReqNote()
		{
			memset(this, 0, sizeof(NewGameMessageReqNote));
		}
	};

	// 请求下注响应 MSG_GAME_RSP_NOTE
	struct NewGameMessageRspNote
	{
		int result;
		long long noteList[MAX_AREA_COUNT];

		NewGameMessageRspNote()
		{
			memset(this, 0, sizeof(NewGameMessageRspNote));
		}
	};	

	// 请求走势 MSG_GAME_REQ_AREA_TREND
	struct NewGameMessageReqAreaTrend
	{
		NewGameMessageReqAreaTrend()
		{
		}
	};

	// 请求走势响应 MSG_GAME_RSP_AREA_TREND
	struct NewGameMessageRspAreaTrend
	{
		TrendData	data[MAX_AREA_TREND_COUNT];
		BYTE		longWin;
		BYTE		huWin;
		int			result[MAX_AREA_TREND_COUNT];// 类型，-1：默认，0：龙，1：虎，2：和

		NewGameMessageRspAreaTrend()
		{
			longWin = 0;
			huWin = 0;
			memset(result, -1, sizeof(result));
		}
	};
	//通知能否续压
	struct NewGameMessageHasXuYa
	{
		BYTE bHasReuqestNote;

		NewGameMessageHasXuYa()
		{
			bHasReuqestNote = false;
		}
	};

	// 通知发牌信息
	struct NewGameMessageNtfSendCard
	{
		BYTE areasCardList[MAX_AREA_COUNT][AREA_CARD_COUNT];	// 区域的牌
		bool isWin[MAX_AREA_COUNT];								// 区域输赢

		NewGameMessageNtfSendCard()
		{
			memset(areasCardList, 255, sizeof(areasCardList));
			memset(isWin, 0, sizeof(isWin));
		}
	};

	// 通知筹码信息(定时器驱动2s一次)
	struct NewGameMessageNtfNoteInfo
	{
		long long	noteList[MAX_AREA_COUNT];			// 区域下注筹码总值
		int chouMaDataCount;							// 筹码数量
		ChouMaInfo chouMaData[MAX_CHOU_MA_COUNT];		// 筹码统计
		
		NewGameMessageNtfNoteInfo()
		{
			memset(this, 0, sizeof(NewGameMessageNtfNoteInfo));
		}
	};

	// 通知游戏状态变化
	struct NewGameMessageNtfGameStatus
	{
		int gameStatus;

		NewGameMessageNtfGameStatus()
		{
			gameStatus = 0;
		}
	};
	
	struct NewGameMessageNtfZhuangInfo
	{
		bool isSystemZhuang;				// 是否系统坐庄
		UserInfo zhuangInfo;				// 庄家信息
		UserInfo shangZhuangList[MAX_SHANGZHUANG_COUNT];		// 上庄列表
		int curZhuangPlayCount;						// 当前局数
		int allZhuangPlayCount;						// 总局数

		NewGameMessageNtfZhuangInfo()
		{
			memset(shangZhuangList,0,sizeof(UserInfo));
			isSystemZhuang = false;
		}
	};

	// 通知结算消息
	struct NewGameMessageNtfSettle
	{
		struct SettleUserInfo
		{
			int userID;
			char name[64];
			char headURL[256];
			long long winMoney;

			SettleUserInfo()
			{
				memset(this, 0, sizeof(SettleUserInfo));
			}
		};

		SettleUserInfo maxWinnerInfo;
		long long dealerWinMoney;
		long long myWinMoney;
		long long dealerMoney;
		BYTE isContinueZhuang;		//提示是否继续坐庄
		long long sitUserWinMoney[MAX_DESK_USER];
		long long sitUserMoney[MAX_DESK_USER];

		NewGameMessageNtfSettle()
		{
			memset(this, 0, sizeof(NewGameMessageNtfSettle));
		}
	};

	// 游戏状态信息
	struct NewGameMessageGameStation
	{
		int cfgWaitBeginKeepTime;				// 等待开始阶段持续时间(s)
		int cfgNoteKeepTime;					// 下注阶段持续时间(s)
		int cfgSendCardKeepTime;				// 发牌阶段持续时间(s)
		int cfgCompareKeepTime;					// 比牌阶段持续时间(s)
		int cfgSettleKeepTime;					// 结算阶段持续时间(s)
		long long cfgShangZhuangLimitMoney;		// 最少多少钱才能上庄
		long long cfgSitLimitMoney;				// 最少多少钱才能坐下
		long long cfgNoteList[MAX_NOTE_SORT];				// 筹码列表
		
		int gameStatus;				// 游戏当前状态
		int nextStageLeftSecs;		// 距离下个阶段的剩余时间
		long long areaNoteList[MAX_AREA_COUNT];	// 区域下注信息
		long long myAreaNoteList[MAX_AREA_COUNT];// 自己下注的信息

		NewGameMessageNtfSendCard sendCardInfo;		// 发牌信息
		NewGameMessageNtfZhuangInfo zhuangInfo;		// 庄信息

		BYTE byIsSuper;								// 是否超端
		int curZhuangPlayCount;						// 当前局数
		int allZhuangPlayCount;						// 总局数

		BYTE deskUser[MAX_DESK_USER];				// 有座位的玩家
		int areaChouMaCount[MAX_AREA_COUNT][MAX_NOTE_SORT];	// 每个区域筹码种类数量

		NewGameMessageGameStation()
		{
			memset(this, 0, sizeof(NewGameMessageGameStation));
		}
	};

	// 请求超端设置
	struct NewGameMessageReqSuperSet
	{
		BYTE bySuperSetType;		//0，默认，1:龙赢，2：虎赢，3：和

		NewGameMessageReqSuperSet()
		{
			memset(this, 0, sizeof(NewGameMessageReqSuperSet));
		}
	};

	// 请求是否继续坐庄
	struct NewGameMessageReqContinueZhuang
	{
		BYTE byType;		//0:下庄，1：继续坐庄

		NewGameMessageReqContinueZhuang()
		{
			memset(this, 0, sizeof(NewGameMessageReqContinueZhuang));
		}
	};

	// 请求坐下或者站起
	struct NewGameMessageReqSit
	{
		BYTE type;		//0坐下，1站起
		BYTE index;		//选择的位置索引，从0开始索引

		NewGameMessageReqSit()
		{
			memset(this, 0, sizeof(NewGameMessageReqSit));
		}
	};

	// 回应坐下或者站起
	struct NewGameMessageRspSit
	{
		BYTE result;	//结果，都是成功
		BYTE type;		//0坐下，1站起
		BYTE index;		//选择的位置索引，从0开始索引

		NewGameMessageRspSit()
		{
			memset(this, 0, sizeof(NewGameMessageRspSit));
		}
	};

	// 通知玩家坐下或者站起
	struct NewGameMessageNtfUserSit
	{
		BYTE type;					// 0坐下，1站起
		BYTE deskStation;			// 玩家座位号
		BYTE index;					// 选择的座位索引
		BYTE deskUser[MAX_DESK_USER];	// 有座位的玩家
		
		NewGameMessageNtfUserSit()
		{
			type = 0;
			deskStation = 255;
			index = 255;
			memset(deskUser, 255, sizeof(deskUser));
		}
	};

#pragma pack()
}
