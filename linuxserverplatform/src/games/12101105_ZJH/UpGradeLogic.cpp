#include "UpGradeLogic.h"


//构造函数
CUpGradeGameLogic::CUpGradeGameLogic(void)
{
	m_bLogic235ShaBaoZi = false;
	m_byLogicHuiPai = 255;
	m_byTeSuPaiXing = 1;//默认为1
}


//获取扑克花色
BYTE CUpGradeGameLogic::GetCardHuaKind(BYTE iCard)
{ 
	int iHuaKind=(iCard&UG_HUA_MASK);

	return iHuaKind; 
}

//获取扑克大小 （2 - 18 ， 15 以上是主牌 ： 2 - 21 ， 15 以上是主）
int CUpGradeGameLogic::GetCardBulk(BYTE iCard, BOOL bExtVol)
{
	if ((iCard==0x4E)||(iCard==0x4F))
	{
		return bExtVol?(iCard-14):(iCard-62);	//大小鬼64+14-62=16			只返回大小猫的值
	}

	int iCardNum=GetCardNum(iCard);
	int iHuaKind=GetCardHuaKind(iCard);

	return ((bExtVol)?((iHuaKind>>4)+(iCardNum*4)):(iCardNum));
}

//排列扑克
BOOL CUpGradeGameLogic::SortCard(BYTE iCardList[], BYTE bUp[], int iCardCount)
{
	BOOL bSorted=TRUE,bTempUp;
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

	return TRUE;
}



//重新排序
BOOL CUpGradeGameLogic::ReSortCard(BYTE iCardList[], int iCardCount)
{
	SortCard(iCardList,NULL,iCardCount);
	//====按牌形排大小
	int iStationVol[45];
	for (int i=0;i<iCardCount;i++)
	{
		iStationVol[i]=GetCardBulk(iCardList[i],false);
	}

	int Start=0;
	int j,step;
	BYTE CardTemp[8];					//用来保存要移位的牌形
	int CardTempVal[8];					//用来保存移位的牌面值
	for(int i=8;i>1;i--)				//在数组中找一个连续i张相同的值
	{
		for(j=Start;j<iCardCount;j++)
		{
			CardTemp[0]=iCardList[j];			//保存当前i个数组相同的值
			CardTempVal[0]=iStationVol[j];
			for(step=1;step<i&&j+step<iCardCount;)			//找一个连续i个值相等的数组(并保存于临时数组中)
			{
				if(iStationVol[j]==iStationVol[j+step])
				{
					CardTemp[step]=iCardList[j+step];			//用来保存牌形
					CardTempVal[step]=iStationVol[j+step];		//面值
					step++;
				}
				else
					break;
			}

			if(step>=i)			//找到一个连续i个相等的数组串起始位置为j,结束位置为j+setp-1
			{					//将从Start开始到j个数组后移setp个
				if(j!=Start)				//排除开始就是有序
				{
					for(;j>=Start;j--)					//从Start张至j张后移动i张
					{
						iCardList[j+i-1]=iCardList[j-1];
						iStationVol[j+i-1]=iStationVol[j-1];
					}
					for(int k=0;k<i;k++)				
					{
						iCardList[Start+k]=CardTemp[k];	//从Start开始设置成CardSave
						iStationVol[Start+k]=CardTempVal[k];
					}
				}
				Start=Start+i;
			}
			j=j+step-1;
		}
	}
	return true;
}


//获取扑克
BYTE CUpGradeGameLogic::GetCardFromHua(int iHuaKind, int iNum)
{
	if (iHuaKind != UG_ERROR_HUA) return (iHuaKind + iNum - 1);
	return 0;
}

//是否对牌
bool CUpGradeGameLogic::IsDouble(BYTE byCardList[], int iCardCount)
{
	if (iCardCount != 3 || nullptr == byCardList)
	{
		return false;
	}
	int temp[17] = {0},itwo = 0;
	int iHuiNums = 0;
	for(int i = 0;i < iCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai) == GetCardNum(byCardList[i]))
		{
			iHuiNums++;
			continue;
		}
		temp[GetCardNum(byCardList[i])]++;
	}

	for(int i = 0;i < 17;i++)
	{
		if((temp[i] + iHuiNums) == 2)
		{
			itwo++;
			iHuiNums -= 2 - temp[i];
		}
	}
	return (itwo == 1);
}

//是否三条
bool CUpGradeGameLogic::IsThree(BYTE byCardList[], int iCardCount)
{
	if (iCardCount != 3 || nullptr == byCardList)
	{
		return false;
	}
	int temp[17] = {0},ithree = 0;
	int iHuiNums=0;
	for(int i = 0;i < iCardCount;i++)
	{
		if(255 != m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai) == GetCardNum(byCardList[i]))
		{
			iHuiNums++;
			continue;
		}
		temp[GetCardNum(byCardList[i])]++;
	}

	if (iHuiNums >= 2)
	{
		return true;
	}

	for(int i = 0;i < 17;i++)
	{
		if((temp[i] + iHuiNums) == 3)
		{
			ithree++;
			iHuiNums -= 3 - temp[i];
		}
	}
	return (ithree == 1);
}

//是否同花(同花为3张牌为一种花式)
bool CUpGradeGameLogic::IsSameHua(BYTE byCardList[], int iCardCount)
{
	if (iCardCount != 3 || nullptr == byCardList) return false;
	int hs = -1;
	SortCard(byCardList,NULL,iCardCount);
	for (int i = 0;i < iCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(byCardList[i]))
		{
			continue;
		}
		int hua = GetCardHuaKind(byCardList[i]);
		if (hs < 0)
		{
			hs = hua;
			continue;
		}
		if (hs != hua) return false;
	}
	return TRUE;
}

bool CUpGradeGameLogic::IsSpecial(BYTE byCardList[],int iCardCount)
{
	if(iCardCount != 3 || nullptr == byCardList)
	{
		return false;
	}

	int iHuiNums=0;
	bool bEr = false,bSan = false,bWu = false;
	for (int i = 0;i < iCardCount;i++)
	{
		if(255 != m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai) == GetCardNum(byCardList[i]))
		{
			iHuiNums++;
			continue;
		}
		if(GetCardNum(byCardList[i]) == 2)
		{
			bEr=true;
		}
		else if(GetCardNum(byCardList[i]) == 3)
		{
			bSan=true;
		}
		else if(GetCardNum(byCardList[i]) == 5)
		{
			bWu=true;
		}
	}
	if(!bEr && iHuiNums > 0)
	{
		bEr=true;
		iHuiNums--;
	}
	if(!bSan && iHuiNums > 0)
	{
		bSan=true;
		iHuiNums--;
	}
	if(!bWu && iHuiNums > 0)
	{
		bWu=true;
		iHuiNums--;
	}
	if(bEr && bSan && bWu)
	{
		return true;
	}
	return false;
}

//是否同花顺（5张）
bool CUpGradeGameLogic::IsSameHuaContinue(BYTE iCardList[], int iCardCount)
{
	if (iCardCount != 3 || NULL == iCardList) 
	{
		return false;
	}
	if (!IsSameHua(iCardList,iCardCount)) 
	{
		return false;
	}
	if (!IsSingleContinue(iCardList,iCardCount))
	{
		return false;
	}
	return true;
}


//是否单顺(至少3张)
bool CUpGradeGameLogic::IsSingleContinue(BYTE byCardList[], int iCardCount)
{
	if(nullptr == byCardList)
	{
		return false;
	}
	int temp[17];
	int iHuiNums=0;
	memset(temp,0,sizeof(temp));
	for(int i = 0;i < iCardCount;i++)
	{
		if(255 != m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai) == GetCardNum(byCardList[i]))
		{
			iHuiNums++;
			continue;
		}
		temp[GetCardNum(byCardList[i])]++;
	}
	if(iHuiNums >= 2)
	{
		return true;
	}
	if (temp[14] == 1) //特殊的顺子 123
	{
		int huiNums = iHuiNums;
		int iCount_ = 0;
		for(int j = 2;j < 4;j++)
		{
			if(0 == temp[j] && huiNums > 0)
			{
				if (huiNums>0)
				{
					huiNums--;
					iCount_ ++;
				}
				continue;
			}
			if(0 == temp[j] && huiNums<=0 || temp[j] > 1)
			{
				continue;
			}
			iCount_ ++;
		}
		if (iCount_ == 2)
		{
			return true;
		}
	}

	for(int i=1;i<15;i++)
	{
		if(temp[i] == 1)
		{
			for(int j = i + 1;j < i + 3;j++)
			{
				if(0 == temp[j] && iHuiNums > 0)
				{
					iHuiNums--;
					continue;
				}
				if(0 == temp[j] && iHuiNums<=0 || temp[j] > 1)
				{
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

//得到牌列花色
BYTE CUpGradeGameLogic::GetCardListHua(BYTE byCardList[], int iCardCount)
{
	int iHuaKind=GetCardHuaKind(byCardList[0]);
	if (GetCardHuaKind(byCardList[iCardCount-1])!=iHuaKind) return UG_ERROR_HUA;
	return iHuaKind;
}

//获取牌型
BYTE CUpGradeGameLogic::GetCardShape(BYTE byCardList[], int iCardCount)
{
	/***************************************************
	同花顺>铁支>葫芦>同花>顺子>三条>两对>对子>散牌
	***************************************************/

	if (iCardCount <= 0 || nullptr == byCardList) 	return UG_ERROR_KIND;		//非法牌
	if(IsThree(byCardList,iCardCount)) return SH_THREE;							//三条最大
	if(IsSameHuaContinue(byCardList,iCardCount)) return SH_SAME_HUA_CONTINUE;	//同花顺
	if(IsSameHua(byCardList,iCardCount)) return SH_SAME_HUA;					//同花
	if(IsSingleContinue(byCardList,iCardCount)) return SH_CONTINUE;				//顺子
	if(IsDouble(byCardList,iCardCount)) return SH_DOUBLE;						//对子
	if(IsSpecial(byCardList,iCardCount)) return SH_SPECIAL;						//特殊
	return SH_OTHER;														    //散牌
}

//比较两手中牌的大小
int CUpGradeGameLogic::CompareCard(BYTE iFirstCard[], int iFirstCount, BYTE iSecondCard[], int iSecondCount)
{
	if (iFirstCard == NULL || iSecondCard == NULL)
	{
		return -1;
	}
	/***************************************************
	同花顺>铁支>葫芦>同花>顺子>三条>两对>对子>散牌
	***************************************************/

	//豹子>顺金>金花>顺子>对子>特殊>单张
	//for example
	//AAA>黑桃678>黑桃679>黑桃6红桃7方片8>88A>花色不相同的235>三张中没有任何类型相同的牌
	BYTE iFirCard[3],iSecCard[3];

	memcpy(iFirCard,iFirstCard,sizeof(BYTE)*iFirstCount);
	memcpy(iSecCard,iSecondCard,sizeof(BYTE)*iSecondCount);

	ReSortCard(iFirCard,iFirstCount);
	ReSortCard(iSecCard,iSecondCount);

	BYTE iFirstCardKind = GetCardShape(iFirCard,iFirstCount),
		iSecondCardKind = GetCardShape(iSecCard,iSecondCount);

	//类型不同 235吃豹子
	if (iFirstCardKind != iSecondCardKind) 
	{
		if(m_bLogic235ShaBaoZi)
		{
			if (iFirstCardKind == SH_THREE && iSecondCardKind == SH_SPECIAL)
				return -1;
			if (iFirstCardKind == SH_SPECIAL && iSecondCardKind == SH_THREE)
				return 1;
		}
		//DebugPrintf(" /////////特殊牌型的设定  m_byTeSuPaiXing:%d ",m_byTeSuPaiXing);
		if (1 == m_byTeSuPaiXing) // 1 顺子 > 同花 
		{
			//DebugPrintf("-------- 进入 顺子 大于 同花 比较函数 ----------");
			//DebugPrintf("牌型1：%d  牌型2：%d ",iFirstCardKind,iSecondCardKind);
			if (iFirstCardKind == SH_SAME_HUA && iSecondCardKind == SH_CONTINUE)
			{
				//DebugPrintf(" 返回-1 ");
				return -1;
			}	
			if (iFirstCardKind == SH_CONTINUE && iSecondCardKind == SH_SAME_HUA)
			{
				//DebugPrintf(" 返回1 ");
				return 1;
			}
				
		}

		if (2 == m_byTeSuPaiXing) // 2 同花 > 顺子
		{
			//DebugPrintf("-------- 进入  同花  大于 顺子 比较函数 ----------");
			//DebugPrintf("牌型1：%d  牌型2：%d ",iFirstCardKind,iSecondCardKind);
			if (iFirstCardKind == SH_SAME_HUA && iSecondCardKind == SH_CONTINUE)
			{
				//DebugPrintf(" 返回1 ");
				return 1;
			}	
			if (iFirstCardKind == SH_CONTINUE && iSecondCardKind == SH_SAME_HUA)
			{
				//DebugPrintf(" 返回-1 ");
				return -1;
			}
		}
		//DebugPrintf(" --- 其他牌型比较 -- ");
		return (iFirstCardKind - iSecondCardKind > 0) ? 1 : -1;
	}

	//类型相同(先比较最大牌，后比较花色)
	switch (iFirstCardKind)
	{
	case SH_SAME_HUA_CONTINUE://同花顺
	case SH_CONTINUE://顺子
		{
			return SingleContinueCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount);
		}
	case SH_THREE://三条
		{
			return ThreeCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount,false);
		}
	case SH_SAME_HUA://同花
	case SH_OTHER:	//单牌比较
		{
			return OneCardCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount,false);
		}
	case SH_SPECIAL:
		{
			return -1;//who 开牌who输
		}
	case SH_DOUBLE://对子
		{
			return DoubleCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount);
		}
	}
	return -1;
}

//比较两手中牌的大小，一样大返回0
int CUpGradeGameLogic::CompareCardEx(BYTE iFirstCard[], int iFirstCount, BYTE iSecondCard[], int iSecondCount)
{
	if (iFirstCard == NULL || iSecondCard == NULL)
	{
		return -1;
	}

	BYTE iFirCard[3],iSecCard[3];
	memcpy(iFirCard,iFirstCard,sizeof(BYTE)*iFirstCount);
	memcpy(iSecCard,iSecondCard,sizeof(BYTE)*iSecondCount);
	ReSortCard(iFirCard,iFirstCount);
	ReSortCard(iSecCard,iSecondCount);
	BYTE iFirstCardKind = GetCardShape(iFirCard,iFirstCount),
		iSecondCardKind = GetCardShape(iSecCard,iSecondCount);

	//类型不同 235吃豹子  0 顺子>同花  1 同花 > 顺子
	if (iFirstCardKind != iSecondCardKind) 
	{
		if(m_bLogic235ShaBaoZi)
		{
			if (iFirstCardKind == SH_THREE && iSecondCardKind == SH_SPECIAL)
				return -1;
			if (iFirstCardKind == SH_SPECIAL && iSecondCardKind == SH_THREE)
				return 1;
		}
		//DebugPrintf("////*********------到达最大轮数自动结束游戏-------*******////");
		//DebugPrintf("牌型1：%d  牌型2：%d ",iFirstCardKind,iSecondCardKind);
		if (1 == m_byTeSuPaiXing)//顺子>同花
		{
			//DebugPrintf("--------找赢家 进入 顺子 大于 同花 比较函数 ----------");
			if (iFirstCardKind == SH_SAME_HUA && iSecondCardKind == SH_CONTINUE)
			{
				//DebugPrintf("--- 返回 -1 --");
				return -1;
			}
				
			if (iFirstCardKind == SH_CONTINUE && iSecondCardKind == SH_SAME_HUA)
			{
				//DebugPrintf("--- 返回 1 --");
				return 1;
			}
		}
		else if (2 == m_byTeSuPaiXing)// 同花 > 顺子
		{
			//DebugPrintf("--------找赢家 进入  同花  大于 顺子 比较函数 ----------");
			if (iFirstCardKind == SH_SAME_HUA && iSecondCardKind == SH_CONTINUE)
			{
				//DebugPrintf("--- 返回 1 --");
				return 1;
			}

			if (iFirstCardKind == SH_CONTINUE && iSecondCardKind == SH_SAME_HUA)
			{
				//DebugPrintf("--- 返回 -1 --");
				return -1;
			}
		}
		//DebugPrintf("--- 其他牌型比较 --");
		return (iFirstCardKind - iSecondCardKind > 0) ? 1 : -1;
	}

	//类型相同(先比较最大牌，后比较花色)
	switch (iFirstCardKind)
	{
	case SH_SAME_HUA_CONTINUE://同花顺
	case SH_CONTINUE://顺子
		{
			return SingleContinueCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount,true);
		}
	case SH_THREE://三条
		{
			return ThreeCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount,true);
		}
	case SH_SAME_HUA://同花
	case SH_OTHER:	//单牌比较
		{
			return OneCardCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount,true);
		}
	case SH_SPECIAL:
		{
			return 0;//一样大返回0
		}
	case SH_DOUBLE://对子
		{
			return DoubleCompare(iFirstCard,iFirstCount,iSecondCard,iSecondCount,true);
		}
	}
	return 0;
}

//对比单牌
bool CUpGradeGameLogic::CompareOnlyOne(BYTE iFirstCard, BYTE iNextCard)
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

//清除 0 位扑克
int CUpGradeGameLogic::RemoveNummCard(BYTE iCardList[], int iCardCount)
{
	int iRemoveCount=0;
	for (int i=0;i<iCardCount;i++)
	{
		if (iCardList[i]!=0) iCardList[i-iRemoveCount]=iCardList[i];
		else iRemoveCount++;
	}
	return iRemoveCount;
}

//混乱扑克
BYTE CUpGradeGameLogic::RandCard(BYTE iCard[], int iCardCount,int iDeskIndex)
{
	static const BYTE m_CardArray[54]={
		0x01, 0x02 ,0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,		//方块 2 - A
		0x11, 0x12 ,0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,		//梅花 2 - A
		0x21, 0x22 ,0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,		//红桃 2 - A
		0x31, 0x32 ,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,		//黑桃 2 - A
		0x4E, 0x4F
	};																		//小鬼，大鬼

	BYTE iSend=0,iStation=0,iCardList[216];
	memset(iCardList,0,sizeof(iCardList));
	for (int i=0;i<iCardCount;i += 52)
		memcpy(&iCardList[i],m_CardArray,sizeof(m_CardArray)); 

	BYTE temp = 255,data_ = 255;
	int iCount = 3;
	while(--iCount >= 0)
	{
		for(int i=0;i<iCardCount;i++)
		{
			temp = CUtil::GetRandNum() %(iCardCount-i)+i;
			data_ = iCardList[temp];
			iCardList[temp] =iCardList[i];
			iCardList[i] = data_;
		}
	}
	int iIndex = CUtil::GetRandNum() %(iCardCount-20) + 20;
	int iCardIndex = 0;
	for (int i = iIndex; i < iCardCount; i ++)
	{
		iCard[iCardIndex++] = iCardList[i];
	}
	for (int i = 0; i < iIndex; i ++)
	{
		iCard[iCardIndex++] = iCardList[i];
	}
	/*do
	{
	iStation=rand() % (iCardCount-iSend);
	iCard[iSend]=iCardList[iStation];
	iSend++;
	iCardList[iStation]=iCardList[iCardCount-iSend];
	} while (iSend < iCardCount);*/
	return iCardCount;
}

//特殊顺子牌型
BYTE CUpGradeGameLogic::RandCard_ShunZi(BYTE iCard[], int iCardCount,int iDeskIndex)
{
	static const BYTE m_CardArray[54]={
		0x31,0x12,	0x23,	 0x2A, 0x28, 0x27,	 0x04, 0x05, 0x26,	 0x01, 0x08, 0x09,			//方块 2 - A
		0x11, 0x15 ,0x13,	 0x14, 0x15, 0x16,	 0x17, 0x18, 0x19,	 0x1A, 0x1B, 0x1C,	0x1D,		//梅花 2 - A
		0x21, 0x22 ,0x23,	 0x24, 0x25, 0x26,	 0x27, 0x28, 0x29,	 0x2A, 0x2B, 0x2C,	0x2D,		//红桃 2 - A
		0x31, 0x32 ,0x33,	 0x34, 0x35, 0x36,	 0x37, 0x38, 0x39,	 0x3A, 0x3B, 0x3C,	0x3D,		//黑桃 2 - A
		0x4E, 0x4F ,0x0C
	};																		//小鬼，大鬼

	for (int i = 0; i < 52; i ++)
	{
		iCard[i] = m_CardArray[i];
	}
	return iCardCount;
}

//特殊混乱扑克
void CUpGradeGameLogic::RandCard_CrazyMode(BYTE iCard[], int iCardCount)
{
	static const BYTE m_CardArray[24] = {
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,		//方块 2 - A
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,		//梅花 2 - A
		0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,		//红桃 2 - A
		0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,		//黑桃 2 - A
	};																		//小鬼，大鬼

	BYTE iSend = 0, iStation = 0, iCardList[216];
	memset(iCardList, 0, sizeof(iCardList));
	for (int i = 0;i < iCardCount;i += 52)
		memcpy(&iCardList[i], m_CardArray, sizeof(m_CardArray));

	BYTE temp = 255, data_ = 255;
	int iCount = 3;
	while (--iCount >= 0)
	{
		for (int i = 0;i < iCardCount;i++)
		{
			temp = CUtil::GetRandNum() % (iCardCount - i) + i;
			data_ = iCardList[temp];
			iCardList[temp] = iCardList[i];
			iCardList[i] = data_;
		}
	}
	int iIndex = CUtil::GetRandNum() % (iCardCount - 10) + 4;
	int iCardIndex = 0;
	for (int i = iIndex; i < iCardCount; i++)
	{
		iCard[iCardIndex++] = iCardList[i];
	}
	for (int i = 0; i < iIndex; i++)
	{
		iCard[iCardIndex++] = iCardList[i];
	}
}

//删除扑克
int CUpGradeGameLogic::RemoveCard(BYTE iRemoveCard[], int iRemoveCount, BYTE iCardList[], int iCardCount)
{
	//检验数据
	if ((iRemoveCount > iCardCount)) return 0;

	//把要删除的牌置零
	int iDeleteCount=0;
	for (int i = 0;i < iRemoveCount;i++)
	{
		for (int j = 0;j < iCardCount;j++)
		{
			if (iRemoveCard[i] == iCardList[j])
			{
				iDeleteCount++;
				iCardList[j] = 0;
				break;
			}
		}
	}
	RemoveNummCard(iCardList,iCardCount);
	if (iDeleteCount!=iRemoveCount) return 0;

	return iDeleteCount;
}

BYTE  CUpGradeGameLogic::GetMaxCardNum(BYTE byCardList[],int iCardCount)
{
	if(nullptr == byCardList)
	{
		return 0;
	}

	BYTE iTempNum=0;
	for(int i = 0;i < iCardCount;i ++)
	{
		if(GetCardNum(byCardList[i]) >  iTempNum)
		{
			iTempNum = GetCardNum(byCardList[i]);
		}
	}
	return iTempNum;
}

//顺子比大小
int CUpGradeGameLogic::SingleContinueCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff)
{
	int tempFirst[17];
	memset(tempFirst,0,sizeof(tempFirst));
	BYTE byFirstMax=255,bySecondMax=255,byFirstHuiNums=0,bySecondHuiNums=0;
	BYTE byFirstMax2=255,bySecondMax2=255;
	bool bSpeShunFirst = false , bSpeShunSecond = false;
	for(int i=0;i<iFirstCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iFirstCardList[i]))
		{
			byFirstHuiNums++;
			continue;
		}
		tempFirst[GetCardNum(iFirstCardList[i])]++;
	}
	if(3 == byFirstHuiNums)
	{
		byFirstMax=GetCardNum(0x0B);
	}
	else
	{
		for(int i=16;i>=0;i--)
		{
			if(0!=tempFirst[i])
			{
				if (byFirstMax == 255)
				{
					byFirstMax=i;
				}
				else if (byFirstMax2 == 255)
				{
					byFirstMax2 = i;
				}
			}
		}
		if (tempFirst[14] == 1 && tempFirst[3] == 1)
		{
			bSpeShunFirst = true;
		}
	}

	int tempSecond[17];
	memset(tempSecond,0,sizeof(tempSecond));
	for(int i = 0;i < iSecondCardCount;i++)
	{
		if(255 != m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iSecondCardList[i]))
		{
			bySecondHuiNums++;
			continue;
		}
		tempSecond[GetCardNum(iSecondCardList[i])]++;
	}
	if(3 == bySecondHuiNums)
	{
		bySecondMax=GetCardNum(0x0B);
	}
	else
	{
		for(int i=16;i>=0;i--)
		{
			if(0!=tempSecond[i])
			{
				if (bySecondMax == 255)
				{
					bySecondMax=i;
				}
				else if (bySecondMax2 == 255)
				{
					bySecondMax2=i;
				}
			}
		}
		if (tempSecond[14] == 1 && tempSecond[3] == 1)
		{
			bSpeShunSecond = true;
		}
	}

	//////////////////////////////////结果分析////////////////////////////////////////
	if (bSpeShunFirst && bSpeShunSecond)
	{
		if (bDiff)
		{
			return 0;
		}
		return -1;
	}

	if (bSpeShunFirst && !bSpeShunSecond)
	{
		return -1;
	}

	if (!bSpeShunFirst && bSpeShunSecond)
	{
		return 1;
	}

	if(byFirstMax>bySecondMax)
	{
		return 1;
	}
	
	if(byFirstMax<bySecondMax)
	{
		return -1;
	}

	if (byFirstMax2>bySecondMax2)
	{
		return 1;
	}

	if (byFirstMax2<bySecondMax2)
	{
		return -1;
	}

	if (bDiff)
	{
		return 0;
	}

	return -1;
}

//同花、单牌比较
int CUpGradeGameLogic::OneCardCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff)
{
	BYTE byFirstHuiNums=0,bySecondHuiNums=0;
	BYTE byFirstMax=255,bySecondMax=255;
	BYTE byFirstMax2=255,bySecondMax2=255,byFirstMax3=255,bySecondMax3=255;

	int tempFirst[17];
	memset(tempFirst,0,sizeof(tempFirst));
	for(int i=0;i<iFirstCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iFirstCardList[i]))
		{
			byFirstHuiNums++;
			continue;
		}
		tempFirst[GetCardNum(iFirstCardList[i])]++;
	}
	if(1==byFirstHuiNums)
	{
		byFirstMax = 14;
	}
	for(int i=16;i>=0;i--)
	{
		if(0!=tempFirst[i])
		{
			if (byFirstMax == 255)
			{
				byFirstMax=i;
			}
			else if (byFirstMax2 == 255)
			{
				byFirstMax2 = i;
			}
			else
			{
				byFirstMax3 = i;
			}
		}
	}

	int tempSecond[17];
	memset(tempSecond,0,sizeof(tempSecond));
	for(int i=0;i<iSecondCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iSecondCardList[i]))
		{
			bySecondHuiNums++;
			continue;
		}
		tempSecond[GetCardNum(iSecondCardList[i])]++;
	}
	if(1==bySecondHuiNums)
	{
		bySecondMax = 14;
	}
	for(int i=16;i>=0;i--)
	{
		if(0!=tempSecond[i])
		{
			if (bySecondMax == 255)
			{
				bySecondMax=i;
			}
			else if (bySecondMax2 == 255)
			{
				bySecondMax2=i;
			}
			else
			{
				bySecondMax3=i;
			}
		}
	}

	////////////////////////////////结果分析/////////////////////////////////

	if(byFirstMax>bySecondMax)
	{
		return 1;
	}

	if(byFirstMax<bySecondMax)
	{
		return -1;
	}

	if(byFirstMax2>bySecondMax2)
	{
		return 1;
	}

	if(byFirstMax2<bySecondMax2)
	{
		return -1;
	}

	if(byFirstMax3>bySecondMax3)
	{
		return 1;
	}

	if(byFirstMax3<bySecondMax3)
	{
		return -1;
	}

	if (bDiff)
	{
		return 0;
	}

	return -1;
}

//对子比较
int CUpGradeGameLogic::DoubleCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff)
{
	BYTE byFirstHuiNums=0,bySecondHuiNums=0,byFirstMax=255,byFirstMax2=255,bySecondMax=255,bySecondMax2=255;
	//统计比牌玩家
	int tempFirst[17];
	memset(tempFirst,0,sizeof(tempFirst));
	for(int i=0;i<iFirstCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iFirstCardList[i]))
		{
			byFirstHuiNums++;
			continue;
		}
		tempFirst[GetCardNum(iFirstCardList[i])]++;
	}
	if(1==byFirstHuiNums)
	{
		for(int i=16;i>=0;i--)
		{
			if(0!=tempFirst[i])
			{
				if (byFirstMax == 255)
				{
					byFirstMax=i;
				}
				else if (byFirstMax2 == 255)
				{
					byFirstMax2 = i;
				}
			}
		}
	}
	else if(0==byFirstHuiNums)
	{
		for(int i=0;i<17;i++)
		{
			if(2==tempFirst[i])
			{
				byFirstMax=i;
			}
			if (1==tempFirst[i])
			{
				byFirstMax2=i;
			}
		}
	}

	//统计被比牌玩家
	int tempSecond[17];
	memset(tempSecond,0,sizeof(tempSecond));
	for(int i=0;i<iSecondCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iSecondCardList[i]))
		{
			bySecondHuiNums++;
			continue;
		}
		tempSecond[GetCardNum(iSecondCardList[i])]++;
	}
	if(1==bySecondHuiNums)
	{
		for(int i=16;i>=0;i--)
		{
			if(0!=tempSecond[i])
			{
				if (bySecondMax == 255)
				{
					bySecondMax=i;
				}
				else if (bySecondMax2 == 255)
				{
					bySecondMax2=i;
				}
			}
		}
	}
	else if(0==bySecondHuiNums)
	{
		for(int i=0;i<17;i++)
		{
			if(2==tempSecond[i])
			{
				bySecondMax=i;
			}
			if(1==tempSecond[i])
			{
				bySecondMax2=i;
			}
		}
	}

	//结果分析
	if(byFirstMax>bySecondMax)
	{
		return 1;
	}

	if(byFirstMax<bySecondMax)
	{
		return -1;
	}

	if(byFirstMax2>bySecondMax2)
	{
		return 1;
	}

	if(byFirstMax2<bySecondMax2)
	{
		return -1;
	}
	
	if (bDiff)
	{
		return 0;
	}

	return -1;
}

//三条
int CUpGradeGameLogic::ThreeCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff)
{
	BYTE byFirstHuiNums=0,bySecondHuiNums=0;
	BYTE byFirstMax=255,bySecondMax=255;

	int tempFirst[17];
	memset(tempFirst,0,sizeof(tempFirst));
	for(int i=0;i<iFirstCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iFirstCardList[i]))
		{
			byFirstHuiNums++;
			continue;
		}
		tempFirst[GetCardNum(iFirstCardList[i])]++;
	}
	if(3==byFirstHuiNums)
	{
		byFirstMax = 14;
	}
	for(int i=16;i>=0;i--)
	{
		if(0!=tempFirst[i])
		{
			if (byFirstMax == 255)
			{
				byFirstMax=i;
			}
		}
	}

	int tempSecond[17];
	memset(tempSecond,0,sizeof(tempSecond));
	for(int i=0;i<iSecondCardCount;i++)
	{
		if(255!=m_byLogicHuiPai && GetCardNum(m_byLogicHuiPai)==GetCardNum(iSecondCardList[i]))
		{
			bySecondHuiNums++;
			continue;
		}
		tempSecond[GetCardNum(iSecondCardList[i])]++;
	}
	if(3==bySecondHuiNums)
	{
		bySecondMax = 14;
	}
	for(int i=16;i>=0;i--)
	{
		if(0!=tempSecond[i])
		{
			if (bySecondMax == 255)
			{
				bySecondMax=i;
			}
		}
	}

	////////////////////////////////结果分析/////////////////////////////////

	if (0 == byFirstHuiNums && bySecondHuiNums > 0 && bySecondHuiNums < 3)
	{
		return 1;
	}

	if (byFirstHuiNums < 3 && byFirstHuiNums > 0 && 0 == bySecondHuiNums)
	{
		return -1;
	}

	if(byFirstMax>bySecondMax)
	{
		return 1;
	}

	if(byFirstMax<bySecondMax)
	{
		return -1;
	}

	if (bDiff)
	{
		return 0;
	}

	return -1;
}