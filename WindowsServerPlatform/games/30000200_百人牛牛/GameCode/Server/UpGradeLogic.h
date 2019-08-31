#pragma once

//操作掩码
#define	UG_HUA_MASK					0xF0			//1111 0000
#define	UG_VALUE_MASK				0x0F			//0000 1111

#include <vector>

#define MAKE_CARD_SHAPE(x)	CARD_SHAPE_CATTLE_##x

const int POKER_CARD_NUM = 54;       // 扑克牌的张数 
const int POKER_CARD_NUM_NOKING = 52;		// 扑克牌的张数(没有大小王)

static const BYTE g_cardArray[54] =
{
	//2     3     4     5     6     7     8     9     10   J      Q    K     A
	0x01, 0x02 ,0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,		//方块 2 - A
	0x11, 0x12 ,0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,		//梅花 2 - A
	0x21, 0x22 ,0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,		//红桃 2 - A
	0x31, 0x32 ,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,		//黑桃 2 - A
	0x4E, 0x4F
};

static	int g_iRandNum = 0;

enum CardShape
{
	CARD_SHAPE_CATTLE_NONE = 0,		// 无牛
	CARD_SHAPE_CATTLE_1,			// 牛1
	CARD_SHAPE_CATTLE_2,			// 牛2
	CARD_SHAPE_CATTLE_3,			// 牛3
	CARD_SHAPE_CATTLE_4,			// 牛4
	CARD_SHAPE_CATTLE_5,			// 牛5
	CARD_SHAPE_CATTLE_6,			// 牛6
	CARD_SHAPE_CATTLE_7,			// 牛7
	CARD_SHAPE_CATTLE_8,			// 牛8
	CARD_SHAPE_CATTLE_9,			// 牛9
	CARD_SHAPE_CATTLE_CATTLE,		// 牛牛
	CARD_SHAPE_CATTLE_GOLD,			// 五花牛
	CARD_SHAPE_CATTLE_BOMB,			// 炸弹牛
	CARD_SHAPE_CATTLE_SMALL,		// 五小牛
};

class CPokerGameLogic
{
public:
	CPokerGameLogic();
	~CPokerGameLogic();

public:
	//获取扑克数字
	inline int GetCardNum(BYTE iCard) { return (iCard&UG_VALUE_MASK) + 1; }
	// 获取累加的点数
	int GetSumPoints(const BYTE* pCardList, int cardCount);
	// 获取点数
	int GetCardPoint(BYTE card);
	//获取扑克相对大小
	int GetCardBulk(BYTE iCard, int bExtVol = false);
	//获取扑克花色
	BYTE GetCardHuaKind(BYTE iCard);
	// 是否有牛
	bool IsHaveCattle(const BYTE* pCardList, int cardCount);
	// 是否牛牛
	bool IsCattleCattle(const BYTE* pCardList, int cardCount);
	// 获取牌型
	int GetCardShape(const BYTE* pCardList, int cardCount);
	// 洗牌
	bool RandCard(std::vector<BYTE>& cardVec, int cardPairs);
	// （牌型相等的情况下）比较牌列表A和B中最大的牌 true=A>B false=A<B
	bool CompareCardValue(int iCardShape, const BYTE* pCardListA, int cardCountA, const BYTE* pCardListB, int cardCountB);
	// 获取最大的牌
	BYTE GetMaxCardByNumber(const BYTE* pCardList, int cardCount);
	// 获取牌型倍率
	int GetCardShapeRate(int cardShape);
	/// 金牛
	bool IsGoldBull(const BYTE  byCardList[], int iCardCount = 5);
	/// 炸弹牛
	bool IsBombBull(const BYTE  byCardList[], int iCardCount = 5);
	/// 五小
	bool IsFiveSmall(const BYTE  byCardList[], int iCardCount = 5);
	/// 得到牌组中最小的牌
	int GetMinCard(const BYTE  byCardList[], int iCardCount = 5);
	// 葫芦牛和炸弹牛和五小牛比较大小   true：byFirstCard > bySecondCard;
	bool CompareCardToSpecial(const BYTE byFirstCard[], const BYTE bySecondCard[], int iSpecial);
	// 比较牌型大小,  1:A>B   -1:A<B   0:A=B
	int CompareCardShape(const BYTE* pCardListA, int cardCountA, const BYTE* pCardListB, int cardCountB);
};