#include "ServerManage.h"

#ifndef USER_IDENTITY_TYPE_SUPER
#define USER_IDENTITY_TYPE_SUPER 1
#endif

//构造函数
CServerGameDesk::CServerGameDesk(void) :CGameDesk(ALL_ARGEE)
{
	m_TGameData.InitAllData();

	bzero(m_iTotalCard, sizeof(m_iTotalCard));		//总的牌	
	m_iThisDiZhu = 0;			//本局底注      
	m_i64ThisDingZhu = 0;			//本局总下注上限	
	memset(m_bSuperFlag,false,sizeof(m_bSuperFlag));
	m_bySuperMaxDesk = 255;			//超端设置的最大玩家
	m_bySuperMinDesk = 255;			//超端设定的最小玩家
	//断线重连需要用到的时间数据
	m_iLastTimerTime = 1000;
	m_dwLastGameStartTime = 0;
	m_bBaoZiYouXi = false;
	m_bHuiErPai = false;
	m_b235ShaBaoZi = false;
	m_iFengDing = 0;
	m_tZongResult.Init();
	m_iAllTurnCount = 10;
	memset(m_bWinPlayer,0,sizeof(m_bWinPlayer));
	m_DiFen = 1;
	m_GameAutoPlay = false;
	memset(m_bAuto,false,sizeof(m_bAuto));
	m_byGameAutoPlayTime = 300;///默认5分钟

	m_ChouMaMoney[0]=1;
	m_ChouMaMoney[1]=2;
	m_ChouMaMoney[2]=4;
	m_ChouMaMoney[3]=8;

	m_bAlreadSet = false;
}

/*--------------------------------------------------------------------------*/
//加载ini配置文件
void	CServerGameDesk::LoadIni()
{
	char szfile[256] = "";
	sprintf(szfile, "%s%d_s.ini", CINIFile::GetAppPath().c_str(), NAME_ID);
	CINIFile f(szfile);

	string key = "game";
	m_TGameData.m_byBeginTime = f.GetKeyVal(key, "begintime", 20);
	m_TGameData.m_byThinkTime = f.GetKeyVal(key, "thinktime", 15);
	m_TGameData.m_iDiZhu = f.GetKeyVal(key, "DiZhu", 1);				//最小单注下注
	m_TGameData.m_i64DingZhu = f.GetKeyVal(key, "DingZhu", 100000);		//最大总注
	m_byGameAutoPlayTime = f.GetKeyVal(key, "iGameAutoPlayTime",300);////房卡场设置的托管时间
	//金币场配置默认玩法
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD)
	{
		m_TGameData.m_iDiZhu = GetGameBasePoint();
		m_DiFen = m_TGameData.m_iDiZhu;

		m_iAllTurnCount = f.GetKeyVal(key, "iAllTurnCount",10);		//总轮数
		m_bHuiErPai = f.GetKeyVal(key, "LaiZi",0);					//癞子
		m_b235ShaBaoZi = f.GetKeyVal(key, "BaoZi235",1);			//235杀豹子
		m_bBaoZiYouXi = f.GetKeyVal(key, "BaoZiHaveXi",0);			//豹子有喜
		m_Logic.m_bLogic235ShaBaoZi = m_b235ShaBaoZi;
		m_CompareCardAdd = f.GetKeyVal(key, "CompareCardAdd",0);	//比牌加倍
		m_MenPai = f.GetKeyVal(key, "MenPai",0);					//焖牌 0 不焖，1 焖一轮，2 焖2轮 ,3 焖3轮
		m_TeSuPaiXing = f.GetKeyVal(key, "TeSuPaiXing",2);			//特殊牌型比较 1 顺子>同花 ;2 顺子<同花
		m_playMode = f.GetKeyVal(key, "gameIdx", 1);				//1经典模式，2疯狂模式
		m_Logic.m_byTeSuPaiXing = m_TeSuPaiXing;
	}
}
/*--------------------------------------------------------------------------*/
bool CServerGameDesk::InitDeskGameStation()
{
	LoadDynamicConfig();

	return true;
}
/*--------------------------------------------------------------------------*/
//根据房间ID加载配置文件
BOOL	CServerGameDesk::LoadExtIni(int iRoomID)
{
	char szKey[20];
	char nid[20] = "";
	sprintf(nid, "%d", NAME_ID);
	sprintf(szKey, "%s_%d", nid, iRoomID);
	string szSec = szKey;
	string s = CINIFile::GetAppPath();/////本地路径
	CINIFile f(s + nid + "_s.ini");

	m_TGameData.m_iDiZhu = f.GetKeyVal(szSec, "DiZhu", m_TGameData.m_iDiZhu); //最小单注下注
	m_TGameData.m_i64DingZhu = f.GetKeyVal(szSec, "DingZhu", m_TGameData.m_i64DingZhu); //最大总注
	m_byGameAutoPlayTime = f.GetKeyVal(szSec,"iGameAutoPlayTime",m_byGameAutoPlayTime);////房卡场设置的托管时间
	//金币场配置默认玩法
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD)
	{
		m_bHuiErPai = f.GetKeyVal(szSec, "LaiZi",m_bHuiErPai);						//癞子
		m_b235ShaBaoZi = f.GetKeyVal(szSec, "BaoZi235",m_b235ShaBaoZi);				//235杀豹子
		m_bBaoZiYouXi = f.GetKeyVal(szSec, "BaoZiHaveXi",m_bBaoZiYouXi);			//豹子有喜
		m_Logic.m_bLogic235ShaBaoZi = m_b235ShaBaoZi;
		m_CompareCardAdd = f.GetKeyVal(szSec, "CompareCardAdd",m_CompareCardAdd);	//比牌加倍
		m_MenPai = f.GetKeyVal(szSec, "MenPai",m_MenPai);							//焖牌 0 不焖，1 焖一轮，2 焖2轮 ,3 焖3轮
		m_TeSuPaiXing = f.GetKeyVal(szSec, "TeSuPaiXing",m_TeSuPaiXing);			//特殊牌型比较 1 顺子>同花 ;2 顺子<同花
		m_Logic.m_byTeSuPaiXing = m_TeSuPaiXing;
		m_playMode = f.GetKeyVal(szSec, "gameIdx", m_playMode);						//1经典模式，2疯狂模式
	}
	return TRUE;
}
/*--------------------------------------------------------------------------*/
//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
}

/*-------------------------------------------------------------------*/
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	switch (uTimerID)
	{
	case TIME_SEND_CARD:
		{
			KillTimer(TIME_SEND_CARD);
			if (GetGameStation() == GS_SEND_CARD)
			{
				//开始发牌
				SendCard();
			}
			return true;
		}
	case TIME_BEGIN_PLAY:	//发牌完成 开始下注计时器
		{
			KillTimer(TIME_BEGIN_PLAY);
			if (GetGameStation() == GS_SEND_CARD)
			{
				BeginPlayUpGrade();
			}
			return true;
		}
	case TIME_WAIT_ACTION:	//操作时间到 自动弃牌
		{
			KillTimer(TIME_WAIT_ACTION);
			if (IsGoldRoom() && GetGameStation() == GS_PLAY_GAME)
			{
				//自动弃牌
				AutoGiveUp();
				return true;
			}
			if (!IsGoldRoom() && m_GameAutoPlay)
			{	
				OnHandleUserRequestAuto(m_TGameData.m_byCurrHandleDesk);
			}
			return true;
		}
	case TIME_NOTICE_ACTION:	//通知下注
		{
			KillTimer(TIME_NOTICE_ACTION);
			if (GetGameStation() == GS_PLAY_GAME)
			{
				NoticeUserAction();
			}

			//记录时间段，用于断线重连所是需要的剩余时间段
			m_iLastTimerTime = m_TGameData.m_byThinkTime + 1;
			m_dwLastGameStartTime = time(NULL);

			return true;
		}
	case TIME_GAME_FINISH:	//游戏结束
		{
			KillTimer(TIME_GAME_FINISH);
			if (GetGameStation() == GS_PLAY_GAME)
			{
				GameFinish(0, GF_NORMAL);
			}
			return true;
		}
	case TIME_GAME_TUOGUAN: //托管定时器
		{
			KillTimer(TIME_GAME_TUOGUAN);
			BYTE byStatus = GetGameStation();
			if(byStatus == GS_PLAY_GAME)
			{
				if (!IsGoldRoom() && m_GameAutoPlay)
				{	
					AutoGiveUp();
				}
			}
			return true;
		}
	case TIME_GAME_AUTO_BEGIN:
		{
			KillTimer(TIME_GAME_AUTO_BEGIN);
			if (IsPlayGame(0))
			{
				return true;
			}
			for(int i = 0;i < PLAY_COUNT;i ++)
			{
				if(m_bAuto[i])
				{
					HandleNotifyMessage(i,MSG_ASS_LOADER_GAME_AGREE,NULL,0,false);
				}
			}
		}
		return true;
	case TIME_GAME_WAIT_ARGEE:
		{
			KillTimer(TIME_GAME_WAIT_ARGEE);
			BYTE byStatus = GetGameStation();
			if(byStatus == GS_WAIT_NEXT || byStatus == GS_WAIT_ARGEE)
			{
				for(int i = 0;i < PLAY_COUNT;i++)
				{
					if (GetUserIDByDeskStation(i)<=0)
					{
						continue;
					}
					if (m_UserReady[i])
					{
						continue;
					}
					OnHandleUserRequestAuto(i);
				}
			}
			return true;
		}
	case TIME_AUTO_FOLLOW:
	{
		KillTimer(TIME_AUTO_FOLLOW);
		AutoFollow();

		return true;
	}
	case TIME_AUTO_GIVEUP:
	{
		KillTimer(TIME_AUTO_GIVEUP);
		AutoGiveUp();

		return true;
	}
	}
	return CGameDesk::OnTimer(uTimerID);
}
/*-------------------------------------------------------------------*/
///自动弃牌

void	CServerGameDesk::AutoGiveUp(BYTE byDeskStation/* = 255*/)
{
	C_S_UserGiveUp TUserGiveUp;
	if (byDeskStation == 255)
	{
		TUserGiveUp.byDeskStation = m_TGameData.m_byCurrHandleDesk;
		OnHandleUserGiveUp(m_TGameData.m_byCurrHandleDesk, &TUserGiveUp);
	}
	else if(byDeskStation < PLAY_COUNT && m_TGameData.m_iUserState[byDeskStation] != STATE_ERR)
	{
		TUserGiveUp.byDeskStation = byDeskStation;
		OnHandleUserGiveUp(byDeskStation, &TUserGiveUp);
	}
}

//自动比牌
void	CServerGameDesk::AutoBiPai()
{
	C_S_UserBiPai biPai;
	biPai.byDeskStation = m_TGameData.m_byCurrHandleDesk;
	for (int i = 0; i < PLAY_COUNT; i ++)
	{
		if (i == m_TGameData.m_byCurrHandleDesk)
		{
			continue;
		}
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_FAIL != m_TGameData.m_iUserState[i])
		{
			biPai.byBeDeskStation = i;
			break;
		}
	}
	OnHandleUserOpenCard(m_TGameData.m_byCurrHandleDesk, &biPai);
}

//自动跟注
void	CServerGameDesk::AutoFollow()
{
	C_S_UserGenZhu genZhu;
	genZhu.byDeskStation = m_TGameData.m_byCurrHandleDesk;

	OnHandleUserFollow(m_TGameData.m_byCurrHandleDesk, &genZhu);
}

/*-------------------------------------------------------------------*/
//检测是否结束
bool	CServerGameDesk::CheckFinish()
{
	if (m_iAllTurnCount < m_iGameTurn)
	{
		return true;
	}
	int iRemaindCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_FAIL != m_TGameData.m_iUserState[i])
		{
			iRemaindCount++;
		}
	}
	//剩下最后一个人了 结束
	if (iRemaindCount <= 1)
	{
		return true;
	}
	//是否所有人都处于等待开牌状态
	bool bAllWaiteOpen = true;
	//全部都下注达到了顶住 下注都封顶了 等待开牌 
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (STATE_NORMAL == m_TGameData.m_iUserState[i] || STATE_LOOK == m_TGameData.m_iUserState[i])
		{
			bAllWaiteOpen = false;
		}
	}
	if (bAllWaiteOpen)
	{
		return true;
	}

	return false;  //没有结束
}
/*-------------------------------------------------------------------*/
//获取最终胜利的玩家
BYTE	CServerGameDesk::GetFinalWiner()
{
	//判断还有几个未弃牌的玩家
	memset(m_bWinPlayer,0,sizeof(m_bWinPlayer));
	vector<BYTE> vecPlayer;
	int iCount = 0;
	BYTE byWiner = 255;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_FAIL != m_TGameData.m_iUserState[i])
		{
			iCount++;
			m_bWinPlayer[i] = true;
			byWiner = i;
		}
	}

	//只有一个正常玩家 那么这个玩家就是赢家了
	if (iCount <= 1)
	{	
		return byWiner;
		
	}
	
	//多人，先找谁最大
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (i == byWiner)
		{
			continue;
		}
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_FAIL != m_TGameData.m_iUserState[i])
		{
			int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byWiner], m_TGameData.m_byUserCardCount[byWiner], m_TGameData.m_byUserCard[i], m_TGameData.m_byUserCardCount[i]);
			//被比较着的牌大
			if (-1 == IsWin)
			{
				byWiner = i;
			}
		}
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (i == byWiner)
		{
			continue;
		}
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_FAIL != m_TGameData.m_iUserState[i])
		{
			int IsWin = m_Logic.CompareCardEx(m_TGameData.m_byUserCard[byWiner], m_TGameData.m_byUserCardCount[byWiner], m_TGameData.m_byUserCard[i], m_TGameData.m_byUserCardCount[i]);
			//被比较着的牌大
			if (IsWin > 0)
			{
				m_bWinPlayer[i] = false;
			}
		}
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_bWinPlayer[i])
		{
			vecPlayer.push_back(i);
		}
	}
	return vecPlayer[0];
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser)
{
	if (bWatchUser || bDeskStation >= PLAY_COUNT)
	{
		return TRUE;
	}
	switch (assistID)
	{
	case MSG_ASS_LOADER_GAME_AGREE:		//用户同意游戏
		{
			m_UserReady[bDeskStation]=true;
			return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
		}
	case C_S_SUPER_SET://超级客户端设定
		{
			if (size != sizeof(C_S_SuperUserSet))
			{
				return false;
			}

			C_S_SuperUserSet *pSuperSet = (C_S_SuperUserSet *)pData;
			if (NULL == pSuperSet)
			{
				return false;
			}
			//响应超端设定
			OnHandleSuperSet(bDeskStation, pSuperSet);
			return true;
		}
	case C_S_LOOK_CARD:		//玩家看牌
		{
			if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE )
			{
				return true;
			}
			if (size != sizeof(C_S_LookCard))
			{
				return false;
			}

			//不焖牌
			if (0 == m_MenPai)
			{
				OnHandleUserLookCard(bDeskStation, pData);
				return true;
			}
			//焖牌
			if (m_iGameTurn <= m_MenPai)
			{
				return false;
			}

			OnHandleUserLookCard(bDeskStation, pData);
			return true;
		}
	case C_S_GEN_ZHU:		//玩家跟注
		{
			if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE )
			{
				return true;
			}
			if (size != sizeof(C_S_UserGenZhu))
			{
				return false;
			}
			OnHandleUserFollow(bDeskStation, pData);
			return true;
		}
	case C_S_JIA_ZHU:		//玩家加注
		{
			if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE )
			{
				return true;
			}
			if (size != sizeof(C_S_JiaZhu))
			{
				return false;
			}
			OnHandleUserAddNote(bDeskStation, pData);
			return true;
		}
	case C_S_GIVE_UP:	//玩家弃牌
		{
			if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE )
			{
				return true;
			}

			if (size != sizeof(C_S_UserGiveUp))
			{
				return false;
			}
			OnHandleUserGiveUp(bDeskStation, pData);
			return true;
		}
	case C_S_BI_PAI:	//玩家比牌
		{
			if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE )
			{
				return true;
			}
			if (size != sizeof(C_S_UserBiPai))
			{
				return false;
			}

			OnHandleUserOpenCard(bDeskStation, pData);
			return true;
		}
	
	}
	return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
}

//超端设置
void	CServerGameDesk::OnHandleSuperSet(BYTE byDeskStation, C_S_SuperUserSet *pSuperUserSet)
{
	SpuerProof(byDeskStation);
	S_C_SuperUserSetResult TSuperUserSetErrorResult;
	if (NULL == pSuperUserSet)
	{
		TSuperUserSetErrorResult.byCode = 1;
		SendGameData(byDeskStation, &TSuperUserSetErrorResult, sizeof(TSuperUserSetErrorResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_SET_RESULT, 0);
		return;
	}
	if (pSuperUserSet->byMaxDesk == pSuperUserSet->byMinDesk)
	{
		TSuperUserSetErrorResult.byCode = 4;
		SendGameData(byDeskStation, &TSuperUserSetErrorResult, sizeof(TSuperUserSetErrorResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_SET_RESULT, 0);
		return;
	}
	//是否超端
	if (!m_bSuperFlag[byDeskStation])
	{
		TSuperUserSetErrorResult.byCode = 5;
		SendGameData(byDeskStation, &TSuperUserSetErrorResult, sizeof(TSuperUserSetErrorResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_SET_RESULT, 0);
		return;
	}

	if (GetGameStation() == GS_PLAY_GAME)
	{
		SendGameMessage(byDeskStation, "游戏状态不能设置超端", SMT_EJECT);
		return;
	}

	//保存超端设置结果
	m_bySuperMaxDesk = pSuperUserSet->byMaxDesk;
	m_bySuperMinDesk = pSuperUserSet->byMinDesk;

	S_C_SuperUserSetResult TSuperUserSetResult;
	TSuperUserSetResult.byCode = 0;
	TSuperUserSetResult.byDeskStation = byDeskStation;
	TSuperUserSetResult.byMaxDesk = pSuperUserSet->byMaxDesk;
	TSuperUserSetResult.byMinDesk = pSuperUserSet->byMinDesk;

	long userID = GetUserIDByDeskStation(byDeskStation);
	if (userID > 0)
	{
		SendGameData(byDeskStation, &TSuperUserSetResult, sizeof(TSuperUserSetResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_SET_RESULT, 0);
	}
}
/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserLookCard(BYTE byDeskStation, void * pData)
{
	C_S_LookCard * pLookCard = (C_S_LookCard *)pData;
	if (NULL == pLookCard)
	{
		return;
	}

	// 检测当前操作玩家是否合法
	if (m_TGameData.m_iUserState[byDeskStation] == STATE_ERR
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_FAIL)
	{
		return;
	}

	//记录玩家的状态
	m_TGameData.m_iUserState[byDeskStation] = STATE_LOOK;
	
	//判断是否可加注逻辑
	//m_byBetStation上次下注的玩家，m_TGameData.m_byCurrHandleDesk为当前下注玩家
	memset(m_TGameData.m_bCanAdd[byDeskStation], 0, sizeof(m_TGameData.m_bCanAdd[byDeskStation]));
	if (byDeskStation == m_TGameData.m_byCurrHandleDesk)
	{
		for (int i = 0; i < E_CHOUMA_COUNT; i++)
		{
			__int64 i64XiaZhuNum = m_ChouMaMoney[i];
			if (!IsPrivateRoom())
			{
				if ((m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum) /** GetGameBasePoint()*/ <= GetUserMoney())
				{
					m_TGameData.m_bCanAdd[byDeskStation][i] = true;
				}
				else
				{
					m_TGameData.m_bCanAdd[byDeskStation][i] = false;
				}
			}
			else
			{
				m_TGameData.m_bCanAdd[byDeskStation][i] = true;
			}

			if(m_byBetStation != 255 && m_TGameData.m_iUserState[m_byBetStation] != STATE_LOOK
				&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_LOOK)
			{
				long long i64XiaZhu_ = m_TGameData.m_i64XiaZhuBase * 2;
				if (m_ChouMaMoney[i] <= i64XiaZhu_)
				{
					m_TGameData.m_bCanAdd[byDeskStation][i] = false;
					continue;
				}
			}
			if(m_byBetStation != 255 && m_TGameData.m_iUserState[m_byBetStation] == STATE_LOOK
				&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_LOOK)//前一家不看牌
			{
				long long i64XiaZhu_ = m_TGameData.m_i64XiaZhuBase / 2;
				if (m_ChouMaMoney[i] <= i64XiaZhu_)
				{
					m_TGameData.m_bCanAdd[byDeskStation][i] = false;
					continue;
				}
			}
			else
			{
				if (m_ChouMaMoney[i] <= m_TGameData.m_i64CurrZhuBase)
				{
					m_TGameData.m_bCanAdd[byDeskStation][i] = false;
					continue;
				}
			}
		}

		for (int i = 0; i < E_CHOUMA_COUNT; i++)
		{
			if (m_ChouMaMoney[i] < m_TGameData.m_i64XiaZhuMaxBase)
			{
				m_TGameData.m_bCanAdd[byDeskStation][i] = false;
			}
		}
	}

	S_C_LookCardResult TLookCardResult;
	TLookCardResult.byDeskStation = pLookCard->byDeskStation;
	TLookCardResult.iUserState = m_TGameData.m_iUserState[byDeskStation];
	TLookCardResult.byUserCardCount = m_TGameData.m_byUserCardCount[byDeskStation];
	m_UserCardType[pLookCard->byDeskStation] = m_Logic.GetCardShape(m_TGameData.m_byUserCard[pLookCard->byDeskStation],3);
	TLookCardResult.iCardType = m_UserCardType[pLookCard->byDeskStation];
	memcpy(TLookCardResult.bCanAdd, m_TGameData.m_bCanAdd[byDeskStation], sizeof(TLookCardResult.bCanAdd));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		memset(TLookCardResult.byUserCard, 0, sizeof(TLookCardResult.byUserCard));
		if (i == pLookCard->byDeskStation)
		{
			memcpy(TLookCardResult.byUserCard, m_TGameData.m_byUserCard[i], sizeof(TLookCardResult.byUserCard));
		}

		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TLookCardResult, sizeof(TLookCardResult), MSG_MAIN_LOADER_GAME, S_C_LOOK_CARD_RESULT, 0);
		}
	}

	memset(TLookCardResult.byUserCard,0,sizeof(TLookCardResult.byUserCard));
	SendWatchData(&TLookCardResult, sizeof(TLookCardResult), MSG_MAIN_LOADER_GAME, S_C_LOOK_CARD_RESULT, 0);
}

/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserFollow(BYTE byDeskStation, void * pData)
{
	C_S_UserGenZhu * pUserFollow = (C_S_UserGenZhu *)pData;
	if (NULL == pUserFollow)
	{
		return;
	}

	// 检测当前操作玩家是否合法
	if (byDeskStation != m_TGameData.m_byCurrHandleDesk || m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_ERR ||  m_TGameData.m_iUserState[byDeskStation] == STATE_FAIL)
	{
		return;
	}

	// 防止第一把跟操作, 前家押注不能小于等于0
	if (m_TGameData.m_i64CurrZhuBase <= 0)
	{
		ERROR_LOG("下注错误,%d",GetUserIDByDeskStation(byDeskStation));
		return;
	}

	KillTimer(TIME_AUTO_FOLLOW);
	KillTimer(TIME_AUTO_GIVEUP);

	__int64	i64XiaZhuNum = 0;			// 记录单次下注的筹码值(如果翻倍了也只取单次)
	if (255 == m_byBetStation)//开局跟注
	{
		i64XiaZhuNum = m_iThisDiZhu;
	}
	else
	{
		//如果该玩家已经看牌了就要下注双倍
		if (STATE_LOOK == m_TGameData.m_iUserState[byDeskStation] && STATE_LOOK != m_TGameData.m_iFirstUserState[m_byBetStation])
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase * 2;
		}
		else if (STATE_LOOK!=m_TGameData.m_iUserState[byDeskStation] && STATE_LOOK==m_TGameData.m_iFirstUserState[m_byBetStation])//小一倍
		{
			if(m_ChouMaMoney[0] == m_TGameData.m_i64XiaZhuBase)
			{
				i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase;
			}
			else
			{
				i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase/2;
			}
		}
		else
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase;
		}
	}

	//如果当前下注达到了封顶 那么就要限制下  因为最高下注不能超过封顶
	if (!IsPrivateRoom() && ((m_TGameData.m_i64XiaZhuData[byDeskStation] + i64XiaZhuNum) /** GetGameBasePoint()*/)> GetUserMoney())
	{
		SendGameMessage(byDeskStation, "资源不够跟注，系统自动弃牌");
		AutoGiveUp();//下注金币不足，则默认为弃牌状态
		return ;
	}

	m_OperateCount[byDeskStation]++;
	//杀掉等待动作计时器
	KillTimer(TIME_WAIT_ACTION);
	
	m_TGameData.m_iFirstUserState[byDeskStation] = m_TGameData.m_iUserState[byDeskStation];
	m_TGameData.m_i64XiaZhuBase = i64XiaZhuNum;
	//统计个人下注
	m_TGameData.m_i64XiaZhuData[byDeskStation] += i64XiaZhuNum;
	//累加下注总数
	m_TGameData.m_i64ZongXiaZhu += i64XiaZhuNum;

	if (i64XiaZhuNum > m_TGameData.m_i64XiaZhuMaxBase)
	{
		m_TGameData.m_i64XiaZhuMaxBase = i64XiaZhuNum;
	}

	//记录各个筹码的数量
	if (i64XiaZhuNum > m_ChouMaMoney[E_CHOUMA_COUNT-1])
	{
		m_TGameData.m_iChouMaCount[byDeskStation][E_CHOUMA_COUNT-1] += i64XiaZhuNum / m_ChouMaMoney[E_CHOUMA_COUNT-1];
	}
	else
	{
		int iIndex = ValueChangeIndex(i64XiaZhuNum);
		m_TGameData.m_iChouMaCount[byDeskStation][iIndex] += 1;
	}

	//不是第一次下注
	m_TGameData.m_bFirstNote = false;
	m_TGameData.m_iAddNoteTime++;
	m_byBetStation = byDeskStation;
	S_C_UserGenZhuResult TUserGenZhuResult;
	TUserGenZhuResult.byDeskStation = byDeskStation;
	TUserGenZhuResult.iUserState = m_TGameData.m_iUserState[byDeskStation];
	TUserGenZhuResult.i64FollowNum = i64XiaZhuNum;
	TUserGenZhuResult.i64UserZongZhu = m_TGameData.m_i64XiaZhuData[byDeskStation];
	TUserGenZhuResult.i64AllZongZhu = m_TGameData.m_i64ZongXiaZhu ;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TUserGenZhuResult, sizeof(TUserGenZhuResult), MSG_MAIN_LOADER_GAME, S_C_GEN_ZHU_RESULT, 0);
		}
	}
	SendWatchData(&TUserGenZhuResult, sizeof(TUserGenZhuResult), MSG_MAIN_LOADER_GAME, S_C_GEN_ZHU_RESULT, 0);

	//开始判断是否可以结束了
	if (CheckFinish())
	{
		GameFinish(0, GF_NORMAL);
	}
	else
	{
		//启动计时器准备通知玩家操作
		NoticeUserAction();
	}
}

/*------------------------------------------------------------*/
void CServerGameDesk::OnHandleUserAddNote(BYTE byDeskStation, void * pData)
{
	C_S_JiaZhu * pUserJiaZhu = (C_S_JiaZhu *)pData;
	if (NULL == pUserJiaZhu)
	{
		ERROR_LOG("结构体指针为空 玩家：%d ",GetUserIDByDeskStation(byDeskStation));
		return;
	}

	//检测当前操作玩家是否合法
	if (byDeskStation != m_TGameData.m_byCurrHandleDesk || m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_ERR || m_TGameData.m_iUserState[byDeskStation] == STATE_FAIL)
	{
		ERROR_LOG("还没有轮到操作,%d",GetUserIDByDeskStation(byDeskStation));
		return;
	}
	
	//下注不合法
	if (pUserJiaZhu->iAddType < 0 || pUserJiaZhu->iAddType >= E_CHOUMA_COUNT)
	{
		ERROR_LOG("加注错误_1_,%d : 下注为序号 %d ",GetUserIDByDeskStation(byDeskStation),pUserJiaZhu->iAddType);
		SendGameData(byDeskStation,NULL,0,MSG_MAIN_LOADER_GAME,S_C_JIA_ZHU_FAIL,0);
		return;
	}

	if (m_ChouMaMoney[pUserJiaZhu->iAddType] < m_TGameData.m_i64CurrZhuBase)
	{
		ERROR_LOG("加注错误_2_,%d ,下注序号：%d ,筹码：%d ",GetUserIDByDeskStation(byDeskStation),pUserJiaZhu->iAddType,m_ChouMaMoney[pUserJiaZhu->iAddType]);
		SendGameData(byDeskStation,NULL,0,MSG_MAIN_LOADER_GAME,S_C_JIA_ZHU_FAIL,0);
		return;
	}
	
	if (!m_TGameData.m_bCanAdd[byDeskStation][pUserJiaZhu->iAddType])
	{
		ERROR_LOG("加注错误_3_,%d ，pUserJiaZhu->iAddType = %d ,加注筹码值为：%d ",GetUserIDByDeskStation(byDeskStation),pUserJiaZhu->iAddType,m_TGameData.m_bCanAdd[byDeskStation][pUserJiaZhu->iAddType]);
		SendGameData(byDeskStation,NULL,0,MSG_MAIN_LOADER_GAME,S_C_JIA_ZHU_FAIL,0);
		return;
	}

	KillTimer(TIME_AUTO_FOLLOW);
	KillTimer(TIME_AUTO_GIVEUP);

	__int64	i64XiaZhuNum = 0;
	i64XiaZhuNum = m_ChouMaMoney[pUserJiaZhu->iAddType];

	//如果当前下注达到了封顶 那么就要限制下  因为最高下注不能超过封顶
	if (!IsPrivateRoom() && ((m_TGameData.m_i64XiaZhuData[byDeskStation] + i64XiaZhuNum) /** GetGameBasePoint()*/) > GetUserMoney())
	{
		SendGameMessage(byDeskStation, "资源不够加注，系统自动弃牌");
		AutoGiveUp();	//下注金币不足，默认为弃牌状态
		return ;
	}
	//杀掉等待动作计时器
	KillTimer(TIME_WAIT_ACTION);

	m_TGameData.m_iFirstUserState[byDeskStation] = m_TGameData.m_iUserState[byDeskStation];
	m_OperateCount[byDeskStation]++;
	m_byBetStation = byDeskStation;
	m_TGameData.m_i64XiaZhuBase = i64XiaZhuNum;
	//统计个人下注
	m_TGameData.m_i64XiaZhuData[byDeskStation] += i64XiaZhuNum;
	//累加下注总数
	m_TGameData.m_i64ZongXiaZhu += i64XiaZhuNum;
	//当前的蒙注累加
	m_TGameData.m_i64CurrZhuBase = m_ChouMaMoney[pUserJiaZhu->iAddType];
	//不是第一次下注了
	m_TGameData.m_bFirstNote = false;
	m_TGameData.m_iAddNoteTime++;
	if (i64XiaZhuNum > m_TGameData.m_i64XiaZhuMaxBase)
	{
		m_TGameData.m_i64XiaZhuMaxBase = i64XiaZhuNum;
	}

	//记录各个筹码的数量
	if (i64XiaZhuNum > m_ChouMaMoney[E_CHOUMA_COUNT-1])
	{
		m_TGameData.m_iChouMaCount[byDeskStation][E_CHOUMA_COUNT-1] += i64XiaZhuNum / m_ChouMaMoney[E_CHOUMA_COUNT-1];
	}
	else
	{
		int iIndex = ValueChangeIndex(i64XiaZhuNum);
		m_TGameData.m_iChouMaCount[byDeskStation][iIndex] += 1;
	}

	S_C_JiaZhuResult TUserAddNoteResult;
	TUserAddNoteResult.byDeskStation = byDeskStation;
	TUserAddNoteResult.iUserState = m_TGameData.m_iUserState[byDeskStation];
	TUserAddNoteResult.i64AddNum = i64XiaZhuNum ;
	TUserAddNoteResult.i64UserZongZhu = m_TGameData.m_i64XiaZhuData[byDeskStation];
	TUserAddNoteResult.i64AllZongZhu = m_TGameData.m_i64ZongXiaZhu;
	if (m_iFengDing == i64XiaZhuNum)
	{
		TUserAddNoteResult.bDing = true;
	}
	else
	{
		TUserAddNoteResult.bDing = false;
	}
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TUserAddNoteResult, sizeof(TUserAddNoteResult), MSG_MAIN_LOADER_GAME, S_C_JIA_ZHU_RESULT, 0);
		}
	}
	SendWatchData(&TUserAddNoteResult, sizeof(TUserAddNoteResult), MSG_MAIN_LOADER_GAME, S_C_JIA_ZHU_RESULT, 0);

	//开始判断是否可以结束了
	if (CheckFinish())
	{
		GameFinish(0, GF_NORMAL);
	}
	else
	{
		//启动计时器准备通知玩家操作
		NoticeUserAction();
	}
}

/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserGiveUp(BYTE byDeskStation, void * pData)
{
	C_S_UserGiveUp * pUserGiveUp = (C_S_UserGiveUp *)pData;
	if (NULL == pUserGiveUp)
	{
		return;
	}

	if (m_LastComparisonCard)
	{
		ERROR_LOG("！！！最后一组人比牌，不能弃牌！！！！");
		return;
	}
	
	//记录玩家状态
	m_TGameData.m_iUserState[byDeskStation] = STATE_GIVE_UP;
	m_TGameData.m_iAddNoteTime++;

	S_C_UserGiveUpResult	TUserGiveUpResult;
	TUserGiveUpResult.byDeskStation = byDeskStation;
	TUserGiveUpResult.iUserState = m_TGameData.m_iUserState[byDeskStation];
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TUserGiveUpResult, sizeof(TUserGiveUpResult), MSG_MAIN_LOADER_GAME, S_C_GIVE_UP_RESULT, 0);
		}
	}
	SendWatchData(&TUserGiveUpResult, sizeof(TUserGiveUpResult), MSG_MAIN_LOADER_GAME, S_C_GIVE_UP_RESULT, 0);

	//开始判断是否可以结束了
	if (CheckFinish())
	{
		GameFinish(0, GF_NORMAL);
	}
	else
	{
		//启动计时器准备通知玩家操作
		if (byDeskStation == m_TGameData.m_byCurrHandleDesk)
		{
			KillTimer(TIME_AUTO_FOLLOW);
			KillTimer(TIME_AUTO_GIVEUP);
			KillTimer(TIME_WAIT_ACTION);
			NoticeUserAction();
		}
	}
}

/*------------------------------------------------------------*/
//收到玩家比牌消息
void	CServerGameDesk::OnHandleUserOpenCard(BYTE byDeskStation, void * pData)
{
	C_S_UserBiPai * pUserBiPai = (C_S_UserBiPai *)pData;
	if (NULL == pUserBiPai)
	{
		return;
	}

	// 检测当前操作玩家是否合法
	if (byDeskStation != m_TGameData.m_byCurrHandleDesk)
	{
		ERROR_LOG("不是操作玩家 userID=%d",GetUserIDByDeskStation(byDeskStation));
		return;
	}

	if (m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_ERR 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_FAIL)
	{
		ERROR_LOG("状态错误 userID=%d",GetUserIDByDeskStation(byDeskStation));
		return;
	}

	//被比牌的玩家状态
	if (m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_ERR 
		|| m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_FAIL)
	{
		ERROR_LOG("被比牌状态错误 userID=%d",GetUserIDByDeskStation(byDeskStation));
		return;
	}

	// 一圈下注未结束，不能比牌
	if (m_TGameData.m_iAddNoteTime < CountPlayer())
	{
		ERROR_LOG("一圈内不准比牌 userID=%d",GetUserIDByDeskStation(byDeskStation));
		return;
	}

	KillTimer(TIME_AUTO_FOLLOW);
	KillTimer(TIME_AUTO_GIVEUP);

	__int64	i64XiaZhuNum = 0;
	if (STATE_LOOK == m_TGameData.m_iUserState[byDeskStation] && STATE_LOOK != m_TGameData.m_iUserState[m_byBetStation])
	{
		i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase * 2;
	}
	else if (STATE_LOOK != m_TGameData.m_iUserState[byDeskStation] && STATE_LOOK == m_TGameData.m_iUserState[m_byBetStation])
	{
		if (m_ChouMaMoney[0] == m_TGameData.m_i64XiaZhuBase)
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase;
		}
		else
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase / 2;
		}
	}
	else
	{
		i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase;
	}

	//是否比牌加倍
	if (m_CompareCardAdd)
	{
		i64XiaZhuNum = 2*i64XiaZhuNum;
	}

	//记录各个筹码的数量
	if (i64XiaZhuNum > m_ChouMaMoney[E_CHOUMA_COUNT-1])
	{
		m_TGameData.m_iChouMaCount[byDeskStation][E_CHOUMA_COUNT-1] += i64XiaZhuNum / m_ChouMaMoney[E_CHOUMA_COUNT-1];
	}
	else
	{
		int iIndex = ValueChangeIndex(i64XiaZhuNum);
		m_TGameData.m_iChouMaCount[byDeskStation][iIndex] += 1;
	}

	m_OperateCount[byDeskStation]++;

	//杀掉等待动作计时器
	KillTimer(TIME_WAIT_ACTION);

	m_bOpenRecord[byDeskStation][pUserBiPai->byBeDeskStation] = true;
	m_bOpenRecord[pUserBiPai->byBeDeskStation][byDeskStation] = true;

	//统计个人下注
	m_TGameData.m_i64XiaZhuData[byDeskStation] += i64XiaZhuNum;
	//累加下注总数
	m_TGameData.m_i64ZongXiaZhu += i64XiaZhuNum;
	m_TGameData.m_iAddNoteTime++;

	// 发送比牌结果
	S_C_UserBiPaiResult	TUserBiPaiResult;
	TUserBiPaiResult.byDeskStation = byDeskStation;				//主动比牌的玩家
	TUserBiPaiResult.byBeDeskStation = pUserBiPai->byBeDeskStation;	//被动比牌玩家

	int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byDeskStation], MAX_CARD_COUNT, m_TGameData.m_byUserCard[pUserBiPai->byBeDeskStation], MAX_CARD_COUNT);
	if (IsWin == -1)  //被比较着的牌大
	{
		TUserBiPaiResult.byWinDesk = pUserBiPai->byBeDeskStation;
		TUserBiPaiResult.byLostDesk = byDeskStation;
		m_TGameData.m_iUserState[byDeskStation] = STATE_FAIL;
	}
	else   //比牌者比较大
	{
		TUserBiPaiResult.byWinDesk = byDeskStation;
		TUserBiPaiResult.byLostDesk = pUserBiPai->byBeDeskStation;
		m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] = STATE_FAIL;
	}
	//比牌下注情况
	TUserBiPaiResult.i64AddNum = i64XiaZhuNum;
	//玩家的总下注
	TUserBiPaiResult.i64UserZongZhu = m_TGameData.m_i64XiaZhuData[byDeskStation];
	//所有总下注
	TUserBiPaiResult.i64AllZongZhu = m_TGameData.m_i64ZongXiaZhu;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		TUserBiPaiResult.iUserStation[i] = m_TGameData.m_iUserState[i];
	}
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TUserBiPaiResult, sizeof(TUserBiPaiResult), MSG_MAIN_LOADER_GAME, S_C_BI_PAI_RESULT, 0);
		}
	}
	SendWatchData( &TUserBiPaiResult, sizeof(TUserBiPaiResult), MSG_MAIN_LOADER_GAME, S_C_BI_PAI_RESULT, 0);

	if (CheckFinish())
	{	
		///游戏结束前最后一组比牌，不能弃牌
		m_LastComparisonCard = true;
		// 如果比牌之后游戏结束,发送最终赢家的牌
		BYTE winner = TUserBiPaiResult.byWinDesk;
		TUserBiPaiResult.byFinished = 1;
		TUserBiPaiResult.byWinCardType = m_Logic.GetCardShape(m_TGameData.m_byUserCard[winner], MAX_CARD_COUNT),
			memcpy(TUserBiPaiResult.byWinnerCard, m_TGameData.m_byUserCard[winner], sizeof(TUserBiPaiResult.byWinnerCard));
		SetTimer(TIME_GAME_FINISH, 4 * 1000);
	}
	else
	{
		//启动计时器准备通知玩家操作
		SetTimer(TIME_NOTICE_ACTION, 4 * 1000);
	}
}

/*------------------------------------------------------------*/
//获取游戏状态信息
bool	CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if (!IsGoldRoom())
	{
		GetSpecialRule();
	}

	switch (GetGameStation())
	{
	case GS_WAIT_SETGAME:
	case GS_WAIT_ARGEE:			//等待玩家开始状态
	case GS_WAIT_NEXT:			//等待下一盘游戏开始
		{
			S_C_GameStation_WaiteAgree TGameStation;

			//游戏规则
			TGameStation.bBaoZiYouXi = m_bBaoZiYouXi;
			TGameStation.bHuiErPai = m_bHuiErPai;
			TGameStation.b235ShaBaoZi = m_b235ShaBaoZi;
			TGameStation.byThinkTime = m_TGameData.m_byThinkTime;				//游戏思考时间
			TGameStation.byBeginTime = m_TGameData.m_byBeginTime;				//游戏开始时间
			TGameStation.iGuoDi = m_TGameData.m_iDiZhu;							//锅底值	
			TGameStation.iDiZhu = m_TGameData.m_iDiZhu;							//底注      
			TGameStation.i64DingZhu = m_TGameData.m_i64DingZhu;		//个人下注上限  暗注的，但显示为明注的
			TGameStation.iRuleDingZhu = m_iFengDing;
			TGameStation.iAllTurnCount = m_iAllTurnCount;
			TGameStation.playMode = m_playMode;
			SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
			break;
		}
	case GS_SEND_CARD:		//发牌状态
		{
			S_C_GameStation_SendCard	TGameStation;

			//游戏规则
			TGameStation.bBaoZiYouXi = m_bBaoZiYouXi;
			TGameStation.bHuiErPai = m_bHuiErPai;
			TGameStation.b235ShaBaoZi = m_b235ShaBaoZi;
			TGameStation.byThinkTime = m_TGameData.m_byThinkTime;				//游戏思考时间
			TGameStation.byBeginTime = m_TGameData.m_byBeginTime;				//游戏开始时间
			TGameStation.iGuoDi = m_TGameData.m_iDiZhu;			//锅底值	
			TGameStation.iDiZhu = m_iThisDiZhu;			//底注      
			TGameStation.i64DingZhu = m_i64ThisDingZhu;		//个人下注上限  暗注的，但显示为明注的
			TGameStation.byNtStation = m_TGameData.m_byNtPeople;
			TGameStation.i64AllZongZhu = m_TGameData.m_i64ZongXiaZhu;
			TGameStation.byHuiPai = m_Logic.m_byLogicHuiPai;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				TGameStation.iUserState[i] = m_TGameData.m_iUserState[i];
				TGameStation.byUserCardCount[i] = m_TGameData.m_byUserCardCount[i];
				TGameStation.i64XiaZhuData[i] = m_TGameData.m_i64XiaZhuData[i];
			}
			TGameStation.iRuleDingZhu = m_iFengDing;
			TGameStation.iAllTurnCount = m_iAllTurnCount;
			TGameStation.playMode = m_playMode;
			SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
			break;
		}
	case GS_PLAY_GAME:	//游戏中状态
		{
			S_C_GameStation_PlayGame	TGameStation;
			//游戏规则
			TGameStation.bBaoZiYouXi = m_bBaoZiYouXi;
			TGameStation.bHuiErPai = m_bHuiErPai;
			TGameStation.b235ShaBaoZi = m_b235ShaBaoZi;
			TGameStation.byThinkTime = m_TGameData.m_byThinkTime;				//游戏思考时间
			TGameStation.byBeginTime = m_TGameData.m_byBeginTime;				//游戏开始时间
			TGameStation.iGuoDi = m_TGameData.m_iDiZhu;			//锅底值	
			TGameStation.iDiZhu = m_iThisDiZhu;			//底注    
			TGameStation.iRemainderTime = GetGameRemainTime();
			//TGameStation.i64ShangXian	= m_i64ThisShangXian;	//总下注上限		
			TGameStation.i64DingZhu = m_i64ThisDingZhu;		//个人下注上限  暗注的，但显示为明注的
			TGameStation.byNtStation = m_TGameData.m_byNtPeople;
			TGameStation.i64AllZongZhu = m_TGameData.m_i64ZongXiaZhu;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				TGameStation.iUserState[i] = m_TGameData.m_iUserState[i];
				TGameStation.byUserCardCount[i] = m_TGameData.m_byUserCardCount[i];
				TGameStation.i64XiaZhuData[i] = m_TGameData.m_i64XiaZhuData[i];
			}
			//当前操作的玩家
			TGameStation.byCurrHandleDesk = m_TGameData.m_byCurrHandleDesk;
			//用户手上的牌
			memcpy(TGameStation.byUserCard, m_TGameData.m_byUserCard, sizeof(TGameStation.byUserCard));

			if (bDeskStation < PLAY_COUNT)
			{
				TGameStation.iCardType = m_UserCardType[bDeskStation];
				TGameStation.bCanLook[bDeskStation] = m_TGameData.m_bCanLook[bDeskStation];	//可否看牌

				if (0 == m_MenPai)
				{
					//不焖牌
					TGameStation.bCanLook[bDeskStation] = m_TGameData.m_bCanLook[bDeskStation];
				}
				else if (m_iGameTurn <= m_MenPai)
				{

					m_TGameData.m_bCanLook[bDeskStation] = false;
					TGameStation.bCanLook[bDeskStation] = m_TGameData.m_bCanLook[bDeskStation];
				}
				else
				{
					TGameStation.bCanLook[bDeskStation] = m_TGameData.m_bCanLook[bDeskStation];
				}
				//看了牌之后都为false
				if (m_TGameData.m_iUserState[bDeskStation] >= STATE_LOOK)
				{
					TGameStation.bCanLook[bDeskStation] = false;
				}

				memcpy(TGameStation.bCanAdd, m_TGameData.m_bCanAdd[bDeskStation], sizeof(TGameStation.bCanAdd));//可否加注
			}


			TGameStation.bCanFollow = m_TGameData.m_bCanFollow;	//可否跟注
			TGameStation.bCanOpen = m_TGameData.m_bCanOpen;	//可否比牌
			TGameStation.bCanGiveUp = m_TGameData.m_bCanGiveUp;	//可否弃牌
			TGameStation.iTurn = m_iGameTurn;
			TGameStation.byHuiPai = m_Logic.m_byLogicHuiPai;
			TGameStation.iRuleDingZhu = m_iFengDing;

			//筹码类型数量
			memcpy(TGameStation.iChouMaCount, m_TGameData.m_iChouMaCount, sizeof(m_TGameData.m_iChouMaCount));
			TGameStation.iAllTurnCount = m_iAllTurnCount;
			TGameStation.playMode = m_playMode;
			SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
			break;
		}
	default:
		{
			return false;
		}
	}
	//超端验证
	SpuerProof(bDeskStation);
	return true;
}
/*------------------------------------------------------------------*/
//重置游戏状态
bool	CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	KillTimer(TIME_SEND_CARD);
	KillTimer(TIME_GAME_FINISH);

	//游戏状态
	m_bySuperMaxDesk = 255;			//超端设置的最大玩家
	m_bySuperMinDesk = 255;			//超端设定的最小玩家

	m_TGameData.InitSomeData();
	bzero(m_iTotalCard, sizeof(m_iTotalCard));		//总的牌	
	m_iThisDiZhu = 0;			//本局底注      
	m_i64ThisDingZhu = 0;			//本局总下注上限	
	memset(m_OperateCount, 0, sizeof(m_OperateCount));
	memset(m_bOpenRecord,false,sizeof(m_bOpenRecord));
	m_TGameData.m_byCurrHandleDesk = 255;
	m_iGameTurn = 0;
	m_byBetStation = 255;
	memset(m_bWinPlayer,0,sizeof(m_bWinPlayer));
	memset(m_UserCardType,0,sizeof(m_UserCardType));
	memset(m_UserReady,false,sizeof(m_UserReady));
	m_LastComparisonCard = false;
	return TRUE;
}

/*------------------------------------------------------------------*/
//游戏开始
bool CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (CGameDesk::GameBegin(bBeginFlag) == false)
	{
		GameFinish(0,GF_SALE);
		return false;
	}
	KillTimer(TIME_GAME_FINISH);
	KillTimer(TIME_GAME_WAIT_ARGEE);
	KillTimer(TIME_GAME_TUOGUAN);

	//////////////////////////////确定超端玩家和奖值数据//////////////////////////
	memset(m_byRobot,0,sizeof(m_byRobot));
	memset(m_byHuman,0,sizeof(m_byHuman));
	m_iRobotCount = 0;
	m_iHumanCount = 0;
	//保存玩家的状态
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			m_TGameData.m_iUserState[i] = STATE_NORMAL;
			m_TGameData.m_iFirstUserState[i] = STATE_NORMAL;
			GameUserInfo * pUser = m_pDataManage->GetUser(userID);
			if (pUser == NULL)
			{
				continue;
			}
			if (pUser->isVirtual)
			{
				m_byRobot[m_iRobotCount++] = i;
			}
			else
			{
				m_byHuman[m_iHumanCount++] = i;
			}
		}
		else
		{
			m_TGameData.m_iUserState[i] = STATE_ERR;
			m_TGameData.m_iFirstUserState[i] = STATE_ERR;
		}
	}

	//无真人本局游戏结束
	if (m_iHumanCount <= 0)
	{
		GameFinish(0, GF_SALE);
		return false;
	}

	//游戏状态	
	SetGameStation(GS_SEND_CARD);
	if (!IsGoldRoom())
	{
		m_Logic.m_bLogic235ShaBaoZi = m_b235ShaBaoZi;
		m_Logic.m_byLogicHuiPai = 255;
		for (int i = 0; i < PLAY_COUNT; i ++)
		{
			SpuerProof(i);
		}
	}

	for(int i=0;i<E_CHOUMA_COUNT;i++)
	{	
		m_ChouMaMoney[i] = G_iChouMaMoney[i] * m_DiFen;
	}

	//计算顶注（顶注不能超过最小金币数）
	__int64 i64MinMoney = GetMinMoney();
	if (i64MinMoney < m_TGameData.m_i64DingZhu)
	{
		m_i64ThisDingZhu = i64MinMoney;
	}
	else
	{
		m_i64ThisDingZhu = m_TGameData.m_i64DingZhu;
	}
	m_iThisDiZhu = m_TGameData.m_iDiZhu;
	
	//各家目前押注情况,游戏开始之前每位玩家先下锅底
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_TGameData.m_i64XiaZhuData[i] = 0;
		if (m_TGameData.m_iUserState[i] != STATE_ERR)
		{
			m_TGameData.m_i64XiaZhuData[i] += m_iThisDiZhu;
			m_TGameData.m_i64ZongXiaZhu += m_iThisDiZhu;
		}
	}

	//随机生成庄家
	BYTE byUpGradeRand[PLAY_COUNT];
	memset(byUpGradeRand,0,sizeof(byUpGradeRand));
	int byUpGradeRandCount = 0;
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_TGameData.m_iUserState[i] != STATE_ERR)
		{
			byUpGradeRand[byUpGradeRandCount++] = i;
		}
	}
	m_TGameData.m_byNtPeople = byUpGradeRand[CUtil::GetRandNum() % byUpGradeRandCount];

	S_C_GameBegin TGameBegin;
	memset(&TGameBegin, 0, sizeof(TGameBegin));
	for (int i = 0; i < E_CHOUMA_COUNT; i++)
	{
		TGameBegin.iUserBetValue[i] = m_ChouMaMoney[i];
	}
	TGameBegin.byNtStation = m_TGameData.m_byNtPeople;
	TGameBegin.iGuoDi = m_ChouMaMoney[0];
	TGameBegin.iDiZhu = m_iThisDiZhu;
	TGameBegin.i64DingZhu = m_i64ThisDingZhu;
	TGameBegin.i64ZongXiaZhu = m_TGameData.m_i64ZongXiaZhu;
	//下注情况
	memcpy(TGameBegin.i64XiaZhuData, m_TGameData.m_i64XiaZhuData, sizeof(TGameBegin.i64XiaZhuData));

	// 所有玩家底注+1
	int noteBaseIndex = ValueChangeIndex(m_iThisDiZhu);
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID <= 0)
		{
			continue;
		}

		m_TGameData.m_iChouMaCount[i][noteBaseIndex] += 1;
	}

	//各玩家的状态
	memcpy(TGameBegin.iUserState, m_TGameData.m_iUserState, sizeof(TGameBegin.iUserState));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TGameBegin, sizeof(TGameBegin), MSG_MAIN_LOADER_GAME, S_C_GAME_BEGIN, 0);
		}
	}
	SendWatchData(&TGameBegin, sizeof(TGameBegin), MSG_MAIN_LOADER_GAME, S_C_GAME_BEGIN, 0);

	//启动发牌计时器
	SetTimer(TIME_SEND_CARD, 2 * 1000);
	return TRUE;
}
/*------------------------------------------------------------------*/
//发送扑克给用户
BOOL CServerGameDesk::SendCard()
{
	KillTimer(TIME_SEND_CARD);
	
	int iRandCardCount = 52;
	if (m_playMode == 2)
	{
		m_Logic.RandCard_CrazyMode(m_iTotalCard, 24);
		iRandCardCount = 24;
	}
	else
	{
		//分发扑克
		m_Logic.RandCard(m_iTotalCard, 52, m_deskIdx);
	}

	if (m_bHuiErPai)
	{
		BYTE byTotalCard[52];					//总的牌
		memset(byTotalCard, 0, sizeof(byTotalCard));
		memcpy(byTotalCard, m_iTotalCard, sizeof(byTotalCard));
		memset(m_iTotalCard, 0, sizeof(m_iTotalCard));
		int iRan = CUtil::GetRandNum() % iRandCardCount;
		BYTE byLaiPai = byTotalCard[iRan];
		byTotalCard[iRan] = 0;
		int index = 0;
		for (int i = 0; i < 52; i++)
		{
			if (0 == byTotalCard[i])
			{
				continue;
			}
			m_iTotalCard[index++] = byTotalCard[i];
		}
		m_Logic.m_byLogicHuiPai = byLaiPai;
	}
	else
	{
		m_Logic.m_byLogicHuiPai = 255;
	}

	//配牌
	BrandCard();

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_TGameData.m_iUserState[i] == STATE_NORMAL)
		{
			for (int j = 0; j < MAX_CARD_COUNT; j++)
			{
				BYTE byCard = m_iTotalCard[i * 3 + j];
				m_TGameData.m_byUserCard[i][j] = byCard;
			}
			m_TGameData.m_byUserCardCount[i] = MAX_CARD_COUNT;
		}
		else
		{
			m_TGameData.m_byUserCardCount[i] = 0;
			memset(m_TGameData.m_byUserCard[i], 0, sizeof(m_TGameData.m_byUserCard[i]));
		}
	}

	////////////////////////////////////执行奖值设置//////////////////////////////////////
	if (m_pDataManage->LoadPoolData())
	{
		AiWinAutoCtrl();
	}

	////////////////////////////////////执行超端设置//////////////////////////////////////
	OnSpuerResult();

	S_C_SendCard TSendCard;
	TSendCard.byHuiPai = m_Logic.m_byLogicHuiPai;
	//下发所有玩家牌的数据(不考虑外挂)
	memcpy(TSendCard.byCardCount, m_TGameData.m_byUserCardCount, sizeof(TSendCard.byCardCount));
	memcpy(TSendCard.byCard, m_TGameData.m_byUserCard, sizeof(TSendCard.byCard));
	//发牌顺序  从庄家开始发牌
	int iIndex = 0;
	for (int m = 0; m < MAX_CARD_COUNT; m++)
	{
		for (int n = 0; n < PLAY_COUNT; n++)
		{
			if (m_TGameData.m_iUserState[(m_TGameData.m_byNtPeople + n) % PLAY_COUNT] == STATE_ERR)
			{
				TSendCard.bySendCardTurn[iIndex] = 255;
			}
			else
			{
				BYTE bDesk = (m_TGameData.m_byNtPeople + n) % PLAY_COUNT;
				TSendCard.bySendCardTurn[iIndex] = bDesk;
			}
			iIndex++;
		}
	}
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TSendCard, sizeof(TSendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
		}
	}
	SendWatchData(&TSendCard, sizeof(TSendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);

	//启动开始计时器
	SetTimer(TIME_BEGIN_PLAY, CountPlayer() * 1000);
	return TRUE;
}
/*------------------------------------------------------------------*/
//游戏开始-开始下注
BOOL CServerGameDesk::BeginPlayUpGrade()
{
	KillTimer(TIME_BEGIN_PLAY);
	//设置数据
	SetGameStation(GS_PLAY_GAME);
	//当前的下注基数
	m_TGameData.m_i64CurrZhuBase = m_iThisDiZhu;
	//是第一次下注
	m_TGameData.m_bFirstNote = true;
	//庄家的下家第一个开始下注
	m_TGameData.m_byCurrHandleDesk = GetNextNoteStation(m_TGameData.m_byNtPeople);

	for (int i = 0;i<PLAY_COUNT;i++)
	{
		m_TGameData.m_bCanLook[i] = true;
	}

	m_TGameData.m_bCanFollow = true;
	m_TGameData.m_bCanOpen = false;
	m_TGameData.m_bCanGiveUp = true;

	//判断是否可加注逻辑
	memset(m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk], 0, sizeof(m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk]));
	for (int i = 0; i < E_CHOUMA_COUNT; i++)
	{
		__int64 i64XiaZhuNum = m_ChouMaMoney[i];
		if (!IsPrivateRoom())
		{
			if ((m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum) <= GetUserMoney())
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = true;
			}
			else
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
			}
		}
		else
		{
			m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = true;
		}
	}
	for (int i = 0; i < E_CHOUMA_COUNT; i++)
	{
		if (m_ChouMaMoney[i] < m_TGameData.m_i64CurrZhuBase)
		{
			m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
		}
	}

	//焖牌
	if (m_iGameTurn <= m_MenPai)
	{
		memset(m_TGameData.m_bCanLook,false,sizeof(m_TGameData.m_bCanLook));
	}
	if (0 == m_MenPai)
	{
		//不焖牌
		memset(m_TGameData.m_bCanLook,true,sizeof(m_TGameData.m_bCanLook));
	}

	/////不看牌状态下，只能加到第二大的筹码
	if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_NORMAL)
	{
		m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][E_CHOUMA_COUNT-1] = false;
	}

	m_iGameTurn++;
	S_C_BeginPlay TBeginPlay;
	TBeginPlay.byCurrHandleStation = m_TGameData.m_byCurrHandleDesk;
	memcpy(TBeginPlay.bLook,m_TGameData.m_bCanLook,sizeof(TBeginPlay.bLook));
	TBeginPlay.bFollow = m_TGameData.m_bCanFollow;		//是否可跟注
	memcpy(TBeginPlay.bAdd, m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk], sizeof(TBeginPlay.bAdd));
	TBeginPlay.bOpen = m_TGameData.m_bCanOpen;		//是否可比牌
	TBeginPlay.bGiveUp = m_TGameData.m_bCanGiveUp;		//是否可弃牌
	TBeginPlay.iTurn = m_iGameTurn;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TBeginPlay, sizeof(TBeginPlay), MSG_MAIN_LOADER_GAME, S_C_GAME_PLAY, 0);
		}
	}
	SendWatchData(&TBeginPlay, sizeof(TBeginPlay), MSG_MAIN_LOADER_GAME, S_C_GAME_PLAY, 0);

	m_iLastTimerTime = m_TGameData.m_byThinkTime + 1;
	m_dwLastGameStartTime = time(NULL);

	//判断玩家钱够不够下注
	if (!IsPrivateRoom() && !CanXiZhu())
	{
		//提示玩家充值，设置弃牌定时器
		NoticeUserOperate(m_TGameData.m_byCurrHandleDesk, 1);

		SetTimer(TIME_AUTO_GIVEUP, FRIEND_ROOM_WAIT_BEGIN_TIME * 1000);
	}
	else
	{
		if (m_iGameTurn <= m_MenPai) //比焖，自动焖
		{
			SetTimer(TIME_AUTO_FOLLOW, 500);
		}
		else
		{
			//启动开始计时器
			if (IsGoldRoom())
			{
				SetTimer(TIME_WAIT_ACTION, (m_TGameData.m_byThinkTime + 1) * 1000);
			}
			else
			{
				///////有托管模式的房卡场
				if (m_bAuto[m_TGameData.m_byCurrHandleDesk])
				{
					SetTimer(TIME_GAME_TUOGUAN, 1000);
				}
				else if (m_GameAutoPlay)
				{
					SetTimer(TIME_WAIT_ACTION, m_byGameAutoPlayTime * 1000);
				}
			}
		}
	}

	return TRUE;
}
/*------------------------------------------------------------------*/
//通知玩家下注
void	CServerGameDesk::NoticeUserAction()
{
	KillTimer(TIME_NOTICE_ACTION);

	m_bGuoZhuang = false;
	//设定当前操作的玩家
	m_TGameData.m_byCurrHandleDesk = GetNextNoteStation(m_TGameData.m_byCurrHandleDesk);

	//是否可以看牌
	for (int i=0;i<PLAY_COUNT;i++)
	{
		if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_ERR && m_TGameData.m_iUserState[i] == STATE_NORMAL)
		{
			m_TGameData.m_bCanLook[i] = true;
		}
		else
		{
			m_TGameData.m_bCanLook[i] = false;
		}

		//焖牌
		if (m_iGameTurn <= m_MenPai)
		{
			m_TGameData.m_bCanLook[i] = false;
		}
		if (0 == m_MenPai && m_TGameData.m_iUserState[i] == STATE_NORMAL)//正常状态才会看牌
		{
			//不焖牌
			m_TGameData.m_bCanLook[i] = true;
		}
	}

	//是否可以加注
	memset(m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk], 0, sizeof(m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk]));
	for (int i = 0; i < E_CHOUMA_COUNT; i++)
	{
		__int64	i64XiaZhuNum = m_ChouMaMoney[i];
		if (!IsPrivateRoom())
		{
			if ((m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum) <= GetUserMoney())
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = true;
			}
			else
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
			}
		}
		else
		{
			m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = true;
		}

		if(m_byBetStation != 255 && m_TGameData.m_iUserState[m_byBetStation] != STATE_LOOK
			&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_LOOK)//前一家不看牌
		{
			long long i64XiaZhu_ = m_TGameData.m_i64XiaZhuBase * 2;
			if (m_ChouMaMoney[i] <= i64XiaZhu_)
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
				continue;
			}
		}
		if(m_byBetStation != 255 && m_TGameData.m_iUserState[m_byBetStation] == STATE_LOOK
			&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_LOOK)//前一家看了牌
		{
			long long i64XiaZhu_ = 0;
			if (m_TGameData.m_i64XiaZhuBase == m_ChouMaMoney[0])
			{
				i64XiaZhu_ = m_ChouMaMoney[0];
			}
			else
			{
				i64XiaZhu_ = m_TGameData.m_i64XiaZhuBase / 2;
			}
			if (m_ChouMaMoney[i] <= i64XiaZhu_)
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
				continue;
			}
		}
		else
		{
			if (m_ChouMaMoney[i] <= m_TGameData.m_i64CurrZhuBase)
			{
				m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
				continue;
			}
		}
	}
	for (int i = 0; i < E_CHOUMA_COUNT; i++)
	{
		if (m_ChouMaMoney[i] < m_TGameData.m_i64XiaZhuMaxBase)
		{
			m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][i] = false;
		}
	}
	if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_NORMAL)
	{
		m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk][E_CHOUMA_COUNT-1] = false;//不看牌的状态，最大加注只能加到为第二大的筹码
	}

	//是否可以比牌								
	m_TGameData.m_bCanOpen = (m_TGameData.m_iAddNoteTime >= CountPlayer()) ? true : false;

	//是否可以弃牌	
	if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_ERR 
		&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_GIVE_UP
		&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_FAIL
		)
	{
		m_TGameData.m_bCanGiveUp = true;
	}
	else
	{
		m_TGameData.m_bCanGiveUp = false;
	}

	//通知玩家下注
	S_C_NoticeAction TNoticeAction;
	TNoticeAction.byCurrHandleStation = m_TGameData.m_byCurrHandleDesk;
	memcpy(TNoticeAction.bCanLook,m_TGameData.m_bCanLook,sizeof(TNoticeAction.bCanLook));
	TNoticeAction.bCanFollow = m_TGameData.m_bCanFollow;
	TNoticeAction.bCanOpen = m_TGameData.m_bCanOpen;
	TNoticeAction.bCanGiveUp = m_TGameData.m_bCanGiveUp;
	TNoticeAction.iTurn = m_iGameTurn;
	memcpy(TNoticeAction.bCanAdd, m_TGameData.m_bCanAdd[m_TGameData.m_byCurrHandleDesk], sizeof(TNoticeAction.bCanAdd));

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			SendGameData(i, &TNoticeAction, sizeof(TNoticeAction), MSG_MAIN_LOADER_GAME, S_C_CALL_ACTION, 0);
		}
	}
	SendWatchData(&TNoticeAction, sizeof(TNoticeAction), MSG_MAIN_LOADER_GAME, S_C_CALL_ACTION, 0);

	int iGameTurn = m_iGameTurn;
	if (m_TGameData.m_byNtPeople == m_TGameData.m_byCurrHandleDesk || m_bGuoZhuang)
	{
		m_iGameTurn++;
	}

	if (!IsPrivateRoom() && !CanXiZhu()) //资源不够下注
	{
		//提示玩家充值，设置弃牌定时器
		NoticeUserOperate(m_TGameData.m_byCurrHandleDesk, 1);

		SetTimer(TIME_AUTO_GIVEUP, FRIEND_ROOM_WAIT_BEGIN_TIME * 1000);

		/*int iCount = 0;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_FAIL != m_TGameData.m_iUserState[i])
			{
				iCount++;
			}
		}
		if (iCount == 2 && m_TGameData.m_bCanOpen)
		{
			AutoBiPai();
		}
		else
		{
			AutoGiveUp();
		}
		return;*/
	}
	else
	{
		if (iGameTurn <= m_MenPai && m_OperateCount[m_TGameData.m_byCurrHandleDesk] < m_MenPai) //比焖，自动焖
		{
			SetTimer(TIME_AUTO_FOLLOW, 300);
		}
		else
		{
			if (!IsPrivateRoom() && OnlyEnoughOpenCard()) //资源只够比牌
			{
				NoticeUserOperate(m_TGameData.m_byCurrHandleDesk, 2);
			}
			
			//启动开始计时器
			if (IsGoldRoom())
			{
				SetTimer(TIME_WAIT_ACTION, (m_TGameData.m_byThinkTime + 1) * 1000);
			}
			else
			{
				///////有托管模式的房卡场
				if (m_bAuto[m_TGameData.m_byCurrHandleDesk])
				{
					SetTimer(TIME_GAME_TUOGUAN, 1000);
				}
				else if (m_GameAutoPlay)
				{
					SetTimer(TIME_WAIT_ACTION, m_byGameAutoPlayTime * 1000);
				}
			}
		}
	}
}
/*------------------------------------------------------------------*/
//通知玩家操作
void	CServerGameDesk::NoticeUserOperate(BYTE byDeskStation, BYTE type)
{
	S_C_Notify_Oper msg;
	msg.type = type;
	msg.time = FRIEND_ROOM_WAIT_BEGIN_TIME;
	SendGameData(byDeskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, S_C_NOTIFY_OPER, 0);
}
/*------------------------------------------------------------------*/
//统计玩游戏玩家
BYTE	CServerGameDesk::CountPlayer()
{
	BYTE count = 0;

	for (int i = 0; i < m_byMaxPeople; i++)
	{
		if (m_TGameData.m_iUserState[i] != STATE_ERR)
		{
			count++;
		}
	}

	return count;
}

/*---------------------------------------------------------------------*/
//游戏结束
bool	CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
		{
			int roomType = GetRoomType();
			//设置游戏状态
			SetGameStation(GS_WAIT_ARGEE);
			//获取最终胜利的玩家
			BYTE iWiner = GetFinalWiner();
			if (iWiner == 255)
			{
				ERROR_LOG("获取最终赢家失败");
				return false;
			}
			//下一盘赢家是庄家
			m_TGameData.m_byNtPeople = iWiner;
			//游戏结束
			S_C_GameEnd TGameEnd;
			TGameEnd.byWiner = iWiner;
			memcpy(TGameEnd.iOperateCount, m_OperateCount, sizeof(TGameEnd.iOperateCount));
			//传给客户端当前牌型
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				long userID = GetUserIDByDeskStation(i);
				if (userID <= 0 || m_TGameData.m_iUserState[i] == STATE_ERR)
				{
					continue;
				}
				TGameEnd.iUserSatus[i] = m_TGameData.m_iUserState[i];

				//牌型处理
				TGameEnd.byCardShape[i] = m_Logic.GetCardShape(m_TGameData.m_byUserCard[i], MAX_CARD_COUNT);
			}

			// 获取游戏场倍数
			int basePoint = GetGameBasePoint();

			///////// 有喜加分 //////////
			int iHaveXi = 0;
			if (m_bBaoZiYouXi)
			{
				////三条AAA ，加20分，其他豹子 加 10 分，同花顺 加 5 分
				if (SH_THREE == TGameEnd.byCardShape[iWiner])
				{
					if (14 == m_Logic.GetCardNum(m_TGameData.m_byUserCard[iWiner][0]))
					{
						iHaveXi = 20 * basePoint;
					}
					else
					{
						iHaveXi = 10 * basePoint;
					}
				}
				else if (SH_SAME_HUA_CONTINUE == TGameEnd.byCardShape[iWiner])
				{
					iHaveXi = 5 * basePoint;
				}
			}
			long long llWinMoney = 0;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				long userID = GetUserIDByDeskStation(i);
				if (userID <= 0 || m_TGameData.m_iUserState[i] == STATE_ERR)
				{
					continue;
				}
				if (i == iWiner)
				{
					//赢的局数
					m_tZongResult.iWinCount[i]++;	
					continue;
				}

				//输的局数
				m_tZongResult.iLoseCount[i]++;

				//输掉所有押的注
				long long iTempShuMoney = m_TGameData.m_i64XiaZhuData[i] + iHaveXi;
				if (roomType != ROOM_TYPE_PRIVATE)
				{
					long long llResNums = 0;
					if (GetRoomResNums(i, llResNums) && llResNums < iTempShuMoney)
					{
						iTempShuMoney = llResNums;
					}
				}

				TGameEnd.iUserScore[i] = -iTempShuMoney;
				llWinMoney += iTempShuMoney;
				
				////赢的喜钱
				m_tZongResult.iXiQian[iWiner] += iHaveXi;
				///输家扣的喜钱
				m_tZongResult.iXiQian[i] -= iHaveXi;

			}
			TGameEnd.iUserScore[iWiner] = llWinMoney;

			// 计算抽水
			long long llRateMoney[PLAY_COUNT] = { 0 };
			memset(llRateMoney, 0, sizeof(llRateMoney));
			if (IsGoldRoom())
			{
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (TGameEnd.iUserScore[i] > 0)
					{
						llRateMoney[i] = TGameEnd.iUserScore[i] * 0 / 100;
						TGameEnd.iUserScore[i] -= llRateMoney[i];
					}
				}
			}

			//写入数据库						
			bool temp_cut[PLAY_COUNT];
			memset(temp_cut, 0, sizeof(temp_cut));
			if (roomType != ROOM_TYPE_GOLD)
			{
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					long userID = GetUserIDByDeskStation(i);
					if (userID <= 0 || m_TGameData.m_iUserState[i] == STATE_ERR)
					{
						continue;
					}
					//大结算，玩家的各类牌型累计
					m_tZongResult.iCardTypeCount[i][TGameEnd.byCardShape[i]]++;
					//单局的最大牌型
					if (TGameEnd.byCardShape[i]>m_tZongResult.iMaxCardType[i])
					{
						if (1 == m_TeSuPaiXing && SH_CONTINUE == TGameEnd.byCardShape[i]  && SH_SAME_HUA == m_tZongResult.iMaxCardType[i])////特殊牌型 顺子>同花
						{
							m_tZongResult.iMaxCardType[i] = SH_CONTINUE;
						}
						else
						{
							m_tZongResult.iMaxCardType[i] = TGameEnd.byCardShape[i];
						}
					}
					//单局最大分
					if (TGameEnd.iUserScore[i]>m_tZongResult.iOneMaxScore[i])
					{
						m_tZongResult.iOneMaxScore[i] = TGameEnd.iUserScore[i];
					}
					
					m_tZongResult.iGameScore[i] += TGameEnd.iUserScore[i];

					if (TGameEnd.iUserScore[i] > 0)
					{
						m_tZongResult.byWinCount[i]++;
					}
					else
					{
						m_tZongResult.byLostCount[i]++;
					}
				}

				if (roomType == ROOM_TYPE_PRIVATE)
				{
					ChangeUserPointPrivate(TGameEnd.iUserScore, temp_cut, "", true);
				}
				else if (roomType == ROOM_TYPE_FRIEND)
				{
					ChangeUserPointGoldRoom(TGameEnd.iUserScore, "", true);
				}
				else if (roomType == ROOM_TYPE_FG_VIP)
				{
					ChangeUserFireCoin(TGameEnd.iUserScore, "", true);
				}
			}
			else
			{
				ChangeUserPoint(TGameEnd.iUserScore, temp_cut, llRateMoney);
			}

			memcpy(TGameEnd.iChangeMoney, TGameEnd.iUserScore, sizeof(TGameEnd.iChangeMoney));
			memcpy(TGameEnd.byCard, m_TGameData.m_byUserCard, sizeof(TGameEnd.byCard));
			//发送数据
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				long userID = GetUserIDByDeskStation(i);
				if (userID > 0)
				{
					SendGameData(i, &TGameEnd, sizeof(TGameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
				}
			}
			SendWatchData(&TGameEnd, sizeof(TGameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);

			ReSetGameState(bCloseFlag);
			if ( (m_iRunGameCount < m_iVipGameCount) && m_GameAutoPlay)
			{
				SetTimer(TIME_GAME_AUTO_BEGIN,5*1000);
				SetTimer(TIME_GAME_WAIT_ARGEE,m_byGameAutoPlayTime *1000);
			}
			CGameDesk::GameFinish(bDeskStation, bCloseFlag);
			return true;
		}
	case GF_SALE:			//游戏安全结束	//只有所有人都断线了 才会安全结束
	case GFF_FORCE_FINISH:		//用户断线离开
		{
			SetGameStation(GS_WAIT_ARGEE);
			bCloseFlag = GF_SALE;
			ReSetGameState(bCloseFlag);
			CGameDesk::GameFinish(bDeskStation, bCloseFlag);
			return true;
		}
	}
	//重置数据
	ReSetGameState(bCloseFlag);
	CGameDesk::GameFinish(bDeskStation, bCloseFlag);

	return true;
}
/*---------------------------------------------------------------------*/
//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if (GetGameStation() >= GS_SEND_CARD && GetGameStation() < GS_WAIT_NEXT)
	{
		return true;
	}
	return false;
}

/*---------------------------------------------------------------------*/
//用户离开游戏桌
bool CServerGameDesk::UserLeftDesk(GameUserInfo* pUser)
{
	if (pUser->deskStation >= 0 && pUser->deskStation < PLAY_COUNT)
	{
		m_bSuperFlag[pUser->deskStation]=false;
	}
	if (m_TGameData.m_byNtPeople == pUser->deskStation)
	{
		m_TGameData.m_byNtPeople = 255;
	}
	return CGameDesk::UserLeftDesk(pUser);
}

//用户断线
bool	CServerGameDesk::UserNetCut(GameUserInfo *pUser)
{
	CGameDesk::UserNetCut(pUser);
	//记录断线玩家（在重连前部给该玩家发送消息）
	int iNetNums=0;
	int iAllNums = 0;
	if (IsGoldRoom() && IsPlayGame(0))
	{
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if (m_TGameData.m_iUserState[i] != STATE_ERR)
			{
				iAllNums ++;
			}
			long userID = GetUserIDByDeskStation(i);
			if (userID <= 0)
			{
				continue;
			}
			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			if(pUser && !pUser->IsOnline)
			{
				iNetNums++;
			}
		}
		if(iNetNums>=iAllNums) //金币场掉线超过4个人
		{
			GameFinish(pUser->deskStation,GF_SALE);
		}
	}
	return true;
}

BYTE CServerGameDesk::GetNextNoteStation(BYTE bCurDeskStation)
{
	BYTE bNextStation = bCurDeskStation;
	int kkk = PLAY_COUNT - 1;   //逆时针寻找下一个操作对象(过滤掉弃牌，状态错误，等待开牌的状态)
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		bNextStation = (bNextStation + kkk) % PLAY_COUNT;
		long userID = GetUserIDByDeskStation(bNextStation);
		if (userID <= 0)
		{
			continue;
		}
		if (bNextStation == m_TGameData.m_byNtPeople)
		{
			m_bGuoZhuang = true;
		}
		if (m_TGameData.m_iUserState[bNextStation] != STATE_GIVE_UP && m_TGameData.m_iUserState[bNextStation] != STATE_ERR 
			&& m_TGameData.m_iUserState[bNextStation] != STATE_FAIL
			)
		{
			break;
		}
		if (bNextStation == bCurDeskStation)
		{
			break;
		}
	}
	return bNextStation;
}

/*---------------------------------------------------------------------*/
/*
* @return 返回最小金币玩家的金币,如果配置的输赢上限小于最少金币玩家的金币，则是返回输赢上限
*/
__int64		CServerGameDesk::GetMinMoney()
{
	__int64 Min = -1;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID <= 0 || m_TGameData.m_iUserState[i] == STATE_ERR)
		{
			continue;
		}

		long long llResNums = 0;
		GetRoomResNums(i, llResNums);

		if (Min == -1)
		{
			Min = llResNums;
		}
		else if (llResNums < Min)
		{
			Min = llResNums;
		}
	}
	return Min;
}

//验证是否超端
void	CServerGameDesk::SpuerProof(BYTE byDeskStation)
{
	if (byDeskStation >= PLAY_COUNT)
	{
		return;
	}
	//是超端用户 那么就发送超端消息过去
	GameUserInfo user;
	if (GetUserData(byDeskStation, user) && (user.userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER)
	{
		m_bSuperFlag[byDeskStation] = true;
		S_C_SuperUserProof TSuperUser;
		TSuperUser.bIsSuper = true;
		TSuperUser.byDeskStation = byDeskStation;
		SendGameData(byDeskStation, &TSuperUser, sizeof(TSuperUser), MSG_MAIN_LOADER_GAME, S_C_SUPER_PROOF, 0);
	}
	else
	{
		m_bSuperFlag[byDeskStation] = false;
	}
}

//执行超端设置
void    CServerGameDesk::OnSpuerResult()
{
	//只有游戏状态才能执行设置结果
	if (!IsPlayGame(0))
	{
		return;
	}

	//开始设置最大
	if (m_bySuperMaxDesk >= 0 && m_bySuperMaxDesk < PLAY_COUNT)
	{
		BYTE byTmpMax = 255;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				if (255 == byTmpMax)
				{
					byTmpMax = i;
					break;
				}
			}
		}
		if (byTmpMax != 255)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (i == byTmpMax)
				{
					continue;
				}
				if (STATE_ERR != m_TGameData.m_iUserState[i])
				{
					int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMax], MAX_CARD_COUNT, m_TGameData.m_byUserCard[i], MAX_CARD_COUNT);
					//被比较着的牌大
					if (-1 == IsWin)
					{
						byTmpMax = i;
					}
				}
			}
			//最大的玩家!=设定的玩家 准备换牌
			if (byTmpMax != m_bySuperMaxDesk)
			{
				Change2UserCard(byTmpMax, m_bySuperMaxDesk);
			}
		}
	}

	//开始设置最小
	if (m_bySuperMinDesk >= 0 && m_bySuperMinDesk < PLAY_COUNT)
	{
		BYTE byTmpMin = 255;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				if (255 == byTmpMin)
				{
					byTmpMin = i;
					break;
				}
			}
		}
		if (byTmpMin != 255)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (i == byTmpMin)
				{
					continue;
				}
				if (STATE_ERR != m_TGameData.m_iUserState[i])
				{
					int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMin], MAX_CARD_COUNT, m_TGameData.m_byUserCard[i], MAX_CARD_COUNT);
					//byTmpMin的牌大
					if (1 == IsWin)
					{
						byTmpMin = i;
					}
				}
			}
			//最小的玩家!=设定的玩家 准备换牌
			if (byTmpMin != m_bySuperMinDesk)
			{
				Change2UserCard(byTmpMin, m_bySuperMinDesk);
			}
		}
	}
}

//交换两个人的牌
void	CServerGameDesk::Change2UserCard(BYTE byFirstDesk, BYTE bySecondDesk)
{
	if (byFirstDesk >= 0 && byFirstDesk < PLAY_COUNT && bySecondDesk >= 0 && bySecondDesk < PLAY_COUNT)
	{
		if (m_TGameData.m_iUserState[byFirstDesk] == STATE_ERR || m_TGameData.m_iUserState[bySecondDesk] == STATE_ERR)
		{
			return;
		}
		BYTE byTmpCard[MAX_CARD_COUNT];
		memcpy(byTmpCard, m_TGameData.m_byUserCard[byFirstDesk], sizeof(byTmpCard));
		memcpy(m_TGameData.m_byUserCard[byFirstDesk], m_TGameData.m_byUserCard[bySecondDesk], sizeof(m_TGameData.m_byUserCard[byFirstDesk]));
		memcpy(m_TGameData.m_byUserCard[bySecondDesk], byTmpCard, sizeof(m_TGameData.m_byUserCard[bySecondDesk]));
	}
}

///机器人输赢自动控制,
void	CServerGameDesk::AiWinAutoCtrl()
{
	static unsigned long lAddTime = 0;
	if (lAddTime >= 65535)
		lAddTime = 0;
	
	//首先去判断 这一桌是否既有机器人又有真人 
	BYTE byRobotDesk = 255;		//随便找一个机器人的位置
	BYTE byUserDesk = 255;		//随便找一个真实玩家的位置

	/////////////////////////////////奖值/////////////////////////////////////////
	//根据m_pooInfo值确定概率
	//int	m_iAIWantWinMoneyPercent;   //每张桌子，机器人赢钱概率，0~1000以内的值
	if (m_iRobotCount <= 0 || m_iHumanCount <= 0)
	{
		return;
	}

	int roomType = GetRoomType();
	if (roomType != ROOM_TYPE_GOLD)
	{
		return;
	}

	m_iAIWantWinMoneyPercent = m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent;

	if (m_iAIWantWinMoneyPercent >= 0)
	{
		m_bSystemWin = true;
	}
	else
	{
		m_bSystemWin = false;
	}

	if (CUtil::GetRandNum() % 1000 >= abs(m_iAIWantWinMoneyPercent)) //本局机器人赢
	{
		return;
	}

	//随机生成一个真人和机器人
	byRobotDesk = m_byRobot[CUtil::GetRandNum()%m_iRobotCount];
	byUserDesk = m_byHuman[CUtil::GetRandNum()%m_iHumanCount];

	BYTE byTmpMax = 255,byTmpMax2 = 255;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (STATE_ERR != m_TGameData.m_iUserState[i])
		{
			if (255 == byTmpMax)
			{
				byTmpMax = i;
				break;
			}
		}
	}

	//找最大
	if (byTmpMax != 255)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (i == byTmpMax)
			{
				continue;
			}
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMax], MAX_CARD_COUNT, m_TGameData.m_byUserCard[i], MAX_CARD_COUNT);
				//被比较着的牌大
				if (-1 == IsWin)
				{
					byTmpMax = i;
				}
			}
		}
	}

	//找第二大
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (STATE_ERR != m_TGameData.m_iUserState[i])
		{
			if (255 == byTmpMax2 && i != byTmpMax)
			{
				byTmpMax2 = i;
				break;
			}
		}
	}
	if (byTmpMax2 != 255)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (i == byTmpMax || i == byTmpMax2)
			{
				continue;
			}
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMax2], MAX_CARD_COUNT, m_TGameData.m_byUserCard[i], MAX_CARD_COUNT);
				//被比较着的牌大
				if (-1 == IsWin)
				{
					byTmpMax2 = i;
				}
			}
		}
	}

	if (m_bSystemWin) //系统赢
	{
		//最大的玩家!=设定的玩家 准备换牌
		if (byTmpMax != byRobotDesk)
		{
			Change2UserCard(byTmpMax, byRobotDesk);
		}
		if (byTmpMax2 != 255 && CUtil::GetRandNum()%100 >= 50 && (byTmpMax2 != byRobotDesk || byTmpMax != byUserDesk)
			&& m_Logic.GetCardShape(m_TGameData.m_byUserCard[byTmpMax2],MAX_CARD_COUNT) >= SH_DOUBLE 
			&& byTmpMax2 != byUserDesk)
		{
			Change2UserCard(byTmpMax2, byUserDesk);
		}
	}
	else //真人赢
	{
		if (byTmpMax != byUserDesk)
		{
			Change2UserCard(byTmpMax, byUserDesk);
		}
	}
}

// 获得当前正在运行的游戏定时器的剩余时间
int CServerGameDesk::GetGameRemainTime()
{
	long lTime = m_iLastTimerTime - (time(NULL) - m_dwLastGameStartTime);
	if (lTime < 0)
	{
		return false;
	}
	return ((int)lTime);
}

//能否下注
bool CServerGameDesk::CanXiZhu()
{
	__int64	i64XiaZhuNum = 0;// 记录单次下注的筹码值(如果翻倍了也只取单次)
	if (255 == m_byBetStation)//开局跟注
	{
		i64XiaZhuNum = m_iThisDiZhu;
	}
	else
	{
		//如果该玩家已经看牌了就要下注双倍
		if (STATE_LOOK == m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] && STATE_LOOK != m_TGameData.m_iFirstUserState[m_byBetStation])
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase* 2;
		}
		else if (STATE_LOOK!=m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] && STATE_LOOK==m_TGameData.m_iFirstUserState[m_byBetStation])//小一倍
		{
			if(m_TGameData.m_i64XiaZhuBase == m_ChouMaMoney[0])
			{
				i64XiaZhuNum = m_ChouMaMoney[0];
			}
			else
			{
				i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase/2;
			}
		}
		else
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase;
		}
	}

	long long llResNums = 0;
	if (GetRoomResNums(m_TGameData.m_byCurrHandleDesk, llResNums))
	{
		__int64 iUserMoney = llResNums;//获取玩家当前的金币
		__int64 iXiaZhuMoney = (m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum);//获取玩家当前至少下注多少
		if (iUserMoney < iXiaZhuMoney)
		{
			return false;
		}
	}
	return true;
}

void CServerGameDesk::OnDeskSuccessfulDissmiss(bool isDismissMidway)
{
	if (m_iRunGameCount > 0)
	{
		CGameDesk::SetDeskPercentageScore(m_tZongResult.iGameScore);
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			long userID = GetUserIDByDeskStation(i);
			if (userID <= 0)
			{
				continue;
			}
			SendGameData(i, &m_tZongResult, sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, SUB_S_GAME_END_ALL, 0);
		}
		SendWatchData(&m_tZongResult, sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, SUB_S_GAME_END_ALL, 0);
	}
	
	CGameDesk::OnDeskSuccessfulDissmiss(isDismissMidway);
}

//初始化游戏数据，房卡场非正常结算的时候调用
void CServerGameDesk::InitDeskGameData()
{
	KillTimer(TIME_SEND_CARD);
	KillTimer(TIME_BEGIN_PLAY);
	KillTimer(TIME_WAIT_ACTION);
	KillTimer(TIME_NOTICE_ACTION);
	KillTimer(TIME_GAME_FINISH);
	KillTimer(TIME_AUTO_FOLLOW);
	KillTimer(TIME_AUTO_GIVEUP);
	//设置游戏状态
	SetGameStation(GS_WAIT_ARGEE);

	//初始化房间规则
	m_bBaoZiYouXi = false;
	m_bHuiErPai = false;
	m_b235ShaBaoZi = false;
	m_CompareCardAdd = 0;
	m_MenPai = 0;
	m_TeSuPaiXing = 1;

	m_iFengDing = 0;
	m_tZongResult.Init();
	ReSetGameState(0);
	memset(m_bSuperFlag,false,sizeof(m_bSuperFlag));
	m_bAlreadSet = false;
	memset(m_bAuto,false,sizeof(m_bAuto));
}

//房间规则解析
void CServerGameDesk::GetSpecialRule()
{
	if(m_bAlreadSet)
	{
		return;
	}
	m_bAlreadSet = true;
	int roomType = GetRoomType();

	//ERROR_LOG("游戏规则 m_szGameRules ：%s ",m_szGameRules);
	Json::Reader reader;
	Json::Value root;
	Json::Value arrayWanFa; //玩法
	if (reader.parse(m_szGameRules, arrayWanFa))
	{
		m_b235ShaBaoZi = arrayWanFa["Play_1"].asInt();		//235杀豹子:
		m_bBaoZiYouXi = arrayWanFa["Play_2"].asInt();		//豹子有喜
		m_bHuiErPai = arrayWanFa["Play_3"].asInt();			//随机癞子
		m_TGameData.m_iDiZhu = arrayWanFa["Score"].asInt(); //底分;

		m_iAllTurnCount = arrayWanFa["MoShi"].asInt();///轮数
		m_CompareCardAdd = arrayWanFa["Jiabei_1"].asInt(); //比牌加倍
		m_GameAutoPlay = arrayWanFa["Jiabei_2"].asInt(); //房卡场是否托管
		m_MenPai = arrayWanFa["Menpai"].asInt();////////焖牌
		m_TeSuPaiXing = arrayWanFa["Teshu"].asInt();//特殊牌型
	}

	if(roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		m_TGameData.m_iDiZhu = GetBasePoint();
	}

	m_DiFen = m_TGameData.m_iDiZhu <= 0 ? 1 : m_TGameData.m_iDiZhu;
	m_Logic.m_byTeSuPaiXing = m_TeSuPaiXing;
}

int CServerGameDesk::ValueChangeIndex(int iValue)
{
	iValue = iValue/m_DiFen;
	switch (iValue)
	{
	case 1:
		return 0;
	case 2:
		return 1;
	case 4:
		return 2;
	case 8:
		return 3;
	default:
		return 0;
	}
}

int CServerGameDesk::GetGameBasePoint()
{
	if (!IsGoldRoom())
	{
		return m_TGameData.m_iDiZhu;
	}
	else
	{
		if (!m_pDataManage)
		{
			return 1;
		}
		RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_pDataManage->GetRoomID());
		if (pRoomBaseInfo)
		{
			return pRoomBaseInfo->basePoint;
		}
		return 1;
	}
}

//获取玩家当前的金币或者火币
__int64 CServerGameDesk::GetUserMoney()
{
	long long llResNums = 0;

	GetRoomResNums(m_TGameData.m_byCurrHandleDesk,llResNums);

	return llResNums;
}

//判断玩家身上资源是否只够比牌
bool CServerGameDesk::OnlyEnoughOpenCard()
{
	__int64	i64XiaZhuNum = 0;// 记录单次下注的筹码值(如果翻倍了也只取单次)
	if (255 == m_byBetStation)//开局跟注
	{
		return false;
	}
	else
	{
		//如果该玩家已经看牌了就要下注双倍
		if (STATE_LOOK == m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] && STATE_LOOK != m_TGameData.m_iFirstUserState[m_byBetStation])
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase * 2;
		}
		else if (STATE_LOOK != m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] && STATE_LOOK == m_TGameData.m_iFirstUserState[m_byBetStation])//小一倍
		{
			if (m_TGameData.m_i64XiaZhuBase == m_ChouMaMoney[0])
			{
				i64XiaZhuNum = m_ChouMaMoney[0];
			}
			else
			{
				i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase / 2;
			}
		}
		else
		{
			i64XiaZhuNum = m_TGameData.m_i64XiaZhuBase;
		}
	}

	//是否比牌加倍
	if (m_CompareCardAdd)
	{
		i64XiaZhuNum = 2 * i64XiaZhuNum;
	}

	__int64 iXiaZhuMoney = (m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum);

	long long llResNums = GetUserMoney();
	if (iXiaZhuMoney <= llResNums && llResNums < (iXiaZhuMoney + i64XiaZhuNum * 3 / 2))
	{
		return true;
	}

	return false;
}

// 玩家请求托管
bool CServerGameDesk::OnHandleUserRequestAuto(BYTE bDeskStation)
{
	if (m_GameAutoPlay)
	{
		m_bAuto[bDeskStation] = true;
		// 立即托管
		BYTE byStatus = GetGameStation();
		if(byStatus == GS_WAIT_NEXT || byStatus == GS_WAIT_ARGEE)
		{
			HandleNotifyMessage(bDeskStation,MSG_ASS_LOADER_GAME_AGREE,NULL,0,false);	
		}
		else if(byStatus == GS_PLAY_GAME)
		{
			if (!IsGoldRoom())
			{	
				AutoGiveUp(bDeskStation);
			}
		}
	}
	return CGameDesk::OnHandleUserRequestAuto(bDeskStation);
}

// 玩家取消托管
bool CServerGameDesk::OnHandleUserRequestCancelAuto(BYTE bDeskStation)
{
	if (m_GameAutoPlay)
	{
		m_bAuto[bDeskStation] = false;
	}
	return CGameDesk::OnHandleUserRequestCancelAuto(bDeskStation);
}

// 动态加载配置文件数据
void CServerGameDesk::LoadDynamicConfig()
{
	//加载配置文件
	LoadIni();
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);
}

//配牌器
bool CServerGameDesk::BrandCard()
{
	//读取超端数据
	char cFileName[64];
	memset(cFileName,0,sizeof(cFileName));
	sprintf(cFileName,"C:/web/json/%d.json",NAME_ID);
	ifstream fin;
	fin.open(cFileName);
	if(!fin.is_open())
	{
		return false;
	}

	Json::Reader reader;
	Json::Value  root;
	if (!reader.parse(fin, root, false))
	{
		return false;
	}
	Json::Value statusKeyValue = root["status"];
	int iStatus = statusKeyValue.asInt();
	if(0 == iStatus)
	{
		return false;
	}

	string lwlog;

	//获取手牌数（手牌有几张）
	int iCardCount[PLAY_COUNT];
	memset(iCardCount,0,sizeof(iCardCount));
	int arraySize = Min_(root["CardCout"].size(),PLAY_COUNT);
	arraySize = (arraySize > PLAY_COUNT)? PLAY_COUNT:arraySize;
	for (int i = 0; i <arraySize; i++)
	{
		char _index[10];
		memset(_index,0,sizeof(_index));
		sprintf(_index,"%d",i);
		Json::Value valKeyValue = root["CardCout"][i][_index];
		int tempKeyValue = valKeyValue.asInt();
		if(MAX_CARD_COUNT == tempKeyValue)
		{
			iCardCount[i] = tempKeyValue;
		}
	}

	//获取手牌数据（手牌的具体值）
	BYTE byCardData[PLAY_COUNT][MAX_CARD_COUNT];
	memset(byCardData,0,sizeof(byCardData));
	bool bHandFlag[PLAY_COUNT];
	memset(bHandFlag,false,sizeof(bHandFlag));
	for (int i = 0; i <PLAY_COUNT; i++)
	{
		if(MAX_CARD_COUNT != iCardCount[i])
		{
			continue;
		}
		char _array[10];
		memset(_array,0,sizeof(_array));
		sprintf(_array,"%d",i);
		for(int j=0;j<iCardCount[i];j++)
		{
			char _index[10];
			memset(_index,0,sizeof(_index));
			sprintf(_index,"%d",j);
			Json::Value valKeyValue = root["CardData"][_array][j][_index];
			int tempKeyValue = valKeyValue.asInt();

			byCardData[i][j] = tempKeyValue;
			//bHandFlag[i] = true;
		}
		bHandFlag[i] = true;
	}

	//获取癞子牌
	BYTE byMCardData[PLAY_COUNT][MAX_CARD_COUNT];
	memset(byMCardData,0,sizeof(byMCardData));
	for (int i = 0; i <1; i++)
	{
		char _array[10];
		memset(_array,0,sizeof(_array));
		sprintf(_array,"%d",i);
		for(int j=0;j<1;j++)
		{
			char _index[10];
			memset(_index,0,sizeof(_index));
			sprintf(_index,"%d",j);
			Json::Value valKeyValue = root["MCardData"][_array][j][_index];
			int tempKeyValue = valKeyValue.asInt();

			byMCardData[i][j] = tempKeyValue;
		}
	}

	fin.close();

	//配癞子
	if (m_bHuiErPai)
	{
		m_Logic.m_byLogicHuiPai = byMCardData[0][0];
	}

	//把选择的手牌删除
	int iChangeNums=0;
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(!bHandFlag[i])
		{
			continue;
		}
		for(int b=0;b<MAX_CARD_COUNT;b++)
		{
			if(0 == byCardData[i][b])
			{
				continue;
			}
			for(int c=0;c<52;c++)
			{
				if(0 == m_iTotalCard[c])
				{
					continue;
				}
				if(byCardData[i][b] == m_iTotalCard[c])
				{
					m_iTotalCard[c] = 0;
					iChangeNums++;
					break;
				}
			}
		}
	}

	//先把配的牌配在指定的位置
	int index=0;
	BYTE byTempMenPai[52];
	memset(byTempMenPai,0,sizeof(byTempMenPai));
	for(int j = 0;j<PLAY_COUNT;j++)
	{
		if(bHandFlag[j])
		{
			for(int k=0;k<MAX_CARD_COUNT;k++)
			{
				byTempMenPai[j*MAX_CARD_COUNT + k] = byCardData[j][k];
			}
		}
	}

	//把牌补齐
	for(int i = 0;i < 52;i++)
	{
		if(byTempMenPai[i] == 0)
		{
			for(int k = 0;k < 52;k++)
			{
				if(m_iTotalCard[k] != 0)
				{
					byTempMenPai[i] = m_iTotalCard[k];
					m_iTotalCard[k] = 0;
					break;
				}
			}
		}
	}

	memset(m_iTotalCard,0,sizeof(m_iTotalCard));
	memcpy(m_iTotalCard,byTempMenPai,sizeof(m_iTotalCard));

	return true;
}