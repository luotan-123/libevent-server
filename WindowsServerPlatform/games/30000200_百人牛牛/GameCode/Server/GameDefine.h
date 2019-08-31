#pragma once

#include <list>

// 定义游戏中的一些数据
const int AREA_CARD_COUNT = 5;					// 每个位置牌的数量
const int MAX_AREA_COUNT = 4;					// 区域数
const int MAX_SHANGZHUANG_COUNT = 5;			// 上庄列表最大人数
const int MAX_NOTE_SORT = 6;					// 最大筹码种类
const int MAX_AREA_TREND_COUNT = 10;			// 区域走势最大数量
const int MAX_AREA_CARD_TYPE = 5;				// 牌型最大的数量
const int MAX_GOLD_ROOM_COUNT = 5;				// 金币场最大数量
const int MAX_SHAPE_RATE = 4;					// 最大牌型翻倍
const int MAX_DESK_USER = 6;					// 最大座位号
const int MAX_CHOU_MA_COUNT = 100;				// 最大的筹码数量

// 游戏配置结构
struct GameConfig
{
	bool randBankerCount;
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
		noteList[0][0] = 10;
		noteList[0][1] = 20;
		noteList[0][2] = 50;
		noteList[0][3] = 100;
		noteList[0][4] = 200;
		noteList[0][5] = 400;

		noteList[1][0] = 100;
		noteList[1][1] = 500;
		noteList[1][2] = 1000;
		noteList[1][3] = 2000;
		noteList[1][4] = 5000;
		noteList[1][5] = 10000;

		noteList[2][0] = 500;
		noteList[2][1] = 1000;
		noteList[2][2] = 10000;
		noteList[2][3] = 50000;
		noteList[2][4] = 100000;
		noteList[2][5] = 200000;

		randBankerCount = true;
		randCount = 1;
		CanSitCount = 4;
		shangZhuangLimitMoney = 1000000;
		waitBeginTime = 10;
		noteKeepTime = 30;
		sendCardKeepTime = 8;
		compareKeepTime = 5;
		waitSettleTime = 3;
		maxZhuangGameCount = 1;
		taxRate = 5;
		maxDeskCount = 0;
		sitLimitMoney = 5000;
		allUserMaxNote = 0;
	}
};

// 玩家信息
struct BrnnUserInfo
{
	long userID;
	bool isNote;//标记是否下注
	bool isContinueNote;//是否续压
	long long noteList[MAX_AREA_COUNT];
	long long lastNoteList[MAX_AREA_COUNT];

	BrnnUserInfo()
	{
		clear();
	}

	void clearNoteInfo()
	{
		isContinueNote = false;
		isNote = false;
		memset(noteList, 0, sizeof(noteList));
	}

	void clear()
	{
		memset(this, 0, sizeof(BrnnUserInfo));
	}
};

// 庄家信息
struct DealerInfo
{
	bool isSystem;		// 是否系统坐庄	
	long userID;
	BYTE cardList[AREA_CARD_COUNT];

	DealerInfo()
	{
		memset(this, 0, sizeof(DealerInfo));
	}

	void Clear()
	{
		memset(this, 0, sizeof(DealerInfo));
	}
};

// 区域信息
struct AreaInfo
{
	long long areaNote;
	long long areaRealPeopleNote;	//真人下注值
	BYTE cardList[AREA_CARD_COUNT];
	int currWinRate;				// 当前获得的倍率，输了则为负数
	std::list<int> resultList;		// 区域结果，只保存最近的10条
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

	void SetResult(bool isWin)
	{
		if (resultList.size() >= MAX_AREA_TREND_COUNT)
		{
			resultList.pop_back();
		}

		resultList.push_front(isWin);
	}
};

struct ShangZhuangUser
{
	int userID;
	long long money;
	BYTE deskStation;
	ShangZhuangUser()
	{
		memset(this, 0, sizeof(ShangZhuangUser));
		deskStation = 255;
	}
	ShangZhuangUser(int userID, long long money, BYTE deskStation)
	{
		this->userID = userID;
		this->money = money;
		this->deskStation = deskStation;
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