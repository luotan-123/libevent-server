#include "GameDesk.h"

CGameDesk::CGameDesk(BYTE byBeginMode) : m_byBeginMode(byBeginMode)
{
	m_bPlayGame = false;
	m_byMaxPeople = 0;
	m_byGameStation = 0;
	m_pDataManage = NULL;
	m_deskIdx = INVALID_DESKIDX;
	m_iVipGameCount = 0;		//购买桌子局数
	m_iBuyGameCount = 0;
	m_iRunGameCount = 0;		//游戏运行的局数
	m_masterID = 0;
	m_isMasterNotPlay = false;
	m_needLoadConfig = false;

	memset(m_szDeskPassWord, 0, sizeof(m_szDeskPassWord));
	memset(m_szGameRules, 0, sizeof(m_szGameRules));
	memset(m_uScore, 0, sizeof(m_uScore));
	memset(m_gameWinCount, 0, sizeof(m_gameWinCount));

	m_isDismissStatus = false;
	m_reqDismissDeskStation = INVALID_DESKSTATION;
	m_reqDismissTime = 0;

	m_isBegin = false;
	m_enable = false;
	m_finishedGameCount = 0;
	m_iConfigCount = 0;

	m_beginTime = 0;
	m_finishedTime = 0;
	m_friendsGroupMsg.Init();
	m_autoBeginMode = 0;
	m_bGameStopJoin = 0;
}

CGameDesk::~CGameDesk()
{

}

bool CGameDesk::Init(int deskIdx, BYTE byMaxPeople, CGameMainManage * pDataManage)
{
	if (!pDataManage)
	{
		return false;
	}

	m_byMaxPeople = byMaxPeople;
	m_pDataManage = pDataManage;
	m_deskIdx = deskIdx;

	InitDeskGameStation();
	ReSetGameState(0);

	// 初始化桌子玩家数组大小
	m_deskUserInfoVec.resize(byMaxPeople);
	m_iConfigCount = byMaxPeople;

	return true;
}

bool CGameDesk::SetTimer(UINT uTimerID, int uElapse, BYTE timerType/* = SERVERTIMER_TYPE_SINGLE*/)
{
	if (uTimerID >= TIME_SPACE)
	{
		ERROR_LOG("SetTimer error uTimerID >= %d", TIME_SPACE);
		return false;
	}

	if (!m_pDataManage)
	{
		return false;
	}

	unsigned int realTimerID = m_deskIdx * TIME_SPACE + uTimerID + TIME_START_ID;
	return m_pDataManage->SetTimer(realTimerID, uElapse, timerType);
}

bool CGameDesk::KillTimer(UINT uTimerID)
{
	if (uTimerID >= TIME_SPACE)
	{
		return false;
	}

	if (!m_pDataManage)
	{
		return false;
	}

	return m_pDataManage->KillTimer(m_deskIdx * TIME_SPACE + TIME_START_ID + uTimerID);
}

bool CGameDesk::UserNetCut(GameUserInfo *pUser)
{
	m_deskUserInfoVec[pUser->deskStation].bNetCut = true;

	if (GetRoomSort() == ROOM_SORT_NORMAL || GetRoomSort() == ROOM_SORT_SCENE)
	{
		BroadcastUserOfflineData(pUser->deskStation);
	}
	else if (GetRoomSort() == ROOM_SORT_HUNDRED)
	{
		HundredGameUserNetCutLogic(pUser);
	}

	return true;
}

bool CGameDesk::SendGameData(BYTE deskStation, unsigned int mainID, unsigned int assistID, unsigned int handleCode)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	m_pDataManage->SendData(userID, NULL, 0, mainID, assistID, handleCode);

	return true;
}

bool CGameDesk::SendGameData(BYTE deskStation, void* pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID > 0)
	{
		m_pDataManage->SendData(userID, pData, size, mainID, assistID, handleCode);
	}

	return true;
}

bool CGameDesk::CanBeginGame()
{
	if (GetRoomSort() == ROOM_SORT_SCENE)
	{
		//场景类的游戏开始，交给游戏自己控制
		return false;
	}

	if (IsPlayGame(0))
	{
		return false;
	}

	int agreeCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (pUser)
		{
			if (pUser->playStatus == USER_STATUS_AGREE)
			{
				agreeCount++;
			}
		}
	}

	if (m_byBeginMode == ALL_ARGEE)
	{
		if (agreeCount <= 1)
		{
			return false;
		}

		int roomType = GetRoomType();
		if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
		{
			if (m_autoBeginMode > 0 && agreeCount < m_autoBeginMode)
			{
				return false;
			}
		}

		int currDeskUserCount = 0;
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			if (m_deskUserInfoVec[i].userID > 0)
			{
				currDeskUserCount++;
			}
		}

		if (agreeCount >= currDeskUserCount)
		{
			return true;
		}
	}
	else if (m_byBeginMode == FULL_BEGIN)
	{
		if (m_pDataManage->IsMultiPeopleGame())
		{
			if (agreeCount == m_iConfigCount)
			{
				return true;
			}
		}
		else
		{
			if (agreeCount == m_byMaxPeople)
			{
				return true;
			}
		}
	}

	return false;
}

bool CGameDesk::UserAgreeGame(BYTE deskStation)
{
	if (GetRoomSort() == ROOM_SORT_HUNDRED || GetRoomSort() == ROOM_SORT_SCENE)
	{
		// 百人类游戏和场景类游戏没有准备
		return false;
	}

	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		// 比赛场不需要准备
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("GetUserIDByDeskStation failed deskStation:%d", deskStation);
		return false;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser userID:%d", userID);
		return false;
	}

	// 游戏状态是否正确
	if (m_bPlayGame)
	{
		return false;
	}

	int roomType = GetRoomType();

	// 玩家状态是否正确
	if (pUser->playStatus != USER_STATUS_SITING && (roomType == ROOM_TYPE_GOLD || m_iRunGameCount > 0))
	{
		return false;
	}

	//（金币房和俱乐部VIP房）判断该玩家金币是否满足条件
	if (m_pDataManage->m_uNameID != 37550102 && (roomType == ROOM_TYPE_FRIEND && pUser->money < m_RemoveMinPoint || roomType == ROOM_TYPE_FG_VIP && pUser->fireCoin < m_RemoveMinPoint))
	{
		ERROR_LOG("user(%d) money(%lld)金币不够，无法准备", pUser->userID, pUser->money);
		return true;
	}

	pUser->playStatus = USER_STATUS_AGREE;

	// 通知这个玩家已准备
	BroadcastUserAgreeData(deskStation);

	if ((roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP) && m_pDataManage->IsCanWatch() && m_autoBeginMode == 0)
	{
		// 第一局
		if (m_iRunGameCount == 0 && CanBeginGame())
		{
			SetBeginUser();
			m_pDataManage->SendData(m_beginUserID, NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_CANBEGIN, 0);
		}

		if (m_iRunGameCount >= 1 && CanBeginGame())
		{
			// 第一局之后的游戏不受房主控制
			GameBegin(0);
		}
	}
	else
	{
		if (CanBeginGame())
		{
			GameBegin(0);
		}
	}

	return true;
}

bool CGameDesk::SendGameStation(BYTE deskStation, UINT socketID, bool bWatchUser, void * pData, UINT size)
{
	int roomType = GetRoomType();

	LoaderGameInfo gameInfo;

	gameInfo.bGameStation = m_byGameStation;
	memcpy(gameInfo.gameRules, m_szGameRules, sizeof(gameInfo.gameRules));

	if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		if (m_iRunGameCount == 0 && GetUserIDByDeskStation(deskStation) == m_beginUserID && CanBeginGame())
		{
			m_pDataManage->SendData(m_beginUserID, NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_CANBEGIN, 0);
		}
	}
	else if (roomType == ROOM_TYPE_MATCH) //比赛场特殊处理
	{
		int iSocketIndex = -1;
		int userID = GetUserIDByDeskStation(deskStation);
		if (userID <= 0)
		{
			iSocketIndex = (int)socketID;
		}
		else
		{
			GameUserInfo *pUser = m_pDataManage->GetUser(userID);
			if (pUser)
			{
				iSocketIndex = pUser->socketIdx;
				userID = pUser->userID;
			}
			else
			{
				ERROR_LOG("内存没有玩家数据：userID=%d", userID);
			}
		}

		m_pDataManage->m_pGServerConnect->SendData(iSocketIndex, &gameInfo, sizeof(gameInfo), MSG_MAIN_LOADER_FRAME, MSG_ASS_LOADER_GAME_INFO, 0, userID);
		m_pDataManage->m_pGServerConnect->SendData(iSocketIndex, pData, size, MSG_MAIN_LOADER_FRAME, MSG_ASS_LOADER_GAME_STATION, 0, userID);

		// 发送当前桌子状态
		SendMatchDeskStatus(userID);

		return true;
	}

	//发送游戏信息
	SendGameData(deskStation, &gameInfo, sizeof(gameInfo), MSG_MAIN_LOADER_FRAME, MSG_ASS_LOADER_GAME_INFO, 0);
	SendGameData(deskStation, pData, size, MSG_MAIN_LOADER_FRAME, MSG_ASS_LOADER_GAME_STATION, 0);

	return true;
}

bool CGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (GetRoomSort() == ROOM_SORT_HUNDRED)
	{
		return HundredGameBegin();
	}

	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		return MatchRoomGameBegin();
	}

	// 记录玩家状态，游戏开始最后验证数据
	int iDeskErrorCount = 0, iMemErrorCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			if (!pUser)
			{
				iMemErrorCount++;
				ERROR_LOG("###  游戏开始，内存中没有玩家数据  userID=%d ###", userID);
				m_deskUserInfoVec[i].clear();
				continue;
			}

			if (pUser->deskStation != i)
			{
				//数据异常
				ERROR_LOG("user deskStation is not match userID:%d deskStation=%d i=%d", pUser->userID, pUser->deskStation, i);
				return false;
			}

			pUser->playStatus = USER_STATUS_PLAYING;
		}
		else
		{
			iDeskErrorCount++;
		}
	}
	if (m_byBeginMode == FULL_BEGIN && (iDeskErrorCount + iMemErrorCount) > 0)
	{
		ERROR_LOG("###  游戏无法开始，iDeskErrorCount=%d,iMemErrorCount=%d  ###", iDeskErrorCount, iMemErrorCount);
		return false;
	}

	//记录游戏开始需要的数据
	m_bPlayGame = true;
	m_beginTime = time(NULL);
	int roomType = GetRoomType();

	// 金币场开局扣除玩家金币
	if (roomType == ROOM_TYPE_GOLD)
	{
		ProcessDeduceMoneyWhenGameBegin();
	}
	else if (roomType == ROOM_TYPE_FRIEND)
	{
		ProcessDeduceMoneyWhenGameBegin();
		KillTimer(IDT_FRIEND_ROOM_GAMEBEGIN);
	}
	else if (roomType == ROOM_TYPE_FG_VIP)
	{
		KillTimer(IDT_FRIEND_ROOM_GAMEBEGIN);
	}

	// 局数
	m_iRunGameCount++;

	if (roomType != ROOM_TYPE_GOLD)
	{
		BroadcastDeskBaseInfo();

		int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);
		CRedisLoader* pRedis = m_pDataManage->GetRedis();
		if (pRedis)
		{
			pRedis->SetPrivateDeskGameCount(deskMixID, m_iRunGameCount);
		}
	}

	// 告诉桌子的玩家游戏开始了
	BroadcastDeskData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GAMEBEGIN);

	// 房卡场第一局
	if ((roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP) && m_iRunGameCount == 1)
	{
		if (m_isBegin == false)
		{
			m_isBegin = true;

			// 通知大厅
			if (m_friendsGroupMsg.friendsGroupDeskNumber > 0)
			{
				NotifyLogonDeskStatusChange();
			}
		}
	}

	return true;
}

bool CGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	if (GetRoomSort() == ROOM_SORT_HUNDRED)
	{
		return HundredGameFinish();
	}

	int roomType = GetRoomType();

	if (roomType == ROOM_TYPE_MATCH)
	{
		return MatchRoomGameFinish();
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	if (!m_bPlayGame)
	{
		// 没有开始的游戏怎么会结束
		ERROR_LOG("m_bPlayGame error m_bPlayGame:%d", m_bPlayGame);
		return false;
	}

	m_finishedGameCount++;
	m_bPlayGame = false;
	time_t currTime = time(NULL);
	LoaderNotifyGameFinish finishMsg;
	int rechargePlayerCount = 0;

	if ((roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP) && m_iRunGameCount < m_iVipGameCount)
	{
		OtherConfig otherConfig;
		if (!pRedis->GetOtherConfig(otherConfig))
		{
			otherConfig = ConfigManage()->GetOtherConfig();
		}

		// 不是最后一局
		bool bNeedKickOutUser = false;
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			int userID = m_deskUserInfoVec[i].userID;
			if (userID <= 0)
			{
				continue;
			}

			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			if (pUser && (roomType == ROOM_TYPE_FRIEND && pUser->money < m_RemoveMinPoint
				|| roomType == ROOM_TYPE_FG_VIP && pUser->fireCoin < m_RemoveMinPoint))
			{
				long long leftMoney = (long long)m_MinPoint - (roomType == ROOM_TYPE_FRIEND ? pUser->money : pUser->fireCoin);
				m_pDataManage->NotifyUserRechargeMoney(userID, leftMoney, FRIEND_ROOM_WAIT_BEGIN_TIME);
				finishMsg.rechargeUserID[rechargePlayerCount++] = userID;

				bNeedKickOutUser = true;
			}
		}

		if (bNeedKickOutUser)
		{
			SetTimer(IDT_FRIEND_ROOM_GAMEBEGIN, FRIEND_ROOM_WAIT_BEGIN_TIME * 1000);
		}
	}

	char userIDList[2048] = "";		// 账单使用

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser:%d failed", userID);
			continue;
		}

		sprintf(userIDList + strlen(userIDList), "%d,", userID);
		pUser->playStatus = USER_STATUS_SITING;
		m_deskUserInfoVec[i].lastWaitAgreeTime = currTime;
	}

	PrivateDeskInfo privateDeskInfo;
	if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		int deskMixID = m_pDataManage->m_InitData.uRoomID * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;
		pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo);

		BroadcastDeskData(&finishMsg, sizeof(finishMsg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GAMEFINISH);

		if (m_iRunGameCount >= m_iVipGameCount)
		{
			OnDeskDissmissFinishSendData();
			OnDeskSuccessfulDissmiss(false);
		}
	}
	else if (roomType == ROOM_TYPE_GOLD)
	{
		BroadcastDeskData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GAMEFINISH);

		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];

			int userID = deskUserInfo.userID;
			if (userID <= 0)
			{
				continue;
			}

			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			if (!pUser)
			{
				ERROR_LOG("GetUser:%d failed", userID);
				continue;
			}

			bool bKickOut = false;
			bool bDelUser = false;
			int kickReason = REASON_KICKOUT_STAND;

			if (pUser->deskStation != i)
			{
				// 走到这里数据已经异常了
				ERROR_LOG("user deskStation is not match userID:%d deskStation=%d i=%d", pUser->userID, pUser->deskStation, i);
				bKickOut = true;
				bDelUser = true;
			}
			else if (!pUser->IsOnline)
			{
				bKickOut = true;
				bDelUser = true;
			}
			else
			{
				kickReason = CheckUserMoney(userID);
				if (kickReason != REASON_KICKOUT_DEFAULT)
				{
					bKickOut = true;
				}
			}

			if (bKickOut)
			{
				// 玩家被踢
				UserBeKicked((BYTE)i);

				// 通知所有玩家
				BroadcastUserLeftData((BYTE)i, kickReason);

				// 清理桌子玩家数据
				deskUserInfo.clear();

				// 设置玩家为站起状态
				pUser->playStatus = USER_STATUS_STAND;
				pUser->deskIdx = INVALID_DESKIDX;
				pUser->deskStation = INVALID_DESKSTATION;

				if (bDelUser)
				{
					// 内存和缓存中移除玩家数据
					m_pDataManage->DelUser(userID);
				}
			}
		}
	}

	if (roomType == ROOM_TYPE_GOLD || roomType == ROOM_TYPE_FRIEND)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_GAME_RECORD_INFO, tableName, sizeof(tableName));

		// 生成游戏记录
		BillManage()->WriteBill(&m_pDataManage->m_SQLDataManage,
			"INSERT INTO %s (passwd,deskIdx,roomID,createTime,beginTime,finishedTime,userIDList) VALUES('%s',%d,%d,%d,%d,%d,'%s')",
			tableName, privateDeskInfo.passwd, m_deskIdx, m_pDataManage->GetRoomID(),
			(int)privateDeskInfo.createTime, (int)m_beginTime, (int)m_finishedTime, userIDList);
	}

	// 加载配置
	if (m_needLoadConfig)
	{
		m_needLoadConfig = false;
		LoadDynamicConfig();
	}

	return true;
}

bool CGameDesk::OnTimer(UINT timerID)
{
	int roomType = GetRoomType();
	if (timerID == IDT_AGREE_DISMISS_DESK && (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP))
	{
		OnDeskDissmissFinishSendData();
		OnDeskSuccessfulDissmiss(true);
	}

	if (timerID == IDT_FRIEND_ROOM_GAMEBEGIN && (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP))
	{
		OnTimerFriendRoomGameBegin();
	}

	return false;
}

bool CGameDesk::HandleNotifyMessage(BYTE deskStation, unsigned int assistID, void * pData, int uSize, bool bWatchUser)
{
	switch (assistID)
	{
	case MSG_ASS_LOADER_GAME_AGREE:		//用户同意
	{
		return UserAgreeGame(deskStation);
	}
	case MSG_ASS_LOADER_GAME_AUTO:
	{
		return OnHandleUserRequestAuto(deskStation);
	}
	case MSG_ASS_LOADER_GAME_CANCEL_AUTO:
	{
		return OnHandleUserRequestCancelAuto(deskStation);
	}
	case MSG_ASS_LOADER_GAME_MAGICEXPRESS:
	{
		return OnHandleUserRequestMagicExpress(deskStation, pData, uSize);
	}
	default:
		break;
	}

	return false;
}

bool CGameDesk::HandleDissmissMessage(BYTE deskStation, unsigned int assistID, void * pData, int size)
{
	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return true;
	}

	switch (assistID)
	{
	case MSG_ASS_LOADER_REQ_DESKDISSMISS:
	{
		return OnHandleUserRequestDissmiss(deskStation);
	}
	case MSG_ASS_LOADER_ANSWER_DESKDISMISS:
	{
		return OnHandleUserAnswerDissmiss(deskStation, pData, size);
	}
	default:
	{
		break;
	}
	}
	return false;
}

void CGameDesk::OnDeskSuccessfulDissmiss(bool isDismissMidway /*= true*/)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return;
	}

	CRedisPHP* pRedisPHP = m_pDataManage->GetRedisPHP();
	if (!pRedisPHP)
	{
		return;
	}

	int roomID = m_pDataManage->GetRoomID();
	int deskMixID = roomID * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;

	PrivateDeskInfo privateDeskInfo;
	if (!pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		ERROR_LOG("GetPrivateDeskRecordInfo failed deskMixID(%d)", deskMixID);
		return;
	}

	// 退还房卡
	BuyGameDeskInfo buyDeskInfo = ProcessCostJewels();

	// 清除所有定时器
	KillTimer(IDT_AGREE_DISMISS_DESK);
	KillTimer(IDT_FRIEND_ROOM_GAMEBEGIN);

	char userInfoList[1024] = "";
	char pumpInfoList[512] = "";
	char winCountInfoList[512] = "";
	long long llAllPumpScore = 0;
	long long llReturnMoney = 0;
	int roomType = GetRoomType();
	double rate = GetDeskPercentage();
	int currTime = (int)time(NULL);
	int deskRemainPeopleCount = 0;

	// 通知前端成功解散TODO
	LoaderNotifyDismissSuccess msg;
	msg.isDismissMidway = isDismissMidway;
	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_DISMISS_OK);

	if (m_finishedGameCount > 0)
	{
		//先找到最大的大赢家
		long long llMaxWinMoney = 0;
		if (m_roomTipType == ROOM_TIP_TYPE_MAX_WIN)
		{
			for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
			{
				if (i >= MAX_PLAYER_GRADE)
				{
					break;
				}

				int userID = m_deskUserInfoVec[i].userID;
				if (userID <= 0)
				{
					if (m_deskUserInfoVec[i].leftRoomUser <= 0)
					{
						continue;
					}
					else
					{
						userID = m_deskUserInfoVec[i].leftRoomUser;
					}
				}
				else
				{
					deskRemainPeopleCount++;
				}

				if (m_uScore[i] > llMaxWinMoney)
				{
					llMaxWinMoney = m_uScore[i];
				}
			}
		}

		// 保存大结算战绩TODO
		PrivateDeskGradeSimpleInfo simpleGradeInfo;

		simpleGradeInfo.roomID = roomID;
		simpleGradeInfo.time = currTime;
		memcpy(simpleGradeInfo.gameRules, m_szGameRules, sizeof(simpleGradeInfo.gameRules));
		simpleGradeInfo.masterID = m_masterID;
		simpleGradeInfo.passwd = atoi(m_szDeskPassWord);
		simpleGradeInfo.gameCount = m_finishedGameCount;
		simpleGradeInfo.maxGameCount = m_iBuyGameCount;

		std::vector<int> userIDVec;
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			if (i >= MAX_PLAYER_GRADE)
			{
				break;
			}

			int userID = m_deskUserInfoVec[i].userID;
			if (userID <= 0)
			{
				if (m_deskUserInfoVec[i].leftRoomUser <= 0)
				{
					continue;
				}
				else
				{
					userID = m_deskUserInfoVec[i].leftRoomUser;
				}
			}

			userIDVec.push_back(userID);

			// 计算抽水
			long long taxValue = 0;
			long long AllScore[MAX_PLAYER_GRADE];
			memcpy(AllScore, m_uScore, sizeof(m_uScore));
			if (rate > 0 && m_uScore[i] > 0 && (m_roomTipType == ROOM_TIP_TYPE_ALL_WIN || m_roomTipType == ROOM_TIP_TYPE_MAX_WIN && m_uScore[i] == llMaxWinMoney))
			{
				taxValue = (long long)floor(rate * m_uScore[i]);
				m_uScore[i] -= taxValue;

				if (roomType == ROOM_TYPE_FRIEND)
				{
					pRedis->SetUserMoneyEx(userID, -taxValue, true, RESOURCE_CHANGE_REASON_ROOM_PUMP_CONSUME, roomID, 0, 0, m_friendsGroupMsg.friendsGroupID);

					if (m_friendsGroupMsg.friendsGroupID > 0)
					{
						//将抽水返给管理员
						UserData MuserData;
						int ChouShui = 0;
						int ManagerId = 0;
						if (!pRedisPHP->GetfriendsGroupToUser(m_friendsGroupMsg.friendsGroupID, userID, ChouShui, ManagerId))
						{
							ERROR_LOG("获取俱乐部绑定的管理员ID失败：userId=%d,friendsGroupID=%d", userID, m_friendsGroupMsg.friendsGroupID);
						}
						if (ChouShui > 0 && ChouShui < 100 && ManagerId > 0 && taxValue > 0)
						{
							long long taxValueEx = 0;
							double drate = double(ChouShui) / 100.0;
							taxValueEx = (long long)floor(drate * taxValue);
							//将金币抽水返现给房主
							if (taxValueEx > 0 && pRedis->GetUserData(ManagerId, MuserData))
							{
								MuserData.money += taxValueEx;
								pRedis->SetUserMoneyEx(ManagerId, taxValueEx, true, RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP, roomID, 0, 0, m_friendsGroupMsg.friendsGroupID);

								// 通知金币变化
								m_pDataManage->SendResourcesChangeToLogonServer(ManagerId, RESOURCE_TYPE_MONEY, MuserData.money, RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP, taxValueEx);
							}
							llReturnMoney += taxValueEx;
						}
					}
				}
				else if (roomType == ROOM_TYPE_FG_VIP)
				{
					pRedisPHP->SetUserFriendsGroupMoney(m_friendsGroupMsg.friendsGroupID, userID, -taxValue, true, RESOURCE_CHANGE_REASON_ROOM_PUMP_CONSUME, roomID);

					if (m_friendsGroupMsg.friendsGroupID > 0)
					{
						//将抽水返给管理员
						int ChouShui = 0;
						int ManagerId = 0;
						long long FirMoney = 0;
						if (!pRedisPHP->GetfriendsGroupToUser(m_friendsGroupMsg.friendsGroupID, userID, ChouShui, ManagerId))
						{
							ERROR_LOG("获取俱乐部绑定的管理员ID失败：userId=%d,friendsGroupID=%d", userID, m_friendsGroupMsg.friendsGroupID);
						}
						if (ChouShui > 0 && ChouShui < 100 && ManagerId > 0 && taxValue > 0)
						{
							long long taxValueEx = 0;
							double drate = double(ChouShui) / 100.0;
							taxValueEx = (long long)floor(drate * taxValue);
							//将金币抽水返现给房主
							if (taxValueEx > 0 && pRedisPHP->GetUserFriendsGroupMoney(m_friendsGroupMsg.friendsGroupID, ManagerId, FirMoney))
							{
								FirMoney += taxValueEx;
								pRedisPHP->SetUserFriendsGroupMoney(m_friendsGroupMsg.friendsGroupID, ManagerId, taxValueEx, true, RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP, roomID);

								// 通知火币变化
								m_pDataManage->SendFireCoinChangeToLogonServer(m_friendsGroupMsg.friendsGroupID, ManagerId, FirMoney, RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP, taxValueEx);
							}
							llReturnMoney += taxValueEx;
						}
					}
				}
			}

			// 生成战绩
			sprintf(userInfoList + strlen(userInfoList), "%d,%lld|", userID, m_uScore[i]);

			//生成抽水
			sprintf(pumpInfoList + strlen(pumpInfoList), "%d,%lld|", userID, taxValue);

			//生成胜局数
			sprintf(winCountInfoList + strlen(winCountInfoList), "%d,%d|", userID, m_gameWinCount[i]);

			//累计抽水
			llAllPumpScore += taxValue;

			//设置俱乐部玩家输赢分数
			if (m_friendsGroupMsg.friendsGroupID > 0)
			{
				if (roomType == ROOM_TYPE_PRIVATE)
				{
					pRedisPHP->SetUserFriendsGroupResNums(m_friendsGroupMsg.friendsGroupID, userID, "score", m_uScore[i], true);
				}
				else if (roomType == ROOM_TYPE_FRIEND)
				{
					pRedisPHP->SetUserFriendsGroupResNums(m_friendsGroupMsg.friendsGroupID, userID, "money", m_uScore[i], true);
				}
				else if (roomType == ROOM_TYPE_FG_VIP)
				{
					pRedisPHP->SetUserFriendsGroupResNums(m_friendsGroupMsg.friendsGroupID, userID, "fireCoin", m_uScore[i], true);
				}
			}

			//生成金币业绩
			if (roomType == ROOM_TYPE_FRIEND && m_pDataManage->m_uNameID != 37550102)
			{
				char tableName[128] = "";
				ConfigManage()->GetTableNameByDate(TBL_WEB_AGENT_PUMP_MONEY, tableName, sizeof(tableName));
				BillManage()->WriteBill(&m_pDataManage->m_SQLDataManage, "INSERT INTO %s (userID,time,money,changeMoney,reason,roomID,friendsGroupID,rateMoney) VALUES(%d,%d,%lld,%lld,%d,%d,%d,%lld)",
					tableName, userID, currTime, pRedis->GetUserResNums(userID, "money"), m_uScore[i], RESOURCE_CHANGE_REASON_GAME_FINISHED, roomID, m_friendsGroupMsg.friendsGroupID, taxValue);
			}
		}

		// 人数过大会溢出
		memcpy(simpleGradeInfo.userInfoList, userInfoList, sizeof(simpleGradeInfo.userInfoList));

		// 战绩
		if (m_gradeIDVec.size() > 0)
		{
			pRedis->SetPrivateDeskSimpleInfo(userIDVec, simpleGradeInfo, m_gradeIDVec);
		}

		// 不能超过512个字节
		// 拼接
		sprintf(userInfoList + strlen(userInfoList), "#%s#%s#", pumpInfoList, winCountInfoList);

		// 俱乐部相关
		if (m_friendsGroupMsg.friendsGroupID > 0)
		{
			// 生成俱乐部战绩
			BillManage()->WriteBill(&m_pDataManage->m_SQLDataManage,
				"INSERT INTO %s (friendsGroupID,userID,msgID,time,roomID,realPlayCount,playCount,playMode,passwd,userInfoList,roomType,srcType,roomTipType,roomTipTypeNums) \
				VALUES(%d,%d,%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,%d,%d)",
				TBL_FG_RECORD_ACCOUNTS, m_friendsGroupMsg.friendsGroupID, m_masterID, 0, currTime, roomID, m_finishedGameCount, m_iBuyGameCount, m_playMode
				, m_szDeskPassWord, userInfoList, roomType, m_friendsGroupMsg.friendsGroupDeskNumber, m_roomTipType, m_roomTipTypeNums);

			// 生成俱乐部消耗
			int costMoney = 0, costJewels = 0, moneyPump = 0, fireCoinPump = 0;
			if (buyDeskInfo.gameID > 0 && m_payType == PAY_TYPE_NORMAL)
			{
				if (buyDeskInfo.costResType == RESOURCE_TYPE_MONEY)
				{
					costMoney = buyDeskInfo.costNums;
				}
				else if (buyDeskInfo.costResType == RESOURCE_TYPE_JEWEL)
				{
					costJewels = buyDeskInfo.costNums;
				}
			}
			if (roomType == ROOM_TYPE_FG_VIP)
			{
				fireCoinPump = (int)llAllPumpScore;
			}
			else if (roomType == ROOM_TYPE_FRIEND)
			{
				moneyPump = (int)llAllPumpScore;
			}

			// 生成俱乐部消耗
			BillManage()->WriteBill(&m_pDataManage->m_SQLDataManage,
				"INSERT INTO %s (friendsGroupID,time,userID,costMoney,costJewels,fireCoinRecharge,fireCoinExchange,moneyPump,fireCoinPump,passwd) \
				VALUES(%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s')",
				TBL_FG_COST_ACCOUNTS, m_friendsGroupMsg.friendsGroupID, currTime, m_masterID, costMoney, costJewels, 0, 0, moneyPump, fireCoinPump, m_szDeskPassWord);
		}

		//把剩余金币返回给群主
		UserData userData;
		llAllPumpScore -= llReturnMoney;
		if (llAllPumpScore > 0 && roomType == ROOM_TYPE_FRIEND && m_friendsGroupMsg.friendsGroupID > 0 && pRedis->GetUserData(m_masterID, userData))
		{
			userData.money += llAllPumpScore;
			pRedis->SetUserMoneyEx(m_masterID, llAllPumpScore, true, RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP, roomID, 0, 0, m_friendsGroupMsg.friendsGroupID);

			// 通知金币变化
			m_pDataManage->SendResourcesChangeToLogonServer(m_masterID, RESOURCE_TYPE_MONEY, userData.money, RESOURCE_CHANGE_REASON_GOLD_ROOM_PUMP, llAllPumpScore);
		}

		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_GAME_RECORD_INFO, tableName, sizeof(tableName));

		// 生成游戏记录
		BillManage()->WriteBill(&m_pDataManage->m_SQLDataManage,
			"INSERT INTO %s (passwd,deskIdx,roomID,createTime,beginTime,finishedTime,userIDList) VALUES('%s',%d,%d,%d,%d,%d,'%s')",
			tableName, m_szDeskPassWord, m_deskIdx, roomID,
			(int)privateDeskInfo.createTime, (int)m_beginTime, currTime, userInfoList);
	}

	// 清理房间数据
	ClearAllData(privateDeskInfo);
}

bool CGameDesk::ChangeUserPointPrivate(int *arPoint, bool *bCut, char *pVideoCode, bool bGrade, char * gameData)
{
	if (NULL == arPoint)
	{
		return false;
	}
	long long llArPont[MAX_PLAYER_GRADE] = { 0 };
	for (int i = 0; i < m_iConfigCount; i++)
	{
		llArPont[i] = arPoint[i];
	}
	return ChangeUserPointPrivate(llArPont, bCut, pVideoCode, bGrade, gameData);
}

bool CGameDesk::ChangeUserPointPrivate(long long *arPoint, bool *bCut, char *pVideoCode, bool bGrade, char * gameData)
{
	if (!arPoint)
	{
		ERROR_LOG("invalid arPoint");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("redis initialized failed");
		return false;
	}

	LoaderNotifyUserGrade userGrade;
	char userInfoList[512] = "";

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i >= MAX_PLAYER_GRADE)
		{
			break;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		pRedis->SetUserTotalGameCount(userID);

		if (arPoint[i] > 0)
		{
			pRedis->SetUserWinCount(userID);
			m_gameWinCount[i] ++;
		}

		m_uScore[i] += arPoint[i];

		userGrade.grade[i] = (int)m_uScore[i];
		sprintf(userInfoList + strlen(userInfoList), "%d,%lld|", userID, arPoint[i]);
	}

	m_finishedTime = time(NULL);

	BroadcastDeskData(&userGrade, sizeof(userGrade), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GRADE);

	if (bGrade && !AddDeskGrade(pVideoCode, gameData, userInfoList))
	{
		return false;
	}

	return true;
}

bool CGameDesk::ChangeUserPoint(int *arPoint, bool *bCut, long long * rateMoney/* = NULL*/)
{
	if (NULL == arPoint)
	{
		return false;
	}

	long long llArPont[MAX_PEOPLE] = { 0 };

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		llArPont[i] = arPoint[i];
	}

	return ChangeUserPoint(llArPont, bCut, rateMoney);
}

bool CGameDesk::ChangeUserPoint(long long *arPoint, bool *bCut, long long * rateMoney/* = NULL*/)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	//专门计算输赢和抽水
	long long currSystemTaxMoney = 0;
	long long currSystemTaxRobotMoney = 0;
	long long i64RealPeopleWinMoney = 0;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		//更新内存金币
		long long llRedisMoney = pRedis->GetUserResNums(userID, "money");
		if (llRedisMoney != pUser->money)
		{
			ERROR_LOG("内存金币和redis金币不一致，userID=%d,redisMoney=%lld,memMoney=%lld", userID, llRedisMoney, pUser->money);
			pUser->money = llRedisMoney;
		}

		if (!pUser->isVirtual)
		{
			pRedis->SetUserTotalGameCount(userID);
		}

		long long userWinMoney = arPoint[i];
		long long taxValue = 0;
		if (rateMoney && rateMoney[i] > 0)
		{
			taxValue = rateMoney[i];
			currSystemTaxMoney += taxValue;
		}

		if (pUser->isVirtual)
		{
			currSystemTaxRobotMoney += taxValue;
		}
		else
		{
			i64RealPeopleWinMoney += userWinMoney;
		}

		if (arPoint[i] > 0 && !pUser->isVirtual)
		{
			pRedis->SetUserWinCount(userID);
		}

		pRedis->SetUserMoneyEx(userID, userWinMoney, true, RESOURCE_CHANGE_REASON_GAME_FINISHED, m_pDataManage->GetRoomID(), taxValue, pUser->isVirtual, m_friendsGroupMsg.friendsGroupID, GetRoomType());
		pUser->money += userWinMoney;
		if (pUser->money < 0)
		{
			pUser->money = 0;
		}

		UserSimpleInfo msg;
		if (!MakeUserSimpleInfo((BYTE)i, msg))
		{
			continue;
		}

		if (GetRoomSort() == ROOM_SORT_NORMAL || GetRoomSort() == ROOM_SORT_SCENE)
		{
			BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MONEY);
		}
		else
		{
			SendGameData((BYTE)i, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MONEY, 0);
		}
	}

	m_finishedTime = time(NULL);

	// 统计系统输赢
	long long llGameWinMoney = i64RealPeopleWinMoney + (currSystemTaxMoney - currSystemTaxRobotMoney);
	if (llGameWinMoney != 0)
	{
		pRedis->SetRoomGameWinMoney(m_pDataManage->GetRoomID(), -llGameWinMoney, true);
	}

	// 系统抽水
	if (currSystemTaxMoney - currSystemTaxRobotMoney > 0)
	{
		pRedis->SetRoomPercentageWinMoney(m_pDataManage->GetRoomID(), currSystemTaxMoney - currSystemTaxRobotMoney, true);
	}

	// 统计输赢场次，直接写到数据库中
	if (llGameWinMoney != 0)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_ROOM_WIN_COUNT, tableName, sizeof(tableName));
		BillManage()->WriteBill(&m_pDataManage->m_SQLDataManage,
			"INSERT INTO %s (time,roomID,gameID,runGameCount,winMoney,taxMoney,friendsGroupID,platformCtrlPercent,ctrlParamInfo) VALUES(%lld,%d,%d,%d,%lld,%lld,%d,%lld,'%s')",
			tableName, m_finishedTime, m_pDataManage->GetRoomID(), m_pDataManage->m_uNameID, m_iRunGameCount, -llGameWinMoney, currSystemTaxMoney - currSystemTaxRobotMoney,
			m_friendsGroupMsg.friendsGroupID, pRedis->GetRoomPoolData(m_pDataManage->GetRoomID(), "platformCtrlPercent"), m_ctrlParmRecordInfo.c_str());
	}

	return true;
}

bool CGameDesk::ChangeUserBage(BYTE deskStation, char * gameData, int changeNum, bool add)
{
	if (!gameData)
	{
		return false;
	}

	//机器人不用背包
	if (IsVirtual(deskStation))
	{
		return false;
	}
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	if (changeNum == 0)
	{
		return true;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		return false;
	}
	pRedis->SetUserBag(userID, gameData, changeNum, add);
	return true;
}

bool CGameDesk::ChangeUserPoint(BYTE deskStation, long long point, long long rateMoney, bool notify/* = true*/)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	if (point == 0)
	{
		return true;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		return false;
	}

	if (rateMoney < 0)
	{
		rateMoney = 0;
	}
	pRedis->SetUserMoneyEx(userID, point, true, RESOURCE_CHANGE_REASON_GAME_FINISHED, m_pDataManage->GetRoomID(), rateMoney, pUser->isVirtual, m_friendsGroupMsg.friendsGroupID, GetRoomType());
	pUser->money += point;
	if (pUser->money < 0)
	{
		pUser->money = 0;
	}

	if (notify)
	{
		UserSimpleInfo msg;
		if (!MakeUserSimpleInfo(deskStation, msg))
		{
			return false;
		}

		if (GetRoomSort() == ROOM_SORT_NORMAL || GetRoomSort() == ROOM_SORT_SCENE)
		{
			BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MONEY);
		}
		else
		{
			SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MONEY, 0);
		}
	}

	//统计系统输赢
	if (!pUser->isVirtual)
	{
		pRedis->SetRoomGameWinMoney(m_pDataManage->GetRoomID(), -point, true);
	}

	// 系统抽水
	if (!pUser->isVirtual && rateMoney > 0)
	{
		pRedis->SetRoomPercentageWinMoney(m_pDataManage->GetRoomID(), rateMoney, true);
	}

	return true;
}

bool CGameDesk::ChangeUserFireCoin(int *arPoint, char *pVideoCode /*= NULL*/, bool bGrade/* = false*/, char * gameData/* = NULL*/)
{
	if (NULL == arPoint)
	{
		return false;
	}
	long long llArPont[MAX_PLAYER_GRADE] = { 0 };
	for (int i = 0; i < m_iConfigCount; i++)
	{
		llArPont[i] = arPoint[i];
	}
	return ChangeUserFireCoin(llArPont, pVideoCode, bGrade, gameData);
}

bool CGameDesk::ChangeUserFireCoin(long long *arPoint, char *pVideoCode/* = NULL*/, bool bGrade/* = false*/, char * gameData/* = NULL*/)
{
	if (m_friendsGroupMsg.friendsGroupID <= 0)
	{
		ERROR_LOG("非俱乐部房间禁止更改火币");
		return false;
	}

	if (!arPoint)
	{
		ERROR_LOG("invalid arPoint");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("redis initialized failed");
		return false;
	}

	CRedisPHP* pRedisPHP = m_pDataManage->GetRedisPHP();
	if (!pRedisPHP)
	{
		ERROR_LOG("pRedisPHP initialized failed");
		return false;
	}

	LoaderNotifyUserGrade userGrade;
	char userInfoList[512] = "";

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i >= MAX_PLAYER_GRADE)
		{
			break;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		pRedis->SetUserTotalGameCount(userID);

		long long userWinMoney = arPoint[i];

		if (userWinMoney > 0)
		{
			pRedis->SetUserWinCount(userID);
			m_gameWinCount[i] ++;
		}

		// 修改redis和内存的值
		m_uScore[i] += userWinMoney;
		pUser->fireCoin += (int)userWinMoney;
		pRedisPHP->SetUserFriendsGroupMoney(m_friendsGroupMsg.friendsGroupID, userID, userWinMoney, true, RESOURCE_CHANGE_REASON_GAME_FINISHED, m_pDataManage->GetRoomID());

		userGrade.grade[i] = (int)pUser->fireCoin;
		sprintf(userInfoList + strlen(userInfoList), "%d,%lld|", userID, userWinMoney);
	}

	BroadcastDeskData(&userGrade, sizeof(userGrade), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GRADE);

	m_finishedTime = time(NULL);

	if (bGrade && !AddDeskGrade(pVideoCode, gameData, userInfoList))
	{
		return false;
	}

	return true;
}

bool CGameDesk::ChangeUserPointGoldRoom(int *arPoint, char *pVideoCode/* = NULL*/, bool bGrade /*= false*/, char * gameData/* = NULL*/, long long * rateMoney/* = NULL*/)
{
	if (NULL == arPoint)
	{
		return false;
	}
	long long llArPont[MAX_PLAYER_GRADE] = { 0 };
	for (int i = 0; i < m_iConfigCount; i++)
	{
		llArPont[i] = arPoint[i];
	}
	return ChangeUserPointGoldRoom(llArPont, pVideoCode, bGrade, gameData, rateMoney);
}

bool CGameDesk::ChangeUserPointGoldRoom(long long *arPoint, char *pVideoCode /*= NULL*/, bool bGrade/* = false*/, char * gameData/* = NULL*/, long long * rateMoney/* = NULL*/)
{
	if (!arPoint)
	{
		ERROR_LOG("invalid arPoint");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	//专门计算输赢和抽水
	long long currSystemTaxMoney = 0;
	long long currSystemTaxRobotMoney = 0;
	long long i64RealPeopleWinMoney = 0;

	char userInfoList[512] = "";
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i >= MAX_PLAYER_GRADE)
		{
			break;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		//更新内存金币
		long long llRedisMoney = pRedis->GetUserResNums(userID, "money");
		if (llRedisMoney != pUser->money)
		{
			ERROR_LOG("内存金币和redis金币不一致，userID=%d,redisMoney=%lld,memMoney=%lld", userID, llRedisMoney, pUser->money);
			pUser->money = llRedisMoney;
		}

		pRedis->SetUserTotalGameCount(userID);

		long long userWinMoney = arPoint[i];
		if (userWinMoney > 0)
		{
			pRedis->SetUserWinCount(userID);
			m_gameWinCount[i] ++;
		}

		long long taxValue = 0;
		if (rateMoney && rateMoney[i] > 0)
		{
			taxValue = rateMoney[i];
			currSystemTaxMoney += taxValue;
		}

		if (pUser->isVirtual)
		{
			currSystemTaxRobotMoney += taxValue;
		}
		else
		{
			i64RealPeopleWinMoney += userWinMoney;
		}

		pRedis->SetUserMoneyEx(userID, userWinMoney, true, RESOURCE_CHANGE_REASON_GAME_FINISHED, m_pDataManage->GetRoomID(), taxValue, pUser->isVirtual, m_friendsGroupMsg.friendsGroupID);
		pUser->money += userWinMoney;
		if (pUser->money < 0)
		{
			pUser->money = 0;
		}

		m_uScore[i] += userWinMoney;
		sprintf(userInfoList + strlen(userInfoList), "%d,%lld|", userID, userWinMoney);

		if (userWinMoney == 0) //没有输赢，不用发送数据
		{
			continue;
		}

		UserSimpleInfo msg;
		if (!MakeUserSimpleInfo((BYTE)i, msg))
		{
			continue;
		}

		BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MONEY);
	}

	m_finishedTime = time(NULL);

	if (bGrade && !AddDeskGrade(pVideoCode, gameData, userInfoList))
	{
		return false;
	}

	//统计系统输赢
	long long llGameWinMoney = i64RealPeopleWinMoney + (currSystemTaxMoney - currSystemTaxRobotMoney);
	if (llGameWinMoney != 0)
	{
		pRedis->SetRoomGameWinMoney(m_pDataManage->GetRoomID(), -llGameWinMoney, true);
	}

	// 系统抽水
	if (currSystemTaxMoney - currSystemTaxRobotMoney > 0)
	{
		pRedis->SetRoomPercentageWinMoney(m_pDataManage->GetRoomID(), currSystemTaxMoney - currSystemTaxRobotMoney, true);
	}

	return true;
}

bool CGameDesk::ChangeUserPointJewelsRoom(long long *arPoint, char *pVideoCode /*= NULL*/, bool bGrade /*= false*/, char * gameData/* = NULL*/, long long * rateJewels/* = NULL*/)
{
	if (!arPoint)
	{
		ERROR_LOG("invalid arPoint");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	char userInfoList[512] = "";
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		//更新内存钻石
		int iRedisJewels = (int)pRedis->GetUserResNums(userID, "jewels");
		if (iRedisJewels != pUser->jewels)
		{
			ERROR_LOG("内存钻石和redis钻石不一致，userID=%d,redis=%d,mem=%d", userID, iRedisJewels, pUser->jewels);
			pUser->jewels = iRedisJewels;
		}

		pRedis->SetUserTotalGameCount(userID);

		int userWinJewels = (int)arPoint[i];
		if (userWinJewels > 0)
		{
			pRedis->SetUserWinCount(userID);
		}

		pRedis->SetUserJewelsEx(userID, userWinJewels, true, RESOURCE_CHANGE_REASON_GAME_FINISHED,
			m_pDataManage->GetRoomID(), rateJewels == NULL || rateJewels[i] < 0 ? 0 : (int)rateJewels[i], pUser->isVirtual, m_friendsGroupMsg.friendsGroupID);
		pUser->jewels += userWinJewels;
		if (pUser->jewels < 0)
		{
			pUser->jewels = 0;
		}

		if (bGrade && i < MAX_PLAYER_GRADE)
		{
			if (userWinJewels > 0)
			{
				m_gameWinCount[i] ++;
			}
			m_uScore[i] += userWinJewels;
			sprintf(userInfoList + strlen(userInfoList), "%d,%d|", userID, userWinJewels);
		}

		if (userWinJewels == 0) //没有输赢，不用发送数据
		{
			continue;
		}

		UserSimpleInfo msg;
		if (!MakeUserSimpleInfo((BYTE)i, msg))
		{
			continue;
		}

		BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_JEWELS);
	}

	m_finishedTime = time(NULL);

	if (bGrade && !AddDeskGrade(pVideoCode, gameData, userInfoList))
	{
		return false;
	}

	return true;
}

bool CGameDesk::ChangeUserPointJewelsRoom(int *arPoint, char *pVideoCode /*= NULL*/, bool bGrade/* = false*/, char * gameData/* = NULL*/, long long * rateJewels/* = NULL*/)
{
	if (NULL == arPoint)
	{
		return false;
	}
	long long llArPont[MAX_PEOPLE] = { 0 };
	for (int i = 0; i < m_iConfigCount; i++)
	{
		llArPont[i] = arPoint[i];
	}
	return ChangeUserPointJewelsRoom(llArPont, pVideoCode, bGrade, gameData, rateJewels);
}

bool CGameDesk::SendWatchData(void * pData, int size, int mainID, int assistID, int handleCode)
{
	if (GetRoomType() == ROOM_TYPE_MATCH && m_matchWatchUserID.size() > 0)
	{
		for (auto iter = m_matchWatchUserID.begin(); iter != m_matchWatchUserID.end(); ++iter)
		{
			GameUserInfo *pUser = m_pDataManage->GetUser(*iter);
			if (pUser)
			{
				m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, pData, size, mainID, assistID, handleCode, *iter);
			}
		}
	}

	if (m_watchUserInfoSet.size() <= 0)
	{
		return true;
	}

	auto iter = m_watchUserInfoSet.begin();
	for (; iter != m_watchUserInfoSet.end(); ++iter)
	{
		const WatchUserInfo& info = *iter;
		int userID = info.userID;
		m_pDataManage->SendData(userID, pData, size, mainID, assistID, handleCode);
	}

	return true;
}

bool CGameDesk::SendGameMessage(BYTE deskStation, const char * lpszMessage, int wType/* = SMT_EJECT*/)
{
	if (!lpszMessage)
	{
		return false;
	}

	LoaderNotifyERRMsg msg;

	int sizeCount = strlen(lpszMessage);
	if (sizeCount == 0 || sizeCount >= sizeof(msg.notify))
	{
		return true;
	}

	msg.msgType = wType;
	msg.sizeCount = sizeCount;
	memcpy(msg.notify, lpszMessage, sizeCount);

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID > 0)
	{
		m_pDataManage->SendData(userID, &msg, 8 + msg.sizeCount, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_ERR_MSG, 0);
	}

	return true;
}

// 广播消息，设定不广播特定玩家
void CGameDesk::BroadcastGameMessageExcept(BYTE deskStation, const char * lpszMessage, int wType/* = SMT_EJECT*/)
{
	if (!lpszMessage)
	{
		return;
	}

	LoaderNotifyERRMsg msg;

	int sizeCount = strlen(lpszMessage);
	if (sizeCount == 0 || sizeCount >= sizeof(msg.notify))
	{
		return;
	}

	msg.msgType = wType;
	msg.sizeCount = sizeCount;
	memcpy(msg.notify, lpszMessage, sizeCount);

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].isVirtual || i == deskStation)
		{
			continue;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			m_pDataManage->SendData(userID, &msg, 8 + msg.sizeCount, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_ERR_MSG, 0);
		}
	}
}

bool CGameDesk::GetVideoCode(char *pCode, int iLen)
{
	if (!pCode || iLen <= 0)
	{
		return false;
	}

	char buf[64] = "";

	SYSTEMTIME sys;
	GetLocalTime(&sys);

	sprintf(buf, "%04d%02d%02d%02d%02d%02d%03d",
		sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);

	memcpy(pCode, buf, iLen);

	return true;
}

bool CGameDesk::UserSitDesk(GameUserInfo* pUser)
{
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		return PrivateSitDeskLogic(pUser);
	}
	else if (roomType == ROOM_TYPE_MATCH)
	{
		return MatchRoomSitDeskLogic(pUser);
	}
	else
	{
		return MoneySitDeskLogic(pUser);
	}
}

bool CGameDesk::HandleFrameMessage(BYTE deskStation, unsigned int assistID, void * pData, int size, bool bWatchUser/* = false*/)
{
	switch (assistID)
	{
		// 房间信息
	case MSG_ASS_LOADER_DESK_INFO:
	{
		return OnHandleUserRequestDeskInfo(deskStation);
	}
	//游戏状态
	case MSG_ASS_LOADER_GAME_INFO:
	{
		return OnHandleUserRequestGameInfo(deskStation);
	}
	case MSG_ASS_LOADER_DESK_USERINFO:
	{
		return OnHandleUserRequestALLUserInfo(deskStation);
	}
	case MSG_ASS_LOADER_DESK_ONE_USERINFO:
	{
		return OnHandleUserRequestOneUserInfo(deskStation, pData, size);
	}
	default:
		break;
	}

	return false;
}

bool CGameDesk::UserLeftDesk(GameUserInfo * pUser)
{
	if (!pUser)
	{
		return false;
	}

	// 增加百人类相关逻辑 TODO
	int roomSort = GetRoomSort();
	int roomType = GetRoomType();
	if (roomSort == ROOM_SORT_NORMAL || roomSort == ROOM_SORT_SCENE)
	{
		if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP)
		{
			return PrivateUserLeftDeskLogic(pUser);
		}
		else if (roomType == ROOM_TYPE_MATCH)
		{
			return MatchRoomUserLeftDeskLogic(pUser);
		}
		else
		{
			return MoneyUserLeftDeskLogic(pUser);
		}
	}
	else if (roomSort == ROOM_SORT_HUNDRED)
	{
		return HundredGameUserLeftLogic(pUser);
	}

	return true;
}

bool CGameDesk::OnHandleUserRequestDeskInfo(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	// 发送桌子基本信息
	SendDeskBaseInfo(deskStation);

	// 发送解散信息
	SendDismissData();

	return true;
}

bool CGameDesk::OnHandleUserRequestGameInfo(BYTE deskStation)
{
	bool isWatcher = false;
	if (deskStation >= MAX_PEOPLE)
	{
		isWatcher = true;
	}

	//发送用户游戏状态(游戏实现)
	return OnGetGameStation(deskStation, -1, isWatcher);
}

bool CGameDesk::OnHandleUserRequestALLUserInfo(BYTE deskStation)
{
	SendAllDeskUserInfo(deskStation);

	return true;
}

bool CGameDesk::OnHandleUserRequestOneUserInfo(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(LoaderRequestOneUserSimpleInfo))
	{
		return false;
	}

	LoaderRequestOneUserSimpleInfo* pMessage = (LoaderRequestOneUserSimpleInfo*)pData;
	if (!pMessage)
	{
		return false;
	}

	UserSimpleInfo userInfo;
	if (!MakeUserSimpleInfo(pMessage->deskStation, userInfo))
	{
		return false;
	}

	SendGameData(deskStation, &userInfo, sizeof(userInfo), MSG_MAIN_LOADER_FRAME, MSG_ASS_LOADER_DESK_ONE_USERINFO, 0);
	return true;
}

bool CGameDesk::OnHandleUserRequestDissmiss(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("GetUserIDByDeskStation failed deskStation: %d", deskStation);
		return false;
	}

	// 桌子是否有效
	if (!m_enable)
	{
		ERROR_LOG("invalid desk status m_enable=%d", m_enable);
		return false;
	}

	// 已经有人申请解散了
	if (m_reqDismissDeskStation != INVALID_DESKSTATION)
	{
		ERROR_LOG("deskStation: %d already request dismiss", m_reqDismissDeskStation);
		return true;
	}

	if (!m_isBegin)
	{
		// 还未开始之前只能房主解散
		if (userID != m_masterID)
		{
			ERROR_LOG("user request dismiss before playing game but is not master deskStation:%d userID:%d m_masterID:%d", deskStation, userID, m_masterID);
			return false;
		}
	}

	//判断当前人数
	int currUserCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID > 0)
		{
			currUserCount++;
		}
	}

	// 如果桌子还未开始游戏，那么直接解散
	if (!m_isBegin || currUserCount <= 1)
	{
		// 直接解散(不用进入解散状态)
		OnDeskDissmissFinishSendData();
		OnDeskSuccessfulDissmiss(true);
		return true;
	}

	DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];

	deskUserInfo.dismissType = DISMISS_TYPE_AGREE;

	// 记录相关信息
	m_isDismissStatus = true;
	m_reqDismissDeskStation = deskStation;
	m_reqDismissTime = time(NULL);

	// 广播这件事
	BroadcastDismissData();

	// 启动解散定时器
	SetTimer(IDT_AGREE_DISMISS_DESK, CFG_DISMISS_DESK_WAIT_TIME * 1000);

	return true;
}

bool CGameDesk::OnHandleUserAnswerDissmiss(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(LoaderRequestAnswerDismiss))
	{
		return false;
	}

	LoaderRequestAnswerDismiss* pMessage = (LoaderRequestAnswerDismiss*)pData;
	if (!pMessage)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	if (!m_enable)
	{
		SendGameData(deskStation, NULL, 0, MSG_MAIN_LOADER_DESKDISSMISS, MSG_ASS_LOADER_ANSWER_DESKDISMISS, ERROR_INVALID_DESK);
		return false;
	}

	// 是否处于待解散状态
	if (!m_isDismissStatus)
	{
		SendGameData(deskStation, NULL, 0, MSG_MAIN_LOADER_DESKDISSMISS, MSG_ASS_LOADER_ANSWER_DESKDISMISS, ERROR_DESK_NOT_INDISMISSS);
		return false;
	}

	if (m_reqDismissDeskStation == INVALID_DESKSTATION)
	{
		return false;
	}

	// 是否已经操作过了
	DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];

	if (deskUserInfo.dismissType != DISMISS_TYPE_DEFAULT)
	{
		return false;
	}

	if (pMessage->isAgree == true)
	{
		return OnUserAgreeDismiss(deskStation);
	}
	else
	{
		return OnDeskDismissFailed();
	}

	return false;
}

bool CGameDesk::OnWatchUserOffline(int userID)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	auto iter = m_watchUserInfoSet.begin();
	for (; iter != m_watchUserInfoSet.end(); ++iter)
	{
		if (iter->userID == userID)
		{
			break;
		}
	}

	if (iter == m_watchUserInfoSet.end())
	{
		ERROR_LOG("watcher list find user(%d) failed", userID);
		return false;
	}

	m_watchUserInfoSet.erase(iter);

	int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);
	pRedis->SetPrivateDeskCurrWatchUserCount(deskMixID, (int)m_watchUserInfoSet.size());

	m_pDataManage->RemoveWatchUser(userID);

	return true;
}

BYTE CGameDesk::AllocWatcherDeskStation()
{
	int tempList[INVALID_DESKSTATION] = { 0 };
	for (auto iter = m_watchUserInfoSet.begin(); iter != m_watchUserInfoSet.end(); ++iter)
	{
		const WatchUserInfo& info = *iter;
		if (info.deskStation < INVALID_DESKSTATION)
		{
			tempList[info.deskStation] = 1;
		}
	}

	for (BYTE i = MAX_PEOPLE; i < INVALID_DESKSTATION; i++)
	{
		if (tempList[i] == 0)
		{
			return i;
		}
	}

	return INVALID_DESKSTATION;
}

bool CGameDesk::IsWatcher(int userID)
{
	auto iter = m_watchUserInfoSet.begin();
	for (; iter != m_watchUserInfoSet.end(); ++iter)
	{
		const WatchUserInfo& info = *iter;
		if (info.userID == userID)
		{
			return true;
		}
	}

	return false;
}

bool CGameDesk::OnHandleTalkMessage(BYTE deskStation, void * pData, int size)
{
	if (size <= 4)
	{
		return false;
	}

	LoaderRequestTalk* pMessage = (LoaderRequestTalk*)pData;
	if (!pMessage)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	if (pMessage->sizeCount + 4 != size || pMessage->sizeCount >= 1024)
	{
		ERROR_LOG("聊天字节数量发送错误,userID=%d,pMessage->sizeCount=%d", userID, pMessage->sizeCount);
		return true;
	}

	char words[1024] = "";
	memcpy(words, pMessage->words, pMessage->sizeCount);

	if (CUtil::IsContainDirtyWord(words) == true)
	{
		SendGameData(deskStation, 0, NULL, MSG_MAIN_LOADER_VOICEANDTALK, MSG_ASS_LOADER_TALK, ERROR_HAVE_DIRTYWORD);
		return true;
	}

	LoaderNotifyTalk msg;

	msg.userID = userID;
	memcpy(msg.words, words, sizeof(words));

	// 计算发送字节数量
	msg.sizeCount = min(strlen(msg.words) + 1, sizeof(msg.words));
	int iSendSize = 8 + msg.sizeCount;

	BroadcastDeskData(&msg, iSendSize, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_TALK, false);

	return true;
}

bool CGameDesk::OnHandleVoiceMessage(BYTE deskStation, void * pData, int size)
{
	if (size != sizeof(LoaderRequestVoice))
	{
		return false;
	}

	LoaderRequestVoice* pMessage = (LoaderRequestVoice*)pData;
	if (!pMessage)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	LoaderNotifyVoice msg;

	msg.userID = userID;
	msg.voiceID = pMessage->voiceID;

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_VOICE, false);

	return true;
}

bool CGameDesk::OnHandleUserRequestMagicExpress(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(LoaderRequestMagicExpress))
	{
		return false;
	}

	LoaderRequestMagicExpress* pMessage = (LoaderRequestMagicExpress*)pData;
	if (!pMessage)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("GetUserIDByDeskStation failed deskStation=%d", deskStation);
		return false;
	}

	int targetUserID = pMessage->userID;
	if (targetUserID == userID)
	{
		return true;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	OtherConfig otherConfig;
	if (!pRedis->GetOtherConfig(otherConfig))
	{
		otherConfig = ConfigManage()->GetOtherConfig();
	}
	int roomType = GetRoomType();

	if (roomType == ROOM_TYPE_MATCH)
	{
		otherConfig.useMagicExpressCostMoney = 0;
	}

	if (otherConfig.useMagicExpressCostMoney > 0)
	{
		if (roomType == ROOM_TYPE_GOLD)
		{
			RoomBaseInfo roomBasekInfo;
			RoomBaseInfo* pRoomBaseInfo = NULL;
			if (pRedis->GetRoomBaseInfo(m_pDataManage->GetRoomID(), roomBasekInfo))
			{
				pRoomBaseInfo = &roomBasekInfo;
			}
			else
			{
				pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_pDataManage->GetRoomID());
			}
			if (pRoomBaseInfo && pRoomBaseInfo->minPoint > 0 && pUser->money - otherConfig.useMagicExpressCostMoney < pRoomBaseInfo->minPoint)
			{
				SendGameMessage(deskStation, "资源不足，无法发魔法表情");
				return true;
			}
		}

		if (pUser->money < otherConfig.useMagicExpressCostMoney)
		{
			SendGameMessage(deskStation, "资源不足，无法发魔法表情");
			return true;
		}

		// 扣钱
		long long newMoney = pUser->money - otherConfig.useMagicExpressCostMoney;
		pUser->money = newMoney;
		pRedis->SetUserMoneyEx(userID, -otherConfig.useMagicExpressCostMoney, true, RESOURCE_CHANGE_REASON_MAGIC_EXPRESS, m_pDataManage->GetRoomID(), 0, pUser->isVirtual, m_friendsGroupMsg.friendsGroupID);

		if (pUser->isVirtual)
		{
			//将金币回收到奖池		
			//pRedis->SetRoomPoolMoney(m_pDataManage->GetRoomID(), otherConfig.useMagicExpressCostMoney, true);
		}
		else
		{
			//将金币放入其它方式赢钱
			pRedis->SetRoomPoolData(m_pDataManage->GetRoomID(), "otherWinMoney", otherConfig.useMagicExpressCostMoney, true);
		}

		// 通知钱变化
		NotifyUserResourceChange(userID, RESOURCE_TYPE_MONEY, newMoney, -otherConfig.useMagicExpressCostMoney);
	}

	// 广播使用魔法表情的通知
	LoaderNotifyMagicExpress msg;
	msg.magicType = pMessage->magicType;
	msg.srcUserID = userID;
	msg.targetUserID = targetUserID;

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MAGICEXPRESS, false);

	return true;
}

bool CGameDesk::OnHandleUserRequestAuto(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];

	if (deskUserInfo.isAuto == false)
	{
		deskUserInfo.isAuto = true;

		LoaderNotifyIsAuto msg;

		msg.isAuto = true;
		msg.userID = userID;

		BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_ISAUTO, false);
	}

	return true;
}

bool CGameDesk::OnHandleUserRequestCancelAuto(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];

	deskUserInfo.isAuto = false;

	LoaderNotifyIsAuto msg;

	msg.isAuto = false;
	msg.userID = userID;

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_ISAUTO, false);

	return true;
}

void CGameDesk::MakeAllUserInfo(char* buf, int size, int& realSize)
{
	int roomType = GetRoomType();

	// 遍历寻找所有玩家的信息
	std::vector<int> temp;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			temp.push_back(userID);
		}
	}

	int pos = 0;

	int* p = (int *)buf;
	*p = 0;
	pos += sizeof(int);

	for (size_t i = 0; i < temp.size(); i++)
	{
		int userID = temp[i];

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID(%d)", userID);
			continue;
		}

		if (pos + sizeof(UserSimpleInfo) > size - sizeof(NetMessageHead))
		{
			break;
		}

		UserSimpleInfo* pUserInfo = (UserSimpleInfo*)(buf + pos);
		if (pUserInfo)
		{
			pUserInfo->userID = userID;
			memcpy(pUserInfo->name, pUser->name, sizeof(pUser->name));
			memcpy(pUserInfo->headURL, pUser->headURL, sizeof(pUser->headURL));
			pUserInfo->isOnline = pUser->IsOnline;
			pUserInfo->deskStation = pUser->deskStation;
			pUserInfo->playStatus = pUser->playStatus;
			pUserInfo->iDeskIndex = pUser->deskIdx;
			memcpy(pUserInfo->longitude, pUser->longitude, sizeof(pUserInfo->longitude));
			memcpy(pUserInfo->latitude, pUser->latitude, sizeof(pUserInfo->latitude));
			memcpy(pUserInfo->ip, pUser->ip, sizeof(pUserInfo->ip));
			memcpy(pUserInfo->address, pUser->address, sizeof(pUserInfo->address));
			memcpy(pUserInfo->motto, pUser->motto, sizeof(pUserInfo->motto));

			pUserInfo->sex = pUser->sex;
			pUserInfo->isAuto = IsAuto(pUser->deskStation);
			pUserInfo->money = pUser->money;
			pUserInfo->jewels = pUser->jewels;

			if (roomType == ROOM_TYPE_PRIVATE)
			{
				pUserInfo->score = (long)m_uScore[pUser->deskStation];
			}
			else if (roomType == ROOM_TYPE_FG_VIP)
			{
				pUserInfo->score = (long)pUser->fireCoin;
			}
			else if (roomType == ROOM_TYPE_MATCH)
			{
				pUserInfo->score = (long)pUser->matchSocre;
			}

			pos += sizeof(UserSimpleInfo);
			*p = *p + 1;
		}
	}

	realSize = pos;
}

void CGameDesk::HundredMakeAllUserInfo(char* buf, int size, int& realSize)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return;
	}

	// 遍历寻找所有玩家的信息
	std::vector<long> temp;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			temp.push_back(userID);
		}
	}

	int pos = 0;

	int* p = (int *)buf;
	*p = 0;
	pos += sizeof(int);

	for (size_t i = 0; i < temp.size(); i++)
	{
		int userID = temp[i];

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID(%d)", userID);
			continue;
		}

		if (pos + sizeof(HundredGameUserSimpleInfo) > size - sizeof(NetMessageHead))
		{
			break;
		}

		HundredGameUserSimpleInfo* pUserInfo = (HundredGameUserSimpleInfo*)(buf + pos);
		if (pUserInfo)
		{
			pUserInfo->userID = userID;
			pUserInfo->isOnline = pUser->IsOnline;
			pUserInfo->deskStation = pUser->deskStation;
			pUserInfo->userStatus = pUser->playStatus;
			pUserInfo->isAuto = IsAuto(pUser->deskStation);
			pUserInfo->money = pUser->money;

			pos += sizeof(HundredGameUserSimpleInfo);
			*p = *p + 1;
		}
	}

	realSize = pos;
}

bool CGameDesk::OnUserRequsetGameBegin(int userID)
{
	//// 只要是房主,就能开始(房主能否在外面(比如大厅)控制开始todo)
	//if (userID != m_masterID)
	//{
	//	ERROR_LOG("user request gamebegin but is not master userID(%d) m_masterID(%d)", userID, m_masterID);
	//	return false;
	//}

	if (m_isBegin == true)
	{
		ERROR_LOG("game is already begin deskIdx(%d)", m_deskIdx);
		return false;
	}

	int errCode = 0;
	if (CanBeginGame() == true)
	{
		GameBegin(0);
	}
	else
	{
		errCode = ERROR_GAME_CANNOT_GAMEBEGIN;
	}

	m_pDataManage->SendData(userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_GAME_BEGIN, errCode);

	return true;
}

bool CGameDesk::OnWatcherLeftDesk(GameUserInfo* pUser, const PrivateDeskInfo & deskInfo)
{
	if (!pUser)
	{
		ERROR_LOG("invalid pUser is null");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	auto iter = m_watchUserInfoSet.begin();
	for (; iter != m_watchUserInfoSet.end(); ++iter)
	{
		const WatchUserInfo& info = *iter;
		if (pUser->userID == info.userID)
		{
			break;
		}
	}

	if (iter == m_watchUserInfoSet.end())
	{
		ERROR_LOG("user(%d) playstatus is watch but not in desk(%d) watcher list", pUser->userID, m_deskIdx);
		return false;
	}

	// 旁观者列表中移除玩家
	m_watchUserInfoSet.erase(iter);

	// 修改缓存人数
	int deskMixID = m_pDataManage->GetRoomID() * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;
	pRedis->SetPrivateDeskCurrWatchUserCount(deskMixID, m_watchUserInfoSet.size());


	m_pDataManage->RemoveWatchUser(pUser->userID);

	return true;
}

bool CGameDesk::OnDeskUserLeftDesk(GameUserInfo* pUser, const PrivateDeskInfo& deskInfo)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	if (pUser->deskStation >= (BYTE)m_deskUserInfoVec.size())
	{
		ERROR_LOG("user deskStation is invalid deskStation=%d,userID = %d", pUser->deskStation, pUser->userID);
		return false;
	}

	if (pUser->playStatus == USER_STATUS_STAND)
	{
		ERROR_LOG("玩家已经是站起状态 pUser->playStatus=%d,deskStation=%d,userID=%d", pUser->playStatus, pUser->deskStation, pUser->userID);
		return false;
	}

	if (m_bPlayGame)
	{
		ERROR_LOG("游戏中不能离开。deskStation=%d,userID=%d", pUser->deskStation, pUser->userID);
		SendGameMessage(pUser->deskStation, "游戏中不能离开");
		return false;
	}

	int roomType = GetRoomType();
	bool bIsRecord = false;

	if (m_isBegin && (roomType == ROOM_TYPE_FRIEND && pUser->money < m_RemoveMinPoint
		|| roomType == ROOM_TYPE_FG_VIP && pUser->fireCoin < m_RemoveMinPoint))
	{
		bIsRecord = true;
	}
	else
	{
		// 如果是游戏中则无法离开
		if (m_isBegin)
		{
			ERROR_LOG("game is begin but user want left desk userID=%d", pUser->userID);
			SendGameMessage(pUser->deskStation, "游戏中不能离开");
			return false;
		}
	}

	// 通知所有玩家
	BroadcastUserLeftData(pUser->deskStation, REASON_KICKOUT_STAND);

	// 清理玩家桌子数据
	m_deskUserInfoVec[pUser->deskStation].clear();
	if (bIsRecord)
	{
		// 记录被清理的玩家数据
		m_deskUserInfoVec[pUser->deskStation].leftRoomUser = pUser->userID;
	}

	// 清理玩家游戏数据
	pUser->playStatus = USER_STATUS_STAND;
	pUser->deskIdx = INVALID_DESKIDX;
	pUser->deskStation = INVALID_DESKSTATION;

	// 清除缓存中玩家数量
	int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);
	int currDeskUserCount = deskInfo.currDeskUserCount - 1;
	pRedis->SetPrivateDeskCurrUserCount(deskMixID, currDeskUserCount);

	// 通知桌子人数变化
	NotifyLogonBuyDeskInfoChange(m_masterID, currDeskUserCount, pUser->userID, 1, deskMixID);

	if ((roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP) && m_pDataManage->IsCanWatch() && m_autoBeginMode == 0)
	{
		// 第一局
		if (m_iRunGameCount == 0 && CanBeginGame())
		{
			SetBeginUser();
			m_pDataManage->SendData(m_beginUserID, NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_CANBEGIN, 0);
		}

		if (m_iRunGameCount >= 1 && CanBeginGame())
		{
			// 第一局之后的游戏不受房主控制
			GameBegin(0);
		}
	}
	else
	{
		if (CanBeginGame())
		{
			GameBegin(0);
		}
	}

	return true;
}

void CGameDesk::BroadcastDeskData(void * pData, int size, unsigned int mainID, unsigned int assistID, bool sendVirtual/* = true*/, unsigned int handleCode/* = 0*/)
{
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (!sendVirtual && m_deskUserInfoVec[i].isVirtual)
		{
			continue;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			m_pDataManage->SendData(userID, pData, size, mainID, assistID, handleCode);
		}
	}

	SendWatchData(pData, size, mainID, assistID, 0);
}

void CGameDesk::SendAllDeskUserInfo(BYTE deskStation)
{
	char buf[MAX_TEMP_SENDBUF_SIZE] = "";		// 加入对拼接数据包长度的检查
	int realSize = 0;

	if (GetRoomSort() == ROOM_SORT_NORMAL || GetRoomSort() == ROOM_SORT_SCENE)
	{
		MakeAllUserInfo(buf, sizeof(buf), realSize);
		SendGameData(deskStation, buf, realSize, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_ALL_USERINFO, 0);
	}
	else
	{
		HundredMakeAllUserInfo(buf, sizeof(buf), realSize);
		SendGameData(deskStation, buf, realSize, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_HUNDRED_ALL_USER, 0);
	}
}

void CGameDesk::SendDeskBaseInfo(BYTE deskStation)
{
	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("GetUserIDByDeskStation failed %d", deskStation);
		return;
	}

	UserData masterData;
	if (!pRedis->GetUserData(m_masterID, masterData))
	{
		ERROR_LOG("GetUserData failed: %d", m_masterID);
		return;
	}

	LoaderNotifyDeskInfo msg;

	msg.deskIdx = m_deskIdx;
	memcpy(msg.deskPasswd, m_szDeskPassWord, sizeof(m_szDeskPassWord));
	msg.deskPasswdLen = 6;
	msg.masterID = m_masterID;
	memcpy(msg.masterName, masterData.name, sizeof(masterData.name));
	msg.runGameCount = m_iRunGameCount;
	msg.totalGameCount = m_iBuyGameCount;
	msg.isDismissStatus = m_isDismissStatus;

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_BASEINFO, 0);

	// 处于解散状态的话，额外推送解散面板信息
	if (m_isDismissStatus)
	{
		if (IsWatcher(userID) == false)
		{
			BroadcastDismissData();
		}
	}
}

void CGameDesk::BroadcastDeskBaseInfo()
{
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			SendDeskBaseInfo((BYTE)i);
		}
	}
}

int CGameDesk::GetUserIDByDeskStation(BYTE deskStation)
{
	if (deskStation == INVALID_DESKSTATION)
	{
		return 0;
	}

	if (deskStation < MAX_PEOPLE)
	{
		// 玩家
		if (deskStation < m_deskUserInfoVec.size())
		{
			return m_deskUserInfoVec[deskStation].userID;
		}
	}
	else
	{
		// 旁观者
		for (auto iter = m_watchUserInfoSet.begin(); iter != m_watchUserInfoSet.end(); ++iter)
		{
			const WatchUserInfo& info = *iter;
			if (deskStation == info.deskStation)
			{
				return info.userID;
			}
		}
	}

	return 0;
}

BYTE CGameDesk::GetDeskStationByUserID(int userID)
{
	if (userID <= 0)
	{
		return INVALID_DESKSTATION;
	}

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID == userID)
		{
			return (BYTE)i;
		}
	}

	return INVALID_DESKSTATION;
}

bool CGameDesk::GetUserData(BYTE deskStation, UserData & userData)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	bool ret = pRedis->GetUserData(userID, userData);
	if (!ret)
	{
		return false;
	}

	return true;
}

bool CGameDesk::GetUserData(BYTE deskStation, GameUserInfo & userData)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	if (!m_pDataManage)
	{
		return false;
	}

	GameUserInfo * pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		return false;
	}

	userData = *pUser;

	return true;
}

int CGameDesk::GetRoomType()
{
	if (m_pDataManage)
	{
		return m_pDataManage->m_InitData.iRoomType;
	}

	return ROOM_TYPE_PRIVATE;		// 默认房卡场
}

int CGameDesk::GetRoomSort()
{
	if (m_pDataManage)
	{
		return m_pDataManage->m_InitData.iRoomSort;
	}

	return ROOM_SORT_NORMAL;
}

int CGameDesk::GetRoomLevel()
{
	if (m_pDataManage)
	{
		return m_pDataManage->m_InitData.iRoomLevel;
	}

	return 0;
}

void CGameDesk::BroadcastUserSitData(BYTE deskStation)
{
	UserSimpleInfo msg;
	if (!MakeUserSimpleInfo(deskStation, msg))
	{
		return;
	}

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_SIT);
}

void CGameDesk::BroadcastUserSitDataExceptMyself(BYTE deskStation)
{
	UserSimpleInfo msg;
	if (!MakeUserSimpleInfo(deskStation, msg))
	{
		return;
	}

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i == deskStation && !m_deskUserInfoVec[i].isVirtual)
		{
			continue;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			m_pDataManage->SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_SIT, 0);
		}
	}
}

void CGameDesk::BroadcastUserSimpleInfo(BYTE deskStation)
{
	UserSimpleInfo msg;
	if (!MakeUserSimpleInfo(deskStation, msg))
	{
		return;
	}

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_INFO);
}

bool CGameDesk::BroadcastUserLeftData(BYTE deskStation, int reason)
{
	LoaderNotifyUserLeft msg;

	msg.reason = reason;

	if (!MakeUserSimpleInfo(deskStation, msg.userInfo))
	{
		return false;
	}

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_LEFT);

	if (IsGoldRoom() && m_pDataManage->IsCanCombineDesk() && !IsPlayGame(deskStation))
	{
		BroadcastGameMessageExcept(deskStation, "有玩家离开游戏，请重新换桌");
	}

	return true;
}

void CGameDesk::SendUserLeftData(BYTE deskStation, int reason)
{
	LoaderNotifyUserLeft msg;

	msg.reason = reason;

	if (!MakeUserSimpleInfo(deskStation, msg.userInfo))
	{
		return;
	}

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_LEFT, 0);
}

void CGameDesk::BroadcastUserAgreeData(BYTE deskStation)
{
	UserSimpleInfo userInfo;
	if (!MakeUserSimpleInfo(deskStation, userInfo))
	{
		return;
	}

	BroadcastDeskData(&userInfo, sizeof(userInfo), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_AGREE);
}

void CGameDesk::BroadcastUserOfflineData(BYTE deskStation)
{
	UserSimpleInfo userInfo;
	if (!MakeUserSimpleInfo(deskStation, userInfo))
	{
		return;
	}

	BroadcastDeskData(&userInfo, sizeof(userInfo), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_OFFLINE);
}

void CGameDesk::BroadcastUserKickoutData(BYTE deskStation)
{
	UserSimpleInfo userInfo;
	if (!MakeUserSimpleInfo(deskStation, userInfo))
	{
		return;
	}

	BroadcastDeskData(&userInfo, sizeof(userInfo), MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_BE_KICKOUT);
}

void CGameDesk::ClearAllData(const PrivateDeskInfo& privateDeskInfo)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("GetRedis failed");
		return;
	}

	int masterID = privateDeskInfo.masterID;
	if (masterID <= 0)
	{
		ERROR_LOG("无效的房主 masterID=%d，清理桌子失败", masterID);
		return;
	}

	int roomID = m_pDataManage->GetRoomID();
	int deskMixID = roomID * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;

	// 清理玩家数据
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID=%d", pUser);
			continue;
		}

		// 设置为站起状态
		pUser->deskIdx = INVALID_DESKIDX;
		pUser->deskStation = INVALID_DESKSTATION;
		pUser->playStatus = USER_STATUS_STAND;

		// 彻底清除断线玩家的相关信息(在线的玩家需要发送登出)
		if (pUser->IsOnline == false)
		{
			m_pDataManage->RemoveOfflineUser(userID);
		}
		else
		{
			pRedis->SetUserRoomID(pUser->userID, 0);
			pRedis->SetUserDeskIdx(pUser->userID, INVALID_DESKIDX);
		}
	}

	// 清理桌子玩家
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		m_deskUserInfoVec[i].clear();
	}

	// 清理旁观者数据
	for (auto iter = m_watchUserInfoSet.begin(); iter != m_watchUserInfoSet.end(); ++iter)
	{
		m_pDataManage->RemoveWatchUser(iter->userID);
	}
	m_watchUserInfoSet.clear();

	//填充俱乐部信息
	m_friendsGroupMsg.friendsGroupID = privateDeskInfo.friendsGroupID;
	if (m_friendsGroupMsg.friendsGroupID != 0)
	{
		m_friendsGroupMsg.friendsGroupDeskNumber = privateDeskInfo.friendsGroupDeskNumber;
	}

	// 清除cache中桌子数据
	pRedis->DelPrivateDeskRecord(deskMixID);

	// 清除房主的开房记录
	pRedis->DelUserBuyDeskInfoInSet(masterID, privateDeskInfo.passwd);

	// 扣除房主购买桌子次数
	if (privateDeskInfo.friendsGroupID == 0)
	{
		pRedis->SetUserBuyingDeskCount(masterID, -1, true);
	}

	// 清理临时战绩数据
	m_gradeIDVec.clear();

	// 通知大厅
	NotifyLogonDeskDissmiss(privateDeskInfo);

	// 清理购买桌子数据
	InitBuyDeskData();

	// 清理解散数据
	InitDismissData();

	m_finishedGameCount = 0;
	m_autoBeginMode = 0;
	m_bGameStopJoin = 0;
	// 置为无效，可以复用
	m_enable = false;
	//清理俱乐部数据
	m_friendsGroupMsg.Init();
}

bool CGameDesk::OnDeskDismissFailed()
{
	// 清除定时器
	KillTimer(IDT_AGREE_DISMISS_DESK);

	// 清除状态
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		m_deskUserInfoVec[i].dismissType = DISMISS_TYPE_DEFAULT;
	}

	InitDismissData();

	// 通知前端
	BroadcastDeskData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_DISMISS_FAILED);

	return true;
}

bool CGameDesk::OnUserAgreeDismiss(BYTE deskStation)
{
	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		return false;
	}

	int roomType = GetRoomType();

	DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];

	deskUserInfo.dismissType = DISMISS_TYPE_AGREE;

	// 广播这件事
	BroadcastDismissData();

	// 在线玩家数量
	int onLineCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (pUser && pUser->IsOnline == true)
		{
			onLineCount++;
		}
	}

	// 同意解散玩家数量(在线)
	int agreeDismissCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		int dismissType = m_deskUserInfoVec[i].dismissType;

		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		//特殊房间特殊处理。金币房和火币房如果玩家金币不足也算是同意解散
		if (m_pDataManage->m_uNameID != 37550102 && (roomType == ROOM_TYPE_FRIEND && pUser->money < m_RemoveMinPoint || roomType == ROOM_TYPE_FG_VIP && pUser->fireCoin < m_RemoveMinPoint))
		{
			agreeDismissCount++;
			INFO_LOG("默认同意解散：userID=%d", userID);
			continue;
		}

		if (dismissType == DISMISS_TYPE_AGREE && pUser->IsOnline)
		{
			agreeDismissCount++;
		}
	}

	if (onLineCount > 0 && agreeDismissCount >= onLineCount)
	{
		// 解散
		OnDeskDissmissFinishSendData();
		OnDeskSuccessfulDissmiss(true);
	}

	return true;
}

void CGameDesk::MakeDismissData(char* buf, int& size)
{
	if (!buf)
	{
		return;
	}

	int userCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID > 0)
		{
			userCount++;
		}
	}

	int pos = 0;

	// 申请者
	BYTE* pDeskStation = (BYTE*)buf;
	*pDeskStation = m_reqDismissDeskStation;
	pos += sizeof(BYTE);

	// 解散剩余的时间
	int* pLeftDismissTime = (int*)(buf + pos);
	*pLeftDismissTime = (int)(m_reqDismissTime + CFG_DISMISS_DESK_WAIT_TIME - time(NULL));
	pos += sizeof(int);

	int* pCfgWaitDismissTime = (int*)(buf + pos);
	*pCfgWaitDismissTime = CFG_DISMISS_DESK_WAIT_TIME;
	pos += sizeof(int);

	// 玩家数量
	int*pdeskUserCount = (int*)(buf + pos);
	*pdeskUserCount = userCount;
	pos += sizeof(int);

	// 人数太多缓冲区不够的问题 TODO
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		const DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
		if (deskUserInfo.userID <= 0)
		{
			continue;
		}

		DeskDismissType *pDismissType = (DeskDismissType *)(buf + pos);
		if (pDismissType)
		{
			pDismissType->deskStation = (BYTE)i;
			pDismissType->dismissType = deskUserInfo.dismissType;

			pos += sizeof(DeskDismissType);
		}
	}

	size = pos;
}

void CGameDesk::BroadcastDismissData()
{
	char buf[MAX_TEMP_SENDBUF_SIZE] = "";
	int size = 0;

	MakeDismissData(buf, size);

	BroadcastDeskData(buf, size, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_DISMISS_INFO);
}

void CGameDesk::SendDismissData()
{
	if (!m_isDismissStatus)
	{
		return;
	}

	char buf[MAX_TEMP_SENDBUF_SIZE] = "";
	int size = 0;

	MakeDismissData(buf, size);

	BroadcastDeskData(buf, size, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_DISMISS_INFO);
}

void CGameDesk::SendLeftWaitAgreeData(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return;
	}

	const DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];

	LoaderNotifyWaitAgree msg;

	int leftWaitAgreeSecs = 0;
	if (!m_bPlayGame)
	{
		leftWaitAgreeSecs = int(GOLD_DESK_TIMEOUT_UNAGREE_KICKOUT_SECS + deskUserInfo.lastWaitAgreeTime - time(NULL));
	}

	msg.cfgWaitAgreeSecs = GOLD_DESK_TIMEOUT_UNAGREE_KICKOUT_SECS;
	msg.leftWaitAgreeSecs = leftWaitAgreeSecs;

	m_pDataManage->SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_LEFT_WAITAGREE_TIME, 0);
}

bool CGameDesk::MakeUserSimpleInfo(BYTE deskStation, UserSimpleInfo & userInfo)
{
	if (deskStation >= m_deskUserInfoVec.size())
	{
		return false;
	}

	const DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];
	int userID = deskUserInfo.userID;
	if (userID <= 0)
	{
		return false;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser Fail userID=%d", userID);
		return false;
	}

	int roomType = GetRoomType();

	userInfo.deskStation = deskStation;
	userInfo.isOnline = pUser->IsOnline;
	userInfo.userID = userID;
	memcpy(userInfo.headURL, pUser->headURL, sizeof(userInfo.headURL));
	memcpy(userInfo.name, pUser->name, sizeof(userInfo.name));
	userInfo.playStatus = pUser->playStatus;
	memcpy(userInfo.longitude, pUser->longitude, sizeof(userInfo.longitude));
	memcpy(userInfo.latitude, pUser->latitude, sizeof(userInfo.latitude));
	memcpy(userInfo.address, pUser->address, sizeof(userInfo.address));
	memcpy(userInfo.ip, pUser->ip, sizeof(userInfo.ip));
	userInfo.sex = pUser->sex;
	userInfo.isAuto = deskUserInfo.isAuto;
	userInfo.money = pUser->money;
	userInfo.jewels = pUser->jewels;
	userInfo.iDeskIndex = pUser->deskIdx;
	memcpy(userInfo.motto, pUser->motto, sizeof(userInfo.motto));

	if (roomType == ROOM_TYPE_PRIVATE)
	{
		userInfo.score = (int)m_uScore[deskStation];
	}
	else if (roomType == ROOM_TYPE_FG_VIP)
	{
		userInfo.score = (int)pUser->fireCoin;
	}
	else if (roomType == ROOM_TYPE_MATCH)
	{
		userInfo.score = (int)pUser->matchSocre;
	}

	return true;
}

void CGameDesk::InitBuyDeskData()
{
	m_beginUserID = 0;
	m_playMode = 0;
	m_payType = PAY_TYPE_NORMAL;
	m_roomTipType = 0;
	m_roomTipTypeNums = 0;
	m_RemoveMinPoint = 1;
	m_MinPoint = 0;
	m_basePoint = 1;
	m_isBegin = false;
	m_iVipGameCount = 0;
	m_iBuyGameCount = 0;
	memset(m_szGameRules, 0, sizeof(m_szGameRules));
	memset(m_szDeskPassWord, 0, sizeof(m_szDeskPassWord));
	memset(m_uScore, 0, sizeof(m_uScore));
	memset(m_gameWinCount, 0, sizeof(m_gameWinCount));
	m_iRunGameCount = 0;
	m_masterID = 0;
	m_bPlayGame = false;
	m_byGameStation = 0;
	m_isMasterNotPlay = false;
	InitDeskGameData();
}

void CGameDesk::InitDismissData()
{
	m_isDismissStatus = false;
	m_reqDismissTime = 0;
	m_reqDismissDeskStation = INVALID_DESKSTATION;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		m_deskUserInfoVec[i].dismissType = DISMISS_TYPE_DEFAULT;
	}
}

void CGameDesk::LoadPrivateDeskInfo(const PrivateDeskInfo& privateDeskInfo)
{
	m_masterID = privateDeskInfo.masterID;
	m_iVipGameCount = privateDeskInfo.buyGameCount;
	m_iBuyGameCount = privateDeskInfo.buyGameCount;
	m_isMasterNotPlay = privateDeskInfo.masterNotPlay == 0 ? false : true;
	strcpy(m_szDeskPassWord, privateDeskInfo.passwd);
	// 规则
	if (strcmp(privateDeskInfo.gameRules, REDIS_STR_DEFAULT))
	{
		memcpy(m_szGameRules, privateDeskInfo.gameRules, sizeof(m_szGameRules));
	}

	//俱乐部信息
	m_friendsGroupMsg.Init();
	m_friendsGroupMsg.friendsGroupID = privateDeskInfo.friendsGroupID;
	if (m_friendsGroupMsg.friendsGroupID != 0)
	{
		m_friendsGroupMsg.friendsGroupDeskNumber = privateDeskInfo.friendsGroupDeskNumber;
	}

	m_enable = true;
}

bool CGameDesk::PrivateSitDeskLogic(GameUserInfo* pUser)
{
	if (!pUser)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	// 只有旁观状态才能坐下
	if (pUser->playStatus != USER_STATUS_WATCH)
	{
		ERROR_LOG("user(%d) status is invalid(%d)", pUser->userID, pUser->playStatus);
		return false;
	}

	auto iter = m_watchUserInfoSet.begin();
	for (; iter != m_watchUserInfoSet.end(); ++iter)
	{
		const WatchUserInfo& info = *iter;
		if (info.userID == pUser->userID)
		{
			break;
		}
	}

	if (iter == m_watchUserInfoSet.end())
	{
		ERROR_LOG("can't find user(%d) in watchUserSet", pUser->userID);
		return false;
	}

	PrivateDeskInfo privateDeskInfo;
	int deskMixID = m_pDataManage->GetRoomID() * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;
	if (!pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		ERROR_LOG("GetPrivateDeskRecordInfo failed deskMixID:%d", deskMixID);
		return false;
	}

	//判断中途是否禁止加入
	if (m_bGameStopJoin == 1 && m_isBegin)
	{
		m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_STOP_JOIN);
		return true;
	}

	//游戏中禁止坐下
	if (m_bPlayGame)
	{
		m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_GAME_PLAYING_ERR_SIT);
		return true;
	}

	// 桌子是不是满了
	if (privateDeskInfo.currDeskUserCount >= privateDeskInfo.maxDeskUserCount)
	{
		m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_DESK_FULL);
		return true;
	}

	// 是否可以坐桌
	if (!WatchCanSit(pUser))
	{
		return true;
	}

	int roomType = GetRoomType();

	// 判断玩家俱乐部火币
	if (roomType == ROOM_TYPE_FG_VIP)
	{
		if (pUser->fireCoin < m_MinPoint)
		{
			m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_FRIENDSGROUP_FIRECOIN_LIMIT);
			return true;
		}
	}

	// 房间里面是不是有这个人了
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (pUser->userID == m_deskUserInfoVec[i].userID)
		{
			return true;
		}
	}

	BYTE deskStation = INVALID_DESKSTATION;
	int deskUserInfoVecSize = m_deskUserInfoVec.size();

	if (pUser->choiceDeskStation == INVALID_DESKSTATION) //系统分配座位
	{
		if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
		{
			for (int i = 0; i < deskUserInfoVecSize; i++)
			{
				DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
				if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == pUser->userID)
				{
					deskUserInfo.userID = pUser->userID;
					deskStation = (BYTE)i;
					deskUserInfo.leftRoomUser = 0;
					break;
				}
			}

			if (deskStation == INVALID_DESKSTATION)
			{
				for (int i = 0; i < deskUserInfoVecSize; i++)
				{
					DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
					if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == 0)
					{
						deskUserInfo.userID = pUser->userID;
						deskStation = (BYTE)i;
						break;
					}
				}
			}

			if (deskStation == INVALID_DESKSTATION)
			{
				m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_FRIEND_ROOM_DESK_FULL);
				ERROR_LOG("私人房 deskIdx=%d userID=%d m_deskUserInfoVec size=%d", m_deskIdx, pUser->userID, deskUserInfoVecSize);
				return true;
			}
		}
		else
		{
			for (int i = 0; i < deskUserInfoVecSize; i++)
			{
				DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
				if (deskUserInfo.userID == 0)
				{
					deskUserInfo.userID = pUser->userID;
					deskStation = (BYTE)i;
					break;
				}
			}

			if (deskStation == INVALID_DESKSTATION)
			{
				ERROR_LOG("calc user deskStation is invalid deskIdx=%d userID=%d m_deskUserInfoVec size=%d", m_deskIdx, pUser->userID, deskUserInfoVecSize);
				return false;
			}
		}
	}
	else // 玩家自己选择座位
	{
		if (pUser->choiceDeskStation >= deskUserInfoVecSize)
		{
			m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_DESKSTATION_NOTEXISTS);
			return true;
		}

		if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP) // 金币房
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[pUser->choiceDeskStation];
			if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == pUser->userID)
			{
				deskUserInfo.userID = pUser->userID;
				deskStation = pUser->choiceDeskStation;
				deskUserInfo.leftRoomUser = 0;
			}
			else if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == 0)
			{
				deskUserInfo.userID = pUser->userID;
				deskStation = pUser->choiceDeskStation;
			}
			else
			{
				m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_DESKSTATION_HAVEUSER);
				return true;
			}
		}
		else //积分房
		{
			if (m_deskUserInfoVec[pUser->choiceDeskStation].userID == 0)
			{
				m_deskUserInfoVec[pUser->choiceDeskStation].userID = pUser->userID;
				deskStation = pUser->choiceDeskStation;
			}
			else
			{
				m_pDataManage->SendData(pUser->userID, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_SIT, ERROR_DESKSTATION_HAVEUSER);
				return true;
			}
		}
	}

	// 旁观列表中移除玩家
	m_watchUserInfoSet.erase(iter);

	// 设置玩家的桌子号，座位号，状态等
	pUser->deskIdx = m_deskIdx;
	pUser->deskStation = deskStation;
	pUser->playStatus = USER_STATUS_SITING;
	pUser->choiceDeskStation = INVALID_DESKSTATION;
	pUser->lastOperateTime = time(NULL);
	m_deskUserInfoVec[deskStation].bNetCut = false;
	m_deskUserInfoVec[deskStation].lastWaitAgreeTime = time(NULL);
	m_deskUserInfoVec[deskStation].isVirtual = pUser->isVirtual;

	// 坐下人数和旁观人数
	int currDeskUserCount = privateDeskInfo.currDeskUserCount + 1;
	int currWatchUserCount = privateDeskInfo.currWatchUserCount - 1;
	pRedis->SetPrivateDeskCurrUserCount(deskMixID, currDeskUserCount);
	pRedis->SetPrivateDeskCurrWatchUserCount(deskMixID, currWatchUserCount);

	// 广播坐下的信息
	BroadcastUserSitData(pUser->deskStation);

	// 通知特殊游戏首次坐桌
	UserSitDeskActionNotify(pUser->deskStation);

	// 通知大厅开房信息发生变化
	NotifyLogonBuyDeskInfoChange(m_masterID, currDeskUserCount, pUser->userID, 0, deskMixID);

	if (currDeskUserCount >= privateDeskInfo.maxDeskUserCount)
	{
		// 桌子人数满了，通知旁观者
		SendWatchData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_SITFULL, 0);
	}

	return true;
}

bool CGameDesk::MoneySitDeskLogic(GameUserInfo* pUser)
{
	if (!pUser)
	{
		ERROR_LOG("MoneySitDeskLogic::pUser==NULL");
		return false;
	}

	int userID = pUser->userID;
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID: userID=%d", userID);
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	time_t currTime = time(NULL);

	if (pUser->deskStation == INVALID_DESKSTATION)
	{
		// 第一次坐下
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			if (m_deskUserInfoVec[i].userID == userID)
			{
				ERROR_LOG("user deskStation is invalid but user is in desk userID=%d", userID);
				pUser->deskStation = (BYTE)i;
				break;
			}
		}

		if (pUser->deskStation == INVALID_DESKSTATION)
		{
			// 分配一个座位
			for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
			{
				DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
				if (deskUserInfo.userID == 0)
				{
					deskUserInfo.userID = userID;
					pUser->deskStation = (BYTE)i;
					break;
				}
			}
		}
	}

	// 这里玩家不可能没有座位了
	if (pUser->deskStation >= m_deskUserInfoVec.size())
	{
		ERROR_LOG("user deskStation is invalid userID = %d deskStation = %d", userID, pUser->deskStation);
		return false;
	}

	if (!IsEnable())
	{
		m_enable = true;
		m_masterID = 0;
		m_iVipGameCount = 0;
		m_iBuyGameCount = 0;

		memset(m_szDeskPassWord, 0, sizeof(m_szDeskPassWord));
		memset(m_szGameRules, 0, sizeof(m_szGameRules));
		memset(m_uScore, 0, sizeof(m_uScore));
	}

	pRedis->SetUserRoomID(pUser->userID, m_pDataManage->GetRoomID());

	// 设置玩家的桌子号，座位号，状态等
	pUser->lastOperateTime = currTime;
	pUser->deskIdx = m_deskIdx;
	if (USER_STATUS_PLAYING != pUser->playStatus)
	{
		pUser->playStatus = USER_STATUS_SITING;
	}

	m_deskUserInfoVec[pUser->deskStation].bNetCut = false;
	m_deskUserInfoVec[pUser->deskStation].lastWaitAgreeTime = currTime;
	m_deskUserInfoVec[pUser->deskStation].isVirtual = pUser->isVirtual;

	if (pUser->isVirtual == 1)
	{
		RobotPositionInfo positionInfo;
		int iRobotIndex = pRedis->GetRobotInfoIndex();
		ConfigManage()->GetRobotPositionInfo(iRobotIndex, positionInfo);
		strcpy(pUser->name, positionInfo.name.c_str());
		strcpy(pUser->headURL, positionInfo.headUrl.c_str());
		pUser->sex = positionInfo.sex;
		strcpy(pUser->longitude, positionInfo.longitude.c_str());
		strcpy(pUser->latitude, positionInfo.latitude.c_str());
		strcpy(pUser->address, positionInfo.address.c_str());
		strcpy(pUser->ip, positionInfo.ip.c_str());

		//修改redis中机器人的数据
		std::unordered_map<std::string, std::string> umap;
		umap["name"] = pUser->name;
		umap["headURL"] = pUser->headURL;
		umap["sex"] = CUtil::Tostring(pUser->sex);
		umap["logonIP"] = pUser->ip;
		umap["address"] = pUser->address;
		umap["Lng"] = pUser->longitude;
		umap["Lat"] = pUser->latitude;
		pRedis->hmset(TBL_USER, pUser->userID, umap);

		//既然换名字，金币数量也换
		//回收机器人金币数量
		long long _i64PoolMoney = 0;
		long long _i64MoneyChange = 0;

		RoomBaseInfo roomBasekInfo;
		RoomBaseInfo* pRoomBaseInfo = NULL;
		if (pRedis->GetRoomBaseInfo(m_pDataManage->GetRoomID(), roomBasekInfo))
		{
			pRoomBaseInfo = &roomBasekInfo;
		}
		else
		{
			pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_pDataManage->GetRoomID());
		}
		if (pRoomBaseInfo)
		{
			if (pRoomBaseInfo->minPoint <= pRoomBaseInfo->basePoint)
			{
				_i64MoneyChange = CUtil::GetRandRange(1000 * pRoomBaseInfo->basePoint, 40000 * pRoomBaseInfo->basePoint);
			}
			else if (pRoomBaseInfo->maxPoint <= 0 || pRoomBaseInfo->maxPoint <= pRoomBaseInfo->minPoint)
			{
				if (pRoomBaseInfo->minPoint < 1000)
				{
					_i64MoneyChange = CUtil::GetRandRange(20 * pRoomBaseInfo->minPoint, 100 * pRoomBaseInfo->minPoint);
				}
				else
				{
					_i64MoneyChange = CUtil::GetRandRange(2 * pRoomBaseInfo->minPoint, 10 * pRoomBaseInfo->minPoint);
				}

			}
			else
			{
				_i64MoneyChange = CUtil::GetRandRange(pRoomBaseInfo->minPoint, pRoomBaseInfo->maxPoint);
			}

			//如果后台设置了金币范围直接按照后台来
			int iMinRobotHaveMoney_ = m_pDataManage->GetPoolConfigInfo("minM");
			int iMaxRobotHaveMoney_ = m_pDataManage->GetPoolConfigInfo("maxM");
			if (iMaxRobotHaveMoney_ > iMinRobotHaveMoney_)
			{
				_i64MoneyChange = CUtil::GetRandRange(iMinRobotHaveMoney_, iMaxRobotHaveMoney_);
			}

			_i64PoolMoney = pUser->money - _i64MoneyChange;
			//pRedis->SetRoomPoolMoney(m_pDataManage->GetRoomID(), _i64PoolMoney, true);
			pRedis->SetUserMoney(userID, _i64MoneyChange);
			pUser->money = _i64MoneyChange;
		}
	}

	// 通知等待剩余时间
	SendLeftWaitAgreeData(pUser->deskStation);

	// 广播坐下的信息（不广播自己）
	BroadcastUserSitData(pUser->deskStation);

	// 通知特殊游戏首次坐桌
	UserSitDeskActionNotify(pUser->deskStation);

	return true;
}

bool CGameDesk::PrivateUserLeftDeskLogic(GameUserInfo* pUser)
{
	if (!pUser)
	{
		ERROR_LOG("invalid pUser = null");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("GetRedis failed");
		return false;
	}

	int deskMixID = m_pDataManage->GetRoomID() * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;
	PrivateDeskInfo deskInfo;
	if (!pRedis->GetPrivateDeskRecordInfo(deskMixID, deskInfo))
	{
		ERROR_LOG("GetPrivateDeskRecordInfo failed deskMixID=%d", deskMixID);
		return false;
	}

	if (pUser->playStatus == USER_STATUS_WATCH)
	{
		// 旁观者
		return OnWatcherLeftDesk(pUser, deskInfo);
	}
	else
	{
		return OnDeskUserLeftDesk(pUser, deskInfo);
	}
}

bool CGameDesk::MoneyUserLeftDeskLogic(GameUserInfo * pUser)
{
	if (!pUser)
	{
		ERROR_LOG("MoneyUserLeftDeskLogic::pUser is NULL");
		return false;
	}

	if (pUser->deskStation >= (BYTE)m_deskUserInfoVec.size())
	{
		ERROR_LOG("user(%d) deskStation is invalid deskStation=%d", pUser->userID, pUser->deskStation);
		return false;
	}

	// 先通知桌子中的所有玩家
	BroadcastUserLeftData(pUser->deskStation, REASON_KICKOUT_STAND);

	// 清理桌子中玩家的数据
	m_deskUserInfoVec[pUser->deskStation].clear();

	// 清理玩家游戏数据
	pUser->playStatus = USER_STATUS_STAND;
	pUser->deskIdx = INVALID_DESKIDX;
	pUser->deskStation = INVALID_DESKSTATION;

	// 玩家离开可能会触发游戏开始
	if (m_bPlayGame == false && CanBeginGame())
	{
		GameBegin(0);
	}

	return true;
}

bool CGameDesk::HundredGameUserLeftLogic(GameUserInfo* pUser)
{
	if (!pUser)
	{
		ERROR_LOG("pUser is NULL");
		return false;
	}

	if (pUser->deskStation >= (BYTE)m_deskUserInfoVec.size())
	{
		ERROR_LOG("user(%d) deskStation is invalid deskStation=%d", pUser->userID, pUser->deskStation);
		return false;
	}

	// 先通知桌子中的所有玩家
	BroadcastUserLeftData(pUser->deskStation, REASON_KICKOUT_STAND);

	// 清理桌子中玩家的数据
	m_deskUserInfoVec[pUser->deskStation].clear();

	// 清理玩家游戏数据
	pUser->playStatus = USER_STATUS_STAND;
	pUser->deskIdx = INVALID_DESKIDX;
	pUser->deskStation = INVALID_DESKSTATION;

	return true;
}

bool CGameDesk::HundredGameUserNetCutLogic(GameUserInfo * pUser)
{
	if (!pUser)
	{
		return false;
	}

	return true;
}

int CGameDesk::CheckUserMoney(int userID)
{
	int reason = REASON_KICKOUT_DEFAULT;
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID = 0");
		return REASON_KICKOUT_DEFAULT;
	}

	if (!m_pDataManage)
	{
		return REASON_KICKOUT_DEFAULT;
	}

	CRedisLoader * pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return REASON_KICKOUT_DEFAULT;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return REASON_KICKOUT_DEFAULT;
	}

	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (pRedis->GetRoomBaseInfo(m_pDataManage->GetRoomID(), roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_pDataManage->GetRoomID());
	}
	if (!pRoomBaseInfo)
	{
		return REASON_KICKOUT_DEFAULT;
	}

	if (pRoomBaseInfo->minPoint > 0 && pUser->money < pRoomBaseInfo->minPoint)
	{
		return REASON_KICKOUT_STAND_MINLIMIT;
	}

	if (pRoomBaseInfo->minPoint > 0 && pRoomBaseInfo->maxPoint > pRoomBaseInfo->minPoint && pUser->money > pRoomBaseInfo->maxPoint)
	{
		return REASON_KICKOUT_STAND_MAXLIMIT;
	}

	if (pUser->money < 0)
	{
		return REASON_KICKOUT_STAND_MINLIMIT;
	}

	return REASON_KICKOUT_DEFAULT;
}

void CGameDesk::ProcessDeduceMoneyWhenGameBegin()
{
	if (GetRoomSort() == ROOM_SORT_HUNDRED || GetRoomSort() == ROOM_SORT_SCENE)
	{
		return;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("GetRedis failed");
		return;
	}

	int roomID = m_pDataManage->GetRoomID();
	int roomType = GetRoomType();
	long long costMoney = 0;

	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (pRedis->GetRoomBaseInfo(roomID, roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
	}
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("GetRoomBaseInfo failed roomID(%d)", roomID);
		return;
	}

	// 金币场开局扣除玩家金币
	if (roomType == ROOM_TYPE_GOLD)
	{
		costMoney = pRoomBaseInfo->gameBeginCostMoney;
	}
	else if (roomType == ROOM_TYPE_FRIEND && m_payType == PAY_TYPE_AA)
	{
		BuyGameDeskInfo buyGameDeskInfo;
		BuyGameDeskInfo* pBuyGameDeskInfo = NULL;
		BuyGameDeskInfoKey buyDekKey(pRoomBaseInfo->gameID, m_iBuyGameCount, pRoomBaseInfo->type);
		if (pRedis->GetBuyGameDeskInfo(buyDekKey, buyGameDeskInfo))
		{
			pBuyGameDeskInfo = &buyGameDeskInfo;
		}
		else
		{
			pBuyGameDeskInfo = ConfigManage()->GetBuyGameDeskInfo(buyDekKey);
		}
		if (!pBuyGameDeskInfo)
		{
			return;
		}
		costMoney = pBuyGameDeskInfo->AAcostNums;
	}

	//不扣金币
	if (costMoney <= 0)
	{
		return;
	}

	long long percentageWinMoney = 0, recovMoney = 0;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		if (pUser->isVirtual)
		{
			recovMoney += costMoney;
		}
		percentageWinMoney += costMoney;

		long long newMoney = pUser->money - costMoney;
		pUser->money = newMoney;
		pRedis->SetUserMoneyEx(userID, -costMoney, true, RESOURCE_CHANGE_REASON_GAME_BEGIN, roomID, 0, pUser->isVirtual, m_friendsGroupMsg.friendsGroupID);

		// 通知变化
		NotifyUserResourceChange(userID, RESOURCE_TYPE_MONEY, newMoney, -costMoney);
	}

	//回收金币到奖池
	pRedis->SetRoomPercentageWinMoney(roomID, percentageWinMoney - recovMoney, true);
	//pRedis->SetRoomPoolMoney(roomID, recovMoney, true);
}

void CGameDesk::NotifyLogonBuyDeskInfoChange(int masterID, int userCount, int userID, BYTE updateType, int deskMixID)
{
	PlatformDeskPeopleCountChange msg;

	msg.friendsGroupID = m_friendsGroupMsg.friendsGroupID;
	msg.friendsGroupDeskNumber = m_friendsGroupMsg.friendsGroupDeskNumber;
	msg.masterID = masterID;
	memcpy(msg.roomPasswd, m_szDeskPassWord, sizeof(msg.roomPasswd));
	msg.currUserCount = userCount;
	msg.updateType = updateType;
	msg.userID = userID;

	m_pDataManage->SendMessageToCenterServer(CENTER_MESSAGE_LOADER_BUYDESKINFO_CHANGE, &msg, sizeof(msg));

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("GetRedis failed");
		return;
	}

	pRedis->SetPrivateDeskUserID(deskMixID, userID, updateType);
}

void CGameDesk::NotifyLogonDeskDissmiss(const PrivateDeskInfo& privateDeskInfo)
{
	PlatformDeskDismiss msg;

	msg.masterID = privateDeskInfo.masterID;
	msg.friendsGroupID = privateDeskInfo.friendsGroupID;
	msg.friendsGroupDeskNumber = privateDeskInfo.friendsGroupDeskNumber;
	memcpy(msg.passwd, privateDeskInfo.passwd, sizeof(msg.passwd));
	msg.bDeleteDesk = m_friendsGroupMsg.bDeleteDesk;
	msg.roomType = GetRoomType();
	msg.gameID = m_pDataManage->m_uNameID;
	msg.maxCount = privateDeskInfo.buyGameCount;
	memcpy(msg.gameRules, privateDeskInfo.gameRules, sizeof(msg.gameRules));

	m_pDataManage->SendMessageToCenterServer(CENTER_MESSAGE_LOADER_DESK_DISSMISS, &msg, sizeof(msg));

	if (m_friendsGroupMsg.friendsGroupID <= 0)
	{
		return;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("GetRedis failed");
		return;
	}

	// 判断是否需要删除redis牌桌数据
	if (m_friendsGroupMsg.bDeleteDesk)
	{
		// 删除redis中的牌桌
		pRedis->DelFGDeskRoom(privateDeskInfo.friendsGroupID, privateDeskInfo.friendsGroupDeskNumber);
	}
}

void CGameDesk::NotifyLogonDeskStatusChange()
{
	PlatformDeskGameCountChange msg;

	msg.friendsGroupID = m_friendsGroupMsg.friendsGroupID;
	msg.friendsGroupDeskNumber = m_friendsGroupMsg.friendsGroupDeskNumber;
	msg.gameStatus = m_isBegin;

	m_pDataManage->SendMessageToCenterServer(CENTER_MESSAGE_LOADER_DESK_STATUS_CHANGE, &msg, sizeof(msg));
}

BuyGameDeskInfo CGameDesk::ProcessCostJewels()
{
	BuyGameDeskInfo buyGameDeskInfo;

	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD || roomType == ROOM_TYPE_MATCH)
	{
		return buyGameDeskInfo;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("GetRedis failed");
		return buyGameDeskInfo;
	}

	int roomID = m_pDataManage->GetRoomID();
	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (pRedis->GetRoomBaseInfo(roomID, roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
	}
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("GetRoomBaseInfo failed roomID(%d)", roomID);
		return buyGameDeskInfo;
	}

	int deskMixID = roomID * MAX_ROOM_HAVE_DESK_COUNT + m_deskIdx;
	PrivateDeskInfo privateDeskInfo;
	if (!pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		ERROR_LOG("GetPrivateDeskRecordInfo failed, deskMixID(%d)", deskMixID);
		return buyGameDeskInfo;
	}

	BuyGameDeskInfo* pBuyGameDeskInfo = NULL;
	BuyGameDeskInfoKey buyDekKey(pRoomBaseInfo->gameID, privateDeskInfo.buyGameCount, GetRoomType());
	if (pRedis->GetBuyGameDeskInfo(buyDekKey, buyGameDeskInfo))
	{
		pBuyGameDeskInfo = &buyGameDeskInfo;
	}
	else
	{
		pBuyGameDeskInfo = ConfigManage()->GetBuyGameDeskInfo(buyDekKey);
	}
	if (!pBuyGameDeskInfo)
	{
		ERROR_LOG("GetAABuyGameDeskInfo failed gameID(%d) gameCount(%d)", pRoomBaseInfo->gameID, privateDeskInfo.buyGameCount);
		return buyGameDeskInfo;
	}
	buyGameDeskInfo = *pBuyGameDeskInfo;

	//退还金币或者房卡
	if (m_finishedGameCount <= 0)
	{
		if (privateDeskInfo.payType == PAY_TYPE_NORMAL && pBuyGameDeskInfo->costNums > 0)		// 普通支付
		{
			int masterID = privateDeskInfo.masterID;
			UserData masterData;
			if (!pRedis->GetUserData(masterID, masterData))
			{
				ERROR_LOG("返还金币或者房卡失败:masterID=%d", masterID);
				return buyGameDeskInfo;
			}

			if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL)
			{
				long long newJewels = masterData.jewels + pBuyGameDeskInfo->costNums;
				pRedis->SetUserJewelsEx(masterID, pBuyGameDeskInfo->costNums, true, RESOURCE_CHANGE_REASON_GAME_SELLETE_ROLLBACK, roomID, 0, masterData.isVirtual, m_friendsGroupMsg.friendsGroupID);

				// 通知房卡变化
				m_pDataManage->SendResourcesChangeToLogonServer(masterID, RESOURCE_TYPE_JEWEL, newJewels, RESOURCE_CHANGE_REASON_GAME_SELLETE_ROLLBACK, pBuyGameDeskInfo->costNums);
			}
			else
			{
				long long newGolds = masterData.money + pBuyGameDeskInfo->costNums;
				pRedis->SetUserMoneyEx(masterID, pBuyGameDeskInfo->costNums, true, RESOURCE_CHANGE_REASON_GAME_SELLETE_ROLLBACK, roomID, 0, 0, m_friendsGroupMsg.friendsGroupID);
				//算房卡场系统赢钱
				pRedis->SetRoomGameWinMoney(roomID, -pBuyGameDeskInfo->costNums, true);

				// 通知金币变化
				m_pDataManage->SendResourcesChangeToLogonServer(masterID, RESOURCE_TYPE_MONEY, newGolds, RESOURCE_CHANGE_REASON_GAME_SELLETE_ROLLBACK, pBuyGameDeskInfo->costNums);
			}
		}
		return buyGameDeskInfo;
	}

	if (privateDeskInfo.payType != PAY_TYPE_NORMAL && pRoomBaseInfo->type != ROOM_TYPE_FRIEND)		// AA支付，金币房暂时没有AA支付，是开局消耗
	{
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			int userID = m_deskUserInfoVec[i].userID;
			if (userID <= 0)
			{
				continue;
			}

			if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL)
			{
				pRedis->SetUserJewelsEx(userID, -pBuyGameDeskInfo->AAcostNums, true, RESOURCE_CHANGE_REASON_GAME_SELLETE_AA, roomID, 0, m_deskUserInfoVec[i].isVirtual, m_friendsGroupMsg.friendsGroupID);
			}
			else
			{
				pRedis->SetUserMoneyEx(userID, -pBuyGameDeskInfo->AAcostNums, true, RESOURCE_CHANGE_REASON_GAME_SELLETE_AA, roomID, 0, 0, m_friendsGroupMsg.friendsGroupID);
			}
		}
	}

	return buyGameDeskInfo;
}

bool CGameDesk::ProcessUserWatch(GameUserInfo* pUser, const PrivateDeskInfo& privateDeskInfo)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	int userID = pUser->userID;

	if ((int)m_watchUserInfoSet.size() >= privateDeskInfo.maxWatchUserCount)
	{
		ERROR_LOG("watch user is too much currUser(%d) maxWatchUser(%d)", m_watchUserInfoSet.size(), privateDeskInfo.maxDeskUserCount);
		return false;
	}

	auto iter = m_watchUserInfoSet.begin();
	for (; iter != m_watchUserInfoSet.end(); iter++)
	{
		const WatchUserInfo& info = *iter;
		if (info.userID == userID)
		{
			ERROR_LOG("user is already watch user(%d)", userID);
			return false;
		}
	}

	BYTE deskStation = AllocWatcherDeskStation();
	if (deskStation == INVALID_DESKSTATION)
	{
		ERROR_LOG("AllocWatcherDeskStation failed");
		return false;
	}

	int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);
	pRedis->SetPrivateDeskCurrWatchUserCount(deskMixID, privateDeskInfo.currWatchUserCount + 1);

	pUser->playStatus = USER_STATUS_WATCH;
	pUser->deskIdx = m_deskIdx;
	pUser->deskStation = deskStation;

	WatchUserInfo watchUserInfo;
	watchUserInfo.userID = userID;
	watchUserInfo.deskStation = deskStation;
	m_watchUserInfoSet.insert(watchUserInfo);

	return true;
}

bool CGameDesk::ProcessPrivateUserSitDesk(GameUserInfo* pUser, const PrivateDeskInfo& privateDeskInfo)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	// 房间里面是不是有这个人了
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (pUser->userID == m_deskUserInfoVec[i].userID)
		{
			ERROR_LOG("用户已经存在 userID = %d", pUser->userID);
			return true;
		}
	}

	BYTE deskStation = AllocDeskStation(pUser->userID);
	if (deskStation == INVALID_DESKSTATION)
	{
		ERROR_LOG("AllocDeskStation 分配座位号失败,userID:%d", pUser->userID);
		return false;
	}

	m_deskUserInfoVec[deskStation].userID = pUser->userID;
	m_deskUserInfoVec[deskStation].deskStation = deskStation;
	m_deskUserInfoVec[deskStation].leftRoomUser = 0;
	m_deskUserInfoVec[deskStation].bNetCut = false;
	m_deskUserInfoVec[deskStation].lastWaitAgreeTime = time(NULL);
	m_deskUserInfoVec[deskStation].isVirtual = pUser->isVirtual;

	pUser->playStatus = USER_STATUS_SITING;
	pUser->deskIdx = m_deskIdx;
	pUser->deskStation = deskStation;
	pUser->lastOperateTime = time(NULL);

	// 坐下人数
	int currDeskUserCount = privateDeskInfo.currDeskUserCount + 1;
	int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);
	pRedis->SetPrivateDeskCurrUserCount(deskMixID, currDeskUserCount);

	// 广播坐下的信息
	BroadcastUserSitData(pUser->deskStation);

	// 通知特殊游戏首次坐桌
	UserSitDeskActionNotify(pUser->deskStation);

	// 通知大厅开房信息发生变化
	NotifyLogonBuyDeskInfoChange(m_masterID, currDeskUserCount, pUser->userID, 0, deskMixID);

	return true;
}

BYTE CGameDesk::AllocDeskStation(int userID)
{
	int deskUserInfoVecSize = m_deskUserInfoVec.size();
	if (GetRoomType() == ROOM_TYPE_FRIEND || GetRoomType() == ROOM_TYPE_FG_VIP)
	{
		for (int i = 0; i < deskUserInfoVecSize; i++)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
			if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == userID)
			{
				return (BYTE)i;
			}
		}

		for (int i = 0; i < deskUserInfoVecSize; ++i)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
			if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == 0)
			{
				return (BYTE)i;
			}
		}
	}
	else
	{
		for (int i = 0; i < deskUserInfoVecSize; ++i)
		{
			if (m_deskUserInfoVec[i].userID == 0)
			{
				return (BYTE)i;
			}
		}
	}

	return INVALID_DESKSTATION;
}

bool CGameDesk::OnPrivateUserLogin(int userID, const PrivateDeskInfo& info)
{
	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	int roomType = GetRoomType();

	// 从缓存中加载桌子数据
	if (!IsEnable())
	{
		LoadPrivateDeskInfo(info);
		memset(m_uScore, 0, sizeof(m_uScore));
		if (m_pDataManage->IsMultiPeopleGame())
		{
			Json::Reader reader;
			Json::Value value;
			if (reader.parse(info.gameRules, value))
			{
				m_iConfigCount = value["rs"].asInt();
				m_deskUserInfoVec.resize(m_iConfigCount);
			}
		}

		// 自动开始模式
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(info.gameRules, value))
		{
			m_autoBeginMode = value["Start"].asInt();		//开始方式：0：手动开始，非0满几人开始
			m_bGameStopJoin = value["Stopjoin"].asBool();	//中途禁止加入
			m_MinPoint = value["mPoint"].asInt();			//入场限制
			m_RemoveMinPoint = value["lPoint"].asInt();		//踢人限制
			m_basePoint = value["bPoint"].asInt();			//底注
			m_roomTipType = value["cCSFS"].asInt();			//抽水方式
			m_roomTipTypeNums = value["cCSL"].asInt();		//抽水数量
			m_payType = value["pay"].asInt();				//支付方式：1：普通支付，2：AA支付，3：大赢家支付
			m_playMode = value["gameIdx"].asInt();			//游戏玩法
		}

		if (m_autoBeginMode <= 1)
		{
			m_autoBeginMode = 0;
		}

		if (roomType == ROOM_TYPE_FRIEND) //金币房
		{
			if (m_RemoveMinPoint < 1)
			{
				m_RemoveMinPoint = 1;
			}
		}

		if (m_roomTipTypeNums < 0 || m_roomTipTypeNums > 99)
		{
			m_roomTipTypeNums = 3;
		}

		if (30000007 == m_pDataManage->m_uNameID)
		{
			for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
			{
				m_uScore[i] = value["DRJF"].asInt();
			}
		}
	}

	// 得到玩家俱乐部火币
	if (roomType == ROOM_TYPE_FG_VIP)
	{
		CRedisPHP* pRedisPHP = m_pDataManage->GetRedisPHP();
		if (!pRedisPHP)
		{
			ERROR_LOG("pRedisPHP initialized failed");
			return false;
		}

		long long carryFireCoin = 0;
		if (!pRedisPHP->GetUserFriendsGroupMoney(m_friendsGroupMsg.friendsGroupID, pUser->userID, carryFireCoin))
		{
			ERROR_LOG("获取玩家俱乐部火币失败：userId=%d,friendsGroupID=%d", pUser->userID, m_friendsGroupMsg.friendsGroupID);
		}

		pUser->fireCoin = (int)carryFireCoin;
	}

#ifdef OFFLINE_CHANGE_AGREE_STATUS  //断线取消准备状态
	if (pUser->playStatus == USER_STATUS_AGREE)
	{
		pUser->playStatus = USER_STATUS_SITING;
		return true;
	}

	// 已经坐下或者游戏中的玩家
	if (pUser->playStatus == USER_STATUS_SITING || pUser->playStatus == USER_STATUS_PLAYING)
	{
		// 断线重连广播坐下，不广播自己
		BroadcastUserSitDataExceptMyself(pUser->deskStation);
		return true;
	}
#else  //断线不取消准备状态
	// 已经坐下或者游戏中的玩家
	if (pUser->playStatus == USER_STATUS_SITING || pUser->playStatus == USER_STATUS_PLAYING || pUser->playStatus == USER_STATUS_AGREE)
	{
		// 断线重连广播坐下，不广播自己
		BroadcastUserSitDataExceptMyself(pUser->deskStation);
		return true;
	}
#endif

	if (pUser->playStatus != USER_STATUS_DEFAULT)
	{
		return true;
	}

	bool canWatch = m_pDataManage->IsCanWatch();
	if (canWatch == true)
	{
		// 可旁观并且玩家为默认状态
		return ProcessUserWatch(pUser, info);
	}
	else
	{
		// 不可旁观立即坐下
		return ProcessPrivateUserSitDesk(pUser, info);
	}
}

int CGameDesk::GetAgreeUserCount()
{
	int count = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID < 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (pUser && pUser->playStatus == USER_STATUS_AGREE)
		{
			count++;
		}
	}

	return count;
}

int CGameDesk::GetUserCount()
{
	int count = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID > 0)
		{
			count++;
		}
	}

	return count;
}

int CGameDesk::GetOfflineUserCount()
{
	int count = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID(%d)", userID);
			continue;
		}

		if (pUser->IsOnline == false)
		{
			count++;
		}
	}

	return count;
}

bool CGameDesk::IsAllUserOffline()
{
	int offCount = 0, allCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID(%d)", userID);
			continue;
		}

		allCount++;
		if (pUser->IsOnline == false)
		{
			offCount++;
		}
		else
		{
			return false;
		}
	}

	return allCount <= 0 ? false : offCount >= allCount;
}

bool CGameDesk::IsCanSitDesk()
{
	bool bCanSit = false;

	if (GetRoomSort() == ROOM_SORT_NORMAL)
	{
		if (m_bPlayGame)
		{
			return bCanSit;
		}
	}

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID <= 0)
		{
			bCanSit = true;
			break;
		}
	}

	return bCanSit;
}

int CGameDesk::AgreePeople()
{
	int iAgree = 0;
	if (m_bPlayGame)
	{
		return iAgree;
	}

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID > 0)
		{
			GameUserInfo *pUser = m_pDataManage->GetUser(userID);
			if (pUser && USER_STATUS_AGREE == pUser->playStatus)
			{
				iAgree++;
			}
		}
	}

	return iAgree;
}

void CGameDesk::CheckTimeoutNotAgreeUser(time_t currTime)
{
	if (m_bPlayGame)
	{
		// 游戏开始了则不管
		return;
	}

	bool bIsKickedUser = false;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];

		int userID = deskUserInfo.userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			deskUserInfo.clear();
			continue;
		}

		bool bKickOut = false;
		int kickReason = REASON_KICKOUT_STAND;

		if (pUser->deskStation != i)
		{
			// 走到这里数据已经异常了
			ERROR_LOG("user deskStation is not match userID:%d deskStation=%d i=%d", pUser->userID, pUser->deskStation, i);
			bKickOut = true;
		}
		else if (pUser->playStatus == USER_STATUS_SITING)
		{
			// 玩家超时没准备
			if (currTime - deskUserInfo.lastWaitAgreeTime > GOLD_DESK_TIMEOUT_UNAGREE_KICKOUT_SECS)
			{
				bKickOut = true;
				kickReason = REASON_KICKOUT_NOTAGREE;
			}
		}

		if (bKickOut)
		{
			//玩家被踢
			UserBeKicked((BYTE)i);

			if (!BroadcastUserLeftData((BYTE)i, kickReason))
			{
				ERROR_LOG("send user left fail.userID=%d", pUser->userID);
			}

			deskUserInfo.clear();

			m_pDataManage->DelUser(userID);

			bIsKickedUser = true;
		}
	}

	// 玩家离开可能会触发游戏开始
	if (bIsKickedUser && m_bPlayGame == false && CanBeginGame())
	{
		GameBegin(0);
	}
}

void CGameDesk::CheckTimeoutNotOperateUser(time_t currTime)
{
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];

		int userID = deskUserInfo.userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			deskUserInfo.clear();
			continue;
		}

		if (pUser->lastOperateTime > 0 && !IsPlayGame((BYTE)i) && HundredGameIsInfoChange((BYTE)i)
			&& currTime - pUser->lastOperateTime > HUUNDRED_GAME_TIMEOUT_OPERATE_KICKOUT_SECS)
		{
			//玩家被踢
			UserBeKicked((BYTE)i);

			BroadcastUserLeftData((BYTE)i, REASON_KICKOUT_LONG_TIME_NOOPERATION);

			deskUserInfo.clear();

			if (!pUser->isVirtual)
			{
				m_pDataManage->DelUser(userID);
			}
		}
	}
}

bool CGameDesk::IsAuto(BYTE deskStation)
{
	if (deskStation >= m_deskUserInfoVec.size())
	{
		return false;
	}

	return m_deskUserInfoVec[deskStation].isAuto;
}

bool CGameDesk::IsNetCut(BYTE deskStation)
{
	if (deskStation >= m_deskUserInfoVec.size())
	{
		return false;
	}

	return m_deskUserInfoVec[deskStation].bNetCut;
}

bool CGameDesk::IsVirtual(BYTE deskStation)
{
	if (deskStation >= m_deskUserInfoVec.size())
	{
		return false;
	}

	return m_deskUserInfoVec[deskStation].isVirtual == 0 ? false : true;
}

bool CGameDesk::IsSuperUser(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	if (!m_pDataManage)
	{
		return false;
	}

	GameUserInfo * pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		return false;
	}

	if ((pUser->userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER)
	{
		return true;
	}

	return false;
}

bool CGameDesk::IsVisitorUser(BYTE deskStation)
{
	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return false;
	}

	return m_pDataManage->IsVisitorUser(userID);
}

//////////////////////////////////////////////////////////////////////////
bool CGameDesk::HundredGameBegin()
{
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
		if (deskUserInfo.userID > 0)
		{
			m_pDataManage->SetUserPlayStatus(deskUserInfo.userID, USER_STATUS_PLAYING);
		}
	}

	//记录游戏开始需要的数据
	m_beginTime = time(NULL);
	m_bPlayGame = true;

	// 局数
	m_iRunGameCount++;

	// 告诉桌子的玩家游戏开始了
	BroadcastDeskData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GAMEBEGIN);

	return true;
}

bool CGameDesk::HundredGameFinish()
{
	m_bPlayGame = false;

	// 让所有掉线的玩家离开
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];

		int userID = deskUserInfo.userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID=%d", userID);
			continue;
		}

		bool bKickOut = false;
		bool bDelUser = false;
		int kickReason = REASON_KICKOUT_STAND;

		if (pUser->deskStation != i)
		{
			// 走到这里数据已经异常了
			ERROR_LOG("user deskStation is not match userID:%d deskStation=%d i=%d", pUser->userID, pUser->deskStation, i);
			bKickOut = true;
			bDelUser = true;
		}
		else if (!pUser->IsOnline)
		{
			bKickOut = true;
			bDelUser = true;
		}
		else if (pUser->isVirtual && IsKickOutVirtual((BYTE)i)) // 一部分机器人出去
		{
			bKickOut = true;
			bDelUser = false;
		}
		else
		{
			pUser->playStatus = USER_STATUS_SITING;
			kickReason = CheckUserMoney(userID);
			if (kickReason != REASON_KICKOUT_DEFAULT)
			{
				bKickOut = true;
			}
		}

		if (bKickOut)
		{
			// 玩家被踢
			UserBeKicked((BYTE)i);

			// 通知所有玩家
			BroadcastUserLeftData((BYTE)i, kickReason);

			// 清理桌子玩家数据
			deskUserInfo.clear();

			// 设置玩家为站起状态
			pUser->playStatus = USER_STATUS_STAND;
			pUser->deskIdx = INVALID_DESKIDX;
			pUser->deskStation = INVALID_DESKSTATION;

			if (bDelUser)
			{
				// 内存和缓存中移除玩家数据
				m_pDataManage->DelUser(userID);
			}
		}
	}

	// 加载配置
	if (m_needLoadConfig)
	{
		m_needLoadConfig = false;
		LoadDynamicConfig();
	}

	return true;
}

int CGameDesk::GetRealPeople()
{
	int iReal = 0;
	for (int i = 0; i < (int)m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		if (!m_deskUserInfoVec[i].isVirtual)
		{
			iReal++;
		}
	}
	return iReal;
}

UINT CGameDesk::GetRobotPeople()
{
	UINT iCount = 0;
	for (int i = 0; i < (int)m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		if (m_deskUserInfoVec[i].isVirtual)
		{
			iCount++;
		}
	}
	return iCount;
}

int CGameDesk::GetBasePoint()
{
	if (!m_pDataManage)
	{
		return 1;
	}

	CRedisLoader *pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return 1;
	}

	int roomID = m_pDataManage->GetRoomID();
	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (pRedis->GetRoomBaseInfo(roomID, roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
	}
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("GetRoomBaseInfo failed roomID=%d", roomID);
		return 1;
	}

	if (pRoomBaseInfo->type == ROOM_TYPE_PRIVATE || pRoomBaseInfo->type == ROOM_TYPE_MATCH)
	{
		return 1;
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FRIEND || pRoomBaseInfo->type == ROOM_TYPE_FG_VIP)
	{
		return m_basePoint > 0 ? m_basePoint : 1;
	}

	return pRoomBaseInfo->basePoint;
}

void CGameDesk::NotifyUserResourceChange(int userID, int resourceType, long long value, long long changeValue)
{
	LoaderNotifyResourceChange msg;
	msg.userID = userID;
	msg.resourceType = resourceType;
	msg.value = value;
	msg.changeValue = changeValue;

	if (GetRoomSort() == ROOM_SORT_NORMAL || GetRoomSort() == ROOM_SORT_SCENE)
	{
		BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_RESOURCE_CHANGE, 0, false);
	}
	else
	{
		m_pDataManage->SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOADER_RESOURCE_CHANGE, 0, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CGameDesk::SetServerRoomPoolData(const char * fieldName, long long fieldValue, bool bAdd)
{
	if (!fieldName)
	{
		return false;
	}

	int roomID = m_pDataManage->GetRoomID();
	if (roomID <= 0)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	return pRedis->SetRoomPoolData(roomID, fieldName, fieldValue, bAdd);
}

bool CGameDesk::SetServerRoomPoolData(const char * fieldName, const char * fieldValue)
{
	if (!fieldName)
	{
		return false;
	}

	int roomID = m_pDataManage->GetRoomID();
	if (roomID <= 0)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	std::unordered_map<std::string, std::string> umap;

	umap[fieldName] = fieldValue;

	if (!pRedis->hmset(TBL_REWARDS_POOL, roomID, umap, REDIS_EXTEND_MODE_UPDATE))
	{
		ERROR_LOG("设置奖池信息失败:roomID=%d", roomID);
		return false;
	}

	return true;
}

bool CGameDesk::GetRoomConfigInfo(char configInfo[2048], int size)
{
	if (configInfo == NULL || size <= 1)
	{
		return false;
	}

	int roomID = m_pDataManage->GetRoomID();
	if (roomID <= 0)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	RoomBaseInfo roomBaseInfo;
	if (!pRedis->GetRoomBaseInfo(roomID, roomBaseInfo))
	{
		return false;
	}

	memcpy(configInfo, roomBaseInfo.configInfo, Min_(sizeof(roomBaseInfo.configInfo), size));

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CGameDesk::OnTimerFriendRoomGameBegin()
{
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_GOLD || roomType == ROOM_TYPE_MATCH || m_bPlayGame)
	{
		return false;
	}

	if (!m_enable)
	{
		return false;
	}

	int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	//先计算当前人数
	int currUserCount = 0;
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];

		int userID = deskUserInfo.userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID(%d)", userID);
			continue;
		}
		currUserCount++;
	}

	// 踢掉金钱不足的玩家
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];

		int userID = deskUserInfo.userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID(%d)", userID);
			continue;
		}

		if (roomType == ROOM_TYPE_FRIEND && pUser->money < m_RemoveMinPoint || roomType == ROOM_TYPE_FG_VIP && pUser->fireCoin < m_RemoveMinPoint)
		{
			// 通知桌子所有玩家
			BroadcastUserLeftData((BYTE)i, roomType == ROOM_TYPE_FRIEND ? REASON_KICKOUT_STAND_MINLIMIT : REASON_KICKOUT_STAND_FIRECOIN_MINLIMIT);

			//清理玩家游戏数据
			pUser->playStatus = USER_STATUS_STAND;
			pUser->deskIdx = INVALID_DESKIDX;
			pUser->deskStation = INVALID_DESKSTATION;

			// 清理桌子玩家数据
			deskUserInfo.clear();

			//保存这个位置玩家被清理
			deskUserInfo.leftRoomUser = userID;

			// 内存和缓存中移除玩家数据
			m_pDataManage->DelUser(pUser->userID);

			// 通知桌子人数变化
			--currUserCount;
			NotifyLogonBuyDeskInfoChange(m_masterID, currUserCount, userID, 1, deskMixID);
		}
	}

	// 清除缓存中玩家数量
	pRedis->SetPrivateDeskCurrUserCount(deskMixID, currUserCount);

	///////////////////////////////自动开始游戏功能，可以注释不要///////////////////////////////////////////
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
		int userID = deskUserInfo.userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("金币房踢人出错，GetUser failed userID(%d)", userID);
			continue;
		}
		pUser->playStatus = USER_STATUS_AGREE;
	}

	//////////////////////////踢人需要判断，是否可以开始游戏////////////////////////////////////////////////
	if (CanBeginGame())
	{
		GameBegin(0);
	}

	return true;
}

void CGameDesk::LogonDissmissDesk(int userID, bool bDelete)
{
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD || roomType == ROOM_TYPE_MATCH)
	{
		ERROR_LOG("金币场不能解散桌子");
		return;
	}

	m_friendsGroupMsg.bDeleteDesk = bDelete;
	if (m_isBegin)
	{
		ERROR_LOG("游戏已经开始解散俱乐部牌桌失败 ,userID = %d", userID);
		return;
	}

	//直接解散桌子
	OnDeskDissmissFinishSendData();
	OnDeskSuccessfulDissmiss(true);
}

void CGameDesk::LogonDissmissDesk()
{
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD || roomType == ROOM_TYPE_MATCH)
	{
		ERROR_LOG("金币场不能解散桌子");
		return;
	}

	m_friendsGroupMsg.bDeleteDesk = true;

	//直接解散桌子
	OnDeskDissmissFinishSendData();
	OnDeskSuccessfulDissmiss(true);
}

bool CGameDesk::IsHaveDeskStation(int userID, const PrivateDeskInfo &info)
{
	if (m_pDataManage->IsCanWatch())
	{
		return true;
	}

	int iConfigCount = 0;
	if (m_pDataManage->IsMultiPeopleGame())
	{
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(info.gameRules, value))
		{
			iConfigCount = value["rs"].asInt();
		}
	}
	int deskUserInfoVecSize = iConfigCount > 0 ? iConfigCount : m_deskUserInfoVec.size();

	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_PRIVATE)
	{
		for (int i = 0; i < deskUserInfoVecSize; i++)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
			if (deskUserInfo.userID == 0 || deskUserInfo.userID == userID)
			{
				return true;
			}
		}

		return false;
	}
	else if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		for (int i = 0; i < deskUserInfoVecSize; i++)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
			if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == userID || deskUserInfo.userID == userID && deskUserInfo.leftRoomUser == 0)
			{
				return true;
			}
		}

		for (int i = 0; i < deskUserInfoVecSize; ++i)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
			if (deskUserInfo.userID == 0 && deskUserInfo.leftRoomUser == 0)
			{
				return true;
			}
		}
		return false;
	}

	return true;
}

bool CGameDesk::SetUserScore(BYTE deskStation, long long score)
{
	if (deskStation >= MAX_PLAYER_GRADE)
	{
		return false;
	}

	m_uScore[deskStation] = score;

	return true;
}

long long CGameDesk::GetUserScore(BYTE deskStation)
{
	if (deskStation >= MAX_PLAYER_GRADE)
	{
		return 0;
	}

	return m_uScore[deskStation];
}

bool CGameDesk::KickOutUser(BYTE deskStation, int ReaSon)
{
	// 游戏中不能踢人
	if (IsPlayGame(deskStation))
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	if (deskStation >= m_deskUserInfoVec.size())
	{
		return false;
	}

	// 踢掉玩家
	DeskUserInfo& deskUserInfo = m_deskUserInfoVec[deskStation];
	int userID = deskUserInfo.userID;
	if (userID <= 0)
	{
		return false;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID(%d)", userID);
		return false;
	}

	//玩家被踢
	UserBeKicked(deskStation);

	// 通知桌子所有玩家
	BroadcastUserLeftData(deskStation, ReaSon);

	// 清理桌子玩家数据
	deskUserInfo.clear();

	// 设置玩家为站起状态
	pUser->playStatus = USER_STATUS_STAND;
	pUser->deskIdx = INVALID_DESKIDX;
	pUser->deskStation = INVALID_DESKSTATION;

	// 内存和缓存中移除玩家数据
	if (!pUser->isVirtual)
	{
		m_pDataManage->DelUser(pUser->userID);
	}

	///////////////////如果是非金币场和比赛场，需要修改redis数据//////////////////////
	int deskMixID = MAKE_DESKMIXID(m_pDataManage->GetRoomID(), m_deskIdx);
	if (pRedis->IsKeyExists(TBL_CACHE_DESK, deskMixID))
	{
		int currUserCount = 0;
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			if (m_deskUserInfoVec[i].userID <= 0)
			{
				continue;
			}

			GameUserInfo* pUserTemp = m_pDataManage->GetUser(m_deskUserInfoVec[i].userID);
			if (!pUserTemp)
			{
				continue;
			}

			currUserCount++;
		}

		// 清除缓存中玩家数量
		pRedis->SetPrivateDeskCurrUserCount(deskMixID, currUserCount);

		// 通知桌子人数变化
		NotifyLogonBuyDeskInfoChange(m_masterID, currUserCount, userID, 1, deskMixID);
	}

	return true;
}

void CGameDesk::OnDeskDissmissFinishSendData()
{
	if (m_iRunGameCount <= 0)
	{
		return;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return;
	}

	LoaderNotifyDismissSuccessData msg;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i >= MAX_PLAYER_GRADE)
		{
			break;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			if (m_deskUserInfoVec[i].leftRoomUser <= 0)
			{
				continue;
			}
			else
			{
				userID = m_deskUserInfoVec[i].leftRoomUser;
			}
		}

		UserData userData;
		if (!pRedis->GetUserData(userID, userData))
		{
			ERROR_LOG("大结算发送玩家信息失败：userID=%d", userID);
			continue;
		}

		//填充玩家id
		msg.userID[i] = userID;
		memcpy(msg.name[i], userData.name, sizeof(userData.name));
		memcpy(msg.headURL[i], userData.headURL, sizeof(userData.headURL));
	}

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_DISMISS_USERID);
}

bool CGameDesk::GetRoomResNums(BYTE deskStation, long long &resNums)
{
	if (deskStation >= m_deskUserInfoVec.size())
	{
		return false;
	}

	int userID = m_deskUserInfoVec[deskStation].userID;
	if (userID <= 0)
	{
		return false;
	}

	GameUserInfo * pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		return false;
	}

	resNums = 0;

	int roomType = GetRoomType();

	if (roomType == ROOM_TYPE_FG_VIP)
	{
		resNums = pUser->fireCoin;
	}
	else if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_GOLD)
	{
		resNums = pUser->money;
	}
	else if (roomType == ROOM_TYPE_PRIVATE)
	{
		resNums = m_uScore[deskStation];
	}
	else if (roomType == ROOM_TYPE_MATCH)
	{
		resNums = pUser->matchSocre;
	}

	return true;
}

bool CGameDesk::GetUserBagData(BYTE bDeskStation, UserBag & userBagData)
{
	int userID = GetUserIDByDeskStation(bDeskStation);
	if (userID <= 0)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	pRedis->GetUserBag(userID, userBagData);

	return true;

}

int CGameDesk::GetUserBagDataByKey(BYTE bDeskStation, const char * resName)
{
	if (!resName)
	{
		return false;
	}

	int userID = GetUserIDByDeskStation(bDeskStation);
	if (userID <= 0)
	{
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	int resNums = pRedis->GetUserBagCount(userID, resName);

	return resNums;

}
double CGameDesk::GetDeskPercentage()
{
	if (m_roomTipTypeNums <= 0 || m_roomTipTypeNums >= 100)
	{
		return 0.03;
	}
	return double(m_roomTipTypeNums) / 100.0;
}

bool CGameDesk::SetDeskPercentageScore(int * arPoint, int * ratePoint/* = NULL*/, bool arPointIsChange/* = true*/)
{
	if (m_roomTipType == ROOM_TIP_TYPE_NO)
	{
		return true;
	}

	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_PRIVATE || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return true;
	}

	if (NULL == arPoint)
	{
		return false;
	}

	long long llArPont[MAX_PLAYER_GRADE] = { 0 };
	for (int i = 0; i < m_iConfigCount; i++)
	{
		llArPont[i] = arPoint[i];
	}

	long long llRatePont[MAX_PLAYER_GRADE] = { 0 };
	if (ratePoint)
	{
		for (int i = 0; i < m_iConfigCount; i++)
		{
			llRatePont[i] = ratePoint[i];
		}
		SetDeskPercentageScore(llArPont, llRatePont, arPointIsChange);
	}
	else
	{
		SetDeskPercentageScore(llArPont, NULL, arPointIsChange);
	}

	for (int i = 0; i < m_iConfigCount; i++)
	{
		arPoint[i] = (int)llArPont[i];
	}

	if (ratePoint)
	{
		for (int i = 0; i < m_iConfigCount; i++)
		{
			ratePoint[i] = (int)llRatePont[i];
		}
	}

	return true;
}

bool CGameDesk::SetDeskPercentageScore(long long * arPoint, long long * ratePoint/* = NULL*/, bool arPointIsChange/* = true*/)
{
	if (!arPoint)
	{
		return false;
	}

	if (m_roomTipType == ROOM_TIP_TYPE_NO)
	{
		return true;
	}

	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_PRIVATE || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return true;
	}

	double rate = GetDeskPercentage();

	if (rate <= 0)
	{
		return true;
	}

	//先找到最大的大赢家
	long long llMaxWinMoney = 0;
	if (m_roomTipType == ROOM_TIP_TYPE_MAX_WIN)
	{
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			if (i >= MAX_PLAYER_GRADE)
			{
				break;
			}

			if (m_uScore[i] > llMaxWinMoney)
			{
				llMaxWinMoney = m_uScore[i];
			}
		}
	}

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i >= MAX_PLAYER_GRADE)
		{
			break;
		}

		if (arPoint[i] > 0 && (m_roomTipType == ROOM_TIP_TYPE_ALL_WIN || m_roomTipType == ROOM_TIP_TYPE_MAX_WIN && arPoint[i] == llMaxWinMoney))
		{
			long long llRate = (long long)floor(arPoint[i] * rate);
			if (arPointIsChange)
			{
				arPoint[i] -= llRate;
			}
			if (ratePoint)
			{
				ratePoint[i] = llRate;
			}
		}
	}

	return true;
}

bool CGameDesk::AddDeskGrade(const char *pVideoCode, const char * gameData, const char * userInfoList)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("redis initialized failed");
		return false;
	}

	if (pVideoCode && strlen(pVideoCode) != 0)
	{
		AsyncEventMsgUploadVideo asyncEvent;
		strcpy(asyncEvent.videoCode, pVideoCode);
		m_pDataManage->m_SQLDataManage.PushLine(&asyncEvent.dataLineHead, sizeof(AsyncEventMsgUploadVideo), ASYNC_EVENT_UPLOAD_VIDEO, 0, 0);
	}

	GameGradeInfo gameGradeInfo;
	gameGradeInfo.currTime = time(NULL);
	gameGradeInfo.masterID = m_masterID;
	gameGradeInfo.roomID = m_pDataManage->GetRoomID();
	gameGradeInfo.deskPasswd = atoi(m_szDeskPassWord);
	gameGradeInfo.inning = m_finishedGameCount;
	memcpy(gameGradeInfo.userInfoList, userInfoList, sizeof(gameGradeInfo.userInfoList));
	if (pVideoCode == NULL)
	{
		strcpy(gameGradeInfo.videoCode, REDIS_STR_DEFAULT);
	}
	else
	{
		memcpy(gameGradeInfo.videoCode, pVideoCode, sizeof(gameGradeInfo.videoCode));
	}
	if (gameData == NULL)
	{
		strcpy(gameGradeInfo.gameData, REDIS_STR_DEFAULT);
	}
	else
	{
		memcpy(gameGradeInfo.gameData, gameData, sizeof(gameGradeInfo.gameData));
	}

	long long gradeID = 0;
	if (pRedis->SetPrivateDeskGrade(gameGradeInfo, gradeID) == false)
	{
		ERROR_LOG("SetPrivateDeskGrade failed");
		return false;
	}

	m_gradeIDVec.push_back(gradeID);

	return true;
}

void CGameDesk::SetBeginUser()
{
	int firstUserID = 0;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID <= 0)
		{
			continue;
		}
		GameUserInfo* pUserDesk = m_pDataManage->GetUser(m_deskUserInfoVec[i].userID);
		if (!pUserDesk)
		{
			continue;
		}

		if (firstUserID == 0)
		{
			firstUserID = m_deskUserInfoVec[i].userID;
		}

		if (pUserDesk->IsOnline)
		{
			m_beginUserID = m_deskUserInfoVec[i].userID;
			break;
		}
	}

	if (m_beginUserID == 0)
	{
		m_beginUserID = firstUserID;
	}

	// 如果房主在，由房主开始游戏
	/*GameUserInfo* pUserMaster = m_pDataManage->GetUser(m_masterID);
	if (pUserMaster && pUserMaster->IsOnline && pUserMaster->deskIdx == m_deskIdx)
	{
		m_beginUserID = m_masterID;
	}*/
}

int CGameDesk::GetUserControlParam(BYTE deskStation)
{
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("redis initialized failed");
		return -1;
	}

	int userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return -1;
	}

	int value = -1;

	pRedis->GetUserControlParam(userID, value);

	return value;
}

bool CGameDesk::IsKickOutVirtual(BYTE deskStation)
{
	if (!HundredGameIsInfoChange(deskStation))
	{
		return false;
	}

	// 坐桌数量
	int iMinRobotCount = m_pDataManage->GetPoolConfigInfo("minC");
	int iMaxRobotCount = m_pDataManage->GetPoolConfigInfo("maxC");
	iMinRobotCount = iMinRobotCount <= 0 ? 3 : iMinRobotCount;
	iMaxRobotCount = iMaxRobotCount <= 0 ? 40 : iMaxRobotCount;

	int iRobotCount = GetRobotPeople();
	if (iRobotCount < iMinRobotCount)
	{
		return false;
	}

	if (iRobotCount > iMaxRobotCount)
	{
		return true;
	}

	// 获取机器人最大和最小携带金币，将金币超过限制的机器人踢出去
	int iMinRobotHaveMoney_ = m_pDataManage->GetPoolConfigInfo("minM");
	int iMaxRobotHaveMoney_ = m_pDataManage->GetPoolConfigInfo("maxM");
	long long llResNums = 0;
	if (iMaxRobotHaveMoney_ > 0 && GetRoomResNums(deskStation, llResNums) && (llResNums > iMaxRobotHaveMoney_ || llResNums < iMinRobotHaveMoney_))
	{
		return true;
	}

	int iUpdateRate = m_pDataManage->GetPoolConfigInfo("updateRate");
	iUpdateRate = iUpdateRate <= 0 ? 5 : iUpdateRate;
	if (CUtil::GetRandNum() % 100 < iUpdateRate)
	{
		return false;
	}

	return true;
}

bool CGameDesk::IsOneToOnePlatform()
{
	return m_pDataManage->IsOneToOnePlatform();
}

int CGameDesk::GetPlatformMultiple()
{
	return IsOneToOnePlatform() ? 100 : 1;
}

//////////////////////////////////////////////////////////////////////////
// 比赛场

// 初始化比赛场数据
void CGameDesk::InitDeskMatchData()
{
	m_llPartOfMatchID = 0;
	m_iCurMatchRound = 0;
	m_iMaxMatchRound = 0;
	m_llStartMatchTime = 0;
	m_bFinishMatch = false;

	// 清理桌子玩家
	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		m_deskUserInfoVec[i].clear();
	}

	ReSetGameState(0);
	InitBuyDeskData();
}

// 比赛场游戏结束
bool CGameDesk::MatchRoomGameBegin()
{
	//记录比赛场相关信息
	m_llStartMatchTime = 0;
	m_bFinishMatch = false;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}
		m_pDataManage->SetUserPlayStatus(userID, USER_STATUS_PLAYING);
	}

	//记录游戏开始需要的数据
	m_bPlayGame = true;
	m_beginTime = time(NULL);

	// 局数
	m_iRunGameCount++;

	// 告诉桌子的玩家游戏开始了
	BroadcastDeskData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GAMEBEGIN);

	return true;
}

// 比赛场游戏结束
bool CGameDesk::MatchRoomGameFinish()
{
	m_bFinishMatch = true;
	m_llStartMatchTime = 0;
	m_bPlayGame = false;
	BroadcastDeskData(NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_GAMEFINISH);

	// 将旁观玩家踢出去
	if (m_matchWatchUserID.size() > 0)
	{
		for (auto iter = m_matchWatchUserID.begin(); iter != m_matchWatchUserID.end(); ++iter)
		{
			int userID = *iter;
			if (userID <= 0)
			{
				continue;
			}

			GameUserInfo *pUser = m_pDataManage->GetUser(userID);
			if (pUser)
			{
				MatchQuitMyDeskWatch(pUser, pUser->socketIdx, 2);
			}
		}

		m_matchWatchUserID.clear();
	}

	if (m_pDataManage->IsAllDeskFinishMatch(m_llPartOfMatchID)) //本轮游戏结束
	{
		if (m_iCurMatchRound >= m_iMaxMatchRound) //决赛轮，整个比赛结束
		{
			m_pDataManage->MatchEnd(m_llPartOfMatchID, m_iCurMatchRound, m_iMaxMatchRound);
		}
		else //淘汰部玩家，进入下一轮
		{
			m_pDataManage->MatchNextRound(m_llPartOfMatchID, m_iCurMatchRound, m_iMaxMatchRound);
		}
	}
	else
	{
		// 根据业务需求，这里也可以淘汰玩家
		m_pDataManage->MatchDeskFinish(m_llPartOfMatchID, m_deskIdx);
	}

	return true;
}

// 发送当前桌子的比赛状态
void CGameDesk::SendMatchDeskStatus(int userID)
{
	if (userID <= 0)
	{
		return;
	}

	LoaderNotifyDeskMatchStatus msg;
	msg.iCurMatchRound = m_iCurMatchRound;
	msg.iMaxMatchRound = m_iMaxMatchRound;
	msg.llPartOfMatchID = m_llPartOfMatchID;

	if (IsPlayGame(0))
	{
		msg.status = 0;
	}
	else if (m_bFinishMatch)
	{
		msg.status = 1;
	}
	else
	{
		msg.status = 2;
		msg.remainTime = int(m_llStartMatchTime - time(NULL));
	}

	m_pDataManage->SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MATCH_STATUS, 0);
}

// 比赛场坐桌逻辑
bool CGameDesk::MatchRoomSitDeskLogic(GameUserInfo* pUser)
{
	if (!pUser)
	{
		ERROR_LOG("MatchRoomSitDeskLogic::pUser==NULL");
		return false;
	}

	int userID = pUser->userID;
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID: userID=%d", userID);
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return false;
	}

	time_t currTime = time(NULL);

	if (pUser->deskStation == INVALID_DESKSTATION)
	{
		// 第一次坐下
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			if (m_deskUserInfoVec[i].userID == userID)
			{
				ERROR_LOG("user deskStation is invalid but user is in desk userID=%d", userID);
				return false;
			}
		}

		// 分配一个座位
		for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
		{
			DeskUserInfo& deskUserInfo = m_deskUserInfoVec[i];
			if (deskUserInfo.userID == 0)
			{
				deskUserInfo.userID = userID;
				pUser->deskStation = (BYTE)i;
				break;
			}
		}
	}

	// 这里玩家不可能没有座位了
	if (pUser->deskStation >= m_deskUserInfoVec.size())
	{
		ERROR_LOG("user deskStation is invalid userID = %d deskStation = %d", userID, pUser->deskStation);
		return false;
	}

	// 设置房间id
	pRedis->SetUserRoomID(pUser->userID, m_pDataManage->GetRoomID());

	// 设置比赛状态
	pRedis->SetUserMatchStatus(pUser->userID, m_pDataManage->m_matchTypeMap[m_llPartOfMatchID], USER_MATCH_STATUS_PLAYING);

	// 设置玩家的桌子号，座位号，状态等
	pUser->deskIdx = m_deskIdx;
	if (USER_STATUS_PLAYING != pUser->playStatus)
	{
		pUser->playStatus = USER_STATUS_SITING;
	}

	m_deskUserInfoVec[pUser->deskStation].bNetCut = false;
	m_deskUserInfoVec[pUser->deskStation].lastWaitAgreeTime = currTime;
	m_deskUserInfoVec[pUser->deskStation].isVirtual = pUser->isVirtual;

	// 广播坐下的信息
	BroadcastUserSitData(pUser->deskStation);

	// 发送当前桌子状态
	SendMatchDeskStatus(userID);

	return true;
}

// 比赛场玩家离开桌子
bool CGameDesk::MatchRoomUserLeftDeskLogic(GameUserInfo * pUser)
{
	if (!pUser)
	{
		ERROR_LOG("比赛场开开错误::pUser is NULL");
		return false;
	}

	if (pUser->deskStation >= (BYTE)m_deskUserInfoVec.size())
	{
		ERROR_LOG("user(%d) deskStation is invalid deskStation=%d", pUser->userID, pUser->deskStation);
		return false;
	}

	// 清理桌子中玩家的数据
	m_deskUserInfoVec[pUser->deskStation].clear();

	// 清理玩家游戏数据
	pUser->playStatus = USER_STATUS_STAND;
	pUser->deskIdx = INVALID_DESKIDX;
	pUser->deskStation = INVALID_DESKSTATION;

	return true;
}

// 比赛场获取一个桌子所有玩家id
int CGameDesk::MatchGetDeskUserID(int arrUserID[MAX_PLAYER_GRADE])
{
	if (arrUserID == NULL)
	{
		return 0;
	}

	int peopleCount = 0;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (m_deskUserInfoVec[i].userID > 0)
		{
			arrUserID[peopleCount++] = m_deskUserInfoVec[i].userID;
		}
	}

	return peopleCount;
}

// 比赛场进入本桌旁观
bool CGameDesk::MatchEnterMyDeskWatch(GameUserInfo * pUser, long long partOfMatchID)
{
	if (!pUser)
	{
		return false;
	}

	LoaderResponseMatchEnterWatch msg;
	int iSendSize = 5;

	if (m_llPartOfMatchID == 0 || partOfMatchID != m_llPartOfMatchID)
	{
		ERROR_LOG("异常错误 ：deskIdx=%d,partOfMatchID=%lld", m_deskIdx, partOfMatchID);
		msg.result = 4;
		m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, &msg, iSendSize, MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK, 0, pUser->userID);
		return false;
	}

	if (!IsPlayGame(0))
	{
		ERROR_LOG("本桌游戏结束 ：%d", m_deskIdx);
		msg.result = 1;
		m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, &msg, iSendSize, MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK, 0, pUser->userID);
		return false;
	}

	// 不能同时旁观两个桌子
	if (pUser->watchDeskIdx != INVALID_DESKIDX)
	{
		ERROR_LOG("已经有旁观桌子了 pUser->watchDeskIdx=%d", pUser->watchDeskIdx);
		msg.result = 2;
		m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, &msg, iSendSize, MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK, 0, pUser->userID);
		return false;
	}

	// 游戏中不能去旁观其它玩家
	CGameDesk* pDesk = m_pDataManage->GetDeskObject(pUser->deskIdx);
	if (pDesk && pDesk->IsPlayGame(0))
	{
		ERROR_LOG("游戏中不能旁观,userID=%d", pUser->userID);
		msg.result = 3;
		m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, &msg, iSendSize, MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK, 0, pUser->userID);
		return false;
	}

	// 发送玩家
	char buf[MAX_TEMP_SENDBUF_SIZE] = "";
	buf[0] = 0;
	int realSize = 0;
	MakeAllUserInfo(buf + 1, sizeof(buf) - 1, realSize);
	memcpy(&msg, buf, realSize);
	iSendSize = realSize + 1;
	m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, &msg, iSendSize, MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK, 0, pUser->userID);

	//发送用户游戏状态(游戏实现)
	OnGetGameStation(INVALID_DESKSTATION, pUser->socketIdx, true);

	// 加入旁观
	pUser->watchDeskIdx = m_deskIdx;
	m_matchWatchUserID.insert(pUser->userID);

	return true;
}

// 比赛场退出本桌旁观
bool CGameDesk::MatchQuitMyDeskWatch(GameUserInfo * pUser, int socketIdx, BYTE result)
{
	if (!pUser)
	{
		return false;
	}

	pUser->watchDeskIdx = INVALID_DESKIDX;
	m_matchWatchUserID.erase(pUser->userID);

	if (pUser->socketIdx <= 0)
	{
		return true;
	}

	LoaderMatchQuitWatch msg;
	msg.result = result;
	m_pDataManage->m_pGServerConnect->SendData(pUser->socketIdx, &msg, sizeof(msg), MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_QUIT_WATCH_DESK, 0, pUser->userID);

	return true;
}

bool CGameDesk::ChangeUserPointMatchRoom(long long *arPoint)
{
	if (!arPoint)
	{
		ERROR_LOG("invalid arPoint");
		return false;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		ERROR_LOG("redis initialized failed");
		return false;
	}

	LoaderNotifyUserMatchRoomGrade userGrade;

	for (size_t i = 0; i < m_deskUserInfoVec.size(); i++)
	{
		if (i >= MAX_PLAYER_GRADE)
		{
			break;
		}

		int userID = m_deskUserInfoVec[i].userID;
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		pRedis->SetUserTotalGameCount(userID);

		if (arPoint[i] > 0)
		{
			pRedis->SetUserWinCount(userID);
		}

		pUser->matchSocre += (int)arPoint[i];

		userGrade.changeGrade[i] = (int)arPoint[i];
		userGrade.grade[i] = pUser->matchSocre;
	}

	m_finishedTime = time(NULL);

	BroadcastDeskData(&userGrade, sizeof(userGrade), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_MATCH_GRADE);

	return true;
}

bool CGameDesk::ChangeUserPointMatchRoom(int *arPoint)
{
	if (NULL == arPoint)
	{
		return false;
	}
	long long llArPont[MAX_PLAYER_GRADE] = { 0 };
	for (int i = 0; i < m_iConfigCount; i++)
	{
		llArPont[i] = arPoint[i];
	}
	return ChangeUserPointMatchRoom(llArPont);
}