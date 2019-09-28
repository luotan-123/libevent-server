#include "Stdafx.h"
#include "NewServerManage.h"
#include "UpgradeMessage.h"

CNewServerManage::CNewServerManage() : CGameDesk(ALL_ARGEE)
{
	m_gameStatus = 0;
	m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	m_LastDealerDeskStation = 255;
	m_LastShangzhuangMoney = 0;
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
	m_dealerInfoVec.resize(MAX_SHANGZHUANG_COUNT);

	LoadDynamicConfig();

	ReSetGameState(0);

	m_allCount = 0;
	m_longWinCount = 0;
	m_iContinueZhuang = -1;

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
	const DTUserInfo& userInfo = m_userInfoVec[deskStation];
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


	msg.curZhuangPlayCount = m_currDealerGameCount;
	msg.allZhuangPlayCount = m_gameConfig.maxZhuangGameCount;

	memset(msg.deskUser, 255, sizeof(msg.deskUser));
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		msg.deskUser[i] = m_gameDesk[i];
	}
	memcpy(msg.areaChouMaCount, m_areaChouMaCount, sizeof(msg.areaChouMaCount));

	SendGameStation(deskStation, socketID, bWatchUser, &msg, sizeof(msg));
	
	return true;
}

// 抢庄排序
bool SortShangZhuangByGold(const DealerInfo &v1, const DealerInfo &v2)
{
	return v1.shangzhuangMoney > v2.shangzhuangMoney;
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
	case MSG_GAME_REQ_CONTINUE_ZHUANG:
	{
		OnHandleRequestContinueZhuang(deskStation, pData, size);
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

	// 清除玩家的下注信息
	for (size_t i = 0; i < m_userInfoVec.size(); i++)
	{
		DTUserInfo& userInfo = m_userInfoVec[i];
		userInfo.ClearNoteInfo();
	}
	if (!m_dealerInfoVec.empty())
	{
		for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end();)
		{
			long userID = it->userID;
			if (GetUserIDByDeskStation(GetDeskStationByUserID(userID)) != userID)
			{
				it = m_dealerInfoVec.erase(it);
				continue;
			}
			GameUserInfo* pUser = m_pDataManage->GetUser(userID);
			//启动游戏，初始化桌子的时候，也调用这个接口，但是那个时候，用户未登陆，查不到用户信息的会导致错误，需要对指针进行判空操作
			if (NULL == pUser)
			{
				it = m_dealerInfoVec.erase(it);
				continue;
			}
			if (pUser->money < it->shangzhuangMoney)
			{
				it = m_dealerInfoVec.erase(it);
				continue;
			}
			if (pUser->deskStation == m_dealerDeskStation)
			{
				it++;
				continue;
			}
			if (pUser->IsOnline == false)
			{
				it = m_dealerInfoVec.erase(it);
				continue;
			}
			it++;
		}

		std::sort(m_dealerInfoVec.begin(), m_dealerInfoVec.end(), SortShangZhuangByGold);
	}

	// 清理坐下列表
	ClearDeskUser();

	return true;
}

bool CNewServerManage::UserLeftDesk(GameUserInfo* pUser)
{
	if (!pUser)
	{
		ERROR_LOG("pUser is NULL");
		return false;
	}

	if (pUser->deskStation >= m_userInfoVec.size())
	{
		ERROR_LOG("user deskStation is invalid deskStation=%d", pUser->deskStation);
		return false;
	}

	bool bKickShangZhuangUser = false;

	// 玩家为庄
	if (pUser->deskStation == m_dealerDeskStation)
	{
		ERROR_LOG("庄家被踢");
	}
	else
	{
		for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
		{
			if (it->userID == pUser->userID)
			{
				it = m_dealerInfoVec.erase(it);
				bKickShangZhuangUser = true;
				break;
			}
		}
	}

	if (bKickShangZhuangUser)
	{
		std::sort(m_dealerInfoVec.begin(), m_dealerInfoVec.end(), SortShangZhuangByGold);
		BroadcastZhuangInfo();
	}

	DTUserInfo& userInfo = m_userInfoVec[pUser->deskStation];
	userInfo.Clear();

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

	GameUserInfo* pUser = m_pDataManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	if (deskStation == m_dealerDeskStation)
	{
		return false;
	}

	NewGameMessageRspShangZhuang msg;

	// 在上庄列表内无法请求上庄
	for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
	{
		if (it->userID == userID)
		{
			ERROR_LOG("user:%d is already in shangzhuang list");
			SendGameMessage(deskStation, "您已经申请了上庄，不能再次申请！", SMT_EJECT);
			return false;
		}
	}

	// 上庄列表满了无法上庄
	if (m_dealerInfoVec.size() >= MAX_SHANGZHUANG_COUNT)
	{
		SendGameMessage(deskStation, "上庄列表已满!", SMT_EJECT);
		return true;
	}

	// 身上钱不够
	if (pUser->money < pMessage->shangzhuangMoney)
	{
		SendGameMessage(deskStation, "资金不够", SMT_EJECT);
		return true;
	}

	// 钱不够无法上庄
	if (pMessage->shangzhuangMoney < m_gameConfig.shangZhuangLimitMoney)
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
	DealerInfo info;
	info.isSystem = false;
	info.userID = userID;
	info.shangzhuangMoney = pMessage->shangzhuangMoney;
	m_dealerInfoVec.push_back(info);

	// 抢庄排序
	std::sort(m_dealerInfoVec.begin(), m_dealerInfoVec.end(), SortShangZhuangByGold);

	// 通知变化
	BroadcastZhuangInfo();

	SendGameMessage(deskStation, "上庄成功！", SMT_EJECT);

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_SHANGZHUANG, 0);
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

	bool bXiaZhuang = false;
	// 只有处于上庄列表或者自己为庄家才能下庄
	for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
	{
		if (it->userID == userID)
		{
			bXiaZhuang = true;
			break;
		}
	}
	// 机器人下庄特殊判断
	if (IsVirtual(deskStation) && !IsVirtualCanShangXiazZhuang(deskStation, 1))
	{
		return false;
	}

	if (!bXiaZhuang && deskStation != m_dealerDeskStation)
	{
		return false;
	}

	if (deskStation == m_dealerDeskStation)
	{
		// 设置这局结束之后立刻让庄家下庄
		SendGameMessage(deskStation, "本局已经开始，本局结束后立刻下庄！");
		m_xiaZhuangRightNow = true;
	}
	else
	{
		// 从上庄列表中移除
		for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
		{
			if (it->userID == userID)
			{
				it = m_dealerInfoVec.erase(it);
				break;
			}
		}
		SendGameMessage(deskStation, "下庄成功！");
	}


	// 抢庄排序
	std::sort(m_dealerInfoVec.begin(), m_dealerInfoVec.end(), SortShangZhuangByGold);

	BroadcastZhuangInfo();

	SendGameData(deskStation, NULL, 0, MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_XIAZHUANG, 0);
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

	NewGameMessageRspNote msg;

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

	if ((iAllBet + note) > llUserResNums)
	{
		//身上的钱不够下注
		SendGameMessage(deskStation, "下注超过自身限制");
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

	//区域总下注值 龙
	long long llDragonAreaBet = m_areaInfoArr[0].areaNote;
	//和牌区域下注值  和
	long long llDrawAreaBet = m_areaInfoArr[1].areaNote;
	//区域总下注值 虎
	long long llTigerAreaBet = m_areaInfoArr[2].areaNote;

	int iNoteArea = pMessage->area; //下注区域
	if (iNoteArea == 0)
	{
		llDragonAreaBet += note;
	}
	else if (iNoteArea == 1)
	{
		llDrawAreaBet += note;
	}
	else
	{
		llTigerAreaBet += note;
	}

	if (pMessage->area != 1)
	{
		//下注不能超过庄家(非系统)上庄金币
		if (abs(llDragonAreaBet - llTigerAreaBet) > m_dealerInfo.shangzhuangMoney)
		{
			SendGameMessage(deskStation, "下注超过庄家限制");
			return true;
		}
	}
	else
	{
		//下注不能超过庄家(非系统)上庄金币
		if (llDrawAreaBet * 8 > m_dealerInfo.shangzhuangMoney)
		{
			SendGameMessage(deskStation, "下注超过庄家限制");
			return true;
		}
	}

	DTUserInfo& userInfo = m_userInfoVec[deskStation];
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

	//自己是庄家，无法续压
	if (deskStation == m_dealerDeskStation)
	{
		SendGameMessage(deskStation, "庄家不能续压", SMT_EJECT);
		return false;
	}

	// 下注了就不能续压
	DTUserInfo& userInfo = m_userInfoVec[deskStation];
	if (userInfo.isNote == true)
	{
		SendGameMessage(deskStation, "已经下注，不能续压", SMT_EJECT);
		return false;
	}

	// 已经续压了不能再次续压
	if (userInfo.isContinueNote == true)
	{
		SendGameMessage(deskStation, "已经续压过，不能续压", SMT_EJECT);
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
		SendGameMessage(deskStation, "上一局没有下注，不能续压", SMT_EJECT);
		return true;
	}

	//续压不能超过当前金币的百分之10
	if (iAllbet > llUserResNums)
	{
		SendGameMessage(deskStation, "续压不能超过自身金额", SMT_EJECT);
		return true;
	}

	//总注不超过庄家30%
	if ((llAllAreaBet + iAllbet) > m_dealerInfo.shangzhuangMoney * 3 / 10)
	{
		SendGameMessage(deskStation, "下注超过庄家限制", SMT_EJECT);
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

	NewGameMessageRspNote msg;

	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		msg.noteList[i] = userInfo.noteList[i];
	}

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_CONTINUE_NOTE, 0);

	//算一下要多少个筹码
	for (int i = 0; i < MAX_AREA_COUNT; i++)
	{
		if (userInfo.noteList[i]>0)
		{
			long long noteNum = userInfo.noteList[i];
			int noteIndex = MAX_NOTE_SORT - 1;
			while (noteNum>0)
			{
				if (noteNum >= GetNoteByIndex(noteIndex))
				{
					noteNum -= GetNoteByIndex(noteIndex);
					//记录筹码
					m_areaChouMaCount[i][noteIndex] ++;
					if (m_tempChouMaDataCount < MAX_CHOU_MA_COUNT && m_tempChouMaDataCount >= 0)
					{
						m_tempChouMaData[m_tempChouMaDataCount].deskStation = deskStation;
						m_tempChouMaData[m_tempChouMaDataCount].index = noteIndex;
						m_tempChouMaData[m_tempChouMaDataCount].area = i;
						m_tempChouMaData[m_tempChouMaDataCount].nums = GetNoteByIndex(noteIndex);
						m_tempChouMaDataCount++;
					}
					continue;
				}
				noteIndex--;
				if (noteIndex < 0) break;
			}
		}
	}




	BrodcastNoteInfo();

	return true;
}

bool CNewServerManage::OnHandleRequestAreaTrend(BYTE deskStation)
{
	NewGameMessageRspAreaTrend msg;

	if (m_allCount == 0)
	{
		msg.longWin = 100;
		msg.huWin = 100;
	}
	else
	{
		msg.longWin = m_longWinCount * 100 / m_allCount;
		msg.huWin = (m_allCount - m_longWinCount) * 100 / m_allCount;
	}

	int iCount = 0;

	for (auto itr = m_resultList.begin(); itr != m_resultList.end(); itr++)
	{
		msg.data[iCount].type = itr->type;
		msg.data[iCount].count = itr->count;
		iCount++;
	}

	iCount = 0;
	for (auto itr = m_resultSummaryList.begin(); itr != m_resultSummaryList.end(); itr++)
	{
		msg.result[iCount] = *itr;
		iCount++;
	}

	SendGameData(deskStation, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_RSP_AREA_TREND, 0);

	return true;
}

// 请求超端设置
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

	SendGameMessage(deskStation, "设置成功", SMT_EJECT);

	return true;
}

// 请求是否继续坐庄
bool CNewServerManage::OnHandleRequestContinueZhuang(BYTE deskStation, void* pData, int size)
{
	if (size != sizeof(NewGameMessageReqContinueZhuang))
	{
		return false;
	}

	NewGameMessageReqContinueZhuang* pMessage = (NewGameMessageReqContinueZhuang*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (m_gameStatus != GAME_STATUS_WAITNEXT && m_gameStatus != GAME_STATUS_SETTLE)
	{
		SendGameMessage(deskStation, "消息已经过时", SMT_EJECT);
		return false;
	}

	if (deskStation != m_dealerDeskStation)
	{
		SendGameMessage(deskStation, "不是庄家", SMT_EJECT);
		return false;
	}


	m_iContinueZhuang = pMessage->byType;

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

void CNewServerManage::OnTimerSendCard()
{
	KillAllTimer();
	TransToSendCardStage();
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
	bool bChangeZhuang = false;
	if (m_xiaZhuangRightNow)
	{
		bChangeZhuang = true;
	}
	else if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		//上庄列表中玩家携带上庄金币数最多的那个
		long long topDealerMoney = 0;
		if (!m_dealerInfoVec.empty())
		{
			for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
			{
				topDealerMoney = (*it).shangzhuangMoney;
				break;
			}
		}

		//玩家上庄携带金币数少于列表中的玩家金币数的时候下庄，否则继续坐庄
		if (m_currDealerGameCount >= m_gameConfig.maxZhuangGameCount && m_dealerInfo.shangzhuangMoney < topDealerMoney)
		{
			m_xiaZhuangRightNow = true;
			bChangeZhuang = true;
		}

		if (m_currDealerGameCount >= m_gameConfig.maxZhuangGameCount && m_dealerInfo.shangzhuangMoney >= topDealerMoney && m_iContinueZhuang != 1)
		{
			m_xiaZhuangRightNow = true;
			bChangeZhuang = true;
		}

		if (m_xiaZhuangRightNow == true)
		{
			m_currDealerGameCount = 0;
			m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
		}
	}

	if (m_dealerDeskStation == INVALID_DESKSTATION || IsHundredGameSystemDealer())
	{
		// 初始游戏或者系统坐庄
		if (!m_dealerInfoVec.empty())
		{
			for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
			{
				m_dealerInfo = *it;
				it = m_dealerInfoVec.erase(it);
				break;
			}
			if (m_dealerInfo.userID <= 0)
			{
				ERROR_LOG("m_shangZhuangList front an invalid userID userID=%d", m_dealerInfo.userID);
				return false;
			}

			BYTE deskStation = GetDeskStationByUserID(m_dealerInfo.userID);
			if (deskStation == INVALID_DESKSTATION)
			{
				ERROR_LOG("GetDeskStationByUserID failed userID=%d", m_dealerInfo.userID);
				return false;
			}

			m_dealerDeskStation = deskStation;
		}
		else
		{
			m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
		}
	}
	else
	{
		if (!bChangeZhuang)
		{
			return true;
		}

		// 玩家或者机器人坐庄,要上一庄家下庄后，才能切换
		if (!m_dealerInfoVec.empty())
		{
			vector<DealerInfo>::iterator it;
			for (it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
			{
				m_dealerInfo = *it;
				it = m_dealerInfoVec.erase(it);
				break;
			}

			if (m_dealerInfo.userID <= 0)
			{
				ERROR_LOG("m_shangZhuangList front an invalid userID userID=%d", m_dealerInfo.userID);
				return false;
			}

			BYTE deskStation = GetDeskStationByUserID(m_dealerInfo.userID);
			if (deskStation == INVALID_DESKSTATION)
			{
				ERROR_LOG("GetDeskStationByUserID failed userID=%d", m_dealerInfo.userID);
				return false;
			}
			m_dealerDeskStation = deskStation;
		}
		else
		{
			m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
		}
	}

	std::sort(m_dealerInfoVec.begin(), m_dealerInfoVec.end(), SortShangZhuangByGold);

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
	else if (m_gameStatus == GAME_STATUS_SENDCARD)
	{
		return m_startTime + m_gameConfig.waitBeginTime + m_gameConfig.sendCardKeepTime - currTime;
	}
	else if (m_gameStatus == GAME_STATUS_NOTE)
	{
		return m_startTime + m_gameConfig.waitBeginTime + m_gameConfig.sendCardKeepTime + m_gameConfig.noteKeepTime - currTime;
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

	BroadcastGameStatus();

	// 先开始发牌
	SetTimer(GAME_TIMER_SENDCARD, (m_gameConfig.waitBeginTime + 1) * 1000);

	// 超端确认
	m_bySuperSetType = 0;
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

void CNewServerManage::TransToSendCardStage()
{
	// 确定庄家
	if (ConfirmDealer() == false)
	{
		ERROR_LOG("ConfirmDealer failed");
		m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	}

	if (m_xiaZhuangRightNow == false && m_iContinueZhuang == 1)
	{
		m_currDealerGameCount = 0;
	}

	if (m_dealerDeskStation != m_LastDealerDeskStation)
	{
		m_currDealerGameCount = 0;
	}

	if (m_dealerDeskStation != m_LastDealerDeskStation)
	{
		m_currDealerGameCount = 0;
	}


	// 初始化庄家选择
	m_iContinueZhuang = -1;

	ChangeSystemZhuangInfo();

	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		m_currDealerGameCount++;
		DelDeskUser(m_dealerDeskStation);
	}

	GameBegin(0);
	m_xiaZhuangRightNow = false;

	// 通知庄信息
	BroadcastZhuangInfo();

	// 洗牌
	std::vector<BYTE> cardVec;
	m_logic.RandCard(cardVec, 1);

	std::queue<BYTE> tempQueue;
	m_deskCardQueue.swap(tempQueue);

	for (size_t i = 0; i < cardVec.size(); i++)
	{
		m_deskCardQueue.push(cardVec[i]);
	}

	// 发区域的牌 龙 虎
	for (size_t areaIdx = 0; areaIdx < m_areaInfoArr.size(); areaIdx++)
	{
		if (areaIdx != 1)
		{
			AreaInfo& areaInfo = m_areaInfoArr[areaIdx];
			SendAreaCards(areaInfo.cardList);
		}
	}

	m_gameStatus = GAME_STATUS_SENDCARD;

	// 通知前端阶段变化
	BroadcastGameStatus();

	// 发完牌下注
	SetTimer(GAME_TIMER_NOTE, m_gameConfig.sendCardKeepTime * 1000);
}

void CNewServerManage::TransToNoteStage()
{
	m_gameStatus = GAME_STATUS_NOTE;

	// 通知所有玩家下注开始
	BroadcastGameStatus();

	NewGameMessageHasXuYa msg;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (GetUserIDByDeskStation(i) <= 0) continue;
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
		SendGameData(i, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NIF_XUYA, 0);
	}

	// 设置发送下注信息定时器
	//SetTimer(GAME_TIMER_NOTEINFO, 200);
	m_tempChouMaDataCount = 0;

	// 下注完比牌
	SetTimer(GAME_TIMER_COMPARE, m_gameConfig.noteKeepTime * 1000);
}

void CNewServerManage::TransToCompareStage()
{
	KillTimer(GAME_TIMER_NOTEINFO);
	BrodcastNoteInfo();

	m_gameStatus = GAME_STATUS_COMPARE;

	if (!m_bySuperSetType)
	{
		//执行概率控制
		AiWinAutoCtrl();
	}

	//执行超端设置
	SuperSetChange();

	// 计算区域输赢信息
	CalcAreaRate();

	// 再发生一次牌信息
	// 把牌的信息发送给所有前端
	NewGameMessageNtfSendCard msg;
	MakeSendCardInfo(msg);
	BroadcastDeskData(&msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_SENDCARD);

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
	m_LastDealerDeskStation = m_dealerDeskStation;

	long long winMoneyList[PLAY_COUNT] = { 0 };
	memset(winMoneyList, 0, sizeof(winMoneyList));

	// 计算输赢
	BYTE maxWinnerDeskStation = INVALID_DESKSTATION;
	long long maxWinMoney = 0;
	long long dealerWinMoney = 0;
	double shurate = double(10) / 100.0;
	for (size_t i = 0; i < m_userInfoVec.size(); i++)
	{
		DTUserInfo& userInfo = m_userInfoVec[i];

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
		if (m_currWinArea != 2)
		{
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
		}
		else
		{
			//开和，则扣除龙虎区下注的10%，收取和区的8倍
			long long tmpwinMoney0 = -userInfo.noteList[0] * m_areaInfoArr[0].currWinRate;
			long long tmpwinMoney2 = -userInfo.noteList[2] * m_areaInfoArr[2].currWinRate;
			winMoney -= (long long)floor((tmpwinMoney0 + tmpwinMoney2) * shurate);
			winMoney += userInfo.noteList[1] * m_areaInfoArr[1].currWinRate; //计算压和的赌注，这里算赢			
		}

		winMoneyList[i] = winMoney;
		dealerWinMoney -= winMoney;

		if (winMoney > maxWinMoney)
		{
			maxWinnerDeskStation = (BYTE)i;
			maxWinMoney = winMoney;
		}
	}

	// 庄家
	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		if (m_dealerDeskStation < m_userInfoVec.size())
		{
			winMoneyList[m_dealerDeskStation] = dealerWinMoney;
		}
	}

	///////////////////////////////计算抽水///////////////////////////////////////////
	double rate = double(m_gameConfig.taxRate) / 100.0;
	long long llRateMoney[PLAY_COUNT] = { 0 };
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long long taxValue = 0;
		if (winMoneyList[i] == 0)
		{
			continue;
		}
		if (winMoneyList[i] > 0)
		{
			long long i64WinMoney = winMoneyList[i]; //记录玩家赢的钱
			taxValue = (long long)floor(rate * i64WinMoney);
			i64WinMoney -= taxValue;
			llRateMoney[i] += taxValue; //累加抽水金币数
			winMoneyList[i] = i64WinMoney; //玩家赢的钱，扣除掉抽水后的
		}
	}

	if (m_dealerDeskStation == SYSTEM_DEALER_DESKSTATION)
	{
		long long taxValue = 0;
		if (dealerWinMoney > 0)
		{
			taxValue = (long long)floor(rate * dealerWinMoney);
			dealerWinMoney -= taxValue;
		}
	}

	// 金币变化
	ChangeUserPoint(winMoneyList, NULL, llRateMoney);

	// 结算面板相关信息TODO
	NewGameMessageNtfSettle msg;
	if (m_dealerDeskStation != SYSTEM_DEALER_DESKSTATION)
	{
		msg.dealerWinMoney = winMoneyList[m_dealerDeskStation];
		m_dealerInfo.shangzhuangMoney += winMoneyList[m_dealerDeskStation];
	}
	else
	{
		msg.dealerWinMoney = dealerWinMoney;
		m_dealerInfo.shangzhuangMoney += dealerWinMoney;
	}
	msg.dealerMoney = m_dealerInfo.shangzhuangMoney;  //记录庄家携带金币数
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
	for (size_t i = 0; i < m_gameDesk.size(); i++)
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

	for (BYTE i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			if (m_dealerDeskStation == i && m_currDealerGameCount >= m_gameConfig.maxZhuangGameCount && m_dealerInfo.shangzhuangMoney >= m_gameConfig.shangZhuangLimitMoney)
			{
				msg.isContinueZhuang = 1;
			}
			else
			{
				msg.isContinueZhuang = 0;
			}
			msg.myWinMoney = winMoneyList[i];
			SendGameData(i, &msg, sizeof(msg), MSG_MAIN_LOADER_GAME, MSG_GAME_NTF_SETTLE, 0);
		}
	}

	// 调用接口
	HundredGameFinish();

	m_LastShangzhuangMoney = m_dealerInfo.shangzhuangMoney; //上庄携带金币数变更
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

		////上庄列表中玩家携带上庄金币数最多的那个
		//long long topDealerMoney = 0;
		//if (!m_dealerInfoVec.empty())
		//{
		//	for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
		//	{
		//		topDealerMoney = (*it).shangzhuangMoney;
		//		break;
		//	}
		//}

		////玩家上庄携带金币数少于列表中的玩家金币数的时候下庄，否则继续坐庄
		//if (m_currDealerGameCount >= m_gameConfig.maxZhuangGameCount && m_dealerInfo.shangzhuangMoney < topDealerMoney)
		//{
		//	m_xiaZhuangRightNow = true;
		//}

		//上庄携带金币数小于下限，自动下庄
		if (m_dealerInfo.shangzhuangMoney < m_gameConfig.shangZhuangLimitMoney)
		{
			m_xiaZhuangRightNow = true;
		}
	}

	if (m_xiaZhuangRightNow == true)
	{
		m_currDealerGameCount = 0;
		m_dealerDeskStation = SYSTEM_DEALER_DESKSTATION;
	}

	ReSetGameState(0);

	BroadcastGameStatus();

	// 通知庄信息
	//BroadcastZhuangInfo();

	//每局可以上座的机器人数
	UpSitPlayerCount();

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
	//当前庄家的信息
	MakeZhuangInfo(msg);
	//接下来获取庄家列表中的庄家信息
	int i = 0;
	auto iter = m_dealerInfoVec.begin();
	for (; iter != m_dealerInfoVec.end(); iter++, i++)
	{
		long userID = iter->userID;
		GameUserInfo* pDealer = m_pDataManage->GetUser(userID);
		if (pDealer)
		{
			msg.shangZhuangList[i].userID = pDealer->userID;
			msg.shangZhuangList[i].money = pDealer->money;
			memcpy(msg.shangZhuangList[i].name, pDealer->name, sizeof(msg.zhuangInfo.name));
			memcpy(msg.shangZhuangList[i].headURL, pDealer->headURL, sizeof(msg.zhuangInfo.headURL));
			msg.shangZhuangList[i].shangzhuangMoney = iter->shangzhuangMoney;
		}
	}
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
	// 庄家信息
	long dealerID = GetUserIDByDeskStation(m_dealerDeskStation);
	if (dealerID > 0)
	{
		GameUserInfo* pDealer = m_pDataManage->GetUser(dealerID);
		if (pDealer)
		{
			msg.isSystemZhuang = false;
			msg.zhuangInfo.shangzhuangMoney = m_dealerInfo.shangzhuangMoney;
			msg.zhuangInfo.userID = pDealer->userID;
			msg.zhuangInfo.money = m_dealerInfo.shangzhuangMoney;
			memcpy(msg.zhuangInfo.name, pDealer->name, sizeof(msg.zhuangInfo.name));
			memcpy(msg.zhuangInfo.headURL, pDealer->headURL, sizeof(msg.zhuangInfo.headURL));
		}
	}
	else
	{
		// 系统坐庄
		msg.isSystemZhuang = true;
		msg.zhuangInfo.money = m_dealerInfo.shangzhuangMoney;
		strcpy(msg.zhuangInfo.name, "系统坐庄");
		strcpy(msg.zhuangInfo.headURL, "aa");
		msg.zhuangInfo.shangzhuangMoney = m_dealerInfo.shangzhuangMoney;
	}

	msg.curZhuangPlayCount = m_currDealerGameCount;
	msg.allZhuangPlayCount = m_gameConfig.maxZhuangGameCount;

	return true;
}

bool CNewServerManage::MakeSendCardInfo(NewGameMessageNtfSendCard& msg)
{
	//必须下注以后的状态才有数据
	if (m_gameStatus == GAME_STATUS_NOTE || m_gameStatus == GAME_STATUS_SENDCARD)
	{
		return true;
	}

	for (size_t i = 0; i < m_areaInfoArr.size(); i++)
	{
		const AreaInfo& areaInfo = m_areaInfoArr[i];

		memcpy(msg.areasCardList[i], areaInfo.cardList, sizeof(msg.areasCardList[i]));

		if (areaInfo.currWinRate > 0)
		{
			msg.isWin[i] = true;
		}
	}

	return true;
}

void CNewServerManage::CalcAreaRate()
{
	if (m_areaInfoArr.size() >= 3)
	{
		//比最大的牌点数
		int isWin = m_logic.CompareCardValue(m_areaInfoArr[0].cardList, AREA_CARD_COUNT, m_areaInfoArr[2].cardList, AREA_CARD_COUNT);
		m_currWinArea = isWin;
		if (0 == isWin)  //虎赢
		{
			SetResult(1);
			for (size_t i = 0; i < m_areaInfoArr.size(); i++)
			{
				if (i == 2)
				{
					m_areaInfoArr[i].currWinRate = 1;
				}
				else
				{
					m_areaInfoArr[i].currWinRate = -1;
				}
			}
		}
		else if (1 == isWin) //龙赢
		{
			SetResult(0);
			for (size_t i = 0; i < m_areaInfoArr.size(); i++)
			{
				if (i == 0)
				{
					m_areaInfoArr[i].currWinRate = 1;
				}
				else
				{
					m_areaInfoArr[i].currWinRate = -1;
				}
			}
		}
		else if (2 == isWin) //和
		{
			SetResult(2);
			for (size_t i = 0; i < m_areaInfoArr.size(); i++)
			{
				if (i == 1)
				{
					m_areaInfoArr[i].currWinRate = 8;
				}
				else
				{
					m_areaInfoArr[i].currWinRate = -1;
				}
			}
		}
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

void CNewServerManage::SetResult(int result)
{
	m_allCount++;
	if (result == 0)
	{
		m_longWinCount++;
	}

	m_resultSummaryList.push_front(result);
	if (m_resultSummaryList.size() > MAX_AREA_TREND_COUNT)
	{
		m_resultSummaryList.pop_back();
	}

	if (m_resultList.size() >= MAX_AREA_TREND_COUNT)
	{
		m_resultList.pop_back();
	}

	if (m_resultList.size() == 0)
	{
		m_resultList.push_back(TrendData(result, 1));
	}
	else
	{
		TrendData &data = m_resultList.front();
		if (data.type == result)
		{
			data.count++;
		}
		else
		{
			m_resultList.push_front(TrendData(result, 1));
		}
	}
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

	long userID = GetUserIDByDeskStation(deskStation);

	for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
	{
		if (it->userID == userID)
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

void CNewServerManage::UserBeKicked(BYTE deskStation)
{
	if (deskStation >= m_userInfoVec.size())
	{
		ERROR_LOG("user deskStation is invalid deskStation=%d", deskStation);
		return;
	}

	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0)
	{
		return;
	}

	bool bKickShangZhuangUser = false;

	// 玩家为庄
	if (deskStation == m_dealerDeskStation)
	{
		ERROR_LOG("庄家被踢");
	}
	else
	{
		for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); ++it)
		{
			if (it->userID == userID)
			{
				it = m_dealerInfoVec.erase(it);
				bKickShangZhuangUser = true;
				break;
			}
		}
	}

	if (bKickShangZhuangUser)
	{
		std::sort(m_dealerInfoVec.begin(), m_dealerInfoVec.end(), SortShangZhuangByGold);
		BroadcastZhuangInfo();
	}

	DTUserInfo& userInfo = m_userInfoVec[deskStation];
	userInfo.Clear();

	// 删除座位号
	DelDeskUser(deskStation);
}

bool CNewServerManage::IsPlayGame(BYTE deskStation)
{
	if (deskStation >= m_userInfoVec.size())
	{
		return false;
	}
	DTUserInfo& userInfo = m_userInfoVec[deskStation];
	if (userInfo.isNote == false)
	{
		//庄家
		if (deskStation == m_dealerDeskStation)
		{
			return true;
		}

		return false;
	}

	return true;
}

// 动态加载配置文件数据
void CNewServerManage::LoadDynamicConfig()
{
	//////////////////////////////////公共配置////////////////////////////////////////
	CString nameID;
	nameID.Format("%d", NAME_ID);
	CINIFile f(CINIFile::GetAppPath() + nameID + "_s.ini");
	CString key = TEXT("game");
	m_gameConfig.waitBeginTime = f.GetKeyVal(key, "waitBeginTime", 5);
	m_gameConfig.noteKeepTime = f.GetKeyVal(key, "noteKeepTime", 15);
	m_gameConfig.sendCardKeepTime = f.GetKeyVal(key, "sendCardKeepTime", 2);
	m_gameConfig.compareKeepTime = f.GetKeyVal(key, "compareKeepTime", 8);
	m_gameConfig.waitSettleTime = f.GetKeyVal(key, "waitSettleTime", 3);
	m_gameConfig.maxZhuangGameCount = f.GetKeyVal(key, "maxZhuangGameCount", 5);
	m_gameConfig.taxRate = f.GetKeyVal(key, "taxRate", 5);
	m_gameConfig.randBankerCount = f.GetKeyVal(key, "randBankerCount", 1);
	m_gameConfig.randCount = f.GetKeyVal(key, "randCount", 60);
	m_gameConfig.CanSitCount = f.GetKeyVal(key, "CanSitCount", 4);
	if (m_gameConfig.CanSitCount < 0 || m_gameConfig.CanSitCount > 10)
	{
		m_gameConfig.CanSitCount = 4;
	}
	if (m_gameConfig.randCount <= 0 || m_gameConfig.randCount > 100)
	{
		m_gameConfig.randCount = 60;
	}

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
	if (m_gameConfig.GamePlayCount_3 <= m_gameConfig.GamePlayCount_2 || m_gameConfig.GamePlayCount_2 >= MAX_PLAYER_GRADE && m_gameConfig.GamePlayCount_3 >= MAX_PLAYER_GRADE)
	{
		m_gameConfig.GamePlayCount_1 = 3;
		m_gameConfig.GamePlayCount_2 = 5;
		m_gameConfig.GamePlayCount_3 = 10;
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
		m_gameConfig.shangZhuangLimitMoney = f.GetKeyVal(key, "shangZhuangLimitMoney1", 200000)* GetPlatformMultiple();
		m_gameConfig.sitLimitMoney = f.GetKeyVal(key, "sitLimitMoney1", 5000)* GetPlatformMultiple();
		m_gameConfig.noteList[0][0] = f.GetKeyVal(key, "noteList_1_0", 100)* GetPlatformMultiple();
		m_gameConfig.noteList[0][1] = f.GetKeyVal(key, "noteList_1_1", 200)* GetPlatformMultiple();
		m_gameConfig.noteList[0][2] = f.GetKeyVal(key, "noteList_1_2", 500)* GetPlatformMultiple();
		m_gameConfig.noteList[0][3] = f.GetKeyVal(key, "noteList_1_3", 1000)* GetPlatformMultiple();
		m_gameConfig.noteList[0][4] = f.GetKeyVal(key, "noteList_1_4", 2000)* GetPlatformMultiple();
		m_gameConfig.noteList[0][5] = f.GetKeyVal(key, "noteList_1_5", 4000)* GetPlatformMultiple();
		m_gameConfig.allUserMaxNote = f.GetKeyVal(key, "allUserMaxNote1", 2000000)* GetPlatformMultiple();
	}
	else if (GetRoomLevel() == 2)
	{
		m_gameConfig.shangZhuangLimitMoney = f.GetKeyVal(key, "shangZhuangLimitMoney2", 500000)* GetPlatformMultiple();
		m_gameConfig.sitLimitMoney = f.GetKeyVal(key, "sitLimitMoney2", 5000)* GetPlatformMultiple();
		m_gameConfig.noteList[1][0] = f.GetKeyVal(key, "noteList_2_0", 1000)* GetPlatformMultiple();
		m_gameConfig.noteList[1][1] = f.GetKeyVal(key, "noteList_2_1", 5000)* GetPlatformMultiple();
		m_gameConfig.noteList[1][2] = f.GetKeyVal(key, "noteList_2_2", 10000)* GetPlatformMultiple();
		m_gameConfig.noteList[1][3] = f.GetKeyVal(key, "noteList_2_3", 20000)* GetPlatformMultiple();
		m_gameConfig.noteList[1][4] = f.GetKeyVal(key, "noteList_2_4", 50000)* GetPlatformMultiple();
		m_gameConfig.noteList[1][5] = f.GetKeyVal(key, "noteList_2_5", 100000)* GetPlatformMultiple();
		m_gameConfig.allUserMaxNote = f.GetKeyVal(key, "allUserMaxNote2", 2000000)* GetPlatformMultiple();
	}
	else if (GetRoomLevel() == 3)
	{
		m_gameConfig.shangZhuangLimitMoney = f.GetKeyVal(key, "shangZhuangLimitMoney3", 1000000)* GetPlatformMultiple();
		m_gameConfig.sitLimitMoney = f.GetKeyVal(key, "sitLimitMoney3", 5000)* GetPlatformMultiple();
		m_gameConfig.noteList[2][0] = f.GetKeyVal(key, "noteList_3_0", 5000)* GetPlatformMultiple();
		m_gameConfig.noteList[2][1] = f.GetKeyVal(key, "noteList_3_1", 10000)* GetPlatformMultiple();
		m_gameConfig.noteList[2][2] = f.GetKeyVal(key, "noteList_3_2", 100000)* GetPlatformMultiple();
		m_gameConfig.noteList[2][3] = f.GetKeyVal(key, "noteList_3_3", 500000)* GetPlatformMultiple();
		m_gameConfig.noteList[2][4] = f.GetKeyVal(key, "noteList_3_4", 1000000)* GetPlatformMultiple();
		m_gameConfig.noteList[2][5] = f.GetKeyVal(key, "noteList_3_5", 2000000)* GetPlatformMultiple();
		m_gameConfig.allUserMaxNote = f.GetKeyVal(key, "allUserMaxNote3", 2000000)* GetPlatformMultiple();
	}

	////////////////////////////////特殊房间配置//////////////////////////////////////////
	TCHAR szKey[20];
	wsprintf(szKey, "%s_%d", nameID, m_pDataManage->m_InitData.uRoomID);
	key = szKey;
	m_gameConfig.waitBeginTime = f.GetKeyVal(key, "waitBeginTime", m_gameConfig.waitBeginTime);


	//重新设置座位
	m_gameDesk.resize(m_gameConfig.maxDeskCount);
	for (size_t i = 0; i < m_gameDesk.size(); i++)
	{
		m_gameDesk[i] = 255;
	}
}

void CNewServerManage::AiWinAutoCtrl()
{
	if (!m_pDataManage->LoadPoolData())
	{
		return;
	}

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

	for (int iCount = 0; iCount < 5; iCount++)
	{
		BYTE maxCardA = m_logic.GetMaxCardByNumber(m_areaInfoArr[0].cardList, AREA_CARD_COUNT);
		if (m_logic.GetCardBulk(maxCardA) == 1)
		{
			SendAreaCards(m_areaInfoArr[0].cardList);
		}
		else
		{
			break;
		}
	}

	m_countResultCardVec.clear();

	for (size_t i = 0; i < m_deskCardQueue.size(); i++)
	{
		// 改变虎区域牌
		SendAreaCards(m_areaInfoArr[2].cardList);

		if (iCtrlType == 1 && CalcSystemWinMoney(iRateValue) >= 0 || iCtrlType == -1 && CalcSystemWinMoney(iRateValue) <= 0)
		{
			m_countResultCardVec.push_back(m_areaInfoArr[2].cardList[0]);
		}
	}

	if (m_countResultCardVec.size() > 0)
	{
		m_areaInfoArr[2].cardList[0] = m_countResultCardVec[CUtil::GetRandNum() % m_countResultCardVec.size()];
	}
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

	if (!m_pDataManage->GetPoolConfigInfoString("ctrl", pArray, 20, iArrayCount))
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
	for (int i = 0; i < iArrayCount; i++)
	{
		sprintf(strTemp + strlen(strTemp), "%d,", pArray[i]);
	}
	if (strcmp(strTemp, m_szLastCtrlParam))
	{
		m_curCtrlWinCount = 0;
		m_runCtrlCount = 0;
		m_ctrlParam.clear();
		memcpy(m_szLastCtrlParam, strTemp, sizeof(m_szLastCtrlParam));

		for (int i = 0; i < iArrayCount; i += 2)
		{
			CtrlSystemParam param;
			param.winCount = pArray[i];
			param.allCount = pArray[i + 1];
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

//计算当前系统赢钱
long long CNewServerManage::CalcSystemWinMoney(int iRateValue)
{
	long long winMoney = 0;

	//比最大的牌点数
	int isWin = m_logic.CompareCardValue(m_areaInfoArr[0].cardList, AREA_CARD_COUNT, m_areaInfoArr[2].cardList, AREA_CARD_COUNT);

	if (iRateValue == 1)
	{
		if (0 == isWin)  //虎赢
		{
			winMoney += m_areaInfoArr[0].areaRealPeopleNote;
			winMoney += m_areaInfoArr[1].areaRealPeopleNote;
			winMoney -= m_areaInfoArr[2].areaRealPeopleNote;
		}
		else if (1 == isWin) //龙赢
		{
			winMoney -= m_areaInfoArr[0].areaRealPeopleNote;
			winMoney += m_areaInfoArr[1].areaRealPeopleNote;
			winMoney += m_areaInfoArr[2].areaRealPeopleNote;
		}
		else if (2 == isWin) //和
		{
			winMoney += m_areaInfoArr[0].areaRealPeopleNote;
			winMoney -= 8 * m_areaInfoArr[1].areaRealPeopleNote;
			winMoney += m_areaInfoArr[2].areaRealPeopleNote;
		}
	}
	else
	{
		if (0 == isWin)  //虎赢
		{
			winMoney += (m_areaInfoArr[0].areaNote - m_areaInfoArr[0].areaRealPeopleNote);
			winMoney += (m_areaInfoArr[1].areaNote - m_areaInfoArr[1].areaRealPeopleNote);
			winMoney -= (m_areaInfoArr[2].areaNote - m_areaInfoArr[2].areaRealPeopleNote);
		}
		else if (1 == isWin) //龙赢
		{
			winMoney -= (m_areaInfoArr[0].areaNote - m_areaInfoArr[0].areaRealPeopleNote);
			winMoney += (m_areaInfoArr[1].areaNote - m_areaInfoArr[1].areaRealPeopleNote);
			winMoney += (m_areaInfoArr[2].areaNote - m_areaInfoArr[2].areaRealPeopleNote);
		}
		else if (2 == isWin) //和
		{
			winMoney += (m_areaInfoArr[0].areaNote - m_areaInfoArr[0].areaRealPeopleNote);
			winMoney -= 8 * (m_areaInfoArr[1].areaNote - m_areaInfoArr[1].areaRealPeopleNote);
			winMoney += (m_areaInfoArr[2].areaNote - m_areaInfoArr[2].areaRealPeopleNote);
		}

		winMoney = -winMoney;
	}

	return winMoney;
}

// 更换系统名字资源
void CNewServerManage::ChangeSystemZhuangInfo()
{
	// 庄家信息
	if (m_dealerDeskStation == SYSTEM_DEALER_DESKSTATION)
	{
		m_dealerInfo.isSystem = true;
		// 系统坐庄
		if (m_dealerDeskStation != m_LastDealerDeskStation)
		{
			m_dealerInfo.shangzhuangMoney = CUtil::GetRandRange((int)m_gameConfig.shangZhuangLimitMoney, 9 * (int)m_gameConfig.shangZhuangLimitMoney);
		}
		else
		{
			if (m_dealerInfo.shangzhuangMoney < m_gameConfig.shangZhuangLimitMoney)
			{
				m_dealerInfo.shangzhuangMoney = CUtil::GetRandRange((int)m_gameConfig.shangZhuangLimitMoney, 9 * (int)m_gameConfig.shangZhuangLimitMoney);
			}
			else
			{
				m_dealerInfo.shangzhuangMoney = m_LastShangzhuangMoney;
			}
		}
	}
}

//执行超端设置
void CNewServerManage::SuperSetChange()
{
	if (!m_bySuperSetType)
	{
		return;
	}

	//比最大的牌点数
	int isWin = m_logic.CompareCardValue(m_areaInfoArr[0].cardList, AREA_CARD_COUNT, m_areaInfoArr[2].cardList, AREA_CARD_COUNT);

	if (0 == isWin && m_bySuperSetType == 2)  //虎赢
	{
		return;
	}
	else if (1 == isWin && m_bySuperSetType == 1) //龙赢
	{
		return;
	}
	else if (2 == isWin && m_bySuperSetType == 3) //和
	{
		return;
	}

	for (int iCount = 0; iCount < 5; iCount++)
	{
		BYTE maxCardA = m_logic.GetMaxCardByNumber(m_areaInfoArr[0].cardList, AREA_CARD_COUNT);
		if (m_logic.GetCardBulk(maxCardA) == 1)
		{
			SendAreaCards(m_areaInfoArr[0].cardList);
		}
		else
		{
			break;
		}
	}

	for (size_t i = 0; i < m_deskCardQueue.size(); i++)
	{
		//改变虎区域牌
		SendAreaCards(m_areaInfoArr[2].cardList);

		//比最大的牌点数
		int isWin = m_logic.CompareCardValue(m_areaInfoArr[0].cardList, AREA_CARD_COUNT, m_areaInfoArr[2].cardList, AREA_CARD_COUNT);

		if (0 == isWin && m_bySuperSetType == 2)  //虎赢
		{
			return;
		}
		else if (1 == isWin && m_bySuperSetType == 1) //龙赢
		{
			return;
		}
		else if (2 == isWin && m_bySuperSetType == 3) //和
		{
			return;
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
	for (auto it = m_dealerInfoVec.begin(); it != m_dealerInfoVec.end(); it++)
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
	iNoteRate = iNoteRate <= 0 ? 3 : iNoteRate;

	if (llAllRobotNote + note > m_LastShangzhuangMoney * iNoteRate / 100)
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

