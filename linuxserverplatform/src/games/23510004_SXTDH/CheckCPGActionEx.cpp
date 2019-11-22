#include "CheckCPGActionEx.h"

CheckCPGActionEx::CheckCPGActionEx(void)
{
}

CheckCPGActionEx::~CheckCPGActionEx(void)
{
}
///初始化数据
void CheckCPGActionEx::InitData()
{
	CheckCPGAction::InitData();
}
///能否碰牌
bool CheckCPGActionEx::CanPeng(GameDataEx *pGameData, BYTE station, BYTE pai)
{
	if (!pGameData->m_mjAction.bPeng)
	{
		return false;
	}
	if (pGameData->m_mjAction.bQiPeng && pGameData->m_bQiPeng[station]) //打出来不碰、自己抓牌前不准碰
	{
		for (size_t i = 0; i < pGameData->m_vecQiPengCard[station].size(); i++)
		{
			if (pai == pGameData->m_vecQiPengCard[station][i])
			{
				return false;
			}
		}
	}
	if (pai != 255 && pGameData->GetAHandPaiCount(station, pai) >= 2)
	{
		return true;
	}
	return false;
}
///能否杠牌
bool CheckCPGActionEx::CanGang(GameDataEx *pGameData, BYTE station, BYTE pai, BYTE Result[][2], bool bMe, CheckHuPaiEx *pCheckHuData)
{
	if (!pGameData->m_mjAction.bGang)
	{
		return false;
	}

	/////////////////////////////杠牌，听口不能变/////////////////////////////////////////////
	BYTE byTempHandCard[HAND_CARD_NUM];
	bool bb = false;
	if (bMe)//杠自己的牌：暗杠和补杠
	{
		int temp = 0, index = 0;
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (pGameData->GetAHandPaiCount(station, pGameData->m_byArHandPai[station][i]) >= 4 && temp != pGameData->m_byArHandPai[station][i])
			{
				temp = pGameData->m_byArHandPai[station][i];
				memset(byTempHandCard, 255, sizeof(byTempHandCard));
				pGameData->CopyOneUserHandPai(byTempHandCard, station);
				if (pGameData->m_bTing[station]) //检测听口是否改变
				{
					for (int i = 0; i < HAND_CARD_NUM; i++)
					{
						if (byTempHandCard[i] == temp)
						{
							byTempHandCard[i] = 255;
						}
					}
					if (pCheckHuData && pCheckHuData->IsTingKouChange(pGameData, station, byTempHandCard))
					{
						continue;
					}
				}
				Result[index][1] = pGameData->m_byArHandPai[station][i];
				Result[index][0] = ACTION_AN_GANG;
				bb = true;
				index++;
				continue;
			}
			if (pGameData->m_mjAction.bBuGang)
			{
				if (pGameData->IsUserHavePengPai(station, pGameData->m_byArHandPai[station][i]))
				{
					Result[index][1] = pGameData->m_byArHandPai[station][i];
					Result[index][0] = ACTION_BU_GANG;
					temp = pGameData->m_byArHandPai[station][i];
					bb = true;
					index++;
				}
			}
		}
		return bb;
	}
	else//明杠
	{
		if (pai == 255 || pai == 0)
			return false;
		memset(byTempHandCard, 255, sizeof(byTempHandCard));
		pGameData->CopyOneUserHandPai(byTempHandCard, station);
		if (pGameData->GetAHandPaiCount(station, pai) >= 3)
		{
			if (pGameData->m_bTing[station]) //检测听口是否改变
			{
				for (int i = 0; i < HAND_CARD_NUM; i++)
				{
					if (byTempHandCard[i] == pai)
					{
						byTempHandCard[i] = 255;
					}
				}
				if (pCheckHuData && pCheckHuData->IsTingKouChange(pGameData, station, byTempHandCard))
				{
					return false;
				}
			}
			bb = true;
			Result[0][1] = pai;
			Result[0][0] = ACTION_MING_GANG;
		}
	}
	return bb;

}
//杠牌算分,byCard是杠的牌
void CheckCPGActionEx::CountGangFen(GameDataEx *pGameData, int iStation, int iBeStation, BYTE byType)
{
	int iShuGang = 0;
	if (ACTION_AN_GANG == byType)//暗杠每家2分
	{
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			int iFen = 0;
			if (iStation == i)
			{
				continue;
			}
			iFen = 2 * pGameData->m_mjRule.iGameBase;
			pGameData->m_iGangScore[i] -= iFen;
			iShuGang += iFen;
			pGameData->AddToGangFenStruct(i, MJ_GANG_FEN_TYPE_BEI_AN_GANG, pGameData->GetPlayerStationRelation(i, iStation), false, -iFen);
		}
		pGameData->m_iGangScore[iStation] += iShuGang;
		pGameData->AddToGangFenStruct(iStation, MJ_GANG_FEN_TYPE_AN_GANG, MJ_STATION_ALL_JIA, false, iShuGang);
	}
	else if (ACTION_BU_GANG == byType)//补杠每家1分
	{
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			int iFen = 0;
			if (iStation == i)
			{
				continue;
			}
			iFen = 1 * pGameData->m_mjRule.iGameBase;
			pGameData->m_iGangScore[i] -= iFen;
			iShuGang += iFen;
			pGameData->AddToGangFenStruct(i, MJ_GANG_FEN_TYPE_BEI_BU_GANG, pGameData->GetPlayerStationRelation(i, iStation), false, -iFen);
		}
		pGameData->m_iGangScore[iStation] += iShuGang;
		pGameData->AddToGangFenStruct(iStation, MJ_GANG_FEN_TYPE_BU_GANG, MJ_STATION_ALL_JIA, false, iShuGang);
	}
	else if (ACTION_MING_GANG == byType)//明杠一家3分
	{
		if (pGameData->m_bTing[iStation]) //报听点杠，三家都出分
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				int iFen = 0;
				if (iStation == i)
				{
					continue;
				}
				iFen = 1 * pGameData->m_mjRule.iGameBase;
				pGameData->m_iGangScore[i] -= iFen;
				iShuGang += iFen;
				if (i == iBeStation)
				{
					pGameData->AddToGangFenStruct(i, MJ_GANG_FEN_TYPE_FANG_GANG, pGameData->GetPlayerStationRelation(i, iStation), true, -iFen);
				}
				else
				{
					pGameData->AddToGangFenStruct(i, MJ_GANG_FEN_TYPE_BEI_MING_GANG, pGameData->GetPlayerStationRelation(i, iStation), true, -iFen);
				}
			}
			pGameData->m_iGangScore[iStation] += iShuGang;
			pGameData->AddToGangFenStruct(iStation, MJ_GANG_FEN_TYPE_MING_GANG, MJ_STATION_ALL_JIA, true, iShuGang);
		}
		else
		{
			iShuGang = 3 * pGameData->m_mjRule.iGameBase;
			pGameData->m_iGangScore[iBeStation] -= iShuGang;
			pGameData->m_iGangScore[iStation] += iShuGang;

			pGameData->AddToGangFenStruct(iBeStation, MJ_GANG_FEN_TYPE_FANG_GANG, pGameData->GetPlayerStationRelation(iBeStation, iStation), false, -iShuGang);
			pGameData->AddToGangFenStruct(iStation, MJ_GANG_FEN_TYPE_MING_GANG, pGameData->GetPlayerStationRelation(iStation, iBeStation), false, iShuGang);
		}
	}
}

//杠牌算分：：必须先计算胡分，才能计算杠分
void CheckCPGActionEx::CountGangFenEx(GameDataEx *pGameData)
{
	//// 统计消耗
	////AUTOCOST("CountGangFenEx 耗时统计");
	//memset(pGameData->m_iGangScore,0,sizeof(pGameData->m_iGangScore));
	//for (int j = 0;j<5;j++)
	//{
	//	for(int i=0;i<PLAY_COUNT;i++)
	//	{
	//		int iShuGang=0;
	//		if(ACTION_AN_GANG==pGameData->m_UserGCPData[i][j].byType)//暗杠每家2分
	//		{
	//			for(int k=0;k<PLAY_COUNT;k++)
	//			{
	//				int iFen = 0;
	//				if(i==k)
	//				{
	//					continue;
	//				}
	//				iFen = 2* pGameData->m_mjRule.iGameBase;
	//				pGameData->m_iGangScore[k]-=iFen;
	//				iShuGang+=iFen;
	//				pGameData->AddToGangFenStruct(k,MJ_GANG_FEN_TYPE_BEI_AN_GANG,pGameData->GetPlayerStationRelation(k,i),-iFen);
	//			}
	//			pGameData->m_iGangScore[i]+=iShuGang;
	//			pGameData->AddToGangFenStruct(i,MJ_GANG_FEN_TYPE_AN_GANG,MJ_STATION_ALL_JIA,iShuGang);
	//		}
	//		else if(ACTION_BU_GANG==pGameData->m_UserGCPData[i][j].byType)//补杠每家1分(明杠)
	//		{
	//			for(int k=0;k<PLAY_COUNT;k++)
	//			{
	//				int iFen = 0;
	//				if(i==k)
	//				{
	//					continue;
	//				}
	//				iFen = 1* pGameData->m_mjRule.iGameBase;
	//				pGameData->m_iGangScore[k]-=iFen;
	//				iShuGang+=iFen;
	//				pGameData->AddToGangFenStruct(k,MJ_GANG_FEN_TYPE_BEI_BU_GANG,pGameData->GetPlayerStationRelation(k,i),-iFen);
	//			}
	//			pGameData->m_iGangScore[i]+=iShuGang;
	//			pGameData->AddToGangFenStruct(i,MJ_GANG_FEN_TYPE_BU_GANG,MJ_STATION_ALL_JIA,iShuGang);
	//		}
	//		else if(ACTION_MING_GANG==pGameData->m_UserGCPData[i][j].byType)//明杠一家3分（吃杠）
	//		{
	//			iShuGang = 3* pGameData->m_mjRule.iGameBase;
	//			pGameData->m_iGangScore[pGameData->m_UserGCPData[i][j].iBeStation]-=iShuGang;
	//			pGameData->m_iGangScore[i]+=iShuGang;
	//			pGameData->AddToGangFenStruct(pGameData->m_UserGCPData[i][j].iBeStation,MJ_GANG_FEN_TYPE_FANG_GANG,
	//				pGameData->GetPlayerStationRelation(pGameData->m_UserGCPData[i][j].iBeStation,i),-iShuGang);
	//			pGameData->AddToGangFenStruct(i,MJ_GANG_FEN_TYPE_MING_GANG,
	//				pGameData->GetPlayerStationRelation(i,pGameData->m_UserGCPData[i][j].iBeStation),iShuGang);
	//		}
	//	}
	//}
}

//减去抢杠胡所得杠分,减补杠分
void CheckCPGActionEx::MinusGangFen(GameDataEx *pGameData, int iStation, int iBeStation)
{
	if (pGameData->m_mjRule.bIsSuanGangHu)
	{
		return;
	}
	int iShuGang = 0;
	for (int i = 0;i < PLAY_COUNT;i++)//算赢分
	{
		int iFen = 0;
		if (iStation == i)
		{
			continue;
		}
		iFen = 1 * pGameData->m_mjRule.iGameBase;
		pGameData->m_iGangScore[i] += iFen;
		iShuGang += iFen;
		pGameData->DelBuGangFenStruct(i, MJ_GANG_FEN_TYPE_BEI_BU_GANG, pGameData->GetPlayerStationRelation(i, iStation), false, iFen);
	}
	pGameData->m_iGangScore[iStation] -= iShuGang;
	pGameData->DelBuGangFenStruct(iStation, MJ_GANG_FEN_TYPE_BU_GANG, MJ_STATION_ALL_JIA, false, -iShuGang);
}