#include "stdafx.h"
#include "GameDataEx.h"

GameDataEx::GameDataEx(void)
{
	GameData::GameData();
	//memset(m_byDingQue, 255, sizeof(m_byDingQue));
	//memset(m_iGangScore, 0, sizeof(m_iGangScore));
	memset(m_bIsHu, false, sizeof(m_bIsHu));
	memset(m_bQiHu, 0, sizeof(m_bQiHu));//玩家是否弃糊状态
	m_iHuangZhuangCount = 0;
	LoadIni();
	InitData();
}

GameDataEx::~GameDataEx(void)
{
}

//virtual 初始化数据
void GameDataEx::InitData()
{
	GameData::InitData();
	//memset(m_byDingQue, 255, sizeof(m_byDingQue));
	memset(m_iGangScore, 0, sizeof(m_iGangScore));
	memset(m_bIsHu, false, sizeof(m_bIsHu));
	memset(m_bQiHu, 0, sizeof(m_bQiHu));//玩家是否弃糊状态
	memset(m_byOutCanHuCard, 255, sizeof(m_byOutCanHuCard));
	memset(m_byHuCard, 255, sizeof(m_byHuCard));
	memset(m_byOutAfterHuCard, 255, sizeof(m_byOutAfterHuCard));
	memset(m_byCardRemainNum, 0, sizeof(m_byCardRemainNum));

	m_byCurrentOperatorUser = 255;	//当前可操作吃碰杠的玩家
	//起手补花
	for (int i = 0;i < PLAY_COUNT;++i)
	{
		m_bIsBuHua[i] = false;
		m_bIsBuAllHua[i] = false;
		m_vHuapaiTemp[i].clear();
		m_vecQiPengCard[i].clear();
	}
	memset(m_byFangWei, 255, sizeof(m_byFangWei));
	memset(m_byHuaPaiType, 255, sizeof(m_byHuaPaiType));
	memset(m_bIsGangShangGang, 0, sizeof(m_bIsGangShangGang));
	m_byNowOutStation = 255;
	m_tMatchMJ.Init();
	m_byQiHuTurn = 255;
	m_byPengTurn = 255;
	memset(m_bQiPeng, false, sizeof(m_bQiPeng));
	memset(m_iHuaScore, 0, sizeof(m_iHuaScore)); //花胡
	for (int i = 0;i < PLAY_COUNT;++i)
	{
		m_countHuFen[i].Init();     //胡分
		m_countGangFen[i].Init();   //杠分
		//m_countGenZhuang[i].Init(); //跟庄分
		//m_countHorseFen[i].Init();  //马分
	}
}

///初始化数据
void GameDataEx::LoadIni()
{
	GameData::LoadIni();
}

///检查是否存在指定的门牌
bool GameDataEx::IsHaveAMenPai(BYTE pai)
{
	for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
	{
		if (m_MenPai.byMenPai[i] != 255 && m_MenPai.byMenPai[i] == pai)
			return true;
	}
	return false;
}

///得到某种花色牌（0万 1筒 2条)
int GameDataEx::GetAKindPai(BYTE station, BYTE kind)
{
	kind %= 3;
	int count = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] / 10 == kind)
			return m_byArHandPai[station][i];
	}
	return 0;
}

///检查某种花色牌的个数（0万 1筒 2条)
int GameDataEx::GetAKindPaiCount(BYTE station, BYTE kind)
{
	kind %= 3;
	int count = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] == 255)
			continue;
		if (m_byArHandPai[station][i] / 10 == kind)
			count++;
	}
	return count;
}

//ChangePai手牌
//byNewCard门牌
//从门牌中换手中一张牌
//换指定的一张手牌
bool	GameDataEx::ChangeAHandPai(BYTE station, BYTE ChangePai, BYTE byNewCard)
{
	if (station < 0 || station >= PLAY_COUNT)
	{
		return false;
	}
	if (0 == ChangePai || 255 == ChangePai || 0 == byNewCard || 255 == byNewCard)
	{
		return false;
	}
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] == ChangePai && ChangeAMenPai(byNewCard, ChangePai))
		{
			m_byArHandPai[station][i] = byNewCard;
			return true;
		}
	}
	return false;
}

//换指定一张门牌
bool	GameDataEx::ChangeAMenPai(BYTE ChangePai, BYTE byNewCard)
{
	if (0 == ChangePai || 255 == ChangePai || 0 == byNewCard || 255 == byNewCard)
	{
		return false;
	}
	for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
	{
		if (m_MenPai.byMenPai[i] != 255 && m_MenPai.byMenPai[i] == ChangePai)
		{
			m_MenPai.byMenPai[i] = byNewCard;
			return true;
		}
	}
	return false;
}

///名称：IsOutPaiPeople
///描述：是否出牌玩家
///@param 
///@return  true 是 ,false 不是
bool GameDataEx::IsOutPaiPeople(BYTE station)
{
	if (m_byNowOutStation < 0 || m_byNowOutStation >= PLAY_COUNT)
	{
		return false;
	}
	if (m_byArHandPaiCount[station] % 3 != 2)
	{
		return false;
	}
	return m_byNowOutStation == station ? true : false;
}

///获取手牌的牌色数
int GameDataEx::GetPaiSeCount(BYTE station)
{
	int count = 0;
	bool se[3];
	memset(se, 0, sizeof(se));
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] < 0 || m_byArHandPai[station][i] > CMjEnum::MJ_TYPE_B9)
			continue;
		se[m_byArHandPai[station][i] / 10] = true;
	}
	for (int i = 0;i < 3;i++)
	{
		if (se[i])
			count++;
	}
	return count;
}
///是否还有缺门的牌
bool GameDataEx::IsHaveQueMen(BYTE station, BYTE type)
{
	if (type > 2 || type < 0)
		return false;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] < 0 || m_byArHandPai[station][i] > CMjEnum::MJ_TYPE_B9)
			continue;
		if (m_byArHandPai[station][i] / 10 == type)
			return true;
	}
	return false;
}
//获取一张data中没有的牌，用来换牌
BYTE GameDataEx::GetChangePai(BYTE station, BYTE data[])
{
	BYTE pai = 255;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		bool have = false;
		for (int j = 0;j < HAND_CARD_NUM;++j)
		{
			if (data[j] == m_byArHandPai[station][i])
				have = true;//换牌数组中存在该牌，不能拿去替换
		}
		if (!have)
		{
			return m_byArHandPai[station][i];
		}
	}
	return pai;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///名称：AddToGCP
///描述：添加一组数据到杠吃碰数组中
///@param gcpData 要添加的吃碰杠数据
///@return 
void GameDataEx::AddToGCP(BYTE station, TCPGStruct *gcpData)
{
	//GameData::AddToGCP(station,gcpData);
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == m_UserGCPData[station][i].byType)
		{
			m_UserGCPData[station][i].byType = gcpData->byType;
			m_UserGCPData[station][i].iBeStation = gcpData->iBeStation;
			m_UserGCPData[station][i].iOutpai = gcpData->iOutpai;
			m_UserGCPData[station][i].iStation = gcpData->iStation;
			for (int j = 0;j < 4;j++)
			{
				m_UserGCPData[station][i].byData[j] = gcpData->byData[j];
			}
			break;
		}
	}
}

/////////////////////花牌操作////////////////////////////////////////////////////////
///检测某个人是否那张花牌
bool GameDataEx::CheckIsHaveHuaPai(BYTE station, BYTE pai)
{
	for (int i = 0; i < HUA_CARD_NUM; i++)
	{
		if (pai != 255 && m_byArHuaPai[station][i] == pai)
		{
			return true;
		}
	}
	return false;
}

//混乱牌
bool GameDataEx::DisPatchCard()
{
	BYTE index = 0;
	memset(m_MenPai.byMenPai, 255, sizeof(m_MenPai.byMenPai));
	if (m_mjRule.bHaveWan)
	{
		for (int i = 0;i < 9;i++)
		{
			for (int j = 0;j < 4;j++)
			{
				m_MenPai.byMenPai[index] = stcArr_A_Mj_W[i];
				index++;
			}
		}
	}
	if (m_mjRule.bHaveTiao)
	{
		for (int i = 0;i < 9;i++)
		{
			for (int j = 0;j < 4;j++)
			{
				m_MenPai.byMenPai[index] = stcArr_A_Mj_T[i];
				index++;
			}
		}
	}
	if (m_mjRule.bHaveBing)
	{
		for (int i = 0;i < 9;i++)
		{
			for (int j = 0;j < 4;j++)
			{
				m_MenPai.byMenPai[index] = stcArr_A_Mj_B[i];
				index++;
			}
		}
	}
	if (m_mjRule.bHaveHongZhong)
	{
		for (int i = 0;i < 4;i++)
		{
			m_MenPai.byMenPai[index] = CMjEnum::MJ_TYPE_ZHONG;
			index++;
		}
	}
	if (m_mjRule.bHaveFaCai)
	{
		for (int i = 0;i < 4;i++)
		{
			m_MenPai.byMenPai[index] = CMjEnum::MJ_TYPE_FA;
			index++;
		}
	}
	if (m_mjRule.bHaveBaiBan)
	{
		for (int i = 0;i < 4;i++)
		{
			m_MenPai.byMenPai[index] = CMjEnum::MJ_TYPE_BAI;
			index++;
		}
	}
	if (m_mjRule.bHaveFengPai)
	{
		for (int i = 0;i < 4;i++)
		{
			for (int b = 0;b < 4;b++)
			{
				m_MenPai.byMenPai[index] = stcArr_A_Mj_FENG[i];;
				index++;
			}
		}
	}
	if (m_mjRule.bHaveFlower)
	{
		for (int i = 0;i < 8;i++)
		{
			m_MenPai.byMenPai[index] = stcArr_A_Mj_Other[i];
			index++;
		}
	}
	if (m_mjRule.byAllMjCount != index)
	{
		return false;
	}
	///打乱牌
	srand(GetTickCount());
	BYTE temp = 255, data = 0;
	int iCount = 2;
	while (--iCount)
	{
		for (int i = 0;i < index;i++)
		{
			temp = rand() % (index - i) + i;
			data = m_MenPai.byMenPai[temp];
			m_MenPai.byMenPai[temp] = m_MenPai.byMenPai[i];
			m_MenPai.byMenPai[i] = data;
		}
	}

	//根据打骰子情况，确定抓门牌开始索引和结束索引
	//根据不同的麻将不同
	//4张牌为一墩
	int num = 0;
	for (int i = 0; i < 4; i++)
	{
		if (i >= m_MenPai.byGetPaiDir)
		{
			break;
		}
		num += m_mjRule.byMenPaiNum[i];
	}
	num += m_MenPai.byGetPai * 4;
	if (num >= index)
	{
		num = num - index;
	}
	m_MenPai.byStartIndex = num; //门牌起始索引
	m_MenPai.byAllPaiNum = index;//所有牌的数量
	m_MenPai.byPaiNum = index;   //剩余门牌数量
	int iVirtualIndex = m_MenPai.byStartIndex;
	if (iVirtualIndex == 0)
	{
		iVirtualIndex = m_MenPai.byAllPaiNum - 1;
	}
	else
	{
		--iVirtualIndex;
	}
	m_MenPai.byEndIndex = iVirtualIndex;
	return true;
}
//描述：抓一张牌
BYTE GameDataEx::GetPai(int iStation, bool bhead, BYTE *pIndex, BYTE byGetPai)
{
	BYTE paiIndex = 255;//被抓牌的位置
	BYTE card = 255;
	if (bhead)//从前抓
	{
		if (m_MenPai.byStartIndex >= m_MenPai.byAllPaiNum)
		{
			m_MenPai.byStartIndex = 0;
		}

		if (byGetPai != 255) //要抓指定牌，
		{
			bool bIsOk = false;
			int iIndex = 0;
			for (int i = 0; i < m_MenPai.byAllPaiNum; i++)
			{
				if (m_MenPai.byMenPai[i] == byGetPai)
				{
					bIsOk = true;
					iIndex = i;
					break;
				}
			}
			if (bIsOk) //找到了调换位置
			{
				BYTE byTemp = m_MenPai.byMenPai[m_MenPai.byStartIndex];
				m_MenPai.byMenPai[m_MenPai.byStartIndex] = m_MenPai.byMenPai[iIndex];
				m_MenPai.byMenPai[iIndex] = byTemp;
			}
		}

		paiIndex = m_MenPai.byStartIndex;//被抓牌的位置
		card = m_MenPai.byMenPai[m_MenPai.byStartIndex];
		m_MenPai.byMenPai[m_MenPai.byStartIndex] = 255;
		m_MenPai.byStartIndex++;
	}
	else//从后面抓
	{
		if (byGetPai != 255) //要抓指定牌，
		{
			bool bIsOk = false;
			int iIndex = 0;
			for (int i = 0; i < m_MenPai.byAllPaiNum; i++)
			{
				if (m_MenPai.byMenPai[i] == byGetPai)
				{
					bIsOk = true;
					iIndex = i;
					break;
				}
			}
			if (bIsOk) //找到了调换位置
			{
				BYTE byTemp = m_MenPai.byMenPai[m_MenPai.byEndIndex];
				m_MenPai.byMenPai[m_MenPai.byEndIndex] = m_MenPai.byMenPai[iIndex];
				m_MenPai.byMenPai[iIndex] = byTemp;
			}
		}

		paiIndex = m_MenPai.byEndIndex;//被抓牌的位置
		card = m_MenPai.byMenPai[m_MenPai.byEndIndex];
		m_MenPai.byMenPai[m_MenPai.byEndIndex] = 255;
		if (m_MenPai.byEndIndex == 0)
		{
			m_MenPai.byEndIndex = m_MenPai.byAllPaiNum - 1;
		}
		else
		{
			m_MenPai.byEndIndex--;
		}
	}
	if (iStation != 255)
	{
		AddToHandPai(iStation, card);
	}
	if (pIndex != NULL)
	{
		*pIndex = paiIndex;//被抓牌的位置
	}
	return card;
}
BYTE GameDataEx::GetNextStation(BYTE station, bool shun)
{
	BYTE re = 255;
	if (shun)//顺时针
	{
		re = (station + 1) % PLAY_COUNT;
	}
	else
	{
		re = (station + PLAY_COUNT - 1) % PLAY_COUNT;
	}
	return re;
}
//配牌
BYTE GameDataEx::MatchMJ()
{
	CString lw;
	if (!m_mjRule.bIsMatchMJ)
	{
		return 255;
	}

	m_tMatchMJ.byArHandPaiCount[0] = 14;//庄家
	m_tMatchMJ.byArHandPaiCount[1] = 13;
	m_tMatchMJ.byArHandPaiCount[2] = 13;
	m_tMatchMJ.byArHandPaiCount[3] = 13;

	//	m_GameData.m_byArHandPai[0][0]=2;
	//	m_GameData.m_byArHandPai[0][1]=5;
	//	m_GameData.m_byArHandPai[0][2]=9;
	//	m_GameData.m_byArHandPai[0][3]=15;
	//	m_GameData.m_byArHandPai[0][4]=21;
	//	m_GameData.m_byArHandPai[0][5]=21;
	//	m_GameData.m_byArHandPai[0][6]=22;
	//	m_GameData.m_byArHandPai[0][7]=25;
	//	m_GameData.m_byArHandPai[0][8]=26;
	//	m_GameData.m_byArHandPai[0][9]=31;
	//	m_GameData.m_byArHandPai[0][10]=34;
	//	m_GameData.m_byArHandPai[0][11]=31;
	//	m_GameData.m_byArHandPai[0][12]=36;
	//	m_GameData.m_byArHandPaiCount[0]=13;


	m_tMatchMJ.byArHandPai[0][0] = 1;
	m_tMatchMJ.byArHandPai[0][1] = 1;
	m_tMatchMJ.byArHandPai[0][2] = 1;
	m_tMatchMJ.byArHandPai[0][3] = 1;
	m_tMatchMJ.byArHandPai[0][4] = 2;
	m_tMatchMJ.byArHandPai[0][5] = 2;
	m_tMatchMJ.byArHandPai[0][6] = 2;
	m_tMatchMJ.byArHandPai[0][7] = 3;
	m_tMatchMJ.byArHandPai[0][8] = 3;
	m_tMatchMJ.byArHandPai[0][9] = 3;
	m_tMatchMJ.byArHandPai[0][10] = 34;
	m_tMatchMJ.byArHandPai[0][11] = 31;
	m_tMatchMJ.byArHandPai[0][12] = 36;
	m_tMatchMJ.byArHandPai[0][13] = 33;

	m_tMatchMJ.byArHandPai[1][0] = 32;
	m_tMatchMJ.byArHandPai[1][1] = 32;
	m_tMatchMJ.byArHandPai[1][2] = 32;
	m_tMatchMJ.byArHandPai[1][3] = 12;
	m_tMatchMJ.byArHandPai[1][4] = 14;
	m_tMatchMJ.byArHandPai[1][5] = 14;
	m_tMatchMJ.byArHandPai[1][6] = 15;
	m_tMatchMJ.byArHandPai[1][7] = 15;
	m_tMatchMJ.byArHandPai[1][8] = 17;
	m_tMatchMJ.byArHandPai[1][9] = 17;
	m_tMatchMJ.byArHandPai[1][10] = 19;
	m_tMatchMJ.byArHandPai[1][11] = 19;
	m_tMatchMJ.byArHandPai[1][12] = 33;

	m_tMatchMJ.byArHandPai[2][0] = 1;
	m_tMatchMJ.byArHandPai[2][1] = 2;
	m_tMatchMJ.byArHandPai[2][2] = 6;
	m_tMatchMJ.byArHandPai[2][3] = 7;
	m_tMatchMJ.byArHandPai[2][4] = 8;
	m_tMatchMJ.byArHandPai[2][5] = 23;
	m_tMatchMJ.byArHandPai[2][6] = 23;
	m_tMatchMJ.byArHandPai[2][7] = 16;
	m_tMatchMJ.byArHandPai[2][8] = 17;
	m_tMatchMJ.byArHandPai[2][9] = 18;
	m_tMatchMJ.byArHandPai[2][10] = 24;
	m_tMatchMJ.byArHandPai[2][11] = 25;
	m_tMatchMJ.byArHandPai[2][12] = 26;

	m_tMatchMJ.byArHandPai[3][0] = 1;
	m_tMatchMJ.byArHandPai[3][1] = 17;
	m_tMatchMJ.byArHandPai[3][2] = 18;
	m_tMatchMJ.byArHandPai[3][3] = 24;
	m_tMatchMJ.byArHandPai[3][4] = 25;
	m_tMatchMJ.byArHandPai[3][5] = 26;
	m_tMatchMJ.byArHandPai[3][6] = 14;
	m_tMatchMJ.byArHandPai[3][7] = 16;
	m_tMatchMJ.byArHandPai[3][8] = 29;
	m_tMatchMJ.byArHandPai[3][9] = 3;
	m_tMatchMJ.byArHandPai[3][10] = 3;
	m_tMatchMJ.byArHandPai[3][11] = 23;
	m_tMatchMJ.byArHandPai[3][12] = 24;
	int iLastIndex = m_MenPai.byStartIndex - 1;
	if (iLastIndex < 0)
	{
		iLastIndex = m_MenPai.byAllPaiNum - 1;
	}
	int iGetPaiNum = 0;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (0 == i)
		{
			for (int b = 0;b < 14;b++)
			{
				for (int c = 0;c < MAX_MJ_PAI_COUNT;c++)
				{
					if (m_tMatchMJ.byArHandPai[i][b] == m_MenPai.byMenPai[c])
					{
						m_MenPai.byMenPai[c] = 255;
						m_MenPai.byStartIndex++;
						break;
					}
				}
			}
		}
		else
		{
			for (int b = 0;b < 13;b++)
			{
				for (int c = 0;c < MAX_MJ_PAI_COUNT;c++)
				{
					if (m_tMatchMJ.byArHandPai[i][b] == m_MenPai.byMenPai[c])
					{
						m_MenPai.byMenPai[c] = 255;
						m_MenPai.byStartIndex++;
						break;
					}
				}
			}
		}
	}
	int index = 0;
	BYTE byTempMenPai[MAX_MJ_PAI_COUNT];
	memset(byTempMenPai, 255, sizeof(byTempMenPai));
	for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
	{
		if (255 != m_MenPai.byMenPai[i])
		{
			byTempMenPai[index] = m_MenPai.byMenPai[i];
			index++;
		}
	}
	memset(m_MenPai.byMenPai, 255, sizeof(m_MenPai.byMenPai));
	int iStartInddex = m_MenPai.byStartIndex;
	for (int i = 0;i < index;i++)
	{
		if (iStartInddex >= m_MenPai.byAllPaiNum)
		{
			iStartInddex = 0;
		}
		m_MenPai.byMenPai[iStartInddex] = byTempMenPai[i];
		iStartInddex++;
	}
	memcpy(m_byArHandPai[m_byNtStation], m_tMatchMJ.byArHandPai[0], sizeof(m_byArHandPai[m_byNtStation]));
	m_byArHandPaiCount[m_byNtStation] = 14;
	int copyindex = 1;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byNtStation == i)
		{
			continue;
		}
		memcpy(m_byArHandPai[i], m_tMatchMJ.byArHandPai[copyindex], sizeof(m_byArHandPai[i]));
		m_byArHandPaiCount[i] = 13;
		copyindex++;
	}
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		SortHandPai(i, false);
	}
	return m_tMatchMJ.byArHandPai[0][13];
}

//翻鬼
void GameDataEx::TurningString(BYTE &byPai0, BYTE &byPai1)
{
	BYTE byCard = GetPai(255, true, NULL);
	byPai0 = byCard;
	bool bFlag = false;
	if (CMjEnum::MJ_TYPE_W9 == byCard)
	{
		byCard = CMjEnum::MJ_TYPE_W1;
		bFlag = true;
	}
	if (CMjEnum::MJ_TYPE_T9 == byCard)
	{
		byCard = CMjEnum::MJ_TYPE_T1;
		bFlag = true;
	}
	if (CMjEnum::MJ_TYPE_B9 == byCard)
	{
		byCard = CMjEnum::MJ_TYPE_B1;
		bFlag = true;
	}
	if (CMjEnum::MJ_TYPE_BAI == byCard)
	{
		byCard = CMjEnum::MJ_TYPE_FD;
		bFlag = true;
	}
	if (!bFlag)
	{
		byCard = byCard + 1;
	}
	byPai1 = byCard;
}

//判断是否满足跟庄规则
int GameDataEx::CheckIsShouZhangGen(bool bIsNext)
{
	int cout_ = 0;
	int index0 = 0, index1 = 0, index2 = 0, index3 = 0;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		int j = 0;
		for (j = index0; j < index0 + HUA_CARD_NUM;)
		{
			if (CheckIsHuaPai(m_byArOutPai[0][j]))
				j++;
			else
				break;
		}
		index0 = j;

		for (j = index1; j < index1 + HUA_CARD_NUM;)
		{
			if (CheckIsHuaPai(m_byArOutPai[1][j]))
				j++;
			else
				break;
		}
		index1 = j;

		for (j = index2; j < index2 + HUA_CARD_NUM;)
		{
			if (CheckIsHuaPai(m_byArOutPai[2][j]))
				j++;
			else
				break;
		}
		index2 = j;

		for (j = index3; j < index3 + HUA_CARD_NUM;)
		{
			if (CheckIsHuaPai(m_byArOutPai[3][j]))
				j++;
			else
				break;
		}
		index3 = j;
		if (m_byArOutPai[0][index0] == 255 || m_byArOutPai[1][index1] == 255 ||
			m_byArOutPai[2][index2] == 255 || m_byArOutPai[3][index3] == 255)
		{
			break;
		}

		if (m_byArOutPai[0][index0] == m_byArOutPai[1][index1] && m_byArOutPai[1][index1] == m_byArOutPai[2][index2]
			&& m_byArOutPai[2][index2] == m_byArOutPai[3][index3])
		{
			cout_++;
			index0++;
			index1++;
			index2++;
			index3++;
			if (!bIsNext)
			{
				break;
			}
		}
		else
			break;
	}
	return cout_;
}

//获取两个玩家的相对位置关系
BYTE GameDataEx::GetPlayerStationRelation(BYTE byMeStation, BYTE byOtherStation)
{
	if (byMeStation < 0 || byMeStation >= PLAY_COUNT
		|| byOtherStation < 0 || byOtherStation >= PLAY_COUNT)
	{
		return MJ_STATION_NO;
	}
	if (byMeStation == byOtherStation)
	{
		return MJ_STATION_BEN_JIA;
	}
	int i = 0;
	for (i = 0; i < PLAY_COUNT;i++)
	{
		int j = (byMeStation + PLAY_COUNT - 1 - i) % PLAY_COUNT;
		if (j == byOtherStation)
			break;
	}
	if (i == 0)
	{
		return MJ_STATION_XIA_JIA;
	}
	else if (i == 1)
	{
		return MJ_STATION_DUI_JIA;
	}
	else if (i == 2)
	{
		return MJ_STATION_SHANG_JIA;
	}
	return MJ_STATION_NO;
}

//获取byMeStation玩家和非byNoStation的关系
BYTE GameDataEx::GetPlayerStationRelationEx(BYTE byMeStation, BYTE byNoStation)
{
	if (PLAY_COUNT != 4)
	{
		return MJ_STATION_NO;
	}
	BYTE byPlayer1 = 255, byPlayer2 = 255;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (i != byMeStation && i != byNoStation)
		{
			if (byPlayer1 != 255)
			{
				byPlayer1 = i;
			}
			else if (byPlayer2 != 255)
			{
				byPlayer2 = i;
			}
		}
	}
	return GetPlayerStationRelation(byMeStation, byPlayer1) | GetPlayerStationRelation(byMeStation, byPlayer2);
}

//获得剩余门牌
BYTE GameDataEx::GetRemainMenPai(BYTE byRemainMenPai[MAX_REMAIN_MEN_PAI_NUM])
{
	BYTE byNum = 0;
	for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
	{
		if (m_MenPai.byMenPai[i] != 255)
		{
			if (byNum < MAX_REMAIN_MEN_PAI_NUM)
			{
				byRemainMenPai[byNum] = m_MenPai.byMenPai[i];
			}
			byNum++;
		}
	}
	return byNum;
}

//BYTE byHuType[MAX_HUPAI_TYPE];		//胡牌类型 ，255默认填充值
//BYTE byNameType;					//相应的名字， 枚举类型MJ_HU_FEN_TYPE
//BYTE byXiangYingStation[MAX_COUNT_HUPAI_FEN]; //相应玩家，枚举类型MJ_STATION
//int iFanCount;						//胡牌类型的番数，比如大四喜88番，清一色16番
//int iAllFen[MAX_COUNT_HUPAI_FEN];   //输赢的胡分
bool GameDataEx::AddToHuFenStruct(BYTE byStation, BYTE byNameType, BYTE byXiangYingStation, BYTE byXiangYingStationEx, bool bBaoTing, int iAllFen)
{
	if (byStation >= PLAY_COUNT || byStation < 0 || m_countHuFen[byStation].byNums >= 3)
	{
		return false;
	}
	m_countHuFen[byStation].byNameType[m_countHuFen[byStation].byNums] = byNameType;
	m_countHuFen[byStation].byXiangYingStation[m_countHuFen[byStation].byNums] = byXiangYingStation;
	m_countHuFen[byStation].byXiangYingStationEx[m_countHuFen[byStation].byNums] = byXiangYingStationEx;
	//m_countHuFen[byStation].bBaoTing[m_countHuFen[byStation].byNums] = bBaoTing;
	m_countHuFen[byStation].iAllFen[m_countHuFen[byStation].byNums] = iAllFen;
	m_countHuFen[byStation].byNums++;
	return true;
}

//BYTE byGangType[MAX_COUNT_GANG_FEN]; //杠类型 ，枚举类型MJ_GANG_FEN_TYPE
//BYTE byXiangYingStation[MAX_COUNT_GANG_FEN]; //相应玩家，枚举类型MJ_STATION
//int iAllFen[MAX_COUNT_GANG_FEN];//总分
bool GameDataEx::AddToGangFenStruct(BYTE byStation, BYTE byGangType, BYTE byXiangYingStation, bool bBaoTing, int iAllFen)
{
	if (m_countGangFen[byStation].byNums >= MAX_COUNT_GANG_FEN || byGangType == MJ_GANG_FEN_TYPE_NO)
	{
		return false;
	}
	bool bIsXiangTongGang = false;
	int iIndex = 0;
	for (int i = 0; i < m_countGangFen[byStation].byNums; i++)
	{
		if (m_countGangFen[byStation].byGangType[i] == byGangType && m_countGangFen[byStation].bBaoTing[i] == bBaoTing
			&& m_countGangFen[byStation].byXiangYingStation[i] == byXiangYingStation)
		{
			bIsXiangTongGang = true;
			iIndex = i;
			break;
		}
	}
	if (bIsXiangTongGang)
	{
		m_countGangFen[byStation].byGangNums[iIndex] ++;
		m_countGangFen[byStation].iAllFen[iIndex] += iAllFen;
	}
	else
	{
		m_countGangFen[byStation].byGangType[m_countGangFen[byStation].byNums] = byGangType;
		m_countGangFen[byStation].byXiangYingStation[m_countGangFen[byStation].byNums] = byXiangYingStation;
		m_countGangFen[byStation].bBaoTing[m_countGangFen[byStation].byNums] = bBaoTing;
		m_countGangFen[byStation].byGangNums[m_countGangFen[byStation].byNums] ++;
		m_countGangFen[byStation].iAllFen[m_countGangFen[byStation].byNums] = iAllFen;
		m_countGangFen[byStation].byNums++;
	}
	return true;
}

//清除补杠列表分
bool GameDataEx::DelBuGangFenStruct(BYTE byStation, BYTE byGangType, BYTE byXiangYingStation, bool bBaoTing, int iAllFen)
{
	if (m_countGangFen[byStation].byNums >= MAX_COUNT_GANG_FEN || byGangType == MJ_GANG_FEN_TYPE_NO)
	{
		return false;
	}
	bool bIsXiangTongGang = false;
	int iIndex = 0;
	for (int i = 0; i < m_countGangFen[byStation].byNums; i++)
	{
		if (m_countGangFen[byStation].byGangType[i] == byGangType && m_countGangFen[byStation].bBaoTing[i] == bBaoTing
			&& m_countGangFen[byStation].byXiangYingStation[i] == byXiangYingStation)
		{
			bIsXiangTongGang = true;
			iIndex = i;
			break;
		}
	}
	if (bIsXiangTongGang)
	{
		if (m_countGangFen[byStation].byGangNums[iIndex] > 1)
		{
			m_countGangFen[byStation].byGangNums[iIndex] --;
			m_countGangFen[byStation].iAllFen[iIndex] += iAllFen;
		}
		else
		{
			m_countGangFen[byStation].byGangType[iIndex] = MJ_GANG_FEN_TYPE_NO;
		}
		return true;
	}
	return false;
}
////添加买马分
//bool GameDataEx::AddToHorseFenStruct(BYTE byStation,BYTE byBuyHorseType,BYTE byMaiStation,BYTE byBeiMaiStation,int iAllFen)
//{
//	if (m_countHorseFen[byStation].byNums >= MAX_COUNT_HORSE_FEN)
//	{
//		return false;
//	}
//	m_countHorseFen[byStation].byBuyHorseType[m_countHorseFen[byStation].byNums] = byBuyHorseType;
//	m_countHorseFen[byStation].byMaiStation[m_countHorseFen[byStation].byNums] = byMaiStation;
//	m_countHorseFen[byStation].byBeiMaiStation[m_countHorseFen[byStation].byNums] = byBeiMaiStation;
//	m_countHorseFen[byStation].iAllFen[m_countHorseFen[byStation].byNums] = iAllFen;
//	m_countHorseFen[byStation].byNums ++;
//	return true;
//}

//BYTE byGenZhuangType; //类型 【0，默认填充值】【1,跟庄】【2，被跟庄】
//BYTE byXiangYingStation; //相应玩家，枚举类型MJ_STATION
//int iAllFen;//跟庄分
//bool GameDataEx::AddToGenZhuangFenStruct(BYTE byStation,BYTE byGenZhuangType,BYTE byXiangYingStation,int iAllFen)
//{
//	if (m_countGenZhuang[byStation].byNums >= MAX_COUNT_GEN_ZHUANG_FEN)
//	{
//		return false;
//	}
//	m_countGenZhuang[byStation].byGenZhuangType[m_countGenZhuang[byStation].byNums] = byGenZhuangType;
//	m_countGenZhuang[byStation].byXiangYingStation[m_countGenZhuang[byStation].byNums] = byXiangYingStation;
//	m_countGenZhuang[byStation].iAllFen[m_countGenZhuang[byStation].byNums] = iAllFen;
//	m_countGenZhuang[byStation].byNums ++;
//	return true;
//}