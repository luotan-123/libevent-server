#include "GameData.h"
GameData::GameData(void)
{			
}
GameData::~GameData(void)
{
}
///初始化数据
void GameData::InitData()
{
	for(int i=0;i<PLAY_COUNT;i++)
	{
		m_bTryBigAction[i]=ACK_NULL;
		for(int j=0;j<7;j++)
		{
			m_tCPTWPArray[i][j].Init();
		}
	}	
	m_tJingData.Init();
	memset(m_bHuJing,false,sizeof(m_bHuJing));
	memset(m_bHuTiOrPao,false,sizeof(m_bHuTiOrPao));
	memset(m_bHuWei,false,sizeof(m_bHuWei));
	memset(m_bForbidOutCard,true,sizeof(m_bForbidOutCard));
	memset(m_byHuPriority,255,sizeof(m_byHuPriority));
	memset(m_byChiPriority,255,sizeof(m_byChiPriority));
	m_bFinish=false;
	memset(m_byWeiPai,0,sizeof(m_byWeiPai));
	memset(m_byOutPai,0,sizeof(m_byOutPai));
	memset(m_bTing,false,sizeof(m_bTing));
	memset(m_bBiePaoSpecialHu,false,sizeof(m_bBiePaoSpecialHu));
	memset(m_bBiePaoSpecialChi,false,sizeof(m_bBiePaoSpecialChi));
	memset(m_bStartHu,false,sizeof(m_bStartHu));
}

int GameData::GetUserChiNum(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<7;i++)
	{
		if(ACK_CHI==m_tCPTWPArray[station][i].iWeaveKind || ACK_CHI_EX==m_tCPTWPArray[station][i].iWeaveKind)
		{
			num++;
		}
	}
	return num;
}

int GameData::GetUserPengNum(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<7;i++)
	{
		if(ACK_PENG==m_tCPTWPArray[station][i].iWeaveKind)
		{
			num++;
		}
	}
	return num;
}
int GameData::GetUserPaoNum(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<7;i++)
	{
		if(ACK_PAO==m_tCPTWPArray[station][i].iWeaveKind)
		{
			num++;
		}
	}
	return num;
}

int GameData::GetUserTiNum(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<7;i++)
	{
		if(ACK_TI==m_tCPTWPArray[station][i].iWeaveKind)
		{
			num++;
		}
	}
	return num;
}

bool GameData::IsUserHavePengPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	for(int i=0;i<7;i++)
	{
		if(ACK_NULL == m_tCPTWPArray[station][i].iWeaveKind)
			continue;
		if(m_tCPTWPArray[station][i].iWeaveKind == ACK_PENG &&  m_tCPTWPArray[station][i].byCardList[0] == pai)
		{
			return true;
		}
	}
	return false;
}

void GameData::AddToCPTWP(BYTE station,tagWeaveItem CPTWPData)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<7;i++)
	{
		if(ACK_NULL==m_tCPTWPArray[station][i].iWeaveKind)
		{
			m_tCPTWPArray[station][i]=CPTWPData;
			break;
		}
	}
}

void GameData::DelleteCPTWPData(BYTE station,BYTE type ,BYTE pai)
{			
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<7;i++)
	{
		if(m_tCPTWPArray[station][i].iWeaveKind == type && (m_tCPTWPArray[station][i].byCardList[0]==pai ||m_tCPTWPArray[station][i].byCardList[1]==pai || m_tCPTWPArray[station][i].byCardList[2]==pai 
			|| m_tCPTWPArray[station][i].byCardList[3]==pai))
		{
			m_tCPTWPArray[station][i].Init();
			break;
		}
	}
	ShortCPTWP(station);
}	

int GameData::CopyCPTWPData(BYTE station,tagWeaveItem gcpData[])
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	for(int i=0;i<7;i++)
	{
		gcpData[i] = m_tCPTWPArray[station][i];
	}
	return GetCPTWPCount(station);
}

int GameData::GetCPTWPCount(BYTE station)
{
	if(station<0 || station>=PLAY_COUNT)
		return 0;
	int count = 0;
	for(int i=0; i<7; i++)
	{
		if(ACK_NULL != m_tCPTWPArray[station][i].iWeaveKind)
		{
			count++;
		}
	}
	return count;
}

void GameData::ShortCPTWP(BYTE station)
{
	if(station<0 || station>=PLAY_COUNT)
		return ;
	int index = 0;
	tagWeaveItem TempGCPData[7];
	for(int i=0; i<7; i++)
	{
		TempGCPData[i] = m_tCPTWPArray[station][i];
		m_tCPTWPArray[station][i].Init();
	}
	for(int i=0;i<7;i++)
	{
		if(ACK_NULL != TempGCPData[i].iWeaveKind)
		{
			m_tCPTWPArray[station][index++] = TempGCPData[i];
		}
	}
}

int  GameData::GetCPTWPXing(BYTE station,BYTE byXing)
{
	//胡息计算
	int num = 0;
	for (BYTE i = 0;i < GetCPTWPCount(station);i++)
	{
		for(int j = 0;j < 4;j ++)
		{
			if(byXing == m_tCPTWPArray[station][i].byCardList[j])
			{
				num ++;
			}
		}
	}
	return num;
}

int  GameData::GetCPTWPEQS(BYTE station)
{
	int num = 0;
	for(int i = 0;i < GetCPTWPCount(station);i ++)
	{
		if(ACK_NULL==m_tCPTWPArray[station][i].iWeaveKind)
		{
			continue;
		}
		for(int j = 0;j < m_tCPTWPArray[station][i].byCardCount;j ++)
		{
			BYTE byCard=m_tCPTWPArray[station][i].byCardList[j];
			if(0==byCard)
			{
				continue;
			}
			BYTE byCardValue =  (byCard & MASK_VALUE);
			if(0x02 == byCardValue || 0x07 == byCardValue || 0x0A == byCardValue)
			{
				num++;
			}
		}
	}
	return num;
}
