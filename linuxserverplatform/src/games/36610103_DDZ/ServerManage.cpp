#include "StdAfx.h"
#include "ServerManage.h"
#include <math.h>
#include <windows.h> 
#include <DbgHelp.h>  
#include <stdlib.h>  

#ifndef USER_IDENTITY_TYPE_SUPER
#define USER_IDENTITY_TYPE_SUPER 1
#endif

//构造函数
CServerGameDesk::CServerGameDesk(void) :CGameDesk(FULL_BEGIN)
{
	//InitMinDump();
	//c++ test
	::ZeroMemory(m_iUserCardCount, sizeof(m_iUserCardCount));
	::ZeroMemory(m_iUserCard, sizeof(m_iUserCard));
	::ZeroMemory(m_iBackCard, sizeof(m_iBackCard));
	::ZeroMemory(m_iDeskCard, sizeof(m_iDeskCard));

	//已初始化
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_iLastCardCount[i] = 0;
		m_iAwardPoint[i] = 0;
		m_iWinPoint[i] = 0;

		m_bUserNetCut[i] = false;
	}
	::ZeroMemory(m_iLastOutCard, PLAY_COUNT * 45);
	m_iRecvMsg = 0;
	m_iGameFlag = GS_FLAG_NORMAL;
	m_iUpGradePeople = -1;
	m_iLeaveArgee = 0;
	m_iNtFirstCount = 0;		//第一首牌清零
	//	m_iNowCallScorePeople = -1;		//
	m_iBaseOutCount = 0;
	m_iDealPeople = -1;
	m_iFirstOutPeople = -1;
	m_iNowBigPeople = -1;
	m_iOutCardPeople = -1;
	m_iBeenPlayGame = 0;
	SetGameStation(GS_WAIT_ARGEE);
	m_bThrowoutCard = 0x00;
	m_bFirstCallScore = 255;
	m_iPrepareNT = 255;		//预备庄家
	m_iFirstShow = 255;
	m_iCurrentRobPeople = 255;
	m_iFirstRobNt = 255;
	m_bHaveSendBack = false;

	::memset(m_iDeskCardCount, 0, sizeof(m_iDeskCardCount));
	::memset(m_bAuto, 0, sizeof(m_bAuto));				//托管
	::memset(m_iCallScore, -1, sizeof(m_iCallScore));		//叫分
	::memset(m_bIsCall, false, sizeof(m_bIsCall));		//标记叫分
	::memset(m_bCanleave, 1, sizeof(m_bCanleave));
	::memset(m_bUserReady, 0, sizeof(m_bUserReady));
	::memset(m_iRobStation, 0, sizeof(m_iRobStation));
	::memset(m_iAddStation, 0, sizeof(m_iAddStation));

	::memset(m_byPass, 0, sizeof(m_byPass));
	::memset(m_byLastTurnPass, 0, sizeof(m_byLastTurnPass));

	m_bySendFinishCount = 0;
	m_iGameBaseScore = 1;

	m_iFirstCutPeople = 255;
	m_icountleave = 0;

	m_iBaseMult = 1;
	m_iAddDoubleLimit = 0;
	m_iGameMaxLimit = 0;
	m_iLimitPoint = 0;

	m_GameMutiple.IniData(m_iBaseMult);
	m_bRetryStart = false;
	m_iFengDing = 0;
	m_iRunTime = 0;//单位秒
	m_iGameBeginTime = 0; //单位秒
	m_iGameEndTime = 0;

	m_lastCardListIndex = 0;
	memset(m_lastCardList, 0, sizeof(m_lastCardList));
}

//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
}

//加载局数限制
BOOL CServerGameDesk::LoadExtIni()
{
	char szfile[256] = "";
	sprintf_s(szfile, sizeof(szfile), "%s%d_s.ini", CINIFile::GetAppPath().c_str(), NAME_ID);
	CINIFile f(szfile);

	int roomType = GetRoomType();

	///////////////////////////////////游戏配置///////////////////////////////////////
	string key = TEXT("game");
	m_bHaveKing = f.GetKeyVal(key, "haveking", 1);
	m_iPlayCard = f.GetKeyVal(key, "card", 1);
	m_iPlayCount = f.GetKeyVal(key, "cardcount", 54);
	m_iSendCount = f.GetKeyVal(key, "sendcard", 51);
	m_iBackCount = f.GetKeyVal(key, "backcard", 3);
	m_iUserCount = f.GetKeyVal(key, "usercount", 17);
	m_iAddDoubleLimit = f.GetKeyVal(key, "AddDoubleLimit", 0);
	if (roomType == ROOM_TYPE_GOLD)
	{
		RoomBaseInfo * pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_pDataManage->GetRoomID());
		m_iGameBaseScore = pRoomBaseInfo->basePoint > 0 ? pRoomBaseInfo->basePoint : 1;
		m_iAddDoubleLimit = pRoomBaseInfo->minPoint * 2;
	}
	else if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		m_iGameBaseScore = m_MinPoint;
	}

	if (m_iBaseMult <= 0)
	{
		m_iBaseMult = 1;
	}
	if (m_iGameBaseScore <= 0)
	{
		m_iGameBaseScore = 1;
	}
	m_iGameMaxLimit = f.GetKeyVal(key, "GameMaxLimit", 0);
	m_iLimitPoint = f.GetKeyVal(key, "LimitPoint", 0);
	m_bNoXiPai = f.GetKeyVal(key, "XiPai", 0); //1是不洗牌模式
	m_bKingCanReplace = f.GetKeyVal(key, "kingcanreplace", 0);
	m_iTimeOutNoCardOut = f.GetKeyVal(key, "TimeOutNoCardOut", 1);
	m_iModel = f.GetKeyVal(key, "Model", 1); //1:叫分模式,0:抢地主模式
	m_bRobnt = f.GetKeyVal(key, "robnt", 1);   //（1：游戏必须抢地主 0：由游戏决定地主）
	m_bTurnRule = f.GetKeyVal(key, "TurnRule", 1);
	m_iLimitPlayGame = f.GetKeyVal(key, "limitplay", 1); ////至少打完多少局
	m_bAdddouble = f.GetKeyVal(key, "adddouble", 0); //是否加倍
	m_bShowcard = f.GetKeyVal(key, "showcard", 0);   //是否明牌


	//////////////////////////////////游戏时间////////////////////////////////////////
	key = TEXT("other");
	m_iBeginTime = f.GetKeyVal(key, "begintime", 15);
	m_iThinkTime = f.GetKeyVal(key, "thinktime", 20);
	m_iSendCardTime = f.GetKeyVal(key, "SendCardTime", 1);
	m_iRobNTTime = f.GetKeyVal(key, "RobNTTime", 10);
	m_iCallScoreTime = f.GetKeyVal(key, "CallScoretime", 15);
	m_iAddDoubleTime = f.GetKeyVal(key, "adddoubletime", 5);

	/////////////////////////////////牌型/////////////////////////////////////////
	key = TEXT("cardshape");
	m_iCardShape &= 0x00000000;
	m_iCardShape |= (f.GetKeyVal(key, "one", 1) & 0xFFFFFFFF);//单张
	m_iCardShape |= ((f.GetKeyVal(key, "two", 1) << 1) & 0xFFFFFFFF);//对
	m_iCardShape |= ((f.GetKeyVal(key, "three", 1) << 2) & 0xFFFFFFFF);//三条
	m_iCardShape |= ((f.GetKeyVal(key, "threeandone", 1) << 3) & 0xFFFFFFFF);//三带单
	m_iCardShape |= ((f.GetKeyVal(key, "threeandtwo", 0) << 4) & 0xFFFFFFFF);//三带二单
	m_iCardShape |= ((f.GetKeyVal(key, "threeanddouble", 1) << 5) & 0xFFFFFFFF);//三带对
	m_iCardShape |= ((f.GetKeyVal(key, "traight", 1) << 6) & 0xFFFFFFFF);//单顺
	m_iCardShape |= ((f.GetKeyVal(key, "traightflush", 0) << 7) & 0xFFFFFFFF);//同花顺
	m_iCardShape |= ((f.GetKeyVal(key, "twosequence", 1) << 8) & 0xFFFFFFFF);//双顺
	m_iCardShape |= ((f.GetKeyVal(key, "threesequence", 1) << 9) & 0xFFFFFFFF);//三顺
	m_iCardShape |= ((f.GetKeyVal(key, "threeandonesequence", 1) << 10) & 0xFFFFFFFF);//三带带单
	m_iCardShape |= ((f.GetKeyVal(key, "threeandtwosequence", 0) << 11) & 0xFFFFFFFF);//三带二单顺
	m_iCardShape |= ((f.GetKeyVal(key, "threeanddublesequence", 1) << 12) & 0xFFFFFFFF);//三带对顺
	m_iCardShape |= ((f.GetKeyVal(key, "fourandone", 0) << 13) & 0xFFFFFFFF);//四带一
	m_iCardShape |= ((f.GetKeyVal(key, "fourandtwo", 1) << 14) & 0xFFFFFFFF);//四带二单
	m_iCardShape |= ((f.GetKeyVal(key, "fourandonedouble", 1) << 15) & 0xFFFFFFFF);//四带一对
	m_iCardShape |= ((f.GetKeyVal(key, "fourandtwodouble", 1) << 16) & 0xFFFFFFFF);//四带二对
	m_iCardShape |= ((f.GetKeyVal(key, "510k", 0) << 17) & 0xFFFFFFFF);			//510k
	m_iCardShape |= ((f.GetKeyVal(key, "master510k", 0) << 18) & 0xFFFFFFFF);		//同花510k
	m_iCardShape |= ((f.GetKeyVal(key, "bomb", 1) << 19) & 0xFFFFFFFF);			//炸弹
	m_iCardShape |= ((f.GetKeyVal(key, "kingbomb", 1) << 20) & 0xFFFFFFFF);		//王炸
	InitThisGame();
	return TRUE;
}

//根据房间ID加载配置文件
BOOL CServerGameDesk::LoadExtIni(int iRoomID)
{
	TCHAR szKey[20];
	char nid[20] = "";
	sprintf(nid, "%d", NAME_ID);
	sprintf(szKey, "%s_%d", nid, iRoomID);
	string key = szKey;
	string s = CINIFile::GetAppPath();/////本地路径
	CINIFile f(s + nid + "_s.ini");

	m_iLimitPlayGame = f.GetKeyVal(key, "limitplay", m_iLimitPlayGame);
	m_iModel = f.GetKeyVal(key, "robnt", m_iModel);
	m_bRobnt = f.GetKeyVal(key, "robnt", m_bRobnt);
	m_bAdddouble = f.GetKeyVal(key, "adddouble", m_bAdddouble);
	m_bShowcard = f.GetKeyVal(key, "showcard", m_bShowcard);
	m_bTurnRule = f.GetKeyVal(key, "TurnRule", m_bTurnRule);
	m_iBaseMult = f.GetKeyVal(key, "BaseMutiple", m_iBaseMult);
	m_iGameMaxLimit = f.GetKeyVal(key, "GameMaxLimit", m_iGameMaxLimit);
	m_iLimitPoint = f.GetKeyVal(key, "LimitPoint", m_iLimitPoint);
	m_bNoXiPai = f.GetKeyVal(key, "XiPai", m_bNoXiPai);
	return TRUE;
}

//设置游戏状态
BOOL CServerGameDesk::InitThisGame()
{
	m_Logic.SetKingCanReplace(m_bKingCanReplace);
	m_Logic.SetCardShape(m_iCardShape);
	return true;
}

//初始化桌子
bool CServerGameDesk::InitDeskGameStation()
{
	LoadDynamicConfig();

	m_GameMutiple.IniData(m_iBaseMult);
	m_tZongResult.Init();
	m_byGameEndType = 0;
	return true;
}

//桌子成功解散
void CServerGameDesk::OnDeskSuccessfulDissmiss(bool isDismissMidway)
{

	if (m_iRunGameCount > 0) //大于一局才发大结算
	{
		UpdateCalculateBoard();
	}
	__super::OnDeskSuccessfulDissmiss(isDismissMidway);
}

void CServerGameDesk::InitDeskGameData()
{
	/*for (int i=0; i<PLAY_COUNT; i++)
	{
	m_tZongResult[i].Init();
	}*/
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_vecWinInfo[i].clear();
	}
	KillAllTimer();
	ReSetGameState(GFF_FORCE_FINISH);
	SetGameStation(GS_WAIT_SETGAME);
	memset(m_bUserReady, 0, sizeof(m_bUserReady));
	m_iFengDing = 0;
	m_iRunTime = 0;//单位秒
	m_iGameBeginTime = 0; //单位秒
	m_iGameEndTime = 0;
	m_bFirstCallScore = 255;
	m_byGameEndType = 0;

	m_lastCardListIndex = 0;
	memset(m_lastCardList, 0, sizeof(m_lastCardList));
}

// 玩家请求托管
bool CServerGameDesk::OnHandleUserRequestAuto(BYTE deskStation)
{
	return __super::OnHandleUserRequestAuto(deskStation);
}

// 玩家取消托管
bool CServerGameDesk::OnHandleUserRequestCancelAuto(BYTE deskStation)
{
	return __super::OnHandleUserRequestCancelAuto(deskStation);
}

// 动态加载配置文件数据
void CServerGameDesk::LoadDynamicConfig()
{
	LoadExtIni();
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser)
{
	switch (assistID)
	{
	case ASS_SEND_FINISH:
	{
		/*m_bySendFinishCount ++ ;

		if(m_bySendFinishCount >= PLAY_COUNT)
		{
		KillTimer(TIME_SEND_CARD_ANI) ;
		KillTimer(TIME_SEND_CARD_FINISH) ;
		SetTimer(TIME_SEND_CARD_FINISH , 300) ;
		}*/
		return true;
	}
	case ASS_CALL_SCORE:
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(CallScoreStruct))
		{
			return true;
		}

		CallScoreStruct * pCallScore = (CallScoreStruct *)pData;
		UserCallScore(bDeskStation, pCallScore->iValue);
		return true;
	}
	case ASS_ROB_NT:						//抢地主
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(RobNTStruct))
		{
			return true;
		}

		if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE || GetGameStation() == GS_WAIT_SETGAME)
		{
			return true;
		}

		RobNTStruct * pRobNT = (RobNTStruct *)pData;
		if (NULL == pRobNT)
		{
			return true;
		}
		UserRobNT(pRobNT->byDeskStation, pRobNT->iValue);
		return TRUE;
	}
	case ASS_ADD_DOUBLE:					//加倍
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(AddDoubleStruct))
		{
			return true;
		}

		if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE || GetGameStation() == GS_WAIT_SETGAME)
		{
			return true;
		}

		AddDoubleStruct * pAddDouble = (AddDoubleStruct *)pData;
		UserAddDouble(pAddDouble->bDeskStation, pAddDouble->iValue);
		return TRUE;
	}
	case ASS_SHOW_CARD:	//亮牌
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(ShowCardStruct))
		{
			return true;
		}

		if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE || GetGameStation() == GS_WAIT_SETGAME)
		{
			return true;
		}

		ShowCardStruct * pShowCard = (ShowCardStruct *)pData;
		UserShowCard(pShowCard->bDeskStation, pShowCard->iValue);
		return true;
	}
	case ASS_OUT_CARD:		//用户出牌
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(CMD_C_OutCard))
		{
			SendOutCardError(bDeskStation, 1);
			return true;
		}

		if (GetGameStation() != GS_PLAY_GAME)
		{
			SendOutCardError(bDeskStation, 2);
			return true;
		}

		CMD_C_OutCard * pOutCard = (CMD_C_OutCard *)pData;
		if (NULL == pOutCard)
		{
			SendOutCardError(bDeskStation, 5);
			return true;
		}
		UserOutCard(bDeskStation, pOutCard->iCardList, pOutCard->iCardCount);
		return true;
	}
	case ASS_AUTO:			//托管
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(AutoStruct))
		{
			return true;
		}

		AutoStruct * pAuto = (AutoStruct*)pData;
		UserSetAuto(bDeskStation, pAuto->bAuto);
		return true;
	}
	case ASS_HAVE_THING:	//有事请求离开
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(HaveThingStruct))
		{
			return true;
		}

		HaveThingStruct * pThing = (HaveThingStruct *)pData;
		UserHaveThing(bDeskStation, pThing->szMessage);
		return true;
	}
	case ASS_LEFT_RESULT: //同意用户离开
	{
		if (bWatchUser)
		{
			return true;
		}
		if (size != sizeof(LeaveResultStruct))
		{
			return true;
		}

		LeaveResultStruct * pLeave = (LeaveResultStruct *)pData;
		ArgeeUserLeft(bDeskStation, pLeave->bArgeeLeave);
		return true;
	}
	default:
	{
		break;
	}
	}
	return __super::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
}

//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	KillTimer(uTimerID);
	switch (uTimerID)
	{
	case TIME_START_GAME:
	{
		if (GetGameStation() == GS_WAIT_NEXT || GetGameStation() == GS_WAIT_ARGEE)
		{
			StartGame();
		}
		else
		{
			KillTimer(TIME_START_GAME);
		}
		break;
	}
	case TIME_SEND_CARD:				//發牌
	{
		if (GetGameStation() == GS_SEND_CARD)
		{
			SendCard();
		}
		else KillTimer(TIME_SEND_CARD);
		break;
	}
	case TIME_SEND_ALL_CARD:	//准备发牌计时器	DWJ
	{
		if (GetGameStation() == GS_SEND_CARD)
		{
			SendAllCard();
		}
		else
		{
			KillTimer(TIME_SEND_ALL_CARD);
		}
		break;
	}
	case TIME_SEND_CARD_ANI:		///客户端发牌定时器		DWJ
	{
		KillTimer(TIME_SEND_CARD_ANI);
		SetTimer(TIME_SEND_CARD_FINISH, 300);
		break;
	}
	case  TIME_SEND_CARD_FINISH:	//发牌结束计时器	DWJ
	{
		KillTimer(TIME_SEND_CARD_FINISH);
		SendCardFinish();
		break;
	}
	case TIME_JIAO_FEN:                // 叫分定时器
	{
		KillTimer(TIME_JIAO_FEN);
		if (!IsGoldRoom())
		{
			if (GS_FLAG_CALL_SCORE == m_iGameFlag)
			{
				// 自动不叫分
				UserCallScore(m_bCurrentCallScore, 0);
				break;
			}
		}
		if (GS_WAIT_BACK == GetGameStation() && GS_FLAG_CALL_SCORE == m_iGameFlag)
		{
			// 自动不叫分
			UserCallScore(m_bCurrentCallScore, 0);
		}
		break;
	}
	case TIME_WAIT_NEWTURN:				//新一輪
	{
		KillTimer(TIME_WAIT_NEWTURN);
		if (GetGameStation() == GS_PLAY_GAME)
			NewPlayTurn(m_iNowBigPeople);
		break;
	}
	case TIME_ADD_DOUBLE:		//加倍计时器	DWJ
	{
		if (GetGameStation() == GS_WAIT_BACK)
		{
			AddDoubleResult();
		}
		else
		{
			KillTimer(TIME_ADD_DOUBLE);
		}
		break;
	}
	case TIME_ROB_NT:	//玩家叫地主、抢地主计时器	DWJ
	{
		KillTimer(TIME_ROB_NT);
		if (GetGameStation() == GS_WAIT_BACK)
		{
			UserRobNT(m_iCurrentRobPeople, 0);
		}
		break;
	}
	case TIME_OUT_CARD:
	{
		KillTimer(TIME_OUT_CARD);
		if (!IsGoldRoom())
		{
			////托管出牌，并自动进入托管状态  修改房卡模式托管后无反应 2017/12/20
			//int iTemp = m_iOutCardPeople;
			//UserAutoOutCard(m_iOutCardPeople);
			break;
		}
		if (GetGameStation() == GS_PLAY_GAME)
		{
			//m_iTimeOutNoCardOut为1表示不出牌，会导致托管后无反应
			if (1 != m_iTimeOutNoCardOut)//修改金币场托管后无反应  2017/12/21
			{
				break;
			}
			else
			{
				//托管出牌，并自动进入托管状态
				int iTemp = m_iOutCardPeople;
				UserAutoOutCard(m_iOutCardPeople);
				UserSetAuto(iTemp, true);
			}
		}
		break;
	}
	case TIME_GAME_FINISH:				//結束
	{
		KillTimer(TIME_GAME_FINISH);
		if (GetGameStation() == GS_PLAY_GAME)
			GameFinish(0, GF_NORMAL);
		break;
	}
	case TIME_SHOW_CARD:	//加倍计时器	DWJ
	{
		KillTimer(TIME_SHOW_CARD);
		if (GetGameStation() == GS_WAIT_BACK)
		{
			UserShowCard(m_iUpGradePeople, 0);
		}
		break;
	}
	}
	return __super::OnTimer(uTimerID);
}

//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if (!IsGoldRoom())
	{
		GetSpecialRule();
	}

	/// 玩家重连，表示不在托管状态，因此在这里设置为不托管
	if (!bWatchUser)
	{
		m_bAuto[bDeskStation] = false;
	}
	m_bUserNetCut[bDeskStation] = false;

	switch (GetGameStation())
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
	case GS_WAIT_NEXT:			//等待下一盘游戏开始
	{
		int iUserCount = 0;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			iUserCount += (GetUserIDByDeskStation(i) > 0);
		}
		if (iUserCount == 1)
		{
			ReSetGameState(GFF_FORCE_FINISH);
		}
		GameStation_2 GameStation;
		::memset(&GameStation, 0, sizeof(GameStation));
		//游戏版本核对
		GameStation.iVersion = DEV_HEIGHT_VERSION;			//游戏高版本
		GameStation.iVersion2 = DEV_LOW_VERSION;			//低版本
		//游戏内容设置
		GameStation.iCardShape = m_iCardShape;						//牌型设置
		//辅助时间
		GameStation.iBeginTime = m_iBeginTime;				//游戏开始时间设置
		GameStation.iThinkTime = m_iThinkTime;				//游戏思考时间
		GameStation.iCallScoreTime = m_iCallScoreTime;			//叫分时间
		GameStation.iRobNTTime = m_iRobNTTime;
		GameStation.iAddDoubleTime = m_iAddDoubleTime;			//叫分时间
		//房间倍数
		GameStation.iDeskBasePoint = GetDeskBasePoint();	//桌子倍数
		GameStation.iRoomBasePoint = GetRoomMul();	//房间倍数
		GameStation.iRunPublish = GetRunPublish();			//逃跑扣分

		GameStation.iGameMutiple = m_iBaseMult;
		GameStation.iAddDoubleLimit = m_iAddDoubleLimit;
		GameStation.iGameMaxLimit = m_iGameMaxLimit;
		GameStation.iFengDing = m_iFengDing;
		GameStation.iRunTime = m_iRunTime;
		if (m_iRunGameCount == 0)
		{
			GameStation.iRemainRunTime = m_iRunTime;
		}
		else
		{
			GameStation.iRemainRunTime = int(m_iGameEndTime - GetCurTime());
		}

		//memcpy(GameStation.bUserReady ,m_bUserReady , sizeof(GameStation.bUserReady )) ;
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &GameStation, sizeof(GameStation));

		break;
	}
	case GS_SEND_CARD:		//发牌状态
	case GS_WAIT_BACK:		//等待埋底牌状态
	{
		//发送取消机器人托管
		GameStation_3 GameStation;
		::memset(&GameStation, 0, sizeof(GameStation));
		//游戏版本核对
		GameStation.iVersion = DEV_HEIGHT_VERSION;			//游戏高版本
		GameStation.iVersion2 = DEV_LOW_VERSION;			//低版本
		//辅助时间
		GameStation.iBeginTime = m_iBeginTime;				//游戏开始时间设置
		GameStation.iThinkTime = m_iThinkTime;				//游戏思考时间
		GameStation.iCallScoreTime = m_iCallScoreTime;			//叫分时间
		GameStation.iRobNTTime = m_iRobNTTime;
		GameStation.iAddDoubleTime = m_iAddDoubleTime;			//叫分时间
		//房间倍数
		GameStation.iDeskBasePoint = GetDeskBasePoint();	//桌子倍数
		GameStation.iRoomBasePoint = GetRoomMul();	//房间倍数
		GameStation.iRunPublish = GetRunPublish();			//逃跑扣分

		GameStation.iCardShape = m_iCardShape;						//牌型设置
		GameStation.iGameMutiple = m_iBaseMult;
		GameStation.iAddDoubleLimit = m_iAddDoubleLimit;
		GameStation.iGameMaxLimit = m_iGameMaxLimit;

		GameStation.iUpGradePeople = m_iUpGradePeople;
		GameStation.iGameFlag = m_iGameFlag;

		memcpy(GameStation.bCanleave, m_bCanleave, sizeof(m_bCanleave));
		memcpy(GameStation.bAuto, m_bAuto, sizeof(m_bAuto));
		GameStation.iCallScorePeople = m_bCurrentCallScore;
		GameStation.byJiaoFenSFTime = m_iCallScoreTime + 1 - (GetCurTime() - m_iJiaoFenBeginTime) > 0 ?
			m_iCallScoreTime + 1 - (GetCurTime() - m_iJiaoFenBeginTime) : m_iCallScoreTime;
		int iPos = 0;
		ZeroMemory(&GameStation.iUserCardList, sizeof(GameStation.iUserCardList));
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			GameStation.iCallScore[i] = m_iCallScore[i];
			GameStation.iUserCardCount[i] = m_iUserCardCount[i];
			/// 只发自己的手牌，Modified by zxd 20100314
			if (bDeskStation == i || m_GameMutiple.sMingPaiMutiple[i] > 0)
			{
				::CopyMemory(&GameStation.iUserCardList[iPos], m_iUserCard[i], sizeof(BYTE)*m_iUserCardCount[i]);
			}
			iPos += m_iUserCardCount[i];
		}

		GameStation.iBackCardCount = m_iBackCount;
		memset(&GameStation.iGameBackCard, 0, sizeof(GameStation.iGameBackCard));
		memcpy(GameStation.bIsCall, m_bIsCall, sizeof(GameStation.bIsCall));
		switch (m_iGameFlag)
		{
		case  GS_FLAG_ROB_NT:
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_iFirstRobNt != 255)
				{
					if (m_iRobStation[i] == 255) ///不抢
					{
						GameStation.iRobNT[i] = 2; //不抢地主
					}
					else if (m_iRobStation[i] > 0 && m_GameMutiple.sRobNtMutiple[bDeskStation] > 0)
					{
						GameStation.iRobNT[i] = 3; //抢地主
					}
					else
					{
						GameStation.iRobNT[i] = -1;
					}
				}
				else
				{
					if (m_iRobStation[i] == 255) ///不叫
					{
						GameStation.iRobNT[i] = 0; //不叫地主
					}
					else if (m_iRobStation[i] > 0) ///叫地主
					{
						GameStation.iRobNT[i] = 3; ///叫地主
					}
					else ///没有叫
					{
						GameStation.iRobNT[i] = -1; //没有叫过地主
					}
				}
			}

			GameStation.iCurOperator = m_iCurrentRobPeople;  ///当前叫地主的人

			break;
		}
		case  GS_FLAG_ADD_DOUBLE:
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_iAddStation[i] == 255)  //不加倍
				{
					GameStation.iUserDoubleValue[i] = -1;
				}
				else
				{
					GameStation.iUserDoubleValue[i] = m_iAddStation[i];
				}
			}
			::CopyMemory(&GameStation.iGameBackCard, m_iBackCard, sizeof(BYTE)*m_iBackCount);
			GameStation.iUpGradePeople = m_iUpGradePeople;
			break;
		}
		case  GS_FLAG_SHOW_CARD:
		{
			::CopyMemory(&GameStation.iGameBackCard, m_iBackCard, sizeof(BYTE)*m_iBackCount);

			GameStation.iUpGradePeople = m_iUpGradePeople;
			break;
		}
		default:
		{
			break;
		}
		}

		GameStation.gameMutiple = m_GameMutiple;
		m_Logic.SetGameTask(GameStation.gameTask);
		GameStation.iFengDing = m_iFengDing;
		GameStation.iRunTime = m_iRunTime;
		GameStation.iRemainRunTime = int(m_iGameEndTime - GetCurTime());;
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &GameStation, sizeof(GameStation));

		break;
	}
	case GS_PLAY_GAME:	//游戏中状态
	{
		GameStation_4 GameStation;
		::memset(&GameStation, 0, sizeof(GameStation));
		//游戏版本核对
		GameStation.iVersion = DEV_HEIGHT_VERSION;			//游戏高版本
		GameStation.iVersion2 = DEV_LOW_VERSION;			//低版本
		//游戏内容设置
		GameStation.iCardShape = m_iCardShape;						//牌型设置
		//辅助时间
		GameStation.iBeginTime = m_iBeginTime;				//游戏开始时间设置
		GameStation.iThinkTime = m_iThinkTime;				//游戏思考时间
		GameStation.iCallScoreTime = m_iCallScoreTime;			//叫分时间
		GameStation.iRobNTTime = m_iRobNTTime;
		GameStation.iAddDoubleTime = m_iAddDoubleTime;			//叫分时间
		//房间倍数
		GameStation.iDeskBasePoint = GetDeskBasePoint();	//桌子倍数
		GameStation.iRoomBasePoint = GetRoomMul();	//房间倍数
		GameStation.iRunPublish = GetRunPublish();			//逃跑扣分

		GameStation.iGameMutiple = m_iBaseMult;
		GameStation.iAddDoubleLimit = m_iAddDoubleLimit;
		GameStation.iGameMaxLimit = m_iGameMaxLimit;

		GameStation.iUpGradePeople = m_iUpGradePeople;

		GameStation.iOutCardPeople = m_iOutCardPeople;
		GameStation.iFirstOutPeople = m_iFirstOutPeople;
		GameStation.iBigOutPeople = m_iNowBigPeople;
		memcpy(GameStation.iCallScoreResult, m_iCallScore, sizeof(GameStation.iCallScoreResult));
		//游戏倍数	
		memcpy(GameStation.bCanleave, m_bCanleave, sizeof(m_bCanleave));
		memcpy(GameStation.bAuto, m_bAuto, sizeof(m_bAuto));

		memcpy(GameStation.bPass, m_byPass, sizeof(GameStation.bPass));
		memcpy(GameStation.bLastTurnPass, m_byLastTurnPass, sizeof(GameStation.bLastTurnPass));

		//设置各家手中牌
		int iPos = 0;
		ZeroMemory(&GameStation.iUserCardList, sizeof(GameStation.iUserCardList));
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			//设置用户手中牌
			GameStation.iUserCardCount[i] = m_iUserCardCount[i];
			if (i == bDeskStation || m_GameMutiple.sMingPaiMutiple[i] > 0)
			{
				::CopyMemory(&GameStation.iUserCardList[iPos], m_iUserCard[i], sizeof(BYTE)*m_iUserCardCount[i]);
			}
			iPos += m_iUserCardCount[i];
		}

		//设置用户桌面牌牌
		GameStation.bIsLastCard = m_bIsLastCard;

		GameStation.iBaseOutCount = m_iBaseOutCount;

		::CopyMemory(&GameStation.iBaseCardList, m_iDeskCard[m_iNowBigPeople], sizeof(BYTE)*m_iBaseOutCount);
		::CopyMemory(&GameStation.iDeskCardCount, m_iDeskCardCount, sizeof(GameStation.iDeskCardCount));
		::CopyMemory(GameStation.iDeskCardList, m_iDeskCard, sizeof(GameStation.iDeskCardList));

		::CopyMemory(&GameStation.iLastCardCount, m_iLastCardCount, sizeof(GameStation.iLastCardCount));
		::CopyMemory(GameStation.iLastOutCard, m_iLastOutCard, sizeof(GameStation.iLastOutCard));

		//是否不出
		GameStation.bIsPass = m_byteHitPass;

		GameStation.iBackCardCount = m_iBackCount;

		::CopyMemory(&GameStation.iGameBackCard, m_iBackCard, sizeof(BYTE)*m_iBackCount);

		GameStation.gameMutiple = m_GameMutiple;

		m_Logic.SetGameTask(GameStation.gameTask);

		/*GameStation.bJueTou				= m_bJueTou;
		GameStation.bSiDaiEr			= m_bSiDaiEr;
		GameStation.bSiDaiLiangDui		= m_bSiDaiLiangDui;*/
		GameStation.iFengDing = m_iFengDing;
		GameStation.iRunTime = m_iRunTime;
		GameStation.iRemainRunTime = int(m_iGameEndTime - GetCurTime());;

		GameStation.byOutCardSFTime = m_iThinkTime + 1 - (GetCurTime() - m_iOutCardnBeginTime) > 0 ?
			m_iThinkTime + 1 - (GetCurTime() - m_iOutCardnBeginTime) : m_iThinkTime;

		SendGameStation(bDeskStation, uSocketID, bWatchUser, &GameStation, sizeof(GameStation));

		break;
	}
	default:
		return false;
	}

	//超端验证
	SpuerExamine(bDeskStation);

	return true;
}

//清所有计时器
void CServerGameDesk::KillAllTimer()
{
	KillTimer(TIME_SEND_CARD);
	KillTimer(TIME_SEND_ALL_CARD);
	KillTimer(TIME_SEND_CARD_ANI);
	KillTimer(TIME_SEND_CARD_FINISH);
	KillTimer(TIME_ROB_NT);
	KillTimer(TIME_ADD_DOUBLE);
	KillTimer(TIME_OUT_CARD);
	KillTimer(TIME_WAIT_NEWTURN);
	KillTimer(TIME_GAME_FINISH);
	KillTimer(TIME_JIAO_FEN);
	KillTimer(TIME_SHOW_CARD);
	KillTimer(TIME_START_GAME);

}

//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	KillAllTimer();
	if ((bLastStation == GFF_FORCE_FINISH) || (bLastStation == GF_SALE))
	{
		::memset(m_iWinPoint, 0, sizeof(m_iWinPoint));
		m_iBeenPlayGame = 0;
		m_iDealPeople = -1;
	}

	m_iPrepareNT = 255;		//预备庄家
	m_iCurrentRobPeople = 255;
	m_iFirstRobNt = 255;
	m_iFirstShow = 255;
	m_iUpGradePeople = -1;

	m_iFirstCutPeople = 255;

	for (int i = 0;i < PLAY_COUNT; i++)
	{
		m_iAwardPoint[i] = 0;
	}
	m_iGameFlag = GS_FLAG_NORMAL;
	m_bCurrentCallScore = 255;
	m_iNtFirstCount = 0;		//庄家出的第一手牌数量
	m_iLeaveArgee = 0;
	m_iBaseOutCount = 0;
	m_iNowBigPeople = -1;
	m_iOutCardPeople = -1;
	m_iSendCardPos = 0;
	m_bHaveSendBack = false;

	::memset(m_iDeskCardCount, 0, sizeof(m_iDeskCardCount));
	::memset(m_iUserCardCount, 0, sizeof(m_iUserCardCount));
	::memset(m_bAuto, 0, sizeof(m_bAuto));				//托管
	::memset(m_iCallScore, -1, sizeof(m_iCallScore));		//叫分
	::memset(m_bIsCall, false, sizeof(m_bIsCall));		//标记叫分
	::memset(m_iRobStation, 0, sizeof(m_iRobStation));
	::memset(m_iAddStation, 0, sizeof(m_iAddStation));
	memset(m_bUserReady, 0, sizeof(m_bUserReady));
	m_GameMutiple.IniData(m_iBaseMult);
	m_byMaxScoreStation = 255;
	memset(m_bPoChan, 0, sizeof(m_bPoChan));
	memset(m_bIsRobot, 0, sizeof(m_bIsRobot));
	return TRUE;
}

//服务端自动开始游戏
BOOL CServerGameDesk::StartGame()
{
	/*for(int i = 0; i < PLAY_COUNT ;i++)
	{
	if(GetUserIDByDeskStation(i) > 0)
	{
	UseAI(i);
	}
	}*/
	if (GameBegin(ALL_ARGEE))
	{
		KillTimer(TIME_START_GAME);
	}
	return TRUE;
}

//游戏开始
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (__super::GameBegin(bBeginFlag) == false)
	{
		GameFinish(0, GF_SALE);
		return false;
	}
	m_iRobotCount = 0;
	m_byRobotDesk1 = 255;
	m_byRobotDesk2 = 255;
	if (IsGoldRoom())
	{
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			GameUserInfo user;
			if (GetUserData(i, user) && user.isVirtual)
			{
				m_iRobotCount++;
				if (m_byRobotDesk1 == 255)
				{
					m_byRobotDesk1 = i;
				}
				else
				{
					m_byRobotDesk2 = i;
				}
				m_bIsRobot[i] = true;
			}
			else
			{
				m_bIsRobot[i] = false;
			}
		}
		if (m_iRobotCount >= PLAY_COUNT)
		{
			GameFinish(0, GF_SALE);
			return false;
		}
	}
	if (m_bRetryStart)
	{
		m_bRetryStart = !m_bRetryStart;
		//m_iRunGameCount--;
	}
	if (m_iRunTime > 0 && m_iRunGameCount == 1) //时间模式, 重第一局开始
	{
		m_iVipGameCount = INT_MAX;
		m_iGameBeginTime = GetCurTime();
		m_iGameEndTime = m_iGameBeginTime + m_iRunTime;
	}
	if (m_iRunTime > 0 && GetCurTime() > m_iGameEndTime) //时间模式
	{
		m_iVipGameCount = m_iRunGameCount;
		GameFinish(0, GF_SALE);
		return true;
	}

	m_icountleave = 0;
	m_iSendCardPos = 0;
	m_bySendFinishCount = 0;
	m_iBeenPlayGame++;
	SetGameStation(GS_SEND_CARD);

	::memset(m_iDeskCardCount, 0, sizeof(m_iDeskCardCount));
	::memset(m_iUserCardCount, 0, sizeof(m_iUserCardCount));
	::memset(m_bCanleave, 1, sizeof(m_bCanleave));
	::memset(m_bUserReady, 0, sizeof(m_bUserReady));
	memset(m_bUserNetCut, false, sizeof(m_bUserNetCut));
	memset(m_iLastCardCount, 0, sizeof(m_iLastCardCount));
	m_byMaxScoreStation = 255;

	::ZeroMemory(m_iLastOutCard, PLAY_COUNT * 45);
	m_bIsLastCard = false;
	m_byteHitPass = 0;

	//发送游戏开始消息
	GameBeginStruct TGameBegin;
	TGameBegin.iBeenPlayGame = m_iBeenPlayGame;
	TGameBegin.iPlayLimit = m_iLimitPlayGame;
	TGameBegin.iCardShape = m_iCardShape;
	memcpy(TGameBegin.byUserMingBase, m_GameMutiple.sMingPaiMutiple, sizeof(TGameBegin.byUserMingBase));

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &TGameBegin, sizeof(TGameBegin), MSG_MAIN_LOADER_GAME, ASS_GAME_BEGIN, 0);
	}
	//SendWatchData(m_byMaxPeople,&TGameBegin,sizeof(TGameBegin),MSG_MAIN_LOADER_GAME,ASS_GAME_BEGIN,0);

	//服务端分发所有玩家的扑克
	BYTE iCardArray[162];
	bool bOk = true;
	for (int i = 0; i < m_iPlayCount; i ++)
	{
		if (m_lastCardList[i] == 0 || m_lastCardList[i] == 255)
		{
			bOk = false;
			break;
		}
	}
	if (m_bNoXiPai && IsGoldRoom() && m_iRunGameCount % 10 == 0)
	{
		bOk = false;
	}
	if (m_bNoXiPai && bOk)
	{
		////不洗牌模式
		//m_Logic.MatchDDZ(iCardArray, m_iPlayCount);

		//进行一次切牌
		int iQiePaiIndex = CUtil::GetRandNum() % (m_iPlayCount / 2) + m_iPlayCount / 4;
		int iCardIndex = 0;
		for (int i = iQiePaiIndex; i < m_iPlayCount; i++)
		{
			iCardArray[iCardIndex++] = m_lastCardList[i];
		}
		for (int i = 0; i < iQiePaiIndex; i++)
		{
			iCardArray[iCardIndex++] = m_lastCardList[i];
		}

		//memcpy(iCardArray, m_lastCardList, sizeof(BYTE) * m_iPlayCount);
	}
	else
	{
		m_Logic.RandCard(iCardArray, m_iPlayCount, m_pDataManage->m_InitData.uRoomID, m_bHaveKing);
	}
	m_lastCardListIndex = 0;
	memset(m_lastCardList, 0, sizeof(m_lastCardList));

	//明牌
	srand(GetTickCount());
	m_bThrowoutCard = iCardArray[CUtil::GetRandNum() % (m_iSendCount/*m_iPlayCount - 3*/)];

	//拷贝玩家牌数据
	int iSendUserIndex = CUtil::GetRandNum() % PLAY_COUNT;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		::CopyMemory(m_iUserCard[iSendUserIndex], &iCardArray[m_iUserCount*i], sizeof(BYTE)*m_iUserCount);
		iSendUserIndex = (iSendUserIndex + 1) % PLAY_COUNT;
	}
	::CopyMemory(m_iBackCard, &iCardArray[m_iSendCount], sizeof(BYTE)*m_iBackCount);

	//启动机器人手牌增加程序
	if (m_iRobotCount == 2 && m_byRobotDesk1 != 255 && m_byRobotDesk2 != 255)
	{
		m_Logic.MatchMergeDDZ(m_iUserCard[m_byRobotDesk1], m_iUserCount, m_iUserCard[m_byRobotDesk2], m_iUserCount, m_bNoXiPai);
	}

	//初始化任务
	m_Logic.InitGameTask();

	//获取底牌牌型
	if (!m_Logic.GetBackCardType(m_iBackCard, m_iBackCount))
	{
		//随机一个任务
		m_Logic.GetRandTask(m_deskIdx);
	}
	GamePrepare();

	return TRUE;
}

//准备发牌
BOOL	CServerGameDesk::GamePrepare()
{
	SetTimer(TIME_SEND_ALL_CARD, 200);
	return TRUE;
}

//发送扑克给用户
BOOL CServerGameDesk::SendCard()
{
	if (m_iSendCardPos == m_iSendCount)
	{
		KillTimer(TIME_SEND_CARD);
		SendCardFinish();
		return TRUE;
	}
	//继续发送扑克(1次发两张)
	for (int i = 0; i < 2; i++)
	{
		BYTE bDeskStation = (m_iDealPeople + m_iSendCardPos) % PLAY_COUNT;
		SendCardStruct SendCard;
		SendCard.bDeskStation = bDeskStation;
		SendCard.bCard = m_iUserCard[bDeskStation][m_iSendCardPos / PLAY_COUNT];

		for (int i = 0; i < PLAY_COUNT; i++)
			SendGameData(i, &SendCard, sizeof(SendCard), MSG_MAIN_LOADER_GAME, ASS_SEND_CARD, 0);
		//SendWatchData(m_byMaxPeople,&SendCard,sizeof(SendCard),MSG_MAIN_LOADER_GAME,ASS_SEND_CARD,0);

		SendCardMsg(bDeskStation, SendCard.bCard);

		m_iUserCardCount[bDeskStation] ++;
		m_iSendCardPos++;
		if (m_iSendCardPos == m_iSendCount)
			break;
	}
	return TRUE;
}

//发牌消息
BOOL CServerGameDesk::SendCardMsg(BYTE bDeskStation, BYTE bCard)
{
	if (m_bFirstCallScore != 255)
		return TRUE;
	if (bCard == m_bThrowoutCard)
	{
		m_bFirstCallScore = bDeskStation;
		SendCardStruct sendcard;
		sendcard.bDeskStation = bDeskStation;
		sendcard.bCard = m_bThrowoutCard;
		for (int i = 0; i < PLAY_COUNT; i++)
			SendGameData(i, &sendcard, sizeof(sendcard), MSG_MAIN_LOADER_GAME, ASS_SEND_CARD_MSG, 0);

		//SendWatchData(m_byMaxPeople,&sendcard,sizeof(sendcard),MSG_MAIN_LOADER_GAME,ASS_SEND_CARD_MSG,0);
	}
	return TRUE;
}

//一次性发送所有牌
BOOL	CServerGameDesk::SendAllCard()
{
	SendAllStruct TSendAll;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_iUserCardCount[i] = m_iUserCount;
		TSendAll.iUserCardCount[i] = m_iUserCardCount[i];
	}

	//发送数据
	int iPos = 0;
	int iTempPos = 0;
	for (int i = 0;i < PLAY_COUNT; i++)
	{
		int iTempPos = 0;
		for (int j = 0; j < PLAY_COUNT;j++)
		{
			if (i == j || m_GameMutiple.sMingPaiMutiple[j] > 0 || m_bIsRobot[i])
			{
				::CopyMemory(&TSendAll.iUserCardList[iTempPos], m_iUserCard[j], sizeof(BYTE)*m_iUserCardCount[j]);
			}

			iTempPos += m_iUserCardCount[j];
		}

		iPos += m_iUserCardCount[i];
		SendGameData(i, &TSendAll, sizeof(TSendAll), MSG_MAIN_LOADER_GAME, ASS_SEND_ALL_CARD, 0);
		//SendWatchData(i,&TSendAll,sizeof(TSendAll),MSG_MAIN_LOADER_GAME,ASS_SEND_ALL_CARD,0);
		::ZeroMemory(&TSendAll.iUserCardList, sizeof(TSendAll.iUserCardList));
	}

	m_iSendCardPos++;
	if (m_iSendCardPos == 1)
	{
		KillTimer(TIME_SEND_ALL_CARD);
		SetTimer(TIME_SEND_CARD_ANI, m_iSendCardTime * 1000); ///给玩家发牌定时器
		return TRUE;
	}

	return FALSE;
}

//发牌结束
BOOL	CServerGameDesk::SendCardFinish()
{
	SendCardFinishStruct Tsendcardfinish;

	//发送发牌结束
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		memset(&Tsendcardfinish, 0, sizeof(Tsendcardfinish));
		//如果是超端 就把所有的牌数据发送过去
		if (IsSuperUser(i))
		{
			//所有玩家的牌数据
			memcpy(&Tsendcardfinish.byUserCard, m_iUserCard, sizeof(Tsendcardfinish.byUserCard));
			memcpy(&Tsendcardfinish.byUserCardCount, m_iUserCardCount, sizeof(Tsendcardfinish.byUserCardCount));
			//底牌数据
			memcpy(&Tsendcardfinish.byBackCardList, m_iBackCard, sizeof(Tsendcardfinish.byBackCardList));
		}

		SendGameData(i, &Tsendcardfinish, sizeof(Tsendcardfinish), MSG_MAIN_LOADER_GAME, ASS_SEND_FINISH, 0);
	}
	//SendWatchData(m_byMaxPeople,&Tsendcardfinish,sizeof(Tsendcardfinish),MSG_MAIN_LOADER_GAME,ASS_SEND_FINISH,0);
	BYTE byCard = 0;
	srand((unsigned)time(NULL));
	int iUser = CUtil::GetRandNum() % PLAY_COUNT;
	int iCardIndex = CUtil::GetRandNum() % m_iUserCount;
	byCard = m_iUserCard[iUser][iCardIndex];
	if (1 == m_iModel)//叫分模式
	{
		/*if (m_bFirstCallScore == 255)
		{*/
		//srand(GetTickCount());
		//m_bFirstCallScore = rand()%PLAY_COUNT;
		//随机生成一张牌（非底牌发给用户）,谁有谁先叫庄
		m_bFirstCallScore = (BYTE)iUser;
		//}
		SendCallScore(m_bFirstCallScore, byCard);
	}
	else if (0 == m_iModel)//抢地主模式
	{
		SetGameStation(GS_WAIT_BACK);
		if (m_iFirstShow != 255)  ///有明牌开始的人
		{
			m_iCurrentRobPeople = m_iFirstShow;
		}
		else
		{
			m_iCurrentRobPeople = iUser;
		}
		SendRobNT(m_iCurrentRobPeople, byCard);
	}
	return TRUE;
}

//发送给第一个叫分
BOOL CServerGameDesk::SendCallScore(BYTE bDeskStation, BYTE byCard)
{
	SetGameStation(GS_WAIT_BACK);
	m_iGameFlag = GS_FLAG_CALL_SCORE;
	CallScoreStruct CallScore;
	CallScore.iValue = (m_iPrepareNT == 255) ? -1 : m_iCallScore[m_iPrepareNT];
	CallScore.bDeskStation = bDeskStation;
	CallScore.bCallScoreflag = true;
	if (byCard != 255 && byCard != 0)
	{
		CallScore.byFirstCard = byCard;
	}
	memcpy(CallScore.bIsCall, m_bIsCall, sizeof(CallScore.bIsCall));
	m_bCurrentCallScore = bDeskStation;
	for (int i = 0; i < PLAY_COUNT; i++)
		SendGameData(i, &CallScore, sizeof(CallScore), MSG_MAIN_LOADER_GAME, ASS_CALL_SCORE, 0);
	//SendWatchData(m_byMaxPeople,&CallScore,sizeof(CallScore),MSG_MAIN_LOADER_GAME,ASS_CALL_SCORE,0);
	m_iJiaoFenBeginTime = GetCurTime();
	if (m_bAuto[bDeskStation])
	{
		SetTimer(TIME_JIAO_FEN, 1500);
	}
	else
	{
		SetTimer(TIME_JIAO_FEN, (m_iCallScoreTime + 2) * 1000);
	}
	return TRUE;
}

//叫分
BOOL CServerGameDesk::UserCallScore(BYTE bDeskStation, int iVal)
{
	if (bDeskStation != m_bCurrentCallScore)
	{
		return true;
	}
	if (-1 != m_iCallScore[bDeskStation])
	{
		return true;
	}
	// 结束叫分定时器
	KillTimer(TIME_JIAO_FEN);
	if (iVal == 3)						//有人直接叫3分
	{
		bool bFirstScore = true;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (-1 != m_iCallScore[i])
			{
				bFirstScore = false;
				break;
			}
		}
		if (bFirstScore)
		{
			//m_byMaxScoreStation=bDeskStation;
			/*CString lwlog;
			lwlog.Format("lwlog::首次叫三分位置m_byMaxScoreStation=%d",m_byMaxScoreStation);
			OutputDebugString(lwlog);*/
		}
	}
	if (iVal == 0)
	{
		m_iCallScore[bDeskStation] = 0;		//====某位置不叫分
	}
	else
	{
		if (iVal > 0 && iVal < 4)
		{
			m_bIsCall[iVal - 1] = true;
		}
		m_iCallScore[bDeskStation] = iVal;	//叫分类型
		m_iPrepareNT = bDeskStation;				//最后叫分者
		if (iVal == 3)						//有人直接叫3分
		{
			CallScoreStruct callscore;
			callscore.bDeskStation = bDeskStation;
			callscore.bCallScoreflag = FALSE;
			callscore.iValue = m_iCallScore[bDeskStation];		//当前叫分类型保存
			for (int i = 0; i < PLAY_COUNT; i++)
				SendGameData(i, &callscore, sizeof(callscore), MSG_MAIN_LOADER_GAME, ASS_CALL_SCORE_RESULT, 0);		//将叫分情况发给其他用户
			CallScoreFinish();
			return true;
		}
	}
	CallScoreStruct callscore;
	callscore.bDeskStation = bDeskStation;
	callscore.bCallScoreflag = FALSE;
	callscore.iValue = m_iCallScore[bDeskStation];		//当前叫分类型保存
	for (int i = 0; i < PLAY_COUNT; i++)
		SendGameData(i, &callscore, sizeof(callscore), MSG_MAIN_LOADER_GAME, ASS_CALL_SCORE_RESULT, 0);		//将叫分情况发给其他用户

	//SendWatchData(m_byMaxPeople,&callscore,sizeof(callscore),MSG_MAIN_LOADER_GAME,ASS_CALL_SCORE_RESULT,0);
	int iNextPeople = GetNextDeskStation(bDeskStation);
	// iNextPeople == m_bFirstCallScore ,这句代码的作用就是，没人叫3分，还让第一个叫分玩家确认叫分
	if (m_iCallScore[iNextPeople] == -1 /*|| (iNextPeople == m_bFirstCallScore && m_bIsCall[2] == false)*/)
	{
		SendCallScore(iNextPeople);
		return true;
	}
	m_iPrepareNT = m_bFirstCallScore;
	BYTE byStation = m_bFirstCallScore;
	int iCallScore = m_iCallScore[m_bFirstCallScore];
	for (int i = 0;i < PLAY_COUNT - 1;i++)
	{
		byStation = GetNextDeskStation(byStation);
		if (m_iCallScore[byStation] > iCallScore)
		{
			iCallScore = m_iCallScore[byStation];
			m_iPrepareNT = byStation;
		}
	}
	CallScoreFinish();
	return true;
}

//叫分结束
BOOL CServerGameDesk::CallScoreFinish()
{
	if (m_iPrepareNT == 255)	//没有人叫分.重新发牌
	{
		GameFinish(0, GF_NO_CALL_SCORE);
		return true;
	}
	if (m_iCallScore[m_iPrepareNT] == 0)	//没有人叫分.重新发牌
	{
		GameFinish(0, GF_NO_CALL_SCORE);
		return true;
	}
	m_iBaseMult = m_iCallScore[m_iPrepareNT];
	m_GameMutiple.IniData(m_iBaseMult);
	//用于抢地主位置结束控制
	m_iUpGradePeople = m_iPrepareNT;
	CallScoreStruct scoreresult;
	scoreresult.iValue = m_iCallScore[m_iPrepareNT];
	scoreresult.bDeskStation = m_iPrepareNT;
	scoreresult.bCallScoreflag = false;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &scoreresult, sizeof(scoreresult), MSG_MAIN_LOADER_GAME, ASS_CALL_SCORE_FINISH, 0);		//将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&scoreresult,sizeof(scoreresult),MSG_MAIN_LOADER_GAME,ASS_CALL_SCORE_FINISH,0);

	//发送底牌
	SendBackCard();
	return TRUE;
}

//发送抢地主消息
BOOL	CServerGameDesk::SendRobNT(BYTE bDeskStation, BYTE byCard)
{
	m_iGameFlag = GS_FLAG_ROB_NT;				//抢地主																
	//抢地主情况
	RobNTStruct  TRobNT;
	TRobNT.byDeskStation = bDeskStation;
	TRobNT.iValue = ((m_iFirstRobNt == 255) ? 0 : 1);
	TRobNT.byFirstCard = byCard;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &TRobNT, sizeof(TRobNT), MSG_MAIN_LOADER_GAME, ASS_ROB_NT, 0);
	}
	//SendWatchData(m_byMaxPeople,&TRobNT,sizeof(TRobNT),MSG_MAIN_LOADER_GAME,ASS_ROB_NT,0);

	if (m_bAuto[bDeskStation])
	{
		SetTimer(TIME_ROB_NT, 1500);
	}
	else
	{
		SetTimer(TIME_ROB_NT, (m_iRobNTTime + 3) * 1000);
	}
	return TRUE;
}

//玩家抢地主消息
BOOL	CServerGameDesk::UserRobNT(BYTE bDeskStation, int iValue)
{
	if (bDeskStation != m_iCurrentRobPeople)
	{
		return TRUE;
	}

	if (m_iRobStation[bDeskStation] > 1)
	{
		return TRUE;
	}

	KillTimer(TIME_ROB_NT);

	//抢地主结果情况
	if (iValue > 0)
	{
		m_iPrepareNT = bDeskStation;
		//新一预备庄家
		m_iRobStation[bDeskStation] += 1;
	}
	else
	{
		m_iRobStation[bDeskStation] = 255;
	}


	int iTempValue = 0;
	if (m_iFirstRobNt != 255)
	{
		if (iValue > 0)
		{
			iTempValue = 3;	//抢地主
			m_GameMutiple.sRobNtMutiple[bDeskStation] += 1;
		}
		else
		{
			iTempValue = 2;	//不抢地主
		}
	}
	else
	{
		iTempValue = iValue;
	}
	RobNTStruct  robnt;
	robnt.byDeskStation = bDeskStation;
	robnt.iValue = iTempValue;  // 0-叫地主状态 1-抢地主状态  2 - 不抢地主   3 - 抢地主
	robnt.byRobCount = m_GameMutiple.sRobNtMutiple[bDeskStation];

	for (int i = 0;i < PLAY_COUNT; i++)
	{
		SendGameData(i, &robnt, sizeof(robnt), MSG_MAIN_LOADER_GAME, ASS_ROB_NT_RESULT, 0);		  //将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&robnt,sizeof(robnt),MSG_MAIN_LOADER_GAME,ASS_ROB_NT_RESULT,0);
	//如果其他两个玩家都不叫地主 那么叫地主、抢地主结束
	if (m_iRobStation[(bDeskStation + 1) % PLAY_COUNT] == 255 && m_iRobStation[(bDeskStation + 2) % PLAY_COUNT] == 255)
	{
		RobNTFinish();
		return  TRUE;
	}

	if (m_iPrepareNT != 255 && m_iRobStation[(m_iPrepareNT + 1) % PLAY_COUNT] == 255 && m_iRobStation[(m_iPrepareNT + 2) % PLAY_COUNT] == 255)
	{
		RobNTFinish();
		return  TRUE;
	}
	//如果当前操作的玩家==第一个叫地主的玩家
	if (bDeskStation == m_iFirstRobNt)
	{
		RobNTFinish();
		return TRUE;
	}

	///第一个 选择抢地主的人
	if (iValue > 0 && m_iFirstRobNt == 255)
	{
		m_iFirstRobNt = bDeskStation;
	}

	BYTE bDesk = GetRobNtDeskStation(bDeskStation);
	if (bDesk == bDeskStation)
	{
		RobNTFinish();
		return TRUE;
	}

	if (iValue > 0 && false == m_bRobnt)
	{
		RobNTFinish();
		return TRUE;
	}

	//记录当前操作的玩家
	m_iCurrentRobPeople = bDesk;
	SendRobNT(bDesk);

	return TRUE;
}

//获取一个抢地主玩家
BYTE CServerGameDesk::GetRobNtDeskStation(BYTE bDeskStation)
{
	for (int i = GetNextDeskStation(bDeskStation); ; i = GetNextDeskStation(i))
	{
		if (m_iRobStation[i] != 255)
		{
			return i;
		}
	}
	return bDeskStation;
}

//抢地主结束
BOOL	CServerGameDesk::RobNTFinish()
{
	//三个人叫地主结束 如果没有人叫地主 那么游戏重新开始
	if (m_iPrepareNT == 255)
	{
		//第一个明牌的玩家就是地主
		if (m_iFirstShow != 255)
		{
			m_iPrepareNT = m_iFirstShow;
		}
		else
		{
			GameFinish(0, GF_NO_CALL_SCORE);
			return TRUE;
		}
	}

	m_iUpGradePeople = m_iPrepareNT;

	//抢地主是否成功
	RobNTStruct  robresult;
	robresult.byDeskStation = m_iPrepareNT;
	robresult.iValue = -1;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &robresult, sizeof(robresult), MSG_MAIN_LOADER_GAME, ASS_ROB_NT_FINISH, 0);
	}
	//SendWatchData(m_byMaxPeople,&robresult,sizeof(robresult),MSG_MAIN_LOADER_GAME,ASS_ROB_NT_FINISH,0);

	//抢地主结束发底牌
	SendBackCard();
	return TRUE;
}

//发送底牌
BOOL	CServerGameDesk::SendBackCard()
{
	if (m_bHaveSendBack)
	{
		return TRUE;
	}
	m_bHaveSendBack = true;
	m_iUpGradePeople = m_iPrepareNT;
	BackCardExStruct BackCard;
	memset(&BackCard, 0, sizeof(BackCard));
	BackCard.iGiveBackPeople = m_iUpGradePeople;
	BackCard.iBackCardCount = m_iBackCount;
	m_Logic.SetGameTask(BackCard.gameTask);
	BackCard.gameTask.byBackCardMutiple = 1;//m_Logic.GetBackCardMytiple() ; 
	m_GameMutiple.sBackCardMutiple = 1;//m_Logic.GetBackCardMytiple()  ; 
	::CopyMemory(&m_iUserCard[m_iUpGradePeople][m_iUserCardCount[m_iUpGradePeople]], m_iBackCard, sizeof(BYTE)*BackCard.iBackCardCount);
	m_iUserCardCount[m_iUpGradePeople] += BackCard.iBackCardCount;
	::CopyMemory(BackCard.iBackCard, m_iBackCard, sizeof(BYTE)*BackCard.iBackCardCount);
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &BackCard, sizeof(BackCard), MSG_MAIN_LOADER_GAME, ASS_BACK_CARD_EX, 0);
	}
	//SendWatchData(m_byMaxPeople,&BackCard,sizeof(BackCard),MSG_MAIN_LOADER_GAME,ASS_BACK_CARD_EX,0);
	if (m_bAdddouble)
	{
		SendAddDouble();
	}
	else
	{
		AddDoubleFinish();
	}
	return TRUE;
}

//发送加倍消息
BOOL	CServerGameDesk::SendAddDouble()
{
	m_iGameFlag = GS_FLAG_ADD_DOUBLE;				//加倍																
	m_iRecvMsg = 0;
	SetTimer(TIME_ADD_DOUBLE, (m_iAddDoubleTime + 1) * 1000);



	//抢地主情况
	AddDoubleStruct  adddouble;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_bAuto[i])
		{
			UserAddDouble(i, 0);
			continue;
		}
		memset(&adddouble, 0, sizeof(adddouble));
		adddouble.bDeskStation = i;
		if (IsPrivateRoom())
		{
			adddouble.iValue = 1;
		}
		else
		{
			long long llResNums = 0;
			if (!GetRoomResNums(i, llResNums))
			{
				continue;
			}

			if (llResNums > 5 * m_iAddDoubleLimit)
			{
				adddouble.iValue = 2;
			}
			else if (llResNums > m_iAddDoubleLimit)
			{
				adddouble.iValue = 1;
			}
			else
			{
				adddouble.iValue = 0;
			}
		}

		SendGameData(i, &adddouble, sizeof(adddouble), MSG_MAIN_LOADER_GAME, ASS_ADD_DOUBLE, 0);		//将叫分情况发给其他用户
		if (adddouble.iValue == 0)
		{
			UserAddDouble(i, 0);
		}
	}
	return  TRUE;
}

//玩家加倍
BOOL	CServerGameDesk::UserAddDouble(BYTE bDeskStation, int iVal)
{
	if (m_iAddStation[bDeskStation] > 0)
	{
		return TRUE;
	}

	m_iRecvMsg++;

	if (iVal > 0)
	{
		m_GameMutiple.sAddGameMutiple[bDeskStation] = 1;
		m_iAddStation[bDeskStation] = iVal;
	}
	else
	{
		m_iAddStation[bDeskStation] = 255;
	}

	//发送加倍结果
	AddDoubleStruct  adddouble;
	adddouble.bDeskStation = bDeskStation;
	adddouble.iValue = iVal;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &adddouble, sizeof(adddouble), MSG_MAIN_LOADER_GAME, ASS_ADD_DOUBLE_RESULT, 0);		//将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&adddouble,sizeof(adddouble),MSG_MAIN_LOADER_GAME,ASS_ADD_DOUBLE_RESULT,0);

	if (m_iRecvMsg >= 3)
	{
		AddDoubleResult();
	}

	return TRUE;
}

//加倍结果
BOOL	CServerGameDesk::AddDoubleResult()
{
	KillTimer(TIME_ADD_DOUBLE);
	AddDoubleFinish();
	return TRUE;
}

//加倍结束
BOOL	CServerGameDesk::AddDoubleFinish()
{
	//抢地主结果情况
	AddDoubleStruct  adddouble;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		::memset(&adddouble, 0, sizeof(adddouble));
		adddouble.bDeskStation = i;
		adddouble.iValue = m_GameMutiple.sAddGameMutiple[i];
		SendGameData(i, &adddouble, sizeof(adddouble), MSG_MAIN_LOADER_GAME, ASS_ADD_DOUBLE_FINISH, 0);		//将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&adddouble,sizeof(adddouble),MSG_MAIN_LOADER_GAME,ASS_ADD_DOUBLE_FINISH,0);
	if (m_bShowcard && !m_GameMutiple.sMingPaiMutiple[m_iUpGradePeople])
	{
		SendShowCard();
	}
	else
	{
		ShowCardFinish();
	}
	return true;
}

//亮牌
BOOL	CServerGameDesk::SendShowCard()
{
	m_iGameFlag = GS_FLAG_SHOW_CARD;				//亮牌																
	ShowCardStruct show;
	show.bDeskStation = m_iUpGradePeople;

	SetTimer(TIME_SHOW_CARD, m_iAddDoubleTime * 1000);

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &show, sizeof(show), MSG_MAIN_LOADER_GAME, ASS_SHOW_CARD, 0);		//将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&show,sizeof(show),MSG_MAIN_LOADER_GAME,ASS_SHOW_CARD,0);
	return TRUE;
}

//明牌消息
BOOL	CServerGameDesk::UserShowCard(BYTE bDeskStation, int iValue)
{
	KillTimer(TIME_SHOW_CARD);
	if (iValue > 0)
	{
		m_GameMutiple.sMingPaiMutiple[bDeskStation] = iValue;

		if (255 == m_iFirstShow &&GS_FLAG_SHOW_CARD != m_iGameFlag)
		{
			m_iFirstShow = bDeskStation;
		}
	}

	ShowCardStruct showresult;
	::ZeroMemory(&showresult, sizeof(showresult));
	showresult.bDeskStation = bDeskStation;
	showresult.iValue = iValue;

	showresult.iCardCount = m_iUserCardCount[bDeskStation];
	memcpy(showresult.iCardList, m_iUserCard[bDeskStation], sizeof(BYTE) *m_iUserCardCount[bDeskStation]);

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &showresult, sizeof(showresult), MSG_MAIN_LOADER_GAME, ASS_SHOW_CARD_RESULT, 0);		//将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&showresult,sizeof(showresult),MSG_MAIN_LOADER_GAME,ASS_SHOW_CARD_RESULT,0);

	if (m_iGameFlag == GS_FLAG_SHOW_CARD)   ///庄家明牌
	{
		ShowCardFinish();
	}
	return TRUE;
}

//明牌结束
BOOL	CServerGameDesk::ShowCardFinish()
{
	m_iGameFlag = GS_FLAG_PLAY_GAME;

	ShowCardStruct showresult;
	::memset(&showresult, 0, sizeof(showresult));

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		SendGameData(i, &showresult, sizeof(showresult), MSG_MAIN_LOADER_GAME, ASS_SHOW_CARD_FINISH, 0);		//将叫分情况发给其他用户
	}
	//SendWatchData(m_byMaxPeople,&showresult,sizeof(showresult),MSG_MAIN_LOADER_GAME,ASS_SHOW_CARD_FINISH,0);

	BeginPlay();

	return TRUE;
}

//游戏开始
BOOL	CServerGameDesk::BeginPlay()
{
	KillAllTimer();
	//设置数据
	SetGameStation(GS_PLAY_GAME);
	m_iBaseOutCount = 0;
	m_iNowBigPeople = m_iOutCardPeople = m_iFirstOutPeople = m_iUpGradePeople;

	::memset(m_iDeskCardCount, 0, sizeof(m_iDeskCardCount));

	//排列扑克
	for (int i = 0; i < PLAY_COUNT; i++)
		m_Logic.SortCard(m_iUserCard[i], NULL, m_iUserCardCount[i]);
	//发送游戏开始消息
	BeginPlayStruct Begin;
	Begin.iOutDeskStation = m_iOutCardPeople;
	for (int i = 0;i < PLAY_COUNT; i++)
	{
		SendGameData(i, &Begin, sizeof(Begin), MSG_MAIN_LOADER_GAME, ASS_GAME_PLAY, 0);
	}
	//SendWatchData(m_byMaxPeople,&Begin,sizeof(Begin),MSG_MAIN_LOADER_GAME,ASS_GAME_PLAY,0);

	long userID = GetUserIDByDeskStation(m_iOutCardPeople);

	if (userID > 0)
	{
		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (NULL != pUser)
		{
			if (!pUser->IsOnline)
			{
				SetTimer(TIME_OUT_CARD, 1000);
			}
			else
			{
				if (m_bAuto[m_iOutCardPeople])
				{
					SetTimer(TIME_OUT_CARD, 1000);
				}
				else
				{
					m_iOutCardnBeginTime = GetCurTime() + 2;
					SetTimer(TIME_OUT_CARD, (m_iThinkTime + 5) * 1000);
				}
			}
		}
	}

	return TRUE;
}

//用户出牌
BOOL	CServerGameDesk::UserOutCard(BYTE bDeskStation, BYTE iOutCard[], int iCardCount)
{
	BYTE iOutCardShape = m_Logic.GetCardShape(iOutCard, iCardCount);
	/*CSingleLock singleLock(&m_Mutex);
	singleLock.Lock();*/

	if (bDeskStation != m_iOutCardPeople)
	{
		SendOutCardError(bDeskStation, 3);
		return true;
	}
	else
	{
		//当前最大出牌的玩家不能不出
		if (0 == iCardCount  && bDeskStation == m_iNowBigPeople)
		{
			SendOutCardError(bDeskStation, 3);
			return true;
		}
	}
	//处理服务器牌
	if (iCardCount > 0)
	{
		//增加出牌逻辑合法性判断zht 2010-03-23
		if (!m_Logic.CanOutCard(iOutCard, iCardCount, m_iDeskCard[m_iNowBigPeople], m_iDeskCardCount[m_iNowBigPeople],
			m_iUserCard[bDeskStation], m_iUserCardCount[bDeskStation], bDeskStation == m_iNowBigPeople))
		{
			ERROR_LOG("=========================牌型错误=========================");
			SendOutCardError(bDeskStation, 6);
			return TRUE;
		}
		if (m_Logic.RemoveCard(iOutCard, iCardCount, m_iUserCard[bDeskStation], m_iUserCardCount[bDeskStation]) == 0)
		{
			ERROR_LOG("=========================删除牌出错===========================");
			SendOutCardError(bDeskStation, 4);
			return TRUE;
		}
		if (0 == m_iNtFirstCount)		//地主出的第一手牌
		{
			m_iNtFirstCount = iCardCount;
		}
		m_iNowBigPeople = bDeskStation;
		m_iBaseOutCount = iCardCount;
		m_iUserCardCount[bDeskStation] -= iCardCount;
		//记录出牌信息
		m_iDeskCardCount[bDeskStation] = iCardCount;
		::CopyMemory(m_iDeskCard[bDeskStation], iOutCard, sizeof(BYTE)*iCardCount);
		m_byPass[bDeskStation] = false;

		//记录出牌
		memcpy(&m_lastCardList[m_lastCardListIndex], iOutCard, sizeof(BYTE)*iCardCount);
		m_lastCardListIndex += iCardCount;
	}
	else
	{
		//某一位置什麼牌也不出
		m_iDeskCardCount[bDeskStation] = iCardCount;
		memset(m_iDeskCard[bDeskStation], 0, sizeof(m_iDeskCard[bDeskStation]));

		m_byteHitPass |= (1 << bDeskStation);	//记录PASS
		m_byPass[bDeskStation] = true;
	}

	KillTimer(TIME_OUT_CARD);
	//发送玩家出牌结果
	CMD_S_OutCard UserOutResult;
	UserOutResult.byCurOutCardStation = bDeskStation;
	UserOutResult.byCardCount = iCardCount;
	::CopyMemory(UserOutResult.byCardList, iOutCard, sizeof(BYTE)*iCardCount);
	UserOutResult.byHandCardCount = m_iUserCardCount[bDeskStation];
	for (int i = 0;i < PLAY_COUNT; i++)
	{
		if (i == bDeskStation)
		{
			::CopyMemory(UserOutResult.byHandCard, m_iUserCard[bDeskStation], sizeof(BYTE)*m_iUserCardCount[bDeskStation]);
		}
		else
		{
			memset(UserOutResult.byHandCard, 0, sizeof(UserOutResult.byHandCard));
		}
		SendGameData(i, &UserOutResult, sizeof(UserOutResult), MSG_MAIN_LOADER_GAME, ASS_OUT_CARD_RESULT, 0);
	}
	//SendWatchData(m_byMaxPeople,&UserOutResult,sizeof(UserOutResult),MSG_MAIN_LOADER_GAME,ASS_OUT_CARD_RESULT,0);

	//是否为加分牌型
	IsAwardPoin(iOutCard, iCardCount, bDeskStation);

	if (m_Logic.IsKingBomb(iOutCard, iCardCount) || m_Logic.IsBomb(iOutCard, iCardCount))
	{
		m_tZongResult.iBombCount[bDeskStation]++;
	}

	//====判断是否这个玩家是否出完牌
	if (m_iUserCardCount[bDeskStation] <= 0)
	{
		m_bFirstCallScore = bDeskStation;
		m_Logic.SetLastCardData(iOutCard, iCardCount);

		m_iDealPeople = bDeskStation;
		//出完牌各參數設置
		SetTimer(TIME_GAME_FINISH, 1000);
		return true;

	}
	//所出的牌最大,重新开始新一轮
	if (m_Logic.IsKingBomb(iOutCard, iCardCount))
	{
		m_iNowBigPeople = m_iFirstOutPeople = m_iOutCardPeople = bDeskStation;
		IsNewTurn();
		return true;
	}

	//计算下一轮出牌者
	m_iOutCardPeople = GetNextDeskStation(bDeskStation);//(bDeskStation+1)%PLAY_COUNT;

	for (int i = m_iOutCardPeople;;i = GetNextDeskStation(i))
	{
		m_iOutCardPeople = i;				//当前出牌者
		if (IsNewTurn())
			return true;
		//当前出牌者有牌未出
		if (m_iUserCardCount[i] > 0)
			break;
	}

	CMD_S_NotifyOutCard UserOut;
	UserOut.byNextDeskStation = (BYTE)m_iOutCardPeople;
	for (int i = 0;i < m_byMaxPeople;i++)
	{
		SendGameData(i, &UserOut, sizeof(UserOut), MSG_MAIN_LOADER_GAME, ASS_OUT_CARD, 0);
	}
	//SendWatchData(m_byMaxPeople,&UserOut,sizeof(UserOut),MSG_MAIN_LOADER_GAME,ASS_OUT_CARD,0);

	long userID = GetUserIDByDeskStation(m_iOutCardPeople);

	if (userID > 0)
	{
		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (NULL != pUser)
		{
			if (!pUser->IsOnline)
			{
				// 自动出牌
				SetTimer(TIME_OUT_CARD, 1000);
			}
			else
			{
				if (m_bAuto[m_iOutCardPeople])
				{
					SetTimer(TIME_OUT_CARD, 1000);
				}
				else
				{
					m_iOutCardnBeginTime = GetCurTime();
					SetTimer(TIME_OUT_CARD, (m_iThinkTime + 1) * 1000);
				}
			}
		}
	}

	return TRUE;
}

//获取下一个玩家位置
BYTE CServerGameDesk::GetNextDeskStation(BYTE bDeskStation)
{
	if (!m_bTurnRule)//顺时针
	{
		return (bDeskStation + 1) % PLAY_COUNT;
	}
	//逆时针
	return 	(bDeskStation + (PLAY_COUNT - 1)) % PLAY_COUNT;
}

//是否為新一輪
BOOL CServerGameDesk::IsNewTurn()
{
	if (m_iOutCardPeople == m_iFirstOutPeople)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
			SendGameData(i, MSG_MAIN_LOADER_GAME, ASS_ONE_TURN_OVER, 0);

		//SendWatchData(m_byMaxPeople,MSG_MAIN_LOADER_GAME,ASS_ONE_TURN_OVER,0);
	}
	if (m_iOutCardPeople == m_iNowBigPeople)			//最先出牌者
	{
		//m_iOutCardPeople = -1;
		SetTimer(TIME_WAIT_NEWTURN, 100);
		return true;
	}
	return false;
}

//新一轮开始
BOOL CServerGameDesk::NewPlayTurn(BYTE bDeskStation)
{
	m_iBaseOutCount = 0;
	m_iOutCardPeople = m_iFirstOutPeople = m_iNowBigPeople = bDeskStation;
	//保存上一轮出牌情况
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_bIsLastCard = true;
		m_byteHitPass = 0;
		m_iLastCardCount[i] = m_iDeskCardCount[i];
		::CopyMemory(m_iLastOutCard[i], m_iDeskCard[i], sizeof(BYTE)*m_iLastCardCount[i]);
		::CopyMemory(m_byLastTurnPass, m_byPass, sizeof(m_byLastTurnPass));
	}
	::memset(m_iDeskCardCount, 0, sizeof(m_iDeskCardCount));
	::memset(m_byPass, 0, sizeof(m_byPass));
	NewTurnStruct turn;
	turn.bDeskStation = bDeskStation;
	for (int i = 0; i < PLAY_COUNT; i++)
		SendGameData(i, &turn, sizeof(turn), MSG_MAIN_LOADER_GAME, ASS_NEW_TURN, 0);
	//SendWatchData(m_byMaxPeople,&turn,sizeof(turn),MSG_MAIN_LOADER_GAME,ASS_NEW_TURN,0);

	long userID = GetUserIDByDeskStation(m_iOutCardPeople);

	if (userID > 0)
	{
		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (NULL != pUser)
		{
			if (!pUser->IsOnline)
			{
				//自动出牌
				SetTimer(TIME_OUT_CARD, 1000);
			}
			else
			{
				if (m_bAuto[m_iOutCardPeople])
				{
					SetTimer(TIME_OUT_CARD, 1000);
				}
				else
				{
					m_iOutCardnBeginTime = GetCurTime() + 2;
					SetTimer(TIME_OUT_CARD, (m_iThinkTime + 5) * 1000);
				}
			}
		}
	}
	return TRUE;
}

//所出牌中讨赏设置
BOOL CServerGameDesk::IsAwardPoin(BYTE iOutCard[], int iCardCount, BYTE bDeskStation)
{
	if (m_iFengDing > 0 && m_GameMutiple.sBombCount >= m_iFengDing)
	{
		//炸弹封顶了
		return true;
	}
	if (m_Logic.IsKingBomb(iOutCard, iCardCount) || m_Logic.IsBomb(iOutCard, iCardCount))
	{
		m_GameMutiple.sBombCount += 1;

		m_iAwardPoint[bDeskStation] += 1;

		AwardPointStruct award;
		//award.iAwardPoint = m_iAwardPoint[bDeskStation]; //delete zrr
		award.iAwardPoint = m_GameMutiple.sBombCount;		//add zrr
		award.bDeskStation = bDeskStation;

		//发送奖分情况
		for (int i = 0;i < PLAY_COUNT; i++)
			SendGameData(i, &award, sizeof(award), MSG_MAIN_LOADER_GAME, ASS_AWARD_POINT, 0);
		//SendWatchData(m_byMaxPeople,&award,sizeof(award),MSG_MAIN_LOADER_GAME,ASS_AWARD_POINT,0);
	}
	return true;
}

void CServerGameDesk::UpdateCalculateBoard()
{
	int iBigWinnerStation = 0;
	vector<int> iContinueWinCount;
	vector<int> temp;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		//m_tZongResult[i].byGameEndType = m_byGameEndType;
		m_tZongResult.byGameEndType[i] = m_byGameEndType;
		temp.clear();
		for (int j = 0; j < m_vecWinInfo[i].size();j++)
		{
			temp.push_back(m_vecWinInfo[i][j]);
		}

		//最高连胜
		iContinueWinCount.clear();
		for (auto ite = temp.begin(); ite != temp.end(); ite++)
		{
			//第一个赢，插入1
			if (ite == temp.begin())
			{
				if (*ite == true)
				{
					iContinueWinCount.push_back(1);
				}
			}
			//有两连续的，说明是连胜，连胜加1
			else if (*(ite - 1) == true && *ite == true)
			{
				iContinueWinCount.at(iContinueWinCount.size() - 1) += 1;
			}
			//前一个不赢这个赢，新插入一个1
			else if (*ite == true)
			{
				iContinueWinCount.push_back(1);
			}
		}

		for (auto ite = iContinueWinCount.begin(); ite != iContinueWinCount.end(); ite++)
		{
			if (m_tZongResult.iMaxContinueCount[i] < *ite)
			{
				//m_tZongResult[i].iMaxContinueCount = *ite;
				m_tZongResult.iMaxContinueCount[i] = *ite;
			}
		}
		//大赢家
		//m_tZongResult[i].bWinner = false;
		m_tZongResult.bWinner[i] = false;
		if (m_tZongResult.i64WinMoney[i] >= m_tZongResult.i64WinMoney[iBigWinnerStation])
		{
			iBigWinnerStation = i;
		}

		GameUserInfo userData;
		if (!GetUserData(i, userData))
		{
			//m_tZongResult[i].i64UserMoney = userData.money;
			m_tZongResult.i64UserMoney[i] = userData.money;
		}

	}
	if (m_tZongResult.i64WinMoney[iBigWinnerStation] > 0)
	{
		m_tZongResult.bWinner[iBigWinnerStation] = true;
	}

	// 计算抽水值
	__super::SetDeskPercentageScore(m_tZongResult.i64WinMoney);

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID <= 0)
		{
			continue;
		}
		SendGameData(i, &m_tZongResult, sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, S_C_UPDATE_CALCULATE_BOARD_SIG, 0);
	}
}

//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	//编写代码
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
	{
		int roomType = GetRoomType();
		if (roomType != ROOM_TYPE_PRIVATE)
		{
			///游戏结束后所有玩家取消托管
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				long userID = GetUserIDByDeskStation(i);
				if (userID > 0)
				{
					UserSetAuto(i, false);
				}
			}
		}

		//保存出牌列表
		for (int i = 0; i < PLAY_COUNT; i ++)
		{
			if (m_iUserCardCount[i] <= 0)
			{
				continue;
			}

			memcpy(&m_lastCardList[m_lastCardListIndex], m_iUserCard[i], m_iUserCardCount[i]);
			m_lastCardListIndex += m_iUserCardCount[i];
		}

		//////////////////////////////小结算////////////////////////////////////////////
		//游戏结束
		GameEndStruct GameEnd;
		::memcpy(GameEnd.iUserCard, m_iUserCard, sizeof(m_iUserCard));
		::memcpy(GameEnd.iUserCardCount, m_iUserCardCount, sizeof(m_iUserCardCount));
		GameEnd.iBaseFen = GetBasePoint();
		GameEnd.iBombCount = m_GameMutiple.sBombCount;
		int iUserAllDouble[PLAY_COUNT];//用户总倍数
		memset(iUserAllDouble, 0, sizeof(iUserAllDouble));
		if (GetNoOutCard())
		{
			m_GameMutiple.sSprintMutiple = 2;
			GameEnd.bChunTian = true;
		}
		else
		{
			GameEnd.bChunTian = false;
		}
		int iBaseDouble = m_GameMutiple.GetPublicMutiple();
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (i == m_iUpGradePeople)
			{
				continue;
			}
			iUserAllDouble[i] = iBaseDouble;
		}
		int  iTurePoint = iBaseDouble * GetBasePoint();
		//是谁胜
		if (m_iUserCardCount[m_iUpGradePeople] == 0)
		{
			iTurePoint = iTurePoint;
		}
		else
		{
			iTurePoint = -iTurePoint;
		}

		////计算加倍
		int  iNtMutiple = m_GameMutiple.sAddGameMutiple[m_iUpGradePeople] > 0 ? 2 : 1;
		int  iAllFarMutiple = 0; //农民的总倍数
		long long llMoney[PLAY_COUNT];			//玩家得分
		memset(llMoney, 0, sizeof(llMoney));
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			GameEnd.bAddDouble[i] = m_GameMutiple.sAddGameMutiple[i];
			int iMyMutiple = m_GameMutiple.sAddGameMutiple[i] > 0 ? 2 : 1;
			if (i == m_iUpGradePeople)//庄家
			{
				continue;
			}
			else
			{
				llMoney[i] = -iTurePoint*iNtMutiple*iMyMutiple;//加倍
				iUserAllDouble[i] *= iNtMutiple*iMyMutiple;
				iAllFarMutiple += iUserAllDouble[i];
			}
			llMoney[m_iUpGradePeople] -= llMoney[i];
		}
		iUserAllDouble[m_iUpGradePeople] = iAllFarMutiple;
		memcpy(GameEnd.iAllDouble, iUserAllDouble, sizeof(GameEnd.iAllDouble));

		///一下计算玩家的得分情况，如果是金币场的话需要判断是否超过玩家身上的钱，如果是积分场则不需判断
		if (roomType != ROOM_TYPE_PRIVATE)
		{
			long long _Int64Money = 0;

			//控制赢
			if (llMoney[m_iUpGradePeople] > 0) //庄家赢
			{
				long long llResNums = 0;
				if (GetRoomResNums(m_iUpGradePeople, llResNums) && llMoney[m_iUpGradePeople] > llResNums)
				{
					for (int i = 0; i < PLAY_COUNT; i++)
					{
						if (i == m_iUpGradePeople)
							continue;
						llMoney[i] = llMoney[i] * llResNums / llMoney[m_iUpGradePeople];
					}

					llMoney[m_iUpGradePeople] = llResNums;
				}
			}
			else//庄家输
			{
				_Int64Money = 0;
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (i == m_iUpGradePeople)
					{
						continue;
					}
					long long llResNums = 0;
					GetRoomResNums(i, llResNums);
					if (llMoney[i] > llResNums)
					{
						llMoney[i] = llResNums;
					}
					_Int64Money += -llMoney[i];
				}
				llMoney[m_iUpGradePeople] = _Int64Money;
			}

			//控制输
			if (llMoney[m_iUpGradePeople] > 0) //庄家赢
			{
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (i == m_iUpGradePeople)
					{
						continue;
					}
					long long llResNums = 0;
					GetRoomResNums(i, llResNums);
					if (llMoney[i] + llResNums < 0)
					{
						llMoney[i] = -llResNums;
						m_bPoChan[i] = true;
					}
					_Int64Money += _abs64(llMoney[i]);
				}
				llMoney[m_iUpGradePeople] = _Int64Money;
			}
			else//庄家输
			{
				long long llResNums = 0;
				GetRoomResNums(m_iUpGradePeople, llResNums);
				if (llMoney[m_iUpGradePeople] + llResNums < 0)
				{
					for (int i = 0; i < PLAY_COUNT; i++)
					{
						if (i == m_iUpGradePeople)
							continue;
						llMoney[i] = llMoney[i] * llResNums / _abs64(llMoney[m_iUpGradePeople]);
					}
					llMoney[m_iUpGradePeople] = -_abs64(llResNums);
					m_bPoChan[m_iUpGradePeople] = true;
				}
			}
		}

		//计算抽水
		long long UserPumpGold[PLAY_COUNT] = { 0 };
		memset(UserPumpGold, 0, sizeof(UserPumpGold));
		if (roomType == ROOM_TYPE_GOLD)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (llMoney[i] > 0)
				{
					UserPumpGold[i] = llMoney[i] * 0 / 100;
					llMoney[i] -= UserPumpGold[i];
				}
			}
		}

		if (roomType == ROOM_TYPE_PRIVATE)
		{
			//房卡场结算
			ChangeUserPointPrivate(llMoney, NULL, "", true);
		}
		else if (roomType == ROOM_TYPE_FRIEND)
		{
			// 积分房结算
			//ChangeUserPointPrivate(llMoney, temp_cut,"",true);
			//ChangeUserPoint(llMoney, temp_cut);
			ChangeUserPointGoldRoom(llMoney, "", true);
		}
		else if (roomType == ROOM_TYPE_GOLD)
		{
			//金币场结算
			ChangeUserPoint(llMoney, NULL, UserPumpGold);
		}
		else if (roomType == ROOM_TYPE_FG_VIP) {
			//俱乐部房间
			ChangeUserFireCoin(llMoney, "", true);
		}


		for (int i = 0; i < PLAY_COUNT; i++)
		{
			GameEnd.iUserScore[i] = (int)llMoney[i];
			m_tZongResult.i64WinMoney[i] += llMoney[i];
		}
		if (m_iRunTime > 0 && GetCurTime() > m_iGameEndTime) //时间模式
		{
			m_iVipGameCount = m_iRunGameCount;
			GameEnd.iVipGameCount = m_iVipGameCount;
		}

		for (int i = 0; i < PLAY_COUNT; i++)
		{
			GameEnd.iZongFen = (int)llMoney[i];
			SendGameData(i, &GameEnd, sizeof(GameEnd), MSG_MAIN_LOADER_GAME, ASS_CONTINUE_END, 0);
			//SendWatchData(i,&GameEnd,sizeof(GameEnd),MSG_MAIN_LOADER_GAME,ASS_CONTINUE_END,0);
		}

		for (int i = 0; i < PLAY_COUNT; i++)
		{
			//最多赢钱数目
			if (m_tZongResult.iMaxWinMoney[i] <= m_tZongResult.i64WinMoney[i])
			{
				m_tZongResult.iMaxWinMoney[i] = (int)m_tZongResult.i64WinMoney[i];
			}
			//胜利局数
			if (llMoney[i] > 0)
			{
				//保存输赢记录
				m_vecWinInfo[i].push_back(true);
				m_tZongResult.iWinCount[i]++;
			}
			else
			{
				//保存输赢记录
				m_vecWinInfo[i].push_back(false);
			}
		}
		//设置数据 
		SetGameStation(GS_WAIT_ARGEE);
		ReSetGameState(bCloseFlag);
		__super::GameFinish(bDeskStation, GF_NORMAL);
		return true;
	}
	case GF_NO_CALL_SCORE:
	{
		if (IsGoldRoom())
		{
			///游戏结束后所有玩家取消托管
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				long userID = GetUserIDByDeskStation(i);
				if (userID > 0)
				{
					UserSetAuto(i, false);
				}
			}
		}
		//设置数据 
		SetGameStation(GS_WAIT_ARGEE);

		m_lastCardListIndex = 0;
		memset(m_lastCardList, 0, sizeof(m_lastCardList));

		BOOL bhavecut = false;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			long userID = GetUserIDByDeskStation(i);
			if (userID <= 0)
			{
				continue;
			}
			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			if (!pUser)
			{
				continue;
			}
			if (!pUser->IsOnline)
			{
				bhavecut = true;
			}
		}
		if (bhavecut)
		{
			GameFinish(255, GF_SALE);
			return true;
		}
		////游戏结束
		//GameEndStruct GameEnd;
		//::memset(&GameEnd,0,sizeof(GameEnd));

		//::memcpy(GameEnd.iUserCard,m_iUserCard,sizeof(m_iUserCard));
		//::memcpy(GameEnd.iUserCardCount,m_iUserCardCount,sizeof(m_iUserCardCount));

		////发送数据
		//for (int i = 0; i < PLAY_COUNT; i ++) 
		//	SendGameData(i,&GameEnd,sizeof(GameEnd),MSG_MAIN_LOADER_GAME,ASS_NO_CALL_SCORE_END,0);
		////SendWatchData(m_byMaxPeople,&GameEnd,sizeof(GameEnd),MSG_MAIN_LOADER_GAME,ASS_NO_CALL_SCORE_END,0);

		ReSetGameState(GF_NORMAL);
		if (m_iRunTime > 0 && GetCurTime() > m_iGameEndTime) //时间模式
		{
			m_iVipGameCount = m_iRunGameCount;
		}
		if (!IsGoldRoom() && m_iRunGameCount >= m_iVipGameCount)
		{
			m_byGameEndType = 1;
			__super::GameFinish(bDeskStation, bCloseFlag);
		}
		else
		{
			m_bRetryStart = true;
			GameBegin(ALL_ARGEE);
		}
		return true;
	}

	case GFF_SAFE_FINISH:
	case GF_SALE:			//游戏安全结束
	{
		//设置数据
		SetGameStation(GS_WAIT_ARGEE);

		m_lastCardListIndex = 0;
		memset(m_lastCardList, 0, sizeof(m_lastCardList));

		/*GameCutStruct CutEnd;
		::memset(&CutEnd,0,sizeof(CutEnd));

		CutEnd.bDeskStation = -1 ;

		for (int i = 0; i < PLAY_COUNT; i ++)
		SendGameData(i,&CutEnd,sizeof(CutEnd),MSG_MAIN_LOADER_GAME,ASS_SAFE_END,0);
		//SendWatchData(m_byMaxPeople,&CutEnd,sizeof(CutEnd),MSG_MAIN_LOADER_GAME,ASS_SAFE_END,0);*/

		bCloseFlag = GF_SALE;

		ReSetGameState(bCloseFlag);
		if (m_iRunTime > 0 && GetCurTime() > m_iGameEndTime) //时间模式
		{
			m_byGameEndType = 1;
			m_iVipGameCount = m_iRunGameCount;
		}
		__super::GameFinish(bDeskStation, bCloseFlag);
		return true;
	}
	case GFF_FORCE_FINISH:		//用户断线离开
	{
		return TRUE;
	}
	case GF_AHEAD_END://提前结束
	{
		//设置数据 
		SetGameStation(GS_WAIT_ARGEE);
		GameCutStruct CutEnd;
		::memset(&CutEnd, 0, sizeof(CutEnd));
		for (int i = 0; i < PLAY_COUNT; i++)
			SendGameData(i, &CutEnd, sizeof(CutEnd), MSG_MAIN_LOADER_GAME, ASS_AHEAD_END, 0);
		//SendWatchData(m_byMaxPeople,&CutEnd,sizeof(CutEnd),MSG_MAIN_LOADER_GAME,ASS_AHEAD_END,0);

		bCloseFlag = GF_AHEAD_END;

		ReSetGameState(bCloseFlag);
		__super::GameFinish(0, bCloseFlag);
		return true;
	}
	}
	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation, bCloseFlag);

	return true;
}

//逃跑扣分
int CServerGameDesk::GetRunPublish()
{
	return m_pDataManage->m_InitData.uRunPublish;
}

//逃跑扣分
int CServerGameDesk::GetRunPublish(BYTE bDeskStation)
{
	BYTE iResultCardList[45];
	memset(iResultCardList, 0, sizeof(iResultCardList));

	int  iResultCardcount = 0;
	int  ibombCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			ibombCount += m_Logic.GetBombCount(m_iUserCard[i], m_iUserCardCount[i], 4);
		}
		if (m_Logic.TackOutKingBomb(m_iUserCard[i], m_iUserCardCount[i], iResultCardList, iResultCardcount))
		{
			ibombCount++;
		}
	}

	m_GameMutiple.sBombCount += ibombCount;

	if (m_GameMutiple.sBaseMutiple <= 0)
	{
		return m_pDataManage->m_InitData.uRunPublish;
	}

	int  iMutiple = m_GameMutiple.GetPublicMutiple() / m_GameMutiple.sBaseMutiple;


	if (bDeskStation == m_iUpGradePeople)
	{
		if (iMutiple < 10)
		{
			iMutiple = 20;
		}
		else
		{
			iMutiple *= 4;
		}
	}
	else
	{
		if (iMutiple < 10)
		{
			iMutiple = 10;
		}
		else
		{
			iMutiple *= 2;
		}
	}

	return  (iMutiple *m_GameMutiple.sBaseMutiple);

}

//游戏基础倍数
int CServerGameDesk::GetRoomMul()
{
	return m_pDataManage->m_InitData.uBasePoint; // 倍数
}

//桌子倍数
int CServerGameDesk::GetDeskBasePoint()
{
	return m_iBaseMult;
}

//设置下一个庄家,即下一个发牌位置,叫分位置
BOOL CServerGameDesk::SetNextBanker(BYTE bGameFinishState)
{
	switch (bGameFinishState)
	{
	case GF_NORMAL://正常结束本局胜者下一局优先叫分
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_iUserCardCount[i] == 0)
			{
				m_iDealPeople = m_iUpGradePeople = i;
				break;
			}
		}
		break;
	case GF_NO_CALL_SCORE://无人叫分
#ifdef TWO
		m_iDealPeople = m_iUpGradePeople = (m_iUpGradePeople + 1) % PLAY_COUNT;
#else
		m_iDealPeople = m_iUpGradePeople = (m_iUpGradePeople + 2) % PLAY_COUNT;
#endif
		break;
	default:
		m_iDealPeople = m_iUpGradePeople = -1;
	}
	return TRUE;
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if ((GetGameStation() >= GS_SEND_CARD && GetGameStation() < GS_WAIT_NEXT))
		return true;
	return false;
}

//用户请求离开
BOOL CServerGameDesk::UserHaveThing(BYTE bDeskStation, char * szMessage)
{
	// 已经点过退出按钮(一局只能单击一次)
	if (!m_bCanleave[bDeskStation])
		return TRUE;

	if (GetGameStation() < GS_SEND_CARD)
		return TRUE;

	if (m_iHaveThingPeople != bDeskStation)
	{
		m_iLeaveArgee = 0;
	}

	m_icountleave = 0;
	m_iHaveThingPeople = bDeskStation;
	m_iLeaveArgee |= 1 << bDeskStation;
	m_bCanleave[bDeskStation] = false;

	if (m_iLeaveArgee != 7)
	{
		HaveThingStruct HaveThing;
		HaveThing.pos = bDeskStation;
		::CopyMemory(HaveThing.szMessage, szMessage, 60 * sizeof(char));

		for (int i = 0; i < PLAY_COUNT; i++)
			if (i != bDeskStation)
				SendGameData(i, &HaveThing, sizeof(HaveThing), MSG_MAIN_LOADER_GAME, ASS_HAVE_THING, 0);
	}
	else
		GameFinish(255, GF_SALE);
	return TRUE;
}

//同意用户离开
BOOL CServerGameDesk::ArgeeUserLeft(BYTE bDeskStation, BOOL bArgee)
{
	m_icountleave++;
	if (bArgee) m_iLeaveArgee |= 1 << bDeskStation;
	else
		m_iLeaveArgee = 0;
	if (m_iLeaveArgee != 7)				//3个人游戏
	{
		LeaveResultStruct Leave;
		Leave.bDeskStation = bDeskStation;
		Leave.bArgeeLeave = bArgee;
		for (int i = 0; i < PLAY_COUNT; i++)
			if (i != bDeskStation)
				SendGameData(i, &Leave, sizeof(Leave), MSG_MAIN_LOADER_GAME, ASS_LEFT_RESULT, 0);
	}
	else
	{
		GameFinish(255, GF_SALE);
		LeaveResultStruct Leave1;
		Leave1.bDeskStation = m_iHaveThingPeople;
		Leave1.bArgeeLeave = true;
		SendGameData(m_iHaveThingPeople, &Leave1, sizeof(Leave1), MSG_MAIN_LOADER_GAME, ASS_LEFT_RESULT, 0);
	}
	if (m_icountleave >= 2)
	{
		m_iLeaveArgee = 0;
	}

	return TRUE;
}

//托管设置
bool CServerGameDesk::UserSetAuto(BYTE bDeskStation, bool bAutoCard)
{
	if (!IsPlayGame(0)) //非游戏状态不能托管
	{
		return true;
	}
	bool bTempAuto = m_bAuto[bDeskStation];
	m_bAuto[bDeskStation] = bAutoCard;
	AutoStruct autoset;
	autoset.bAuto = bAutoCard;
	autoset.bDeskStation = bDeskStation;

	for (int i = 0; i < PLAY_COUNT; i++)
		SendGameData(i, &autoset, sizeof(autoset), MSG_MAIN_LOADER_GAME, ASS_AUTO, 0);
	//SendWatchData(m_byMaxPeople,&autoset,sizeof(autoset),MSG_MAIN_LOADER_GAME,ASS_AUTO,0);
	if (GetGameStation() == GS_PLAY_GAME) //非游戏状态不能托管
	{
		if (!bTempAuto && bAutoCard && bDeskStation == m_iOutCardPeople)
		{
			SetTimer(TIME_OUT_CARD, 1000);
		}
		if (bTempAuto && !bAutoCard && bDeskStation == m_iOutCardPeople)
		{
			KillTimer(TIME_OUT_CARD);
		}
	}
	else if (GetGameStation() == GS_SEND_CARD || GetGameStation() == GS_WAIT_BACK)
	{
		if (!bTempAuto && bAutoCard)
		{
			if (m_iGameFlag == GS_FLAG_CALL_SCORE && m_bCurrentCallScore == bDeskStation)
			{
				UserCallScore(bDeskStation, 0);
			}
			if (m_iGameFlag == GS_FLAG_ROB_NT && m_iCurrentRobPeople == bDeskStation)
			{
				UserRobNT(bDeskStation, 0);
			}
			if (m_iGameFlag == GS_FLAG_ADD_DOUBLE)
			{
				UserAddDouble(bDeskStation, 0);
			}
		}
	}
	return true;
}

//自动出牌
BOOL CServerGameDesk::UserAutoOutCard(BYTE bDeskStation)
{
	if (bDeskStation >= PLAY_COUNT)
	{
		return FALSE;
	}

	BYTE bCardList[45];
	memset(bCardList, 0, sizeof(BYTE) * 45);
	int iCardCount = 0;
	//if(bDeskStation == m_iFirstOutPeople)   // duanxiaohui alter 20100319
	{
		m_Logic.AutoOutCard(m_iUserCard[bDeskStation], m_iUserCardCount[bDeskStation],
			m_iDeskCard[m_iNowBigPeople], m_iDeskCardCount[m_iNowBigPeople],
			bCardList, iCardCount, bDeskStation == m_iNowBigPeople);
	}
	//else
	//{
	//	iCardCount = 0;
	//}
	return UserOutCard(bDeskStation, bCardList, iCardCount);
}

//用户断线离开
bool CServerGameDesk::UserNetCut(GameUserInfo *pUser)
{
	__super::UserNetCut(pUser);

	if (IsGoldRoom() && IsPlayGame(0) && IsAllUserOffline())
	{
		GameFinish(pUser->deskStation, GF_SALE);
	}
	
	return true;
}

//用户离开游戏桌
bool CServerGameDesk::UserLeftDesk(GameUserInfo* pUser)
{
	m_bUserReady[pUser->deskStation] = false;
	m_GameMutiple.sMingPaiMutiple[pUser->deskStation] = 0;
	return __super::UserLeftDesk(pUser);
}

//是否为未出过牌
BYTE CServerGameDesk::GetNoOutCard()
{
	BOOL bOutCard = FALSE;
	//两闲家没有出过牌
	if (m_iUserCardCount[(m_iUpGradePeople + 1) % PLAY_COUNT] == m_iUserCardCount[(m_iUpGradePeople + 2) % PLAY_COUNT]
		&& m_iUserCardCount[(m_iUpGradePeople + 2) % PLAY_COUNT] == 17)
	{
		bOutCard = true;
	}
#ifdef TWO
	if (m_iUserCardCount[(m_iUpGradePeople + 1) % PLAY_COUNT] == 17)
	{
		bOutCard = true;
	}
#endif
	//庄家仅出了第一首牌
	if (m_iUserCardCount[m_iUpGradePeople] == 20 - m_iNtFirstCount)
	{
		bOutCard = true;
	}
	return bOutCard;
}

bool	CServerGameDesk::IsSuperUser(BYTE byDeskStation)
{
#ifdef SUPER_CLIENT
	GameUserInfo user;
	if (GetUserData(byDeskStation, user) && (user.userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER)
	{
		return true;
	}
#endif
	return false;
}

//验证是否超端
void	CServerGameDesk::SpuerExamine(BYTE byDeskStation)
{
	//是超端用户 那么就发送超端消息过去
	if (IsSuperUser(byDeskStation))
	{
		SuperUserMsg TSuperUser;
		TSuperUser.bIsSuper = true;
		TSuperUser.byDeskStation = byDeskStation;
		SendGameData(byDeskStation, &TSuperUser, sizeof(TSuperUser), MSG_MAIN_LOADER_GAME, ASS_SUPER_USER, 0);
	}
}

//房间规则解析
void CServerGameDesk::GetSpecialRule()
{
	Json::Reader reader;
	Json::Value root;
	Json::Value arrayWanFa; //玩法

	if (reader.parse(m_szGameRules, root))
	{
		//m_iFengDing //封顶倍数（炸弹）//
		//时间模式
		int iTemp = 0;
		iTemp = root["Fd"].asInt();
		if (iTemp == 1)
		{
			m_iFengDing = 4;
		}
		else if (iTemp == 2)
		{
			m_iFengDing = 5;
		}
		else if (iTemp == 3)
		{
			m_iFengDing = 6;
		}
		iTemp = root["Time"].asInt();			//运行时间
		if (iTemp == 1)
		{
			m_iRunTime = 5 * 60;
		}
		else if (iTemp == 2)
		{
			m_iRunTime = 10 * 60;
		}
		iTemp = root["gameIdx"].asInt();			//玩法
		if (iTemp == 2)
		{
			m_bNoXiPai = true;
		}
	}

	m_iAddDoubleLimit = m_MinPoint * 2;
}

//获取当前时间，返回秒
long long CServerGameDesk::GetCurTime()
{
	return time(NULL);
}

//发送出牌出错消息
void CServerGameDesk::SendOutCardError(BYTE byDeskStation, int iErrorCode)
{
	if (byDeskStation < 0 || byDeskStation >= PLAY_COUNT)
	{
		return;
	}
	CMD_S_OutCard_Result rsOutCard;
	rsOutCard.bResult = false;
	rsOutCard.byErrorCode = iErrorCode;
	rsOutCard.byHandCardCount = m_iUserCardCount[byDeskStation];
	memcpy(rsOutCard.byHandCard, m_iUserCard[byDeskStation], sizeof(rsOutCard.byHandCard));
	SendGameData(byDeskStation, &rsOutCard, sizeof(rsOutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD_RESULT, 0);
	//SendWatchData(byDeskStation,&rsOutCard,sizeof(rsOutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD_RESULT, 0);
}