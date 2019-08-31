#include"stdafx.h"
#include "UpGradeLogic.h"

#include <vector>
#include <algorithm>

CPokerGameLogic::CPokerGameLogic()
{
}

CPokerGameLogic::~CPokerGameLogic()
{

}

int CPokerGameLogic::GetSumPoints(const BYTE * pCardList, int cardCount)
{
	if (!pCardList || cardCount != 5)
	{
		return -1;
	}

	int points = 0;
	for (int i = 0; i < cardCount; i++)
	{
		points += GetCardPoint(pCardList[i]);
	}

	return points;
}

int CPokerGameLogic::GetCardPoint(BYTE Card)
{
	if (Card == 0x00)
		return 0;
	switch (GetCardNum(Card))
	{
	case 10:
	case 11:
	case 12:
	case 13:
	case 15:
	case 16:
		return 10;
	case 14:
		return 1;
	default:
		return GetCardNum(Card);
	}
	return 0;
}

//获取扑克大小 （2 - 18 ， 15 以上是主牌 ： 2 - 21 ， 15 以上是主）
int CPokerGameLogic::GetCardBulk(BYTE iCard, int bExtVol)
{
	if ((iCard == 0x4E) || (iCard == 0x4F))
	{
		return bExtVol ? (iCard - 14) : (iCard - 62);	//大小鬼64+14-62=16			只返回大小猫的值
	}

	int iCardNum = GetCardNum(iCard);
	int iHuaKind = GetCardHuaKind(iCard);

	if (iCardNum == 14)	// A为1点,牛牛中特有	
	{
		if (bExtVol)
			return ((iHuaKind >> 4) + (1 * 4));
		else
			return 1;
	}
	return ((bExtVol) ? ((iHuaKind >> 4) + (iCardNum * 4)) : (iCardNum));
}

//获取扑克花色
BYTE CPokerGameLogic::GetCardHuaKind(BYTE iCard)
{
	return iCard&UG_HUA_MASK;
}

bool CPokerGameLogic::IsHaveCattle(const BYTE * pCardList, int cardCount)
{
	if (!pCardList || cardCount != 5)
	{
		return false;
	}

	for (int first = 0; first < cardCount; first++)
	{
		for (int second = 0; second < cardCount; second++)
		{
			if (second == first)
			{
				continue;
			}
			for (int third = 0; third < cardCount; third++)
			{
				if (third == first || third == second)
				{
					continue;
				}

				int num1 = GetCardPoint(pCardList[first]);
				int num2 = GetCardPoint(pCardList[second]);
				int num3 = GetCardPoint(pCardList[third]);

				if ((num1 + num2 + num3) % 10 == 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CPokerGameLogic::IsCattleCattle(const BYTE * pCardList, int cardCount)
{
	return false;
}

int CPokerGameLogic::GetCardShape(const BYTE * pCardList, int cardCount)
{
	if (!pCardList || cardCount != 5)
	{
		return CARD_SHAPE_CATTLE_NONE;
	}

	if (IsFiveSmall(pCardList, cardCount))
	{
		return CARD_SHAPE_CATTLE_SMALL;
	}

	if (IsBombBull(pCardList, cardCount))
	{
		return CARD_SHAPE_CATTLE_BOMB;
	}

	if (IsGoldBull(pCardList, cardCount))
	{
		return CARD_SHAPE_CATTLE_GOLD;
	}

	if (IsHaveCattle(pCardList, cardCount))
	{
		int allPoints = GetSumPoints(pCardList, cardCount);
		if (allPoints <= 0)
		{
			return CARD_SHAPE_CATTLE_NONE;
		}

		int remainder = allPoints % 10;
		if (remainder == 0)	return CARD_SHAPE_CATTLE_CATTLE;
		else if (remainder == 1) return  CARD_SHAPE_CATTLE_1;
		else if (remainder == 2) return  CARD_SHAPE_CATTLE_2;
		else if (remainder == 3) return  CARD_SHAPE_CATTLE_3;
		else if (remainder == 4) return  CARD_SHAPE_CATTLE_4;
		else if (remainder == 5) return  CARD_SHAPE_CATTLE_5;
		else if (remainder == 6) return  CARD_SHAPE_CATTLE_6;
		else if (remainder == 7) return  CARD_SHAPE_CATTLE_7;
		else if (remainder == 8) return  CARD_SHAPE_CATTLE_8;
		else if (remainder == 9) return  CARD_SHAPE_CATTLE_9;
	}

	return CARD_SHAPE_CATTLE_NONE;
}

bool CPokerGameLogic::RandCard(std::vector<BYTE>& cardVec, int cardPairs)
{
	// 添加牌
	for (int pairs = 0; pairs < cardPairs; pairs++)
	{
		for (int i = 0; i < POKER_CARD_NUM_NOKING; i++)
		{
			cardVec.push_back(g_cardArray[i]);
		}
	}

	// 乱序
	random_shuffle(cardVec.begin(), cardVec.end());

	return false;
}

//（牌型相等的情况下）比较牌大小
bool CPokerGameLogic::CompareCardValue(int iCardShape, const BYTE * pCardListA, int cardCountA, const BYTE * pCardListB, int cardCountB)
{
	if (!pCardListA || !pCardListB)
	{
		return false;
	}

	if (cardCountA <= 0 || cardCountB <= 0)
	{
		return false;
	}

	// 牌型一样大，特殊牌型比较大小
	if (iCardShape == CARD_SHAPE_CATTLE_BOMB)
	{
		return CompareCardToSpecial(pCardListA, pCardListB, 4) ? true : false;
	}
	else if (iCardShape == CARD_SHAPE_CATTLE_SMALL)
	{
		return CompareCardToSpecial(pCardListA, pCardListB, 1) ? true : false;
	}

	BYTE maxCardA = GetMaxCardByNumber(pCardListA, cardCountA);
	BYTE maxCardB = GetMaxCardByNumber(pCardListB, cardCountB);

	if (GetCardBulk(maxCardA) != GetCardBulk(maxCardB))
	{
		return (GetCardBulk(maxCardA) - GetCardBulk(maxCardB) > 0) ? true : false;
	}
	else
	{
		return (GetCardHuaKind(maxCardA) - GetCardHuaKind(maxCardB) > 0) ? true : false;
	}
}

BYTE CPokerGameLogic::GetMaxCardByNumber(const BYTE* pCardList, int cardCount)
{
	if (!pCardList || cardCount <= 0)
	{
		return 0xff;
	}

	BYTE card = pCardList[0];
	for (int i = 1; i < cardCount; i++)
	{
		if (GetCardBulk(card) < GetCardBulk(pCardList[i]))
		{
			card = pCardList[i];
		}
		else if (GetCardBulk(card) == GetCardBulk(pCardList[i]))
		{
			if (GetCardHuaKind(card) < GetCardHuaKind(pCardList[i]))
			{
				card = pCardList[i];
			}
		}
	}
	return card;
}

int CPokerGameLogic::GetCardShapeRate(int cardShape)
{
	switch (cardShape)
	{
	case CARD_SHAPE_CATTLE_SMALL:
		return 4;
	case CARD_SHAPE_CATTLE_BOMB:
		return 4;
	case CARD_SHAPE_CATTLE_GOLD:
		return 4;
	case CARD_SHAPE_CATTLE_CATTLE:
		return 3;
	case CARD_SHAPE_CATTLE_9:
		return 2;
	case CARD_SHAPE_CATTLE_8:
		return 2;
	case CARD_SHAPE_CATTLE_7:
		return 2;
	case CARD_SHAPE_CATTLE_6:
		return 1;
	case CARD_SHAPE_CATTLE_5:
		return 1;
	case CARD_SHAPE_CATTLE_4:
		return 1;
	case CARD_SHAPE_CATTLE_3:
		return 1;
	case CARD_SHAPE_CATTLE_2:
		return 1;
	case CARD_SHAPE_CATTLE_1:
		return 1;
	default:
		return 1;
	}

	return 1;
}

/// 判断是否金牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CPokerGameLogic::IsGoldBull(const BYTE  byCardList[], int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}
	for (int i = 0; i < iCardCount; i++)
	{
		//金牛是JQK
		if (GetCardNum(byCardList[i]) != 11 && GetCardNum(byCardList[i]) != 12
			&& GetCardNum(byCardList[i]) != 13)
		{
			return false;
		}
	}
	return true;
}

/// 判断是否炸弹牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CPokerGameLogic::IsBombBull(const BYTE  byCardList[], int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}
	int temp[17] = { 0 };
	for (int i = 0; i < iCardCount; i++)
	{
		temp[GetCardNum(byCardList[i])]++;
	}
	for (int i = 0; i < 17; i++)
	{
		if (temp[i] == 4)
		{
			return true;
		}
	}
	return false;
}

/// 判断是否五小牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CPokerGameLogic::IsFiveSmall(const BYTE  byCardList[], int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}
	int iPoint = 0;
	for (int i = 0; i < iCardCount; i++)
	{
		int tmpPoint = GetCardPoint(byCardList[i]);
		if (tmpPoint > 5) return false;
		iPoint += tmpPoint;
	}
	if (iPoint < 10)
	{
		return true;
	}
	return false;
}

/// 得到牌组中最小的牌
int CPokerGameLogic::GetMinCard(const BYTE  byCardList[], int iCardCount)
{
	BYTE card = byCardList[0];
	for (int i = 1; i < iCardCount; i++)
	{
		if (GetCardBulk(card) > GetCardBulk(byCardList[i]))
		{
			card = byCardList[i];
		}
		else if (GetCardBulk(card) == GetCardBulk(byCardList[i]))
		{
			if (GetCardHuaKind(card) > GetCardHuaKind(byCardList[i]))
			{
				card = byCardList[i];
			}
		}
	}
	return card;
}

// 葫芦牛和炸弹牛比较大小  true：byFirstCard > bySecondCard;
bool CPokerGameLogic::CompareCardToSpecial(const BYTE byFirstCard[], const BYTE bySecondCard[], int iSpecial)
{
	if (byFirstCard == NULL || bySecondCard == NULL)
	{
		return false;
	}

	if (iSpecial == 3 || iSpecial == 4)
	{
		int iCardNum1[14] = { 0 };
		for (int i = 0; i < AREA_CARD_COUNT; i++)
		{
			int iNum = GetCardNum(byFirstCard[i]);
			if (14 == iNum)
			{
				iNum = iNum - 13;
			}
			iCardNum1[iNum]++;
		}

		int iCardNum2[14] = { 0 };
		for (int i = 0; i < AREA_CARD_COUNT; i++)
		{
			int iNum = GetCardNum(bySecondCard[i]);
			if (14 == iNum)
			{
				iNum = iNum - 13;
			}
			iCardNum2[iNum]++;
		}

		for (int i = 1; i < 14; i++)
		{
			if (iCardNum1[i] == iSpecial)
			{
				return false;
			}

			if (iCardNum2[i] == iSpecial)
			{
				return true;
			}
		}
	}
	else if (iSpecial == 1)
	{
		return GetMinCard(byFirstCard, AREA_CARD_COUNT) < GetMinCard(bySecondCard, AREA_CARD_COUNT) ? true : false;
	}

	return true;
}

// 比较牌型大小,  1:A>B   -1:A<B   0:A=B
int CPokerGameLogic::CompareCardShape(const BYTE* pCardListA, int cardCountA, const BYTE* pCardListB, int cardCountB)
{
	int cardShapeA = GetCardShape(pCardListA, cardCountA);
	int cardShapeB = GetCardShape(pCardListB, cardCountB);

	if (cardShapeA > cardShapeB)
	{
		return 1;
	}
	else if (cardShapeA < cardShapeB)
	{
		return -1;
	}
	else
	{
		// 牌型相等，比最大的牌点数
		if (CompareCardValue(cardShapeA, pCardListA, AREA_CARD_COUNT, pCardListB, AREA_CARD_COUNT))
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	return 0;
}