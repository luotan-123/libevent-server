#include "CheckCPGAction.h"

CheckCPGAction::CheckCPGAction(void)
{
}

CheckCPGAction::~CheckCPGAction(void)
{
}
///初始化数据
void CheckCPGAction::InitData()
{
}
//能否吃牌
bool CheckCPGAction::CanChi(GameDataEx *pGameData, BYTE station, BYTE pai, BYTE Result[][3], bool bChi[3])
{
	bool canchi = false;
	int index = 0;
	memset(bChi, false, 3 * sizeof(bool));
	if (!pGameData->m_mjAction.bChi || pai == 255)
	{
		return false;
	}
	if (!pGameData->m_mjAction.bChiJing && pGameData->m_StrJing.CheckIsJing(pai))
	{
		return false;
	}
	memset(Result, 255, 9 * sizeof(BYTE));
	if (pai <= CMjEnum::MJ_TYPE_B9)//吃万饼条
	{
		BYTE huase = CMjRef::GetHuaSe(pai);
		if (pGameData->IsHaveAHandPai(station, pai + 1) && pGameData->IsHaveAHandPai(station, pai + 2) && CMjRef::GetHuaSe(pai + 1) == huase && CMjRef::GetHuaSe(pai + 2) == huase)//吃头
		{
			if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(pai + 1) && !pGameData->m_StrJing.CheckIsJing(pai + 2)))
			{
				bChi[0] = true;
				Result[index][0] = pai;
				Result[index][1] = pai + 1;
				Result[index][2] = pai + 2;
				index++;
				canchi = true;
			}
		}
		if (pGameData->IsHaveAHandPai(station, pai + 1) && pGameData->IsHaveAHandPai(station, pai - 1) && CMjRef::GetHuaSe(pai + 1) == huase && CMjRef::GetHuaSe(pai - 1) == huase)//吃中
		{
			if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(pai + 1) && !pGameData->m_StrJing.CheckIsJing(pai - 1)))
			{
				bChi[1] = true;
				Result[index][0] = pai - 1;
				Result[index][1] = pai;
				Result[index][2] = pai + 1;
				index++;
				canchi = true;
			}
		}
		if (pGameData->IsHaveAHandPai(station, pai - 1) && pGameData->IsHaveAHandPai(station, pai - 2) && CMjRef::GetHuaSe(pai - 1) == huase && CMjRef::GetHuaSe(pai - 2) == huase)//吃尾
		{
			if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(pai - 1) && !pGameData->m_StrJing.CheckIsJing(pai - 2)))
			{
				bChi[2] = true;
				Result[index][0] = pai - 2;
				Result[index][1] = pai - 1;
				Result[index][2] = pai;
				index++;
				canchi = true;
			}
		}

	}
	else if (pai <= CMjEnum::MJ_TYPE_FB &&  pGameData->m_mjAction.bChiFeng)
	{//吃东南西北
		switch (pai)
		{
		case CMjEnum::MJ_TYPE_FD://东
		{
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FN) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FX))//东南西
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FN) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FX)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FD;
					Result[index][1] = CMjEnum::MJ_TYPE_FN;
					Result[index][2] = CMjEnum::MJ_TYPE_FX;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FN) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FB))//东南北
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FN) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FB)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FD;
					Result[index][1] = CMjEnum::MJ_TYPE_FN;
					Result[index][2] = CMjEnum::MJ_TYPE_FB;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FX) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FB))//东西北
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FX) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FB)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FD;
					Result[index][1] = CMjEnum::MJ_TYPE_FX;
					Result[index][2] = CMjEnum::MJ_TYPE_FB;
					index++;
					canchi = true;
				}
			}
		}
		break;
		case CMjEnum::MJ_TYPE_FN://南
		{
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FD) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FX))//南东西
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FD) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FX)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FN;
					Result[index][1] = CMjEnum::MJ_TYPE_FD;
					Result[index][2] = CMjEnum::MJ_TYPE_FX;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FD) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FB))//南东北
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FD) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FB)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FN;
					Result[index][1] = CMjEnum::MJ_TYPE_FD;
					Result[index][2] = CMjEnum::MJ_TYPE_FB;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FX) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FB))//南西北
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FX) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FB)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FN;
					Result[index][1] = CMjEnum::MJ_TYPE_FX;
					Result[index][2] = CMjEnum::MJ_TYPE_FB;
					index++;
					canchi = true;
				}
			}
		}
		break;
		case CMjEnum::MJ_TYPE_FX://西
		{
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FD) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FN))//西东南
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FD) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FN)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FX;
					Result[index][1] = CMjEnum::MJ_TYPE_FD;
					Result[index][2] = CMjEnum::MJ_TYPE_FN;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FN) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FB))//西南北
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FN) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FB)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FX;
					Result[index][1] = CMjEnum::MJ_TYPE_FN;
					Result[index][2] = CMjEnum::MJ_TYPE_FB;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FD) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FB))//西东北
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FD) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FB)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FX;
					Result[index][1] = CMjEnum::MJ_TYPE_FD;
					Result[index][2] = CMjEnum::MJ_TYPE_FB;
					index++;
					canchi = true;
				}
			}
		}
		break;
		case CMjEnum::MJ_TYPE_FB://北
		{
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FD) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FN))//北东南
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FD) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FN)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FB;
					Result[index][1] = CMjEnum::MJ_TYPE_FD;
					Result[index][2] = CMjEnum::MJ_TYPE_FN;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FD) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FX))//北东西
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FD) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FX)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FB;
					Result[index][1] = CMjEnum::MJ_TYPE_FD;
					Result[index][2] = CMjEnum::MJ_TYPE_FX;
					index++;
					canchi = true;
				}
			}
			if (pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FN) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FX))//北南西
			{
				if (pGameData->m_mjAction.bChiJing || (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FN) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FX)))
				{
					Result[index][0] = CMjEnum::MJ_TYPE_FB;
					Result[index][1] = CMjEnum::MJ_TYPE_FN;
					Result[index][2] = CMjEnum::MJ_TYPE_FX;
					index++;
					canchi = true;
				}
			}
		}
		break;
		}
	}
	else if (pGameData->m_mjAction.bChiJian &&  pai <= CMjEnum::MJ_TYPE_BAI && pai >= CMjEnum::MJ_TYPE_ZHONG)
	{//吃中发白
		if (pai == CMjEnum::MJ_TYPE_ZHONG && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FA) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_BAI))
		{
			canchi = true;
		}
		else if (pai == CMjEnum::MJ_TYPE_FA && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_ZHONG) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_BAI))
		{
			canchi = true;
		}
		else if (pai == CMjEnum::MJ_TYPE_BAI && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_ZHONG) && pGameData->IsHaveAHandPai(station, CMjEnum::MJ_TYPE_FA))
		{
			canchi = true;
		}
		if (canchi)
		{
			if (pGameData->m_mjAction.bChiJing
				|| (!pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_ZHONG) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_FA) && !pGameData->m_StrJing.CheckIsJing(CMjEnum::MJ_TYPE_BAI))
				)
			{
				Result[0][0] = CMjEnum::MJ_TYPE_ZHONG;
				Result[0][1] = CMjEnum::MJ_TYPE_FA;
				Result[0][2] = CMjEnum::MJ_TYPE_BAI;
			}
			else
			{
				canchi = false;
				memset(Result, 255, 9 * sizeof(BYTE));
			}
		}
	}
	return canchi;
}
///能否碰牌
bool CheckCPGAction::CanPeng(GameDataEx *pGameData, BYTE station, BYTE pai)
{
	if (pGameData->m_mjAction.bPeng  && pai != 255 && pGameData->GetAHandPaiCount(station, pai) >= 2)
	{
		return true;
	}
	return false;
}
//能否坎牌
bool CheckCPGAction::CanKan(GameDataEx *pGameData, BYTE station, BYTE kan[])
{
	if (!pGameData->m_mjAction.bKan)
		return false;
	bool bKan = false;
	int temp = 0, index = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (pGameData->m_byArHandPai[station][i] == 255 || temp == pGameData->m_byArHandPai[station][i])
			continue;
		if (pGameData->GetAHandPaiCount(station, pGameData->m_byArHandPai[station][i]) >= 3)
		{
			temp = pGameData->m_byArHandPai[station][i];
			kan[index] = pGameData->m_byArHandPai[station][i];
			index++;
			bKan = true;
		}
	}
	return bKan;
}
///能否杠牌
bool CheckCPGAction::CanGang(GameDataEx *pGameData, BYTE station, BYTE pai, BYTE Result[][2], bool bMe)
{
	if (!pGameData->m_mjAction.bGang)
		return false;
	bool bGang = false;
	if (bMe)//杠自己的牌：暗杠和补杠
	{
		int temp = 0, index = 0;
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (pGameData->m_byArHandPai[station][i] == 255)
				continue;
			if (pGameData->GetAHandPaiCount(station, pGameData->m_byArHandPai[station][i]) >= 4 && temp != pGameData->m_byArHandPai[station][i])
			{
				Result[index][1] = pGameData->m_byArHandPai[station][i];
				Result[index][0] = ACTION_AN_GANG;
				temp = pGameData->m_byArHandPai[station][i];
				bGang = true;
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
					bGang = true;
					index++;
				}
			}
		}
	}
	else//明杠
	{
		if (pai == 255 || pai == 0)
			return false;
		if (pGameData->GetAHandPaiCount(station, pai) >= 3)
		{
			bGang = true;
			Result[0][1] = pai;
			Result[0][0] = ACTION_MING_GANG;
		}
	}
	return bGang;
}