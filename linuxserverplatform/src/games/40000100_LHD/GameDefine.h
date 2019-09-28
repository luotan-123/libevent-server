#pragma once

#include <list>

#define SYSTEMMONEY 88888888

// 定义游戏中的一些数据
const int AREA_CARD_COUNT = 1;					// 每个位置牌的数量
const int MAX_AREA_COUNT = 3;					// 区域数
const int MAX_SHANGZHUANG_COUNT = 5;			// 上庄列表最大人数
const int MAX_NOTE_SORT = 6;					// 最大筹码种类
const int MAX_AREA_TREND_COUNT = 20;			// 区域走势最大数量
const int MAX_GOLD_ROOM_COUNT = 5;				// 金币场最大数量
const int MAX_DESK_USER = 6;					// 最大座位号
const int MAX_CHOU_MA_COUNT = 100;				// 最大的筹码数量

// 游戏配置结构
struct GameConfig
{
	BYTE randBankerCount;
	BYTE randCount;						//随机坐庄概率
	BYTE CanSitCount;					//机器人最多能坐下多少人
	BYTE GamePlayCount_1;			//根据游戏人数来判断机器人坐下人数
	BYTE GamePlayCount_2;			//根据游戏人数来判断机器人坐下人数
	BYTE GamePlayCount_3;			//根据游戏人数来判断机器人坐下人数

	int waitBeginTime;					// 等待开始时间
	int noteKeepTime;					// 下注阶段持续时间
	int sendCardKeepTime;				// 发牌阶段持续时间
	int compareKeepTime;				// 比牌阶段持续时间
	int waitSettleTime;					// 等待结算时间
	int maxZhuangGameCount;				// 庄家最多进行的局数
	int taxRate;						// 抽水
	int maxDeskCount;					// 最多座位号
	long long shangZhuangLimitMoney;
	long long sitLimitMoney;			// 坐下最少需要金币
	long long noteList[MAX_GOLD_ROOM_COUNT][MAX_NOTE_SORT];
	long long allUserMaxNote;			// 所有玩家(包括机器人)，总共能下注数量

	GameConfig()
	{
		randBankerCount = 1;
		randCount = 1;
		CanSitCount = 4;
		waitBeginTime = 5;
		noteKeepTime = 15;
		sendCardKeepTime = 2;
		compareKeepTime = 8;
		waitSettleTime = 3;
		maxZhuangGameCount = 5;
		taxRate = 5;
		maxDeskCount = 0;

		shangZhuangLimitMoney = 100000;
		sitLimitMoney = 5000;

		noteList[0][0] = 100;
		noteList[0][1] = 200;
		noteList[0][2] = 500;
		noteList[0][3] = 1000;
		noteList[0][4] = 2000;
		noteList[0][5] = 4000;

		noteList[1][0] = 1000;
		noteList[1][1] = 5000;
		noteList[1][2] = 10000;
		noteList[1][3] = 20000;
		noteList[1][4] = 50000;
		noteList[1][5] = 100000;

		noteList[2][0] = 5000;
		noteList[2][1] = 10000;
		noteList[2][2] = 100000;
		noteList[2][3] = 500000;
		noteList[2][4] = 1000000;
		noteList[2][5] = 2000000;
		allUserMaxNote = 0;
	}
};

// 玩家信息
struct DTUserInfo
{
	long userID;								// 玩家ID
	bool isNote;								// 标记是否下注
	bool isContinueNote;						// 是否续压
	long long noteList[MAX_AREA_COUNT];			// 记录当前压注
	long long lastNoteList[MAX_AREA_COUNT];		// 记录上局压注, 用于续压

	DTUserInfo()
	{
		Clear();
	}

	void ClearNoteInfo()
	{
		isContinueNote = false;
		isNote = false;
		memset(noteList, 0, sizeof(noteList));
	}

	void Clear()
	{
		memset(this, 0, sizeof(DTUserInfo));
	}
};

// 庄家信息
struct DealerInfo
{
	bool isSystem;					// 是否系统坐庄	
	long userID;					// 用户ID
	long long shangzhuangMoney;		// 上庄携带金币数

	DealerInfo()
	{
		memset(this, 0, sizeof(DealerInfo));
	}

	void Clear()
	{
		memset(this, 0, sizeof(DealerInfo));
	}
};

#pragma pack(1)
// 趋势
struct TrendData
{
	int type;			// 类型，-1：默认，0：龙，1：虎，2：和
	BYTE count;			// 数量
	TrendData()
	{
		type = -1;
		count = 0;
	}
	TrendData(int type, BYTE count)
	{
		this->type = type;
		this->count = count;
	}
};
#pragma pack()

// 区域信息
struct AreaInfo
{
	long long areaNote;				//区域下注额
	long long areaRealPeopleNote;	//真人下注值
	BYTE cardList[AREA_CARD_COUNT];
	int currWinRate;				// 当前获得的倍率，输了则为负数
	
	AreaInfo()
	{
		Clear();
	}

	void ClearInfo()
	{
		areaNote = 0;
		areaRealPeopleNote = 0;
		memset(cardList, 255, sizeof(cardList));
		currWinRate = 0;
	}

	void Clear()
	{
		areaNote = 0;
		areaRealPeopleNote = 0;
		memset(cardList, 255, sizeof(cardList));
		currWinRate = 0;
	}
};

// 输赢控制相关
struct CtrlSystemParam
{
	int winCount;
	int allCount;
	CtrlSystemParam()
	{
		winCount = 5;
		allCount = 10;
	}
};

// 游戏状态
enum GameStatus
{
	GAME_STATUS_WAITNEXT = 1,		// 等待下局开始
	GAME_STATUS_NOTE,				// 下注阶段
	GAME_STATUS_SENDCARD,			// 发牌阶段
	GAME_STATUS_COMPARE,			// 比牌阶段
	GAME_STATUS_SETTLE,				// 结算阶段
};

// 定时器
enum GameTimer
{
	GAME_TIMER_DEFAULT = 40,
	GAME_TIMER_WAITNEXT,
	GAME_TIMER_NOTE,				// 下注阶段定时器
	GAME_TIMER_NOTEINFO,			// 发送下注信息定时器
	GAME_TIMER_SENDCARD,			// 发牌定时器
	GAME_TIMER_COMPARE,				// 比牌阶段定时器
	GAME_TIMER_SETTLE,				// 结算阶段定时器
	GAME_TIMER_END,
};

#define SYSTEM_DEALER_DESKSTATION	254		// 系统坐庄