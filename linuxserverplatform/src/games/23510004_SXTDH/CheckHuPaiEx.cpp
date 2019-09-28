#include "stdafx.h"
#include "CheckHuPaiEx.h"
using namespace std;

CheckHuPaiEx::CheckHuPaiEx(void)
{
	CheckHuPai::CheckHuPai();
	InitData();
}

CheckHuPaiEx::~CheckHuPaiEx(void)
{
}
//初始化数据
void CheckHuPaiEx::InitData()
{
	CheckHuPai::InitData();
	memset(m_iHuFenType, 0, sizeof(m_iHuFenType));
	m_byTingMaxFan = 0;//听牌可能的最大番
}

//糊牌                                              胡牌玩家      放炮玩家
bool CheckHuPaiEx::CanHu(GameDataEx *pGameData, BYTE station, BYTE byBeStation, BYTE lastpai, bool zimo)
{
	// 统计消耗
	//WAUTOCOST("CanHu耗时 玩家%d 放炮:%d", station, byBeStation);
	//非自摸的情况下，玩家弃糊状态,不能弃先糊后
	if (pGameData->m_mjAction.bQxHh && !zimo && pGameData->m_bQiHu[station])
	{
		return false;
	}
	bool bHu = false;
	///玩家是否自摸胡牌
	m_bZimo = zimo;
	//最后自摸或点炮的牌
	m_byLastPai = lastpai;
	m_byStation = station;
	m_byBeStation = byBeStation;
	memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		m_byArHandPai[i] = pGameData->m_byArHandPai[station][i];
		if (pGameData->m_byArHandPai[station][i] == 255 && lastpai != 255 && !zimo)
		{
			m_byArHandPai[i] = lastpai;
			break;
		}
	}
	///从小到大排序
	CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
	///获取一副手牌的重要数据
	ChangeHandPaiData(pGameData, m_byArHandPai);
	//清除糊牌类型
	ClearHupaiType(m_byHuType);

	//由于七对和十三幺并不属于平胡，七对和十三幺必须单独检测
	if (CheckQiDuiEx(pGameData)) //玩法有七对则检测，无则注释
	{
		bHu = true;
		if (CheckLongQiDui(pGameData))
		{
			SetAHupaiType(HUPAI_TYPE_LongQiDui, m_byHuType);
		}
		else
			SetAHupaiType(HUPAI_TYPE_QiDui, m_byHuType);
	}
	if (!bHu && CheckPingHu(pGameData))//平糊组牌
	{
		bHu = true;
		if (CheckQiangGangHe(pGameData))
		{
			SetAHupaiType(HUPAI_TYPE_QiangGang, m_byHuType);
		}
	}
	if (!bHu && CheckShiSanYao(pGameData)) //十三幺
	{
		bHu = true;
		SetAHupaiType(HUPAI_TYPE_ShiSanYao, m_byHuType);
	}
	if (bHu)
	{
		memcpy(pGameData->T_CountFen.byHuType[m_byStation], m_byHuType, sizeof(pGameData->T_CountFen.byHuType[m_byStation]));//胡牌类型
	}
	return bHu;
}

//计算胡分，获取胡牌类型
void CheckHuPaiEx::CountHuFen(GameDataEx *pGameData)
{
	//最后自摸或点炮的牌
	BYTE byBeStation = pGameData->T_HuPai.byFangPao;
	bool bZimo = pGameData->T_HuPai.bZimo;
	BYTE byStation = pGameData->T_HuPai.byHu;
	BYTE byHuType[MAX_HUPAI_TYPE];
	memset(byHuType, 255, sizeof(byHuType));
	memset(pGameData->T_CountFen.iFanCount, 0, sizeof(pGameData->T_CountFen.iFanCount));
	memset(m_iHuFenType, 0, sizeof(m_iHuFenType));
	int iFanNum = 1;//翻倍
	int iShuFen = 0;//输分
	if (pGameData->T_HuPai.bZimo)//自摸
	{
		pGameData->T_CountFen.byUserNums++;
		pGameData->T_CountFen.bIsHu[byStation] = true;
		//预判算胡分时候检测胡牌类型，这里直接用。
		memcpy(byHuType, pGameData->T_CountFen.byHuType[byStation], sizeof(byHuType));
		iFanNum = GetHuTypeFans(pGameData, byHuType);
		int iCountScore = 3;
		if (iFanNum == 1) //自摸屁胡
		{
			iCountScore = pGameData->m_tGameRuler.iZiMoScore;
		}

		pGameData->T_CountFen.iFanCount[byStation] = iFanNum * iCountScore;
		iShuFen = 0;
		for (int i = 0;i < PLAY_COUNT;i++) //先算各玩家输分
		{
			int iFen = 0;
			if (byStation == i)
			{
				continue;
			}
			iFen = pGameData->m_mjRule.iGameBase * iCountScore * iFanNum;
			m_iHuFenType[i][0] -= iFen;
			iShuFen += iFen;
			pGameData->AddToHuFenStruct(i, MJ_HU_FEN_TYPE_BEI_ZI_MO, MJ_STATION_BEN_JIA, pGameData->GetPlayerStationRelation(i, byStation), false, -iFen);
		}
		m_iHuFenType[byStation][0] += iShuFen;
		pGameData->AddToHuFenStruct(byStation, MJ_HU_FEN_TYPE_ZI_MO, MJ_STATION_BEN_JIA, MJ_STATION_ALL_JIA, false, iShuFen);
	}
	else
	{
		for (int i = 0;i < PLAY_COUNT; i++)
		{
			if (pGameData->T_HuPai.bHaveHu[i])
			{
				pGameData->T_CountFen.byUserNums++;
				pGameData->T_CountFen.bIsHu[i] = true;
				//预判算胡分时候检测胡牌类型，这里直接用。
				memcpy(byHuType, pGameData->T_CountFen.byHuType[i], sizeof(byHuType));
				iFanNum = GetHuTypeFans(pGameData, byHuType);

				if (pGameData->m_bTing[i]) //报听3家平分
				{
					pGameData->T_CountFen.iFanCount[i] = iFanNum;
					iShuFen = 0;
					for (int j = 0;j < PLAY_COUNT;j++) //先算各玩家输分
					{
						int iFen = 0;
						if (j == i)
						{
							continue;
						}
						iFen = pGameData->m_mjRule.iGameBase * iFanNum;
						m_iHuFenType[j][0] -= iFen;
						iShuFen += iFen;
						if (j == byBeStation)
						{
							pGameData->AddToHuFenStruct(j, MJ_HU_FEN_TYPE_DIAN_PAO, MJ_STATION_BEN_JIA, pGameData->GetPlayerStationRelation(j, i), true, -iFen);
						}
						else
						{
							pGameData->AddToHuFenStruct(j, MJ_HU_FEN_TYPE_DIAN_PAO_HU, pGameData->GetPlayerStationRelation(j, i),
								pGameData->GetPlayerStationRelation(j, byBeStation), true, -iFen);
						}
					}
					m_iHuFenType[i][0] += iShuFen;
					pGameData->AddToHuFenStruct(i, MJ_HU_FEN_TYPE_DIAN_PAO_HU, MJ_STATION_BEN_JIA, pGameData->GetPlayerStationRelation(i, byBeStation), true, iShuFen);
				}
				else
				{
					pGameData->T_CountFen.iFanCount[i] = iFanNum * 3;
					iShuFen = pGameData->m_mjRule.iGameBase * iFanNum * 3;
					m_iHuFenType[byBeStation][0] -= iShuFen;
					m_iHuFenType[i][0] += iShuFen;
					pGameData->AddToHuFenStruct(i, MJ_HU_FEN_TYPE_DIAN_PAO_HU, MJ_STATION_BEN_JIA, pGameData->GetPlayerStationRelation(i, byBeStation), false, iShuFen);
					pGameData->AddToHuFenStruct(byBeStation, MJ_HU_FEN_TYPE_DIAN_PAO, MJ_STATION_BEN_JIA, pGameData->GetPlayerStationRelation(byBeStation, i), false, -iShuFen);
				}
			}
		}
	}

	//算分计数
	memcpy(pGameData->T_CountFen.iHuFenType, m_iHuFenType, sizeof(pGameData->T_CountFen.iHuFenType)); //胡分
}

//根据胡牌类型，获取胡牌倍数
int CheckHuPaiEx::GetHuTypeFans(GameDataEx *pGameData, BYTE byHuType[MAX_HUPAI_TYPE])
{
	int iFanNum = 1;
	if (CheckHupaiType(HUPAI_TYPE_PingHu, byHuType) || !pGameData->m_tGameRuler.bDaHu) //平胡1番
	{
		return iFanNum;
	}

	if (CheckHupaiType(HUPAI_TYPE_QiDui, byHuType) ||
		CheckHupaiType(MJ_HUPAI_TYPE_QingYiSe, byHuType) ||
		CheckHupaiType(MJ_HUPAI_TYPE_YI_TIAO_LONG, byHuType)
		)//清一色，七小对，一条龙
	{
		iFanNum = 3;
	}

	if (CheckHupaiType(HUPAI_TYPE_LongQiDui, byHuType))//豪华七对
	{
		iFanNum = 6;
	}

	if (CheckHupaiType(MJ_HUPAI_TYPE_ShiSanYao, byHuType))//十三幺
	{
		iFanNum = 9;
	}

	return iFanNum;
}

//提取糊牌
void CheckHuPaiEx::ExtractHu(GameDataEx *pGameData, BYTE station, BYTE byHuCard[])
{
	//// 统计消耗
	//LARGE_INTEGER BegainTime ;     
	//LARGE_INTEGER EndTime ;     
	//LARGE_INTEGER Frequency ;     
	//QueryPerformanceFrequency(&Frequency);     
	//QueryPerformanceCounter(&BegainTime) ;
	m_bZimo = false;      //玩家是否自摸胡牌
	m_byStation = station;
	int iHuIndex = 0;
	for (int i = 0;i < MAX_MJ_KIND_NUM;i++)
	{
		if (pGameData->m_byCardType[i] == 255)
		{
			break;
		}
		bool bHu = false;
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
		int iCardIndex = 0;
		for (int b = 0; b < HAND_CARD_NUM; b++)
		{
			if (255 == pGameData->m_byArHandPai[station][b])
			{
				continue;
			}
			m_byArHandPai[iCardIndex++] = pGameData->m_byArHandPai[station][b];
		}
		m_byArHandPai[iCardIndex] = pGameData->m_byCardType[i];
		///从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
		///牌数据转换
		ChangeHandPaiData(pGameData, m_byArHandPai);
		if (CheckQiDuiEx(pGameData))
		{
			bHu = true;
		}
		if (!bHu && CheckPingHu(pGameData))//平糊组牌
		{
			bHu = true;
		}
		if (!bHu && CheckShiSanYao(pGameData)) //十三幺
		{
			bHu = true;
		}
		if (bHu && byHuCard)
		{
			byHuCard[iHuIndex++] = pGameData->m_byCardType[i];
		}
	}
	////输出运行时间
	//QueryPerformanceCounter(&EndTime);
	//GameLog::OutputFile("==ExtractHu耗时：%.1f==",(double)( EndTime.QuadPart - BegainTime.QuadPart )/ Frequency.QuadPart * 1000000);
}

//从手牌中出一张牌，然后提取能糊的牌
bool CheckHuPaiEx::OutCardExtractHu(GameDataEx *pGameData, BYTE station, BYTE byOutCanHuCard[HAND_CARD_NUM],
	BYTE byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM], BYTE byCardRemainNum[MAX_MJ_PAI])
{
	//// 统计消耗
	//LARGE_INTEGER BegainTime ;     
	//LARGE_INTEGER EndTime ;     
	//LARGE_INTEGER Frequency ;     
	//QueryPerformanceFrequency(&Frequency);     
	//QueryPerformanceCounter(&BegainTime) ;
	if (station<0 || station>PLAY_COUNT)
	{
		return false;
	}
	int iCardIndex = 0;
	BYTE byTempCard[HAND_CARD_NUM];
	memset(byTempCard, 255, sizeof(byTempCard));
	BYTE byTestCard_ = 255;
	for (int i = 0; i < HAND_CARD_NUM; i++)
	{
		byTempCard[i] = pGameData->m_byArHandPai[station][i];
	}
	///从小到大排序
	CLogicBase::SelectSort(byTempCard, HAND_CARD_NUM, false);
	m_byStation = station;

	//开始检测
	int iCanHuCardIndex = 0;
	for (int j = 0; j < HAND_CARD_NUM; j++)
	{
		if (byTempCard[j] == 255 || byTempCard[j] == byTestCard_)
			continue;
		//将这张牌打出
		byTestCard_ = byTempCard[j];
		byTempCard[j] = 255;
		int iHuIndex = 0;
		for (int i = 0;i < MAX_MJ_KIND_NUM;i++)
		{
			if (pGameData->m_byCardType[i] == 255)
			{
				break;
			}
			bool bHu = false;
			memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
			iCardIndex = 0;
			for (int b = 0; b < HAND_CARD_NUM; b++)
			{
				if (255 == byTempCard[b])
				{
					continue;
				}
				m_byArHandPai[iCardIndex++] = byTempCard[b];
			}
			m_byArHandPai[iCardIndex] = pGameData->m_byCardType[i];
			///从小到大排序
			CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
			///牌数据转换
			ChangeHandPaiData(pGameData, m_byArHandPai);
			if (CheckQiDuiEx(pGameData))
			{
				bHu = true;
			}
			if (!bHu && CheckPingHu(pGameData))//平糊组牌
			{
				bHu = true;
			}
			if (!bHu && CheckShiSanYao(pGameData)) //十三幺
			{
				bHu = true;
			}
			if (bHu)
			{
				byOutCanHuCard[iCanHuCardIndex] = byTestCard_;
				byHuCard[iCanHuCardIndex][iHuIndex++] = pGameData->m_byCardType[i];
			}
		}

		if (byOutCanHuCard[iCanHuCardIndex] != 255)
		{
			iCanHuCardIndex++;
		}
		//恢复重新检测
		byTempCard[j] = byTestCard_;
	}

	//计算每张牌的剩余数量
	BYTE byTempAllOutCardNum[MAX_MJ_PAI];
	memcpy(byTempAllOutCardNum, pGameData->m_byAllOutCardNum, sizeof(byTempAllOutCardNum));
	for (int i = 0; i < HAND_CARD_NUM; i++)
	{
		if (255 == pGameData->m_byArHandPai[station][i])
		{
			continue;
		}
		byTempAllOutCardNum[pGameData->m_byArHandPai[station][i]] ++;
	}
	BYTE byTingOutCardIndex = pGameData->m_byTingOutCardIndex[station];
	if (byTingOutCardIndex != 255)
	{
		byTempAllOutCardNum[pGameData->m_byArOutPai[station][byTingOutCardIndex]] ++;
	}
	for (int i = 0; i < MAX_MJ_PAI; i++)
	{
		if (byTempAllOutCardNum[i] > 4)
		{
			continue;
		}
		byCardRemainNum[i] = 4 - byTempAllOutCardNum[i];
	}
	bool bCanTing_ = false;
	if (byOutCanHuCard[0] != 255)
	{
		bCanTing_ = true;
	}
	////输出运行时间
	//QueryPerformanceCounter(&EndTime);
	//GameLog::OutputFile("==OutCardExtractHu耗时：%.1f微妙==",(double)( EndTime.QuadPart - BegainTime.QuadPart )/ Frequency.QuadPart * 1000000);
	/*CString str;
	for (int i = 0; i < HAND_CARD_NUM; i++)
	{
	if (byOutCanHuCard[i] == 255)
	continue;
	CString str1;
	str1.Format("$$打出(%d,%d)听::",i,byOutCanHuCard[i]);
	str += str1;
	for (int j = 0; j < MAX_MJ_KIND_NUM;j++)
	{
	if (byHuCard[i][j] == 255)
	break;
	CString str2 = "";
	str2.Format("[胡:%d]",byHuCard[i][j]);
	str += str1;
	}
	}
	GameLog::OutputFile(str);*/
	return bCanTing_;
}

//将将胡
bool	CheckHuPaiEx::CheckAllJiang(GameDataEx *pGameData)
{
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.data[i][0] % 10 != 2 && m_tHuPaiStruct.data[i][0] % 10 != 5 && m_tHuPaiStruct.data[i][0] % 10 != 8)
		{
			return false;
		}
	}
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		BYTE pai = 255;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		switch (pGameData->m_UserGCPData[m_byStation][i].byType)
		{
		case ACTION_CHI://吃牌动作
		{
			return false;
			break;
		}
		case ACTION_PENG:
		case ACTION_BU_GANG:
		case ACTION_AN_GANG:
		case ACTION_MING_GANG:
		{
			if (pai % 10 != 2 && pai % 10 != 5 && pai % 10 != 8)
			{
				return false;
			}
		}
		break;
		}
	}
	return true;
}

///计算七对中四张牌的数量
int CheckHuPaiEx::CountLongQi()
{
	int count = 0;
	for (int i = 0;i < m_tHuTempData.count;++i)
	{
		if (m_tHuTempData.data[i][0] == 255 || m_tHuTempData.data[i][1] < 4 || m_tHuTempData.data[i][1] == 255)
			continue;
		count++;
	}
	return count;
}

///计算七对中四张牌的数量
bool CheckHuPaiEx::CheckHuGen(BYTE lastpai)
{
	if (lastpai == 255)
		return false;
	if (CLogicBase::GetNumInArr(m_byArHandPai, lastpai, HAND_CARD_NUM) >= 4)
		return true;
	return false;
}

//七对：由7个对子组成和牌。不能有混牌充当
bool CheckHuPaiEx::CheckQiDuiEx(GameDataEx *pGameData)
{
	if (pGameData->GetGCPCount(m_byStation) > 0)//不能有吃碰杠
	{
		return false;
	}
	int need = 0;
	for (int i = 0;i < m_tHuTempData.count;++i)
	{
		if (m_tHuTempData.data[i][1] == 0)
			continue;
		if ((m_tHuTempData.data[i][1] & 1) == 1)
			return false;
	}
	return true;
}

///描述：检测能否听牌
///@param  handpai[] 手牌数组, pai 最后一张牌, gcpData[] 吃碰杠数组, MainJing 正精, LessJing 副精, CanOut[] 打出后能听牌的牌（可以有多张）
///@return true 能听，false 不能听
bool CheckHuPaiEx::CanTing(GameDataEx *pGameData, BYTE station, BYTE CanOut[])
{
	if (station<0 || station>PLAY_COUNT)
	{
		return false;
	}
	if (!pGameData->m_mjAction.bTing)
	{
		return false;
	}
	bool bTing = false;
	m_byStation = station;
	m_bZimo = true;
	int iCanNum = 0, iOutNums = 0;
	BYTE byTempArHandPai[HAND_CARD_NUM];
	BYTE byTempOutPai[HAND_CARD_NUM];
	BYTE byPaiNums[49];
	memset(byPaiNums, 0, sizeof(byPaiNums));
	memset(byTempOutPai, 255, sizeof(byTempOutPai));
	for (int i = 0;i < pGameData->m_byArHandPaiCount[station];i++)
	{
		if (255 == pGameData->m_byArHandPai[station][i])
		{
			continue;
		}
		byPaiNums[pGameData->m_byArHandPai[station][i]]++;
	}
	for (int i = 0;i < 49;i++)
	{
		if (byPaiNums[i] > 0)
		{
			byTempOutPai[iOutNums++] = i;
		}
	}
	for (int i = 0;i < iOutNums;i++)
	{
		memset(byTempArHandPai, 255, sizeof(byTempArHandPai));
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
		BYTE byOutPai = byTempOutPai[i];
		int temp = 0, OutNums = 0;
		for (int b = 0;b < pGameData->m_byArHandPaiCount[station];b++)
		{
			if (OutNums == 0 && byOutPai == pGameData->m_byArHandPai[station][b])
			{
				OutNums++;
				continue;
			}
			byTempArHandPai[temp++] = pGameData->m_byArHandPai[station][b];
		}

		memcpy(m_byArHandPai, byTempArHandPai, sizeof(BYTE)*temp);
		/*m_byArHandPai[temp]=byCardType[b];
		m_byLastPai = byCardType[b];*/
		///从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
		///牌数据转换
		ChangeHandPaiData(pGameData, m_byArHandPai);
		m_byJingNum++;

		//胡牌检测
		if (CheckQiDui(pGameData))
		{
			CanOut[iCanNum++] = byOutPai;
			bTing = true;
		}
		else if (CheckPingHu(pGameData))//平糊组牌
		{
			CanOut[iCanNum++] = byOutPai;
			bTing = true;
		}

		m_byJingNum--;
	}
	return bTing;
}

//获取一张可听的牌
BYTE CheckHuPaiEx::GetCanTingCard(GameDataEx *pGameData, BYTE station)
{
	if (station<0 || station>PLAY_COUNT)
	{
		return 255;
	}
	m_byStation = station;
	m_bZimo = true;
	int iOutNums = 0;
	BYTE byTempArHandPai[HAND_CARD_NUM];
	BYTE byTempOutPai[HAND_CARD_NUM];
	BYTE byPaiNums[49];
	memset(byPaiNums, 0, sizeof(byPaiNums));
	memset(byTempOutPai, 255, sizeof(byTempOutPai));
	for (int i = 0;i < pGameData->m_byArHandPaiCount[station];i++)
	{
		if (255 == pGameData->m_byArHandPai[station][i])
		{
			continue;
		}
		byPaiNums[pGameData->m_byArHandPai[station][i]]++;
	}
	for (int i = 0;i < 49;i++)
	{
		if (byPaiNums[i] > 0)
		{
			byTempOutPai[iOutNums++] = i;
		}
	}
	for (int i = 0;i < iOutNums;i++)
	{
		memset(byTempArHandPai, 255, sizeof(byTempArHandPai));
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
		BYTE byOutPai = byTempOutPai[i];
		int temp = 0, OutNums = 0;
		for (int b = 0;b < pGameData->m_byArHandPaiCount[station];b++)
		{
			if (OutNums == 0 && byOutPai == pGameData->m_byArHandPai[station][b])
			{
				OutNums++;
				continue;
			}
			byTempArHandPai[temp++] = pGameData->m_byArHandPai[station][b];
		}

		memcpy(m_byArHandPai, byTempArHandPai, sizeof(BYTE)*temp);
		/*m_byArHandPai[temp]=byCardType[b];
		m_byLastPai = byCardType[b];*/
		///从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
		///牌数据转换
		ChangeHandPaiData(pGameData, m_byArHandPai);
		m_byJingNum++;
		if (CheckPingHu(pGameData))//平糊组牌
		{
			return byOutPai;
		}
		m_byJingNum--;
	}
	return 255;
}

//检测两幅手牌的胡牌是否改变
bool CheckHuPaiEx::IsTingKouChange(GameDataEx *pGameData, BYTE station, BYTE byChangeCard[HAND_CARD_NUM])
{
	BYTE byHuCard[MAX_CANHU_CARD_NUM]; //可胡的牌
	memset(byHuCard, 255, sizeof(byHuCard));
	m_bZimo = false;      //玩家是否自摸胡牌
	m_byStation = station;
	int iHuIndex = 0;
	for (int i = 0;i < MAX_MJ_KIND_NUM;i++)
	{
		if (pGameData->m_byCardType[i] == 255)
		{
			break;
		}
		bool bHu = false;
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
		int iCardIndex = 0;
		for (int b = 0; b < HAND_CARD_NUM; b++)
		{
			if (255 == byChangeCard[b])
			{
				continue;
			}
			m_byArHandPai[iCardIndex++] = byChangeCard[b];
		}
		m_byArHandPai[iCardIndex] = pGameData->m_byCardType[i];
		///从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
		///牌数据转换
		ChangeHandPaiData(pGameData, m_byArHandPai);
		if (CheckQiDuiEx(pGameData))
		{
			bHu = true;
		}
		if (!bHu && CheckPingHu(pGameData))//平糊组牌
		{
			bHu = true;
		}
		if (!bHu && CheckShiSanYao(pGameData)) //十三幺
		{
			bHu = true;
		}
		if (bHu)
		{
			byHuCard[iHuIndex++] = pGameData->m_byCardType[i];
		}
	}

	//比较两个听牌数组
	///从小到大排序
	BYTE byHuCard__[MAX_CANHU_CARD_NUM]; //可胡的牌
	memcpy(byHuCard__, pGameData->m_byOutAfterHuCard[station], sizeof(byHuCard__));
	CLogicBase::SelectSort(byHuCard, MAX_CANHU_CARD_NUM, false);
	CLogicBase::SelectSort(byHuCard__, MAX_CANHU_CARD_NUM, false);
	bool bRet = false;
	for (int i = 0; i < MAX_CANHU_CARD_NUM; i++)
	{
		if (byHuCard[i] != byHuCard__[i])
		{
			bRet = true;
			break;
		}
	}
	return bRet;
}