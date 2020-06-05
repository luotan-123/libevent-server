#include "UpGradeLogic.h"
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
BYTE  GetMinNumInThree(int iFirst ,int iSecond ,int iThird)
{
	return min(iFirst,min(iSecond,iThird)) ; 
}

//扑克数据
BYTE const CGameLogic::m_cbyCardDataArray[84]=
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字(1-10)
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字(17-26)
	0x2F,0x2F,0x2F,0x2F													//赖子(47)
};


CGameLogic::CGameLogic(void)
{
	m_iCPGWPTRedCardNum = 0;
	m_byXingPai = 255;
	m_bZiMo = false;
	m_byStation=255;
	m_byBeStation=255;
	m_bWeaveReplace=false;
	memset(m_realJingNum, 0, sizeof(m_realJingNum));
}

CGameLogic::~CGameLogic(void)
{

}
void CGameLogic::Init(GameData *pGameData)
{
	m_pGameData=pGameData;
	memset(m_iMaxDun, 0, sizeof(m_iMaxDun));
	m_bWeaveReplace=false;
}
//混乱扑克
void CGameLogic::RandCardData(BYTE byCardData[], BYTE byMaxCount)
{
	//混乱准备
	BYTE byCardDataTemp[84];
	memcpy(byCardDataTemp,m_cbyCardDataArray,sizeof(BYTE)*byMaxCount);

	//混乱扑克
	BYTE byRandCount = 0,byPosition = 0;
	do
	{
		byPosition = ::rand() % (byMaxCount - byRandCount);
		byCardData[byRandCount++] = byCardDataTemp[byPosition];
		byCardDataTemp[byPosition] = byCardDataTemp[byMaxCount - byRandCount];
	} while (byRandCount < byMaxCount);

	return;
}

//删除扑克
bool CGameLogic::RemoveCard(BYTE byCardIndex[MAX_INDEX], BYTE byRemoveCard)
{
	//删除扑克
	BYTE byRemoveIndex = SwitchToCardIndex(byRemoveCard);
	if (byCardIndex[byRemoveIndex] > 0)
	{
		byCardIndex[byRemoveIndex]--;
		return true;
	}

	return false;
}

//删除扑克
bool CGameLogic::RemoveCard(BYTE byCardIndex[MAX_INDEX], BYTE cbRemoveCard[], BYTE bRemoveCount)
{
	//删除扑克
	for (BYTE i = 0;i < bRemoveCount;i++)
	{
		//删除扑克
		BYTE cbRemoveIndex = SwitchToCardIndex(cbRemoveCard[i]);
		if (byCardIndex[cbRemoveIndex] == 0)
		{
			//还原删除
			for (BYTE j = 0;j < i; j++) 
			{
				byCardIndex[SwitchToCardIndex(cbRemoveCard[j])]++;
			}

			return false;
		}
		else 
		{
			//删除扑克
			--byCardIndex[cbRemoveIndex];
		}
	}

	return true;
}

//删除扑克
bool CGameLogic::RemoveCard(BYTE byCardData[], BYTE byCardCount, BYTE byRemoveCard[], BYTE byRemoveCount)
{
	//定义变量
	BYTE byDeleteCount = 0,byTempCardData[MAX_COUNT];
	if (byCardCount > sizeof(byTempCardData)) return false;
	memcpy(byTempCardData,byCardData,byCardCount * sizeof(byCardData[0]));

	//置零扑克
	for (BYTE i = 0;i < byRemoveCount;i++)
	{
		for (BYTE j = 0;j < byCardCount;j++)
		{
			if (byRemoveCard[i] == byTempCardData[j])
			{
				byDeleteCount++;
				byTempCardData[j] = 0;
				break;
			}
		}
	}

	//成功判断
	if (byDeleteCount != byRemoveCount) 
	{
		return false;
	}

	//清理扑克
	BYTE byCardPos=0;
	for (BYTE i = 0;i < byCardCount;i++)
	{
		if (byTempCardData[i] != 0) byCardData[byCardPos++] = byTempCardData[i];
	}

	return true;
}


//扑克转换
BYTE CGameLogic::SwitchToCardData(BYTE byCardIndex)
{
	return ((byCardIndex / 10) << 4)|(byCardIndex % 10 + 1);
}

//扑克转换
BYTE CGameLogic::SwitchToCardIndex(BYTE byCardData)
{
	if(KINGVALUE == byCardData)
	{
		return 0;
	}
	return ((byCardData & MASK_COLOR) >> 4) * 10 + (byCardData & MASK_VALUE) - 1;
}

//扑克转换
BYTE CGameLogic::SwitchToCardData(BYTE byCardIndex[MAX_INDEX], BYTE byCardData[], BYTE byMaxCount,BYTE byJinNum)
{
	//转换扑克
	BYTE byPosition = 0;
	for(int i = 0;i < byJinNum;i ++)
	{
		byCardData[byPosition++] = 0x2f;
	}
	for (BYTE i = 0;i < MAX_INDEX;i++)
	{
		if (byCardIndex[i] != 0)
		{
			for (BYTE j = 0;j < byCardIndex[i];j++)
			{
				byCardData[byPosition++]=SwitchToCardData(i);
			}
		}
	}
	return byPosition;
}

//扑克转换
BYTE CGameLogic::SwitchToCardIndex(BYTE byCardData[], BYTE byCardCount, BYTE byCardIndex[MAX_INDEX],BYTE &byJinNum)
{
	//设置变量
	memset(byCardIndex,0,sizeof(BYTE)*MAX_INDEX);
	//转换扑克
	for (BYTE i = 0;i < byCardCount;i++)
	{
		if(0x2f != byCardData[i])
		{
			byCardIndex[SwitchToCardIndex(byCardData[i])]++;
		}
		else
		{
			byJinNum ++;
		}
	}

	return byCardCount;
}

//有效判断
bool CGameLogic::IsValidCard(BYTE byCardData)
{
	if(0x02F == byCardData)
	{
		return true;
	}
	BYTE cbValue = (byCardData & MASK_VALUE);
	BYTE cbColor = (byCardData & MASK_COLOR) >> 4;
	return ((cbValue >= 1) && (cbValue <= 10) && (cbColor <= 1));
}

//扑克数目
BYTE CGameLogic::GetCardCount(BYTE byCardIndex[MAX_INDEX],BYTE byJinNum)
{
	//数目统计
	BYTE byCount = 0;
	for (BYTE i = 0;i < MAX_INDEX; i++) byCount += byCardIndex[i];
	byCount += byJinNum;
	return byCount;
}
//检测当前是否有牌
bool CGameLogic::CheckIsHaveCard(bool bCardIndex[MAX_INDEX],BYTE byCurrentCard)
{
	BYTE byCurrentIndex = SwitchToCardIndex(byCurrentCard);
	if(byCurrentIndex < MAX_INDEX)
	{
		return bCardIndex[byCurrentIndex];
	}
	return false;
}
//获取胡息
BYTE CGameLogic::GetWeaveHuXi(const tagWeaveItem & tWeaveItem,bool bWei)
{
	//计算胡息
	switch (tWeaveItem.iWeaveKind)
	{
	case ACK_TI:	//提
		{
			return ((tWeaveItem.byCardList[0] & MASK_COLOR) == 0x10) ? 12 : 9;
		}
	case ACK_PAO:	//跑
		{
			return ((tWeaveItem.byCardList[0] & MASK_COLOR) == 0x10) ? 9 : 6;
		}
	case ACK_WEI:	//偎
	case ACK_WEI_EX://嗅偎
		{
			if(bWei)
			{
				return 3;
			}
			if (tWeaveItem.byCardList[0] == 0x2f) //3个王
			{
				return 6;
			}
			return ((tWeaveItem.byCardList[0] & MASK_COLOR) == 0x10) ? 6 : 3;
		}
	case ACK_PENG:	//碰
		{
			return ((tWeaveItem.byCardList[0] & MASK_COLOR) == 0x10) ? 3 : 1;
		}
	case ACK_CHI:	//吃
		{
			//获取数值
			BYTE cbValue1=tWeaveItem.byCardList[0] & MASK_VALUE;
			BYTE cbValue2=tWeaveItem.byCardList[1] & MASK_VALUE;
			BYTE cbValue3=tWeaveItem.byCardList[2] & MASK_VALUE;
			//一二三吃
			if ((cbValue1 == 1) && (cbValue2 == 2) && (cbValue3 == 3)) return ((tWeaveItem.byCardList[0] & MASK_COLOR) == 0x10) ? 6 : 3;
			//二七十吃
			if ((cbValue1 == 2) && (cbValue2 == 7) && (cbValue3 == 10)) return ((tWeaveItem.byCardList[0] & MASK_COLOR) == 0x10) ? 6 : 3;
			return 0;
		}
	}
	return 0;
}

bool CGameLogic::IsEQS(BYTE byCard)
{
	BYTE byCardValue =  (byCard & MASK_VALUE);
	if(0x02 == byCardValue || 0x07 == byCardValue || 0x0A == byCardValue)
	{
		return true;
	}
	return false;
}

//提牌判断
BYTE CGameLogic::GetAcitonTiCard(BYTE byCardIndex[MAX_INDEX], BYTE byTiCardIndex[5])
{
	//提牌搜索
	BYTE byTiCardCount = 0;
	for (BYTE i = 0;i < MAX_INDEX;i++)
	{
		if (byCardIndex[i] == 4) byTiCardIndex[byTiCardCount++] = i;
	}

	return byTiCardCount;
}

//畏牌判断
BYTE CGameLogic::GetActionWeiCard(BYTE byCardIndex[MAX_INDEX], BYTE byWeiCardIndex[7])
{
	//畏牌搜索
	BYTE byWeiCardCount = 0;
	for (BYTE i = 0;i < MAX_INDEX;i++)
	{
		if (byCardIndex[i] == 3) byWeiCardIndex[byWeiCardCount++] = i;
	}

	return byWeiCardCount;
}

///判断是否能提取指定的牌型
bool  CGameLogic::JudgeCanTrackOutEatType(BYTE byCardIndex[MAX_INDEX],BYTE byEatCardIndex,BYTE byFirstCard,BYTE byCenterCard,BYTE byEndCard,int &iMinCount)
{
	if(!IsValidCard(byFirstCard) || !IsValidCard(byCenterCard) || !IsValidCard(byEndCard))
	{
		return false;
	}

	BYTE byFirstIndex = SwitchToCardIndex(byFirstCard);
	BYTE bySeconIndex = SwitchToCardIndex(byCenterCard);
	BYTE byThreeIndex = SwitchToCardIndex(byEndCard);

	if(byCardIndex[byFirstIndex] <= 0 ||	 
		byCardIndex[bySeconIndex] <= 0 ||
		byCardIndex[byThreeIndex] <= 0)
	{
		return false;
	}

	iMinCount = GetMinNumInThree(byCardIndex[byFirstIndex], byCardIndex[bySeconIndex], byCardIndex[byThreeIndex]);
	return true;
}

//吃牌判断
BYTE CGameLogic::GetActionChiCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard, EatCardMemory MemeoryEatCard[6])
{
	if (byCurrentCard == 0) return 0;								//效验扑克

	BYTE byChiCardCount = 0;
	BYTE byCurrentIndex = SwitchToCardIndex(byCurrentCard);

	if (byCardIndex[byCurrentIndex] >= 3) return byChiCardCount;	//三牌判断

	BYTE byCardIndexTemp[MAX_INDEX];
	for(int i = 0;i < MAX_INDEX;i ++)
	{
		if(byCardIndex[i] >= 3)	{ byCardIndexTemp[i] = 0; }			//三张以上不能拆
		else { byCardIndexTemp[i] = byCardIndex[i];       }
	}
	int iSpecialCount = byCardIndexTemp[byCurrentIndex];
	byCardIndexTemp[byCurrentIndex] ++;								//将吃的牌加进来

	EatData iTempBuildWord[10];   ///最多10种
	int _iThirdCounts[10][10];
	memset(_iThirdCounts,0,sizeof(_iThirdCounts));

	memset(iTempBuildWord,0,sizeof(iTempBuildWord));
	int iBuildTempCount = 0;
	for(int iPos = byCurrentCard - 2;iPos < byCurrentCard + 1;iPos ++)
	{
		int iMinInThree = 0;
		if(JudgeCanTrackOutEatType(byCardIndexTemp,byCurrentIndex,iPos,iPos + 1,iPos + 2,iMinInThree))
		{
			for(int i = 0;i < iMinInThree;i ++)
			{
				iTempBuildWord[iBuildTempCount].byCard[0] = iPos;
				iTempBuildWord[iBuildTempCount].byCard[1] = iPos + 1;
				iTempBuildWord[iBuildTempCount].byCard[2] = iPos + 2;
				iBuildTempCount++;
			}
		}
	}

	if(IsEQS(byCurrentCard))
	{
		bool bBigCard = (byCurrentCard & MASK_COLOR) > 0 ? true : false;
		int iPixValue = (bBigCard == true ? 0x10 : 0);
		int iMinInThree;
		if(JudgeCanTrackOutEatType(byCardIndexTemp,byCurrentIndex,iPixValue + 0x02,iPixValue + 0x07,iPixValue + 0x0A,iMinInThree))
		{
			for(int i = 0;i < iMinInThree;i ++)
			{
				iTempBuildWord[iBuildTempCount].byCard[0] = iPixValue + 0x02;
				iTempBuildWord[iBuildTempCount].byCard[1] = iPixValue + 0x07;
				iTempBuildWord[iBuildTempCount].byCard[2] = iPixValue + 0x0A;
				iBuildTempCount++;
			}
		}
	}

	///拷贝绞牌：：大大小，小小大
	BYTE byCardOther = byCurrentCard > 16 ? (byCurrentCard - 16) : (byCurrentCard + 16);
	BYTE byCardOtherIndex = SwitchToCardIndex(byCardOther);

	int iCardOtherCardCount = byCardIndexTemp[byCardOtherIndex];
	int iCurrentCardCount = byCardIndexTemp[byCurrentIndex];

	if (iCardOtherCardCount >= 2 && iCurrentCardCount >= 2)
	{
		//可能有这两种
		iTempBuildWord[iBuildTempCount].byCard[0] = byCurrentCard;
		iTempBuildWord[iBuildTempCount].byCard[1] = byCardOther;
		iTempBuildWord[iBuildTempCount].byCard[2] = byCardOther;
		iBuildTempCount++;

		iTempBuildWord[iBuildTempCount].byCard[0] = byCurrentCard;
		iTempBuildWord[iBuildTempCount].byCard[1] = byCurrentCard;
		iTempBuildWord[iBuildTempCount].byCard[2] = byCardOther;
		iBuildTempCount++;
	}
	//别的两个，自己手中米有这个，桌面上一个
	else if (iCardOtherCardCount >= 2 && iCurrentCardCount == 1)
	{
		iTempBuildWord[iBuildTempCount].byCard[0] = byCurrentCard;
		iTempBuildWord[iBuildTempCount].byCard[1] = byCardOther;
		iTempBuildWord[iBuildTempCount].byCard[2] = byCardOther;
		iBuildTempCount++;
	}
	//别的1个，手中 有1个或者两个
	else if (iCardOtherCardCount == 1 && iCurrentCardCount >= 2)
	{
		iTempBuildWord[iBuildTempCount].byCard[0] = byCurrentCard;
		iTempBuildWord[iBuildTempCount].byCard[1] = byCurrentCard;
		iTempBuildWord[iBuildTempCount].byCard[2] = byCardOther;
		iBuildTempCount++;
	}

	if(0 == iBuildTempCount)
	{
		return 0;
	}

	int iMemoryCount = 0; //吃牌的时候，比牌的数
	BYTE byLastSave[3];
	memset(byLastSave,0,sizeof(byLastSave));
	BYTE byCardTempArray[MAX_INDEX];
	for(int i = 0;i < iBuildTempCount;i ++)
	{
		int iBuiltSecCount = 0;
		if(byLastSave[0] == iTempBuildWord[i].byCard[0] &&
			byLastSave[1] == iTempBuildWord[i].byCard[1] &&
			byLastSave[2] == iTempBuildWord[i].byCard[2])
		{
			continue;
		}
		memcpy(byCardTempArray,byCardIndexTemp,sizeof(byCardIndexTemp));
		memcpy(byLastSave,iTempBuildWord[i].byCard,sizeof(byLastSave));		//保存,重复不用再检测
		MemeoryEatCard[iMemoryCount].WordArray = iTempBuildWord[i];
		
		for(int s = 0;s < 3;s ++)
		{
			byCardTempArray[SwitchToCardIndex(byLastSave[s])]--;
		}
		BYTE bySecSave[3];
		memset(bySecSave,0,sizeof(bySecSave));
		for(int j = 0;j < iBuildTempCount;j ++)
		{
			if(i == j) continue;
			if(bySecSave[0] == iTempBuildWord[j].byCard[0] &&
				bySecSave[1] == iTempBuildWord[j].byCard[1] &&
				bySecSave[2] == iTempBuildWord[j].byCard[2])
			{
				continue;
			}
			int iBuiltThirdCount = 0;
			BYTE bySecTempArray[MAX_INDEX];
			memcpy(bySecTempArray, byCardTempArray, sizeof(byCardIndexTemp));	//保存数据
			memcpy(bySecSave,iTempBuildWord[j].byCard,sizeof(bySecSave));
			bool failed = false;
			for(int s = 0;s < 3;s ++)
			{
				BYTE byLoaclIndex = SwitchToCardIndex(bySecSave[s]);
				bySecTempArray[byLoaclIndex]--;
				if(bySecTempArray[byLoaclIndex] > 4)
				{ 
					failed = true;
					break;
				}
			}
			if(failed)	continue;
			MemeoryEatCard[iMemoryCount].SecWordArray[iBuiltSecCount] = iTempBuildWord[j];
			iBuiltSecCount++;
			MemeoryEatCard[iMemoryCount].SecCompareArrayCount = iBuiltSecCount;

			BYTE byThirdSave[3];
			memset(byThirdSave,0,sizeof(byThirdSave));
			for (int k = 0; k < iBuildTempCount; k++)
			{
				if (k == i || k == j) continue;

				if( byThirdSave[0] == iTempBuildWord[k].byCard[0] &&
					byThirdSave[1] == iTempBuildWord[k].byCard[1] &&
					byThirdSave[2] == iTempBuildWord[k].byCard[2])
				{
					continue;
				}
				BYTE byThirdTempArray[MAX_INDEX];
				memcpy(byThirdTempArray, bySecTempArray, sizeof(byThirdTempArray));
				memcpy(byThirdSave,iTempBuildWord[k].byCard,sizeof(byThirdSave));
				bool failed = false;
				for(int s = 0;s < 3;s ++)
				{
					BYTE byLoaclIndex = SwitchToCardIndex(byThirdSave[s]);
					byThirdTempArray[byLoaclIndex]--;
					if(byThirdTempArray[byLoaclIndex] > 4)
					{ 
						failed = true;
						break;
					}
				}		
				if(failed)	continue;
				MemeoryEatCard[iMemoryCount].ThirdWordArray[iBuiltSecCount - 1][iBuiltThirdCount] = iTempBuildWord[k];
				_iThirdCounts[i][iBuiltSecCount - 1]++;
				iBuiltThirdCount++;
				MemeoryEatCard[iMemoryCount].ThirdCompareArrayCount = iBuiltThirdCount;
			}
		}
		//-----------------------------------------------------------------------------
		if(iSpecialCount >= 1)
		{
			bool bShallThirdSort = false;
			bool bShallSecSort = false;
			int itemp = iSpecialCount + 1;//加上吃的那张
			for(int k = 0;k < 3;k ++)
			{
				if(byCurrentCard == MemeoryEatCard[iMemoryCount].WordArray.byCard[k])
				{
					itemp --;
				}
			}
			bShallSecSort = false;
			int iSecCount = MemeoryEatCard[iMemoryCount].SecCompareArrayCount;
			for(int j = 0;j < iSecCount;j ++)
			{
				int iSecTemp = itemp;
				for(int k = 0;k < 3;k ++)
				{
					if(byCurrentCard == MemeoryEatCard[iMemoryCount].SecWordArray[j].byCard[k])
					{
						iSecTemp --;
					}
				}
				bShallThirdSort = false;
				int iThirdCount = MemeoryEatCard[iMemoryCount].ThirdCompareArrayCount;
				for(int w = 0;w < iThirdCount;w ++)
				{
					int iThirdTemp = iSecTemp;
					for(int m = 0;m < 3;m ++)
					{
						if(byCurrentCard == MemeoryEatCard[iMemoryCount].ThirdWordArray[j][w].byCard[m])
						{
							iThirdTemp --;
						}
					}
					if(0 != iThirdTemp)
					{
						bShallThirdSort = true;
						memset(&MemeoryEatCard[iMemoryCount].ThirdWordArray[j][w], 0, sizeof(EatData));
						_iThirdCounts[i][j]--;
					}
				}
				if(bShallThirdSort)
				{
					EatData temp;
					for (int w = 0; w < iThirdCount - 1; w++)
					{
						for (int h = 0; h < iThirdCount - 1 - w; h++)
						{
							if (MemeoryEatCard[iMemoryCount].ThirdWordArray[j][h].byCard[0] < MemeoryEatCard[iMemoryCount].ThirdWordArray[j][h + 1].byCard[0])
							{
								//memcpy(&temp, &MemeoryEatCard[iMemoryCount].ThirdWordArray[j][h], sizeof(EatData));
								//memcpy(&MemeoryEatCard[iMemoryCount].ThirdWordArray[j][h], &MemeoryEatCard[iMemoryCount].ThirdWordArray[j][h + 1], sizeof(EatData));
								//memcpy(&MemeoryEatCard[iMemoryCount].ThirdWordArray[j][h + 1], &temp, sizeof(EatData));
							}
						}
					}
				}
				if (_iThirdCounts[i][j] <= 0 && iSecTemp != 0)
				{
					bShallSecSort = true;
					memset(&MemeoryEatCard[iMemoryCount].SecWordArray[j], 0, sizeof(EatData));
					MemeoryEatCard[iMemoryCount].SecCompareArrayCount--;
				}
			}
			if (bShallSecSort)
			{
				EatData temp;
				for (int w = 0; w < iSecCount - 1; w++)
				{
					for (int h = 0; h < iSecCount - 1 - w; h++)
					{
						if (MemeoryEatCard[iMemoryCount].SecWordArray[h].byCard[0] < MemeoryEatCard[iMemoryCount].SecWordArray[h + 1].byCard[0])
						{
							//memcpy(&temp, &MemeoryEatCard[iMemoryCount].SecWordArray[h], sizeof(EatData));
							//memcpy(&MemeoryEatCard[iMemoryCount].SecWordArray[h], &MemeoryEatCard[iMemoryCount].SecWordArray[h + 1], sizeof(EatData));
							//memcpy(&MemeoryEatCard[iMemoryCount].SecWordArray[h + 1], &temp, sizeof(EatData));
						}
					}
				}
			}

			if (itemp != 0 && MemeoryEatCard[iMemoryCount].SecCompareArrayCount == 0)
			{
				memset(&MemeoryEatCard[iMemoryCount], 0, sizeof(EatData));
			}
			else
			{
				iMemoryCount++;
			}
		}
		else
		{
			iMemoryCount++;
		}
	}
	//增加整理代码
	int iSetFirstIndex=0,iSetSecondIndex=0,iSetThirdIndex=0;
	EatCardMemory tSetMemeoryEatCard[6];
	for(int i=0;i<6;i++)
	{
		if(0==MemeoryEatCard[i].WordArray.byCard[0] || 0==MemeoryEatCard[i].WordArray.byCard[1] || 0==MemeoryEatCard[i].WordArray.byCard[2])
		{
			continue;
		}
		tSetMemeoryEatCard[iSetFirstIndex].WordArray=MemeoryEatCard[i].WordArray;
		iSetSecondIndex=0;
		for(int j=0;j<6;j++)
		{
			if(0==MemeoryEatCard[i].SecWordArray[j].byCard[0] || 0==MemeoryEatCard[i].SecWordArray[j].byCard[1] || 0==MemeoryEatCard[i].SecWordArray[j].byCard[2])
			{
				continue;
			}
			tSetMemeoryEatCard[iSetFirstIndex].SecWordArray[iSetSecondIndex]=MemeoryEatCard[i].SecWordArray[j];
			iSetThirdIndex=0;
			for(int k=0;k<6;k++)
			{
				if(0==MemeoryEatCard[i].ThirdWordArray[j][k].byCard[0] || 0==MemeoryEatCard[i].ThirdWordArray[j][k].byCard[1] || 0==MemeoryEatCard[i].ThirdWordArray[j][k].byCard[2])
				{
					continue;
				}
				tSetMemeoryEatCard[iSetFirstIndex].ThirdWordArray[iSetSecondIndex][iSetThirdIndex]=MemeoryEatCard[i].ThirdWordArray[j][k];
				iSetThirdIndex++;
			}
			tSetMemeoryEatCard[iSetFirstIndex].ThirdCompareArrayCount=iSetThirdIndex;
			iSetSecondIndex++;
		}
		tSetMemeoryEatCard[iSetFirstIndex].SecCompareArrayCount=iSetSecondIndex;
		iSetFirstIndex++;
	}
	for(int i=0;i<6;i++)
	{
		MemeoryEatCard[i]=tSetMemeoryEatCard[i];
	}
	return iMemoryCount;
}


//是否吃牌
bool CGameLogic::IsChiCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard)
{
	//效验扑克
	if (byCurrentCard == 0) return false;
	if(byCurrentCard == 0x2f)	return false;				//金牌不能吃
	//构造扑克
	BYTE byCardIndexTemp[MAX_INDEX];
	::memcpy(byCardIndexTemp,byCardIndex,sizeof(byCardIndexTemp));
	//插入扑克
	BYTE byCurrentIndex=SwitchToCardIndex(byCurrentCard);
	byCardIndexTemp[byCurrentIndex]++;
	//提取判断
	while (byCardIndexTemp[byCurrentIndex] > 0)
	{
		BYTE byResult[3];
		if (TakeOutChiCard(byCardIndexTemp,byCurrentCard,byResult) == CK_NULL) break;
	}
	//吃完这张牌之后，没有牌可以出了,那就不能吃了
	BYTE i = 0;
	for (i = 0;i < MAX_INDEX;i++)
	{
		if ((byCardIndexTemp[i] > 0) && (byCardIndexTemp[i] < 3)) break;
	}
	if (i == MAX_INDEX) { return false; }
	return (byCardIndexTemp[byCurrentIndex] == 0);
}

//是否提跑
bool CGameLogic::IsTiPaoCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard)
{
	//效验扑克
	if (byCurrentCard == 0) return false;

	if(byCurrentCard == 0x2f)	return false;				//金牌不能跑

	//转换索引
	BYTE cbCurrentIndex=SwitchToCardIndex(byCurrentCard);

	//提跑判断
	return (byCardIndex[cbCurrentIndex] == 3) ? true : false;
}

//是否偎碰
bool CGameLogic::IsWeiPengCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard)
{
	//效验扑克
	if (byCurrentCard==0) return false;

	if(byCurrentCard == 0x2f)	return false;				//金牌不能碰

	//转换索引
	BYTE byCurrentIndex = SwitchToCardIndex(byCurrentCard);

	//偎碰判断
	return (byCardIndex[byCurrentIndex] == 2) ? true : false;
}
//能否碰牌
bool CGameLogic::IsPengCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard)
{
	//效验扑克
	if (byCurrentCard == 0) return false;

	if(byCurrentCard == 0x2f)	return false;				//金牌不能碰
	//构造扑克
	BYTE byCardIndexTemp[MAX_INDEX];
	::memcpy(byCardIndexTemp,byCardIndex,sizeof(byCardIndexTemp));
	//转换索引
	BYTE byCurrentIndex = SwitchToCardIndex(byCurrentCard);
	byCardIndexTemp[byCurrentIndex] -= 2;

	//碰完这张牌之后，没有牌可以出了,那就不能碰了
	BYTE i = 0;
	for (i = 0;i < MAX_INDEX;i++)
	{
		if ((byCardIndexTemp[i] > 0) && (byCardIndexTemp[i] < 3)) break;
	}
	if (i == MAX_INDEX) { return false; }

	return (byCardIndexTemp[byCurrentIndex] == 0);
}

//胡牌结果
bool CGameLogic::GetHuCardInfo(BYTE byStation,BYTE byBeStation,BYTE byCardIndex[MAX_INDEX], int iJinNum,BYTE byCurrentCard, BYTE byHuXiWeave, tagHuCardInfo tHuCardInfo[],bool bZiMo,tagWeaveItem tWeave[],bool bOutCard, bool bCheckAnyHu)
{
	clock_t start,finish;
	double totaltime;
	start=clock();
	m_AnalyseItemTemp[0].Init();			//临时保存分析的胡牌数据
	m_AnalyseItemTemp[1].Init();
	m_AnalyseItemTemp[2].Init();
	m_AnalyseItemTemp[3].Init();
	m_AnalyseItemTemp[4].Init();
	m_tHuCardInforTemp.Clear();
	m_byStation=byStation;
	m_byBeStation=byBeStation;
	memset(m_iMaxDun, 0, sizeof(m_iMaxDun));
	m_bWeaveReplace=false;
	memcpy(m_tCPTWPArray,tWeave,sizeof(m_tCPTWPArray));
	//构造扑克
	BYTE byCardIndexTemp[MAX_INDEX];
	memcpy(byCardIndexTemp,byCardIndex,sizeof(byCardIndexTemp));
	m_iCPTWPHuXi = byHuXiWeave;	//桌面上的胡息
	m_bZiMo = bZiMo;

	//提取碰牌
	if ((byCurrentCard != 0) && (IsWeiPengCard(byCardIndexTemp,byCurrentCard) == true))
	{
		//判断胡牌	
		if (byCurrentCard != 255)
		{
			byCardIndexTemp[SwitchToCardIndex(byCurrentCard)]++;
		}
		BYTE byJingWeiTemp[MAX_INDEX];
		memcpy(byJingWeiTemp,byCardIndexTemp,sizeof(byJingWeiTemp));
		int iWeiJing=iJinNum;
		if(iJinNum==2)
		{
			for(int iSpace=1;iSpace<=iJinNum;iSpace++)//偎牌替换
			{
				int iTempJing=iJinNum-iSpace;
				memcpy(byCardIndexTemp,byJingWeiTemp,sizeof(byCardIndexTemp));
				m_tHuCardInforTemp.Clear();
				//构造组合
				BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount = 3;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind = ACK_WEI;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard = byCurrentCard;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCurrentCard;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCurrentCard;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCurrentCard;
				for(int i=0;i<iSpace;i++)
				{
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].bIsJing[i]=true;
				}
				//设置胡息
				m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
				//删除扑克
				byCardIndexTemp[SwitchToCardIndex(byCurrentCard)]-=3-iSpace;
				for(int i = 0;i < MAX_INDEX;i ++)
				{
					if(byCardIndexTemp[i] == 3)
					{
						//设置扑克
						byCardIndexTemp[i] = 0;
						//设置组合
						BYTE byCardData = SwitchToCardData(i);
						byIndex = m_tHuCardInforTemp.byWeaveCount++;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

						//设置胡息
						m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
					}
				}
				AnalyseCard(byCardIndexTemp,byCurrentCard,iTempJing);
			}
			iJinNum=iWeiJing;//还原精牌
			m_tHuCardInforTemp.Clear();
			memcpy(byCardIndexTemp,byJingWeiTemp,sizeof(byCardIndexTemp));//还原扑克
		}

		for(int i = 0;i < MAX_INDEX;i ++)//对非摸牌的偎牌提取
		{
			if(SwitchToCardIndex(byCurrentCard)!=i && byCardIndexTemp[i] == 3)//过滤有可能的绞牌
			{
				//设置扑克
				byCardIndexTemp[i] = 0;
				//设置组合
				BYTE byCardData = SwitchToCardData(i);
				BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

				//设置胡息
				m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
			}
		}
		AnalyseCard(byCardIndexTemp,byCurrentCard,iJinNum);

		iJinNum=iWeiJing;//还原精牌
		m_tHuCardInforTemp.Clear();
		memcpy(byCardIndexTemp,byJingWeiTemp,sizeof(byCardIndexTemp));//还原扑克
	}
	else if (byCurrentCard != 0)
	{
		if(0x02f == byCurrentCard)
		{
			iJinNum ++;
		}
		else if (byCurrentCard != 255)
		{
			byCardIndexTemp[SwitchToCardIndex(byCurrentCard)]++;
		}
	}


	//-----------------------------------------------------------------------------------------------------------------
	BYTE byWeiSaveTemp[MAX_INDEX];
	memcpy(byWeiSaveTemp,byCardIndexTemp,sizeof(byWeiSaveTemp));		//保存牌
	int iWeiJing=iJinNum;
	tagHuCardInfo tWeiHuCardInfoTemp;
	memcpy(&tWeiHuCardInfoTemp,&m_tHuCardInforTemp,sizeof(m_tHuCardInforTemp));			//保存上一结构体数据

	for(int i = 0;i < MAX_INDEX;i ++)
	{
		if(byCardIndexTemp[i] == 3)
		{
			//设置扑克
			byCardIndexTemp[i]-=3;

			//设置组合
			BYTE byCardData = SwitchToCardData(i);
			BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;
			//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
			if (byCardData == byCurrentCard && !m_bZiMo)
			{
				m_tHuCardInforTemp.byHuXiCount += 1;
				if ((byCurrentCard & MASK_COLOR) == 0x10)
				{
					m_tHuCardInforTemp.byHuXiCount += 3;
				}
			}
			else
			{
				m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
			}
		}
	}
	AnalyseCard(byCardIndexTemp,byCurrentCard,iJinNum);
	//还原数据
	memcpy(&m_tHuCardInforTemp,&tWeiHuCardInfoTemp,sizeof(m_tHuCardInforTemp));
	memcpy(byCardIndexTemp,byWeiSaveTemp,sizeof(byCardIndexTemp));
	iJinNum=iWeiJing;


	//-----------------------------------------------------------------------------------------------------------------
	BYTE byTiSaveTemp[MAX_INDEX];
	memcpy(byTiSaveTemp,byCardIndexTemp,sizeof(byTiSaveTemp));		//保存牌
	tagHuCardInfo tTiHuCardInfoTemp;
	memcpy(&tTiHuCardInfoTemp,&m_tHuCardInforTemp,sizeof(m_tHuCardInforTemp));			//保存上一结构体数据
	//提取四个一样的牌(提牌)
	for(int i = 0;i < MAX_INDEX;i ++)
	{
		if(byCardIndexTemp[i] == 4)
		{
			//设置扑克
			byCardIndexTemp[i] = 0;

			//设置组合
			BYTE byCardData = SwitchToCardData(i);
			BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 4;
			if((bOutCard && byCurrentCard==byCardData) || (!bZiMo && byCurrentCard==byCardData))
			{
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_PAO;
			}
			else
			{
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_TI;
			}
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;
			m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[3] = byCardData;
			//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
			if (byCardData == byCurrentCard && !m_bZiMo)
			{
				m_tHuCardInforTemp.byHuXiCount += 1;
				if ((byCurrentCard & MASK_COLOR) == 0x10)
				{
					m_tHuCardInforTemp.byHuXiCount += 3;
				}
			}
			else
			{
				m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
			}
		}
	}

	//-----------------------------------------------------------------------------------------------------------------
	if(iJinNum >= 1)
	{
		int iJinNumTemp = iJinNum;
		tagHuCardInfo tHuCardInfoTemp;
		memcpy(&tHuCardInfoTemp,&m_tHuCardInforTemp,sizeof(m_tHuCardInforTemp));			//保存上一结构体数据
		BYTE bySaveCardIndexOneTemp[MAX_INDEX];												
		memcpy(bySaveCardIndexOneTemp,byCardIndexTemp,sizeof(bySaveCardIndexOneTemp));		//保存牌，用于下一个胡牌算法

		tagHuCardInfo tLocalHuCardInfoTemp;
		int iSaveCPTWPHuXi=m_iCPTWPHuXi;
		int iSaveXing=m_iCPTWPXing;
		int iSaveRed=m_iCPGWPTRedCardNum;
		bool bHaveWei=false;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(ACK_WEI==tWeave[i].iWeaveKind || ACK_WEI_EX==tWeave[i].iWeaveKind)
			{
				bHaveWei=true;
				memcpy(byCardIndexTemp,bySaveCardIndexOneTemp,sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
				iJinNum=iJinNumTemp;
				memcpy(&m_tHuCardInforTemp,&tHuCardInfoTemp,sizeof(m_tHuCardInforTemp));
				m_iCPTWPHuXi=iSaveCPTWPHuXi;
				m_iCPTWPXing=iSaveXing;
				m_iCPGWPTRedCardNum=iSaveRed;
				m_bWeaveReplace=false;
				if(iJinNum >= 1)
				{
					//设置组合
					BYTE cbCardData = tWeave[i].byCardList[0];
					BYTE cbIndex=m_tHuCardInforTemp.byWeaveCount++;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardCount = 4;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].iWeaveKind = ACK_TI;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCenterCard = cbCardData;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].bIsJing[0] = true;
					memset(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList,cbCardData,sizeof(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList));
					//设置胡息
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[cbIndex]);
					iJinNum -= 1;
					int iWeaveHuxi=((cbCardData & MASK_COLOR) == 0x10) ? 6 : 3;
					m_iCPTWPHuXi=m_iCPTWPHuXi-iWeaveHuxi;
					BYTE byRedValue =  (cbCardData & MASK_VALUE);
					if(0x02 == byRedValue || 0x07 == byRedValue || 0x0A == byRedValue)
					{
						m_iCPGWPTRedCardNum-=3;
					}
					m_bWeaveReplace=true;
					if(m_byXingPai==cbCardData)
					{
						m_iCPTWPXing-=3;
					}
					m_tCPTWPArray[i].Init();
				}
				else
				{
					continue;
				}
				for(int j = 0;j < MAX_INDEX;j ++)
				{
					if(byCardIndexTemp[j] == 3)
					{
						//设置扑克
						byCardIndexTemp[j] = 0;

						//设置组合 
						BYTE cbCardData=SwitchToCardData(j);
						BYTE cbIndex=m_tHuCardInforTemp.byWeaveCount++;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardCount = 3;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].iWeaveKind = ACK_WEI;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCenterCard = cbCardData;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList[0] = cbCardData;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList[1] = cbCardData;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList[2] = cbCardData;
						//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
						if (cbCardData == byCurrentCard && !m_bZiMo)
						{
							m_tHuCardInforTemp.byHuXiCount += 1;
							if ((byCurrentCard & MASK_COLOR) == 0x10)
							{
								m_tHuCardInforTemp.byHuXiCount += 3;
							}
						}
						else
						{
							m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[cbIndex]);
						}
					}
				}
				AnalyseCard(byCardIndexTemp, byCurrentCard, iJinNum);
			}
		}

		if(bHaveWei)
		{
			memcpy(byCardIndexTemp,bySaveCardIndexOneTemp,sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
			iJinNum = iJinNumTemp;															//恢复精牌数
			memcpy(&m_tHuCardInforTemp,&tHuCardInfoTemp,sizeof(m_tHuCardInforTemp));
			m_iCPTWPHuXi=iSaveCPTWPHuXi;
			m_iCPTWPXing=iSaveXing;
			m_iCPGWPTRedCardNum=iSaveRed;
			m_bWeaveReplace=false;
		}

		for(int i = MAX_INDEX-1;i >= 0;i --)			//大提比小提胡息要多
		{
			memcpy(byCardIndexTemp,bySaveCardIndexOneTemp,sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
			iJinNum=iJinNumTemp;
			memcpy(&m_tHuCardInforTemp,&tHuCardInfoTemp,sizeof(m_tHuCardInforTemp));
			if(byCardIndexTemp[i]<=0)
			{
				continue;
			}
			int tmp = byCardIndexTemp[i];
			for (int k = 1; k <= tmp; ++k)
			{
				memcpy(byCardIndexTemp, bySaveCardIndexOneTemp, sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
				iJinNum = iJinNumTemp;
				memcpy(&m_tHuCardInforTemp, &tHuCardInfoTemp, sizeof(m_tHuCardInforTemp));
				int iSpace = 4 - k;
				if(iSpace>0 && iJinNum>=iSpace)
				{
					//设置扑克
					byCardIndexTemp[i] -= k;

					//设置组合
					BYTE cbCardData = SwitchToCardData(i);
					BYTE cbIndex=m_tHuCardInforTemp.byWeaveCount++;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardCount = 4;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].iWeaveKind = ACK_TI;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCenterCard = cbCardData;
					for(int iAdd=0;iAdd<iSpace;iAdd++)
					{
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].bIsJing[iAdd] = true;
					}
					memset(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList,cbCardData,sizeof(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList));
					//设置胡息
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[cbIndex]);
					iJinNum -=iSpace;
				}

				for(int j = 0;j < MAX_INDEX;j ++)
				{
					if(byCardIndexTemp[j] == 3)
					{
						//设置扑克
						byCardIndexTemp[j] = 0;

						//设置组合 
						BYTE cbCardData=SwitchToCardData(j);
						BYTE cbIndex=m_tHuCardInforTemp.byWeaveCount++;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardCount = 3;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].iWeaveKind = ACK_WEI;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCenterCard = cbCardData;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList[0] = cbCardData;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList[1] = cbCardData;
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList[2] = cbCardData;
						//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
						if (cbCardData == byCurrentCard && !m_bZiMo)
						{
							m_tHuCardInforTemp.byHuXiCount += 1;
							if ((byCurrentCard & MASK_COLOR) == 0x10)
							{
								m_tHuCardInforTemp.byHuXiCount += 3;
							}
						}
						else
						{
							m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[cbIndex]);
						}
					}
				}
				AnalyseCard(byCardIndexTemp,byCurrentCard,iJinNum);
			}
		}
		memcpy(byCardIndexTemp,bySaveCardIndexOneTemp,sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
		iJinNum = iJinNumTemp;															//恢复精牌数
		memcpy(&m_tHuCardInforTemp,&tHuCardInfoTemp,sizeof(m_tHuCardInforTemp));
	}

	//-----------------------------------------------------------------------------------------------------------------
	if (iJinNum >= 1)
	{
		memcpy(byCardIndexTemp, byTiSaveTemp,sizeof(byTiSaveTemp));		//保存牌
		memcpy(&m_tHuCardInforTemp, &tTiHuCardInfoTemp, sizeof(m_tHuCardInforTemp));			//保存上一结构体数据
		for(int i = 0;i < MAX_INDEX;i ++)
		{
			if(SwitchToCardIndex(byCurrentCard) != i && byCardIndexTemp[i] == 4)
			{
				//设置扑克
				byCardIndexTemp[i] -= 3;

				//设置组合
				BYTE byCardData = SwitchToCardData(i);
				BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

				//胡的牌和手上的牌组成3个,自摸时算,别人摸的算3胡
				if (byCardData == byCurrentCard && !m_bZiMo)
				{
					m_tHuCardInforTemp.byHuXiCount += 1;
				}
				else
				{
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
				}
			}
		}

		int iJinNumTemp = iJinNum;
		tagHuCardInfo tHuCardInfoTemp;
		memcpy(&tHuCardInfoTemp, &m_tHuCardInforTemp, sizeof(m_tHuCardInforTemp));			//保存上一结构体数据
		BYTE bySaveCardIndexOneTemp[MAX_INDEX];
		memcpy(bySaveCardIndexOneTemp, byCardIndexTemp, sizeof(bySaveCardIndexOneTemp));		//保存牌，用于下一个胡牌算法

		for (int i = MAX_INDEX - 1; i >= 0; i--)			//大提比小提胡息要多
		{
			memcpy(byCardIndexTemp, bySaveCardIndexOneTemp, sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
			iJinNum = iJinNumTemp;
			memcpy(&m_tHuCardInforTemp, &tHuCardInfoTemp, sizeof(m_tHuCardInforTemp));
			if (byCardIndexTemp[i] <= 0)
			{
				continue;
			}
			int tmp = byCardIndexTemp[i];
			for (int k = 1; k <= tmp; ++k)
			{
				memcpy(byCardIndexTemp, bySaveCardIndexOneTemp, sizeof(bySaveCardIndexOneTemp));	//恢复原来数据
				iJinNum = iJinNumTemp;
				memcpy(&m_tHuCardInforTemp, &tHuCardInfoTemp, sizeof(m_tHuCardInforTemp));
				int iSpace = 4 - k;
				if (iSpace > 0 && iJinNum >= iSpace)
				{
					//设置扑克
					byCardIndexTemp[i] -= k;

					//设置组合
					BYTE cbCardData = SwitchToCardData(i);
					BYTE cbIndex = m_tHuCardInforTemp.byWeaveCount++;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardCount = 4;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].iWeaveKind = ACK_TI;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCenterCard = cbCardData;
					for (int iAdd = 0; iAdd < iSpace; iAdd++)
					{
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].bIsJing[iAdd] = true;
					}
					memset(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList, cbCardData, sizeof(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList));
					//设置胡息
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[cbIndex]);
					iJinNum -= iSpace;

				}

				for(int j = 0;j < MAX_INDEX; j++)
				{
					if(byCardIndexTemp[j] == 3)
					{
						//设置扑克
						byCardIndexTemp[j] = 0;

						//设置组合
						BYTE byCardData = SwitchToCardData(j);
						BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
						m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

						//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
						if (byCardData == byCurrentCard && !m_bZiMo)
						{
							m_tHuCardInforTemp.byHuXiCount += 1;
							if ((byCurrentCard & MASK_COLOR) == 0x10)
							{
								m_tHuCardInforTemp.byHuXiCount += 3;
							}
						}
						else
						{
							m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
						}
					}
				}
				AnalyseCard(byCardIndexTemp, byCurrentCard, iJinNum);
			}
		}
	}
	else
	{
		for(int i = 0;i < MAX_INDEX;i ++)
		{
			if(byCardIndexTemp[i] == 3)
			{
				//设置扑克
				byCardIndexTemp[i] = 0;

				//设置组合
				BYTE byCardData = SwitchToCardData(i);
				BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

				//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
				if (byCardData == byCurrentCard && !m_bZiMo)
				{
					m_tHuCardInforTemp.byHuXiCount += 1;
					if ((byCurrentCard & MASK_COLOR) == 0x10)
					{
						m_tHuCardInforTemp.byHuXiCount += 3;
					}
				}
				else
				{
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
				}
			}
		}
		AnalyseCard(byCardIndexTemp,byCurrentCard,iJinNum);
	}

	if (iJinNum >= 1 && bCheckAnyHu)
	{
		for(int i = 0;i < MAX_INDEX;i ++)
		{
			if(SwitchToCardIndex(byCurrentCard) == i)
			{
				if (byCardIndexTemp[i] == 4)
				{
					//设置扑克
					byCardIndexTemp[i] = 1;

					//设置组合
					BYTE byCardData = SwitchToCardData(i);
					BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
					m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

					//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
					if (byCardData == byCurrentCard && !m_bZiMo)
					{
						m_tHuCardInforTemp.byHuXiCount += 1;
						if ((byCurrentCard & MASK_COLOR) == 0x10)
						{
							m_tHuCardInforTemp.byHuXiCount += 3;
						}
					}
					else
					{
						m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
					}
				}
				else if (byCardIndexTemp[i] == 3)
				{
					//设置扑克
					byCardIndexTemp[i] = 2;

					//设置组合
					BYTE cbCardData = SwitchToCardData(i);
					BYTE cbIndex = m_tHuCardInforTemp.byWeaveCount++;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardCount = 3;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].iWeaveKind = ACK_WEI;
					m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCenterCard = cbCardData;
					for (int iAdd = 0; iAdd < 2; iAdd++)
					{
						m_tHuCardInforTemp.tWeaveItemArray[cbIndex].bIsJing[iAdd] = true;
					}
					memset(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList, cbCardData, sizeof(m_tHuCardInforTemp.tWeaveItemArray[cbIndex].byCardList));
					//设置胡息
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[cbIndex]);
					iJinNum -= 2;
				}
			}
			else if(byCardIndexTemp[i] == 3)
			{
				//设置扑克
				byCardIndexTemp[i] = 0;

				//设置组合
				BYTE byCardData = SwitchToCardData(i);
				BYTE byIndex = m_tHuCardInforTemp.byWeaveCount++;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardCount   = 3;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].iWeaveKind    = ACK_WEI;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCenterCard  = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[0] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[1] = byCardData;
				m_tHuCardInforTemp.tWeaveItemArray[byIndex].byCardList[2] = byCardData;

				//胡的牌和手上的牌组成3个,自摸时算,别人摸的大的算3胡,小的算1胡
				if (byCardData == byCurrentCard && !m_bZiMo)
				{
					m_tHuCardInforTemp.byHuXiCount += 1;
					if ((byCurrentCard & MASK_COLOR) == 0x10)
					{
						m_tHuCardInforTemp.byHuXiCount += 3;
					}
				}
				else
				{
					m_tHuCardInforTemp.byHuXiCount += GetWeaveHuXi(m_tHuCardInforTemp.tWeaveItemArray[byIndex]);
				}
			}
		}
		AnalyseCard(byCardIndexTemp,byCurrentCard,iJinNum);
	}

	bool ret = false;
	if( 0 != m_AnalyseItemTemp[0].iHuType ||
		0 != m_AnalyseItemTemp[1].iHuType ||
		0 != m_AnalyseItemTemp[2].iHuType ||
		0 != m_AnalyseItemTemp[3].iHuType || 
		0 != m_AnalyseItemTemp[4].iHuType
		)
	{
		memcpy(tHuCardInfo,m_AnalyseItemTemp,sizeof(m_AnalyseItemTemp));
		ret = true;
	}
	finish=clock();
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
	return ret;
}

//提取吃牌
BYTE CGameLogic::TakeOutChiCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard, BYTE byResultCard[3])
{

	if (byCurrentCard == 0) return 0;

	//变量定义
	BYTE byFirstIndex = 0;
	BYTE byCurrentIndex = SwitchToCardIndex(byCurrentCard);

	//大小搭吃
	BYTE byReverseIndex=(byCurrentIndex + 10) % MAX_INDEX;
	if ((byCardIndex[byCurrentIndex] >= 2) && (byCardIndex[byReverseIndex] >= 1) && (byCardIndex[byReverseIndex] != 3 && byCardIndex[byReverseIndex] != 4))
	{
		//删除扑克
		byCardIndex[byCurrentIndex]--;
		byCardIndex[byCurrentIndex]--;
		byCardIndex[byReverseIndex]--;

		//设置结果
		byResultCard[0] = byCurrentCard;
		byResultCard[1] = byCurrentCard;
		byResultCard[2] = SwitchToCardData(byReverseIndex);

		return ((byCurrentCard & MASK_COLOR) == 0x00) ? CK_XXD : CK_XDD;
	}

	//大小搭吃
	if (byCardIndex[byReverseIndex] == 2)
	{
		//删除扑克
		byCardIndex[byCurrentIndex]--;
		byCardIndex[byReverseIndex] -= 2;

		//设置结果
		byResultCard[0] = byCurrentCard;
		byResultCard[1] = SwitchToCardData(byReverseIndex);
		byResultCard[2] = SwitchToCardData(byReverseIndex);

		return ((byCurrentCard & MASK_COLOR) == 0x00) ? CK_XDD : CK_XXD;
	}

	//二七十吃
	BYTE bCardValue = (byCurrentCard & MASK_VALUE);
	if ((bCardValue == 0x02)||(bCardValue == 0x07)||(bCardValue == 0x0A))
	{
		//变量定义
		BYTE byExcursion[]={1,6,9};
		BYTE byInceptIndex = ((byCurrentCard & MASK_COLOR) == 0x00) ? 0 : 10;

		int i = 0;
		//类型判断
		for (i = 0;i < sizeof(byExcursion);i++)
		{
			BYTE cbIndex = byInceptIndex + byExcursion[i];
			if ((byCardIndex[cbIndex] == 0)||(byCardIndex[cbIndex] == 3) || (byCardIndex[cbIndex] == 4)) break;
		}

		//成功判断
		if (i == sizeof(byExcursion))
		{
			//删除扑克
			byCardIndex[byInceptIndex+byExcursion[0]]--;
			byCardIndex[byInceptIndex+byExcursion[1]]--;
			byCardIndex[byInceptIndex+byExcursion[2]]--;

			//设置结果
			byResultCard[0] = SwitchToCardData(byInceptIndex+byExcursion[0]);
			byResultCard[1] = SwitchToCardData(byInceptIndex+byExcursion[1]);
			byResultCard[2] = SwitchToCardData(byInceptIndex+byExcursion[2]);

			return CK_EQS;
		}
	}

	BYTE byTempCardIndex[MAX_INDEX];
	memcpy(byTempCardIndex,byCardIndex,sizeof(byTempCardIndex));
	//顺子判断
	BYTE byExcursion[3]={0,1,2};
	BYTE byThreeFindCount=0;
	for (BYTE i = 0;i < sizeof(byExcursion);i++)
	{
		BYTE byValueIndex = byCurrentIndex % 10;
		if ((byValueIndex >= byExcursion[i]) && ((byValueIndex - byExcursion[i]) <= 7))
		{
			//吃牌判断
			byFirstIndex = byCurrentIndex-byExcursion[i];
			BYTE byThreeNumCard[3],byIndex=0;//记录牌数量为3的牌
			memset(byThreeNumCard,255,sizeof(byThreeNumCard));
			bool bInvalidFind=false,bThreeNumFind=false;//分别标识数量为0的牌和为3的牌
			/*CString lwlog;
			lwlog.Format("lwlog::byFirstIndex=%d byCardIndex[byFirstIndex]=%d byCardIndex[byFirstIndex+1]=%d byCardIndex[byFirstIndex+2]",byFirstIndex,byCardIndex[byFirstIndex],byCardIndex[byFirstIndex+1],byCardIndex[byFirstIndex+2]);
			OutputDebugString(lwlog);*/
			if (byCardIndex[byFirstIndex] == 0) 
			{
				bInvalidFind=true;
				//continue;
			}
			if(byCardIndex[byFirstIndex] == 3)
			{
				byThreeNumCard[byIndex++]=byFirstIndex;
				bThreeNumFind=true;
				//continue;
			}
			if(byCardIndex[byFirstIndex] == 4 && byCurrentIndex!=byFirstIndex)
			{
				continue;
			}
			if (byCardIndex[byFirstIndex + 1] == 0)
			{
				bInvalidFind=true;
				//continue;
			}
			if(byCardIndex[byFirstIndex + 1] == 3)
			{
				byThreeNumCard[byIndex++]=byFirstIndex+1;
				bThreeNumFind=true;
				//continue;
			}
			if(byCardIndex[byFirstIndex + 1] == 4 && byCurrentIndex!=(byFirstIndex+1))
			{
				continue;
			}
			if (byCardIndex[byFirstIndex + 2] == 0) 
			{
				bInvalidFind=true;
				//continue;
			}
			if(byCardIndex[byFirstIndex + 2] == 3)
			{
				byThreeNumCard[byIndex++]=byFirstIndex+2;
				bThreeNumFind=true;
				//continue;
			}
			if(byCardIndex[byFirstIndex + 2] == 4 && byCurrentIndex!=(byFirstIndex+2))
			{
				continue;
			}
			if(!bInvalidFind && bThreeNumFind)
			{
				for(int j=0;j<3;j++)
				{
					if(255==byThreeNumCard[j])
					{
						continue;
					}
					if(byCurrentIndex==byThreeNumCard[j])
					{
						byThreeFindCount++;
						byTempCardIndex[byFirstIndex]--;
						byTempCardIndex[byFirstIndex + 1]--;
						byTempCardIndex[byFirstIndex + 2]--;
					}
				}
			}
			if(sizeof(byExcursion)-1==i)
			{
				if(3==byThreeFindCount && 0==byTempCardIndex[byCurrentIndex])
				{
					byCardIndex[byCurrentIndex]=0;
					printf("lwlog::达到预期");
					return CK_NULL;
				}
				else
				{
					if(bInvalidFind || bThreeNumFind)
					{
						continue;
					}
				}
			}
			else
			{
				if(bInvalidFind || bThreeNumFind)
				{
					continue;
				}
			}
			//删除扑克
			byCardIndex[byFirstIndex]--;
			byCardIndex[byFirstIndex + 1]--;
			byCardIndex[byFirstIndex + 2]--;

			//设置结果
			byResultCard[0] = SwitchToCardData(byFirstIndex);
			byResultCard[1] = SwitchToCardData(byFirstIndex+1);
			byResultCard[2] = SwitchToCardData(byFirstIndex+2);

			BYTE byChiKind[3] = {CK_LEFT,CK_CENTER,CK_RIGHT};
			return byChiKind[i];
		}
	}
	return CK_NULL;
}

//牌转换,用于胡牌检测
void CGameLogic::ChangeHandPaiData(BYTE byHandpai[],BYTE byPai)
{
	m_tCheckHuChangleData.Clear();
	for(int i = 0;i < MAX_COUNT;i++)
	{
		if(IsValidCard(byHandpai[i]))
		{
			m_tCheckHuChangleData.AddCard(byHandpai[i]);
		}
	}
	//if(byPai!= 0 && 0x2f != byPai)						//如果抓的这张牌是金牌,那就不用加进来了
	//{
	//	if(IsValidCard(byPai))
	//	{
	//		m_tCheckHuChangleData.AddCard(byPai);
	//	}
	//}
}

//取得当前胡牌时总红字
int  CGameLogic::GetHandRedWordNum()
{
	int iRedCardNum = 0;
	for(int i = 0;i < m_AnalyseItemHu.byWeaveCount;i ++)
	{
		for(int j = 0;j < 4;j ++)
		{
			if(IsEQS(m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j]))
			{
				iRedCardNum ++;
			}
		}
	}

	for(int i = 0;i < m_tHuCardInforTemp.byWeaveCount;i ++)
	{
		for(int j = 0;j < 4;j ++)
		{
			if(IsEQS(m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j]))
			{
				iRedCardNum ++;
			}
		}
	}

	/*
	//一点红的时候王就不要变红牌了
	if(KINGVALUE==m_byXingPai && iRedCardNum > 1)
	{
		for(int i=0;i<m_AnalyseItemHu.byWeaveCount;i++)
		{
			if(ACK_JIANGPAI==m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind && KINGVALUE==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[0]
			&& KINGVALUE==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[1])
			{
				if((m_iCPGWPTRedCardNum+iRedCardNum)>0)
				{
					iRedCardNum+=2;
				}
			}
			if (ACK_WEI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind && KINGVALUE == m_AnalyseItemHu.tWeaveItemArray[i].byCardList[0]
				&& KINGVALUE == m_AnalyseItemHu.tWeaveItemArray[i].byCardList[1] && KINGVALUE == m_AnalyseItemHu.tWeaveItemArray[i].byCardList[2])
			{
				if ((m_iCPGWPTRedCardNum + iRedCardNum) > 0)
				{
					iRedCardNum += 3;
				}
			}
		}
	}*/
	return iRedCardNum;
}
bool CGameLogic::GetMaxHuXi()
{
	BYTE byHuXi = 0;			//总胡息
	int iDunNum = m_tGameRules.minHuXiTun;			//囤数
	int iHuType = 0;			//胡牌类型
	int iFan = 0;
	for(int i = 0;i < m_AnalyseItemHu.byWeaveCount;i ++)
	{
		byHuXi += GetWeaveHuXi(m_AnalyseItemHu.tWeaveItemArray[i]);
	}
	byHuXi += (m_iCPTWPHuXi + m_tHuCardInforTemp.byHuXiCount);			//总胡息
	if (byHuXi == 0 && m_tGameRules.maoHu)
	{
		byHuXi = 15; //勾选毛胡，满足7方门子，且胡息=0，按15胡息计算
	}

	if (byHuXi < m_tGameRules.byMinHuXi)
	{
		return false;
	}

	//囤息转换
	if (m_tGameRules.tunXiRatio > 0)
	{
		iDunNum += ((byHuXi - m_tGameRules.byMinHuXi) / m_tGameRules.tunXiRatio);
	}
	int fanArr[5] = { 0 };

	int iRedWord = GetHandRedWordNum() + m_iCPGWPTRedCardNum;
	if (m_tGameRules.playType == 0)
	{
		iFan = 1;
		iHuType = COMMON_HU;
	}
	else if (m_tGameRules.playType == 1) //红黑点
	{
		fanArr[0] = 2;				//小红胡番(15张玩法,7个红就可以了)
		fanArr[1] = 3;				//大红胡番(15张玩法,没这个)
		fanArr[2] = 3;				//一点红番
		fanArr[3] = 4;				//黑胡番
		fanArr[4] = 4;				//红乌番(15张玩法,10个红就可以了)

		if(IsShiShiRedHu_15(iRedWord))									//小红胡
		{
			iFan = fanArr[0];
			iHuType = SHI_RED_HU_15;
		}
		else if(!m_tGameRules.b15Type && IsDianHu_15(iRedWord))									//大红胡
		{
			iFan = fanArr[1];
			iHuType = DIAN_RED_HU_15;
		}
		else if (IsShiWuRedHu_15(iRedWord))								//红乌
		{
			iFan = fanArr[4];
			iHuType = SHI_WU_RED_HU_15;
		}
		else if(IsOneRedHu_15(iRedWord))								//一点红
		{
			iFan = fanArr[2];
			iHuType = ONE_RED_HU_15;
		}
		else if(IsAllBlackHu_15(iRedWord))								//全黑
		{
			iFan = fanArr[3];
			iHuType = ALL_BLACK_HU_15;
		}
		else															//平胡
		{
			iFan = 1;
			iHuType = COMMON_HU;
		}
	}

	if (m_tGameRules.bXianHuXianFan)
	{
		int tmpFan = iFan;
		if (m_bZiMo)
		{
			tmpFan *= 2;
		}

		int tmp = 1;
		if (IsWanDiao())
		{
			tmp = 4;
		}
		if (IsWangChuang())
		{
			tmp = 8;
		}
		int tmp1 = 0;
		if ((tmp1 = IsSanWangChuang()) > 0)
		{
			tmp = tmp1;
		}
		tmpFan *= tmp;

		int iKingCount = 0;
		if (m_byStation >= 0 && m_byStation < PLAY_COUNT)
		{
			iKingCount = m_realJingNum[m_byStation];
		}
		//1王必须2番,2王必须4番,3王必须8番,4王必须16番
		if ( (iKingCount == 1 && tmpFan < 2) || (iKingCount == 2 && tmpFan < 4) || (iKingCount == 3 && tmpFan < 8) || (iKingCount == 4 && tmpFan < 16) )
		{
			return false;
		}
	}

	int iXing = GetXingCount() + m_iCPTWPXing;			//检测胡牌时的醒牌个数 = 手牌 + 吃碰提偎跑醒的个数
	if (m_tGameRules.bShuangXing) //双醒
	{
		iXing *= 2;
	}

	bool Wang = false;
	int iTypeNoWang=iHuType;
	if(IsWanDiao())						//是否是王钓
	{
		if(iTypeNoWang == COMMON_HU){ iTypeNoWang = 0; }
		iHuType=iTypeNoWang|WANG_DIAO;
		Wang = true;
		int iTempMaxDun = (iDunNum + iXing) * iFan;
		if (iTempMaxDun > m_iMaxDun[1])
		{
			m_iMaxDun[1] = iTempMaxDun;
			SetHuMaxCardData(1,iXing,byHuXi,iDunNum, iFan * 4,iHuType);
		}
	}
	if(IsWangChuang())				//是否是王闯
	{
		if(iTypeNoWang == COMMON_HU){ iTypeNoWang = 0; }
		iHuType=iTypeNoWang|WANG_CHUANG;
		Wang = true;
		int iTempMaxDun = (iDunNum + iXing) * iFan;
		if (iTempMaxDun > m_iMaxDun[2])
		{
			m_iMaxDun[2] = iTempMaxDun;
			SetHuMaxCardData(2,iXing,byHuXi,iDunNum, iFan * 8,iHuType);
		}
	}
	int tmp2 = 0;
	if((tmp2 = IsSanWangChuang()) > 0)//是否是三王闯
	{
		if(iTypeNoWang == COMMON_HU){ iTypeNoWang = 0; }
		iHuType=iTypeNoWang|SWANG_CHUANG;
		Wang = true;
		int iTempMaxDun = (iDunNum + iXing) * iFan;
		if (iTempMaxDun > m_iMaxDun[3])
		{
			m_iMaxDun[3] = iTempMaxDun;
			SetHuMaxCardData(3,iXing,byHuXi,iDunNum, iFan * tmp2,iHuType);
		}
	}
	//if(IsWangZha())						//当前是否是王炸
	//{
	//	if(iTypeNoWang == COMMON_HU){ iTypeNoWang = 0; }
	//	iHuType=iTypeNoWang|WANG_ZHA;
	//	Wang = true;
	//	SetHuMaxCardData(4,iXing,byHuXi,iDunNum, iFan * 32,iHuType);
	//}
	if(!Wang)
	{
		int iTempMaxDun=(iDunNum + iXing) * iFan;
		if(iTempMaxDun>m_iMaxDun[0])
		{
			m_iMaxDun[0]=iTempMaxDun;
			SetHuMaxCardData(0,iXing,byHuXi,iDunNum,iFan,iHuType);
		}
	}

	m_bHuFlag = true;

	return true;
}

//设置最大胡牌最大分	
void CGameLogic::SetHuMaxCardData(int iIndex,int iXing,BYTE byHuXiCount,int iTunSum,int iFan,int iHuType)
{
	m_AnalyseItemTemp[iIndex].byWeaveCount = 0;
	m_AnalyseItemTemp[iIndex].Init();
	int iIndexTemp = 0;
	for(int i = 0;i < m_tHuCardInforTemp.byWeaveCount;i ++)
	{
		iIndexTemp = m_AnalyseItemTemp[iIndex].byWeaveCount ++;
		m_AnalyseItemTemp[iIndex].tWeaveItemArray[iIndexTemp] = m_tHuCardInforTemp.tWeaveItemArray[i];
	}
	for(int i = 0;i < m_AnalyseItemHu.byWeaveCount;i ++)
	{
		iIndexTemp = m_AnalyseItemTemp[iIndex].byWeaveCount ++;
		m_AnalyseItemTemp[iIndex].tWeaveItemArray[iIndexTemp] = m_AnalyseItemHu.tWeaveItemArray[i];
	}
	int iAllTun = iTunSum * iFan;								//总囤数 = 囤数 * 番数
	m_AnalyseItemTemp[iIndex].byHuXiCount = byHuXiCount;		//总胡息
	m_AnalyseItemTemp[iIndex].iXingNum = iXing;					//醒牌数数
	m_AnalyseItemTemp[iIndex].iSumDun = iTunSum;				//囤数
	m_AnalyseItemTemp[iIndex].iSumFan = iFan;					//总番数
	m_AnalyseItemTemp[iIndex].iHuType = iHuType;				//牌型
	m_AnalyseItemTemp[iIndex].iAllDun = iAllTun;				//胡牌总囤数
	m_AnalyseItemTemp[iIndex].byXing = m_byXingPai;				//醒牌
	m_AnalyseItemTemp[iIndex].bZiMo = m_bZiMo;					//自摸
	m_AnalyseItemTemp[iIndex].byStation = m_byStation;			
	m_AnalyseItemTemp[iIndex].byBeStation = m_byBeStation;	
	m_AnalyseItemTemp[iIndex].bWeaveReplace=m_bWeaveReplace;
}

//计算醒牌个数
int  CGameLogic::GetXingCount()
{
	if(255 == m_byXingPai)	return 0;
	int iXingNumber = 0;
	if(KINGVALUE==m_byXingPai)
	{
		BYTE byJingReplace[4];
		memset(byJingReplace,255,sizeof(byJingReplace));
		int iTempIndex=0;
		for(int i=0;i<m_tHuCardInforTemp.byWeaveCount;i++)
		{
			if(ACK_NULL==m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
			{
				continue;
			}
			for(int j=0;j<4;j++)
			{
				if(!m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				byJingReplace[iTempIndex++]=m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j];
			}
		}
		for(int i=0;i<m_AnalyseItemHu.byWeaveCount;i++)
		{
			if(ACK_NULL==m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
			{
				continue;
			}
			for(int j=0;j<4;j++)
			{
				if(!m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				byJingReplace[iTempIndex++]=m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j];
			}
		}
		int iWangNums=0;
		for(int i=0;i<iTempIndex;i++)
		{
			if(255==byJingReplace[i])
			{
				continue;
			}
			if(KINGVALUE==byJingReplace[i])
			{
				iWangNums++;
			}
		}
		int iMaxXingNum=0;
		for(int i=0;i<4;i++)
		{
			if(255==byJingReplace[i])
			{
				continue;
			}
			int iTemNum=m_pGameData->GetCPTWPXing(m_byStation,byJingReplace[i]);
			if(m_bWeaveReplace)
			{
				int ActionNum = 0;
				for (BYTE action= 0;action<MAX_WEAVE;action++)
				{
					for(int card= 0;card< 4;card++)
					{
						if(byJingReplace[i] == m_tCPTWPArray[action].byCardList[card])
						{
							ActionNum ++;
						}
					}
				}
				iTemNum=ActionNum;
			}
			for(int j=0;j<MAX_WEAVE;j++)
			{
				if(ACK_NULL==m_tHuCardInforTemp.tWeaveItemArray[j].iWeaveKind)
				{
					continue;
				}
				for(int t=0;t<4;t++)
				{
					if(byJingReplace[i]==m_tHuCardInforTemp.tWeaveItemArray[j].byCardList[t])
					{
						iTemNum++;
					}
				}
			}
			for(int j=0;j<MAX_WEAVE;j++)
			{
				if(ACK_NULL==m_AnalyseItemHu.tWeaveItemArray[j].iWeaveKind)
				{
					continue;
				}
				for(int t=0;t<4;t++)
				{
					if(byJingReplace[i]==m_AnalyseItemHu.tWeaveItemArray[j].byCardList[t])
					{
						iTemNum++;
					}
				}
			}
			if(iTemNum>iMaxXingNum)
			{
				iMaxXingNum=iTemNum;
			}
		}
		iXingNumber=iMaxXingNum;
		if(iWangNums>0)
		{
			BYTE byCardIndex[MAX_INDEX];
			memset(byCardIndex,0,sizeof(byCardIndex));
			tagWeaveItem gcpData[MAX_WEAVE];
			m_pGameData->CopyCPTWPData(m_byStation,gcpData);
			for(int i=0; i<MAX_WEAVE; i++)
			{
				if(ACK_NULL == gcpData[i].iWeaveKind)
				{
					continue;
				}
				for(int j=0;j<4;j++)
				{
					if(0==gcpData[i].byCardList[j] || 255==gcpData[i].byCardList[j])
					{
						continue;
					}
					byCardIndex[SwitchToCardIndex(gcpData[i].byCardList[j])]++;
				}
			}
			for(int i=0;i<m_tHuCardInforTemp.byWeaveCount;i++)
			{
				if(ACK_NULL==m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
				{
					continue;
				}
				for(int j=0;j<4;j++)
				{
					if(0==m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j] || 255==m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j] || KINGVALUE==m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j])
					{
						continue;
					}
					byCardIndex[SwitchToCardIndex(m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j])]++;
				}
			}
			for(int i=0;i<m_AnalyseItemHu.byWeaveCount;i++)
			{
				if(ACK_NULL==m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
				{
					continue;
				}
				for(int j=0;j<4;j++)
				{
					if(0==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j] || 255==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j] || KINGVALUE==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j])
					{
						continue;
					}
					byCardIndex[SwitchToCardIndex(m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j])]++;
				}
			}
			for(int i=0;i<MAX_INDEX;i++)
			{
				if(byCardIndex[i]<=0)
				{
					continue;
				}
				int iTempNum=iWangNums+byCardIndex[i];
				if(iTempNum>iMaxXingNum)
				{
					iMaxXingNum=iTempNum;
				}
			}
			if(iMaxXingNum>iXingNumber)
			{
				iXingNumber=iMaxXingNum;
			}
		}
	}
	else
	{
		for(int i = 0;i < m_tHuCardInforTemp.byWeaveCount;i ++)
		{
			for(int j = 0;j < 4;j ++)
			{
				if(m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j] == m_byXingPai)
				{
					iXingNumber ++;
				}
				if(m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j] == KINGVALUE)
				{
					iXingNumber ++;
				}
			}	
		}
		for(int i = 0;i < m_AnalyseItemHu.byWeaveCount;i ++)
		{
			for(int j = 0;j < 4;j ++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j] == m_byXingPai)
				{
					iXingNumber ++;
				}
				if(m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j] == KINGVALUE)
				{
					iXingNumber ++;
				}
			}
		}
	}
	return iXingNumber;
}

//组胡牌数据(组成三个或一句话或绞或2710)
bool CGameLogic::MakePingHu(CheckHuStruct &PaiData,tagAnalyseItem &hupaistruct, int &recursiveDepth, int iJingNum)
{
	recursiveDepth++;
	//if (recursiveDepth > 5500)
	//{
	//	ERROR_LOG("recursiveDepth:%d", recursiveDepth);
	//	return false;
	//}

	if(PaiData.GetAllCardCount() + iJingNum <= 0)
	{
		return GetMaxHuXi();
	}

	BYTE byData[4];
	bool bCaishen[4];
	memset(byData,0,sizeof(byData));
	memset(bCaishen,false,sizeof(bCaishen));
	//三财神做一个该子
	if(iJingNum >= 3)
	{
		for (int i = 0; i < MAX_INDEX; ++i)
		{
			BYTE cbCardData = SwitchToCardData(i);
			memset(bCaishen,true,sizeof(bCaishen) - 1);
			memset(byData,cbCardData,sizeof(byData) - 1);
			hupaistruct.AddData(ACK_WEI,byData[0],3,byData,bCaishen);//添加组牌
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 3);//递归
			hupaistruct.DeleteData(ACK_WEI,byData,bCaishen);
			/*
			memset(bCaishen,true,sizeof(bCaishen) - 1);
			memset(byData,0x2f,sizeof(byData) - 1);
			hupaistruct.AddData(ACK_WEI,byData[0],3,byData,bCaishen);//添加组牌
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 3);//递归
			hupaistruct.DeleteData(ACK_WEI,byData,bCaishen);
*/
		}
	}

	BYTE byPai=(BYTE)255;
	int iCount = 0;
	for(int i = 1;i < MAX_CARDVALUE;i ++)
	{
		if(0 != PaiData.iData[i])
		{
			byPai = i;
			iCount = PaiData.iData[i];
			break;
		}
	}

	byData[0] = byPai;
	byData[1] = byPai;
	byData[2] = byPai;
	//刻子检测
	if(iJingNum >= 2)
	{
		//OutputDebugString("lwlog::刻字检测11");
		memset(bCaishen,false,sizeof(bCaishen));
		bCaishen[0] = true;
		bCaishen[1] = true;
		hupaistruct.AddData(ACK_WEI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 2);//递归
		PaiData.AddCard(byPai,1);
		hupaistruct.DeleteData(ACK_WEI,byData,bCaishen);
	}
	if(iJingNum >= 1 && iCount >= 2)
	{
		//OutputDebugString("lwlog::刻字检测22");
		memset(bCaishen,false,sizeof(bCaishen));
		bCaishen[0] = true;
		hupaistruct.AddData(ACK_WEI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,2);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byPai,2);
		hupaistruct.DeleteData(ACK_WEI,byData,bCaishen);
	}
	//if(0==iJingNum && iCount>2)//++
	//{
	//	CString lwlog;
	//	lwlog.Format("lwlog::刻字检测33 byPai=%x",byPai);
	//	OutputDebugString(lwlog);
	//	OutputDebugString("lwlog::刻字检测33");
	//	memset(bCaishen,false,sizeof(bCaishen));
	//	hupaistruct.AddData(ACK_WEI,byData[0],3,byData,bCaishen);//添加组牌
	//	PaiData.DecCard(byPai,3);
	//	MakePingHu(PaiData,hupaistruct,iJingNum);//递归
	//	PaiData.AddCard(byPai,3);
	//	hupaistruct.DeleteData(ACK_WEI,byData,bCaishen);
	//}
	//绞子检测
	//---------------------------------------------------------------------------------------
	//大小搭吃(1张财神 + 大 + 小)
	if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) == 1 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) == 1)
	{
		//OutputDebugString("lwlog::绞子检测00");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[0] = true;
		byData[0] = byPai;
		byData[1] = byPai;
		byData[2] = (byPai + 0x10) % 0x20;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byData[1],1);
		PaiData.DecCard(byData[2],1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byData[1],1);
		PaiData.AddCard(byData[2],1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	//大小搭吃
	if(PaiData.GetPaiCount(byPai) == 2 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) >= 1)
	{
		//OutputDebugString("lwlog::绞子检测11");
		memset(bCaishen,0,sizeof(bCaishen));
		byData[0] = byPai;
		byData[1] = byPai;
		byData[2] = (byPai + 0x10) % 0x20;
		
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,2);
		PaiData.DecCard(byData[2],1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
		PaiData.AddCard(byPai,2);
		PaiData.AddCard(byData[2],1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	//大小搭吃
	if(PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) == 2)
	{
		//OutputDebugString("lwlog::绞子检测22");
		memset(bCaishen,0,sizeof(bCaishen));
		byData[0] = byPai;
		byData[1] = (byPai + 0x10) % 0x20;
		byData[2] = byData[1];
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		PaiData.DecCard(byData[2],2);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
		PaiData.AddCard(byPai,1);
		PaiData.AddCard(byData[2],2);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	//大小搭吃
	if(PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) == 3 && m_byPai == (byPai + 0x10) % 0x20 && !m_bZiMo)
	{
		//OutputDebugString("lwlog::绞子检测22");
		memset(bCaishen,0,sizeof(bCaishen));
		byData[0] = byPai;
		byData[1] = (byPai + 0x10) % 0x20;
		byData[2] = byData[1];
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		PaiData.DecCard(byData[2],2);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
		PaiData.AddCard(byPai,1);
		PaiData.AddCard(byData[2],2);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}

	if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) >= 1)//++
	{
		//OutputDebugString("lwlog::绞子检测00_00");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[0] = true;
		byData[0] = byPai;
		byData[1] = byPai;
		byData[2] = (byPai + 0x10) % 0x20;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byData[1],1);
		PaiData.DecCard(byData[2],1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byData[1],1);
		PaiData.AddCard(byData[2],1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	//大小搭吃(1张财神 + 大 + 小)
	if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) == 1 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) == 1)//++
	{
		//OutputDebugString("lwlog::绞子检测00_11");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[0] = true;
		byData[0] = (byPai + 0x10) % 0x20;
		byData[1] = byPai;
		byData[2] = (byPai + 0x10) % 0x20;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byData[1],1);
		PaiData.DecCard(byData[2],1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byData[1],1);
		PaiData.AddCard(byData[2],1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	//大小搭吃
	//if(PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount((byPai + 0x10) % 0x20) >= 2)//++
	//{
	//	//OutputDebugString("lwlog::绞子检测33");
	//	memset(bCaishen,0,sizeof(bCaishen));
	//	byData[0] = byPai;
	//	byData[1] = (byPai + 0x10) % 0x20;
	//	byData[2] = byData[1];
	//	hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
	//	PaiData.DecCard(byPai,1);
	//	PaiData.DecCard(byData[2],2);
	//	MakePingHu(PaiData,hupaistruct,iJingNum);//递归
	//	PaiData.AddCard(byPai,1);
	//	PaiData.AddCard(byData[2],2);
	//	hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	//}
	//顺子检测
	BYTE byCardValue = (byPai &0x0f);
	//二七十检测
	//---------------------------------------------------------------------------------------
	if ((byCardValue)==0x02)
	{
		//OutputDebugString("lwlog::二七十检测00");
		if(iJingNum >= 1 && PaiData.GetPaiCount(byPai + 8) >= 1)	//pai + x + pai + 8
		{
			//OutputDebugString("lwlog::二七十检测00_00");
			memset(bCaishen,0,sizeof(bCaishen));
			bCaishen[1] = true; 
			byData[0] = byPai;
			byData[1] = byPai + 5;
			byData[2] = byPai + 8;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byPai,1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
			PaiData.AddCard(byPai,1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
		if(iJingNum >= 1 && PaiData.GetPaiCount(byPai + 5) >= 1) //pai + (pai + 5) + x
		{
			//OutputDebugString("lwlog::二七十检测00_11");
			memset(bCaishen,0,sizeof(bCaishen));
			bCaishen[2] = true;
			byData[0] = byPai;
			byData[1] = byPai + 5;
			byData[2] = byPai + 8;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byPai,1);
			PaiData.DecCard(byData[1],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
			PaiData.AddCard(byPai,1);
			PaiData.AddCard(byData[1],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
		if(PaiData.GetPaiCount(byPai + 5) >= 1 && PaiData.GetPaiCount(byPai + 8) >= 1)//pai + (pai + 5) + (pai + 8)
		{
			//OutputDebugString("lwlog::二七十检测00_22");
			memset(bCaishen,0,sizeof(bCaishen));
			byData[0] = byPai;
			byData[1] = byPai + 5;
			byData[2] = byPai + 8;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byPai,1);
			PaiData.DecCard(byData[1],1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
			PaiData.AddCard(byPai,1);
			PaiData.AddCard(byData[1],1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
	}
	if((byCardValue) == 0x07)
	{
		//OutputDebugString("lwlog::二七十检测11");
		if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount(byPai + 3) >= 1)//x + (pai) + (pai + 3)
		{
			//OutputDebugString("lwlog::二七十检测11_00");
			memset(bCaishen,0,sizeof(bCaishen));
			bCaishen[0] = true;
			byData[0] = byPai - 5;
			byData[1] = byPai;
			byData[2] = byPai + 3;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byData[1],1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum-1);//递归
			PaiData.AddCard(byData[1],1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
		if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount(byPai-5) >= 1)//(pai-5) + (pai) + x
		{
			//OutputDebugString("lwlog::二七十检测11_11");
			memset(bCaishen,0,sizeof(bCaishen));
			bCaishen[2] = true;
			byData[0] = byPai - 5;
			byData[1] = byPai;
			byData[2] = byPai + 3;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byData[0],1);
			PaiData.DecCard(byData[1],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum-1);//递归
			PaiData.AddCard(byData[0],1);
			PaiData.AddCard(byData[1],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
		if(PaiData.GetPaiCount(byPai-5) >= 1 && PaiData.GetPaiCount(byPai + 3) >= 1)//(pai-5) + pai + (pai + 3)
		{
			//OutputDebugString("lwlog::二七十检测11_22");
			memset(bCaishen,0,sizeof(bCaishen));
			byData[0] = byPai-5;
			byData[1] = byPai;
			byData[2] = byPai + 3;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byData[0],1);
			PaiData.DecCard(byData[1],1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
			PaiData.AddCard(byData[0],1);
			PaiData.AddCard(byData[1],1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
	}
	if((byCardValue) == 0x0A)
	{
		//OutputDebugString("lwlog::二七十检测22");
		if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount(byPai - 3) >= 1)//x + (pai-3) + pai
		{
			//OutputDebugString("lwlog::二七十检测22_00");
			memset(bCaishen,0,sizeof(bCaishen));
			bCaishen[0] = true;
			byData[0] = byPai - 8;
			byData[1] = byPai-3;
			byData[2] = byPai;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byData[1],1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum-1);//递归
			PaiData.AddCard(byData[1],1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
		if(iJingNum >= 1 && PaiData.GetPaiCount(byPai) >= 1 && PaiData.GetPaiCount(byPai-8) >= 1)//(pai-8) + x + pai
		{
			//OutputDebugString("lwlog::二七十检测22_11");
			memset(bCaishen,0,sizeof(bCaishen));
			bCaishen[1] = true;
			byData[0] = byPai - 8;
			byData[1] = byPai-3;
			byData[2] = byPai;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byData[0],1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum-1);//递归
			PaiData.AddCard(byData[0],1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
		if(PaiData.GetPaiCount(byPai-8) >= 1 && PaiData.GetPaiCount(byPai - 3) >= 1)//(pai-8) + (pai-3) + pai
		{
			//OutputDebugString("lwlog::二七十检测22_22");
			memset(bCaishen,0,sizeof(bCaishen));
			byData[0] = byPai-8;
			byData[1] = byPai-3;
			byData[2] = byPai;
			hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
			PaiData.DecCard(byData[0],1);
			PaiData.DecCard(byData[1],1);
			PaiData.DecCard(byData[2],1);
			MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
			PaiData.AddCard(byData[0],1);
			PaiData.AddCard(byData[1],1);
			PaiData.AddCard(byData[2],1);
			hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
		}
	}
	//---------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------
	if(iJingNum >= 2 && byCardValue >= 3)//组成 x + x + pai 的顺子（x是财神）
	{
		//OutputDebugString("lwlog::顺子检测00");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[0] = true;
		bCaishen[1] = true;
		byData[0] = byPai - 2;
		byData[1] = byPai - 1;
		byData[2] = byPai;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 2);//递归
		PaiData.AddCard(byPai,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	if(iJingNum >= 1 && byCardValue >=2 && byCardValue < 0x0A && PaiData.GetPaiCount(byPai + 1) >= 1)//组成 x + pai + (pai + 1) 的顺子（x是财神）
	{
		//OutputDebugString("lwlog::顺子检测11");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[0] = true;
		byData[0] = byPai - 1;
		byData[1] = byPai;
		byData[2] = byPai + 1;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		PaiData.DecCard(byPai + 1,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byPai,1);
		PaiData.AddCard(byPai + 1,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	if(iJingNum >= 2 && byCardValue >=2 && byCardValue < 0x0A)//组成 x+pai+(x) 的顺子（x是财神）
	{
		//OutputDebugString("lwlog::顺子检测22");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[0] = true;
		bCaishen[2] = true;
		byData[0] = byPai - 1;
		byData[1] = byPai;
		byData[2] = byPai + 1;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 2);//递归
		PaiData.AddCard(byPai,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	if( byCardValue < 9 && PaiData.GetPaiCount(byPai + 1)>=1 && PaiData.GetPaiCount(byPai + 2) >= 1)//组成 pai + (pai + 1) + (pai + 2) 的顺子
	{
		//OutputDebugString("lwlog::顺子检测33");
		memset(bCaishen,0,sizeof(bCaishen));
		byData[0] = byPai;
		byData[1] = byPai + 1;
		byData[2] = byPai + 2;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		PaiData.DecCard(byPai + 1,1);
		PaiData.DecCard(byPai + 2,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum);//递归
		PaiData.AddCard(byPai,1);
		PaiData.AddCard(byPai + 1,1);
		PaiData.AddCard(byPai + 2,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	if(iJingNum >= 1 && byCardValue < 9 && PaiData.GetPaiCount(byPai + 1) < 1 && PaiData.GetPaiCount(byPai + 2) >= 1)//组成 pai+(pai+1)+(x) 的顺子
	{
		//OutputDebugString("lwlog::顺子检测44");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[1] = true;
		byData[0] = byPai;
		byData[1] = byPai + 1;
		byData[2] = byPai + 2;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		PaiData.DecCard(byPai + 2,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byPai,1);
		PaiData.AddCard(byPai + 2,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	if(iJingNum >= 1 && byCardValue < 9 && PaiData.GetPaiCount(byPai + 1)>=1 && PaiData.GetPaiCount(byPai + 2) < 1)//组成 pai+(pai+1)+(x) 的顺子
	{
		//OutputDebugString("lwlog::顺子检测55");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[2] = true;
		byData[0] = byPai;
		byData[1] = byPai + 1;
		byData[2] = byPai + 2;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		PaiData.DecCard(byPai + 1,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byPai,1);
		PaiData.AddCard(byPai + 1,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	if(iJingNum >= 2 && byCardValue < 9 )//组成 pai+(x)+(x) 的顺子
	{
		//OutputDebugString("lwlog::顺子检测66");
		memset(bCaishen,0,sizeof(bCaishen));
		bCaishen[1] = true;
		bCaishen[2] = true;
		byData[0] = byPai;
		byData[1] = byPai + 1;
		byData[2] = byPai + 2;
		hupaistruct.AddData(ACK_CHI,byData[0],3,byData,bCaishen);//添加组牌
		PaiData.DecCard(byPai,1);
		MakePingHu(PaiData,hupaistruct, recursiveDepth, iJingNum - 1);//递归
		PaiData.AddCard(byPai,1);
		hupaistruct.DeleteData(ACK_CHI,byData,bCaishen);
	}
	return false;
}

//胡牌分析
bool CGameLogic::AnalyseCard(BYTE byCardIndex[MAX_INDEX],BYTE byPai,int iJingNum)
{
	//数目统计 
	BYTE byCardCount = 0;
	m_byPai = byPai;
	m_bHuFlag = false;
	for (BYTE i = 0;i < MAX_INDEX;i++) byCardCount += byCardIndex[i];
	byCardCount += iJingNum;
	//数目判断
	if ((byCardCount % 3 != 0)&&((byCardCount + 1) % 3 != 0)) return false;  //牌的数量是否符合

	bool bNeedCardEye=((byCardCount + 1) % 3 == 0);							 //是否需要牌眼判断

	BYTE byPaiTemp[MAX_COUNT];
	memset(byPaiTemp,0,sizeof(byPaiTemp));
	SwitchToCardData(byCardIndex, byPaiTemp, sizeof(byPaiTemp),0);
	ChangeHandPaiData(byPaiTemp,byPai);
	m_iJingNum = iJingNum;
	m_AnalyseItemHu.Init();					//保存胡牌数据
	BYTE pai[4] = { 0 };
	bool caishen[4] = { 0 };
	//CString lwlog;
	//lwlog.Format("lwlog::byCardCount=%d",byCardCount);
	//OutputDebugString(lwlog);
	//--------------------------------------------------------
	int recursiveDepth = 0;
	if(!bNeedCardEye)
	{
		//OutputDebugString("lwlog::直接平胡");
		MakePingHu(m_tCheckHuChangleData,m_AnalyseItemHu, recursiveDepth, m_iJingNum);
	}
	else
	{
		//双金牌做将
		if(m_iJingNum >= 2)
		{
			//OutputDebugString("lwlog::双金牌做将");
			for (int i = 0; i < MAX_INDEX; ++i)
			{
				BYTE cbCardData = SwitchToCardData(i);
				m_AnalyseItemHu.Init();
				memset(caishen,0,sizeof(caishen));
				memset(pai,0,sizeof(pai));
				pai[0] = cbCardData;
				pai[1] = cbCardData;
				caishen[0] = true;
				caishen[1] = true;
				m_AnalyseItemHu.AddData(ACK_JIANGPAI,pai[0],2,pai,caishen);//添加组牌
				MakePingHu(m_tCheckHuChangleData,m_AnalyseItemHu, recursiveDepth, m_iJingNum - 2);
			}
		}

		//单金牌做将
		if(m_iJingNum >= 1)
		{
			for(int i = 1;i <= 0x1A;i ++)
			{
				if(m_tCheckHuChangleData.iData[i] >= 1)
				{
					m_AnalyseItemHu.Init();
					m_tCheckHuChangleData.DecCard(i,1);
					memset(caishen,0,sizeof(caishen));
					memset(pai,0,sizeof(pai));
					pai[0] = i;
					pai[1] = i;
					//lwlog.Format("lwlog::AnalyseCard::单金牌做将::pai[0]=%x",i);
					//OutputDebugString(lwlog);
					caishen[0] = true;
					m_AnalyseItemHu.AddData(ACK_JIANGPAI,pai[0],2,pai,caishen);			//添加组牌
					recursiveDepth = 0;
					MakePingHu(m_tCheckHuChangleData,m_AnalyseItemHu, recursiveDepth, m_iJingNum - 1);
					m_tCheckHuChangleData.AddCard(i,1);
				}
			}
		}

		for(int i = 0x01;i <= 0x1A;i ++)
		{
			if(m_tCheckHuChangleData.iData[i] >= 2)
			{
				m_AnalyseItemHu.Init();
				m_tCheckHuChangleData.DecCard(i,2);
				memset(caishen,0,sizeof(caishen));
				memset(pai,0,sizeof(pai));
				pai[0] = i;
				pai[1] = i;
				/*lwlog.Format("lwlog::AnalyseCard::普通牌做将::pai[0]=%x",i);
				OutputDebugString(lwlog);*/
				m_AnalyseItemHu.AddData(ACK_JIANGPAI,pai[0],2,pai,caishen);//添加组牌
				recursiveDepth = 0;
				MakePingHu(m_tCheckHuChangleData,m_AnalyseItemHu, recursiveDepth, m_iJingNum);
				m_tCheckHuChangleData.AddCard(i,2);
			}
		}
	}
	return m_bHuFlag;
}


//------------------------------------------------------------------
//十五胡玩法
bool CGameLogic::IsShiShiRedHu_15(int iRedNum)			//十红胡
{
	if (m_tGameRules.b15Type)
	{
		return (10 > iRedNum) && (iRedNum >= 7);
	}
	return (13 > iRedNum) && (iRedNum >= 10);
}
bool CGameLogic::IsOneRedHu_15(int iRedNum)				//一个红胡
{
	return (1 == iRedNum);
}
bool CGameLogic::IsDianHu_15(int iRedNum)					//点红胡
{
	return (15 > iRedNum && iRedNum >= 13);
}
bool CGameLogic::IsShiWuRedHu_15(int iRedNum)				//十五红胡
{
	if (m_tGameRules.b15Type)
	{
		return iRedNum >= 10;
	}
	return iRedNum >= 15;
}
bool CGameLogic::IsAllBlackHu_15(int iRedNum)				//全黑
{
	return 0 == iRedNum;
}

//------------------------------------------------------------------
//六胡玩法
bool CGameLogic::IsRedHu_6(int iRedNum)					//红胡
{
	return (10 > iRedNum) && (iRedNum >= 7);
}
bool CGameLogic::IsOneRedHu_6(int iRedNum)				//一个红
{
	return (1 == iRedNum);
}
bool CGameLogic::IsAllRedHu_6(int iRedNum)				//全红
{
	return (iRedNum >= 10);
}
bool CGameLogic::IsJiuRedHu_6(int iRedNum)				//九红胡
{
	return (9 == iRedNum);
}
bool CGameLogic::IsAllBlackHu_6(int iRedNum)			//全黑
{
	return (0 == iRedNum);
}

//------------------------------------------------------------------
//是否是王钓
bool CGameLogic::IsWanDiao()					
{
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_JIANGPAI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			for(int j=0;j<4;j++)
			{
				if(0==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j])
				{
					continue;
				}
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				if((1==iKingCount && m_byPai==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j]) || (2 == iKingCount && m_byPai == KINGVALUE))
				{
					return true;
				}
			}
		}
	}
	return false;
}

//------------------------------------------------------------------
//是否是王闯
bool CGameLogic::IsWangChuang()					
{
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_WEI == m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			for(int j=0;j<4;j++)
			{
				if(0==m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j])
				{
					continue;
				}
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				if((2==iKingCount && m_byPai==m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j]) || (3==iKingCount && m_byPai == KINGVALUE))
				{
					return true;
				}
			}
		}
		if(ACK_JIANGPAI == m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			if(2==iKingCount && KINGVALUE == m_byPai)
			{
				return true;
			}
		}
	}

	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_WEI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			for(int j=0;j<4;j++)
			{
				if(0==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j])
				{
					continue;
				}
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				if((2==iKingCount && m_byPai==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j]) || (3==iKingCount && m_byPai == KINGVALUE))
				{
					return true;
				}
			}
		}
		if(ACK_JIANGPAI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			if(2==iKingCount && KINGVALUE == m_byPai)
			{
				return true;
			}
		}
	}
	return false;
}

//三王闯
int CGameLogic::IsSanWangChuang()		
{
	for (int i = 0; i < MAX_WEAVE; i++)
	{
		if(ACK_WEI == m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			if(3==iKingCount && KINGVALUE == m_byPai)
			{
				return 16;
			}
		}
		if(ACK_TI == m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			for(int j=0;j<4;j++)
			{
				if(0== m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j])
				{
					continue;
				}
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				if(3==iKingCount && m_byPai== m_tHuCardInforTemp.tWeaveItemArray[i].byCardList[j])
				{
					return 16;
				}
				else if(4 == iKingCount && m_byPai == KINGVALUE)
				{
					return 16;
				}
			}
		}
	}

	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_WEI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			if(3==iKingCount && KINGVALUE == m_byPai)
			{
				return 16;
			}
		}
		if(ACK_TI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			for(int j=0;j<4;j++)
			{
				if(0==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j])
				{
					continue;
				}
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					continue;
				}
				if(3==iKingCount && m_byPai==m_AnalyseItemHu.tWeaveItemArray[i].byCardList[j])
				{
					return 16;
				}
				else if (4 == iKingCount && m_byPai == KINGVALUE)
				{
					return 16;
				}
			}
		}
	}

	return 0;
}

//------------------------------------------------------------------
//当前是否是王炸
bool CGameLogic::IsWangZha()					
{
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_TI == m_tHuCardInforTemp.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_tHuCardInforTemp.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			if(4==iKingCount)
			{
				return true;
			}
		}
	}

	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_TI == m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			int iKingCount = 0;
			for(int j=0;j<4;j++)
			{
				if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
				{
					iKingCount++;
				}
			}
			if(4==iKingCount)
			{
				return true;
			}
		}
	}
	return false;
}

//检测癞子胡
bool CGameLogic::CheckAnyHu(BYTE byStation,BYTE byCardIndex[MAX_INDEX], int iJinNum,BYTE byCurrentCard, BYTE byHuXiWeave, tagHuCardInfo tHuCardInfo[],bool bZiMo,int *pHuAction,tagWeaveItem tWeave[])
{
	clock_t start,finish;
	double totaltime;
	start=clock();
	BYTE byCheck[20]={
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字
		0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字
	};
	int iHuMinType =ACK_NULL;
	int iTypeHuCount[3];
	memset(iTypeHuCount,0,sizeof(iTypeHuCount));
	for(int i=0;i<20;i++)
	{
		if(GetHuCardInfo(byStation,255,byCardIndex,iJinNum,byCheck[i],byHuXiWeave,tHuCardInfo,bZiMo,tWeave, false, true))
		{
			int iTempHu=ACK_CHIHU;
			for(int j = 0;j < 5;j ++)
			{
				if(0 != (tHuCardInfo[j].iHuType & WANG_DIAO))   //王钓
				{
					iTempHu=ACT_WANG_DIAO;
					iTypeHuCount[0]++;
				}
				if(0 != (tHuCardInfo[j].iHuType & WANG_CHUANG)) //王闯
				{
					iTempHu=ACT_WANG_CHUANG;
					iTypeHuCount[1]++;
				}
				if(0 != (tHuCardInfo[j].iHuType & SWANG_CHUANG))//三王闯    
				{ 
					iTempHu=ACT_SWANG_CHUANG;
					iTypeHuCount[2]++;
				}
			}
			if(ACK_NULL== iHuMinType)
			{
				iHuMinType =iTempHu;
			}
			else
			{
				if(iTempHu < iHuMinType)
				{
					iHuMinType = iTempHu;
				}
			}
		}
		else
		{
			finish=clock();
			totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
			return false;
		}
	}
	if(iHuMinType & ACK_CHIHU)
	{
		return false;
	}
	if(pHuAction)
	{
		iHuMinType = ACK_NULL;
		//if (iTypeHuCount[0] > 0)
		//{
		//	iHuMaxType |= ACT_WANG_DIAO;
		//}
		//if (iTypeHuCount[1] > 0)
		//{
		//	iHuMaxType |= ACT_WANG_CHUANG;
		//}
		//if (iTypeHuCount[2] > 0)
		//{
		//	iHuMaxType |= ACT_SWANG_CHUANG;
		//}

		if (iTypeHuCount[0] == 20)//王钓
		{
			iHuMinType |= ACT_WANG_DIAO;
		}
		if (iTypeHuCount[1] == 20)//王闯
		{
			iHuMinType |= ACT_WANG_CHUANG;
		}
		if (iTypeHuCount[2] == 20)//三王闯
		{
			iHuMinType |= ACT_SWANG_CHUANG;
		}
		//if(iTypeHuCount[0]>0 && iTypeHuCount[0]==iTypeHuCount[1])//王钓，王闯
		//{
		//	iHuMinType = ACK_NULL;
		//	iHuMinType |= ACT_WANG_DIAO;
		//	iHuMinType |= ACT_WANG_CHUANG;
		//}
		//if(iTypeHuCount[0]>0 && iTypeHuCount[0]==iTypeHuCount[2])//王钓，三王闯
		//{
		//	iHuMinType = ACK_NULL;
		//	iHuMinType |= ACT_WANG_DIAO;
		//	iHuMinType |= ACT_SWANG_CHUANG;
		//}
		//if(iTypeHuCount[1]>0 && iTypeHuCount[1]==iTypeHuCount[2])//王闯，三王闯
		//{
		//	iHuMinType = ACK_NULL;
		//	iHuMinType |= ACT_WANG_CHUANG;
		//	iHuMinType |= ACT_SWANG_CHUANG;
		//}
		//if(iTypeHuCount[0]>0 && iTypeHuCount[0]==iTypeHuCount[1] && iTypeHuCount[1]==iTypeHuCount[2])//王钓，王闯，三王闯
		//{
		//	iHuMinType = ACK_NULL;
		//	iHuMinType |= ACT_WANG_DIAO;
		//	iHuMinType |= ACT_WANG_CHUANG;
		//	iHuMinType |= ACT_SWANG_CHUANG;
		//}
		if (iHuMinType == ACK_NULL)
		{
			return false;
		}
		*pHuAction = iHuMinType;
	}
	finish=clock();
	totaltime=(double)(finish-start)/CLOCKS_PER_SEC;
	return true;
}

//检测听牌
bool CGameLogic::CanTing(BYTE byStation,BYTE byBeStation,BYTE byCardIndex[MAX_INDEX], int iJinNum,BYTE byHuXiWeave, tagHuCardInfo tHuCardInfo[],bool bZiMo,tagWeaveItem tWeave[], BYTE byHuCardsIndex[MAX_INDEX])
{
	BYTE byCheck[20]={
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字
		0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字
	};
	bool bTing=false;
	int count = 0;
	for(int i=0;i<20;i++)
	{
		if(GetHuCardInfo(byStation,255,byCardIndex,iJinNum,byCheck[i],byHuXiWeave,tHuCardInfo,bZiMo,tWeave))
		{
			bTing=true;
			int index = SwitchToCardIndex(byCheck[i]);
			byHuCardsIndex[index] = 1;
		}
	}
	return bTing;
}
int CGameLogic::GetWangCount()
{
	int iKingNums=0;
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_NULL==m_AnalyseItemHu.tWeaveItemArray[i].iWeaveKind)
		{
			continue;
		}
		
		for(int j = 0;j < 4;j ++)
		{
			if(m_AnalyseItemHu.tWeaveItemArray[i].bIsJing[j])
			{
				iKingNums++;
			}
		}
	}
	return iKingNums;
}

//设置游戏规则
void CGameLogic::SetGameRule(GameRuler &gameRule)
{
	m_tGameRules = gameRule;
}

//设置玩家实际精牌个数
void CGameLogic::SetRealJingNum(BYTE realJingNum[])
{
	if (!realJingNum) return;

	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		m_realJingNum[i] = realJingNum[i];
	}
}