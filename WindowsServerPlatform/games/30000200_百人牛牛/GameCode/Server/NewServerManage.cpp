#include "Stdafx.h"
#include "NewServerManage.h"
#include "UpgradeMessage.h"

CNewServerManage::CNewServerManage() : CGameDesk(ALL_ARGEE)
{
	m_gameStatus = 0;
	m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	m_xiaZhuangRightNow = false;
	m_currDealerGameCount = 0;
	m_startTime = 0;
	m_curCtrlWinCount = 0;
	m_runCtrlCount = 0;
	m_ctrlParamIndex = 0;
	memset(m_szLastCtrlParam, 0, sizeof(m_szLastCtrlParam));
}

CNewServerManage::~CNewServerManage()
{
}

bool CNewServerManage::InitDeskGameStation()
{
	m_userInfoVec.resize(PLAY_COUNT);

	LoadDynamicConfig();

	m_dealerResNums = m_gameConfig.shangZhuangLimitMoney;


	ReSetGameState(0);

	return true;
}

bool CNewServerManage::OnStart()
{
	m_gameStatus = GAME_STATUS_WAITNEXT;
	SetTimer(GAME_TIMER_WAITNEXT, m_gameConfig.waitBeginTime * 1000);		// 设置下一阶段的定时器

	ChangeSystemZhuangInfo();

	return true;
}

bool CNewServerManage::OnGetGameStation(BYTE deskStation, UINT socketID, bool bWatchUser)
{
	if (deskStation >= PLAY_COUNT)
	{
		return false;
	}

	GameUserInfo user;
	GetUserData(deskStation, user);
	m_bIsSuper[deskStation] = (user.userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER ? 1 : 0;

	NewGameMessageGameStation msg;

	msg.byIsSuper = m_bIsSuper[deskStation];

	// 配置相关
	msg.cfgWaitBeginKeepTime = m_gameConfig.waitBeginTime;
	msg.cfgNoteKeepTime = m_gameConfig.noteKeepTime;
	msg.cfgSendCardKeepTime = m_gameConfig.sendCardKeepTime;
	msg.cfgCompareKeepTime = m_gameConfig.compareKeepTime;
	msg.cfgSettleKeepTime = m_gameConfig.waitSettleTime;
	msg.cfgShangZhuangLimitMoney = m_gameConfig.shangZhuangLimitMoney;
	msg.cfgSitLimitMoney = m_gameConfig.sitLimitMoney;
	int roomLevel = GetRoomLevel() >= 1 && GetRoomLevel() <= 3 ? GetRoomLevel() - 1 : 0;
	memcpy(msg.cfgNoteList, m_gameConfig.noteList[roomLevel], sizeof(msg.cfgNoteList));

	msg.gameStatus = m_gameStatus;
	msg.nextStageLeftSecs = (int)GetNextStageLeftSecs();
	const BrnnUserInfo& userInfo = m_userInfoVec[deskStation];
	memcpy(msg.myAreaNoteList, userInfo.noteList, sizeof(msg.myAreaNoteList));
	//发送区域下注
	if (msg.byIsSuper)
	{
		for (int i = 0; i < MAX_AREA_COUNT; i++)
		{
			msg.areaNoteList[i] = m_areaInfoArr[i].areaRealPeopleNote;
		}
	}
	else
	{
		for (int i = 0; i < MAX_AREA_COUNT; i++)
		{
			msg.areaNoteList[i] = m_areaInfoArr[i].areaNote;
		}
	}

	MakeSendCardInfo(msg.sendCardInfo);
	MakeZhuangInfo(msg.zhuangInfo);

	memset(msg.deskUser, 255, sizeof(msg.deskUser));
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		msg.deskUser[i] = m_gameDesk[i];
	}
	memcpy(msg.areaChouMaCount, m_areaChouMaCount, sizeof(msg.areaChouMaCount));

	SendGameStation(deskStation, socketID, bWatchUser, &msg, sizeof(msg));

	return true;
}

bool CNewServerManage::HandleNotifyMessage(BYTE deskStation, unsigned int assistID, void * pData, int size, bool bWatchUser)
{
	if (bWatchUser == true)
	{
		return false;
	}

	if (deskStation >= (BYTE)m_userInfoVec.size())
	{
		return false;
	}
	switch (assistID)
	{
	case MSG_GAME_REQ_SHANGZHUANG:
	{
		OnHandleUserRequestShangZhuang(deskStation, pData, size);
		return true;
	}
	case MSG_GAME_REQ_XIAZHUANG:
	{
		OnHandleUserRequestXiaZhuang(deskStation);
		return true;
	}
	case MSG_GAME_REQ_NOTE:
	{
		OnHandleUserReuqestNote(deskStation, pData, size);
		return true;
	}
	case MSG_GAME_REQ_CONTINUE_NOTE:
	{
		OnHandleRequestContinueNote(deskStation);
		return true;
	}
	case MSG_GAME_REQ_AREA_TREND:
	{
		OnHandleRequestAreaTrend(deskStation);
		return true;
	}
	case MSG_GAME_REQ_SUPER_SET:
	{
		OnHandleRequestSuperSet(deskStation, pData, size);
		return true;
	}
	case MSG_GAME_REQ_SIT:
	{
		OnHandleRequestSit(deskStation, pData, size);
		return true;
	}
	default:
		break;
	}

	return __super::HandleNotifyMessage(deskStation, assistID, pData, size, bWatchUser);
}

bool CNewServerManage::OnTimer(unsigned int timerID)
{
	if (timerID != GAME_TIMER_NOTEINFO)
	{
		KillTimer(timerID);
	}

	switch (timerID)
	{
	case GAME_TIMER_WAITNEXT:
	{
		OnTimerWaitNext();
		return true;
	}
	case GAME_TIMER_NOTE:
	{
		OnTimerNote();
		return true;
	}
	case GAME_TIMER_NOTEINFO:
	{
		OnTimerSendNoteInfo();
		return true;
	}
	case GAME_TIMER_SENDCARD:
	{
		OnTimerSendCard();
		return true;
	}
	case GAME_TIMER_COMPARE:
	{
		OnTimerCompare();
		return true;
	}
	case GAME_TIMER_SETTLE:
	{
		OnTimerSettle();
		return true;
	}
	default:
		break;
	}

	return true;
}

bool CNewServerManage::ReSetGameState(BYTE bLastStation)
{
	// 清理筹码记录
	m_tempChouMaDataCount = 0;
	memset(m_areaChouMaCount, 0, sizeof(m_areaChouMaCount));

	// 清除区域信息
	for (size_t i = 0; i < m_areaInfoArr.size(); i++)
	{
		m_areaInfoArr[i].Clear();
	}
	m_dealerInfo.Clear();

	// 清除玩家的下注信息
	for (size_t i = 0; i < m_userInfoVec.size(); i++)
	{
		BrnnUserInfo& userInfo = m_userInfoVec[i];
		userInfo.clearNoteInfo();
	}

	// 清理上庄列表
	if (!m_shangZhuangList.empty())
	{
		for (auto it = m_shangZhuangList.begin(); it != m_shangZhuangList.end();)
		{
			long userID = it->userID;
			if (GetUserIDByDeskStation(it->deskStation) != userID)
			{
				m_shangZhuangList.erase(it++);
				continue;
			}
			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			if (pUser == NULL)
			{
				m_shangZhuangList.erase(it++);
				continue;
			}
			if (pUser->deskStation == m_dealerDeskStation)
			{
				it++;
				continue;
			}
			if (pUser->money < it->money)
			{
				m_shangZhuangList.erase(it++);
				continue;
			}
			if (pUser->IsOnline == false)
			{
				m_shangZhuangList.erase(it++);
				continue;
			}

			it++;
		}
	}

	// 清理坐下列表
	ClearDeskUser();

	return true;
}

bool CNewServerManage::UserLeftDesk(GameUserInfo* pUser)
{
	ClearShangZhuangList(pUser->userID, pUser->deskStation);

	// 删除座位号
	DelDeskUser(pUser->deskStation);

	return __super::UserLeftDesk(pUser);
}

bool CNewServerManage::UserNetCut(GameUserInfo* pUser)
{
	return __super::UserNetCut(pUser);
}

bool CNewServerManage::OnHandleUserRequestShangZhuang(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(NewGameMessageReqShangZhuang))
	{
		return false;
	}

	NewGameMessageReqShangZhuang* pMessage = (NewGameMessageReqShangZhuang*)pData;
	if (!pMessage)
	{
		return false;
	}

	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID = 0");
		return false;
	}

	long long llUserResNums = 0;
	if (!GetRoomResNums(deskStation, llUserResNums))
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	if (deskStation == m_dealerDeskStation)
	{
		return false;
	}

	// 在上庄列表内无法请求上庄
	if (IsShangZhuangList(userID))
	{
		SendGameMessage(deskStation, "已经在上庄列表中", SMT_EJECT);
		return false;
	}

	// 上庄列表满了无法上庄
	if (m_shangZhuangList.size() >= MAX_SHANGZHUANG_COUNT)
	{
		SendGameMessage(deskStation, "上庄列表已满", SMT_EJECT);
		return true;
	}

	// 身上钱不够
	if (llUserResNums < pMessage->money)
	{
		SendGameMessage(deskStation, "资金不够", SMT_EJECT);
		return true;
	}

	// 钱不够无法上庄
	if (pMessage->money < m_gameConfig.shangZhuangLimitMoney)
	{
		SendGameMessage(deskStation, "资金不够上庄", SMT_EJECT);
		return true;
	}

	// 机器人上庄特殊判断
	if (IsVirtual(deskStation) && !IsVirtualCanShangXiazZhuang(deskStation, 0))
	{
		return false;
	}

	// 加入上庄列表
	m_shangZhuangList.push_back(ShangZhuangUser(userID, pMessage->money, deskStation));

	SendGameMessage(deskStation, "上庄成功！", SMT_EJECT);
	// 通知变化
	BroadcastZhuangInfo();

	return true;
}

bool CNewServerManage::OnHandleUserRequestXiaZhuang(BYTE deskStation)
{
	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID = 0");
		return false;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	// 只有处于上庄列表或者自己为庄家才能下庄
	if (!IsShangZhuangList(userID) && deskStation != m_dealerDeskStation)
	{
		return false;
	}

	// 机器人下庄特殊判断
	if (IsVirtual(deskStation) && !IsVirtualCanShangXiazZhuang(deskStation, 1))
	{
		return false;
	}

	if (deskStation == m_dealerDeskStation)
	{
		// 设置这局结束之后立刻让庄家下庄
		m_xiaZhuangRightNow = true;

		SendGameMessage(deskStation, "本局游戏结束后，立马下庄", SMT_EJECT);
	}
	else
	{
		// 从上庄列表中移除
		DelShangZhuangUser(userID);
		SendGameMessage(deskStation, "下庄成功！", SMT_EJECT);

		BroadcastZhuangInfo();
	}

	return true;
}

bool CNewServerManage::OnHandleUserReuqestNote(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(NewGameMessageReqNote))
	{
		return false;
	}
	if (m_gameStatus != GAME_STATUS_NOTE)
	{
		return false;
	}

	NewGameMessageReqNote* pMessage = (NewGameMessageReqNote*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (pMessage->area < 0 || pMessage->area >= MAX_AREA_COUNT)
	{
		ERROR_LOG("area is invalid area=%d", pMessage->area);
		return false;
	}

	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID = 0");
		return false;
	}

	// 庄家不能下注
	if (deskStation == m_dealerDeskStation)
	{
		return false;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}
	long long note = GetNoteByIndex(pMessage->noteIndex);
	if (note <= 0)
	{
		ERROR_LOG("note index is invalid noteIndex=%d", pMessage->noteIndex);
		return false;
	}

	long long llUserResNums = 0;
	if (!GetRoomResNums(deskStation, llUserResNums))
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	//区域总下注值
	long long llAllAreaBet = 0;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		llAllAreaBet += m_areaInfoArr[i].areaNote;
	}

	//玩家下注限制
	long long iAllBet = 0;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		iAllBet += m_userInfoVec[deskStation].noteList[i];
	}

	//下注不能超过玩家金币的百分之十
	if ((iAllBet + note) > (llUserResNums / MAX_SHAPE_RATE))
	{
		SendGameMessage(deskStation, "下注不能超过自身的25%！");
		return true;
	}

	//不能超过系统庄家的百分之十
	if ((llAllAreaBet + note) > m_dealerResNums / MAX_SHAPE_RATE)
	{
		SendGameMessage(deskStation, "下注不能超过庄家的25%！");
		return true;
	}

	if (m_gameConfig.allUserMaxNote > 0 && (llAllAreaBet + note) > m_gameConfig.allUserMaxNote)
	{
		SendGameMessage(deskStation, "下注超过限制");
		return true;
	}

	//计算机器人下注
	if (pUser->isVirtual && !IsVirtualCanNote(deskStation, note))
	{
		return false;
	}

	BrnnUserInfo& userInfo = m_userInfoVec[deskStation];
	AreaInfo& areaInfo = m_areaInfoArr[pMessage->area];

	userInfo.isNote = true;
	userInfo.noteList[pMessage->area] += note;
	areaInfo.areaNote += note;

	//保存真人下注值
	if (!pUser->isVirtual)
	{
		areaInfo.areaRealPeopleNote += note;
	}

	// 通知玩家自己下注成功
	NewGameMessageRspNote msg;

	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		msg.noteList[i] = userInfo.noteList[i];
	}

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_NOTE, 0);

	//记录筹码
	m_areaChouMaCount[pMessage->area][pMessage->noteIndex] ++;
	if (m_tempChouMaDataCount < MAX_CHOU_MA_COUNT && m_tempChouMaDataCount >= 0)
	{
		m_tempChouMaData[m_tempChouMaDataCount].deskStation = deskStation;
		m_tempChouMaData[m_tempChouMaDataCount].index = pMessage->noteIndex;
		m_tempChouMaData[m_tempChouMaDataCount].area = pMessage->area;
		m_tempChouMaData[m_tempChouMaDataCount].nums = note;
		m_tempChouMaDataCount++;
	}

	BrodcastNoteInfo();

	return true;
}

bool CNewServerManage::OnHandleRequestContinueNote(BYTE deskStation)
{
	if (m_gameStatus != GAME_STATUS_NOTE)
	{
		return false;
	}

	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		ERROR_LOG("invalid userID = 0");
		return false;
	}

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	long long llUserResNums = 0;
	if (!GetRoomResNums(deskStation, llUserResNums))
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	NewGameMessageRspContinueNote msg;

	//自己是庄家，无法续压
	if (deskStation == m_dealerDeskStation)
	{
		SendGameMessage(deskStation, "您已经是庄家，无法续压！", SMT_EJECT);
		return false;
	}

	// 下注了就不能续压
	BrnnUserInfo& userInfo = m_userInfoVec[deskStation];
	if (userInfo.isNote == true)
	{
		SendGameMessage(deskStation, "您已经下注了，无法续压！", SMT_EJECT);
		return false;
	}

	// 已经续压了不能再次续压
	if (userInfo.isContinueNote == true)
	{
		SendGameMessage(deskStation, "您已经续压了，无法续压！", SMT_EJECT);
		return false;
	}

	//区域总下注值
	long long llAllAreaBet = 0;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		llAllAreaBet += m_areaInfoArr[i].areaNote;
	}

	//玩家续压限制
	long long iAllbet = 0;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		iAllbet += m_userInfoVec[deskStation].lastNoteList[i];
	}

	//上局没有下注
	if (iAllbet <= 0)
	{
		SendGameMessage(deskStation, "您上局没有下注，无法续压！", SMT_EJECT);
		return true;
	}

	//续压不能超过当前金币的百分之10
	if (iAllbet > llUserResNums / MAX_SHAPE_RATE)
	{
		SendGameMessage(deskStation, "您续压金币超过了自身的25%，续压不成功！", SMT_EJECT);
		return true;
	}

	//不能超过系统庄家的百分之十
	if ((llAllAreaBet + iAllbet) > m_dealerResNums / MAX_SHAPE_RATE)
	{
		SendGameMessage(deskStation, "您续压金币超过了庄家的25%，续压不成功！", SMT_EJECT);
		return true;
	}

	if (m_gameConfig.allUserMaxNote > 0 && (llAllAreaBet + iAllbet) > m_gameConfig.allUserMaxNote)
	{
		SendGameMessage(deskStation, "下注超过限制");
		return true;
	}

	userInfo.isContinueNote = true;
	userInfo.isNote = true;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		userInfo.noteList[i] = userInfo.lastNoteList[i];
		m_areaInfoArr[i].areaNote += userInfo.lastNoteList[i];
		if (!pUser->isVirtual)
		{
			m_areaInfoArr[i].areaRealPeopleNote += userInfo.lastNoteList[i];
		}
	}

	msg.errorCode = 0;
	msg.isContinueNote = true;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		msg.noteList[i] = userInfo.noteList[i];
	}
	SendGameMessage(deskStation, "续压成功！", SMT_EJECT);
	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_CONTINUE_NOTE, 0);

	BrodcastNoteInfo();

	return true;
}

bool CNewServerManage::OnHandleRequestAreaTrend(BYTE deskStation)
{
	NewGameMessageRspAreaTrend msg;

	for (size_t i = 0; i < m_areaInfoArr.size(); i++)
	{
		AreaInfo& areaInfo = m_areaInfoArr[i];

		int idx = 0;
		auto iter = areaInfo.resultList.begin();
		for (; iter != areaInfo.resultList.end(); iter++)
		{
			msg.isWin[i][idx] = *iter;
			idx++;
		}
	}

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_AREA_TREND, 0);

	return true;
}

bool CNewServerManage::OnHandleRequestSuperSet(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(NewGameMessageReqSuperSet))
	{
		return false;
	}

	NewGameMessageReqSuperSet* pMessage = (NewGameMessageReqSuperSet*)pData;
	if (!pMessage)
	{
		return false;
	}

	GameUserInfo userData;
	if (!GetUserData(deskStation, userData))
	{
		return false;
	}

	if ((userData.userStatus&USER_IDENTITY_TYPE_SUPER) != USER_IDENTITY_TYPE_SUPER)
	{
		return false;
	}

	m_bySuperSetType = pMessage->bySuperSetType;
	m_bySuperSetArea = pMessage->bySuperSetArea;

	SendGameMessage(deskStation, "设置成功", SMT_EJECT);

	return true;
}

bool CNewServerManage::OnHandleRequestSit(BYTE deskStation, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, NewGameMessageReqSit, pData, size);

	long long llUserResNums = 0;
	if (!GetRoomResNums(deskStation, llUserResNums))
	{
		ERROR_LOG("GetUser failed userID=%d", GetUserIDByDeskStation(deskStation));
		return false;
	}

	//随机机器人坐下数量
	if (IsVirtual(deskStation) && !IsVirtualCanSit(deskStation, pMessage->type))
	{
		return false;
	}

	if (pMessage->type == 0) //坐下
	{
		// 坐下条件判断，金币是否足够
		if (llUserResNums < m_gameConfig.sitLimitMoney)
		{
			char msg[128] = "";
			sprintf_s(msg, sizeof(msg), "您的金币不够%lld，无法坐下", m_gameConfig.sitLimitMoney);
			SendGameMessage(deskStation, msg);
			return false;
		}

		if (deskStation == m_dealerDeskStation)
		{
			SendGameMessage(deskStation, "庄家不能坐下");
			return false;
		}

		if (IsDeskUser(deskStation))
		{
			SendGameMessage(deskStation, "你已经在座位中");
			return false;
		}

		if (IsDeskHaveUser(pMessage->index))
		{
			SendGameMessage(deskStation, "该座位已经有人");
			return false;
		}

		// 添加到座位
		SetDeskUser(deskStation, pMessage->index);
	}
	else //站起
	{
		if (!IsDeskUser(deskStation))
		{
			SendGameMessage(deskStation, "您没有坐下，不能站起");
			return false;
		}

		// 删除位置
		DelDeskUser(deskStation);
	}

	NewGameMessageRspSit msg;
	msg.index = pMessage->index;
	msg.result = 1;
	msg.type = pMessage->type;

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_SIT, 0);

	return true;
}

void CNewServerManage::OnTimerWaitNext()
{
	KillAllTimer();
	TransToWaitNextStage();
}

void CNewServerManage::OnTimerNote()
{
	KillAllTimer();
	TransToNoteStage();
}

void CNewServerManage::OnTimerSendNoteInfo()
{
	BrodcastNoteInfo();
}

void CNewServerManage::OnTimerSendCard()
{
	KillAllTimer();
	TransToSendCardStage();
}

void CNewServerManage::OnTimerCompare()
{
	KillAllTimer();
	TransToCompareStage();
}

void CNewServerManage::OnTimerSettle()
{
	KillAllTimer();
	TransToSettleStage();
}

long long CNewServerManage::GetNoteByIndex(int index)
{
	if (index < 0 || index >= MAX_NOTE_SORT)
	{
		return 0;
	}

	int roomLevel = GetRoomLevel() >= 1 && GetRoomLevel() <= 3 ? GetRoomLevel() - 1 : 0;

	return m_gameConfig.noteList[roomLevel][index];
}

BYTE CNewServerManage::PopCardFromDesk()
{
	if (m_deskCardQueue.empty())
	{
		// log todo
		return 0xff;
	}

	BYTE card = m_deskCardQueue.front();
	m_deskCardQueue.pop();

	return card;
}

bool CNewServerManage::SendAreaCards(BYTE* pCardList)
{
	if (!pCardList)
	{
		return false;
	}

	for (int i = 0; i < AREA_CARD_COUNT; i++)
	{
		BYTE card = PopCardFromDesk();
		if (card == 0xff)
		{
			ERROR_LOG("invalid card");
			return false;
		}

		pCardList[i] = card;
	}

	return true;
}

void CNewServerManage::KillAllTimer()
{
	for (unsigned int i = GAME_TIMER_DEFAULT + 1; i < GAME_TIMER_END; i++)
	{
		KillTimer(i);
	}
}

bool CNewServerManage::ConfirmDealer()
{
	long dealerUserID = GetUserIDByDeskStation(m_dealerDeskStation);
	if (dealerUserID > 0)
	{
		GameUserInfo * pUser = m_pDataManage->GetUser(dealerUserID);
		if (pUser && !m_xiaZhuangRightNow && m_currDealerGameCount < m_gameConfig.maxZhuangGameCount) //真人坐庄
		{
			return true;
		}
	}

	if (!m_shangZhuangList.empty())
	{
		long userID = m_shangZhuangList.front().userID;
		long long money = m_shangZhuangList.front().money;
		BYTE deskStation = m_shangZhuangList.front().deskStation;
		m_shangZhuangList.pop_front();
		if (userID <= 0)
		{
			ERROR_LOG("m_shangZhuangList front an invalid userID userID=%d", userID);
			return false;
		}

		if (deskStation == INVALID_DESKSTATION)
		{
			ERROR_LOG("GET deskStation failed userID=%d", userID);
			return false;
		}

		if (GetUserIDByDeskStation(deskStation) != userID)
		{
			ERROR_LOG("GetUserIDByDeskStation failed");
			return false;
		}

		GameUserInfo * pUser = m_pDataManage->GetUser(userID);
		if (pUser == NULL)
		{
			ERROR_LOG("GetUser failed userID=%d", userID);
			return false;
		}

		m_dealerDeskStation = deskStation;
		m_dealerResNums = money;
		strcpy(m_dealerName, pUser->name);
		strcpy(m_dealerHeadURL, pUser->headURL);
	}
	else
	{
		// 上庄列表为空， 依然系统坐庄
		m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	}

	return true;
}

time_t CNewServerManage::GetNextStageLeftSecs()
{
	time_t currTime = time(NULL);

	if (m_gameStatus == 0)
	{
		return 0;
	}
	else if (m_gameStatus == GAME_STATUS_WAITNEXT)
	{
		return m_startTime + m_gameConfig.waitBeginTime - currTime;
	}
	else if (m_gameStatus == GAME_STATUS_NOTE)
	{
		return m_startTime + m_gameConfig.waitBeginTime + m_gameConfig.noteKeepTime - currTime;
	}
	else if (m_gameStatus == GAME_STATUS_SENDCARD)
	{
		return m_startTime + m_gameConfig.waitBeginTime + m_gameConfig.noteKeepTime + m_gameConfig.sendCardKeepTime - currTime;
	}
	else if (m_gameStatus == GAME_STATUS_COMPARE)
	{
		return m_startTime + m_gameConfig.waitBeginTime + m_gameConfig.noteKeepTime + m_gameConfig.sendCardKeepTime + m_gameConfig.compareKeepTime - currTime;
	}
	else if (m_gameStatus == GAME_STATUS_SETTLE)
	{
		return m_startTime + m_gameConfig.waitBeginTime + m_gameConfig.noteKeepTime + m_gameConfig.sendCardKeepTime + m_gameConfig.compareKeepTime + m_gameConfig.waitSettleTime - currTime;
	}

	return 0;
}

void CNewServerManage::TransToWaitNextStage()
{
	KillAllTimer();
	m_gameStatus = GAME_STATUS_WAITNEXT;
	m_startTime = time(NULL);

	SetTimer(GAME_TIMER_NOTE, m_gameConfig.waitBeginTime * 1000);
	BroadcastGameStatus();

	// 超端确认
	m_bySuperSetType = 0;
	m_bySuperSetArea = 255;
	memset(m_bIsSuper, 0, sizeof(m_bIsSuper));
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID <= 0)
		{
			continue;
		}

		GameUserInfo * pUser = m_pDataManage->GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		if ((pUser->userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER)
		{
			m_bIsSuper[i] = true;
		}
	}
}

void CNewServerManage::TransToNoteStage()
{
	// 确定庄家
	if (ConfirmDealer() == false)
	{
		ERROR_LOG("ConfirmDealer failed");
		m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	}

	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		m_currDealerGameCount++;
		DelDeskUser(m_dealerDeskStation);
	}

	GameBegin(0);
	m_xiaZhuangRightNow = false;

	ChangeSystemZhuangInfo();

	// 下注阶段完成之后进入发牌阶段
	SetTimer(GAME_TIMER_SENDCARD, (m_gameConfig.noteKeepTime + 1) * 1000);

	m_gameStatus = GAME_STATUS_NOTE;

	// 通知庄信息
	BroadcastZhuangInfo();

	NewGameMessageHasXuYa msg;
	
	for (int i = 0;i<PLAY_COUNT;i++)
	{
		if(GetUserIDByDeskStation(i) <= 0) continue;
		long long NoteData = 0;
		msg.bHasReuqestNote = false;
		for (int idx = 0; idx < MAX_AREA_COUNT; idx++)
		{
			NoteData += m_userInfoVec[i].lastNoteList[idx];
		}
		if (NoteData > 0)
		{
			msg.bHasReuqestNote = true;
		}
		SendGameData(i, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NIF_XUYA,0);
	}



	// 通知所有玩家下注开始
	BroadcastGameStatus();

	// 设置发送下注信息定时器
	//SetTimer(GAME_TIMER_NOTEINFO, 200);
	m_tempChouMaDataCount = 0;
}

void CNewServerManage::TransToSendCardStage()
{
	KillTimer(GAME_TIMER_NOTEINFO);
	BrodcastNoteInfo();

	// 洗牌
	std::vector<BYTE> cardVec;
	m_logic.RandCard(cardVec, 1);

	std::queue<BYTE> tempQueue;
	m_deskCardQueue.swap(tempQueue);

	for (size_t i = 0; i < cardVec.size(); i++)
	{
		m_deskCardQueue.push(cardVec[i]);
	}

	// 先发庄家的牌
	SendAreaCards(m_dealerInfo.cardList);

	// 再发区域的牌
	for (size_t areaIdx = 0; areaIdx < m_areaInfoArr.size(); areaIdx++)
	{
		AreaInfo& areaInfo = m_areaInfoArr[areaIdx];
		SendAreaCards(areaInfo.cardList);
	}

	m_gameStatus = GAME_STATUS_SENDCARD;

	//////////////////////////奖值////////////////////////////////////////////
	//执行奖值结果
	//机器人奖池控制
	//获取平台奖池数据，确定本局控制概率
	if (!m_bySuperSetType && m_pDataManage->LoadPoolData())
	{
		AiWinAutoCtrl();
	}

	///////////////////////////超端///////////////////////////////////////////////
	//执行超端设置
	SuperSetChange();

	// 计算区域输赢信息
	CalcAreaRate();

	// 把牌的信息发送给所有前端
	NewGameMessageNtfSendCard msg;
	MakeSendCardInfo(msg);
	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_SENDCARD);

	// 通知前端阶段变化
	BroadcastGameStatus();

	// 发牌阶段结束之后进入比牌阶段
	SetTimer(GAME_TIMER_COMPARE, m_gameConfig.sendCardKeepTime * 1000);
}

void CNewServerManage::TransToCompareStage()
{
	m_gameStatus = GAME_STATUS_COMPARE;

	// 通知前端阶段变化
	BroadcastGameStatus();

	SetTimer(GAME_TIMER_SETTLE, m_gameConfig.compareKeepTime * 1000);
}

void CNewServerManage::TransToSettleStage()
{
	long dealerID = 0;
	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		dealerID = GetUserIDByDeskStation(m_dealerDeskStation);
		if (dealerID <= 0)
		{
			ERROR_LOG("invalid dealerID dealerID=%d, deskStation=%d，庄家不存在，庄家自动变成系统庄家", dealerID, m_dealerDeskStation);

			// 容错处理，庄家自动变成系统庄家
			m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
		}
	}

	m_gameStatus = GAME_STATUS_SETTLE;

	long long winMoneyList[PLAY_COUNT] = { 0 };
	memset(winMoneyList, 0, sizeof(winMoneyList));

	// 计算输赢
	BYTE maxWinnerDeskStation = INVALID_DESKSTATION;
	long long maxWinMoney = 0;
	long long dealerWinMoney = 0;
	for (size_t i = 0; i < m_userInfoVec.size(); i++)
	{
		BrnnUserInfo& userInfo = m_userInfoVec[i];

		//缓存上次下注信息
		for (int idx = 0; idx < MAX_AREA_COUNT; idx++)
		{
			userInfo.lastNoteList[idx] = userInfo.noteList[idx];
		}

		if (userInfo.isNote == false || i == m_dealerDeskStation)
		{
			// 没下注或者为庄家
			continue;
		}

		long long winMoney = 0;
		for (size_t areaIdx = 0; areaIdx < m_areaInfoArr.size(); areaIdx++)
		{
			long long noteMoney = userInfo.noteList[areaIdx];
			if (noteMoney > 0)
			{
				//  下注了
				int areaRate = m_areaInfoArr[areaIdx].currWinRate;
				winMoney += noteMoney * areaRate;
			}
		}

		if (winMoney > maxWinMoney)
		{
			maxWinnerDeskStation = (BYTE)i;
			maxWinMoney = winMoney;
		}

		winMoneyList[i] = winMoney;
		dealerWinMoney -= winMoney;
	}

	// 庄家
	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		if (m_dealerDeskStation < m_userInfoVec.size())
		{
			winMoneyList[m_dealerDeskStation] = dealerWinMoney;
		}
	}

	//计算抽水
	long long UserPumpGold[PLAY_COUNT] = { 0 };
	memset(UserPumpGold, 0, sizeof(UserPumpGold));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (winMoneyList[i] > 0)
		{
			UserPumpGold[i] = winMoneyList[i] * m_gameConfig.taxRate / 100;
			winMoneyList[i] -= UserPumpGold[i];
		}
	}
	///////////////////////////////

	// 金币变化
	ChangeUserPoint(winMoneyList, NULL, UserPumpGold);

	// 结算面板相关信息TODO
	NewGameMessageNtfSettle msg;
	m_dealerResNums += dealerWinMoney;
	msg.dealerWinMoney = m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION ? winMoneyList[m_dealerDeskStation] : dealerWinMoney;
	if (maxWinnerDeskStation != INVALID_DESKSTATION)
	{
		long maxWinnerID = GetUserIDByDeskStation(maxWinnerDeskStation);
		if (maxWinnerID > 0)
		{
			GameUserInfo* pUser = m_pDataManage->GetUser(maxWinnerID);
			if (pUser)
			{
				msg.maxWinnerInfo.userID = pUser->userID;
				memcpy(msg.maxWinnerInfo.name, pUser->name, sizeof(msg.maxWinnerInfo.name));
				memcpy(msg.maxWinnerInfo.headURL, pUser->headURL, sizeof(msg.maxWinnerInfo.headURL));
				msg.maxWinnerInfo.winMoney = maxWinMoney;
			}
		}
	}

	// 获取坐下玩家的输赢情况以及金币
	for (size_t i = 0; i < m_gameDesk.size();i++)
	{
		BYTE sitDeskStation = m_gameDesk[i];
		if (sitDeskStation >= PLAY_COUNT)
		{
			continue;
		}

		msg.sitUserWinMoney[i] = winMoneyList[sitDeskStation];
		GameUserInfo* pUser = m_pDataManage->GetUser(GetUserIDByDeskStation(sitDeskStation));
		if (pUser)
		{
			msg.sitUserMoney[i] = pUser->money;
		}
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			msg.myWinMoney = winMoneyList[i];
			SendGameData(i, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_SETTLE, 0);
		}
	}

	// 调用接口
	HundredGameFinish();

	// 当前庄家还能否继续坐庄
	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		GameUserInfo* pDealer = m_pDataManage->GetUser(dealerID);
		if (!pDealer)
		{
			ERROR_LOG("GetUser failed dealerID=%d", dealerID);
			m_xiaZhuangRightNow = true;
		}

		if (pDealer && pDealer->IsOnline == false)
		{
			m_xiaZhuangRightNow = true;
		}
		bool IsRandZhuang = false;
		if (m_gameConfig.randBankerCount && IsVirtual(m_dealerDeskStation))
		{
			if (rand() %100 < m_gameConfig.randCount)
			{
				IsRandZhuang = true;
			}
		}
		if (IsRandZhuang)
		{
			if (m_currDealerGameCount >= rand() % m_gameConfig.maxZhuangGameCount)
			{
				m_xiaZhuangRightNow = true;
			}
		}
		else
		{
			if (m_currDealerGameCount >= m_gameConfig.maxZhuangGameCount)
			{
				m_xiaZhuangRightNow = true;
			}
		}

	}

	if (m_xiaZhuangRightNow == true)
	{
		// 下庄
		m_currDealerGameCount = 0;
		m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	}

	ReSetGameState(0);

	BroadcastGameStatus();

	//更新下坐人数限制
	UpSitPlayerCount();

	// 通知庄信息
	//BroadcastZhuangInfo();

	// 等待下局阶段
	SetTimer(GAME_TIMER_WAITNEXT, m_gameConfig.waitSettleTime * 1000);
}

void CNewServerManage::BroadcastGameStatus()
{
	NewGameMessageNtfGameStatus msg;

	msg.gameStatus = m_gameStatus;

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_GAMESTATUS);
}

void CNewServerManage::BroadcastZhuangInfo()
{
	NewGameMessageNtfZhuangInfo msg;

	MakeZhuangInfo(msg);

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_ZHUANGINFO);
}

void CNewServerManage::BrodcastNoteInfo(BYTE deskStation)
{
	NewGameMessageNtfNoteInfo msg;

	memcpy(msg.chouMaData, m_tempChouMaData, sizeof(msg.chouMaData));
	msg.chouMaDataCount = m_tempChouMaDataCount;

	int iSendSize = sizeof(msg.noteList) + sizeof(msg.chouMaDataCount) +
		msg.chouMaDataCount * sizeof(ChouMaInfo);

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (GetUserIDByDeskStation(i) <= 0)
		{
			continue;
		}

		if (m_bIsSuper[i])
		{
			for (size_t i = 0; i < m_areaInfoArr.size(); i++)
			{
				msg.noteList[i] = m_areaInfoArr[i].areaRealPeopleNote;
			}
		}
		else
		{
			for (size_t i = 0; i < m_areaInfoArr.size(); i++)
			{
				msg.noteList[i] = m_areaInfoArr[i].areaNote;
			}
		}

		SendGameData(i, &msg, iSendSize, MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_NOTEINFO, 0);
	}

	// 清理掉
	m_tempChouMaDataCount = 0;
}

bool CNewServerManage::MakeZhuangInfo(NewGameMessageNtfZhuangInfo& msg)
{
	// 系统坐庄
	msg.isSystemZhuang = IsHundredGameSystemDealer();
	msg.zhuangInfo.userID = GetUserIDByDeskStation(m_dealerDeskStation);
	msg.zhuangInfo.money = m_dealerResNums;
	strcpy(msg.zhuangInfo.name, m_dealerName);
	strcpy(msg.zhuangInfo.headURL, m_dealerHeadURL);

	// 上庄列表信息
	if (m_shangZhuangList.size() > MAX_SHANGZHUANG_COUNT)
	{
		ERROR_LOG("shang zhuang list is overflow");
		return false;
	}

	int idx = 0;
	auto iter = m_shangZhuangList.begin();
	for (; iter != m_shangZhuangList.end(); iter++)
	{
		long userID = iter->userID;
		GameUserInfo* pUser = m_pDataManage->GetUser(userID);
		if (pUser)
		{
			UserInfo& userInfo = msg.shangZhuangList[idx];
			userInfo.userID = userID;
			userInfo.money = iter->money;
			memcpy(userInfo.name, pUser->name, sizeof(userInfo.name));
			memcpy(userInfo.headURL, pUser->headURL, sizeof(userInfo.headURL));
			idx++;
		}
	}

	return true;
}

bool CNewServerManage::MakeSendCardInfo(NewGameMessageNtfSendCard& msg)
{
	memcpy(msg.zhuangCardList, m_dealerInfo.cardList, sizeof(msg.zhuangCardList));
	msg.zhuangCardShape = m_logic.GetCardShape(m_dealerInfo.cardList, AREA_CARD_COUNT);

	for (size_t i = 0; i < m_areaInfoArr.size(); i++)
	{
		const AreaInfo& areaInfo = m_areaInfoArr[i];

		memcpy(msg.areasCardList[i], areaInfo.cardList, sizeof(msg.areasCardList[i]));
		msg.areasCardShapeList[i] = m_logic.GetCardShape(areaInfo.cardList, AREA_CARD_COUNT);

		if (areaInfo.currWinRate > 0)
		{
			msg.isWin[i] = true;
		}
	}

	return true;
}

void CNewServerManage::CalcAreaRate()
{
	int dealerCardShape = m_logic.GetCardShape(m_dealerInfo.cardList, AREA_CARD_COUNT);

	for (size_t i = 0; i < m_areaInfoArr.size(); i++)
	{
		AreaInfo& areaInfo = m_areaInfoArr[i];

		int cardShape = m_logic.GetCardShape(areaInfo.cardList, AREA_CARD_COUNT);
		if (cardShape > dealerCardShape)
		{
			int rate = m_logic.GetCardShapeRate(cardShape);
			areaInfo.currWinRate = rate;
		}
		else if (cardShape < dealerCardShape)
		{
			int rate = m_logic.GetCardShapeRate(dealerCardShape);
			rate *= -1;
			areaInfo.currWinRate = rate;
		}
		else
		{
			// 牌型相等，比最大的牌点数
			int rate = m_logic.GetCardShapeRate(cardShape);
			if (m_logic.CompareCardValue(cardShape, areaInfo.cardList, AREA_CARD_COUNT, m_dealerInfo.cardList, AREA_CARD_COUNT) == false)
			{
				// 区域小于庄家
				rate *= -1;
			}
			areaInfo.currWinRate = rate;
		}
		bool isWin = areaInfo.currWinRate > 0 ? true : false;
		areaInfo.SetResult(isWin);
	}
}

int CNewServerManage::GetRandNum()
{
	if (g_iRandNum >= 1000)
	{
		g_iRandNum = 0;
	}
	srand((unsigned)time(NULL) + g_iRandNum); rand();
	g_iRandNum++;
	return rand();
}

// 更换系统名字资源
void CNewServerManage::ChangeSystemZhuangInfo()
{
	if (!IsHundredGameSystemDealer())
	{
		return;
	}

	CRedisLoader* pRedis = m_pDataManage->GetRedis();
	if (!pRedis)
	{
		return;
	}

	if (!m_pDataManage->LoadPoolData())
	{
		return;
	}

	// 设置系统名字和资源数量

	m_dealerResNums = SYSTEMMONEY;
	strcpy(m_dealerName, "系统坐庄");
	strcpy(m_dealerHeadURL, "aa");

	//if (m_pooInfo.gameWinMoney < -m_pooInfo.realPeopleFailPercent)
	//{
	//	m_dealerResNums = CUtil::GetRandRange((int)m_gameConfig.shangZhuangLimitMoney, (int)m_gameConfig.shangZhuangLimitMoney * 3);
	//}
	//else
	//{
	//	m_dealerResNums = CUtil::GetRandRange((int)m_gameConfig.shangZhuangLimitMoney, (int)m_gameConfig.shangZhuangLimitMoney * 50);
	//}

	//RobotPositionInfo positionInfo;
	//int iRobotIndex = pRedis->GetRobotInfoIndex();
	//ConfigManage()->GetRobotPositionInfo(iRobotIndex, positionInfo);
	//strcpy(m_dealerName, positionInfo.name.c_str());
	//strcpy(m_dealerHeadURL, positionInfo.headUrl.c_str());
}

// 删除上庄列表某个玩家
void CNewServerManage::DelShangZhuangUser(int userID)
{
	for (auto it = m_shangZhuangList.begin(); it != m_shangZhuangList.end(); it++)
	{
		if (it->userID == userID)
		{
			m_shangZhuangList.erase(it);

			BroadcastZhuangInfo();
			return;
		}
	}
}

// 判断某个玩家是否在上庄列表中
bool CNewServerManage::IsShangZhuangList(int userID)
{
	for (auto it = m_shangZhuangList.begin(); it != m_shangZhuangList.end(); it++)
	{
		if (it->userID == userID)
		{
			return true;
		}
	}
	return false;
}

// 清理庄家列表
void CNewServerManage::ClearShangZhuangList(int userID, BYTE deskStation)
{
	if (userID <= 0 || deskStation >= m_userInfoVec.size())
	{
		return;
	}

	// 玩家为庄
	if (deskStation == m_dealerDeskStation)
	{
		ERROR_LOG("庄家被踢,userID=%d,deskStation=%d", userID, deskStation);
	}
	else
	{
		DelShangZhuangUser(userID);
	}

	BrnnUserInfo& userInfo = m_userInfoVec[deskStation];
	userInfo.clear();
}

//////////////////////////////////////////////////////////////////////////

//最多计算次数
#define MAX_CALC_COUNT 40

///机器人输赢自动控制,
void CNewServerManage::AiWinAutoCtrl()
{
	int iCtrlType = GetCtrlRet();

	m_runCtrlCount++;

	if (iCtrlType == 0)
	{
		return;
	}

	if (iCtrlType == 1)
	{
		m_curCtrlWinCount++;
	}

	int iRateValue = 1;
	if (IsHundredGameSystemDealer())
	{
		iRateValue = 1;
	}
	else
	{
		long dealerID = GetUserIDByDeskStation(m_dealerDeskStation);
		if (dealerID <= 0)
		{
			ERROR_LOG("控制失败，庄家座位号不存在,m_dealerDeskStation = %d", m_dealerDeskStation);
			return;
		}

		GameUserInfo* pDealer = m_pDataManage->GetUser(dealerID);
		if (!pDealer)
		{
			ERROR_LOG("控制失败，庄家座位号不存在,dealerID = %d", dealerID);
			return;
		}

		if (pDealer->isVirtual) //机器人坐庄
		{
			iRateValue = 1;
		}
		else
		{
			iRateValue = -1;
		}
	}

	if (iCtrlType == 1)
	{
		if (CalcSystemWinMoney(iRateValue) >= 0)
		{
			return;
		}
	}
	else
	{
		if (CalcSystemWinMoney(iRateValue) <= 0)
		{
			return;
		}
	}

	BYTE byAllCard[MAX_AREA_COUNT + 1][AREA_CARD_COUNT] = { 0 };
	memcpy(byAllCard[0], m_dealerInfo.cardList, sizeof(byAllCard[0]));
	memcpy(byAllCard[1], m_areaInfoArr[0].cardList, sizeof(byAllCard[1]));
	memcpy(byAllCard[2], m_areaInfoArr[1].cardList, sizeof(byAllCard[2]));
	memcpy(byAllCard[3], m_areaInfoArr[2].cardList, sizeof(byAllCard[3]));
	memcpy(byAllCard[4], m_areaInfoArr[3].cardList, sizeof(byAllCard[4]));

	int iWinArry[MAX_CALC_COUNT][5] = { 0 };
	long long llWinMoney[MAX_CALC_COUNT] = { 0 };
	int iCount = 0;

	GetMinWinMoney(iRateValue, byAllCard, iWinArry, llWinMoney, iCount, iCtrlType);

	//找到赢钱最小的一组组合
	int minIndex = 0;
	for (int i = 1; i < iCount; i++)
	{
		if (llWinMoney[i] < llWinMoney[minIndex])
		{
			minIndex = i;
		}
	}

	memcpy(m_dealerInfo.cardList, byAllCard[iWinArry[minIndex][0]], sizeof(byAllCard[iWinArry[minIndex][0]]));
	memcpy(m_areaInfoArr[0].cardList, byAllCard[iWinArry[minIndex][1]], sizeof(byAllCard[iWinArry[minIndex][1]]));
	memcpy(m_areaInfoArr[1].cardList, byAllCard[iWinArry[minIndex][2]], sizeof(byAllCard[iWinArry[minIndex][2]]));
	memcpy(m_areaInfoArr[2].cardList, byAllCard[iWinArry[minIndex][3]], sizeof(byAllCard[iWinArry[minIndex][3]]));
	memcpy(m_areaInfoArr[3].cardList, byAllCard[iWinArry[minIndex][4]], sizeof(byAllCard[iWinArry[minIndex][4]]));
}

int CNewServerManage::GetCtrlRet()
{
	// 按照概率执行
	if (m_pDataManage->m_rewardsPoolInfo.platformCtrlType == 1)
	{
		if (CUtil::GetRandNum() % 1000 < abs(m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent))
		{
			if (m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent > 0)
			{
				return 1;
			}
			else
			{
				return -1;
			}
		}

		return 0;
	}

	int pArray[20] = { 0 };
	int iArrayCount = 0;

	if (!m_pDataManage->GetPoolConfigInfoString("ctrl", pArray,20, iArrayCount))
	{
		return 0;
	}

	if (iArrayCount <= 0)
	{
		return 0;
	}

	if (iArrayCount % 2 != 0)
	{
		ERROR_LOG("json配置错误");
		return 0;
	}

	// 判断参数是否改变
	char strTemp[256] = "";
	for (int i = 0; i < iArrayCount; i ++)
	{
		sprintf(strTemp + strlen(strTemp), "%d,", pArray[i]);
	}
	if (strcmp(strTemp,m_szLastCtrlParam))
	{
		m_curCtrlWinCount = 0;
		m_runCtrlCount = 0;
		m_ctrlParam.clear();
		memcpy(m_szLastCtrlParam, strTemp, sizeof(m_szLastCtrlParam));

		for (int i = 0; i < iArrayCount; i+=2)
		{
			CtrlSystemParam param;
			param.winCount = pArray[i];
			param.allCount = pArray[i+1];
			if (param.allCount <= 0)
			{
				ERROR_LOG("json配置错误，总局数不能小于0");
				return 0;
			}
			m_ctrlParam.push_back(param);
		}
	}

	if (m_ctrlParam.size() <= 0)
	{
		return 0;
	}

	// 首局
	if (m_runCtrlCount == 0)
	{
		m_ctrlParamIndex = CUtil::GetRandNum() % m_ctrlParam.size();
	}

	// 本次控制完了，换个索引，重置数据
	if (m_runCtrlCount >= m_ctrlParam[m_ctrlParamIndex].allCount)
	{
		m_ctrlParamIndex = CUtil::GetRandNum() % m_ctrlParam.size();
		m_curCtrlWinCount = 0;
		m_runCtrlCount = 0;
	}

	// 将值传给平台
	char paramBuf[128] = "";
	sprintf(paramBuf, "[总共%d局,要赢%d局],目前是[第%d局,已经赢%d局]", m_ctrlParam[m_ctrlParamIndex].allCount,
		m_ctrlParam[m_ctrlParamIndex].winCount, m_runCtrlCount, m_curCtrlWinCount);
	m_ctrlParmRecordInfo = paramBuf;

	// 根据后台设置值，进行算法判断
	if (m_ctrlParam[m_ctrlParamIndex].winCount < 0) //全输
	{
		return -1;
	}
	
	if (m_ctrlParam[m_ctrlParamIndex].winCount == 0) //不控制
	{
		return 0;
	}
	
	if (m_ctrlParam[m_ctrlParamIndex].winCount >= m_ctrlParam[m_ctrlParamIndex].allCount) //全赢
	{
		return 1;
	}

	if (m_curCtrlWinCount >= m_ctrlParam[m_ctrlParamIndex].winCount) //赢的数量到达最大
	{
		return -1;
	}

	if (CUtil::GetRandNum() % (m_ctrlParam[m_ctrlParamIndex].allCount - m_runCtrlCount) < (m_ctrlParam[m_ctrlParamIndex].winCount - m_curCtrlWinCount))
	{
		return 1;
	}
	else
	{
		return -1;
	}

	return 0;
}

//得到最小赢钱
void CNewServerManage::GetMinWinMoney(int iRateValue, BYTE byAllCard[MAX_AREA_COUNT + 1][AREA_CARD_COUNT], int iWinArry[][5], long long llWinMoney[], int &iCount, int iCtrlType)
{
	for (int iIndex1 = 0; iIndex1 < MAX_AREA_COUNT + 1; iIndex1++)
	{
		memcpy(m_dealerInfo.cardList, byAllCard[iIndex1], sizeof(byAllCard[iIndex1]));

		for (int iIndex2 = 0; iIndex2 < MAX_AREA_COUNT + 1; iIndex2++)
		{
			if (iIndex2 == iIndex1)
				continue;
			memcpy(m_areaInfoArr[0].cardList, byAllCard[iIndex2], sizeof(byAllCard[iIndex2]));

			for (int iIndex3 = 0; iIndex3 < MAX_AREA_COUNT + 1; iIndex3++)
			{
				if (iIndex3 == iIndex1 || iIndex3 == iIndex2)
					continue;
				memcpy(m_areaInfoArr[1].cardList, byAllCard[iIndex3], sizeof(byAllCard[iIndex3]));

				for (int iIndex4 = 0; iIndex4 < MAX_AREA_COUNT + 1; iIndex4++)
				{
					if (iIndex4 == iIndex1 || iIndex4 == iIndex2 || iIndex4 == iIndex3)
						continue;
					memcpy(m_areaInfoArr[2].cardList, byAllCard[iIndex4], sizeof(byAllCard[iIndex4]));

					for (int iIndex5 = 0; iIndex5 < MAX_AREA_COUNT + 1; iIndex5++)
					{
						if (iIndex5 == iIndex1 || iIndex5 == iIndex2 || iIndex5 == iIndex3 || iIndex5 == iIndex4)
							continue;
						if (0 == iIndex1 && 1 == iIndex2 && 2 == iIndex3 && 3 == iIndex4 && 4 == iIndex5)
							continue;
						memcpy(m_areaInfoArr[3].cardList, byAllCard[iIndex5], sizeof(byAllCard[iIndex5]));

						long long llTempWinMoney = CalcSystemWinMoney(iRateValue);
						if (llTempWinMoney >= 0 && iCtrlType == 1 || llTempWinMoney <= 0 && iCtrlType == -1)
						{
							iWinArry[iCount][0] = iIndex1;
							iWinArry[iCount][1] = iIndex2;
							iWinArry[iCount][2] = iIndex3;
							iWinArry[iCount][3] = iIndex4;
							iWinArry[iCount][4] = iIndex5;
							llWinMoney[iCount] = llTempWinMoney;
							iCount++;
							if (iCount >= MAX_CALC_COUNT)
							{
								return;
							}
						}
					}
				}
			}
		}
	}
}

//计算当前系统赢钱
long long CNewServerManage::CalcSystemWinMoney(int iRateValue)
{
	long long llWinMoney = 0;
	int dealerCardShape = m_logic.GetCardShape(m_dealerInfo.cardList, AREA_CARD_COUNT);

	if (iRateValue == 1)
	{
		for (size_t i = 0; i < m_areaInfoArr.size(); i++)
		{
			AreaInfo& areaInfo = m_areaInfoArr[i];

			int cardShape = m_logic.GetCardShape(areaInfo.cardList, AREA_CARD_COUNT);
			if (cardShape > dealerCardShape)
			{
				int rate = m_logic.GetCardShapeRate(cardShape);
				rate *= -1;
				llWinMoney += rate * areaInfo.areaRealPeopleNote;
			}
			else if (cardShape < dealerCardShape)
			{
				int rate = m_logic.GetCardShapeRate(dealerCardShape);
				llWinMoney += rate * areaInfo.areaRealPeopleNote;
			}
			else
			{
				// 牌型相等，比最大的牌点数
				int rate = m_logic.GetCardShapeRate(cardShape);
				if (m_logic.CompareCardValue(cardShape, areaInfo.cardList, AREA_CARD_COUNT, m_dealerInfo.cardList, AREA_CARD_COUNT))
				{
					// 区域小于庄家
					rate *= -1;
				}
				llWinMoney += rate * areaInfo.areaRealPeopleNote;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < m_areaInfoArr.size(); i++)
		{
			AreaInfo& areaInfo = m_areaInfoArr[i];

			int cardShape = m_logic.GetCardShape(areaInfo.cardList, AREA_CARD_COUNT);
			if (cardShape > dealerCardShape)
			{
				int rate = m_logic.GetCardShapeRate(cardShape);
				llWinMoney += rate * (areaInfo.areaNote - areaInfo.areaRealPeopleNote);
			}
			else if (cardShape < dealerCardShape)
			{
				int rate = m_logic.GetCardShapeRate(dealerCardShape);
				rate *= -1;
				llWinMoney += rate * (areaInfo.areaNote - areaInfo.areaRealPeopleNote);
			}
			else
			{
				// 牌型相等，比最大的牌点数
				int rate = m_logic.GetCardShapeRate(cardShape);
				if (m_logic.CompareCardValue(cardShape, areaInfo.cardList, AREA_CARD_COUNT, m_dealerInfo.cardList, AREA_CARD_COUNT) == false)
				{
					// 区域小于庄家
					rate *= -1;
				}
				llWinMoney += rate * (areaInfo.areaNote - areaInfo.areaRealPeopleNote);
			}
		}
	}

	return llWinMoney;
}

bool CNewServerManage::IsHundredGameSystemDealer()
{
	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		return false;
	}
	return true;
}

bool CNewServerManage::HundredGameIsInfoChange(BYTE deskStation)
{
	if (deskStation == m_dealerDeskStation)
	{
		return false;
	}
	if (IsVirtual(deskStation))
	{
		if (IsShangZhuangList(GetUserIDByDeskStation(deskStation)))
		{
			return false;
		}

		if (IsDeskUser(deskStation))
		{
			return false;
		}
	}
	return true;
}

//玩家被踢，处理相关逻辑
void CNewServerManage::UserBeKicked(BYTE deskStation)
{
	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return;
	}

	ClearShangZhuangList(userID, deskStation);

	// 删除座位号
	DelDeskUser(deskStation);
}

bool CNewServerManage::IsPlayGame(BYTE deskStation)
{
	if (deskStation >= PLAY_COUNT)
	{
		return false;
	}
	BrnnUserInfo& userInfo = m_userInfoVec[deskStation];
	if (userInfo.isNote == false)
	{
		//庄家
		if (deskStation == m_dealerDeskStation)
		{
			if (m_gameStatus == GAME_STATUS_WAITNEXT)
			{
				m_xiaZhuangRightNow = true;
				return false;
			}
			else
			{
				m_xiaZhuangRightNow = true;
				return true;
			}
		}

		return false;
	}

	if (m_gameStatus == GAME_STATUS_WAITNEXT)
	{
		return false;
	}
	return true;
}

// 动态加载配置文件数据
void CNewServerManage::LoadDynamicConfig()
{
	//////////////////////////////////公共配置////////////////////////////////////////
	char szfile[256] = "";
	sprintf_s(szfile, sizeof(szfile), "%s%d_s.ini", CINIFile::GetAppPath().c_str(), NAME_ID);
	
	CINIFile f(szfile);
	string key = TEXT("game");
	m_gameConfig.waitBeginTime = f.GetKeyVal(key, "waitBeginTime", 10);
	m_gameConfig.noteKeepTime = f.GetKeyVal(key, "noteKeepTime", 30);
	m_gameConfig.sendCardKeepTime = f.GetKeyVal(key, "sendCardKeepTime", 8);
	m_gameConfig.compareKeepTime = f.GetKeyVal(key, "compareKeepTime", 5);
	m_gameConfig.waitSettleTime = f.GetKeyVal(key, "waitSettleTime", 3);
	m_gameConfig.maxZhuangGameCount = f.GetKeyVal(key, "maxZhuangGameCount", 1);
	m_gameConfig.taxRate = f.GetKeyVal(key, "taxRate", 5);
	m_gameConfig.randBankerCount = f.GetKeyVal(key, "randBankerCount", 1);
	m_gameConfig.randCount = f.GetKeyVal(key, "randCount", 60);
	//根据游戏人数来判断机器人坐下人数
	m_gameConfig.GamePlayCount_1 = f.GetKeyVal(key, "GamePlayCount_1", 3);
	m_gameConfig.GamePlayCount_2 = f.GetKeyVal(key, "GamePlayCount_2", 5);
	m_gameConfig.GamePlayCount_3 = f.GetKeyVal(key, "GamePlayCount_3", 10);
	if (m_gameConfig.GamePlayCount_1 < 0 || m_gameConfig.GamePlayCount_1 >= MAX_PLAYER_GRADE)
	{
		m_gameConfig.GamePlayCount_1 = 3;
	}
	if (m_gameConfig.GamePlayCount_2 <= m_gameConfig.GamePlayCount_1 || m_gameConfig.GamePlayCount_2 >= MAX_PLAYER_GRADE)
	{
		m_gameConfig.GamePlayCount_1 = 3;
		m_gameConfig.GamePlayCount_2 = 5;
	}
	if (m_gameConfig.GamePlayCount_3 <= m_gameConfig.GamePlayCount_2 || m_gameConfig.GamePlayCount_2 >= MAX_PLAYER_GRADE || m_gameConfig.GamePlayCount_3 >= MAX_PLAYER_GRADE)
	{
		m_gameConfig.GamePlayCount_1 = 3;
		m_gameConfig.GamePlayCount_2 = 5;
		m_gameConfig.GamePlayCount_3 = 10;
	}
	m_gameConfig.CanSitCount = f.GetKeyVal(key, "CanSitCount", 4);
	if (m_gameConfig.CanSitCount < 0 || m_gameConfig.CanSitCount > 10)
	{
		m_gameConfig.CanSitCount = 4;
	}
	if (m_gameConfig.randCount <=0|| m_gameConfig.randCount>100)
	{
		m_gameConfig.randCount = 60;
	}
	//重新设置座位
	int maxDeskCount = f.GetKeyVal(key, "maxDeskCount", 6);
	if (maxDeskCount != m_gameConfig.maxDeskCount)
	{
		m_gameConfig.maxDeskCount = maxDeskCount;
		m_gameDesk.resize(m_gameConfig.maxDeskCount);
		for (size_t i = 0; i < m_gameDesk.size(); i++)
		{
			m_gameDesk[i] = 255;
		}
	}
	
	//根据初中高进行加载配置
	if (GetRoomLevel() == 1)
	{
		m_gameConfig.shangZhuangLimitMoney = f.GetKeyVal(key, "shangZhuangLimitMoney1", 200000) * GetPlatformMultiple();
		m_gameConfig.sitLimitMoney = f.GetKeyVal(key, "sitLimitMoney1", 5000) * GetPlatformMultiple();
		m_gameConfig.noteList[0][0] = f.GetKeyVal(key, "noteList_1_0", 10) * GetPlatformMultiple();
		m_gameConfig.noteList[0][1] = f.GetKeyVal(key, "noteList_1_1", 20) * GetPlatformMultiple();
		m_gameConfig.noteList[0][2] = f.GetKeyVal(key, "noteList_1_2", 50) * GetPlatformMultiple();
		m_gameConfig.noteList[0][3] = f.GetKeyVal(key, "noteList_1_3", 100) * GetPlatformMultiple();
		m_gameConfig.noteList[0][4] = f.GetKeyVal(key, "noteList_1_4", 200) * GetPlatformMultiple();
		m_gameConfig.noteList[0][5] = f.GetKeyVal(key, "noteList_1_5", 400) * GetPlatformMultiple();
		m_gameConfig.allUserMaxNote = f.GetKeyVal(key, "allUserMaxNote1", 2000000) * GetPlatformMultiple();
	}
	else if (GetRoomLevel() == 2)
	{
		m_gameConfig.shangZhuangLimitMoney = f.GetKeyVal(key, "shangZhuangLimitMoney2", 500000) * GetPlatformMultiple();
		m_gameConfig.sitLimitMoney = f.GetKeyVal(key, "sitLimitMoney2", 5000) * GetPlatformMultiple();
		m_gameConfig.noteList[1][0] = f.GetKeyVal(key, "noteList_2_0", 100) * GetPlatformMultiple();
		m_gameConfig.noteList[1][1] = f.GetKeyVal(key, "noteList_2_1", 500) * GetPlatformMultiple();
		m_gameConfig.noteList[1][2] = f.GetKeyVal(key, "noteList_2_2", 1000) * GetPlatformMultiple();
		m_gameConfig.noteList[1][3] = f.GetKeyVal(key, "noteList_2_3", 2000) * GetPlatformMultiple();
		m_gameConfig.noteList[1][4] = f.GetKeyVal(key, "noteList_2_4", 5000) * GetPlatformMultiple();
		m_gameConfig.noteList[1][5] = f.GetKeyVal(key, "noteList_2_5", 10000) * GetPlatformMultiple();
		m_gameConfig.allUserMaxNote = f.GetKeyVal(key, "allUserMaxNote2", 2000000) * GetPlatformMultiple();
	}
	else if (GetRoomLevel() == 3)
	{
		m_gameConfig.shangZhuangLimitMoney = f.GetKeyVal(key, "shangZhuangLimitMoney3", 1000000) * GetPlatformMultiple();
		m_gameConfig.sitLimitMoney = f.GetKeyVal(key, "sitLimitMoney3", 5000) * GetPlatformMultiple();
		m_gameConfig.noteList[2][0] = f.GetKeyVal(key, "noteList_3_0", 500) * GetPlatformMultiple();
		m_gameConfig.noteList[2][1] = f.GetKeyVal(key, "noteList_3_1", 1000) * GetPlatformMultiple();
		m_gameConfig.noteList[2][2] = f.GetKeyVal(key, "noteList_3_2", 10000) * GetPlatformMultiple();
		m_gameConfig.noteList[2][3] = f.GetKeyVal(key, "noteList_3_3", 50000) * GetPlatformMultiple();
		m_gameConfig.noteList[2][4] = f.GetKeyVal(key, "noteList_3_4", 100000) * GetPlatformMultiple();
		m_gameConfig.noteList[2][5] = f.GetKeyVal(key, "noteList_3_5", 200000) * GetPlatformMultiple();
		m_gameConfig.allUserMaxNote = f.GetKeyVal(key, "allUserMaxNote3", 2000000) * GetPlatformMultiple();
	}

	////////////////////////////////特殊房间配置//////////////////////////////////////////
	TCHAR szKey[20];
	sprintf(szKey, "%d_%d", NAME_ID, m_pDataManage->m_InitData.uRoomID);
	key = szKey;
	m_gameConfig.waitBeginTime = f.GetKeyVal(key, "waitBeginTime", m_gameConfig.waitBeginTime);
}

//////////////////////////////////////////////////////////////////////////
//执行超端设置
void CNewServerManage::SuperSetChange()
{
	//验证超端，数据是否正确
	if (m_bySuperSetType != 1 && m_bySuperSetType != 2)
	{
		return;
	}

	if (m_bySuperSetArea > MAX_AREA_COUNT)
	{
		return;
	}

	if (m_bySuperSetArea == 0)  //设置庄家区域
	{
		BYTE destArea = 0;
		if (m_bySuperSetType == 1) //赢，找到最大的牌
		{
			for (int i = 0; i < MAX_AREA_COUNT; i++)
			{
				if (destArea == 0)
				{
					int iRet = m_logic.CompareCardShape(m_dealerInfo.cardList, AREA_CARD_COUNT, m_areaInfoArr[i].cardList, AREA_CARD_COUNT);
					if (iRet < 0)
					{
						destArea = i + 1;
					}
				}
				else
				{
					int iRet = m_logic.CompareCardShape(m_areaInfoArr[destArea - 1].cardList, AREA_CARD_COUNT, m_areaInfoArr[i].cardList, AREA_CARD_COUNT);
					if (iRet < 0)
					{
						destArea = i + 1;
					}
				}
			}

		}
		else if (m_bySuperSetType == 2) //输，找到最小的牌
		{
			for (int i = 0; i < MAX_AREA_COUNT; i++)
			{
				if (destArea == 0)
				{
					int iRet = m_logic.CompareCardShape(m_dealerInfo.cardList, AREA_CARD_COUNT, m_areaInfoArr[i].cardList, AREA_CARD_COUNT);
					if (iRet > 0)
					{
						destArea = i + 1;
					}
				}
				else
				{
					int iRet = m_logic.CompareCardShape(m_areaInfoArr[destArea - 1].cardList, AREA_CARD_COUNT, m_areaInfoArr[i].cardList, AREA_CARD_COUNT);
					if (iRet > 0)
					{
						destArea = i + 1;
					}
				}
			}
		}

		if (destArea > MAX_AREA_COUNT)
		{
			ERROR_LOG("超端设置错误，找不到目标区域");
			return;
		}

		ExchangeCard(destArea, m_bySuperSetArea);
	}
	else //设置闲家区域
	{
		int iRet = m_logic.CompareCardShape(m_dealerInfo.cardList, AREA_CARD_COUNT, m_areaInfoArr[m_bySuperSetArea - 1].cardList, AREA_CARD_COUNT);
		if (iRet > 0) //闲家输
		{
			if (m_bySuperSetType == 1)
			{
				ExchangeCard(0, m_bySuperSetArea);
			}
		}
		else if (iRet < 0) //闲家赢
		{
			if (m_bySuperSetType == 2)
			{
				ExchangeCard(0, m_bySuperSetArea);
			}
		}
	}
}

//交换两个区域的牌
void CNewServerManage::ExchangeCard(BYTE area1, BYTE area2)
{
	if (area1 == area2 || area1 > MAX_AREA_COUNT || area2 > MAX_AREA_COUNT)
	{
		return;
	}

	BYTE byTempCard[AREA_CARD_COUNT] = { 0 };
	BYTE byTempCardNew[AREA_CARD_COUNT] = { 0 };
	if (area1 == 0)
	{
		memcpy(byTempCard, m_dealerInfo.cardList, sizeof(byTempCard));
		memcpy(m_dealerInfo.cardList, m_areaInfoArr[area2 - 1].cardList, sizeof(m_dealerInfo.cardList));
		memcpy(m_areaInfoArr[area2 - 1].cardList, byTempCard, sizeof(m_areaInfoArr[area2 - 1].cardList));
	}
	else
	{
		if (area2 == 0)
		{
			memcpy(byTempCard, m_dealerInfo.cardList, sizeof(byTempCard));
			memcpy(m_dealerInfo.cardList, m_areaInfoArr[area1 - 1].cardList, sizeof(m_dealerInfo.cardList));
			memcpy(m_areaInfoArr[area1 - 1].cardList, byTempCard, sizeof(m_areaInfoArr[area1 - 1].cardList));
		}
		else
		{
			memcpy(byTempCard, m_areaInfoArr[area1 - 1].cardList, sizeof(byTempCard));
			memcpy(m_areaInfoArr[area1 - 1].cardList, m_areaInfoArr[area2 - 1].cardList, sizeof(m_areaInfoArr[area1 - 1].cardList));
			memcpy(m_areaInfoArr[area2 - 1].cardList, byTempCard, sizeof(m_areaInfoArr[area2 - 1].cardList));
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool CNewServerManage::IsDeskUser(BYTE deskStation)
{
	if (deskStation == 255)
	{
		return false;
	}

	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		if (m_gameDesk[i] == deskStation)
		{
			return true;
		}
	}

	return false;
}

bool CNewServerManage::DelDeskUser(BYTE deskStation)
{
	if (deskStation == 255)
	{
		return false;
	}

	int iIndex = -1;
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		if (m_gameDesk[i] == deskStation)
		{
			iIndex = (int)i;
			break;
		}
	}

	if (iIndex == -1)
	{
		return false;
	}

	m_gameDesk[iIndex] = 255;

	// 推送消息
	NewGameMessageNtfUserSit msg;
	msg.deskStation = deskStation;
	msg.index = iIndex;
	msg.type = 1;
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		msg.deskUser[i] = m_gameDesk[i];
	}

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_SIT);

	return true;
}

bool CNewServerManage::SetDeskUser(BYTE deskStation, BYTE byIndex)
{
	if (deskStation == 255 || byIndex >= m_gameDesk.size())
	{
		return false;
	}

	m_gameDesk[byIndex] = deskStation;

	// 推送消息
	NewGameMessageNtfUserSit msg;
	msg.deskStation = deskStation;
	msg.index = byIndex;
	msg.type = 0;
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		msg.deskUser[i] = m_gameDesk[i];
	}

	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_SIT);

	return true;
}

bool CNewServerManage::IsDeskHaveUser(BYTE byIndex)
{
	if (byIndex >= m_gameDesk.size())
	{
		return false;
	}

	if (m_gameDesk[byIndex] != 255)
	{
		return true;
	}

	return false;
}

void CNewServerManage::ClearDeskUser()
{
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		if (m_gameDesk[i] == 255)
		{
			continue;
		}
		long userID = GetUserIDByDeskStation(m_gameDesk[i]);
		if (userID <= 0)
		{
			DelDeskUser(m_gameDesk[i]);
			continue;
		}

		if (m_pDataManage->GetUser(userID) == NULL)
		{
			DelDeskUser(m_gameDesk[i]);
		}
	}
}

//更新上座人数
void CNewServerManage::UpSitPlayerCount()
{
	BYTE UserCount = GetRealPeople();
	if (UserCount > 0 && UserCount <= m_gameConfig.GamePlayCount_1)
	{
		m_gameConfig.CanSitCount = 4;
	}
	else if (UserCount > m_gameConfig.GamePlayCount_1 && UserCount <= m_gameConfig.GamePlayCount_2)
	{
		m_gameConfig.CanSitCount = 2;
	}
	else if (UserCount > m_gameConfig.GamePlayCount_3)
	{
		m_gameConfig.CanSitCount = 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// 判断机器人是否可以上下庄  0：上庄   1：下庄
bool CNewServerManage::IsVirtualCanShangXiazZhuang(BYTE deskStation, BYTE type)
{
	int iShangZhuangRobotCount = 0;
	for (auto it = m_shangZhuangList.begin(); it != m_shangZhuangList.end(); it++)
	{
		GameUserInfo * pUser = m_pDataManage->GetUser(it->userID);
		if (pUser && pUser->isVirtual)
		{
			iShangZhuangRobotCount++;
		}
	}

	int iMinCount = m_pDataManage->GetPoolConfigInfo("zMinC");
	int iMaxCount = m_pDataManage->GetPoolConfigInfo("zMaxC") + 1;

	if (type == 0) // 上庄
	{
		int iRandCount = CUtil::GetRandRange(iMinCount, iMaxCount);
		if (iRandCount <= 0)
		{
			iRandCount = CUtil::GetRandRange(1, 4);
		}

		if (iShangZhuangRobotCount >= iRandCount)
		{
			return false;
		}
	}
	else // 下庄
	{
		if (iShangZhuangRobotCount <= iMinCount)
		{
			return false;
		}
	}

	return true;
}

bool CNewServerManage::IsVirtualCanNote(BYTE deskStation, long long note)
{
	// 计算机器人总下注
	long long llAllRobotNote = 0;
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		llAllRobotNote += m_areaInfoArr[i].areaNote - m_areaInfoArr[i].areaRealPeopleNote;
	}

	// 获取占比
	int iNoteRate = m_pDataManage->GetPoolConfigInfo("noteRate");
	iNoteRate = iNoteRate <= 0 ? 8 : iNoteRate;

	if (llAllRobotNote + note > m_dealerResNums * iNoteRate / 100)
	{
		return false;
	}

	return true;
}

// 判断机器人是否可以上坐下站起  0：坐下   1：站起
bool CNewServerManage::IsVirtualCanSit(BYTE deskStation, BYTE type)
{
	int iSitRobotCount = 0;

	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		if (IsVirtual(m_gameDesk[i]))
		{
			iSitRobotCount++;
		}
	}

	int iMinCount = m_pDataManage->GetPoolConfigInfo("sMinC");
	int iMaxCount = m_pDataManage->GetPoolConfigInfo("sMaxC") + 1;

	if (type == 0) // 坐下
	{
		int iRandCount = CUtil::GetRandRange(iMinCount, iMaxCount);
		if (iRandCount <= 0)
		{
			iRandCount = CUtil::GetRandRange(1, 4);
		}

		if (iSitRobotCount >= iRandCount)
		{
			return false;
		}
	}
	else // 站起
	{
		if (iSitRobotCount <= iMinCount)
		{
			return false;
		}
	}

	return true;
}