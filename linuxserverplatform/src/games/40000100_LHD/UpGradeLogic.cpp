#include "GameDefine.h"
#include "UpGradeLogic.h"


CPokerGameLogic::CPokerGameLogic()
{
}

CPokerGameLogic::~CPokerGameLogic()
{

}

//获取扑克大小 （2 - 18 ， 15 以上是主牌 ： 2 - 21 ， 15 以上是主）
int CPokerGameLogic::GetCardBulk(BYTE iCard, int bExtVol)
{
	if ((iCard==0x4E)||(iCard==0x4F))
	{
		return bExtVol?(iCard-14):(iCard-62);	//大小鬼64+14-62=16			只返回大小猫的值
	}

	int iCardNum=GetCardNum(iCard);
	int iHuaKind=GetCardHuaKind(iCard);

	if (iCardNum==14)	// A为1点,牛牛, 龙虎斗中特有	
	{
		if(bExtVol)		
			return ((iHuaKind>>4)+(1*4));
		else
			return 1;
	}
	return ((bExtVol)?((iHuaKind>>4)+(iCardNum*4)):(iCardNum));
}

//获取扑克花色
BYTE CPokerGameLogic::GetCardHuaKind(BYTE iCard)
{ 
	return iCard&UG_HUA_MASK; 
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

//比较牌点数大小 比较牌列表A和B中最大的牌  0=A<B 1=A>B 2=A=B
int CPokerGameLogic::CompareCardValue(const BYTE * pCardListA, int cardCountA, const BYTE * pCardListB, int cardCountB)
{
	if (!pCardListA || !pCardListB)
	{
		return -1;
	}

	if (cardCountA <= 0 || cardCountB <= 0)
	{
		return -1;
	}

	BYTE maxCardA = GetMaxCardByNumber(pCardListA, cardCountA);
	BYTE maxCardB = GetMaxCardByNumber(pCardListB, cardCountB);

	if (GetCardBulk(maxCardA) < GetCardBulk(maxCardB))
	{
		return 0;
	}
	else if (GetCardBulk(maxCardA) > GetCardBulk(maxCardB))
	{
		return 1;
	}
	else
	{
		return 2;
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
			if(GetCardHuaKind(card) < GetCardHuaKind(pCardList[i]))
			{
				card = pCardList[i];
			}
		}
	}
	return card;
}