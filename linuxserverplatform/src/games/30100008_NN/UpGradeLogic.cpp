#include "UpGradeLogic.h"

static unsigned long lAddTime = 0;

//获取扑克花色
BYTE CUpGradeGameLogic::GetCardHuaKind(BYTE iCard)
{ 
	return iCard&UG_HUA_MASK; 
}

//获取扑克大小 （2 - 18 ， 15 以上是主牌 ： 2 - 21 ， 15 以上是主）
int CUpGradeGameLogic::GetCardBulk(BYTE iCard, int bExtVol)
{
	if ((iCard==0x4E)||(iCard==0x4F))
	{
		return bExtVol?(iCard-14):(iCard-62);	//大小鬼64+14-62=16			只返回大小猫的值
	}

	int iCardNum=GetCardNum(iCard);
	int iHuaKind=GetCardHuaKind(iCard);

	if (iCardNum==14)	// A为1点,牛牛中特有	
	{
		if(bExtVol)		
			return ((iHuaKind>>4)+(1*4));
		else
			return 1;
	}
	return ((bExtVol)?((iHuaKind>>4)+(iCardNum*4)):(iCardNum));
}

//排列扑克
int CUpGradeGameLogic::SortCard(BYTE iCardList[], BYTE bUp[], int iCardCount)
{
	int bSorted=TRUE,bTempUp;
	int iTemp,iLast=iCardCount-1,iStationVol[45];

	//获取位置数值
	for (int i=0;i<iCardCount;i++)
	{
		iStationVol[i]=GetCardBulk(iCardList[i],TRUE);
	}

	//排序操作
	do
	{
		bSorted=TRUE;
		for (int i=0;i<iLast;i++)
		{
			if (iStationVol[i]<iStationVol[i+1])
			{	
				//交换位置
				iTemp=iCardList[i];
				iCardList[i]=iCardList[i+1];
				iCardList[i+1]=iTemp;
				iTemp=iStationVol[i];
				iStationVol[i]=iStationVol[i+1];
				iStationVol[i+1]=iTemp;
				if (bUp!=NULL)
				{
					bTempUp=bUp[i];
					bUp[i]=bUp[i+1];
					bUp[i+1]=bTempUp;
				}
				bSorted=FALSE;
			}	
		}
		iLast--;
	} while(!bSorted);
	return true;
}

//比较两手中牌的大小
int CUpGradeGameLogic::CompareCard(BYTE iFirstCard[], int iFirstCount,BYTE iFirstUpCard[], BYTE iSecondCard[], int iSecondCount,BYTE iSecondUpCard[])
{
	if(nullptr == iFirstCard || nullptr == iSecondCard || iFirstCount != 5 || iSecondCount != 5)
	{
		return 0;
	}
	BYTE iFirCard[5],iSecCard[5];
	::memcpy(iFirCard,iFirstCard,sizeof(BYTE)*iFirstCount);
	::memcpy(iSecCard,iSecondCard,sizeof(BYTE)*iSecondCount);

	SortCard(iFirCard,NULL,iFirstCount);
	SortCard(iSecCard,NULL,iSecondCount);
	BYTE iFirstCardShape = GetShape(iFirCard,iFirstCount,iFirstUpCard);
	BYTE iSecondCardShape = GetShape(iSecCard,iSecondCount,iSecondUpCard);
	if (iFirstCardShape != iSecondCardShape) 
	{
		return (iFirstCardShape - iSecondCardShape > 0) ? 1 : -1;
	}
	else
	{
		if (iFirstCardShape == UG_HULU_BULL) //葫芦牛
		{
			return CompareCardToSpecial(iFirCard,iSecCard,3) ? 1 : -1;
		}
		else if (iFirstCardShape == UG_BULL_BOMB) //炸弹牛
		{
			return CompareCardToSpecial(iFirCard,iSecCard,4) ? 1 : -1;
		}
		else if(iFirstCardShape == UG_FIVE_SMALL) //五小牛
		{
			return CompareCardToSpecial(iFirCard,iSecCard,1) ? 1 : -1;
		}

		BYTE MaxFir = GetMaxCard(iFirCard, iFirstCount);
		BYTE MaxSec = GetMaxCard(iSecCard, iSecondCount);
		if (GetCardBulk(MaxFir) != GetCardBulk(MaxSec))
		{
			return (GetCardBulk(MaxFir) - GetCardBulk(MaxSec) > 0) ? 1 : -1;
		}
		else //最大牌也一样就比较最大牌花
		{
			return (GetCardHuaKind(MaxFir) - GetCardHuaKind(MaxSec) > 0) ? 1 : -1;
		}
	}
	return 0;
}

//比较手中牌大小(无牌型时的比较)
int CUpGradeGameLogic::NewCompareCard(BYTE byFirstCard[], int iFirstCount, BYTE bySecondCard[], int iSecondCount)
{
	if(nullptr == byFirstCard || nullptr == bySecondCard)
	{
		return 0;
	}
	BYTE iFirCard[5],iSecCard[5];
	::memcpy(iFirCard,byFirstCard,sizeof(BYTE)*iFirstCount);
	::memcpy(iSecCard,bySecondCard,sizeof(BYTE)*iSecondCount);
	SortCard(iFirCard,NULL,iFirstCount);
	SortCard(iSecCard,NULL,iSecondCount);
	BYTE iFirstCardShape = GetShape(iFirCard,iFirstCount);
	BYTE iSecondCardShape = GetShape(iSecCard,iSecondCount);
	if (iFirstCardShape != iSecondCardShape) 
	{
		return (iFirstCardShape - iSecondCardShape > 0) ? 1 : -1;
	}
	else
	{
		if (iFirstCardShape == UG_HULU_BULL) //葫芦牛
		{
			return CompareCardToSpecial(iFirCard,iSecCard,3) ? 1 : -1;
		}
		else if (iFirstCardShape == UG_BULL_BOMB) //炸弹牛
		{
			return CompareCardToSpecial(iFirCard,iSecCard,4) ? 1 : -1;
		}
		else if(iFirstCardShape == UG_FIVE_SMALL) //五小牛
		{
			return CompareCardToSpecial(iFirCard,iSecCard,1) ? 1 : -1;
		}

		BYTE MaxFir = GetMaxCard(iFirCard, iFirstCount);
		BYTE MaxSec = GetMaxCard(iSecCard, iSecondCount);
		if (GetCardBulk(MaxFir) != GetCardBulk(MaxSec)) //都是牛牛就比较最大牌
		{
			return (GetCardBulk(MaxFir) - GetCardBulk(MaxSec) > 0) ? 1 : -1;
		}
		else //最大牌也一样就比较最大牌花
		{
			return (GetCardHuaKind(MaxFir) - GetCardHuaKind(MaxSec) > 0) ? 1 : -1;
		}
	}
	return 0;
}

//对比单牌
int CUpGradeGameLogic::CompareOnlyOne(BYTE iFirstCard, BYTE iNextCard)
{
	int iFirstNum = GetCardNum(iFirstCard);//上手牌
	int iNextNum  = GetCardNum(iNextCard);//本家牌

	if (iFirstCard == 0x4F) return false;//大王
	if (iNextCard == 0x4F) return true;

	if (iFirstCard == 0x4E) return false;//小王
	if (iNextCard == 0x4E) return true;

	if (iFirstNum == 2) return false;//2
	if (iNextNum == 2) return true;

	return ((iNextNum - iFirstNum) > 0 ? TRUE : FALSE);//其他
}

//混乱扑克
BYTE CUpGradeGameLogic::RandCard(BYTE iCard[], int iCardCount,bool bHaveKing)
{
	static const BYTE gCardArray[54] =
	{
		0x01, 0x02 ,0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, //方块 2 - A
		0x11, 0x12 ,0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, //梅花 2 - A
		0x21, 0x22 ,0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, //红桃 2 - A
		0x31, 0x32 ,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, //黑桃 2 - A
		0x4E, 0x4F //小鬼，大鬼
	};

	BYTE iSend=0,iStation=0,iCardList[216],step=(bHaveKing?54:52);
	memset(iCardList,0,sizeof(iCardList));
	for (int i=0;i<iCardCount;i+=step)
		::memcpy(&iCardList[i],gCardArray,sizeof(gCardArray));

	BYTE temp = 255,data_ = 255;
	int iCount = 3;
	while(--iCount >= 0)
	{
		for(int i=0;i<iCardCount;i++)
		{
			temp = CUtil::GetRandNum()%(iCardCount-i)+i;
			data_ = iCardList[temp];
			iCardList[temp] =iCardList[i];
			iCardList[i] = data_;
		}
	}
	int iIndex = CUtil::GetRandNum()%(iCardCount-20) + 20;
	int iCardIndex = 0;
	for (int i = iIndex; i < iCardCount; i ++)
	{
		iCard[iCardIndex++] = iCardList[i];
	}
	for (int i = 0; i < iIndex; i ++)
	{
		iCard[iCardIndex++] = iCardList[i];
	}
	return iCardCount;
}

int CUpGradeGameLogic::GetPoint(int Card)
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

int CUpGradeGameLogic::GetShape(BYTE iCardList[],int iCardCount,BYTE iUpCard[])
{

	if (m_tGRStruct.bSmallNiu && IsFiveSmall(iCardList,iCardCount))
	{
		return UG_FIVE_SMALL;
	}

	if ( m_tGRStruct.bBombNiu && IsBombBull(iCardList,iCardCount)) 
	{
		return UG_BULL_BOMB;
	}

	if ( m_tGRStruct.bHuLuNiu && IsHuLuBull(iCardList, iCardCount))
	{
		return UG_HULU_BULL;
	}

	if ( m_tGRStruct.bTongHuaNiu && IsTongHuaBull(iCardList, iCardCount))
	{
		return UG_TONGHUA_BULL;
	}

	if ( m_tGRStruct.bHuaNiu && IsGoldBull(iCardList, iCardCount))
	{
		return UG_BULL_GOLD;
	}

	if ( m_tGRStruct.bShunNiu && IsShunBull(iCardList, iCardCount))
	{
		return UG_SHUN_BULL;
	}

	switch(IsHaveNote(iCardList,iCardCount))
	{
	case 0:
		if(IsBullBull(iCardList,iCardCount))
		{
			return UG_BULL_BULL;
		}
		break;
	case 1:
		return UG_BULL_ONE;
	case 2:
		return UG_BULL_TWO;
	case 3:
		return UG_BULL_THREE;
	case 4:
		return UG_BULL_FOUR;
	case 5:
		return UG_BULL_FIVE;
	case 6:
		return UG_BULL_SIX;
	case 7:
		return UG_BULL_SEVEN;
	case 8:
		return UG_BULL_EIGHT;
	case 9:
		return UG_BULL_NINE;
	}
	return UG_NO_POINT;
}

//计算牌中点数
int CUpGradeGameLogic::CountPoint(BYTE  iCardList[],int iCardCount)
{
	int point = 0;
	for (int i=0;i<iCardCount;i++)
	{
		point += GetPoint(iCardList[i]);
	}
	return point;
}

//统计选出指定张数牌是否可以组成20,10,0如果返回为非0值,表示余下点数和,返回0表示不成立
int CUpGradeGameLogic::CanSumIn(BYTE iCardList[],int iCardCount,int iSelectNum)
{
	int total = CountPoint(iCardList,iCardCount);
	for (int i = 0; i < 3; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			for (int k = j + 1; k < 5; k++)
			{
				int temp = GetPoint(iCardList[i])+GetPoint(iCardList[j])+GetPoint(iCardList[k]);
				if (temp == 30 || temp == 20 || temp == 10)
				{
					return total-temp;
				}
			}
		}
	}
	return -1;
}

/// 判断是否牛牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsBullBull(BYTE  byCardList[],int iCardCount)
{
	if(nullptr == byCardList || iCardCount != SH_USER_CARD)
	{
		return false;
	}
	int total=CountPoint(byCardList,iCardCount);
	if ((total > 0) && (total % 10 == 0))
	{
		return true;
	}
	return false;
}

/// 判断是否金牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsGoldBull(BYTE  byCardList[],int iCardCount)
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

/// 判断是否银牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsSilverBull(BYTE  byCardList[],int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}
	for (int i=0; i<iCardCount; i++)
	{
		//银牛是10 J Q K 大小王
		if (GetCardNum(byCardList[i]) != 11 && GetCardNum(byCardList[i]) != 12
			&& GetCardNum(byCardList[i]) != 13 && GetCardNum(byCardList[i]) != 15
			&& GetCardNum(byCardList[i]) != 16 &&  GetCardNum(byCardList[i]) != 10)
		{
			return false;
		}
	}
	return true;
}

/// 判断是否炸弹牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsBombBull(BYTE  byCardList[],int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}
	int temp[17]={0};
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
bool CUpGradeGameLogic::IsFiveSmall(BYTE  byCardList[],int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}
	int iPoint = 0;
	for (int i = 0; i < iCardCount; i++)
	{
		int tmpPoint = GetPoint(byCardList[i]);
		if (tmpPoint > 5) return false;
		iPoint += tmpPoint;
	}
	if (iPoint <= 10)
	{
		return true;
	}
	return false;
}

/// 判断是否顺子牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsShunBull(BYTE  byCardList[],int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}

	int iCardNum[15]={0};
	bool bSameHuaSe = true;
	for (int i=0;i<iCardCount;i++)
	{
		int iNum = GetCardNum(byCardList[i]);
		iCardNum[iNum]++;
		
		if (i != 0)
		{
			if (GetCardHuaKind(byCardList[i]) != GetCardHuaKind(byCardList[i-1]))
			{
				bSameHuaSe = false;
			}
		}
	}

	/*if (bSameHuaSe)
	{
		return false;
	}*/

	int iStart = 255;
	for (int i=0;i<15;i++)
	{
		if (0 != iCardNum[i] && 255 == iStart)
		{
			iStart = i;
		}

		if (255 != iStart && i - iStart < iCardCount)
		{
			if (1 != iCardNum[i])
			{
				if (iStart == 2 && i == 6)//12345情况
				{
					if (1 == iCardNum[14])
					{
						return true;
					}
				}
				return false;
			}
		}
	}

	return true;
}

/// 判断是否同花牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsTongHuaBull(BYTE  byCardList[],int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}

	int iCardNum[15]={0};
	bool bSameHuaSe = true;
	for (int i=0;i<iCardCount;i++)
	{
		int iNum = GetCardNum(byCardList[i]);
		iCardNum[iNum]++;

		if (i != 0)
		{
			if (GetCardHuaKind(byCardList[i]) != GetCardHuaKind(byCardList[i-1]))
			{
				bSameHuaSe = false;
			}
		}
	}

	return bSameHuaSe;
}
/// 判断是否葫芦牛牌型
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsHuLuBull(BYTE  byCardList[],int iCardCount)
{
	if (iCardCount != 5 || nullptr == byCardList)
	{
		return false;
	}

	int iCardNum[14]={0};
	for (int i=0;i<iCardCount;i++)
	{
		int iNum = GetCardNum(byCardList[i]);
		if (14 == iNum)
		{
			iNum = iNum - 13;
		}
		iCardNum[iNum]++;
	}

	bool bHaveThree = false;
	bool bHaveTwo = false;
	for (int i=0;i<14;i++)
	{
		if (3 == iCardNum[i])
		{
			bHaveThree = true;
		}

		if (2 == iCardNum[i])
		{
			bHaveTwo = true;
		}
	}

	if (bHaveThree && bHaveTwo)
	{
		return true;
	}

	return false;
}

///是否为有点
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
int CUpGradeGameLogic::IsHaveNote(BYTE  byCardList[],int iCardCount)
{
	if(nullptr == byCardList)
	{
		return 0;
	}
	int Note = CanSumIn(byCardList, iCardCount, 3);
	return (Note % 10);
}

///得到手牌中最大的牌(含花色)
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
int CUpGradeGameLogic::GetMaxCard(BYTE  byCardList[],int iCardCount)
{
	BYTE card = byCardList[0];
	for (int i = 1; i < iCardCount; i++)
	{
		if (GetCardBulk(card) < GetCardBulk(byCardList[i]))
		{
			card = byCardList[i];
		}
		else if (GetCardBulk(card) == GetCardBulk(byCardList[i]))
		{
			if(GetCardHuaKind(card) < GetCardHuaKind(byCardList[i]))
			{
				card = byCardList[i];
			}
		}
	}
	return card;
}

/// 得到牌组中最小的牌
int CUpGradeGameLogic::GetMinCard(BYTE  byCardList[],int iCardCount)
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
			if(GetCardHuaKind(card) > GetCardHuaKind(byCardList[i]))
			{
				card = byCardList[i];
			}
		}
	}
	return card;
}

/// 判断是否有牛
/// @param BYTE  iCardList[] 牌列表
/// @param int iCardCoun 牌张数
bool CUpGradeGameLogic::IsBull(BYTE  byCardList[],int iCardCount)
{
	if(nullptr == byCardList)
	{
		return false;
	}

	int temp = 0;
	for (int i = 0; i < iCardCount; i++)
	{
		temp += GetPoint(byCardList[i]);
	}
	if (temp == 0)
		return false;
	return temp % 10 == 0;
}

//获取无牛的牌型
bool CUpGradeGameLogic::GetBull(BYTE byCardList[],int iCardCount,BYTE iCardResult[])
{
	if(nullptr == byCardList || iCardCount != 5)
	{
		return false;
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			for (int k = j + 1; k < iCardCount; k++)
			{
				int temp = GetPoint(byCardList[i]) + GetPoint(byCardList[j]) + GetPoint(byCardList[k]);
				if (temp == 30 || temp == 20 || temp == 10)
				{
					iCardResult[0] = byCardList[i];
					iCardResult[1] = byCardList[j];
					iCardResult[2] = byCardList[k];
					return true;
				}
			}
		}
	}
	return false;
}

//获取无牛的牌型数组
bool CUpGradeGameLogic::GetNoBull(BYTE iCardList[],int iCardCount,BYTE iCardResult[])
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = i + 1; j < 4; j++)
		{
			for (int k = j + 1; k < iCardCount; k++)
			{
				int temp = GetPoint(iCardList[i]) + GetPoint(iCardList[j]) + GetPoint(iCardList[k]);
				if (temp == 30 || temp == 20 || temp == 10)
				{
					continue;
				}
				else
				{
					iCardResult[0] = iCardList[i];
					iCardResult[1] = iCardList[j];
					iCardResult[2] = iCardList[k];
					return true;
				}
			}
		}
	}
	iCardResult[0] = iCardList[0];
	iCardResult[1] = iCardList[1];
	iCardResult[2] = iCardList[2];

	return false;
}

// 葫芦牛和炸弹牛比较大小  true：byFirstCard > bySecondCard;
bool CUpGradeGameLogic::CompareCardToSpecial(BYTE byFirstCard[], BYTE bySecondCard[],int iSpecial)
{
	if (byFirstCard == NULL || bySecondCard == NULL)
	{
		return false;
	}

	if (iSpecial == 3 || iSpecial == 4)
	{
		int iCardNum1[14]={0};
		for (int i=0;i<SH_USER_CARD;i++)
		{
			int iNum = GetCardNum(byFirstCard[i]);
			if (14 == iNum)
			{
				iNum = iNum - 13;
			}
			iCardNum1[iNum]++;
		}

		int iCardNum2[14]={0};
		for (int i=0;i<SH_USER_CARD;i++)
		{
			int iNum = GetCardNum(bySecondCard[i]);
			if (14 == iNum)
			{
				iNum = iNum - 13;
			}
			iCardNum2[iNum]++;
		}

		for (int i = 1; i < 14; i ++)
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
		return GetMinCard(byFirstCard,SH_USER_CARD) < GetMinCard(bySecondCard,SH_USER_CARD) ? true : false;
	}

	return true;
}