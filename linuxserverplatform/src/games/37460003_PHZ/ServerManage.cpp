#include "ServerManage.h"


//获取下一个玩家位置
BYTE CServerGameDesk::GetNextDeskStation(BYTE bDeskStation, bool realNext /*=false*/)
{
	if (!m_cfgbTurnRule)//顺时针
	{
		int i = 1;
		for (; i < PLAY_COUNT; i++)
		{
			BYTE next = (bDeskStation + i) % PLAY_COUNT;
			long nextUserID = GetUserIDByDeskStation(next);
			if ( nextUserID > 0 && m_byUserStation[next] != STATE_NULL && (realNext || (!realNext && next != m_byZuoXingUser)) )
			{
				return next;
			}
		}
	}
	//逆时针
	int i = PLAY_COUNT - 1;
	for (; i >= 1; i--)
	{
		BYTE next = (bDeskStation + i) % PLAY_COUNT;
		long nextUserID = GetUserIDByDeskStation(next);
		if ( nextUserID > 0 && m_byUserStation[next] != STATE_NULL && (realNext || (!realNext && next != m_byZuoXingUser)) )
		{
			return next;
		}
	}
	return 255;
}

//构造函数
CServerGameDesk::CServerGameDesk(void) :CGameDesk(FULL_BEGIN)
{
	SetGameStation(GS_WAIT_ARGEE);//游戏状态
	ReSetGameState();
	m_tZongResult.Init();
	m_byBankerUser = 0xff;
	m_tGameRules.Clear();
	memset(m_haveAutoCount, 0, sizeof(m_haveAutoCount));
	m_byZuoXingUser = 255;
	m_superSetCard = 255;
	m_superSetChairID = 255;
	m_superWangNum = 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------
CServerGameDesk::~CServerGameDesk()
{

}

//重置游戏状态
bool CServerGameDesk::ReSetGameState()
{
	SetGameStation(GS_WAIT_NEXT);
	memset(m_byUserStation,STATE_NULL,sizeof(m_byUserStation));
	//游戏变量
	m_byCurrentUser = INVALID_CHAIR;
	m_byTiOrPaoPaiUser = INVALID_CHAIR;
	memset(m_byCardIndex,0,sizeof(m_byCardIndex));
	memset(m_byUserHuXiCount,0,sizeof(m_byUserHuXiCount));

	//标志变量
	m_bOutCard = false;
	m_bDispatch = false;
	m_bEstimate = false;

	//提偎变量
	m_bTiCard = false;
	m_bSendNotify = false;

	//辅助变量
	m_byOutCardData = 0;
	m_byResumeUser = INVALID_CHAIR;
	m_byOutCardUser = INVALID_CHAIR;
	//发牌信息
	m_byLeftCardCount=0;
	memset(m_byRepertoryCard,0,sizeof(m_byRepertoryCard));
	memset(m_byCardIndexJing,0,sizeof(m_byCardIndexJing));
	//用户状态
	memset(m_bResponse,0,sizeof(m_bResponse));
	memset(m_iUserAction,0,sizeof(m_iUserAction));
	memset(m_byChiCardKind,0,sizeof(m_byChiCardKind));
	memset(m_iPerformAction,0,sizeof(m_iPerformAction));

	//限制状态
	memset(m_bMingPao,false,sizeof(m_bMingPao));
	memset(m_bAbandonCard,false,sizeof(m_bAbandonCard));
	memset(m_bAbandonCard1,false,sizeof(m_bAbandonCard1));

	//清除胡牌信息
	memset(m_tHuCardInfo,0,sizeof(m_tHuCardInfo));
	m_byHuUser = 255;
	m_iHuOperation = 0;
	m_iHuRealOperation = 0;

	memset(m_byUserOutCard,0,sizeof(m_byUserOutCard));			//玩家出牌数据（弃牌数）
	memset(m_iOutCardCount,0,sizeof(m_iOutCardCount));		    //玩家出牌个数
	m_tOutCard.Clear();
	m_byXingPai=255;
	m_iVideoIndex=0;
	m_element.clear();
	m_root.clear();
	m_fVideoFile.close();
	memset(m_GameData.m_bStartHu,false,sizeof(m_GameData.m_bStartHu));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_piaoV[i] = -1;
	}
	m_byCurrentCatchUser = 255;
	return true;
}

//初始化
bool CServerGameDesk::InitDeskGameStation()
{
	sprintf(m_GameData.m_szLogFile,"\\%s_%d\\",m_pDataManage->m_InitData.szGameRoomName,m_deskIdx);
	m_iRoomID=m_pDataManage->GetRoomID();
	m_byZuoXingUser = 255;
	
	LoadDynamicConfig();

	return true;
}

//加载ini配置文件
BOOL	CServerGameDesk::LoadIni()
{
	char szfile[256] = "";
	sprintf(szfile, "%s%d_s.ini", CINIFile::GetAppPath().c_str(), NAME_ID);
	CINIFile f(szfile);

	string key = "game";

	m_cfgbyBeginTime = f.GetKeyVal(key, "begintime", 30);
	m_cfgPiaoTime = f.GetKeyVal(key, "cfgPiaoTime", 10);
	m_cfgbyThinkTime = f.GetKeyVal(key, "thinktime", 60);

	m_cfgbTurnRule = f.GetKeyVal(key, "TurnRule", 0);
	m_choushui = f.GetKeyVal(key, "cslv", 3);

	m_tGameRules.byKingNum = f.GetKeyVal(key, "byKingNum", 4);
	m_tGameRules.byMinHuXi = f.GetKeyVal(key, "byMinHuXi", 10);
	m_tGameRules.byUserCount = f.GetKeyVal(key, "byUserCount", 3);
	m_tGameRules.bGenXing = (f.GetKeyVal(key, "bGenXing", 1) == 1 ? true : false);
	m_tGameRules.iMaxScore = f.GetKeyVal(key, "iMaxScore", 0);

	m_tGameRules.tunXiRatio = f.GetKeyVal(key, "tunXiRatio", 3);
	m_tGameRules.minHuXiTun = f.GetKeyVal(key, "minHuXiTun", 1);
	m_tGameRules.playType = f.GetKeyVal(key, "playType", 1);
	m_tGameRules.maoHu = (f.GetKeyVal(key, "maoHu", 0) == 1 ? true : false);
	m_tGameRules.b15Type = (f.GetKeyVal(key, "b15Type", 0) == 1 ? true : false);
	m_tGameRules.piaoFen = f.GetKeyVal(key, "piaoFen", 0);
	m_tGameRules.firstBanker = f.GetKeyVal(key, "firstBanker", 0);
	m_tGameRules.autoOverTime = f.GetKeyVal(key, "autoOverTime", 1);
	m_tGameRules.autoCount = f.GetKeyVal(key, "autoCount", 0);
	m_tGameRules.bShuangXing = (f.GetKeyVal(key, "bShuangXing", 0) == 1 ? true : false);
	m_tGameRules.bXianHuXianFan = (f.GetKeyVal(key, "bXianHuXianFan", 1) == 1 ? true : false);

	return true;
}
//根据房间ID加载配置文件
BOOL CServerGameDesk::LoadExtIni(int iRoomID)
{
	char szKey[20];
	char nid[20] = "";
	sprintf(nid, "%d", NAME_ID);
	sprintf(szKey, "%s_%d", nid, iRoomID);
	string key = szKey;
	string s = CINIFile::GetAppPath();/////本地路径
	CINIFile f(s + nid + "_s.ini");



	return TRUE;
}

//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{ 
	//吃碰操作定时器
	if (uTimerID >= TIME_OPER_OUTTIMER && uTimerID < TIME_OPER_OUTTIMER + PLAY_COUNT)
	{
		KillTimer(uTimerID);
		if (m_tGameRules.autoOverTime <= 0)
		{
			return true;
		}
		BYTE chairID = uTimerID - TIME_OPER_OUTTIMER;
		OperOutTimer(chairID);
		return true;
	}

	switch (uTimerID)
	{
		case TIME_AUTO_AGREE:
		{
			KillTimer(TIME_AUTO_AGREE);
			for (BYTE i = 0; i < PLAY_COUNT; i++)
			{
				if (GetUserIDByDeskStation(i) <= 0) continue;
				if (IsAuto(i))
				{
					HandleNotifyMessage(i, MSG_ASS_LOADER_GAME_AGREE, NULL, 0, false);
				}
			}
			return true;
		}
		case TIME_PIAO:
		{
			KillTimer(TIME_PIAO);
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (GetUserIDByDeskStation(i) <= 0) continue;
				if (m_piaoV[i] < 0)
				{
					OnUserPiao(i, 0); //超时选不漂
				}
			}
			return true;
		}
		case TIME_GAME_BEING_PLAY:				//开始出牌
		{
			NoticBeginPlayGame();
			return true; 
		}
		case TIME_OUTCARD_OUTTIME:				//出牌超时
		{
			KillTimer(TIME_OUTCARD_OUTTIME);
			if (m_tGameRules.autoOverTime <= 0)
			{
				return true;
			}
			if (!IsAuto(m_byCurrentUser))
			{
				OnHandleUserRequestAuto(m_byCurrentUser);
			}
			AutoOutCard();

			return true;
		}
		case TIME_SEND_NEXT_INTEVAL:			//定时发送下一张牌
		{
			KillTimer(TIME_SEND_NEXT_INTEVAL);
			OnUserContinueCard(m_byCurrentUser);
			return true;
		}
		case TIME_REMAINING:
		{
			if(m_byRemainingTime != 0)
			{
				m_byRemainingTime --;
			}
			return true;
		}
		case TIME_FINISH:
		{
			KillTimer(TIME_FINISH);
			GameFinish(0, GF_NORMAL);
			return true;
		}
		case TIME_PAO:
			{
				KillTimer(TIME_PAO);
				bool bPerformAction = false;
				CheckPaoPaiAction(m_byCatchCardUser, m_byCatchCardData,m_bDispatch,bPerformAction);
				return true;
			}
		case TIME_WEI_TI_DELAY_HU:
		{
			KillTimer(TIME_WEI_TI_DELAY_HU);
			//发送消息
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (STATE_PLAY_GAME != m_byUserStation[i])	continue;
				SendOperateNotify(i, m_byResumeUser, m_tmpCard, m_iUserAction[i], m_tChiCardInfo[i]);
			}
			KillAllTimer();
			MySetTimer(TIME_OPER_OUTTIMER, m_cfgbyThinkTime * 1000);
			return true;
		}
	}
	return CGameDesk::OnTimer(uTimerID);
}

// 玩家请求托管
bool CServerGameDesk::OnHandleUserRequestAuto(BYTE deskStation)
{
	return CGameDesk::OnHandleUserRequestAuto(deskStation);
}

// 玩家取消托管
bool CServerGameDesk::OnHandleUserRequestCancelAuto(BYTE deskStation)
{
	m_haveAutoCount[deskStation] = 0;

	if (m_byCurrentUser == INVALID_CHAIR && m_iUserAction[deskStation] != ACK_NULL)
	{
		KillTimer(TIME_OPER_OUTTIMER + deskStation);
		MySetTimer(TIME_OPER_OUTTIMER + deskStation, m_cfgbyThinkTime * 1000);
	}
	else if (m_byCurrentUser >= 0 && m_byCurrentUser < PLAY_COUNT)
	{
		KillTimer(TIME_OUTCARD_OUTTIME);
		MySetTimer(TIME_OUTCARD_OUTTIME, m_byRemainingTime * 1000);
	}

	return CGameDesk::OnHandleUserRequestCancelAuto(deskStation);
}

//通知开始出牌
void  CServerGameDesk::NoticBeginPlayGame()
{
	KillTimer(TIME_GAME_BEING_PLAY);

	//设置变量
	m_bOutCard=true;
	m_byOutCardData = 0;
	m_byCurrentUser = m_byBankerUser;
	m_byOutCardUser = INVALID_CHAIR;

	if(!CheckAction(m_byCurrentUser, 0,3))
	{
		S_C_BeginPlay tBeginPlay;
		tBeginPlay.byCurrentUser = m_byCurrentUser;
		BroadcastDeskData(&tBeginPlay, sizeof(S_C_BeginPlay), MSG_MAIN_LOADER_GAME, S_C_GAME_BEGIN_PLAY, 0);

		UserData userData;
		GetUserData(m_byCurrentUser, userData);
		if (userData.isVirtual)
		{
			m_byRemainingTime = 6;
		}
		else if (IsAuto(m_byCurrentUser))
		{
			m_byRemainingTime = 3;
		}
		else
		{
			m_byRemainingTime = m_cfgbyThinkTime;
		}
		MySetTimer(TIME_OUTCARD_OUTTIME, m_byRemainingTime * 1000);
	}
	else
	{
		m_GameData.m_bStartHu[m_byBankerUser]=true;
	}
}

//玩家出牌超时
void  CServerGameDesk::AutoOutCard()
{
	bool bFind = false;
	//寻找单牌
	for (BYTE i = 0;i < sizeof(m_byCardIndex[m_byCurrentUser]);i++)
	{
		if (m_byCardIndex[m_byCurrentUser][i] == 1)
		{
			OnUserOutCard(m_byCurrentUser,m_GameLogic.SwitchToCardData(i));
			bFind = true;
		}
	}

	if(!bFind)
	{
		//寻找对牌
		for (BYTE i = 0;i < sizeof(m_byCardIndex[m_byCurrentUser]);i++)
		{
			if (m_byCardIndex[m_byCurrentUser][i] == 2)
			{
				OnUserOutCard(m_byCurrentUser,m_GameLogic.SwitchToCardData(i));
				bFind = true;
			}
		}
	}
}

//玩家操作牌超时
void  CServerGameDesk::OperOutTimer(BYTE chairID)
{

	for(BYTE i = 0;i < PLAY_COUNT;i ++)
	{
		if (i != chairID) continue;
		if(STATE_PLAY_GAME == m_byUserStation[i])		//此标志位用于记录当前玩家是否玩游戏(因为有一种模试是，四个人进来，但只有三个人在玩)
		{
			if(!m_bResponse[i])							//未操作
			{
				if(m_iUserAction[i])					//玩家有动作
				{
					int iAction=ACK_NULL;
					if(m_iUserAction[i] & ACK_CHIHU)
					{
						iAction=ACK_CHIHU;
					}
					if(m_iUserAction[i] & ACT_WANG_DIAO)
					{
						iAction=ACT_WANG_DIAO;
					}
					if(m_iUserAction[i] & ACT_WANG_CHUANG)
					{
						iAction=ACT_WANG_CHUANG;
					}
					if(m_iUserAction[i] & ACT_SWANG_CHUANG)
					{
						iAction=ACT_SWANG_CHUANG;
					}
					if(m_iUserAction[i] & ACT_WANG_ZHA)
					{
						iAction=ACT_WANG_ZHA;
					}
					if (!IsAuto(i))
					{
						OnHandleUserRequestAuto(i);
					}
					OnUserOperateCard(i,iAction,0,NULL);
				}
			}
		}
	}
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser/* = false*/)
{
	if (bWatchUser)
	{
		return true;
	}
	switch (assistID)
	{
	case MSG_ASS_LOADER_GAME_AGREE:		//用户同意游戏
	{
		if (GetGameStation() != GS_WAIT_ARGEE && GetGameStation() != GS_WAIT_NEXT)
		{
			return true;
		}
		SetGameStation(GS_WAIT_ARGEE);
		if (IsAuto(bDeskStation))
		{
			OnHandleUserRequestCancelAuto(bDeskStation);
		}
		return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
	}
	case C_S_OUT_CARD:				//玩家出牌
	{
		SAFECHECK_STRUCT(pOutCard, C_S_OutCard, pData, size)
		OnUserOutCard(bDeskStation,pOutCard->byCardData);
		return true;
	}
	case C_S_OPERATE_CARD:			//玩家操作牌
	{
		SAFECHECK_STRUCT(pOperateCard,C_S_OperateCard,pData,size)
		OnUserOperateCard(bDeskStation,pOperateCard->iOperateCode,pOperateCard->iChiKind,pOperateCard->byChiData);
		return true;
	}
	case C_S_PIAO_REQ:			//选漂分请求
	{
		SAFECHECK_STRUCT(pPiao, C_S_PiaoReq,pData,size)
		OnUserPiao(bDeskStation, pPiao->piaoV);
		return true;
	}
	case C_S_SUPER_REQUEST_LOOK:		//超端看牌
	{
		if (GetGameStation() == GS_PLAYGAME)
		{
			SuperUserLookMenPai(bDeskStation);
		}
		return true;
	}
	case C_S_SUPER_REQUEST_CRAD:		//超端要牌
	{
		if (GetGameStation() == GS_PLAYGAME)
		{
			SuperUserChangePai(bDeskStation, pData, size);
		}
		return true;
	}
	case C_S_SUPER_GET_WANG_REQ:		//超端设置发牌时拿几个王请求
	{
		SuperUserGetWangReq(bDeskStation, pData, size);
		return true;
	}
	}
	return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
}


//用户出牌
bool CServerGameDesk::OnUserOutCard(BYTE byChairID, BYTE byCardData)
{
	if(STATE_PLAY_GAME != m_byUserStation[byChairID])
	{
		return true;
	}
	//效验状态
	if (m_GameLogic.IsValidCard(byCardData) == false)
	{
		return true;
	}
	if ((byChairID != m_byCurrentUser) || (m_bOutCard == false)) 
	{
		return true;
	}
	if(m_GameData.m_tJingData.CheckIsJing(byCardData))  //癞子不能打
	{ 
		return true;
	} 

	if(m_GameData.m_bFinish)
	{
		return true;
	}
	//三张不能出
	if (m_byCardIndex[byChairID][m_GameLogic.SwitchToCardIndex(byCardData)] >= 3)
	{
		return true;
	}

	//删除扑克
	if (m_GameLogic.RemoveCard(m_byCardIndex[byChairID],byCardData) == false)
	{
		return true;
	}

	m_GameData.m_bHuJing[byChairID]=false;
	memset(m_GameData.m_bHuTiOrPao,false,sizeof(m_GameData.m_bHuTiOrPao));
	memset(m_GameData.m_bHuWei,false,sizeof(m_GameData.m_bHuWei));
	memset(m_GameData.m_bBiePaoSpecialHu,false,sizeof(m_GameData.m_bBiePaoSpecialHu));
	memset(m_GameData.m_bBiePaoSpecialChi,false,sizeof(m_GameData.m_bBiePaoSpecialChi));
	memset(m_GameData.m_bStartHu,false,sizeof(m_GameData.m_bStartHu));
	//发送消息
	S_C_OutCard tOutCard;
	tOutCard.byOutCardUser = byChairID;
	tOutCard.byOutCardData = byCardData;
	AddOutCard(byChairID,byCardData);
	//发送数据
	BroadcastDeskData(&tOutCard, sizeof(S_C_OutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD, 0);

	char logBuf[1024] = { 0 };
	int len = 0;
	len += sprintf(logBuf, "【OUTCARD】 userID:%ld, card:%d\n", GetUserIDByDeskStation(byChairID), byCardData);
	FILE_LOG(GAMENAME, logBuf);

	OnJson(&tOutCard,7);
	//设置变量
	m_bOutCard=false;
	m_bDispatch=false;
	m_bEstimate=false;
	m_bSendNotify = false;
	m_byOutCardUser = byChairID;
	m_byOutCardData = byCardData;
	m_byCurrentUser = INVALID_CHAIR;
	m_byCatchCardUser=INVALID_CHAIR;
	m_byCatchCardData=0;
	m_byResumeUser = GetNextDeskStation(byChairID);
	//臭牌记录
	m_bAbandonCard[byChairID][m_GameLogic.SwitchToCardIndex(m_byOutCardData)]=true;
	m_bAbandonCard1[byChairID][m_GameLogic.SwitchToCardIndex(m_byOutCardData)]=true;
	//动作判断
	BYTE cbCurrentCard=(m_byOutCardUser != INVALID_CHAIR) ? m_byOutCardData : 0;
	bool bUserAction = EstimateUserRespond(m_byOutCardUser,cbCurrentCard,m_bDispatch);
	//派发扑克
	if (!bUserAction)
	{
		m_byCurrentUser = byChairID;
		SetTimer(TIME_SEND_NEXT_INTEVAL,1000);
	}
	return true;
}

//用户操作
bool CServerGameDesk::OnUserOperateCard(BYTE byChairID, int iOperateCode, BYTE byChiKind,BYTE byData[3][3])
{
	//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
	if (iOperateCode == ACT_WANG_ZHA)
	{
		iOperateCode = ACT_SWANG_CHUANG;
	}
	if(STATE_PLAY_GAME != m_byUserStation[byChairID])
	{
		return true;
	}
	if (m_byCurrentUser != INVALID_CHAIR)
	{ 
		return true; 
	}
	if(m_GameData.m_bFinish)
	{
		return true;
	}
	if ((true == m_bResponse[byChairID])||((ACK_NULL != iOperateCode) && (m_iUserAction[byChairID] & iOperateCode)==0))
	{ 
		return true; 
	}
	//构造数据
	S_C_ActionResult tActionRes;
	tActionRes.byDeskStation = byChairID;
	tActionRes.iOperateCode = iOperateCode;

	//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
	if (iOperateCode == ACT_SWANG_CHUANG)
	{
		tActionRes.iOperateCode = ACT_WANG_ZHA;
	}

	//发送数据
	BroadcastDeskData(&tActionRes, sizeof(S_C_ActionResult), MSG_MAIN_LOADER_GAME, S_C_ACTION_RESULT, 0);

	char logBuf[1024] = { 0 };
	int len = 0;
	len += sprintf(logBuf, "【ACTION】 userID:%ld, iOperateCode:%d\n", GetUserIDByDeskStation(byChairID), iOperateCode);
	FILE_LOG(GAMENAME, logBuf);

	OnJson(&tActionRes,12);
	m_GameData.m_bHuJing[byChairID]=false;
	if(iOperateCode == ACK_NULL && (ACK_CHI & m_iUserAction[byChairID]))
	{
		m_bAbandonCard[byChairID][m_GameLogic.SwitchToCardIndex(m_byOutCardData)] = true;
	}
	if (iOperateCode == ACK_NULL && (ACK_PENG & m_iUserAction[byChairID]))
	{
		m_bAbandonCard1[byChairID][m_GameLogic.SwitchToCardIndex(m_byOutCardData)] = true;
	}
	//变量定义
	int iTargetAction = iOperateCode;
	BYTE byTargetUser = (iOperateCode == ACK_NULL) ? INVALID_CHAIR : byChairID;

	//设置变量
	m_bResponse[byChairID] = true;
	m_byChiCardKind[byChairID] = byChiKind;
	m_iPerformAction[byChairID] = iOperateCode;
	if(nullptr != byData)
	{
		memcpy(&m_tChiData[byChairID],byData,sizeof(SaveChiData));
	}
	
	//优先处理
	if ((iOperateCode != ACK_NULL)&&(m_iUserAction[byChairID] & ACK_CHI_EX))
	{
		m_iPerformAction[byChairID] |= ACK_CHI_EX;
	}
//	if((iOperateCode & ACK_CHI) && !m_bResponse[GetNextDeskStation(byChairID)] && (ACK_CHI & m_iUserAction[GetNextDeskStation(byChairID)]))
	if((iOperateCode & ACK_CHI) && m_GameData.m_byChiPriority[0] != 255 && byChairID != m_GameData.m_byChiPriority[0] && m_bResponse[m_GameData.m_byChiPriority[0]] == false)
	{
		return true; 
	}
	//执行判断
	for (BYTE i = 0;i < PLAY_COUNT;i++)
	{
		//获取动作
		int iUserAction = (m_bResponse[i] == false) ? m_iUserAction[i] : m_iPerformAction[i];
		//动作判断
		if (iUserAction > iTargetAction)
		{
			byTargetUser = i;
			iTargetAction = iUserAction;
		}
	}
	if ((byTargetUser != INVALID_CHAIR) && (m_bResponse[byTargetUser] == false)) //最大的未执行则反回
	{ 
		return true; 
	}	
	//吃胡排序 检测吃胡 王钓 王闯 王炸(主要是寻找最胡牌最近的玩家)
	if (iTargetAction == ACK_CHIHU || iTargetAction == ACT_WANG_DIAO ||
		iTargetAction == ACT_WANG_CHUANG ||  ACT_SWANG_CHUANG==iTargetAction || iTargetAction == ACT_WANG_ZHA)
	{
		bool bSuccess = true;
		BYTE byStation=255;
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if(byChairID==m_GameData.m_byHuPriority[i])
			{
				byStation=i;
				break;
			}
		}
		if(255!=byStation && byStation>0)
		{
			for(int i=(byStation-1);i>=0;i--)
			{
				if(255==m_GameData.m_byHuPriority[i])
				{
					continue;
				}
				if(!m_bResponse[m_GameData.m_byHuPriority[i]])
				{
					bSuccess=false;
					break;
				}
			}
		}
		if(!bSuccess)
		{
			return true;
		}
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if(255==m_GameData.m_byHuPriority[i])
			{
				continue;
			}
			BYTE byStation=m_GameData.m_byHuPriority[i];
			if(m_bResponse[byStation] && (ACK_CHIHU==m_iPerformAction[byStation] || ACT_WANG_DIAO==m_iPerformAction[byStation] ||
				ACT_WANG_CHUANG==m_iPerformAction[byStation] ||  ACT_SWANG_CHUANG==m_iPerformAction[byStation] || ACT_WANG_ZHA==m_iPerformAction[byStation]))
			{
				byTargetUser=byStation;
				break;
			}
		}
	}
	if(ACK_NULL==iOperateCode)
	{
		for(int iPri=0;iPri<PLAY_COUNT;iPri++)
		{
			BYTE byPlayer=m_GameData.m_byHuPriority[iPri];
			if(255==byPlayer)
			{
				continue;
			}
			if(m_bResponse[byPlayer] && (m_iPerformAction[byPlayer] == ACK_CHIHU || m_iPerformAction[byPlayer] == ACT_WANG_DIAO ||
				m_iPerformAction[byPlayer] == ACT_WANG_CHUANG ||  ACT_SWANG_CHUANG==m_iPerformAction[byPlayer] || m_iPerformAction[byPlayer] == ACT_WANG_ZHA))
			{
				byTargetUser=byPlayer;
				iTargetAction=m_iPerformAction[byPlayer];
			}
		}
	}
	if (iTargetAction & ACK_CHI || iTargetAction & ACK_CHI_EX)
	{
		for(int i=0;i<2;i++)
		{
			BYTE byStation=m_GameData.m_byChiPriority[i];
			if(255!=byStation && m_bResponse[byStation] && ACK_CHI==m_iPerformAction[byStation])
			{
				byTargetUser=byStation;
				break;
			}
		}
	}
	//设置状态
	memset(m_bResponse,0,sizeof(m_bResponse));
	memset(m_iUserAction,0,sizeof(m_iUserAction));
	memset(m_iPerformAction,0,sizeof(m_iPerformAction));

	//吃牌操作
	if (iTargetAction & ACK_CHI || iTargetAction & ACK_CHI_EX)
	{	
		bool bPerformAction = false;
		if(m_GameData.m_bBiePaoSpecialChi[byTargetUser] && CheckPaoPaiAction(byTargetUser,m_byCatchCardData,m_bDispatch,bPerformAction))
		{
			return true;
		}
		//吃牌处理
		BYTE byDebarCard = m_byOutCardData;
		BYTE byResultCount = 0;
		for (BYTE k = 0;k < 3;k++)
		{
			if(!m_GameLogic.IsValidCard(m_tChiData[byTargetUser].byData[k][0]))
			{
				continue;
			}
			//保存吃牌数据
			tagWeaveItem tCPGTWPData;
			tCPGTWPData.byCardCount = 3;
			tCPGTWPData.iWeaveKind = ACK_CHI;
			tCPGTWPData.byCenterCard = m_byOutCardData;
			tCPGTWPData.byCardList[0] = m_tChiData[byTargetUser].byData[k][0];
			tCPGTWPData.byCardList[1] = m_tChiData[byTargetUser].byData[k][1];
			tCPGTWPData.byCardList[2] = m_tChiData[byTargetUser].byData[k][2];
			m_GameData.AddToCPTWP(byTargetUser,tCPGTWPData);

			//删除扑克
			for (BYTE l = 0;l < 3;l++)
			{
				BYTE cbCurrentCard = m_tChiData[byTargetUser].byData[k][l];
				if (cbCurrentCard == byDebarCard) byDebarCard = 0;
				else m_byCardIndex[byTargetUser][m_GameLogic.SwitchToCardIndex(cbCurrentCard)]--;
			}
			byResultCount ++;
		}
		//更新胡息
		UpdateUserHuXiCount(byTargetUser);
		//设置变量 
		m_byOutCardUser=INVALID_CHAIR;
		//构造数据
		S_C_UserChiCard tUserChiCard;
		tUserChiCard.byActionUser = byTargetUser;
		tUserChiCard.byActionCard = m_byOutCardData;
		tUserChiCard.byResultCount = byResultCount;
		memcpy(tUserChiCard.byCardData,m_tChiData[byTargetUser].byData,sizeof(tUserChiCard.byCardData));
		//发送数据
		BroadcastDeskData(&tUserChiCard, sizeof(S_C_UserChiCard), MSG_MAIN_LOADER_GAME, S_C_USER_CHI_CARD, 0);
		OnJson(&tUserChiCard,8);
		DeleteOutCard();

		SendOutCardNotify(byTargetUser,IsAllowOutCard(byTargetUser,true));
		return true;
	}
	//碰牌操作
	if (iTargetAction & ACK_PENG)
	{
		//设置扑克
		m_byCardIndex[byTargetUser][m_GameLogic.SwitchToCardIndex(m_byOutCardData)] = 0;
		//设置组合
		tagWeaveItem tCPGTWPData;
		tCPGTWPData.byCardCount = 3;
		tCPGTWPData.iWeaveKind = ACK_PENG;
		tCPGTWPData.byCenterCard = m_byOutCardData;
		tCPGTWPData.byCardList[0] = m_byOutCardData;
		tCPGTWPData.byCardList[1] = m_byOutCardData;
		tCPGTWPData.byCardList[2] = m_byOutCardData;
		m_GameData.AddToCPTWP(byTargetUser,tCPGTWPData);

		//更新胡息
		UpdateUserHuXiCount(byTargetUser);
		//设置变量
		m_byOutCardUser = INVALID_CHAIR;
		//构造数据
		S_C_UserPengCard tUserPengCard;
		tUserPengCard.byActionUser = byTargetUser;
		tUserPengCard.byActionCard = m_byOutCardData;
		//发送数据
		BroadcastDeskData(&tUserPengCard, sizeof(S_C_UserPengCard), MSG_MAIN_LOADER_GAME, S_C_USER_PENG_CARD, 0);
		OnJson(&tUserPengCard,9);
		DeleteOutCard();
		SendOutCardNotify(byTargetUser,IsAllowOutCard(byTargetUser,true));
		return true;
	}
	//胡操作
	if (iTargetAction == ACK_CHIHU || iTargetAction == ACT_WANG_DIAO ||
		iTargetAction == ACT_WANG_CHUANG || ACT_SWANG_CHUANG==iTargetAction || iTargetAction == ACT_WANG_ZHA)
	{
		if(m_GameData.m_bTryBigAction[byChairID]>ACK_NULL)
		{
			bool bCheckAnyHu = false;
			if (iTargetAction == ACT_WANG_DIAO || iTargetAction == ACT_WANG_CHUANG || ACT_SWANG_CHUANG == iTargetAction || iTargetAction == ACT_WANG_ZHA)
			{
				bCheckAnyHu = true;
			}
			BYTE bySendCardData =GetPai(byChairID);
			SetXingPai(bySendCardData);

			m_byOutCardData = bySendCardData;
			S_C_SendCard tSendCard;
			tSendCard.byCardData = bySendCardData;
			tSendCard.byAttachUser = byChairID;
			tSendCard.byLeftCardCount = m_byLeftCardCount;

			CountCPTWPXingNumber(byChairID);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(byChairID));
			memset(m_tHuCardInfo,0,sizeof(m_tHuCardInfo));
			m_GameLogic.GetHuCardInfo(byChairID,255,m_byCardIndex[byChairID],m_byCardIndexJing[byChairID],bySendCardData,m_byUserHuXiCount[byChairID],m_tHuCardInfo[byChairID],true,m_GameData.m_tCPTWPArray[byChairID], false, bCheckAnyHu);
			for(int j = 1;j < 5;j ++)
			{
				if(0 != (m_tHuCardInfo[byChairID][j].iHuType & WANG_DIAO))   //王钓
				{ 
					m_tHuCardInfo[byChairID][j].bZiMo = true;   
				}
				if(0 != (m_tHuCardInfo[byChairID][j].iHuType & WANG_CHUANG)) //王闯
				{ 
					m_tHuCardInfo[byChairID][j].bZiMo = true; 
				}
				if(0 != (m_tHuCardInfo[byChairID][j].iHuType & SWANG_CHUANG)) //三王闯
				{ 
					m_tHuCardInfo[byChairID][j].bZiMo = true; 
				}
				if(0 != (m_tHuCardInfo[byChairID][j].iHuType & WANG_ZHA))    //王炸
				{ 
					m_tHuCardInfo[byChairID][j].bZiMo = true;    
				}
			}
			tSendCard.iOperateCode=iTargetAction;
			BroadcastDeskData(&tSendCard, sizeof(S_C_SendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
			OnJson(&tSendCard,6);
			byTargetUser=byChairID;
			m_byHuUser = byTargetUser;
			if(KINGVALUE == bySendCardData)
			{
				if(ACT_WANG_DIAO==iTargetAction)
				{
					iTargetAction=ACT_WANG_CHUANG;
				}
				else if(ACT_WANG_CHUANG==iTargetAction)
				{
					iTargetAction=ACT_SWANG_CHUANG;
				}
				else if(ACT_SWANG_CHUANG==iTargetAction)
				{
					iTargetAction=ACT_WANG_ZHA;
				}
			}
			m_iHuRealOperation = iTargetAction;
			//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
			if (iTargetAction == ACT_WANG_ZHA)
			{
				iTargetAction = ACT_SWANG_CHUANG;
			}
			m_iHuOperation = iTargetAction;
			KillAllTimer();
			m_GameData.m_bFinish=true;
			SetTimer(TIME_FINISH,2*1000);
		}
		else
		{
			m_byHuUser = byTargetUser;
			//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
			if (iTargetAction == ACT_WANG_ZHA)
			{
				iTargetAction = ACT_SWANG_CHUANG;
			}
			m_iHuOperation = iTargetAction;
			KillAllTimer();
			m_GameData.m_bFinish=true;
			//if(m_GameData.m_bForbidOutCard[m_byHuUser] && GetTiPaoNum(m_byHuUser)<=0 && !m_GameData.m_bBiePaoSpecialHu[m_byHuUser] 
			//&& m_bDispatch && KINGVALUE!=m_byCatchCardData && !m_GameData.m_bHuWei[m_byHuUser])//++
			//{
			//	CheckPiePaoHuAction(m_byHuUser,m_byCatchCardData,m_bDispatch);
			//}
			GameFinish(0, GF_NORMAL);
		}
		return true;
	}
	//动作判断(全部点过了之后，有可能还会有跑牌(胡牌的优先级大于跑))
	bool bUserAction = false;
	if(ACK_NULL==iTargetAction)
	{
		BYTE byCurrentCard=(m_byOutCardUser != INVALID_CHAIR) ? m_byOutCardData : 0;
		bool bPerformAction = false;
		if(m_GameData.m_bStartHu[byChairID])
		{
			SendOutCardNotify(byChairID,true);
			return true;
		}
		else if(m_GameData.m_bTryBigAction[byChairID]>ACK_NULL)
		{
			m_GameData.m_bTryBigAction[byChairID]=ACK_NULL;
			//派发扑克
			DispatchCardData(byChairID);
			return true;
		}
		if(m_GameData.m_bHuTiOrPao[byChairID])//出牌
		{
			m_GameData.m_bHuTiOrPao[byChairID]=false;
			SendOutCardNotify(byChairID,IsAllowOutCard(byChairID));
			return true;
		}
		else if(m_GameData.m_bHuWei[byChairID])
		{
			m_GameData.m_bHuWei[byChairID]=false;
			SendOutCardNotify(byChairID,IsAllowOutCard(byChairID,true));
			return true;
		}
		else
		{
			//检测跑牌
			bUserAction=CheckPaoPaiAction(m_byOutCardUser,byCurrentCard,m_bDispatch,bPerformAction);
			if(!bUserAction)//抓大王有动作点过以后通知出牌
			{
				if(KINGVALUE==m_byCatchCardData && byChairID==m_byCatchCardUser)
				{
					m_byCardIndexJing[byChairID]++;
					SendOutCardNotify(m_byCatchCardUser,true);
					return true;
				}
			}
		}
	}
	//派发扑克
	if (!bUserAction)
	{
		if(m_byTiOrPaoPaiUser == INVALID_CHAIR) 
		{
			//检测王钓，王闯，三王闯
			if(m_byLeftCardCount>0 && m_GameData.m_bForbidOutCard[m_byResumeUser] && CheckAction(m_byResumeUser,0,2))
			{
				m_GameData.m_bTryBigAction[m_byResumeUser]=m_iUserAction[m_byResumeUser];
			}
			else
			{
				DispatchCardData(m_byResumeUser);
			}
		}
		else
		{ 
			SendOutCardNotify(m_byTiOrPaoPaiUser,IsAllowOutCard(m_byTiOrPaoPaiUser));
			m_byTiOrPaoPaiUser = INVALID_CHAIR;
		}
	}
	return true;
}

//继续命令
bool CServerGameDesk::OnUserContinueCard(BYTE byChairID)
{
	//效验用户
	if ((byChairID != m_byCurrentUser) || (m_bOutCard == true))
	{
		return false;
	}
	//检测王钓，王闯，三王闯
	if(m_byLeftCardCount>0 && m_GameData.m_bForbidOutCard[GetNextDeskStation(byChairID)] && CheckAction(GetNextDeskStation(byChairID),0,2))
	{
		m_GameData.m_bTryBigAction[GetNextDeskStation(byChairID)]=m_iUserAction[GetNextDeskStation(byChairID)];
	}
	else
	{
		//派发扑克
		DispatchCardData(GetNextDeskStation(byChairID));
	}
	return true;
}


//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if(!IsGoldRoom())
	{
		SetGameRuler();
	}

	switch (GetGameStation())
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
	case GS_WAIT_NEXT:			//等待下一盘游戏开始
	{
		GameStation_WaiteAgree TGameStation;
		TGameStation.byBankerUser = m_byBankerUser;							//庄家
		TGameStation.byBeginTime = m_cfgbyBeginTime;						//游戏开始时间
		TGameStation.byThinkTime = m_cfgbyThinkTime;						//思考时间
		TGameStation.iCellScore = 1;
		TGameStation.tGameRule = m_tGameRules;
		TGameStation.byPiaoTime = m_cfgPiaoTime;							//漂分阶段时间
		TGameStation.bSuper = IsSuperUser(bDeskStation);
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}
	case GS_PIAO:			//漂分阶段
	{
		GameStation_PiaoFen TGameStation;
		TGameStation.byBankerUser = m_byBankerUser;							//庄家
		TGameStation.byBeginTime = m_cfgbyBeginTime;						//游戏开始时间
		TGameStation.byThinkTime = m_cfgbyThinkTime;						//思考时间
		TGameStation.iCellScore = 1;
		TGameStation.tGameRule = m_tGameRules;
		TGameStation.byPiaoTime = m_cfgPiaoTime;							//漂分阶段时间
		memcpy(TGameStation.piaoV, m_piaoV, sizeof(TGameStation.piaoV));
		if (m_piaoV[bDeskStation] < 0) //未选择
		{
			memcpy(TGameStation.piaoList, m_piaoList, sizeof(TGameStation.piaoList));
		}
		TGameStation.byZuoXingUser = m_byZuoXingUser;
		TGameStation.bSuper = IsSuperUser(bDeskStation);

		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}
	case GS_PLAYGAME:			//当前游戏状态
	{
		GameStation_PlayGame  TGameStation;

		//游戏信息
		TGameStation.byZuoXingUser = m_byZuoXingUser;
		TGameStation.byBankerUser = m_byBankerUser;							//庄家
		TGameStation.byBeginTime = m_cfgbyBeginTime;						//游戏开始时间
		TGameStation.byThinkTime = m_cfgbyThinkTime;						//思考时间
		TGameStation.byRemainingTime = m_byRemainingTime;					//余下时间
		TGameStation.iCellScore = 1;
		TGameStation.byCurrentUser = m_byCurrentUser;

		//出牌信息
		TGameStation.byOutCardUser = m_byOutCardUser;
		TGameStation.byOutCardData = m_byOutCardData;
		BYTE byCardIndexJing[PLAY_COUNT];
		memcpy(byCardIndexJing, m_byCardIndexJing, sizeof(byCardIndexJing));
		if (m_GameData.m_bHuJing[bDeskStation])
		{
			TGameStation.byOutCardData = 0;
			byCardIndexJing[bDeskStation]++;
		}
		TGameStation.bOutCard = m_bOutCard;
		memcpy(TGameStation.byUserOutCard, m_byUserOutCard, sizeof(m_byUserOutCard));
		if (255 != m_tOutCard.byChairID) TGameStation.byUserOutCard[m_tOutCard.byChairID][m_iOutCardCount[m_tOutCard.byChairID] - 1] = 0;
		//扑克信息
		TGameStation.byLeftCardCount = m_byLeftCardCount;
		m_GameLogic.SwitchToCardData(m_byCardIndex[bDeskStation], TGameStation.byCardData, CountArray(TGameStation.byCardData), byCardIndexJing[bDeskStation]);
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			TGameStation.byUserCardCount[i] = m_GameLogic.GetCardCount(m_byCardIndex[i], byCardIndexJing[i]);
		}
		//吃碰杠提喂跑信息
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			m_GameData.CopyCPTWPData(i, TGameStation.tCPGTWPArray[i]);
			TGameStation.byCPGTWPCount[i] = m_GameData.GetCPTWPCount(i);
		}
		memcpy(TGameStation.byUserHuXiCount, m_byUserHuXiCount, sizeof(m_byUserHuXiCount));
		//动作信息
		TGameStation.bResponse = m_bResponse[bDeskStation];
		//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
		TGameStation.iUserAction = m_iUserAction[bDeskStation];
		if (TGameStation.iUserAction & SWANG_CHUANG)
		{
			TGameStation.iUserAction &= 0xDFFF;
			TGameStation.iUserAction |= WANG_ZHA;
		}
		memcpy(&TGameStation.tChiCardInfo, &m_tChiCardInfo[bDeskStation], sizeof(TGameStation.tChiCardInfo));
		TGameStation.tGameRule = m_tGameRules;
		TGameStation.byPiaoTime = m_cfgPiaoTime;							//漂分阶段时间
		memcpy(TGameStation.piaoV, m_piaoV, sizeof(TGameStation.piaoV));
		TGameStation.bSuper = IsSuperUser(bDeskStation);

		if (m_byCurrentUser == bDeskStation && m_bOutCard)
		{
//			GetTingData(bDeskStation, TGameStation.byOutPaiTing, TGameStation.byOutPaiTingCards);
		}
		else
		{
//			GetHuPaiData(bDeskStation, TGameStation.byHuPai);
		}

		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(GameStation_PlayGame));
		return true;
	}
	}
	return false;
}

/*---------------------------------------------------------------------------------*/
//游戏开始
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (CGameDesk::GameBegin(bBeginFlag) == false)
	{
		GameFinish(0, GF_SALE);
		return false;
	}
	ReSetGameState();
	m_GameLogic.Init(&m_GameData);
	m_GameData.InitData();
	m_GameLogic.SetGameRule(m_tGameRules);

	if (IsGoldRoom())
	{
		char szGameRules[256] = "";
		sprintf(szGameRules, "{\"gameIdx\":1,\"roomType\":0,\"cCSFS\":1,\"cCSL\":%d}", m_choushui);
		memcpy(m_szGameRules, szGameRules, sizeof(m_szGameRules));
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (GetUserIDByDeskStation(i) > 0)
		{
			m_byUserStation[i] = STATE_PLAY_GAME;
		}
	}

	//选择庄
	if(INVALID_CHAIR == m_byBankerUser)
	{ 
		m_byBankerUser = GetFirstBanker(); 
		//第一局确定坐醒玩家
		if (m_tGameRules.byUserCount == 4 && m_byZuoXingUser == INVALID_CHAIR)
		{
			m_byZuoXingUser = GetNextDeskStation(GetNextDeskStation(m_byBankerUser, true), true);
		}
	}

	if (m_tGameRules.piaoFen != 0)
	{
		NoticeSelectPiao(); //通知选漂分
	}
	else
	{
		NoticeSendCard(); //通知发牌
	}
	return true;
}

//替换一张龙牌
void CServerGameDesk::SwapOneLongCard(BYTE chairID, int LongCardIndex)
{
	for (int i = 0; i < m_byLeftCardCount; i++)
	{
		BYTE tmpCard = m_byRepertoryCard[i];
		BYTE tmpCardIndex = m_GameLogic.SwitchToCardIndex(tmpCard);
		for (int j = 0; j < MAX_INDEX; ++j)
		{
			if(m_byCardIndex[chairID][tmpCardIndex] <= 2)
			{
				//替换掉龙牌
				m_byCardIndex[chairID][LongCardIndex]--;
				m_byCardIndex[chairID][tmpCardIndex]++;
				m_byRepertoryCard[i] = m_GameLogic.SwitchToCardData(LongCardIndex);
				return;
			}
		}
	}
}

//检测龙个数
void CServerGameDesk::CheckLongNum()
{
	for(int i = 0;i < PLAY_COUNT;i ++)
	{
		if(STATE_NULL==m_byUserStation[i]) continue;
		for(int j=0;j<MAX_INDEX;j++)
		{
			if(m_byCardIndex[i][j] >= 4)
			{
				SwapOneLongCard(i, j);
			}
		}
	}
}

/*---------------------------------------------------------------------------------*/
//统计玩游戏玩家
BYTE CServerGameDesk::CountPlayer()
{
	BYTE count = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0 && m_byUserStation[i] != STATE_NULL)
		{
			count++;
		}
	}
	return count;
}

//第一局选择庄家
BYTE  CServerGameDesk::GetFirstBanker()
{
	BYTE ret = 0;
	if (m_tGameRules.firstBanker == 0) //随机
	{
		BYTE bySaveDeskIndex[PLAY_COUNT];
		memset(bySaveDeskIndex,0,sizeof(bySaveDeskIndex));
		int iIndex = 0;
		for(BYTE i = 0; i < PLAY_COUNT; i++)
		{
			if(m_byUserStation[i] != STATE_NULL)
			{
				bySaveDeskIndex[iIndex++] = i;
			}
		}
		if (iIndex > 0)
		{
			ret = bySaveDeskIndex[rand() % iIndex];
		}
	}
	else if (m_tGameRules.firstBanker == 1) //房主
	{
		for (BYTE i = 0; i < PLAY_COUNT; i++)
		{
			long userID = GetUserIDByDeskStation(i);
			if (userID == m_masterID && m_byUserStation[i] != STATE_NULL)
			{
				ret = i;
			}
		}
	}

	return ret;
}

//胡牌后设置相关信息
bool CServerGameDesk::SetHuInfor(S_C_GameEnd &tGameEnd)
{
	int iHuIndex = 0;
	int iIndexTemp = 0;
	int basePoint = GetBasePoint();
	if(m_iHuOperation & ACK_CHIHU)			 
	{ 
		iHuIndex = 0; 
	}
	else if(m_iHuOperation & ACT_WANG_DIAO)  
	{ 
		tGameEnd.huOperationType = 1;
		tGameEnd.huOperationFan = 4;
		iHuIndex = 1; 
	}
	else if(m_iHuOperation & ACT_WANG_CHUANG)
	{ 
		tGameEnd.huOperationType = 3;
		tGameEnd.huOperationFan = 8;
		iHuIndex = 2; 
	}
	else if(m_iHuOperation & ACT_SWANG_CHUANG)
	{ 
		tGameEnd.huOperationType = 5;
		tGameEnd.huOperationFan = 16;
		iHuIndex = 3; 
	}
	else if(m_iHuOperation & ACT_WANG_ZHA)	 
	{ 
		iHuIndex = 4; 
	} 

	if (m_byHuUser < 0 || m_byHuUser >= PLAY_COUNT)
	{
		ERROR_LOG("m_byHuUser:%d", m_byHuUser);
		m_byHuUser = 0;
	}

	tGameEnd.byHuCard = m_byOutCardData;										//胡的牌值
	tGameEnd.byXingPai=m_tHuCardInfo[m_byHuUser][iHuIndex].byXing;
	tGameEnd.byWinUser = m_byHuUser;											//胡牌玩家
	tGameEnd.iHuType     = m_tHuCardInfo[m_byHuUser][iHuIndex].iHuType;			//胡牌类型
	//王钓王,客户端显示王钓,服务器按王闯逻辑算
	//王闯王,客户端显示王闯,服务器按王炸逻辑算
	if (KINGVALUE == m_byOutCardData)
	{
		if (m_iHuRealOperation & ACT_SWANG_CHUANG)
		{
			tGameEnd.iHuType &= 0xDFFF;
			tGameEnd.iHuType |= WANG_CHUANG;
			tGameEnd.huOperationType = 4;
			tGameEnd.huOperationFan = 16;
		}
		else if (m_iHuRealOperation & ACT_WANG_CHUANG)
		{
			tGameEnd.iHuType &= 0xEFFF;
			tGameEnd.iHuType |= WANG_DIAO;
			tGameEnd.huOperationType = 2;
			tGameEnd.huOperationFan = 8;
		}
	}
	//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
	if (tGameEnd.iHuType & SWANG_CHUANG)
	{
		tGameEnd.iHuType &= 0xDFFF;
		tGameEnd.iHuType |= WANG_ZHA;
	}
	tGameEnd.byHuXiCount = m_tHuCardInfo[m_byHuUser][iHuIndex].byHuXiCount;		//胡息
	tGameEnd.iXingNum    = m_tHuCardInfo[m_byHuUser][iHuIndex].iXingNum;		//醒牌个数
	tGameEnd.iSumDun     = m_tHuCardInfo[m_byHuUser][iHuIndex].iSumDun;			//囤数
	tGameEnd.iAllDun	 = m_tHuCardInfo[m_byHuUser][iHuIndex].iAllDun;			//总囤数
	tGameEnd.iSumFan	 = m_tHuCardInfo[m_byHuUser][iHuIndex].iSumFan;			//番数
//	tGameEnd.bZiMo       = m_tHuCardInfo[m_byHuUser][iHuIndex].bZiMo;			//是否是自摸
	memcpy(tGameEnd.tWeaveItemArray,m_tHuCardInfo[m_byHuUser][iHuIndex].tWeaveItemArray,sizeof(tGameEnd.tWeaveItemArray));
	//将吃碰杠数据放置结算里面
	int iIndexEnty = 0;
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(0 == tGameEnd.tWeaveItemArray[i].iWeaveKind)
		{
			iIndexEnty = i;
			break;
		}
	}
	
	//将吃碰杠数据放置结算里面
	if(iIndexEnty<MAX_WEAVE)
	{
		tagWeaveItem tAction[7];
		m_GameData.CopyCPTWPData(m_byHuUser,tAction);
		if(m_tHuCardInfo[m_byHuUser][iHuIndex].bWeaveReplace)
		{
			for(int i = 0;i < MAX_WEAVE;i++)
			{
				if(ACK_TI!=m_tHuCardInfo[m_byHuUser][iHuIndex].tWeaveItemArray[i].iWeaveKind)
				{
					continue;
				}
				for(int j=0;j<MAX_WEAVE;j++)
				{
					if(ACK_NULL==tAction[j].iWeaveKind)
					{
						continue;
					}
					if(ACK_WEI!=tAction[j].iWeaveKind && ACK_WEI_EX!=tAction[j].iWeaveKind)
					{
						continue;
					}
					if(m_tHuCardInfo[m_byHuUser][iHuIndex].tWeaveItemArray[i].byCardList[0]==tAction[j].byCardList[0])
					{
						tAction[j].Init();
					}
				}
				
			}
		}
		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(ACK_NULL==tAction[i].iWeaveKind)
			{
				continue;
			}
			if(iIndexEnty>=MAX_WEAVE)
			{
				break;
			}
			tGameEnd.tWeaveItemArray[iIndexEnty++]=tAction[i];
		}
	}

	//王牌代替的牌
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(tGameEnd.tWeaveItemArray[i].iWeaveKind == ACK_WEI && tGameEnd.tWeaveItemArray[i].byCardList[0] == KINGVALUE
			&& tGameEnd.tWeaveItemArray[i].byCardList[1] == KINGVALUE && tGameEnd.tWeaveItemArray[i].byCardList[2] == KINGVALUE
			)
		{
			tGameEnd.byHuXi[i] = 6;
			continue;
		}

		for(int j = 0;j < 4;j ++)
		{
			if(tGameEnd.tWeaveItemArray[i].bIsJing[j])
			{
				tGameEnd.byKingReplaceCard[iIndexTemp ++] = tGameEnd.tWeaveItemArray[i].byCardList[j];
			}
		}
		tGameEnd.byHuXi[i] = m_GameLogic.GetWeaveHuXi(tGameEnd.tWeaveItemArray[i]);
		for(int j = 0;j < 4;j ++)
		{
			if(tGameEnd.tWeaveItemArray[i].bIsJing[j])
			{
				tGameEnd.tWeaveItemArray[i].byCardList[j]=KINGVALUE;
			}
		}
	}

	tGameEnd.iSumDun += tGameEnd.iXingNum;
	tGameEnd.iAllDun = tGameEnd.iSumDun * tGameEnd.iSumFan;
	int xingScore = tGameEnd.iXingNum  * tGameEnd.iSumFan;
	int piaoV[PLAY_COUNT];
	for (int i = 0; i < PLAY_COUNT; ++i)
	 {
		piaoV[i] =m_piaoV[i];
		if (m_piaoV[i] < 0)
		{
			piaoV[i] = 0;
		}
	}
	if (m_tHuCardInfo[m_byHuUser][iHuIndex].bZiMo) //自摸
	{
		xingScore *= 2;
		tGameEnd.iSumFan *= 2;
		tGameEnd.iAllDun = tGameEnd.iSumDun * tGameEnd.iSumFan;
		int iAllWinMoney=0;
		for(int i = 0;i < PLAY_COUNT;i ++)
		{
			if(m_byUserStation[i] != STATE_NULL && i != m_byZuoXingUser && i != m_byHuUser)
			{
				int iLose = tGameEnd.iAllDun + piaoV[i] + piaoV[m_byHuUser];
				if(m_tGameRules.iMaxScore > 0 && iLose > m_tGameRules.iMaxScore)
				{
					iLose = m_tGameRules.iMaxScore;
				}
				tGameEnd.i64GameScore[i] = -iLose;
				tGameEnd.piaoWinLose[i] = (-1) * (piaoV[i] + piaoV[m_byHuUser]) * basePoint;
				tGameEnd.piaoWinLose[m_byHuUser] += (piaoV[i] + piaoV[m_byHuUser]) * basePoint;
				iAllWinMoney += iLose;
			}
		}
		tGameEnd.i64GameScore[m_byHuUser]=iAllWinMoney;
		DivideXingScore(tGameEnd, xingScore);
	}
	else if(255 != m_tHuCardInfo[m_byHuUser][iHuIndex].byBeStation) //点炮
	{
		tGameEnd.huType = 2;
		BYTE byBeStation = m_tHuCardInfo[m_byHuUser][iHuIndex].byBeStation;
		if (byBeStation >= 0 && byBeStation < PLAY_COUNT)
		{
			int iLose = tGameEnd.iAllDun + (piaoV[byBeStation] + piaoV[m_byHuUser]);
			tGameEnd.i64GameScore[byBeStation] = -iLose;
			tGameEnd.i64GameScore[m_byHuUser] = iLose;
			tGameEnd.piaoWinLose[byBeStation] = (-1) * (piaoV[byBeStation] + piaoV[m_byHuUser]) * basePoint;
			tGameEnd.piaoWinLose[m_byHuUser] = (piaoV[byBeStation] + piaoV[m_byHuUser]) * basePoint;

			int iAllWinMoney = 0;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUserStation[i] != STATE_NULL && i != m_byZuoXingUser && i != m_byHuUser)
				{
					int iLose = tGameEnd.iAllDun + piaoV[i];
					if (m_tGameRules.iMaxScore > 0 && iLose > m_tGameRules.iMaxScore)
					{
						iLose = m_tGameRules.iMaxScore;
					}
					tGameEnd.i64GameScore[byBeStation] -= iLose;
					tGameEnd.i64GameScore[m_byHuUser] += iLose;
					tGameEnd.piaoWinLose[byBeStation] -= (-1) * (piaoV[i] + piaoV[m_byHuUser]) * basePoint;
					tGameEnd.piaoWinLose[m_byHuUser] += (piaoV[i] + piaoV[m_byHuUser]) * basePoint;
				}
			}
		}
		DivideXingScore(tGameEnd, xingScore);
	}
	else if (255 == m_tHuCardInfo[m_byHuUser][iHuIndex].byBeStation) //底胡
	{
		tGameEnd.huType = 1;
		int iAllWinMoney = 0;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_byUserStation[i] != STATE_NULL && i != m_byZuoXingUser && i != m_byHuUser)
			{
				int iLose = tGameEnd.iAllDun + piaoV[i] + piaoV[m_byHuUser];
				if (m_tGameRules.iMaxScore > 0 && iLose > m_tGameRules.iMaxScore)
				{
					iLose = m_tGameRules.iMaxScore;
				}
				tGameEnd.i64GameScore[i] = -iLose;
				tGameEnd.piaoWinLose[i] = (-1) * (piaoV[i] + piaoV[m_byHuUser]) * basePoint;
				tGameEnd.piaoWinLose[m_byHuUser] += (piaoV[i] + piaoV[m_byHuUser]) * basePoint;
				iAllWinMoney += iLose;
			}
		}
		tGameEnd.i64GameScore[m_byHuUser] = iAllWinMoney;
		DivideXingScore(tGameEnd, xingScore);
	}

	return true;
}

//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	char logBuf[1024] = { 0 };
	int len = 0;
	len += sprintf(logBuf, "【ACTION】userIDs|");
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if(STATE_PLAY_GAME == m_byUserStation[i])
		{
			len += sprintf(logBuf + len, " %ld ", GetUserIDByDeskStation(i));
		}
		len += sprintf(logBuf + len, "\n");
	}
	FILE_LOG(GAMENAME, logBuf);

	switch (bCloseFlag)
	{
		case GF_NORMAL:		//游戏正常结束
		{
			KillAllTimer();
			KillTimer(TIME_REMAINING);
			//设置数据 
			SetGameStation(GS_WAIT_NEXT);
			////游戏结束
			S_C_GameEnd TGameEnd;
			memcpy(TGameEnd.byLeftCardData,m_byRepertoryCard,sizeof(BYTE) * m_byLeftCardCount);

			if(m_byHuUser >= 0 && m_byHuUser < PLAY_COUNT)
			{
				m_byBankerUser = m_byHuUser;
				SetHuInfor(TGameEnd);
			}
			else
			{
				m_byBankerUser = GetNextDeskStation(m_byBankerUser); //荒庄则下一家做下局庄
			}

			if (m_tGameRules.byUserCount == 4) //4人场
			{
				m_byZuoXingUser =  GetNextDeskStation(GetNextDeskStation(m_byBankerUser, true), true); //下一局坐醒玩家
			}

			if(!IsGoldRoom())
			{
				for(int i = 0;i < PLAY_COUNT;i ++)
				{
					TGameEnd.i64GameScore[i] *= GetBasePoint();
					//大结算也要乘以底注倍数
					m_tZongResult.iEveryScore[i][m_iRunGameCount-1] = TGameEnd.i64GameScore[i];
					m_tZongResult.iMoney[i] += TGameEnd.i64GameScore[i];
					if(TGameEnd.i64GameScore[i]>0)
					{
						m_tZongResult.byWin[i]++;
					}
				}
			}
			else
			{
				for(int i = 0;i < PLAY_COUNT;i ++)
				{
					TGameEnd.i64GameScore[i] = TGameEnd.i64GameScore[i] * ConfigManage()->GetRoomBaseInfo(m_iRoomID)->basePoint;					
				}
				TGameEnd.iBaseFen=ConfigManage()->GetRoomBaseInfo(m_iRoomID)->basePoint;
				TGameEnd.iConSume=TGameEnd.iAllDun*ConfigManage()->GetRoomBaseInfo(m_iRoomID)->basePoint;
			}
			for(int i = 0;i < PLAY_COUNT;i ++)
			{
				TGameEnd.piaoV[i] = m_piaoV[i];
				m_GameLogic.SwitchToCardData(m_byCardIndex[i],TGameEnd.byCardData[i],CountArray(TGameEnd.byCardData[i]),m_byCardIndexJing[i]);
			}

			bool temp_cut[PLAY_COUNT];
			memset(temp_cut,false,sizeof(temp_cut));
			OnJson(&TGameEnd,10);
			if (IsPrivateRoom())
			{
				char szFileName[256];
				char szTempCode[20];
				memset(szTempCode,0,sizeof(szTempCode));
				memset(szFileName,0,sizeof(szFileName));
				GetVideoCode(szTempCode,20);
				sprintf(szFileName,"C:/web/json/%s.json",szTempCode);
				m_fVideoFile.open(szFileName,ios::app);
				m_root["event"]=m_element;
				m_fVideoFile<<m_root.toStyledString()<<endl;
				ChangeUserPointPrivate(TGameEnd.i64GameScore, temp_cut,szTempCode, true);
			}
			else if(IsFriendRoom()){
				char szFileName[256];
				char szTempCode[20];
				memset(szTempCode,0,sizeof(szTempCode));
				memset(szFileName,0,sizeof(szFileName));
				GetVideoCode(szTempCode,20);
				sprintf(szFileName,"C:/web/json/%s.json",szTempCode);
				m_fVideoFile.open(szFileName,ios::app);
				m_root["event"]=m_element;
				m_fVideoFile<<m_root.toStyledString()<<endl;
				ChangeUserPointGoldRoom(TGameEnd.i64GameScore,szTempCode,true);
			}
			else if (IsGoldRoom())
			{
				char szFileName[256];
				char szTempCode[20];
				memset(szTempCode, 0, sizeof(szTempCode));
				memset(szFileName, 0, sizeof(szFileName));
				GetVideoCode(szTempCode, 20);
				sprintf(szFileName, "C:/web/json/%s.json", szTempCode);
				m_fVideoFile.open(szFileName, ios::app);
				m_root["event"] = m_element;
				m_fVideoFile << m_root.toStyledString() << endl;
				//计算抽水
				long long UserPumpGold[PLAY_COUNT] = { 0 };
				memset(UserPumpGold, 0, sizeof(UserPumpGold));
				double rate = double(m_choushui) / 100.0;
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					long long taxValue = 0;
					if (TGameEnd.i64GameScore[i] > 0)
					{
						long long i64WinMoney = TGameEnd.i64GameScore[i]; //记录玩家赢的钱
						taxValue = (long long)floor(rate * i64WinMoney);
						i64WinMoney -= taxValue;
						UserPumpGold[i] = taxValue; //抽水金币数
						TGameEnd.i64GameScore[i] = i64WinMoney; //玩家赢的钱，扣除掉抽水后的
					}
				}
				ChangeUserPoint(TGameEnd.i64GameScore, temp_cut);
			}
			else if (IsFGVIPRoom()){
				//俱乐部vip房间
				char szFileName[256];
				char szTempCode[20];
				memset(szTempCode,0,sizeof(szTempCode));
				memset(szFileName,0,sizeof(szFileName));
				GetVideoCode(szTempCode,20);
				sprintf(szFileName,"C:/web/json/%s.json",szTempCode);
				m_fVideoFile.open(szFileName,ios::app);
				m_root["event"]=m_element;
				m_fVideoFile<<m_root.toStyledString()<<endl;
				ChangeUserFireCoin(TGameEnd.i64GameScore,szTempCode,true); //俱乐部房间（也有回放）
			}


			if(255==m_byHuUser)
			{
				//流局一个王5分
				int iWangFen=5,iWang=0,iWin=0;
				for(int i=0;i<PLAY_COUNT;i++)
				{
					if(STATE_PLAY_GAME!=m_byUserStation[i])
					{
						continue;
					}
					iWang+=m_byCardIndexJing[i];
					if(m_byCardIndexJing[i]<=0)
					{
						iWin++;
					}
				}
				if(iWin>0)
				{
					int iFen=(iWang*iWangFen)/iWin;
					for(int i=0;i<PLAY_COUNT;i++)
					{
						if(STATE_PLAY_GAME!=m_byUserStation[i])
						{
							continue;
						}
						if(m_byCardIndexJing[i]>0)
						{
							TGameEnd.i64GameScore[i]=-m_byCardIndexJing[i]*iWangFen;
						}
						else
						{
							TGameEnd.i64GameScore[i]=iFen;
						}
					}
				}
				for(int i=0;i<PLAY_COUNT;i++)
				{
					if(TGameEnd.i64GameScore[i]>0)
					{
						TGameEnd.i64GameScore[i] *= GetBasePoint();
						TGameEnd.iAllDun=TGameEnd.i64GameScore[i];
					}
					SendGameData(i,&TGameEnd, sizeof(S_C_GameEnd),MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
				}
			}
			else
			{
				BroadcastDeskData(&TGameEnd, sizeof(S_C_GameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
			}

	 		for (BYTE i = 0; i < PLAY_COUNT; ++i)
			{
				if (GetUserIDByDeskStation(i) <= 0) continue;
				if (IsAuto(i))
				{
					m_haveAutoCount[i]++;
					if (m_tGameRules.autoCount > 0 && m_haveAutoCount[i] >= m_tGameRules.autoCount)
					{
						OnHandleUserRequestCancelAuto(i);
					}
				}
				if (IsAuto(i))
				{
					SetTimer(TIME_AUTO_AGREE, 3000);
				}
			}

			ReSetGameState();
			CGameDesk::GameFinish(bDeskStation, bCloseFlag);
			return true;
		}
		case GF_SALE:			//游戏安全结束
		case GFF_SAFE_FINISH:
		{
			KillAllTimer();
			ReSetGameState();
			CGameDesk::GameFinish(bDeskStation, bCloseFlag);
			return true;
		}
		case GFF_FORCE_FINISH:		//用户断线离开
		{
			return true;
		}
	}

	//重置数据
	ReSetGameState();
	CGameDesk::GameFinish(bDeskStation, bCloseFlag);
	return true;
}


///清理所有计时器
void CServerGameDesk::KillAllTimer()
{
	KillTimer(TIME_GAME_BEING_PLAY);				//开始玩游戏
	KillTimer(TIME_OUTCARD_OUTTIME);	            //出牌超时
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		KillTimer(TIME_OPER_OUTTIMER + i);				    //操作超时
	}
	KillTimer(TIME_SEND_NEXT_INTEVAL);				//定时发送下一张牌
	KillTimer(TIME_PIAO);
	KillTimer(TIME_PAO);
	KillTimer(TIME_REMAINING);
}

//桌子成功解散
void CServerGameDesk::OnDeskSuccessfulDissmiss(bool isDismissMidway)
{
	// 计算抽水值(不抽水，总战绩分开计算)
	CGameDesk::SetDeskPercentageScore(m_tZongResult.iMoney);

	BroadcastDeskData(&m_tZongResult, sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, S_C_BIG_GAME_END, 0);
	KillAllTimer();
	KillTimer(TIME_REMAINING);
	//设置数据 
	SetGameStation(GS_WAIT_NEXT);
	ReSetGameState();
	m_tZongResult.Init();
	m_byBankerUser = 255;
	CGameDesk::OnDeskSuccessfulDissmiss(isDismissMidway);
}

// 动态加载配置文件数据
void CServerGameDesk::LoadDynamicConfig()
{
	//加载配置文件
	LoadIni();
	//重新加载配置文件里面的
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);
}

//=====================================================================================
//设置游戏规则
void CServerGameDesk::SetGameRuler()
{
	Json::Reader reader;
	Json::Value vGameRulerValue;
	if (reader.parse(m_szGameRules, vGameRulerValue))
	{
		m_tGameRules.byKingNum = vGameRulerValue["KingNum"].asInt();		//王牌个数
		m_tGameRules.byMinHuXi = vGameRulerValue["MinHuXi"].asInt();		//最小胡息
		m_tGameRules.byUserCount = vGameRulerValue["rs"].asInt();			//游戏人数
		m_tGameRules.bGenXing = vGameRulerValue["GenXing"].asBool();		//跟醒or翻醒
		m_tGameRules.iMaxScore = vGameRulerValue["ShangXian"].asInt();		//积分上限(0:不限制)
		m_tGameRules.iDianPao= vGameRulerValue["DianPao"].asBool();

		m_tGameRules.tunXiRatio = vGameRulerValue["txr"].asInt();
		m_tGameRules.minHuXiTun = vGameRulerValue["mhxt"].asInt();
//		m_tGameRules.biHuType = vGameRulerValue["bht"].asInt();
		m_tGameRules.playType = vGameRulerValue["pt"].asInt();
		m_tGameRules.maoHu = vGameRulerValue["mh"].asBool();
		m_tGameRules.b15Type = vGameRulerValue["b15"].asBool();
		m_tGameRules.piaoFen = vGameRulerValue["pf"].asInt();
		m_tGameRules.firstBanker = vGameRulerValue["fb"].asInt();
		m_tGameRules.autoOverTime = vGameRulerValue["aot"].asInt();
		m_tGameRules.autoCount = vGameRulerValue["ac"].asInt();
		m_tGameRules.bShuangXing = vGameRulerValue["bsx"].asBool();
		m_tGameRules.bWeiVisible = vGameRulerValue["bwv"].asBool();
		m_tGameRules.bXianHuXianFan = vGameRulerValue["bxhxf"].asBool();
	}
	SetFixedRule();
}

bool CServerGameDesk::UserNetCut(GameUserInfo *pUser)
{
	CGameDesk::UserNetCut(pUser);

	if (IsGoldRoom() && IsPlayGame(0) && IsAllUserOffline())
	{
		GameFinish(0, GF_SALE);
	}

	return true;
}

//玩家离开
bool CServerGameDesk::UserLeftDesk(GameUserInfo* pUser)
{
	if(m_byBankerUser == pUser->deskStation) { m_byBankerUser = 0xff; }
	return CGameDesk::UserLeftDesk(pUser);
}
//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if (GS_PLAYGAME == GetGameStation() || GS_PIAO == GetGameStation())
	{
		return true;
	}
	return false;
}

//派发扑克
bool CServerGameDesk::DispatchCardData(BYTE byCurrentUser)
{
	m_GameData.m_bHuJing[byCurrentUser]=false;
	memset(m_GameData.m_bHuTiOrPao,false,sizeof(m_GameData.m_bHuTiOrPao));
	memset(m_GameData.m_bHuWei,false,sizeof(m_GameData.m_bHuWei));
	memset(m_GameData.m_bBiePaoSpecialHu,false,sizeof(m_GameData.m_bBiePaoSpecialHu));
	memset(m_GameData.m_bBiePaoSpecialChi,false,sizeof(m_GameData.m_bBiePaoSpecialChi));
	memset(m_GameData.m_bStartHu,false,sizeof(m_GameData.m_bStartHu));
	//状态效验
	if (byCurrentUser == INVALID_CHAIR) return false;

	//结束判断
	if (m_byLeftCardCount == 0)
	{
		KillAllTimer();
		m_GameData.m_bFinish=true;
		GameFinish(0, GF_NORMAL);
		return true;
	}
	
	//发送扑克
	BYTE bySendCardData =GetPai(byCurrentUser);
	SetXingPai(bySendCardData);
	m_byCurrentCatchUser = byCurrentUser;
	if (bySendCardData == KINGVALUE)
	{
		BYTE tmpByCardIndexJing[PLAY_COUNT];
		memcpy(tmpByCardIndexJing, m_byCardIndexJing, sizeof(m_byCardIndexJing));
		tmpByCardIndexJing[byCurrentUser]++;
		m_GameLogic.SetRealJingNum(tmpByCardIndexJing);
	}

	//设置变量
	m_bDispatch = true;
	m_bEstimate = false;
	m_byCurrentUser = byCurrentUser;
	m_byOutCardUser = byCurrentUser;
	m_byOutCardData = bySendCardData;
	m_byCatchCardUser=byCurrentUser;						
	m_byCatchCardData=bySendCardData;						
	m_byResumeUser = GetNextDeskStation(byCurrentUser);
	AddOutCard(byCurrentUser,bySendCardData);
	bool bUserAction = false;
//--------------------------------------------------------------------------------------------------------
	//执行变量
	bool bPerformAction = false;
	bool bWeiPai = false,bTiPai=false;
	//提牌检测
	IsTiPaiAction(byCurrentUser, bySendCardData,m_bDispatch,bPerformAction,bTiPai);
	//偎牌检测
	IsWeiPaiAction(byCurrentUser, bySendCardData,m_bDispatch,bPerformAction,bWeiPai);
//--------------------------------------------------------------------------------------------------------

	char logBuf[1024] = { 0 };
	int len = 0;
	len += sprintf(logBuf, "【DISPATCH_CARD】 userID:%ld, card:%d, LeftCardCount:%d\n", GetUserIDByDeskStation(m_byCurrentCatchUser), bySendCardData, m_byLeftCardCount);
	FILE_LOG(GAMENAME, logBuf);

	S_C_SendCard tSendCard;
	tSendCard.byAttachUser = byCurrentUser;
	tSendCard.byLeftCardCount = m_byLeftCardCount;
	//发送数据(无提牌或偎牌时才发给客户端数据)
	if(!(bWeiPai || bTiPai))
	{
		tSendCard.byCardData = bySendCardData;
		BroadcastDeskData(&tSendCard, sizeof(S_C_SendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
		OnJson(&tSendCard,6);
	}
	else
	{
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if(byCurrentUser==i)
			{
				tSendCard.byCardData = bySendCardData;
			}
			else
			{
				tSendCard.byCardData = 0;
			}
			SendGameData(i,&tSendCard,sizeof(tSendCard),MSG_MAIN_LOADER_GAME,S_C_SEND_CARD,0);
		}
		tSendCard.byCardData = bySendCardData;
		OnJson(&tSendCard,6);
		bPerformAction = false;
		bWeiPai = false;
		bTiPai=false;
		//提牌检测
		CheckTiPaiAction(byCurrentUser, bySendCardData,m_bDispatch,bPerformAction,bTiPai);
		//偎牌检测
		CheckWeiPaiAction(byCurrentUser, bySendCardData,m_bDispatch,bPerformAction,bWeiPai);
	}
//--------------------------------------------------------------------------------------------------------
	if(bWeiPai || bTiPai)
	{
		if(bTiPai && !IsAllowOutCard(byCurrentUser,false))//提牌构成重跑的条件，下家摸牌
		{
			SendOutCardNotify(byCurrentUser,false);
			return true;
		}
		if(CheckAction(byCurrentUser, 0,3, true))
		{
			bUserAction = true; 
			if(bTiPai)
			{
				m_GameData.m_bHuTiOrPao[byCurrentUser]=true;
			}
			if(bWeiPai)
			{
				m_GameData.m_bHuWei[byCurrentUser]=true;
			}
		}
	}
	else
	{
		m_bTiCard = false;
		BYTE byPaoUser=255;
		if(IsPaoPaiAction(byCurrentUser, bySendCardData,m_bDispatch,bPerformAction,byPaoUser))
		{
			//if(CheckSpecialHuAction(byCurrentUser,bySendCardData,m_bDispatch,bPerformAction,false,false))//有特殊胡
			//{
			//	m_GameData.m_bBiePaoSpecialHu[byCurrentUser]=true;
			//	return true;
			//}
			//if(GetTiPaoNum(byCurrentUser)<=0 && IsPiePaoHuAction(m_byCurrentUser,m_byCatchCardData,m_bDispatch))//有憋跑胡(重跑状态下禁止憋跑胡)
			//{
			//	return true;
			//}
			if(CheckHuPaiAction(byCurrentUser,bySendCardData,m_bDispatch,bPerformAction,false,false,true))
			{
				bUserAction=true;
				if(m_iUserAction[byCurrentUser] & ACK_CHI)
				{
					m_GameData.m_bBiePaoSpecialChi[byCurrentUser]=true;
				}
			}
			else
			{
				//应客户要求对跑牌延时
				SetTimer(TIME_PAO,500);
				bUserAction = true; 
			}
		}
		else
		{
			//检测吃碰胡
			if(CheckAction(byCurrentUser, bySendCardData,0))
			{ 
				if(KINGVALUE==bySendCardData)
				{
					if (m_iUserAction[byCurrentUser] & ACK_CHIHU || m_iUserAction[byCurrentUser] & ACT_WANG_DIAO ||
						m_iUserAction[byCurrentUser] & ACT_WANG_CHUANG || ACT_SWANG_CHUANG & m_iUserAction[byCurrentUser] || m_iUserAction[byCurrentUser] & ACT_WANG_ZHA)
					{
						m_GameData.m_bHuJing[byCurrentUser]=true;
						NoticeWangTakeOut(byCurrentUser, false);
					}
					else
					{
						NoticeWangTakeOut(byCurrentUser, true);
					}
				}
				bUserAction = true; 
			}
			else
			{
				if(KINGVALUE==bySendCardData)
				{
					NoticeWangTakeOut(byCurrentUser, true);
				}
			}
		}
	}
	if(!bUserAction)
	{
		if(bTiPai)
		{
			SendOutCardNotify(m_byCurrentUser,IsAllowOutCard(m_byCurrentUser));
		}
		else if(bWeiPai)
		{
			SendOutCardNotify(m_byCurrentUser,IsAllowOutCard(m_byCurrentUser,true));
		}
		else if(m_GameData.m_tJingData.CheckIsJing(m_byOutCardData))//金牌
		{
			m_byCardIndexJing[byCurrentUser]++;
			SendOutCardNotify(m_byCurrentUser,true);
		}
		else
		{
			SendOutCardNotify(m_byCurrentUser,false);
		}
	}
	//设置标志
	m_bEstimate = true;
	//清除标志位
	m_bTiCard = false;
	m_bSendNotify = false;
	return true;
}

//出牌提示
bool CServerGameDesk::SendOutCardNotify(BYTE byCurrentUser, bool bOutCard)
{
	//效验参数
	if (byCurrentUser == INVALID_CHAIR) return false;
	//过牌判断
	if (bOutCard == true && !HaveOutCard(byCurrentUser))
	{
		m_GameData.m_bForbidOutCard[byCurrentUser]=false;
		S_C_NotifyForbidCard tForbidCard;
		tForbidCard.byDeskStation = byCurrentUser;
		//发送数据
		BroadcastDeskData(&tForbidCard, sizeof(tForbidCard), MSG_MAIN_LOADER_GAME, S_C_FORBIDCARD, 0);
		OnJson(&tForbidCard,14);
		bOutCard = false;
	}
	//设置变量
	m_bOutCard = bOutCard;
	m_byCurrentUser = byCurrentUser;
	//构造数据
	S_C_OutCardNotify tOutCardNotify;
	tOutCardNotify.byCurrentUser = m_byCurrentUser;
	tOutCardNotify.bOutCard = (m_bOutCard == true) ? true : false;
	BYTE byOutPaiTing[MAX_INDEX] = {0};
	BYTE byOutPaiTingCards[MAX_INDEX][MAX_INDEX] = {0};
	if (bOutCard)
	{
//		GetTingData(byCurrentUser, byOutPaiTing, byOutPaiTingCards);
	}
	memcpy(tOutCardNotify.byOutPaiTing, byOutPaiTing,sizeof(tOutCardNotify.byOutPaiTing));
	memcpy(tOutCardNotify.byOutPaiTingCards, byOutPaiTingCards,sizeof(tOutCardNotify.byOutPaiTingCards));
	//发送数据
	BroadcastDeskData(&tOutCardNotify, sizeof(S_C_OutCardNotify), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD_NOTIFY, 0);

	if(tOutCardNotify.bOutCard)		//玩家要出牌
	{
		KillAllTimer();
		UserData userData;
		GetUserData(m_byCurrentUser, userData);
		if (userData.isVirtual)
		{
			m_byRemainingTime = CUtil::GetRandNum()%2+3;
		}
		else if (IsAuto(m_byCurrentUser))
		{
			m_byRemainingTime = 3;
		}
		else
		{
			m_byRemainingTime = m_cfgbyThinkTime;
		}		
		MySetTimer(TIME_OUTCARD_OUTTIME, m_byRemainingTime * 1000);
	}
	else							//继发下一张牌
	{
		KillAllTimer();
		SetTimer(TIME_SEND_NEXT_INTEVAL,1000); 
	}
	return true;
}

//响应判断
bool CServerGameDesk::EstimateUserRespond(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch)
{
	//执行变量
	bool bPerformAction = false;
	bool bWeiPai = false,bTiPai=false;
	//提牌检测
	CheckTiPaiAction(byCenterUser, byCenterCard,bDispatch,bPerformAction,bTiPai);
	//偎牌检测
	CheckWeiPaiAction(byCenterUser, byCenterCard,bDispatch,bPerformAction,bWeiPai);

	//设置标志
	m_bEstimate = true;
	//出牌提示 
	if (m_bSendNotify == true)
	{
		//发送消息
		if (m_bTiCard == false)
		{
			SendOutCardNotify(m_byResumeUser,true);
		}
		else 
		{
			SendOutCardNotify(m_byResumeUser,IsAllowOutCard(m_byResumeUser));
		}
		//设置变量
		m_bTiCard = false;
		m_bSendNotify = false;
		return true;
	}
	m_bEstimate = false;
	//检测跑牌
	if(CheckPaoPaiAction(byCenterUser,byCenterCard,bDispatch,bPerformAction)){ return true; }
	//设置标志
	m_bEstimate = true;
	//检测吃碰胡
	if(CheckCPPaiAction(byCenterUser,byCenterCard,bDispatch,bPerformAction)) { return true; }
	return false;
}

bool CServerGameDesk::CheckAction(BYTE byStation,BYTE byCard,BYTE byType, bool bWeiTi /*=false*/)
{
	//动作变量
	memset(m_bResponse,0,sizeof(m_bResponse));
	memset(m_iUserAction,0,sizeof(m_iUserAction));
	memset(m_byChiCardKind,0,sizeof(m_byChiCardKind));
	memset(m_iPerformAction,0,sizeof(m_iPerformAction));
	memset(m_tChiCardInfo,0,sizeof(m_tChiCardInfo));
	memset(m_tHuCardInfo,0,sizeof(m_tHuCardInfo));
	memset(m_GameData.m_byHuPriority,255,sizeof(m_GameData.m_byHuPriority));
	memset(m_GameData.m_byChiPriority,255,sizeof(m_GameData.m_byChiPriority));
	if(0==byType)//摸牌
	{
		//吃碰判断
		for (BYTE i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	
			{
				continue;
			}
			//吃碰判断
			BYTE cbOperateIndex = m_GameLogic.SwitchToCardIndex(byCard);
			if (m_bMingPao[i] == false)
			{
				//碰牌判断
				if (byStation != i && m_GameLogic.IsPengCard(m_byCardIndex[i], byCard) == true && (m_bAbandonCard1[i][cbOperateIndex] == false))
				{
					m_iUserAction[i] |= ACK_PENG;
				}
				BYTE byEatUser = GetNextDeskStation(byStation);
				//吃牌判断
				if (((byEatUser==i) || (byStation == i)) && (m_bAbandonCard[i][cbOperateIndex] == false) && m_GameLogic.IsChiCard(m_byCardIndex[i],byCard))
				{
					m_iUserAction[i] |= ACK_CHI;
					if(byStation == i)
					{
						m_GameData.m_byChiPriority[0]=i;
					}
					else
					{
						m_GameData.m_byChiPriority[1]=i;
					}
					m_GameLogic.GetActionChiCard(m_byCardIndex[i],m_byOutCardData,m_tChiCardInfo[i].MemeoryEatCard);
					m_tChiCardInfo[i].byChiPai = m_byOutCardData;
				}
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////
		bool bPeopleHu=false;//有人胡牌
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])
			{
				continue;
			}
			//偎牌时只检测当前偎牌人的胡牌 拿得金牌只能自摸     手中有金牌只能自摸
			if((m_GameData.m_tJingData.CheckIsJing(byCard) || m_byCardIndexJing[i] > 0) && (i != byStation)) 
			{
				continue;
			}
			if(!m_GameData.m_bForbidOutCard[i])
			{
				continue;
			}
			//胡牌判断
			bool bZiMo =(i == byStation);
			CountCPTWPXingNumber(i);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(i));
			BYTE byHuCard = byCard;
			bool bAnyHu=false;
			if (m_GameLogic.GetHuCardInfo(i,255,m_byCardIndex[i],m_byCardIndexJing[i],byHuCard,m_byUserHuXiCount[i],m_tHuCardInfo[i],bZiMo,m_GameData.m_tCPTWPArray[i])==true)
			{
				bPeopleHu=true;
				bool bSpecialHu=false;
				for(int j = 1;j < 5;j ++)
				{
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_DIAO))   //王钓
					{ 
						m_iUserAction[i]|= ACT_WANG_DIAO; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo;   
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_CHUANG)) //王闯
					{ 
						m_iUserAction[i]|= ACT_WANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & SWANG_CHUANG)) //三王闯
					{ 
						m_iUserAction[i]|= ACT_SWANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_ZHA))    //王炸
					{ 
						m_iUserAction[i]|= ACT_WANG_ZHA;
						m_tHuCardInfo[i][j].bZiMo = bZiMo;   
						bSpecialHu=true;
					}		
				}
				if(!bSpecialHu)	
				{ 
					m_iUserAction[i] |= ACK_CHIHU; 
					m_tHuCardInfo[i][0].bZiMo = bZiMo; 
				}
			}
		}

		bool bHaveAction = false;
		for(int i = 0;i < PLAY_COUNT;i ++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	
			{
				continue;
			}
			if(m_iUserAction[i] != ACK_NULL)
			{
				bHaveAction = true; 
				break;
			}
		}

		BYTE byTempCenterStation=GetNextDeskStation(byStation);
		BYTE byTempNextStation=GetNextDeskStation(byTempCenterStation);
		if(KINGVALUE!=byCard && m_byLeftCardCount>0 && ACK_NULL==m_iUserAction[byStation] && ACK_NULL==m_iUserAction[byTempNextStation] && m_GameData.m_bForbidOutCard[byTempCenterStation])//检测byTempCenterStation是否有王闯
		{
			for(int i=0;i<PLAY_COUNT;i++)
			{
				m_GameData.m_bTryBigAction[i]=ACK_NULL;
			}
			bool bSpecialHu=false;
			//胡牌判断
			bool bZiMo = true;
			CountCPTWPXingNumber(byTempCenterStation);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(byTempCenterStation));
			BYTE byHuCard = 0;
			bool bAnyHu=false;
			int iHuAction=ACK_NULL;
			tagHuCardInfo tHuCardInfo[5];
			bAnyHu=m_GameLogic.CheckAnyHu(byTempCenterStation,m_byCardIndex[byTempCenterStation],m_byCardIndexJing[byTempCenterStation],byHuCard,m_byUserHuXiCount[byTempCenterStation],tHuCardInfo,bZiMo,&iHuAction,m_GameData.m_tCPTWPArray[byTempCenterStation]);
			if(bAnyHu)
			{
				m_iUserAction[byTempCenterStation] |= iHuAction; 
				m_GameData.m_bTryBigAction[byTempCenterStation]=iHuAction;
				m_byCurrentUser = INVALID_CHAIR;
				//发送消息
				for (int i = 0;i < PLAY_COUNT;i++)
				{
					S_C_OperateNotify tOperateNotify;
					tOperateNotify.byResumeUser = byTempCenterStation;			
					tOperateNotify.byActionCard = byHuCard;				//胡的牌值
					int tmpAction = m_iUserAction[i];
					//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
					if (tmpAction & ACT_SWANG_CHUANG)
					{
						tmpAction &= 0xFBFF;
						tmpAction |= ACT_WANG_ZHA;
					}
					tOperateNotify.iOperateCode = tmpAction;		//玩家可操作的动作
					memcpy(&tOperateNotify.tChiCardInfo,&m_tChiCardInfo[i],sizeof(tOperateNotify.tChiCardInfo));
					SendGameData(i,&tOperateNotify,sizeof(S_C_OperateNotify),MSG_MAIN_LOADER_GAME,S_C_OPERATE_NOTIFY,0);
				}
				KillAllTimer();
				SetTimer(TIME_OPER_OUTTIMER,m_cfgbyThinkTime * 1000);
				return true;
			}
		}

		//响应处理
		if (bHaveAction)
		{
			if(bPeopleHu)
			{
				HuPriority(byStation,m_iUserAction,m_GameData.m_byHuPriority);
			}
			m_byCurrentUser = INVALID_CHAIR;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (STATE_PLAY_GAME != m_byUserStation[i])	continue;
				SendOperateNotify(i, m_byResumeUser, byCard, m_iUserAction[i], m_tChiCardInfo[i]);
			}
			KillAllTimer();
			MySetTimer(TIME_OPER_OUTTIMER, m_cfgbyThinkTime * 1000);

			return true;
		}
	}
	else if(2==byType)//检测最大动作
	{
		for(int i=0;i<PLAY_COUNT;i++)
		{
			m_GameData.m_bTryBigAction[i]=ACK_NULL;
		}
		bool bSpecialHu=false;
		//胡牌判断
		bool bZiMo = true;
		CountCPTWPXingNumber(byStation);
		m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(byStation));
		BYTE byHuCard = byCard;
		bool bAnyHu=false;
		int iHuAction=ACK_NULL;
		tagHuCardInfo tHuCardInfo[5];
		bAnyHu=m_GameLogic.CheckAnyHu(byStation,m_byCardIndex[byStation],m_byCardIndexJing[byStation],byHuCard,m_byUserHuXiCount[byStation],tHuCardInfo,bZiMo,&iHuAction,m_GameData.m_tCPTWPArray[byStation]);
		if(bAnyHu)
		{
			m_iUserAction[byStation]= iHuAction; 
			m_byCurrentUser = INVALID_CHAIR;
			//发送消息
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if(STATE_PLAY_GAME != m_byUserStation[i])	continue;
				SendOperateNotify(i, byStation, byCard, m_iUserAction[i], m_tChiCardInfo[i]);
			}
			KillAllTimer();
			MySetTimer(TIME_OPER_OUTTIMER,m_cfgbyThinkTime * 1000);
			return true;
		}
	}
	else if(3==byType)//检测胡牌
	{
		bool bPeopleHu=false;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i] || i == m_byZuoXingUser)
			{
				continue;
			}
			//偎牌时只检测当前偎牌人的胡牌 拿得精牌只能自摸     手中有精牌只能自摸
			if((m_GameData.m_tJingData.CheckIsJing(byCard) || m_byCardIndexJing[i] > 0) && (i != byStation)) 
			{
				continue;
			}
			if(!m_GameData.m_bForbidOutCard[i])
			{
				continue;
			}
			//胡牌判断
			bool bZiMo = (i == byStation);
			CountCPTWPXingNumber(i);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(i));
			BYTE byHuCard = byCard;
			if (bZiMo && m_GameLogic.GetHuCardInfo(i,255,m_byCardIndex[i],m_byCardIndexJing[i],byHuCard,m_byUserHuXiCount[i],m_tHuCardInfo[i],bZiMo,m_GameData.m_tCPTWPArray[i])==true)
			{
				bPeopleHu=true;
				bool bSpecialHu = false;
				for(int j = 1;j < 5;j ++)
				{
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_DIAO))   //王钓
					{ 
						m_iUserAction[i]|= ACT_WANG_DIAO; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo;   
						bSpecialHu = true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_CHUANG)) //王闯
					{ 
						m_iUserAction[i]|= ACT_WANG_CHUANG;
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu = true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & SWANG_CHUANG)) //三王闯
					{ 
						m_iUserAction[i]|= ACT_SWANG_CHUANG;
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu = true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_ZHA))    //王炸
					{ 
						m_iUserAction[i]|= ACT_WANG_ZHA;
						m_tHuCardInfo[i][j].bZiMo = bZiMo;    
						bSpecialHu = true;
					}		
				}

				if(m_iUserAction[i] == ACK_NULL)	
				{ 
					m_iUserAction[i] |= ACK_CHIHU; 
					m_tHuCardInfo[i][0].bZiMo = bZiMo; 
				}
				//喂牌,提牌后不能王钓,王闯,三王闯,王炸
				if (bWeiTi && bSpecialHu)
				{
					m_iUserAction[i] = ACK_NULL;
				}
			}
		}

		bool bHaveAction = false;
		for(int i = 0;i < PLAY_COUNT;i ++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	
			{
				continue;
			}
			if(m_iUserAction[i] != ACK_NULL)
			{
				bHaveAction = true; 
				break;
			}
		}
		//响应处理
		if (bHaveAction)
		{
			if(bPeopleHu)
			{
				HuPriority(byStation,m_iUserAction,m_GameData.m_byHuPriority);
			}
			m_byCurrentUser = INVALID_CHAIR;

			m_tmpCard = byCard;
			int tmpTime = 100;
			if (bWeiTi)
			{
				tmpTime = 1000;
			}
			MySetTimer(TIME_WEI_TI_DELAY_HU, tmpTime);

			return true;
		}
	}
	return false;
}

//检测当前是否能提牌
bool CServerGameDesk::CheckTiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bTiPai)
{
	bTiPai=false;
	m_byTiOrPaoPaiUser = INVALID_CHAIR;
	//提牌判断(本身有三个了,那摸到一张就变成提牌)
	//bDispatch: true:当前此牌是摸牌  false:当前是牌是玩家出的牌
	//m_bEstimate:
	if ((m_bEstimate == false)&&(bDispatch == true)&&(m_GameLogic.IsTiPaoCard(m_byCardIndex[byCenterUser],byCenterCard) == true))
	{
		//变量定义
		BYTE cbRemoveIndex = m_GameLogic.SwitchToCardIndex(byCenterCard);
		BYTE byRemoveCount = m_byCardIndex[byCenterUser][cbRemoveIndex];
		//删除扑克
		m_byCardIndex[byCenterUser][cbRemoveIndex] = 0;
		//设置组合
		tagWeaveItem tCPGTWPData;
		tCPGTWPData.byCardCount = 4;
		tCPGTWPData.iWeaveKind = ACK_TI;
		tCPGTWPData.byCenterCard = m_byOutCardData;
		tCPGTWPData.byCardList[0] = m_byOutCardData;
		tCPGTWPData.byCardList[1] = m_byOutCardData;
		tCPGTWPData.byCardList[2] = m_byOutCardData;
		tCPGTWPData.byCardList[3] = m_byOutCardData;
		m_GameData.AddToCPTWP(byCenterUser,tCPGTWPData);

		//更新胡息
		UpdateUserHuXiCount(byCenterUser);
		//发送数据
		S_C_UserTiCard tUserTiCard;
		tUserTiCard.byActionUser = byCenterUser;
		tUserTiCard.byActionCard = byCenterCard;
		tUserTiCard.byRemoveCount = byRemoveCount;
		tUserTiCard.byLeftCardCount = m_byLeftCardCount;
		//发送数据
		BroadcastDeskData(&tUserTiCard, sizeof(S_C_UserTiCard), MSG_MAIN_LOADER_GAME, S_C_USER_TI_CARD, 0);
		OnJson(&tUserTiCard,3);
		//设置变量
		byCenterCard = 0;
		bPerformAction = true;
		m_byResumeUser = byCenterUser;
		m_byOutCardUser = INVALID_CHAIR;
		bTiPai=true;
		//出牌提示
		m_bTiCard = true;
		m_bSendNotify = true;
		DeleteOutCard();
	}
	//玩家本身有一个偎牌了,此时玩家再摸到一张
	//bPerformAction: false: 前面没有操作,true:前面存在操作了
	//偎牌变提牌:bDispatch: true:当前此牌是摸牌  false:当前是牌是玩家出的牌
	if ((bPerformAction == false) && (m_bEstimate == false) && (bDispatch == true))
	{
		int iActionNum=m_GameData.GetCPTWPCount(byCenterUser);
		tagWeaveItem tTempWeaveItem[7];
		m_GameData.CopyCPTWPData(byCenterUser,tTempWeaveItem);

		for (BYTE byIndex = 0;byIndex < iActionNum;byIndex++)
		{
			//变量定义
			BYTE cbWeaveKind = tTempWeaveItem[byIndex].iWeaveKind;
			BYTE cbWeaveCard = tTempWeaveItem[byIndex].byCardList[0];
			//转换判断
			if ((byCenterCard != cbWeaveCard)||(cbWeaveKind != ACK_WEI))
			{
				continue;
			}
			m_GameData.DelleteCPTWPData(byCenterUser,cbWeaveKind,cbWeaveCard);
			tagWeaveItem tAddWeaveItem;
			//设置组合
			tAddWeaveItem.byCardCount = 4;
			tAddWeaveItem.iWeaveKind = ACK_TI;
			tAddWeaveItem.byCardList[0] = byCenterCard;
			tAddWeaveItem.byCardList[1] = byCenterCard;
			tAddWeaveItem.byCardList[2] = byCenterCard;
			tAddWeaveItem.byCardList[3] = byCenterCard;
			m_GameData.AddToCPTWP(byCenterUser,tAddWeaveItem);


			//更新胡息
			UpdateUserHuXiCount(byCenterUser);
			//构造数据
			S_C_UserTiCard tUserTiCard;
			tUserTiCard.byRemoveCount = 0;
			tUserTiCard.byActionUser = byCenterUser;
			tUserTiCard.byActionCard = byCenterCard;
			tUserTiCard.byLeftCardCount = m_byLeftCardCount;
			//发送数据
			BroadcastDeskData(&tUserTiCard, sizeof(S_C_UserTiCard), MSG_MAIN_LOADER_GAME, S_C_USER_TI_CARD, 0);
			OnJson(&tUserTiCard,3);
			//设置变量
			//byCenterCard = 0;
			bPerformAction = true;
			m_byResumeUser = byCenterUser;
			m_byOutCardUser = INVALID_CHAIR;
			bTiPai=true;
			//出牌提示
			m_bTiCard = true;
			m_bSendNotify = true;
			DeleteOutCard();
			break;
		}
	}
	return bTiPai;
}

//检测当前是否能偎牌
bool CServerGameDesk::CheckWeiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bWeiPai)
{
	bWeiPai=false;
	//偎牌判断
	if ((bPerformAction == false) && (bDispatch == true) && (m_GameLogic.IsWeiPengCard(m_byCardIndex[byCenterUser],byCenterCard) == true))
	{
		//设置扑克
		m_byCardIndex[byCenterUser][m_GameLogic.SwitchToCardIndex(byCenterCard)] = 0;

		//设置组合
		tagWeaveItem tCPGTWPData;
		tCPGTWPData.byCardCount = 3;
		tCPGTWPData.byCenterCard = byCenterCard;
		tCPGTWPData.byCardList[0] = byCenterCard;
		tCPGTWPData.byCardList[1] = byCenterCard;
		tCPGTWPData.byCardList[2] = byCenterCard;
		if(m_GameLogic.CheckIsHaveCard(m_bAbandonCard1[byCenterUser],byCenterCard))
		{
			tCPGTWPData.iWeaveKind = ACK_WEI_EX;
		}
		else
		{
			tCPGTWPData.iWeaveKind = ACK_WEI;
		}
		m_GameData.AddToCPTWP(byCenterUser,tCPGTWPData);
		m_GameData.m_byWeiPai[m_GameLogic.SwitchToCardIndex(byCenterCard)]++;
		//更新胡息
		UpdateUserHuXiCount(byCenterUser);
		//构造数据
		S_C_UserWeiCard tUserWeiCard;
		tUserWeiCard.byActionUser = byCenterUser;
		tUserWeiCard.byActionCard = byCenterCard;
		tUserWeiCard.iWeiType = tCPGTWPData.iWeaveKind;
		tUserWeiCard.byLeftCardCount = m_byLeftCardCount;
		//发送数据
		BroadcastDeskData(&tUserWeiCard, sizeof(S_C_UserWeiCard), MSG_MAIN_LOADER_GAME, S_C_USER_WEI_CARD, 0);
		OnJson(&tUserWeiCard,4);
		//设置变量
		//byCenterCard = 0;
		bPerformAction = true;
		bWeiPai = true;
		m_byResumeUser = byCenterUser;
		m_byOutCardUser = INVALID_CHAIR;
		//出牌提示
		m_bTiCard = false;
		m_bSendNotify = true;
		DeleteOutCard();
	}
	return bWeiPai;
}

//检测能否胡牌
bool CServerGameDesk::CheckHuPaiAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch,bool &bPerformAction,bool bWeiPai,bool bPao,bool bChiPeng)
{
	//胡牌判断
	//胡牌只有摸到的牌才能胡  
	if (bPao || ((m_bEstimate == false)&&(m_bDispatch == true)))	
	{
		//动作变量
		::memset(m_bResponse,0,sizeof(m_bResponse));
		::memset(m_iUserAction,0,sizeof(m_iUserAction));
		::memset(m_byChiCardKind,0,sizeof(m_byChiCardKind));
		::memset(m_iPerformAction,0,sizeof(m_iPerformAction));
		::memset(m_tHuCardInfo,0,sizeof(m_tHuCardInfo));
		memset(m_tChiCardInfo,0,sizeof(m_tChiCardInfo));
		memset(m_GameData.m_byHuPriority,255,sizeof(m_GameData.m_byHuPriority));
		memset(m_GameData.m_byChiPriority,255,sizeof(m_GameData.m_byChiPriority));
		bool bPeopleHu=false;
		//胡牌判断  
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	
			{
				continue;
			}
			//用户过滤
			if (!bPao && (bDispatch == false) && (i == byCenterUser))
			{
				continue;
			}
			//偎牌时只检测当前偎牌人的胡牌 拿得金牌只能自摸     手中有金牌只能自摸
			if((bWeiPai || m_bTiCard || m_GameData.m_tJingData.CheckIsJing(byCenterCard) || m_byCardIndexJing[i] > 0) && (i != byCenterUser)) 
			{
				continue;
			}
			if(!m_GameData.m_bForbidOutCard[i])
			{
				continue;
			}
			if(bPao && byCenterUser!=i)
			{
				continue;
			}
			if(bChiPeng && byCenterUser==i)
			{
				//碰牌判断
				if (m_GameLogic.IsPengCard(m_byCardIndex[i],byCenterCard))	
				{
					m_iUserAction[i] |= ACK_PENG;
				}
				//吃牌判断
				if (m_GameLogic.IsChiCard(m_byCardIndex[i],byCenterCard))
				{
					m_iUserAction[i] |= ACK_CHI;
					if(byCenterUser == i)
					{
						m_GameData.m_byChiPriority[0]=i;
					}
					else
					{
						m_GameData.m_byChiPriority[1]=i;
					}
					m_GameLogic.GetActionChiCard(m_byCardIndex[i],byCenterCard,m_tChiCardInfo[i].MemeoryEatCard);
					m_tChiCardInfo[i].byChiPai = byCenterCard;
				}
			}
			//胡牌判断
			bool bZiMo =false;
			BYTE byBeStation=255;
			if(bDispatch && m_byCatchCardUser==i)//++
			{
				bZiMo=true;
			}
			CountCPTWPXingNumber(i);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(i));
			BYTE byHuCard = byCenterCard;
			if(bWeiPai || m_bTiCard) byHuCard = 0;
			bool bAnyHu=false;
			if (m_GameLogic.GetHuCardInfo(i,byBeStation,m_byCardIndex[i],m_byCardIndexJing[i],byHuCard,m_byUserHuXiCount[i],m_tHuCardInfo[i],bZiMo,m_GameData.m_tCPTWPArray[i]))
			{
				bPeopleHu=true;
				bool bSpecialHu=false;
				for(int j = 1;j < 5;j ++)
				{
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_DIAO))   //王钓
					{ 
						m_iUserAction[i]|= ACT_WANG_DIAO; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo;  
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_CHUANG)) //王闯
					{ 
						m_iUserAction[i]|= ACT_WANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & SWANG_CHUANG)) //三王闯
					{ 
						m_iUserAction[i]|= ACT_SWANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_ZHA))    //王炸
					{ 
						m_iUserAction[i]|= ACT_WANG_ZHA;
						m_tHuCardInfo[i][j].bZiMo = bZiMo;  
						bSpecialHu=true;
					}		
				}
				if(!bSpecialHu)	
				{ 
					m_iUserAction[i] |= ACK_CHIHU; 
					m_tHuCardInfo[i][0].bZiMo = bZiMo; 
				}
			}
		}
		//响应处理
		if (bPeopleHu)
		{
			HuPriority(byCenterUser,m_iUserAction,m_GameData.m_byHuPriority);
			if(bPao)
			{
				m_GameData.m_bHuTiOrPao[byCenterUser]=true;
			}
			//设置用户
			m_bEstimate = true;
			m_byCurrentUser = INVALID_CHAIR;
			if(m_bTiCard) 
			{ 
				m_byTiOrPaoPaiUser = byCenterUser; 
			}
			m_bSendNotify = false;
			m_byRemainingTime = m_cfgbyThinkTime;
			//发送消息
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if(STATE_PLAY_GAME != m_byUserStation[i])	continue;
				SendOperateNotify(i, m_byResumeUser, byCenterCard, m_iUserAction[i], m_tChiCardInfo[i]);
			}

			KillAllTimer();
			MySetTimer(TIME_OPER_OUTTIMER,m_cfgbyThinkTime * 1000);
			return true;
		}
	}
	return false;
}

//是否有王钓，王闯，三王闯、王炸胡法
bool CServerGameDesk::CheckSpecialHuAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch,bool &bPerformAction,bool bWeiPai,bool bPao)
{
	//胡牌判断
	//胡牌只有摸到的牌才能胡  
	if (bPao || ((m_bEstimate == false)&&(m_bDispatch == true)))	
	{    
		//动作变量
		::memset(m_bResponse,0,sizeof(m_bResponse));
		::memset(m_iUserAction,0,sizeof(m_iUserAction));
		::memset(m_byChiCardKind,0,sizeof(m_byChiCardKind));
		::memset(m_iPerformAction,0,sizeof(m_iPerformAction));
		::memset(m_tHuCardInfo,0,sizeof(m_tHuCardInfo));
		memset(m_GameData.m_byHuPriority,255,sizeof(m_GameData.m_byHuPriority));
		bool bSpecialHu=false;
		//胡牌判断  
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	
			{
				continue;
			}
			//用户过滤
			if (!bPao && (bDispatch == false) && (i == byCenterUser))
			{
				continue;
			}
			//偎牌时只检测当前偎牌人的胡牌 拿得金牌只能自摸     手中有金牌只能自摸
			if((bWeiPai || m_bTiCard || m_GameData.m_tJingData.CheckIsJing(byCenterCard) || m_byCardIndexJing[i] > 0) && (i != byCenterUser)) 
			{
				continue;
			}
			if(!m_GameData.m_bForbidOutCard[i])
			{
				continue;
			}
			if(bPao && byCenterUser!=i)
			{
				continue;
			}
			//胡牌判断
			bool bZiMo =false;
			BYTE byBeStation=255;
			if(bDispatch && m_byCatchCardUser==i)//++
			{
				bZiMo=true;
			}
			CountCPTWPXingNumber(i);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(i));
			BYTE byHuCard = byCenterCard;
			if(bWeiPai || m_bTiCard) byHuCard = 0;
			bool bAnyHu=false;
			if (m_GameLogic.GetHuCardInfo(i,byBeStation,m_byCardIndex[i],m_byCardIndexJing[i],byHuCard,m_byUserHuXiCount[i],m_tHuCardInfo[i],bZiMo,m_GameData.m_tCPTWPArray[i]))
			{
				for(int j = 1;j < 5;j ++)
				{
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_DIAO))   //王钓
					{ 
						m_iUserAction[i]|= ACT_WANG_DIAO; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_CHUANG)) //王闯
					{ 
						m_iUserAction[i]|= ACT_WANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & SWANG_CHUANG)) //三王闯
					{ 
						m_iUserAction[i]|= ACT_SWANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu=true;
					}		
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_ZHA))    //王炸
					{ 
						m_iUserAction[i]|= ACT_WANG_ZHA;
						m_tHuCardInfo[i][j].bZiMo = bZiMo;   
						bSpecialHu=true;
					}		
				}
			}
		}
		//响应处理
		if (bSpecialHu)
		{
			HuPriority(byCenterUser,m_iUserAction,m_GameData.m_byHuPriority);
			if(bPao)
			{
				m_GameData.m_bHuTiOrPao[byCenterUser]=true;
			}
			//设置用户
			m_bEstimate = true;
			m_byCurrentUser = INVALID_CHAIR;
			if(m_bTiCard) 
			{ 
				m_byTiOrPaoPaiUser = byCenterUser; 
			}
			m_bSendNotify = false;
			m_byRemainingTime = m_cfgbyThinkTime;
			//发送消息
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if(STATE_PLAY_GAME != m_byUserStation[i])	continue;
				SendOperateNotify(i, m_byResumeUser, byCenterCard, m_iUserAction[i], m_tChiCardInfo[i]);
			}
			KillAllTimer();
			MySetTimer(TIME_OPER_OUTTIMER,m_cfgbyThinkTime * 1000);
			return true;
		}
	}
	return false;
}
//检测能否跑牌
bool CServerGameDesk::CheckPaoPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction)
{
	m_byTiOrPaoPaiUser = INVALID_CHAIR;
	//跑牌判断
	for (BYTE i = 0;i < PLAY_COUNT;i++)
	{
		if(STATE_PLAY_GAME != m_byUserStation[i])	
		{
			continue;
		}
		//用户过滤
		if (i == byCenterUser) continue;

		//跑牌判断
		if (true == m_GameLogic.IsTiPaoCard(m_byCardIndex[i],byCenterCard))
		{
			//变量定义
			BYTE byActionUser = i;
			BYTE cbRemoveIndex = m_GameLogic.SwitchToCardIndex(byCenterCard);
			BYTE cbRemoveCount = m_byCardIndex[byActionUser][cbRemoveIndex];

			//删除扑克
			m_byCardIndex[byActionUser][cbRemoveIndex]=0;

			//设置组合
			tagWeaveItem tCPGTWPData;
			tCPGTWPData.byCardCount = 4;
			tCPGTWPData.iWeaveKind = ACK_PAO;
			tCPGTWPData.byCenterCard = byCenterCard;
			tCPGTWPData.byCardList[0] = byCenterCard;
			tCPGTWPData.byCardList[1] = byCenterCard;
			tCPGTWPData.byCardList[2] = byCenterCard;
			tCPGTWPData.byCardList[3] = byCenterCard;
			m_GameData.AddToCPTWP(byActionUser,tCPGTWPData);

			//更新胡息  
			UpdateUserHuXiCount(byActionUser);

			//构造数据
			S_C_UserPaoCard tUserPaoCard;
			tUserPaoCard.byActionUser = byActionUser;
			tUserPaoCard.byActionCard = byCenterCard;
			tUserPaoCard.byRemoveCount = cbRemoveCount;
			//发送数据
			BroadcastDeskData(&tUserPaoCard, sizeof(S_C_UserPaoCard), MSG_MAIN_LOADER_GAME, S_C_USER_PAO_CARD, 0);
			OnJson(&tUserPaoCard,5);
			DeleteOutCard();
			//设置变量
			m_byOutCardUser = INVALID_CHAIR;
//			if(15==m_tGameRules.byMinHuXi && !bDispatch)//15胡跑别人打的牌不能胡牌
			if(!bDispatch)//跑别人打的牌不能胡牌
			{
				SendOutCardNotify(byActionUser,IsAllowOutCard(byActionUser));									//出牌提示
				return true;
			}
			if(m_byCardIndexJing[byActionUser]>0 && i != byActionUser)//跑别人打的牌，如果自己手里有王就不能胡牌
			{
				SendOutCardNotify(byActionUser,IsAllowOutCard(byActionUser));									//出牌提示
				return true;
			}

			SetXingPai(byCenterCard);

			if(CheckHuPaiAction(byActionUser, 0,bDispatch,bPerformAction,false,true))
			{
				m_byTiOrPaoPaiUser = byActionUser;
			}
			else 
			{
				SendOutCardNotify(byActionUser,IsAllowOutCard(byActionUser));									//出牌提示
			}
			return true;
		}
	}

	//碰牌或偎牌转换成跑牌
	//跑牌转换
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		int iActionNum=m_GameData.GetCPTWPCount(i);
		tagWeaveItem tTempWeaveItem[7];
		m_GameData.CopyCPTWPData(i,tTempWeaveItem);

		for (BYTE byIndex = 0;byIndex < iActionNum;byIndex++)
		{
			//变量定义
			BYTE byWeaveKind = tTempWeaveItem[byIndex].iWeaveKind;
			BYTE byWeaveCard = tTempWeaveItem[byIndex].byCardList[0];

			//转换判断
			bool bChangeWeave = false;
			if (byCenterCard == byWeaveCard)
			{
				if (byWeaveKind == ACK_WEI) bChangeWeave=true;
				//else if ((byWeaveKind == ACK_PENG) && (byCenterUser == i)) bChangeWeave = true;
				//更正（自己碰的牌可以跑别人摸的牌）
				else if (byWeaveKind == ACK_PENG && bDispatch) bChangeWeave = true;
			}
			if (bChangeWeave == false) continue;

			//变量定义
			BYTE byActionUser = i;
			BYTE byResultWeave = ((byCenterUser == i)&&(byWeaveKind == ACK_WEI)) ? ACK_TI : ACK_PAO;

			m_GameData.DelleteCPTWPData(byActionUser,byWeaveKind,byWeaveCard);
			tagWeaveItem tAddWeaveItem;
			//设置组合
			tAddWeaveItem.byCardCount = 4;
			tAddWeaveItem.iWeaveKind = byResultWeave;
			tAddWeaveItem.byCardList[0] = byCenterCard;
			tAddWeaveItem.byCardList[1] = byCenterCard;
			tAddWeaveItem.byCardList[2] = byCenterCard;
			tAddWeaveItem.byCardList[3] = byCenterCard;
			m_GameData.AddToCPTWP(byActionUser,tAddWeaveItem);

			//明跑判断
			//if ((m_bDispatch == false) && (byResultWeave == ACK_PAO)) m_bMingPao[byCenterUser]=true;

			//更新胡息
			UpdateUserHuXiCount(byActionUser);

			//发送数据
			if (byResultWeave == ACK_TI)
			{
				//提跑数据
				S_C_UserTiCard tUserTiCard;
				tUserTiCard.byRemoveCount = 0;
				tUserTiCard.byActionUser = byActionUser;
				tUserTiCard.byActionCard = byCenterCard;
				//发送数据
				BroadcastDeskData(&tUserTiCard, sizeof(S_C_UserTiCard), MSG_MAIN_LOADER_GAME, S_C_USER_TI_CARD, 0);
				OnJson(&tUserTiCard,3);
				DeleteOutCard();
			}
			else
			{
				//跑牌数据
				S_C_UserPaoCard tUserPaoCard;
				tUserPaoCard.byRemoveCount = 0;
				tUserPaoCard.byActionUser = byActionUser;
				tUserPaoCard.byActionCard = byCenterCard;
				//发送数据
				BroadcastDeskData(&tUserPaoCard, sizeof(S_C_UserPaoCard), MSG_MAIN_LOADER_GAME, S_C_USER_PAO_CARD, 0);
				OnJson(&tUserPaoCard,5);
				DeleteOutCard();
			}
			//设置变量
			m_byOutCardUser = INVALID_CHAIR;
			if(m_byCardIndexJing[byActionUser]>0 && i != byActionUser)//跑别人打的牌，如果自己手里有王就不能胡牌
			{
				SendOutCardNotify(byActionUser,IsAllowOutCard(byActionUser));									//出牌提示
				return true;
			}

			SetXingPai(byCenterCard);

			//更正（跑牌玩家检测胡牌;如果是别人打的牌跑碰的牌不能检测胡牌）
			if(ACK_WEI==byWeaveKind && CheckHuPaiAction(byActionUser,0,bDispatch,bPerformAction,false,true))
			{
				m_byTiOrPaoPaiUser = byActionUser;
				return true; 
			}
			if(ACK_PENG==byWeaveKind && bDispatch && CheckHuPaiAction(byActionUser,0,bDispatch,bPerformAction,false,true))
			{
				m_byTiOrPaoPaiUser = byActionUser;
				return true; 
			}
			SendOutCardNotify(byActionUser,IsAllowOutCard(byActionUser));

			return true;
		}
	}
	return false;
}

//检测能否吃碰杠胡
bool CServerGameDesk::CheckCPPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction)
{
	//动作变量
	memset(m_bResponse,0,sizeof(m_bResponse));
	memset(m_iUserAction,0,sizeof(m_iUserAction));
	memset(m_byChiCardKind,0,sizeof(m_byChiCardKind));
	memset(m_iPerformAction,0,sizeof(m_iPerformAction));
	memset(m_tChiCardInfo,0,sizeof(m_tChiCardInfo));
	memset(m_GameData.m_byHuPriority,255,sizeof(m_GameData.m_byHuPriority));
	memset(m_GameData.m_byChiPriority,255,sizeof(m_GameData.m_byChiPriority));
	//吃碰判断
	for (BYTE i = 0;i < PLAY_COUNT;i++)
	{
		if(STATE_PLAY_GAME != m_byUserStation[i])	
		{
			continue;
		}
		//用户过滤
		if ((bDispatch == false) && (i == byCenterUser)) continue;
		//吃碰判断
		BYTE cbOperateIndex = m_GameLogic.SwitchToCardIndex(byCenterCard);
		if ((m_bMingPao[i] == false))
		{
			//碰牌判断
			if (m_GameLogic.IsPengCard(m_byCardIndex[i], byCenterCard) == true && (m_bAbandonCard1[i][cbOperateIndex] == false))
			{
				m_iUserAction[i] |= ACK_PENG;
			}
			//吃牌判断
			BYTE byEatUser = GetNextDeskStation(byCenterUser);
			if (((byEatUser == i) || (byCenterUser == i)) && (m_bAbandonCard[i][cbOperateIndex] == false) && (m_GameLogic.IsChiCard(m_byCardIndex[i],byCenterCard) == true))
			{
				m_iUserAction[i] |= ACK_CHI;
				if(byCenterUser == i)
				{
					m_GameData.m_byChiPriority[0]=i;
				}
				else
				{
					m_GameData.m_byChiPriority[1]=i;
				}
				m_GameLogic.GetActionChiCard(m_byCardIndex[i],m_byOutCardData,m_tChiCardInfo[i].MemeoryEatCard);
				m_tChiCardInfo[i].byChiPai = m_byOutCardData;
			}
		}
	}

	//检查放炮
	if (m_tGameRules.iDianPao)
	{
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i] || i == byCenterUser)
			{
				continue;
			}

			//手中有金牌只能自摸
			if (m_byCardIndexJing[i] > 0)
			{
				continue;
			}

			if(!m_GameData.m_bForbidOutCard[i])
			{
				continue;
			}
			CountCPTWPXingNumber(i);
			m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(i));
			BYTE byHuCard = byCenterCard;
			bool bZiMo=false;
			if (m_GameLogic.GetHuCardInfo(i, byCenterUser,m_byCardIndex[i],m_byCardIndexJing[i],byHuCard,m_byUserHuXiCount[i],m_tHuCardInfo[i],bZiMo,m_GameData.m_tCPTWPArray[i],true)==true)
			{
				bool bSpecialHu = false;
				for(int j = 1;j < 5;j ++)
				{
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_DIAO))   //王钓
					{ 
						m_iUserAction[i]|= ACT_WANG_DIAO; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo;   
						bSpecialHu = true;
					}
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_CHUANG)) //王闯
					{ 
						m_iUserAction[i]|= ACT_WANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu = true;
					}
					if(0 != (m_tHuCardInfo[i][j].iHuType & SWANG_CHUANG)) //三王闯
					{ 
						m_iUserAction[i]|= ACT_SWANG_CHUANG; 
						m_tHuCardInfo[i][j].bZiMo = bZiMo; 
						bSpecialHu = true;
					}
					if(0 != (m_tHuCardInfo[i][j].iHuType & WANG_ZHA))    //王炸
					{ 
						m_iUserAction[i]|= ACT_WANG_ZHA;
						m_tHuCardInfo[i][j].bZiMo = bZiMo;    
						bSpecialHu = true;
					}
				}
				if(!bSpecialHu)	
				{ 
					m_iUserAction[i] |= ACK_CHIHU; 
					m_tHuCardInfo[i][0].bZiMo = bZiMo; 
				}
			}
		}
	}

	bool bHaveAction = false;
	for(int i = 0;i < PLAY_COUNT;i ++)
	{
		if(ACK_NULL != m_iUserAction[i])
		{
			bHaveAction = true;
			break;
		}
	}
	//响应处理
	if (bHaveAction)
	{
		//设置用户
		m_byCurrentUser = INVALID_CHAIR;
		//发送消息
		for (BYTE i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	continue;
			SendOperateNotify(i, m_byResumeUser, byCenterCard, m_iUserAction[i], m_tChiCardInfo[i]);
		}

		m_byRemainingTime = m_cfgbyThinkTime;
		KillAllTimer();
		MySetTimer(TIME_OPER_OUTTIMER,m_cfgbyThinkTime * 1000);

		return true;
	}
	return false;
}

//出牌判断
bool CServerGameDesk::IsAllowOutCard(BYTE byChairID,bool bHand)
{
	//跑提计算
	BYTE byTiPaoCount = 0;
	if(bHand)
	{
		for(int i = 0;i < MAX_INDEX;i ++)
		{
			if(m_byCardIndex[byChairID][i] >= 4) byTiPaoCount++;
		}
	}
	else
	{
		byTiPaoCount=m_GameData.GetUserPaoNum(byChairID)+m_GameData.GetUserTiNum(byChairID);
		for(int i = 0;i < MAX_INDEX;i ++)
		{
			if(m_byCardIndex[byChairID][i] >= 4) byTiPaoCount++;
		}
	}
	return (byTiPaoCount <= 1) ? true : false;
}
//获取提跑数量
int CServerGameDesk::GetTiPaoNum(BYTE byChairID)
{
	//跑提计算
	int iTiPaoCount = 0;
	iTiPaoCount=m_GameData.GetUserPaoNum(byChairID)+m_GameData.GetUserTiNum(byChairID);
	for(int i = 0;i < MAX_INDEX;i ++)
	{
		if(m_byCardIndex[byChairID][i] >= 4) iTiPaoCount++;
	}
	return iTiPaoCount;
}
//是否有跑牌
bool CServerGameDesk::IsPaoPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,BYTE &byPaoUser)
{
	for (BYTE i = 0;i < PLAY_COUNT;i++)
	{
		if(STATE_PLAY_GAME != m_byUserStation[i])	
		{
			continue;
		}
		//用户过滤
		if (i == byCenterUser) continue;
		//跑牌判断
		if (true == m_GameLogic.IsTiPaoCard(m_byCardIndex[i],byCenterCard))
		{
			byPaoUser=i;
			return true;
		}
	}

	//碰牌或偎牌转换成跑牌
	//跑牌转换
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		int iActionNum=m_GameData.GetCPTWPCount(i);
		tagWeaveItem tTempWeaveItem[7];
		m_GameData.CopyCPTWPData(i,tTempWeaveItem);

		for (BYTE byIndex = 0;byIndex < iActionNum;byIndex++)
		{
			//变量定义
			BYTE byWeaveKind = tTempWeaveItem[byIndex].iWeaveKind;
			BYTE byWeaveCard = tTempWeaveItem[byIndex].byCardList[0];

			//转换判断
			bool bChangeWeave = false;
			if (byCenterCard == byWeaveCard)
			{
				if (byWeaveKind == ACK_WEI) bChangeWeave=true;
				//else if ((byWeaveKind == ACK_PENG) && (byCenterUser == i)) bChangeWeave = true;
				//更正（自己碰的牌可以跑别人摸的牌）
				else if (byWeaveKind == ACK_PENG && bDispatch) bChangeWeave = true;
			}
			if (bChangeWeave == false) continue;
			byPaoUser=i;
			return true;
		}
	}
	return false;
}
//检测当前是否能提牌
bool CServerGameDesk::IsTiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bTiPai)
{
	bTiPai=false;
	//提牌判断(本身有三个了,那摸到一张就变成提牌)
	//bDispatch: true:当前此牌是摸牌  false:当前是牌是玩家出的牌
	//m_bEstimate:
	if ((m_bEstimate == false)&&(bDispatch == true)&&(m_GameLogic.IsTiPaoCard(m_byCardIndex[byCenterUser],byCenterCard) == true))
	{
		bPerformAction = true;
		bTiPai=true;
	}
	//玩家本身有一个偎牌了,此时玩家再摸到一张
	//bPerformAction: false: 前面没有操作,true:前面存在操作了
	//偎牌变提牌:bDispatch: true:当前此牌是摸牌  false:当前是牌是玩家出的牌
	if ((bPerformAction == false) && (m_bEstimate == false) && (bDispatch == true))
	{
		int iActionNum=m_GameData.GetCPTWPCount(byCenterUser);
		tagWeaveItem tTempWeaveItem[7];
		m_GameData.CopyCPTWPData(byCenterUser,tTempWeaveItem);
		for (BYTE byIndex = 0;byIndex < iActionNum;byIndex++)
		{
			//变量定义
			BYTE cbWeaveKind = tTempWeaveItem[byIndex].iWeaveKind;
			BYTE cbWeaveCard = tTempWeaveItem[byIndex].byCardList[0];
			//转换判断
			if ((byCenterCard != cbWeaveCard)||(cbWeaveKind != ACK_WEI))
			{
				continue;
			}
			bPerformAction = true;
			bTiPai=true;
			break;
		}
	}
	return bTiPai;
}

//检测当前是否能偎牌
bool CServerGameDesk::IsWeiPaiAction(BYTE byCenterUser, BYTE &byCenterCard, bool bDispatch,bool &bPerformAction,bool &bWeiPai)
{
	bWeiPai=false;
	//偎牌判断
	if ((bPerformAction == false) && (bDispatch == true) && (m_GameLogic.IsWeiPengCard(m_byCardIndex[byCenterUser],byCenterCard) == true))
	{
		bWeiPai = true;
	}
	return bWeiPai;
}

//撇跑胡
bool CServerGameDesk::IsPiePaoHuAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch)
{
	if(!m_GameData.m_bForbidOutCard[byCenterUser])
	{
		return false;
	}
	BYTE byCardIndex[MAX_INDEX];	//玩家手牌(转换成索引值)
	memcpy(byCardIndex,m_byCardIndex[byCenterUser],sizeof(byCardIndex));
	bool bCanPao=false;
	//碰牌或偎牌转换成跑牌
	//跑牌转换
	int iActionNum=m_GameData.GetCPTWPCount(byCenterUser);
	tagWeaveItem tTempWeaveItem[MAX_WEAVE];
	m_GameData.CopyCPTWPData(byCenterUser,tTempWeaveItem);
	for (BYTE byIndex = 0;byIndex < iActionNum;byIndex++)
	{
		//变量定义
		BYTE byWeaveKind = tTempWeaveItem[byIndex].iWeaveKind;
		BYTE byWeaveCard = tTempWeaveItem[byIndex].byCardList[0];

		//转换判断
		bool bChangeWeave = false;
		if (byCenterCard == byWeaveCard)
		{
			if (byWeaveKind == ACK_WEI) bChangeWeave=true;
			//else if ((byWeaveKind == ACK_PENG) && (byCenterUser == i)) bChangeWeave = true;
			//更正（自己碰的牌可以跑别人摸的牌）
			else if (byWeaveKind == ACK_PENG && bDispatch) bChangeWeave = true;
		}
		if (bChangeWeave == false) continue;
		BYTE byResultWeave = ((byCenterUser == m_byCatchCardUser)&&(byWeaveKind == ACK_WEI)) ? ACK_TI : ACK_PAO;

		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(tTempWeaveItem[i].iWeaveKind == byWeaveKind && (tTempWeaveItem[i].byCardList[0]==byWeaveCard || tTempWeaveItem[i].byCardList[1]==byWeaveCard || tTempWeaveItem[i].byCardList[2]==byWeaveCard 
				|| tTempWeaveItem[i].byCardList[3]==byWeaveCard))
			{
				tTempWeaveItem[i].Init();
				break;
			}
		}

		tagWeaveItem TempCopyGCPData[MAX_WEAVE];
		for(int i=0; i<MAX_WEAVE; i++)
		{
			TempCopyGCPData[i] = tTempWeaveItem[i];
			tTempWeaveItem[i].Init();
		}
		int ActionIndex = 0;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(ACK_NULL != TempCopyGCPData[i].iWeaveKind)
			{
				tTempWeaveItem[ActionIndex++] = TempCopyGCPData[i];
			}
		}

		tagWeaveItem tAddWeaveItem;
		//设置组合
		tAddWeaveItem.byCardCount = 4;
		tAddWeaveItem.iWeaveKind = byResultWeave;
		tAddWeaveItem.byCardList[0] = byCenterCard;
		tAddWeaveItem.byCardList[1] = byCenterCard;
		tAddWeaveItem.byCardList[2] = byCenterCard;
		tAddWeaveItem.byCardList[3] = byCenterCard;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(ACK_NULL==tTempWeaveItem[i].iWeaveKind)
			{
				tTempWeaveItem[i]=tAddWeaveItem;
				break;
			}
		}
		bCanPao=true;
		break;
	}
	if(!bCanPao)
	{
		return false;
	}
	BYTE byTempUserHuXiCount=0;			
	for (BYTE i = 0;i < MAX_WEAVE;i++)
	{
		if(ACK_NULL == tTempWeaveItem[i].iWeaveKind)
		{
			continue;
		}
		byTempUserHuXiCount += m_GameLogic.GetWeaveHuXi(tTempWeaveItem[i]);
	}
	tagHuCardInfo tTempHuCardInfo[5];
	bool bPeopleHu=false;
	int iXingCount=0;
	for (BYTE i = 0;i<MAX_WEAVE;i++)
	{
		if(ACK_NULL == tTempWeaveItem[i].iWeaveKind)
		{
			continue;
		}
		for(int j = 0;j < 4;j ++)
		{
			if(m_byXingPai == tTempWeaveItem[i].byCardList[j])
			{
				iXingCount ++;
			}
		}
	}

	int iRed = 0;
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_NULL == tTempWeaveItem[i].iWeaveKind)
		{
			continue;
		}
		for(int j = 0;j < 4;j ++)
		{
			BYTE byCard=tTempWeaveItem[i].byCardList[j];
			if(0==byCard)
			{
				continue;
			}
			BYTE byCardValue =  (byCard & MASK_VALUE);
			if(0x02 == byCardValue || 0x07 == byCardValue || 0x0A == byCardValue)
			{
				iRed++;
			}
		}
	}

	m_GameLogic.SetCPTWPXingNum(iXingCount);
	m_GameLogic.SetXingPai(m_byXingPai);
	m_GameLogic.SetCPGTWPRedCardNum(iRed);
	BYTE byHuCard = 0;
	bool bZimo=(byCenterUser == m_byCatchCardUser)?true:false;
	if (m_GameLogic.GetHuCardInfo(byCenterUser,255,byCardIndex,m_byCardIndexJing[byCenterUser],byHuCard,byTempUserHuXiCount,tTempHuCardInfo,bZimo,tTempWeaveItem))
	{
		//动作变量
		::memset(m_bResponse,0,sizeof(m_bResponse));
		::memset(m_iUserAction,0,sizeof(m_iUserAction));
		::memset(m_byChiCardKind,0,sizeof(m_byChiCardKind));
		::memset(m_iPerformAction,0,sizeof(m_iPerformAction));
		::memset(m_tHuCardInfo,0,sizeof(m_tHuCardInfo));
		memset(m_GameData.m_byHuPriority,255,sizeof(m_GameData.m_byHuPriority));
		m_byCurrentUser = INVALID_CHAIR;
		bool bPeopleHu=true;
		m_iUserAction[byCenterUser] |= ACK_CHIHU; 
		m_tHuCardInfo[byCenterUser][0].bZiMo = bZimo; 
		if(bPeopleHu)
		{
			HuPriority(byCenterUser,m_iUserAction,m_GameData.m_byHuPriority);
		}

		//发送消息
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if(STATE_PLAY_GAME != m_byUserStation[i])	continue;
			SendOperateNotify(i, m_byResumeUser, byCenterCard, m_iUserAction[i], m_tChiCardInfo[i]);
		}
		KillAllTimer();
		MySetTimer(TIME_OPER_OUTTIMER,m_cfgbyThinkTime * 1000);
		return true;
	}
	return false;
}

//撇跑胡
bool CServerGameDesk::CheckPiePaoHuAction(BYTE byCenterUser, BYTE byCenterCard, bool bDispatch)
{
	BYTE byCardIndex[MAX_INDEX];	//玩家手牌(转换成索引值)
	memcpy(byCardIndex,m_byCardIndex[byCenterUser],sizeof(byCardIndex));
	bool bCanPao=false;
	//碰牌或偎牌转换成跑牌
	//跑牌转换
	int iActionNum=m_GameData.GetCPTWPCount(byCenterUser);
	tagWeaveItem tTempWeaveItem[MAX_WEAVE];
	m_GameData.CopyCPTWPData(byCenterUser,tTempWeaveItem);
	for (BYTE byIndex = 0;byIndex < iActionNum;byIndex++)
	{
		//变量定义
		BYTE byWeaveKind = tTempWeaveItem[byIndex].iWeaveKind;
		BYTE byWeaveCard = tTempWeaveItem[byIndex].byCardList[0];

		//转换判断
		bool bChangeWeave = false;
		if (byCenterCard == byWeaveCard)
		{
			if (byWeaveKind == ACK_WEI) bChangeWeave=true;
			//else if ((byWeaveKind == ACK_PENG) && (byCenterUser == i)) bChangeWeave = true;
			//更正（自己碰的牌可以跑别人摸的牌）
			else if (byWeaveKind == ACK_PENG && bDispatch) bChangeWeave = true;
		}
		if (bChangeWeave == false) continue;
		BYTE byResultWeave = ((byCenterUser == m_byCatchCardUser)&&(byWeaveKind == ACK_WEI)) ? ACK_TI : ACK_PAO;

		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(tTempWeaveItem[i].iWeaveKind == byWeaveKind && (tTempWeaveItem[i].byCardList[0]==byWeaveCard || tTempWeaveItem[i].byCardList[1]==byWeaveCard || tTempWeaveItem[i].byCardList[2]==byWeaveCard 
				|| tTempWeaveItem[i].byCardList[3]==byWeaveCard))
			{
				tTempWeaveItem[i].Init();
				break;
			}
		}

		tagWeaveItem TempCopyGCPData[MAX_WEAVE];
		for(int i=0; i<MAX_WEAVE; i++)
		{
			TempCopyGCPData[i] = tTempWeaveItem[i];
			tTempWeaveItem[i].Init();
		}
		int ActionIndex = 0;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(ACK_NULL != TempCopyGCPData[i].iWeaveKind)
			{
				tTempWeaveItem[ActionIndex++] = TempCopyGCPData[i];
			}
		}

		tagWeaveItem tAddWeaveItem;
		//设置组合
		tAddWeaveItem.byCardCount = 4;
		tAddWeaveItem.iWeaveKind = byResultWeave;
		tAddWeaveItem.byCardList[0] = byCenterCard;
		tAddWeaveItem.byCardList[1] = byCenterCard;
		tAddWeaveItem.byCardList[2] = byCenterCard;
		tAddWeaveItem.byCardList[3] = byCenterCard;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			if(ACK_NULL==tTempWeaveItem[i].iWeaveKind)
			{
				tTempWeaveItem[i]=tAddWeaveItem;
				break;
			}
		}
		bCanPao=true;
		break;
	}
	if(!bCanPao)
	{
		return false;
	}
	BYTE byTempUserHuXiCount=0;			
	for (BYTE i = 0;i < MAX_WEAVE;i++)
	{
		if(ACK_NULL == tTempWeaveItem[i].iWeaveKind)
		{
			continue;
		}
		byTempUserHuXiCount += m_GameLogic.GetWeaveHuXi(tTempWeaveItem[i]);
	}
	tagHuCardInfo tTempHuCardInfo[5];
	bool bPeopleHu=false;
	int iXingCount=0;
	for (BYTE i = 0;i<MAX_WEAVE;i++)
	{
		if(ACK_NULL == tTempWeaveItem[i].iWeaveKind)
		{
			continue;
		}
		for(int j = 0;j < 4;j ++)
		{
			if(m_byXingPai == tTempWeaveItem[i].byCardList[j])
			{
				iXingCount ++;
			}
		}
	}

	int iRed = 0;
	for(int i = 0;i < MAX_WEAVE;i ++)
	{
		if(ACK_NULL == tTempWeaveItem[i].iWeaveKind)
		{
			continue;
		}
		for(int j = 0;j < 4;j ++)
		{
			BYTE byCard=tTempWeaveItem[i].byCardList[j];
			if(0==byCard)
			{
				continue;
			}
			BYTE byCardValue =  (byCard & MASK_VALUE);
			if(0x02 == byCardValue || 0x07 == byCardValue || 0x0A == byCardValue)
			{
				iRed++;
			}
		}
	}
	
	m_GameLogic.SetCPTWPXingNum(iXingCount);
	m_GameLogic.SetXingPai(m_byXingPai);
	m_GameLogic.SetCPGTWPRedCardNum(iRed);
	BYTE byHuCard = 0;
	bool bZimo=(byCenterUser == m_byCatchCardUser)?true:false;
	if (m_GameLogic.GetHuCardInfo(byCenterUser,255,byCardIndex,m_byCardIndexJing[byCenterUser],byHuCard,byTempUserHuXiCount,tTempHuCardInfo,bZimo,tTempWeaveItem))
	{
		tTempHuCardInfo[0].bZiMo = bZimo; 
		memcpy(m_tHuCardInfo[byCenterUser],tTempHuCardInfo,sizeof(m_tHuCardInfo[byCenterUser]));
		memcpy(m_GameData.m_tCPTWPArray[byCenterUser],tTempWeaveItem,sizeof(m_GameData.m_tCPTWPArray[byCenterUser]));
		m_byUserHuXiCount[byCenterUser]=byTempUserHuXiCount;
		return true;
	}
	return false;
}
//更新胡息
bool CServerGameDesk::UpdateUserHuXiCount(BYTE byChairID)
{
	//胡息计算
	BYTE bUserHuXiCount = 0;
	int iActionNum=m_GameData.GetCPTWPCount(byChairID);
	tagWeaveItem TempCPTWP[7];
	m_GameData.CopyCPTWPData(byChairID,TempCPTWP);
	for (BYTE i = 0;i < iActionNum;i++)
	{
		bUserHuXiCount += m_GameLogic.GetWeaveHuXi(TempCPTWP[i]);
	}
	//设置胡息
	m_byUserHuXiCount[byChairID] = bUserHuXiCount;
	for(int i=0;i<PLAY_COUNT;i++)
	{
		S_C_UpdataHuXi tUpdataHuXi;
		tUpdataHuXi.byUserHuXi[i]=m_byUserHuXiCount[i];
		for(int j=0;j<PLAY_COUNT;j++)
		{
			if(i==j)
			{
				continue;
			}
			tagWeaveItem OtherCPTWP[MAX_WEAVE];
			int iTempAction=m_GameData.CopyCPTWPData(j,OtherCPTWP);
			for (int iAction = 0;iAction < iTempAction;iAction++)
			{
				tUpdataHuXi.byUserHuXi[j]+=m_GameLogic.GetWeaveHuXi(OtherCPTWP[iAction],true);
			}
		}
		SendGameData(i,&tUpdataHuXi, sizeof(S_C_UpdataHuXi),MSG_MAIN_LOADER_GAME, S_C_UPDATA_HUXI, 0);
	}

	S_C_UpdataHuXi tUpdataHuXi;
	memcpy(tUpdataHuXi.byUserHuXi,m_byUserHuXiCount,sizeof(m_byUserHuXiCount));
	OnJson(&tUpdataHuXi,11);
	return true;
}
//增加出牌数据
void CServerGameDesk::AddOutCard(BYTE byChairID,BYTE byCard)
{
	if(!m_GameData.m_tJingData.CheckIsJing(byCard))
	{
		if(byChairID < PLAY_COUNT && m_GameLogic.IsValidCard(byCard))
		{
			if(m_iOutCardCount[byChairID] < OUT_CARD_NUM)
			{
				m_byUserOutCard[byChairID][m_iOutCardCount[byChairID] ++] = byCard;
				m_tOutCard.byChairID = byChairID;
				m_tOutCard.byOutCard = byCard;
			}
		}
	}
	else
	{
		m_tOutCard.Clear();
	}
}

//删除牌
void CServerGameDesk::DeleteOutCard()
{
	if(m_tOutCard.byChairID < PLAY_COUNT && m_GameLogic.IsValidCard(m_tOutCard.byOutCard))
	{
		if(m_iOutCardCount[m_tOutCard.byChairID] > 0)
		{
			m_byUserOutCard[m_tOutCard.byChairID][m_iOutCardCount[m_tOutCard.byChairID] - 1] = 0;
			m_iOutCardCount[m_tOutCard.byChairID] --;
			m_tOutCard.Clear();
		}
	}
}
//设置醒牌
void CServerGameDesk::SetXingPai(BYTE byCard)
{
	if(m_tGameRules.bGenXing) //跟醒
	{
		m_byXingPai = byCard;
	}
	else	//翻醒
	{
		int iCardIndex=m_byLeftCardCount;
		if(iCardIndex>0)
		{
			m_byXingPai = m_byRepertoryCard[--iCardIndex];
		}
		else
		{
			m_byXingPai = byCard;
		}
	}
}
//统计醒牌个数
int  CServerGameDesk::CountCPTWPXingNumber(BYTE byChairID)
{
	int iXingCount=m_GameData.GetCPTWPXing(byChairID,m_byXingPai);
	m_GameLogic.SetCPTWPXingNum(iXingCount);
	m_GameLogic.SetXingPai(m_byXingPai);
	return iXingCount;
}

//获取一张牌
BYTE CServerGameDesk::GetPai(BYTE byStation)
{
	//结束判断
	if (m_byLeftCardCount<=0)
	{
		return 0;
	}

	//超端摸牌时，将设置的牌换到最上面
	if (m_superSetCard != 255 && byStation == m_superSetChairID)
	{
		for (int i = 0; i < m_byLeftCardCount; ++i)
		{
			if (m_byRepertoryCard[i] == m_superSetCard)
			{
				m_byRepertoryCard[i] = m_byRepertoryCard[m_byLeftCardCount - 1];
				m_byRepertoryCard[m_byLeftCardCount - 1] = m_superSetCard;
				m_superSetCard = 255;
				m_superSetChairID = 255;
				break;
			}
		}
	}

	return m_byRepertoryCard[--m_byLeftCardCount];
}

//是否有出牌
bool CServerGameDesk::HaveOutCard(BYTE byStation)
{
	for (int i = 0;i < MAX_INDEX;i++)
	{
		if ((m_byCardIndex[byStation][i] > 0) && (m_byCardIndex[byStation][i] < 3)) 
		{
			return true;
		}
	}
	return false;
}

void CServerGameDesk::HuPriority(BYTE byHuStation,int iUserAction[],BYTE byPri[])
{
	BYTE byde[PLAY_COUNT];
	memset(byde,255,sizeof(byde));
	int index=0;
	BYTE byStation=byHuStation;
	for(int ci=0;ci<PLAY_COUNT;ci++)
	{
		if((iUserAction[byStation] & ACK_CHIHU) || (iUserAction[byStation] & ACT_WANG_DIAO) || (iUserAction[byStation] & ACT_WANG_CHUANG)
			|| (iUserAction[byStation] & ACT_SWANG_CHUANG) || (iUserAction[byStation] & ACT_WANG_ZHA))
		{
			byde[index++]=byStation;
		}
		byStation=GetNextDeskStation(byStation);
	}
	memcpy(byPri,byde,sizeof(byde));
}

//json存储
void CServerGameDesk::OnJson(void *pData,int iType)
{
	if(IsGoldRoom())
	{
		return;
	}
	if(1==iType)
	{
		S_C_SendCardData *pChangeData=(S_C_SendCardData *)pData;
		if(!pChangeData)
		{
			return;
		}
		m_root["gameID"]= m_pDataManage->m_InitData.uNameID;
		m_root["byBankerUser"]=pChangeData->byBankerUser;
		m_root["byLeftCardCount"]=pChangeData->byLeftCardCount;
		m_root["bRoomPassward"]=m_szDeskPassWord;
		for(int i=0;i<PLAY_COUNT;i++)
		{
			int iUserID=GetUserIDByDeskStation(i);
			if(iUserID<=0)
			{
				continue;
			}
			if(m_masterID==iUserID)
			{
				m_root["bRoomMasterName"]=m_pDataManage->GetUser(iUserID)->name;
				break;
			}
		}
		m_root["bCurPlayCount"]=m_iRunGameCount;
		m_root["bAllPlayCount"]=m_iVipGameCount;
		Json::Value _index;
		for(int i=0;i<PLAY_COUNT;i++)
		{
			int iUserID=GetUserIDByDeskStation(i);
			if(iUserID<=0)
			{
				continue;
			}
			char key[10];
			memset(key,0,sizeof(key));
			sprintf(key,"%d",i);
			Json::Value _info;
			_info["nickName"]=Json::Value(m_pDataManage->GetUser(iUserID)->name);
			_info["headURL"]=Json::Value(m_pDataManage->GetUser(iUserID)->headURL);
			_info["score"]=Json::Value((int)0);
			_info["bSex"]=Json::Value(m_pDataManage->GetUser(iUserID)->sex);
			_info["userID"]=Json::Value(iUserID);
			_index[key]=Json::Value(_info);
		}
		m_root["user"]=_index;
		Json::Value array_1;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			char szkey[10];
			memset(szkey,0,sizeof(szkey));
			sprintf(szkey,"%d",i);
			Json::Value array_2;
			for(int j=0;j<MAX_COUNT;j++)
			{
				char key_2[10];
				memset(key_2,0,sizeof(key_2));
				sprintf(key_2,"%d",j);
				array_2[key_2]=Json::Value(pChangeData->byCardData[i][j]);
			}
			array_1[szkey]=Json::Value(array_2);
		}
		m_root["handArrData"]=array_1;

		//游戏玩法
		m_root["byKingNum"]=m_tGameRules.byKingNum;
		m_root["byMinHuXi"]=m_tGameRules.byMinHuXi;
		m_root["byUserCount"]=m_tGameRules.byUserCount;
		m_root["bGenXing"]=m_tGameRules.bGenXing;
		m_root["iMaxScore"]=m_tGameRules.iMaxScore;
		m_root["iDianPao"]=m_tGameRules.iDianPao;

		m_root["tunXiRatio"]=m_tGameRules.tunXiRatio;
		m_root["minHuXiTun"]=m_tGameRules.minHuXiTun;
		m_root["playType"]=m_tGameRules.playType;
		m_root["maoHu"]=m_tGameRules.maoHu;
		m_root["b15Type"]=m_tGameRules.b15Type;
		m_root["piaoFen"]=m_tGameRules.piaoFen;
		m_root["firstBanker"]=m_tGameRules.firstBanker;
		m_root["autoOverTime"]=m_tGameRules.autoOverTime;
		m_root["autoCount"]=m_tGameRules.autoCount;
		m_root["bShuangXing"]=m_tGameRules.bShuangXing;

		m_root["zuoXingUser"] = m_byZuoXingUser;
	}
	else if(2==iType)//亮牌
	{
		S_C_ShowCard *pChangeData=(S_C_ShowCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("ShowCard");
		element["bUser"]=Json::Value(pChangeData->byShowUser);
		element["cardValue"]=Json::Value(pChangeData->byCard);
		element["byLeftCardCount"]=Json::Value(pChangeData->byLeftCardCount);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(3==iType)//提牌
	{
		S_C_UserTiCard *pChangeData=(S_C_UserTiCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("Ti");
		element["bUser"]=Json::Value(pChangeData->byActionUser);
		element["cardValue"]=Json::Value(pChangeData->byActionCard);
		element["byRemoveCount"]=Json::Value(pChangeData->byRemoveCount);
		element["byLeftCardCount"]=Json::Value(pChangeData->byLeftCardCount);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(4==iType)//偎牌
	{
		S_C_UserWeiCard *pChangeData=(S_C_UserWeiCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("Wei");
		element["bUser"]=Json::Value(pChangeData->byActionUser);
		element["cardValue"]=Json::Value(pChangeData->byActionCard);
		element["iWeiType"]=Json::Value(pChangeData->iWeiType);
		element["byLeftCardCount"]=Json::Value(pChangeData->byLeftCardCount);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(5==iType)//跑牌
	{
		S_C_UserPaoCard *pChangeData=(S_C_UserPaoCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("Pao");
		element["bUser"]=Json::Value(pChangeData->byActionUser);
		element["cardValue"]=Json::Value(pChangeData->byActionCard);
		element["byRemoveCount"]=Json::Value(pChangeData->byRemoveCount);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(6==iType)//摸牌
	{
		S_C_SendCard *pChangeData=(S_C_SendCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("SendCard");
		element["bUser"]=Json::Value(pChangeData->byAttachUser);
		element["cardValue"]=Json::Value(pChangeData->byCardData);
		element["byLeftCardCount"]=Json::Value(pChangeData->byLeftCardCount);
		element["iOperateCode"]=Json::Value(pChangeData->iOperateCode);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(7==iType)//出牌结果
	{
		S_C_OutCard *pChangeData=(S_C_OutCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("OutCard");
		element["bUser"]=Json::Value(pChangeData->byOutCardUser);
		element["cardValue"]=Json::Value(pChangeData->byOutCardData);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(8==iType)//吃牌结果
	{
		S_C_UserChiCard *pChangeData=(S_C_UserChiCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("ChiAction");
		element["bUser"]=Json::Value(pChangeData->byActionUser);
		element["cardValue"]=Json::Value(pChangeData->byActionCard);

		BYTE byChiData[9];
		memset(byChiData,0,sizeof(byChiData));
		int iChiIndex=0;
		for (BYTE k = 0;k < 3;k++)
		{
			if(!m_GameLogic.IsValidCard(pChangeData->byCardData[k][0]))
			{
				continue;
			}
			byChiData[iChiIndex++] = pChangeData->byCardData[k][0];
			byChiData[iChiIndex++] = pChangeData->byCardData[k][1];
			byChiData[iChiIndex++] = pChangeData->byCardData[k][2];
		}
		Json::Value array_chiList;
		for(int i=0;i<9;i++)
		{
			if(!m_GameLogic.IsValidCard(byChiData[i]))
			{
				continue;
			}
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_chiList[key_1]=Json::Value(byChiData[i]);
		}
		element["chiList"]=Json::Value(array_chiList);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(9==iType)//碰牌结果
	{
		S_C_UserPengCard *pChangeData=(S_C_UserPengCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("PengAction");
		element["bUser"]=Json::Value(pChangeData->byActionUser);
		element["cardValue"]=Json::Value(pChangeData->byActionCard);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(10==iType)//结算
	{
		S_C_GameEnd *pChangeData=(S_C_GameEnd *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("Result");
		element["byHuCard"]=Json::Value(pChangeData->byHuCard);
		element["byWinUser"]=Json::Value(pChangeData->byWinUser);
		element["byHuXiCount"]=Json::Value(pChangeData->byHuXiCount);
		element["iXingNum"]=Json::Value(pChangeData->iXingNum);
		element["iSumDun"]=Json::Value(pChangeData->iSumDun);
		element["iSumFan"]=Json::Value(pChangeData->iSumFan);
		element["iAllDun"]=Json::Value(pChangeData->iAllDun);
		element["iHuType"]=Json::Value(pChangeData->iHuType);
//		element["bZiMo"]=Json::Value(pChangeData->bZiMo);
		element["bZiMo"]=0;
		element["byXingPai"]=Json::Value(pChangeData->byXingPai);
		element["iBaseFen"]=Json::Value(pChangeData->iBaseFen);
		element["iConSume"]=Json::Value(pChangeData->iConSume);

		element["huType"]=Json::Value(pChangeData->huType);	//胡类型(0:自摸  1:底胡  2:点炮)

		Json::Value array_ReplaceList;
		for(int i=0;i<4;i++)
		{
			if(!m_GameLogic.IsValidCard(pChangeData->byKingReplaceCard[i]))
			{
				continue;
			}
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_ReplaceList[key_1]=Json::Value(pChangeData->byKingReplaceCard[i]);
		}
		element["byKingReplaceCard"]=Json::Value(array_ReplaceList);

		Json::Value array_LeftList;
		for(int i=0;i<50;i++)
		{
			if(!m_GameLogic.IsValidCard(pChangeData->byLeftCardData[i]))
			{
				continue;
			}
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_LeftList[key_1]=Json::Value(pChangeData->byLeftCardData[i]);
		}
		element["byLeftCardData"]=Json::Value(array_LeftList);

		Json::Value array_HuXiList;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_HuXiList[key_1]=Json::Value(pChangeData->byHuXi[i]);
		}
		element["byHuXi"]=Json::Value(array_HuXiList);

		Json::Value array_GameScoreList;
		Json::Value array_piaoV;
		Json::Value array_cards;
		Json::Value array_piaoWinLose;
		for(int i=0;i<PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_GameScoreList[key_1]=Json::Value((int)pChangeData->i64GameScore[i]);
			array_piaoV[key_1]=Json::Value(pChangeData->piaoV[i]);
			array_piaoWinLose[key_1]=Json::Value(pChangeData->piaoWinLose[i]);

			Json::Value array_2;
			for (int j = 0; j<MAX_COUNT; j++)
			{
				char key_2[10];
				memset(key_2, 0, sizeof(key_2));
				sprintf(key_2, "%d", j);
				array_2[key_2] = Json::Value(pChangeData->byCardData[i][j]);
			}
			array_cards[key_1] = Json::Value(array_2);
		}
		element["i64GameScore"]=Json::Value(array_GameScoreList);
		element["piaoV"]=Json::Value(array_piaoV);
		element["cardData"]=Json::Value(array_cards);
		element["piaoWinLose"]=Json::Value(array_piaoWinLose);

		Json::Value array_WeaveKindList;
		for(int i=0;i<MAX_WEAVE;i++)
		{
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_WeaveKindList[key_1]=Json::Value((int)pChangeData->tWeaveItemArray[i].iWeaveKind);
		}
		element["iWeaveKind"]=Json::Value(array_WeaveKindList);
		

		Json::Value array_WeaveCardList;
		for (int i = 0; i < MAX_WEAVE; i++)
		{
			char szkey[10];
			memset(szkey,0,sizeof(szkey));
			sprintf(szkey,"%d",i);
			Json::Value array_2;
			for(int j=0;j<4;j++)
			{
				char key_2[10];
				memset(key_2,0,sizeof(key_2));
				sprintf(key_2,"%d",j);
				array_2[key_2]=Json::Value(pChangeData->tWeaveItemArray[i].byCardList[j]);
			}
			array_WeaveCardList[szkey]=Json::Value(array_2);
		}
		element["iWeaveCardList"]=Json::Value(array_WeaveCardList);

		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(11==iType)//更新胡息
	{
		S_C_UpdataHuXi *pChangeData=(S_C_UpdataHuXi *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("UpdateHuXi");
		Json::Value array_HuXiList;
		for(int i=0;i<PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1,0,sizeof(key_1));
			sprintf(key_1,"%d",i);
			array_HuXiList[key_1]=Json::Value(pChangeData->byUserHuXi[i]);
		}
		element["HuXi"]=Json::Value(array_HuXiList);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(12==iType)//动作结果
	{
		S_C_ActionResult *pChangeData=(S_C_ActionResult *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("ActionRes");
		element["byDeskStation"]=Json::Value(pChangeData->byDeskStation);
		element["iOperateCode"]=Json::Value(pChangeData->iOperateCode);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(13==iType)//王隐藏
	{
		S_C_WangTake *pChangeData=(S_C_WangTake *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("WangTake");
		element["bTake"]=Json::Value(pChangeData->bTake);
		element["byDeskStation"]=Json::Value(pChangeData->byDeskStation);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
	else if(14==iType)//通知死守
	{
		S_C_NotifyForbidCard *pChangeData=(S_C_NotifyForbidCard *)pData;
		if(!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex,0,sizeof(szEventIndex));
		sprintf(szEventIndex,"%d",m_iVideoIndex);
		Json::Value element;
		element["eventType"]=Json::Value("NotifyForbid");
		element["byDeskStation"]=Json::Value(pChangeData->byDeskStation);
		m_element[szEventIndex]=Json::Value(element);
		m_iVideoIndex++;
	}
}
//配牌
BYTE CServerGameDesk::HandleCard()
{
	char cFileName[64];
	memset(cFileName,0,sizeof(cFileName));
	sprintf(cFileName,"C:/web/json/phz.json");
	ifstream fin;
	fin.open(cFileName);
	if(!fin.is_open())
	{
		return 0;
	}

	Json::Reader reader;
	Json::Value  root;
	if (!reader.parse(fin, root, false))
	{
		return 0;
	}
	Json::Value statusKeyValue = root["status"];
	int iStatus = statusKeyValue.asInt();
	if(0 == iStatus)
	{
		return 0;
	}
	Json::Value ntKeyValue = root["first"];
	int iNt = ntKeyValue.asInt();
	if (iNt > 0 && iNt <= PLAY_COUNT)
	{
		m_byBankerUser = iNt - 1;
	}
	//坐醒玩家
	if (m_tGameRules.byUserCount == 4)
	{
		m_byZuoXingUser = GetNextDeskStation(GetNextDeskStation(m_byBankerUser, true), true);
	}

	memset(m_byCardIndexJing,0,sizeof(m_byCardIndexJing));
	memset(m_byCardIndex,0,sizeof(m_byCardIndex));
	//获取手牌数据
	BYTE byBankerCard=0;
	int iHandCount=0;
	int iCardCount[PLAY_COUNT];
	memset(iCardCount,0,sizeof(iCardCount));
	int arraySize = root["CardCout"].size();
	arraySize = (arraySize > PLAY_COUNT)? PLAY_COUNT:arraySize;
	for (int i = 0; i <arraySize; i++)
	{
		char _index[10];
		memset(_index,0,sizeof(_index));
		sprintf(_index,"%d",i);
		Json::Value valKeyValue = root["CardCout"][i][_index];
		int tempKeyValue = valKeyValue.asInt();
		for (int j = i; j < PLAY_COUNT; j++)
		{
			if (j == m_byZuoXingUser) continue;
			iCardCount[j] = tempKeyValue;
			iHandCount+=tempKeyValue;
			break;
		}
		//if(iCardCount[i]>0)
		//{
		//	m_byCardIndexJing[i]=0;
		//	memset(m_byCardIndex[i],0,sizeof(m_byCardIndex[i]));
		//}
	}
	
	int index = 0;
	for (int i = 0; i <PLAY_COUNT; i++)
	{
		if (i == m_byZuoXingUser) continue;
		char _array[10];
		memset(_array,0,sizeof(_array));
		sprintf(_array,"%d", index);
		for(int j=0;j<iCardCount[i];j++)
		{
			char _index[10];
			memset(_index,0,sizeof(_index));
			sprintf(_index,"%d",j);
			Json::Value valKeyValue = root["CardData"][_array][j][_index];
			int tempKeyValue = valKeyValue.asInt();
			if(m_byBankerUser==i && iCardCount[i]-1==j)
			{
				byBankerCard=tempKeyValue;
				continue;
			}
			if(KINGVALUE==tempKeyValue)
			{
				m_byCardIndexJing[i]+=1;
			}
			else
			{
				m_byCardIndex[i][m_GameLogic.SwitchToCardIndex(tempKeyValue)]++;
			}
		}
		index++;
	}

	//获取门牌数据
	int iMCardCount[2];
	memset(iMCardCount,0,sizeof(iMCardCount));
	int iMSize = root["MCardCout"].size();
	iMSize = (iMSize > 2)? 2:iMSize;
	for (int i = 0; i <iMSize; i++)
	{
		char _index[10];
		memset(_index,0,sizeof(_index));
		sprintf(_index,"%d",i);
		Json::Value valKeyValue = root["MCardCout"][i][_index];
		int tempKeyValue = valKeyValue.asInt();

		iMCardCount[i] = tempKeyValue;
	}

	int iMenIndex=(80+m_tGameRules.byKingNum)-iHandCount-1;
	for (int i = 0; i <2; i++)
	{
		char _array[10];
		memset(_array,0,sizeof(_array));
		sprintf(_array,"%d",i);
		for(int j=0;j<iMCardCount[i];j++)
		{
			char _index[10];
			memset(_index,0,sizeof(_index));
			sprintf(_index,"%d",j);
			Json::Value valKeyValue = root["MCardData"][_array][j][_index];
			int tempKeyValue = valKeyValue.asInt();
			if(iMenIndex>=0)
			{
				m_byRepertoryCard[iMenIndex--]=tempKeyValue;
			}
		}
	}
	fin.close();
	return byBankerCard;
}

//写死部分规则值
void CServerGameDesk::SetFixedRule()
{
	if (m_tGameRules.piaoFen == 0)
	{
		memset(m_piaoList, 0, sizeof(m_piaoList));
	}
	else if (m_tGameRules.piaoFen == 1)
	{
		m_piaoList[0] = 5;
		m_piaoList[1] = 10;
		m_piaoList[2] = 20;
	}
	else if (m_tGameRules.piaoFen == 2)
	{
		m_piaoList[0] = 20;
		m_piaoList[1] = 30;
		m_piaoList[2] = 50;
	}

	if (m_tGameRules.autoOverTime == 0)
	{
		m_cfgbyThinkTime = 0;
		m_tGameRules.autoCount = 0;
	}
	else if (m_tGameRules.autoOverTime > 0)
	{
		m_cfgbyThinkTime = m_tGameRules.autoOverTime * 60;
	}
}

//通知选漂分
void CServerGameDesk::NoticeSelectPiao()
{
	S_C_NoticePiao msg;
	memcpy(msg.piaoList, m_piaoList, sizeof(msg.piaoList));
	msg.byZuoXingUser = m_byZuoXingUser;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if(m_byUserStation[i] != STATE_NULL && i != m_byZuoXingUser)
		{
			SendGameData(i, &msg, sizeof(S_C_NoticePiao), MSG_MAIN_LOADER_GAME, S_C_NOTICE_PIAO, 0);
		}
	}

	SetGameStation(GS_PIAO);
	SetTimer(TIME_PIAO, m_cfgPiaoTime * 1000);
}

//选漂分请求
void CServerGameDesk::OnUserPiao(BYTE byStation, int piaoV)
{
	if (byStation < 0 || byStation >= PLAY_COUNT)
	{
		return;
	}

	//检查是否是有效漂分选项
	if (!IsValidPiaoV(piaoV))
	{
		return;
	}

	m_piaoV[byStation] = piaoV;
	SendPiaoResp(byStation, piaoV);

	if (IsAllSelectPiao())
	{
		KillTimer(TIME_PIAO);
		NoticeSendCard(); //通知发牌
	}
}

//选漂分回复
void CServerGameDesk::SendPiaoResp(BYTE byStation, int piaoV)
{
	S_C_PiaoResp msg;
	msg.deskStation = byStation;
	msg.piaoV = piaoV;

	BroadcastDeskData(&msg, sizeof(S_C_PiaoResp), MSG_MAIN_LOADER_GAME, S_C_PIAO_RESP, 0);
}

//检查是否是有效漂分选项
bool CServerGameDesk::IsValidPiaoV(int piaoV)
{
	for (int i = 0; i < PIAO_COUNT; i++)
	{
		if (m_piaoList[i] != -1 && piaoV == m_piaoList[i])
		{
			return true;
		}
	}

	return true;
}

//检查是否所有人都已选漂分
bool CServerGameDesk::IsAllSelectPiao()
{
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (GetUserIDByDeskStation(i) <= 0) continue;
		if (m_piaoV[i] < 0) 
		{
			return false;
		}
	}

	return true;
}

//通知发牌
void CServerGameDesk::NoticeSendCard()
{
	//游戏状态
	SetGameStation(GS_PLAYGAME);

	m_GameData.m_tJingData.AddJing(KINGVALUE);
	m_GameLogic.SetMinHuXi(m_tGameRules.byMinHuXi);
	//混乱扑克
	m_byLeftCardCount=80+m_tGameRules.byKingNum;
	m_GameLogic.RandCardData(m_byRepertoryCard,m_byLeftCardCount);

	//设置玩家牌
	if(m_tGameRules.b15Type) //15张玩法
	{
		for(int i = 0;i < PLAY_COUNT;i ++)
		{
			if(STATE_PLAY_GAME == m_byUserStation[i] && i != m_byZuoXingUser)
			{
				m_byLeftCardCount -= 14;
				m_byCardCount[i] = 14;
				m_GameLogic.SwitchToCardIndex(&m_byRepertoryCard[m_byLeftCardCount],14,m_byCardIndex[i],m_byCardIndexJing[i]);
			}
		}
	}
	else
	{
		for(int i = 0;i < PLAY_COUNT;i ++)
		{
			if(STATE_PLAY_GAME == m_byUserStation[i] && i != m_byZuoXingUser)
			{
				m_byLeftCardCount -= (MAX_COUNT - 1);
				m_byCardCount[i] = (MAX_COUNT - 1);
				m_GameLogic.SwitchToCardIndex(&m_byRepertoryCard[m_byLeftCardCount],MAX_COUNT - 1,m_byCardIndex[i],m_byCardIndexJing[i]);
			}
		}
	}

	BYTE byBankerCard = GetPai(m_byBankerUser);

	//json配牌
	BYTE byHandCard=HandleCard();
	if(0!=byHandCard)
	{
		byBankerCard=byHandCard;
	}

	if(KINGVALUE != byBankerCard)
	{
		m_byCardIndex[m_byBankerUser][m_GameLogic.SwitchToCardIndex(byBankerCard)]++;
	}
	else
	{
		m_byCardIndexJing[m_byBankerUser]++;
	}
	m_byCardCount[m_byBankerUser]++;

	//放GetPai后,设置要4王且byBankerCard为王时,会有5王BUG
	SuperGetWangLogic();

	CheckLongNum();

	if(KINGVALUE==byBankerCard)
	{
		NoticeWangTakeOut(m_byBankerUser, true);
	}

	//构造数据
	bool bTiCardAction=false;
	S_C_SendCardData tGameStart;
	tGameStart.byBankerUser  = m_byBankerUser;
	tGameStart.byCurrentUser = INVALID_CHAIR;
	tGameStart.byLeftCardCount = m_byLeftCardCount;
	tGameStart.byZuoXingUser  = m_byZuoXingUser;

	S_C_SendCardData tReplayGameStart;
	tReplayGameStart=tGameStart;
	for(int i = 0;i < PLAY_COUNT;i ++)
	{
		//设置数据
		m_GameLogic.SwitchToCardData(m_byCardIndex[i],tGameStart.byCardData[i],CountArray(tGameStart.byCardData[i]),m_byCardIndexJing[i]);
		memcpy(tReplayGameStart.byCardData[i],tGameStart.byCardData[i],sizeof(tReplayGameStart.byCardData[i]));
		SendGameData(i,&tGameStart,sizeof(S_C_SendCardData),MSG_MAIN_LOADER_GAME,S_C_SEND_ALL_CARD,0);
		tGameStart.Init();
	}

	OnJson(&tReplayGameStart,1);

	SetXingPai(byBankerCard);

	m_GameLogic.SetRealJingNum(m_byCardIndexJing);

	char logBuf[1024] = { 0 };
	int len = 0;
	len += sprintf(logBuf, "【GAME_BEGIN】");
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if(STATE_PLAY_GAME == m_byUserStation[i])
		{
			len += sprintf(logBuf + len, "userID:%ld, jingNum:%d, cards|", GetUserIDByDeskStation(i), m_byCardIndexJing[i]);
			for (int j = 0; j < MAX_INDEX; ++j)
			{
				len += sprintf(logBuf + len, "%d", m_byCardIndex[i][j]);
			}
		}
		len += sprintf(logBuf + len, "\n");
	}
	FILE_LOG(GAMENAME, logBuf);

	SetTimer(TIME_GAME_BEING_PLAY,3000);//显示牌
	SetTimer(TIME_REMAINING,1000);
}

//获取听牌提示数据
void CServerGameDesk::GetTingData(BYTE byStation, BYTE byOutPaiTing[MAX_INDEX], BYTE byOutPaiTingCards[MAX_INDEX][MAX_INDEX])
{
	//初始化
	for (int i = 0; i < MAX_INDEX; i++)
	{
		byOutPaiTing[i] = 0;
		for (int j = 0; j < MAX_INDEX; j++)
		{
			byOutPaiTingCards[i][j] = 0;
		}
	}

	CountCPTWPXingNumber(byStation);
	m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(byStation));
	BYTE byOutCardIndex[MAX_INDEX];	
	memset(byOutCardIndex,0,sizeof(byOutCardIndex));
	for (int i = 0;i < MAX_INDEX;i++)
	{
		if(m_byCardIndex[byStation][i]<=0 || m_byCardIndex[byStation][i]>=3)
		{
			continue;
		}
		BYTE byCheckHuCardIndex[MAX_INDEX];	
		memcpy(byCheckHuCardIndex,m_byCardIndex[byStation],sizeof(byCheckHuCardIndex));
		byCheckHuCardIndex[i]--;
		tagHuCardInfo tHuCardInfo[5];
		BYTE byHuCardsIndex[MAX_INDEX] = {0};
		if(m_GameLogic.CanTing(byStation,255,byCheckHuCardIndex,m_byCardIndexJing[byStation],m_byUserHuXiCount[byStation],tHuCardInfo,true,m_GameData.m_tCPTWPArray[byStation], byHuCardsIndex))
		{
			byOutPaiTing[i]=m_byCardIndex[byStation][i];
			byOutCardIndex[i]=m_byCardIndex[byStation][i];
			memcpy(byOutPaiTingCards[i], byHuCardsIndex,sizeof(byHuCardsIndex));
		}
	}
	memcpy(m_GameData.m_byOutPai[byStation],byOutCardIndex,sizeof(m_GameData.m_byOutPai[byStation]));
}

//王隐藏
void CServerGameDesk::NoticeWangTakeOut(BYTE byStation, bool bTake, int iType/*=13*/)
{
	S_C_WangTake msg;
	msg.bTake = bTake;
	msg.byDeskStation = byStation;
	BroadcastDeskData(&msg, sizeof(S_C_WangTake), MSG_MAIN_LOADER_GAME, S_C_WANGTAKEOUT, 0);
	OnJson(&msg, iType);
}

//发送操作选项
void CServerGameDesk::SendOperateNotify(BYTE byStation, BYTE byResumeUser, BYTE byActionCard, int iOperateCode, const tagChiCardInfo &tChiCardInfo)
{
	S_C_OperateNotify msg;
	msg.byResumeUser = byResumeUser;			//下一玩家
	msg.byActionCard = byActionCard;			//胡的牌值
	//永州麻将,3王闯做王炸逻辑搞(发给客户端的是王炸,但是服务器以3王闯跑逻辑)
	if (iOperateCode & ACT_SWANG_CHUANG)
	{
		iOperateCode &= 0xFBFF;
		iOperateCode |= ACT_WANG_ZHA;
	}
	msg.iOperateCode = iOperateCode;			//玩家可操作的动作
	memcpy(&msg.tChiCardInfo,&tChiCardInfo,sizeof(msg.tChiCardInfo));
	SendGameData(byStation,&msg,sizeof(S_C_OperateNotify),MSG_MAIN_LOADER_GAME,S_C_OPERATE_NOTIFY,0);
}

//设置定时器
bool CServerGameDesk::MySetTimer(UINT uTimerID, int uElapse)
{
	if (uElapse > 0)
	{
		if (uTimerID == TIME_OPER_OUTTIMER)
		{
			for (BYTE i = 0; i < PLAY_COUNT; ++i)
			{
				if (STATE_PLAY_GAME != m_byUserStation[i] || m_iUserAction[i] == ACK_NULL)	continue;
				if (IsAuto(i)) //托管了,时间就短点
				{
					uElapse = 1500;
				}
				SetTimer(TIME_OPER_OUTTIMER + i, uElapse);
			}
			return true;
		}
		else
		{
			return SetTimer(uTimerID, uElapse);
		}
	}
	return false;
}

//是否有胡操作
bool CServerGameDesk::HaveHuAction(int iUserAction)
{
	if ( (iUserAction & ACK_CHIHU) || (iUserAction & ACT_WANG_DIAO) || (iUserAction & ACT_WANG_CHUANG) || (iUserAction & ACT_SWANG_CHUANG) || (iUserAction & ACT_WANG_ZHA) )
	{
		return true;
	}

	return false;
}

//胡家,坐醒玩家分醒钱逻辑(4人玩法)
void CServerGameDesk::DivideXingScore(S_C_GameEnd &tGameEnd, int xingScore)
{
	if (m_tGameRules.byUserCount != 4 || m_byZuoXingUser < 0 || m_byZuoXingUser >= PLAY_COUNT || m_byHuUser < 0 || m_byHuUser >= PLAY_COUNT)
	{
		return;
	}

	int tmp = xingScore / 2;
	tGameEnd.i64GameScore[m_byHuUser] -= tmp;
	tGameEnd.i64GameScore[m_byZuoXingUser] += tmp;
}

//超端玩家请求看门牌
bool CServerGameDesk::SuperUserLookMenPai(BYTE byDeskStation)
{
	if (!IsSuperUser(byDeskStation))
	{
		return false;
	}

	SendLeftCards(byDeskStation);

	return true;
}

//超端玩家请求要牌
bool CServerGameDesk::SuperUserChangePai(BYTE byDeskStation, void *pData, int size)
{
	CMD_C_Super_Request_Card *pSuperReq = (CMD_C_Super_Request_Card*)pData;
	if (!pSuperReq || pSuperReq->card == 255)
	{
		return false;
	}

	if (!IsSuperUser(byDeskStation))
	{
		SuperChangePaiResp(byDeskStation, 1);
		return false;
	}

	if (!IsHaveOneLeftCard(pSuperReq->card))
	{
		SuperChangePaiResp(byDeskStation, 2);
		return false;
	}

	SuperChangePaiResp(byDeskStation, 0);

	m_superSetCard = pSuperReq->card;
	m_superSetChairID = byDeskStation;

	return true;
}

//发送牌堆数据
void CServerGameDesk::SendLeftCards(BYTE byDeskStation)
{
	CMD_S_SuperLookResult msg;

	for (int i = 0; i < m_byLeftCardCount; ++i)
	{
		int card = m_byRepertoryCard[i];
		if (m_GameData.m_tJingData.CheckIsJing(m_byRepertoryCard[i]))
		{
			msg.byRemainPaiDate[20]++;
			continue;
		}
		int index = m_GameLogic.SwitchToCardIndex(card);
		if (index >= 0 && index < 20)
		{
			msg.byRemainPaiDate[index]++;
		}
	}

	SendGameData(byDeskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, S_C_SUPER_REQUEST_LOOK_RESULT, 0);
}

//牌堆是否有该牌
bool CServerGameDesk::IsHaveOneLeftCard(BYTE card)
{
	if (card == 0 || card == 255)
	{
		return false;
	}

	for (int i = 0; i < m_byLeftCardCount; ++i)
	{
		if (card == m_byRepertoryCard[i])
		{
			return true;
		}
	}

	return false;
}

//超端要牌返回
void CServerGameDesk::SuperChangePaiResp(BYTE byDeskStation, int errCode)
{
	CMD_S_Super_Request_Card_Result msg;

	msg.ECode = errCode;
	SendGameData(byDeskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, S_C_SUPER_REQUEST_CRAD_RESULT, 0);
}

//获取胡牌数据
void CServerGameDesk::GetHuPaiData(BYTE byStation, BYTE byHuPai[MAX_INDEX])
{
	//初始化
	for (int i = 0; i < MAX_INDEX; i++)
	{
		byHuPai[i] = 0;
	}

	BYTE byCheck[20] = {
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小字
		0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大字
	};

	CountCPTWPXingNumber(byStation);
	m_GameLogic.SetCPGTWPRedCardNum(m_GameData.GetCPTWPEQS(byStation));
	int count = 0;
	for (int i = 0; i < 20; i++)
	{
		tagHuCardInfo tHuCardInfo[5];
		BYTE byHuCardsIndex[MAX_INDEX] = { 0 };
		if (m_GameLogic.GetHuCardInfo(byStation, 255, m_byCardIndex[byStation], m_byCardIndexJing[byStation], byCheck[i], m_byUserHuXiCount[byStation], tHuCardInfo, true, m_GameData.m_tCPTWPArray[byStation]))
		{
			int index = m_GameLogic.SwitchToCardIndex(byCheck[i]);
			byHuPai[index] = 1;
		}
	}
}

//超端设置发牌时拿几个王请求
bool CServerGameDesk::SuperUserGetWangReq(BYTE byDeskStation, void *pData, int size)
{
	CMD_C_SuperGetWangReq *pSuperReq = (CMD_C_SuperGetWangReq*)pData;
	if (!pSuperReq)
	{
		return false;
	}

	if (!IsSuperUser(byDeskStation))
	{
		SuperUserGetWangResp(byDeskStation, 1);
		return false;
	}

	if (GetGameStation() == GS_PLAYGAME)
	{
		SuperUserGetWangResp(byDeskStation, 3);
		return false;
	}

	if (pSuperReq->wangNum > m_tGameRules.byKingNum)
	{
		SuperUserGetWangResp(byDeskStation, 2);
		return false;
	}

	SuperChangePaiResp(byDeskStation, 0);

	m_superWangNum = pSuperReq->wangNum;
	m_superSetChairID = byDeskStation;

	return true;
}

//超端设置发牌时拿几个王返回
void CServerGameDesk::SuperUserGetWangResp(BYTE byDeskStation, int errCode)
{
	CMD_S_SuperGetWangResp msg;

	msg.ECode = errCode;
	SendGameData(byDeskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, S_C_SUPER_GET_WANG_RESP, 0);
}

//超端发牌拿王逻辑
void CServerGameDesk::SuperGetWangLogic()
{
	if (m_superWangNum <= 0 || m_superSetChairID < 0 || m_superSetChairID >= PLAY_COUNT)
	{
		return;
	}
	int findCount = 0;
	//差几张
	int differ = m_superWangNum - m_byCardIndexJing[m_superSetChairID];
	//正常发牌已经满足要求了
	if (differ <= 0)
	{
		goto END;
	}

	m_byCardIndexJing[m_superSetChairID] = m_superWangNum;

	//在底牌中找王
	for (int i = 0; i < m_byLeftCardCount; ++i)
	{
		if (0x2f == m_byRepertoryCard[i]) //王牌
		{
			for (int j = 0; j < MAX_INDEX; ++j)
			{
				if (m_byCardIndex[m_superSetChairID][j] > 0)
				{
					m_byRepertoryCard[i] = m_GameLogic.SwitchToCardData(j);
					m_byCardIndex[m_superSetChairID][j]--;
					findCount++;
					if (findCount >= differ)
					{
						goto END;
					}
					break;
				}
			}
		}
	}

	//在别家手上找王
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_byUserStation[i] == STATE_NULL || i == m_superSetChairID || m_byCardIndexJing[i] <= 0)
		{
			continue;
		}
		int size = m_byCardIndexJing[i];
		for (int j = 0; j < size; ++j)
		{
			for (int k = 0; k < MAX_INDEX; ++k)
			{
				if (m_byCardIndex[m_superSetChairID][k] > 0 && m_byCardIndex[i][k] < 3) //保证起手不要有4张的牌
				{
					m_byCardIndexJing[i]--;
					m_byCardIndex[m_superSetChairID][k]--;
					m_byCardIndex[i][k]++;
					findCount++;
					if (findCount >= differ)
					{
						goto END;
					}
					break;
				}
			}
		}
	}

END:
	m_superSetChairID = 255;
	m_superWangNum = 0;
}

//是否是机器人
bool CServerGameDesk::IsVirtual(BYTE byDeskStation)
{
	GameUserInfo *pUser = m_pDataManage->GetUser(GetUserIDByDeskStation(byDeskStation));
	if (pUser && pUser->isVirtual)
	{
		return true;
	}

	return false;
}

