#include "GameMainManage.h"
#include "GameDesk.h"

//////////////////////////////////////////////////////////////////////
CGameMainManage::CGameMainManage()
{
	m_uDeskCount = 0;
	m_pDesk = NULL;
	m_pDeskArray = NULL;
	m_uNameID = 0;
	m_pGameUserManage = NULL;
}

//////////////////////////////////////////////////////////////////////
CGameMainManage::~CGameMainManage()
{
	m_uNameID = 0;

	if (m_pGameUserManage)
	{
		m_pGameUserManage->Release();
	}
	SAFE_DELETE(m_pGameUserManage);
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnInit(ManageInfoStruct* pInitData, KernelInfoStruct * pKernelData)
{
	if (!pInitData || !pKernelData)
	{
		return false;
	}

	//游戏ID
	m_uNameID = pKernelData->uNameID;

	// 玩家管理对象
	m_pGameUserManage = new CGameUserManage;
	if (!m_pGameUserManage)
	{
		return false;
	}

	bool ret = m_pGameUserManage->Init();
	if (!ret)
	{
		return false;
	}

	// 恢复游戏的数据
	int roomID = GetRoomID();
	ret = m_pRedis->CleanRoomAllData(roomID);
	if (!ret)
	{
		return false;
	}

	ret = InitGameDesk(pInitData->uDeskCount, pInitData->uDeskType);
	if (!ret)
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnUnInit()
{
	m_uDeskCount = 0;

	SafeDeleteArray(m_pDesk);
	DeleteDeskObject(&m_pDeskArray);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnStart()
{
	// 设置数据库句柄
	if (m_pRedis)
	{
		m_pRedis->SetDBManage(&m_SQLDataManage);

		// 加载奖池数据
		LoadPoolData();
	}
	if (m_pRedisPHP)
	{
		m_pRedisPHP->SetDBManage(&m_SQLDataManage);
	}

	// 启动一些定时器
	SetTimer(LOADER_TIMER_SAVE_ROOM_PEOPLE_COUNT, CHECK_SAVE_ROOM_PEOPLE_COUNT * 1000);
	SetTimer(LOADER_TIMER_CHECK_REDIS_CONNECTION, CHECK_REDIS_CONNECTION_SECS * 1000);
	SetTimer(LOADER_TIMER_CHECK_INVALID_STATUS_USER, CHECK_INVALID_STATUS_USER_SECS * 1000);
	SetTimer(LOADER_TIMER_CHECK_TIMEOUT_DESK, CHECK_TIMEOUT_DESK_SECS * 1000);
	SetTimer(LOADER_TIMER_COMMON_TIMER, CHECK_COMMON_TIMER_SECS * 1000);

	if (m_InitData.iRoomSort == ROOM_SORT_HUNDRED)
	{
		SetTimer(LOADER_TIMER_HUNDRED_GAME_START, (3 + CUtil::GetRandNum() % 5) * 1000, SERVERTIMER_TYPE_SINGLE);
	}
	else if (m_InitData.iRoomSort == ROOM_SORT_SCENE)
	{
		SetTimer(LOADER_TIMER_SCENE_GAME_START, (3 + CUtil::GetRandNum() % 5) * 1000, SERVERTIMER_TYPE_SINGLE);
	}

	if (m_InitData.bCanCombineDesk && GetRoomType() == ROOM_TYPE_GOLD)
	{
		SetTimer(LOADER_TIMER_COMBINE_DESK_GAME_BENGIN, CHECK_COMBINE_DESK_GAME_BENGIN_SECS * 1000);
		m_combineUserSet.clear();
		m_combineRealUserVec.clear();
		m_combineRobotUserVec.clear();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnStop()
{
	KillAllTimer();

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnUpdate()
{
	// 加载奖池数据
	LoadPoolData();

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk == NULL)
		{
			continue;
		}

		if (pDesk->GetPlayGame())
		{
			pDesk->m_needLoadConfig = true;
		}
		else
		{
			pDesk->m_needLoadConfig = false;
			pDesk->LoadDynamicConfig();
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	if (!pNetHead)
	{
		return false;
	}

	// 统计消耗
	AUTOCOST("statistics message cost mainID: %d assistID: %d", pNetHead->uMainID, pNetHead->uAssistantID);

	int userID = pNetHead->uIdentification;
	if (userID <= 0)
	{
		ERROR_LOG("not send userID，OnSocketRead failed mainID=%d assistID=%d socketIdx:%d", pNetHead->uMainID, pNetHead->uAssistantID, uIndex);
		return false;
	}

	if (!m_pGServerConnect)
	{
		ERROR_LOG("m_pGServerConnect == NULL 无法处理数据");
		return false;
	}

	// 掉线特殊处理
	if (pNetHead->uMainID == MSG_MAIN_USER_SOCKET_CLOSE)
	{
		return OnUserSocketClose(userID, uIndex);
	}

	// 登录特殊处理
	if (pNetHead->uMainID == MSG_MAIN_LOADER_LOGON)
	{
		return OnHandleLogonMessage(pNetHead->uAssistantID, pData, uSize, uAccessIP, uIndex, userID);
	}

	// 判断内存中是否还存在该玩家，动作消息特殊处理。修改部分bug
	if (pNetHead->uMainID == MSG_MAIN_LOADER_ACTION && pNetHead->uAssistantID == MSG_ASS_LOADER_ACTION_STAND && GetUser(userID) == NULL)
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_NO_USER, 0, userID);
		return false;
	}

	switch (pNetHead->uMainID)
	{
	case MSG_MAIN_LOADER_ACTION:			//用户动作
	{
		return OnHandleActionMessage(userID, pNetHead->uAssistantID, pData, uSize);
	}
	case MSG_MAIN_LOADER_FRAME:				//框架消息
	{
		return OnHandleFrameMessage(userID, pNetHead->uAssistantID, pData, uSize);
	}
	case MSG_MAIN_LOADER_GAME:			// 游戏消息
	{
		return OnHandleGameMessage(userID, pNetHead->uAssistantID, pData, uSize);
	}
	case MSG_MAIN_LOADER_DESKDISSMISS:
	{
		return OnHandleDismissMessage(userID, pNetHead->uAssistantID, pData, uSize);
	}
	case MSG_MAIN_LOADER_VOICEANDTALK:
	{
		return OnHandleVoiceAndTalkMessage(userID, pNetHead->uAssistantID, pData, uSize);
	}
	case MSG_MAIN_LOADER_MATCH:
	{
		return OnHandleMatchMessage(userID, pNetHead->uAssistantID, pData, uSize);
	}
	default:
		break;
	}

	ERROR_LOG("unavalible mainID: %d", pNetHead->uMainID);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime)
{
	// 废弃不用改用，OnUserSocketClose
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnAsynThreadResult(AsynThreadResultLine * pResultData, void * pData, UINT uSize)
{
	if (!pResultData)
	{
		return false;
	}

	if (pResultData->uHandleKind == ASYNC_EVENT_SQL_STATEMENT)
	{
		switch (pResultData->uMsgID)
		{
		case ASYNC_MESSAGE_DATABASE_TEST:
		{
			AsyncEventMsg_Test allUser;
			allUser.ParseFromArray(pData, uSize); // or parseFromString

			for (int i = 0; i < allUser.user_size(); i++) {
				AsyncEventMsg_Test_User user = allUser.user(i); // 按索引解repeated成员
				INFO_LOG("userID=%d,name=%s,headURL=%s,money=%lld,winCount=%d",
					user.userid(), user.name().c_str(), user.headurl().c_str(), user.money(), user.wincount());
			}
		}
		break;
		}

	}
	else if (pResultData->uHandleKind == ASYNC_EVENT_HTTP)
	{
		char* pBuffer = (char*)pData;
		if (pBuffer == NULL)
		{
			ERROR_LOG("请求php失败，userID=%d,postType=%d", pResultData->uIndex, pResultData->uMsgID);
			return false;
		}

		if (strcmp(pBuffer, "0"))
		{
			ERROR_LOG("请求php失败，userID=%d,postType=%d,result=%s", pResultData->uIndex, pResultData->uMsgID, pBuffer);
			return false;
		}
	}
	else if (pResultData->uHandleKind == ASYNC_EVENT_LOG)
	{

	}

	return true;
}

//////////////////////////////////////////////////////////////////////
CGameDesk* CGameMainManage::GetDeskObject(int deskIdx)
{
	CGameDesk *pDesk = NULL;
	if (deskIdx >= 0 && deskIdx < (int)m_uDeskCount)
	{
		pDesk = *(m_pDesk + deskIdx);
	}

	return pDesk;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnTimerMessage(UINT uTimerID)
{
	AUTOCOST("（timerID: %d ）定时器耗时 ", uTimerID);

	//游戏定时器
	if (uTimerID >= TIME_START_ID)
	{
		int deskIdx = (int)((uTimerID - TIME_START_ID) / TIME_SPACE);
		if (deskIdx < (int)m_InitData.uDeskCount)
		{
			return (*(m_pDesk + deskIdx))->OnTimer((uTimerID - TIME_START_ID) % TIME_SPACE);
		}
	}

	//内部定时器
	switch (uTimerID)
	{
	case LOADER_TIMER_SAVE_ROOM_PEOPLE_COUNT:
	{
		OnSaveRoomPeopleCount();
		return true;
	}
	case LOADER_TIMER_CHECK_REDIS_CONNECTION:
	{
		CheckRedisConnection();
		return true;
	}
	case LOADER_TIMER_CHECK_INVALID_STATUS_USER:
	{
		OnTimerCheckInvalidStatusUser();
		return true;
	}
	case LOADER_TIMER_CHECK_TIMEOUT_DESK:
	{
		CheckTimeOutDesk();
		return true;
	}
	case LOADER_TIMER_COMMON_TIMER:
	{
		OnCommonTimer();
		return true;
	}
	case LOADER_TIMER_HUNDRED_GAME_START:
	{
		OnHundredGameStart();
		return true;
	}
	case LOADER_TIMER_SCENE_GAME_START:
	{
		OnSceneGameStart();
		return true;
	}
	case LOADER_TIMER_COMBINE_DESK_GAME_BENGIN:
	{
		OnCombineDeskGameBegin();
		return true;
	}
	default:
		break;
	}

	return true;
}

// 清除所有定时器
void CGameMainManage::KillAllTimer()
{
	for (unsigned int i = LOADER_TIMER_BEGIN + 1; i < LOADER_TIMER_END; i++)
	{
		KillTimer(i);
	}
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::InitGameDesk(UINT uDeskCount, UINT uDeskType)
{
	//建立游戏桌子
	UINT uDeskClassSize = 0;
	m_pDeskArray = CreateDeskObject(uDeskCount, uDeskClassSize);
	if (m_pDeskArray == NULL || uDeskClassSize == 0)
	{
		throw new CException("CGameMainManage::InitGameDesk 内存申请失败", 0x418);
	}

	//申请内存
	m_uDeskCount = uDeskCount;
	m_pDesk = new CGameDesk *[m_uDeskCount];
	if (m_pDesk == NULL)
	{
		throw new CException("CGameMainManage::InitGameDesk 内存申请失败", 0x419);
	}

	//设置指针
	for (UINT i = 0; i < m_uDeskCount; i++)
	{
		*(m_pDesk + i) = (CGameDesk *)((char *)m_pDeskArray + i*uDeskClassSize);
		(*(m_pDesk + i))->Init(i, m_KernelData.uDeskPeople, this);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//玩家掉线
bool CGameMainManage::OnUserSocketClose(int userID, UINT uSocketIndex)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		return false;
	}

	if (pUser->IsOnline == false)
	{
		ERROR_LOG("user is not online userID: %d", userID);
		return false;
	}

	pUser->IsOnline = false;
	pUser->socketIdx = -1;

#ifdef OFFLINE_CHANGE_AGREE_STATUS  //断线取消准备
	// 准备状态变为坐下
	if (pUser->playStatus == USER_STATUS_AGREE)
	{
		pUser->playStatus = USER_STATUS_SITING;
	}
#endif

	CGameDesk* pDesk = GetDeskObject(pUser->deskIdx);

	// 旁观者断线
	if (pUser->playStatus == USER_STATUS_WATCH && pDesk)
	{
		// 直接让玩家回到大厅
		return pDesk->OnWatchUserOffline(pUser->userID);
	}

	// 比赛场旁观者掉线
	if (GetRoomType() == ROOM_TYPE_MATCH && pUser->watchDeskIdx != INVALID_DESKIDX)
	{
		CGameDesk* pDeskWatch = GetDeskObject(pUser->watchDeskIdx);
		if (pDeskWatch)
		{
			pDeskWatch->MatchQuitMyDeskWatch(pUser, uSocketIndex, 0);
		}
	}

	if (pDesk)
	{
		int roomType = GetRoomType();
		if ((roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
			&& (m_pRedis && pDesk->m_iRunGameCount > 0 && pDesk->IsAllUserOffline()))
		{
			m_pRedis->SetPrivateDeskCheckTime(MAKE_DESKMIXID(m_InitData.uRoomID, pDesk->m_deskIdx), (int)time(NULL));
		}

		pDesk->UserNetCut(pUser);
	}
	else
	{
		if (m_InitData.bCanCombineDesk)
		{
			// 玩家掉线，从匹配列表中删除
			DelCombineDeskUser(userID, pUser->isVirtual);
		}

		// 走到这段的两种情况
		// 1: 房卡场房间解散但玩家还未登出
		// 2: 金币场离开桌子之后未进入下一个桌子
		DelUser(userID);
	}

	return true;

}

//////////////////////////////////////////////////////////////////////
int CGameMainManage::GetUserPlayStatus(int userID)
{
	GameUserInfo* pUser = GetUser(userID);
	if (pUser)
	{
		return pUser->playStatus;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::SetUserPlayStatus(int userID, int status)
{
	GameUserInfo* pUser = GetUser(userID);
	if (pUser)
	{
		pUser->playStatus = status;
		return true;
	}

	return false;
}

int CGameMainManage::GetOnlineUserCount()
{
	if (m_pGameUserManage)
	{
		return m_pGameUserManage->GetOnlineUserCount();
	}

	return 0;
}

bool CGameMainManage::IsCanWatch()
{
	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(m_uNameID);
	if (!pGameBaseInfo)
	{
		return false;
	}

	return pGameBaseInfo->canWatch == 1;
}

bool CGameMainManage::IsCanCombineDesk()
{
	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(m_uNameID);
	if (!pGameBaseInfo)
	{
		return false;
	}

	return pGameBaseInfo->canCombineDesk == 1;
}

bool CGameMainManage::IsMultiPeopleGame()
{
	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(m_uNameID);
	if (!pGameBaseInfo)
	{
		return false;
	}

	return pGameBaseInfo->multiPeopleGame == 1;
}

void CGameMainManage::NotifyUserRechargeMoney(int userID, long long rechargeMoney, int leftSecs)
{
	LoaderNotifyRecharge msg;
	msg.needRechargeMoney = rechargeMoney;
	msg.leftSecs = leftSecs;

	SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_RECHARGE, 0);
}

bool CGameMainManage::IsVisitorUser(int userID)
{
	UserData userData;
	if (m_pRedis->GetUserData(userID, userData) && userData.registerType == LOGON_VISITOR)
	{
		return true;
	}

	return false;
}

bool CGameMainManage::GoldRoomIsHaveDeskstation()
{
	if (m_InitData.iRoomSort == ROOM_SORT_HUNDRED)
	{
		for (int i = 0; i < (int)m_uDeskCount; i++)
		{
			CGameDesk *pGameDesk = GetDeskObject(i);
			if (pGameDesk && pGameDesk->IsCanSitDesk())
			{
				return true;
			}
		}

		return false;
	}

	return true;
}

bool CGameMainManage::LoadPoolData()
{
	if (!m_pRedis)
	{
		return false;
	}

	if (!m_pRedis->GetRewardsPoolInfo(m_InitData.uRoomID, m_rewardsPoolInfo))
	{
		ERROR_LOG("加载奖池数据失败");
		return false;
	}

	return true;
}

int CGameMainManage::GetPoolConfigInfo(const char * fieldName)
{
	if (!fieldName)
	{
		return 0;
	}

	if (m_rewardsPoolInfo.detailInfo[0] != '\0')
	{
		Json::Reader reader;
		Json::Value value;
		if (!reader.parse(m_rewardsPoolInfo.detailInfo, value))
		{
			ERROR_LOG("parse json(%s) failed", m_rewardsPoolInfo.detailInfo);
			return 0;
		}

		if (value[fieldName].isString())
		{
			return atoi(value[fieldName].asString().c_str());
		}
		else if (value[fieldName].isInt())
		{
			return value[fieldName].asInt();
		}
		else
		{
			ERROR_LOG("字段错误，fieldName=%s", fieldName);
		}
	}

	return 0;
}

bool CGameMainManage::GetPoolConfigInfoString(const char * fieldName, int * pArray, int size, int &iArrayCount)
{
	if (!fieldName || !pArray)
	{
		return false;
	}

	iArrayCount = 0;

	if (m_rewardsPoolInfo.detailInfo[0] != '\0')
	{
		Json::Reader reader;
		Json::Value value;
		if (!reader.parse(m_rewardsPoolInfo.detailInfo, value))
		{
			ERROR_LOG("parse json(%s) failed", m_rewardsPoolInfo.detailInfo);
			return false;
		}

		Json::Value jsonValue = value[fieldName];

		if (jsonValue.size() > (UINT)size)
		{
			ERROR_LOG("json过长");
			return false;
		}

		for (unsigned i = 0; i < jsonValue.size(); i++)
		{
			if (jsonValue[i].isInt())
			{
				pArray[i] = jsonValue[i].asInt();
				iArrayCount++;
			}
			else
			{
				ERROR_LOG("字段错误，fieldName=%s", fieldName);
				return false;
			}
		}

		return true;
	}

	return false;
}

bool CGameMainManage::IsOneToOnePlatform()
{
	OtherConfig otherConfig = ConfigManage()->GetOtherConfig();

	return otherConfig.byIsOneToOne == 0 ? false : true;
}

///////////////////////////////登陆相关///////////////////////////////////////
bool CGameMainManage::OnHandleLogonMessage(unsigned int assistID, void* pData, int size, unsigned int accessIP, unsigned int socketIdx, int userID)
{
	switch (assistID)
	{
		// 登录
	case MSG_ASS_LOADER_LOGON:
	{
		return OnUserRequestLogon(pData, size, accessIP, socketIdx, userID);
	}
	// 登出
	case MSG_ADD_LOADER_LOGOUT:
	{
		return OnUserRequestLogout(pData, size, accessIP, socketIdx, userID);
	}
	default:
		break;
	}

	return true;
}

///////////////////////////////子类型///////////////////////////////////////////
bool CGameMainManage::OnUserRequestLogon(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID)
{
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD)
	{
		return OnMoneyLogonLogic(pData, uSize, uAccessIP, uIndex, userID);
	}
	else if (roomType == ROOM_TYPE_MATCH)
	{
		return OnMatchLogonLogic(pData, uSize, uAccessIP, uIndex, userID);
	}
	else if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP)
	{
		return OnPrivateLogonLogic(pData, uSize, uAccessIP, uIndex, userID);
	}

	return false;
}

bool CGameMainManage::OnPrivateLogonLogic(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID)
{
	if (uSize != sizeof(LoaderRequestLogon))
	{
		return false;
	}

	LoaderRequestLogon* pMessage = (LoaderRequestLogon *)pData;
	if (!pMessage)
	{
		return false;
	}

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("登陆失败，GetUserData from redis failed: userID：%d", userID);
		return false;
	}

	if (userData.sealFinishTime != 0)
	{
		int iCurTime = (int)time(NULL);
		if (iCurTime < userData.sealFinishTime || userData.sealFinishTime < 0) //解封时间到了，解封账号
		{
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ACCOUNT_SEAL, userID);
			return false;
		}
	}

	if (!userData.isVirtual)
	{
		// 获取服务器状态
		int serverStatus = 0;
		m_pRedis->GetServerStatus(serverStatus);
		if (serverStatus == SERVER_PLATFROM_STATUS_CLOSE)
		{
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_SERVER_CLOSE, userID);
			return false;
		}
	}

	int roomID = userData.roomID;
	if (roomID != GetRoomID() || pMessage->roomID != roomID)
	{
		ERROR_LOG("roomID is not match roomID(%d) userData.roomID(%d) pMessage->roomID(%d)", GetRoomID(), roomID, pMessage->roomID);
		return false;
	}

	int deskIdx = userData.deskIdx;
	if (userData.deskIdx == INVALID_DESKIDX)
	{
		ERROR_LOG("invalid deskStation userID: %d", userID);
		return false;
	}

	CGameDesk* pDesk = GetDeskObject(deskIdx);
	if (!pDesk)
	{
		ERROR_LOG("GetDeskObject failed deskIdx(%d)", deskIdx);
		return false;
	}

	int deskMixID = roomID * MAX_ROOM_HAVE_DESK_COUNT + deskIdx;
	PrivateDeskInfo privateDeskInfo;
	if (!m_pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_NO_THIS_DESK, userID);
		m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		m_pRedis->SetUserRoomID(userID, 0);
		ERROR_LOG("invalid deskMixID %d userID %d", deskMixID, userID);
		return false;
	}

	//判断是否还有座位分配给该玩家
	if (!pDesk->IsHaveDeskStation(userID, privateDeskInfo))
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_DESK_FULL, userID);
		m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		m_pRedis->SetUserRoomID(userID, 0);
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);

	if (pUser && pUser->deskIdx != deskIdx)
	{
		ERROR_LOG("userID = %d 内存 deskIdx(%d) and redis deskIdx(%d) is not match。socketIdx = %d deskStation = %d,playStatus = %d",
			userID, pUser->deskIdx, deskIdx, pUser->socketIdx, pUser->deskStation, pUser->playStatus);

		// 内存和缓存中移除玩家数据
		if (m_pGameUserManage)
		{
			m_pGameUserManage->DelUser(userID);
		}
		pUser = NULL;
	}

	if (!pUser)
	{
		pUser = new GameUserInfo;
		pUser->userID = userID;
		pUser->deskIdx = INVALID_DESKIDX;
		m_pGameUserManage->AddUser(pUser);
	}

	// 玩家的一些数据
	pUser->IsOnline = true;
	pUser->socketIdx = uIndex;
	pUser->userStatus = userData.status;
	memcpy(pUser->name, userData.name, sizeof(pUser->name));
	memcpy(pUser->headURL, userData.headURL, sizeof(pUser->headURL));
	memcpy(pUser->longitude, userData.Lng, sizeof(pUser->longitude));
	memcpy(pUser->latitude, userData.Lat, sizeof(pUser->latitude));
	memcpy(pUser->address, userData.address, sizeof(pUser->address));
	pUser->isVirtual = userData.isVirtual;
	pUser->sex = userData.sex;
	pUser->money = userData.money;
	pUser->jewels = userData.jewels;
	memcpy(pUser->motto, userData.motto, sizeof(pUser->motto));
	strcpy(pUser->ip, userData.logonIP);

	LoaderResponseLogon msg;

	msg.isInDesk = true;
	msg.deskIdx = userData.deskIdx;
	msg.deskPasswdLen = strlen(privateDeskInfo.passwd);

	if (pUser->playStatus == USER_STATUS_DEFAULT)
	{
		if (IsCanWatch() == true)
		{
			msg.playStatus = USER_STATUS_WATCH;
		}
		else
		{
			msg.playStatus = USER_STATUS_SITING;
		}

	}
#ifdef OFFLINE_CHANGE_AGREE_STATUS  //断线取消准备
	else if (pUser->playStatus == USER_STATUS_AGREE)
	{
		msg.playStatus = USER_STATUS_SITING;
	}
#endif
	else
	{
		msg.playStatus = pUser->playStatus;
	}

	memcpy(msg.deskPasswd, privateDeskInfo.passwd, strlen(privateDeskInfo.passwd));

	m_pGServerConnect->SendData(uIndex, &msg, sizeof(msg), MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, 0, userID);

	pDesk->OnPrivateUserLogin(userID, privateDeskInfo);

	return true;
}

bool CGameMainManage::OnMoneyLogonLogic(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID)
{
	if (uSize != sizeof(LoaderRequestLogon))
	{
		ERROR_LOG("size is not match");
		return false;
	}

	LoaderRequestLogon* pMessage = (LoaderRequestLogon *)pData;
	if (!pMessage)
	{
		ERROR_LOG("message is NULL");
		return false;
	}

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("OnMoneyLogonLogic::GetUserData from redis failed: userID：%d", userID);
		return false;
	}

	if (userData.sealFinishTime != 0)
	{
		int iCurTime = (int)time(NULL);
		if (iCurTime < userData.sealFinishTime || userData.sealFinishTime < 0) //解封时间到了，解封账号
		{
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ACCOUNT_SEAL, userID);
			return false;
		}
	}

	if (!userData.isVirtual)
	{
		// 获取服务器状态
		int serverStatus = 0;
		m_pRedis->GetServerStatus(serverStatus);
		if (serverStatus == SERVER_PLATFROM_STATUS_CLOSE)
		{
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_SERVER_CLOSE, userID);
			return false;
		}
	}

	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (m_pRedis->GetRoomBaseInfo(GetRoomID(), roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(GetRoomID());
	}
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("OnMoneyLogonLogic::GetRoomBaseInfo error");
		return false;
	}

	if (pRoomBaseInfo->minPoint > 0 && userData.money < pRoomBaseInfo->minPoint)
	{
		ERROR_LOG("user money is not enough usermoney=%lld minMoney=%d", userData.money, pRoomBaseInfo->minPoint);
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ROOM_NO_MONEYREQUIRE, userID);
		return false;
	}

	if (pRoomBaseInfo->minPoint > 0 && pRoomBaseInfo->maxPoint > pRoomBaseInfo->minPoint && userData.money > pRoomBaseInfo->maxPoint)
	{
		ERROR_LOG("user money is too much usermoney=%lld maxMoney=%d", userData.money, pRoomBaseInfo->maxPoint);
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ROOM_NO_MONEYREQUIRE, userID);
		return false;
	}

	if (0 != userData.roomID && GetRoomID() != userData.roomID)
	{
		if (userData.isVirtual)
		{
			m_pRedis->SetUserRoomID(userID, 0);
			m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		}
		else
		{
			LoaderResponseLogon msg;
			msg.iRoomID = userData.roomID;
			m_pGServerConnect->SendData(uIndex, &msg, sizeof(msg), MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ROOM_EXISTING, userID);
			return false;
		}
	}

	GameUserInfo* pUser = GetUser(userID);

	if (pUser && pUser->deskIdx == INVALID_DESKIDX)
	{
		//INFO_LOG("用户桌子索引异常 uIndex=%d,userID = %d,pUser->socketIdx = %d,pUser->deskStation = %d,pUser->playStatus = %d ",
		//	uIndex, userID, pUser->socketIdx, pUser->deskStation, pUser->playStatus);

		// 内存和缓存中移除玩家数据
		if (m_pGameUserManage)
		{
			m_pGameUserManage->DelUser(userID);
		}
		pUser = NULL;
	}

	if (pUser)
	{
		if (INVALID_DESKIDX != pUser->deskIdx)
		{
			pUser->money = userData.money;
			pUser->jewels = userData.jewels;
			pUser->IsOnline = true;
			pUser->socketIdx = uIndex;
			pUser->userStatus = userData.status;
			pUser->isVirtual = userData.isVirtual;

			memcpy(pUser->longitude, userData.Lng, sizeof(pUser->longitude));
			memcpy(pUser->latitude, userData.Lat, sizeof(pUser->latitude));
			memcpy(pUser->address, userData.address, sizeof(pUser->address));
			memcpy(pUser->motto, userData.motto, sizeof(pUser->motto));

			LoaderResponseLogon msg;
			msg.isInDesk = true;
			m_pGServerConnect->SendData(uIndex, &msg, sizeof(msg), MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, 0, userID);

			CGameDesk * pGameDesk = GetDeskObject(pUser->deskIdx);
			if (pGameDesk)
			{
				pGameDesk->UserSitDesk(pUser);
			}
			else
			{
				ERROR_LOG("用户桌子不存在,userID = %d,pUser->deskIdx = %d", userID, pUser->deskIdx);
				return false;
			}
		}
		else
		{
			ERROR_LOG("用户桌子索引异常,userID = %d", userID);
			return false;
		}
	}
	else
	{
		// 预判是否还有位置给玩家坐下
		if (!userData.isVirtual && !GoldRoomIsHaveDeskstation())
		{
			ERROR_LOG("没有足够的位置坐下 userID=%d", userID);
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_DESK_FULL, userID);
			return false;
		}

		// 创建一个新玩家
		pUser = new GameUserInfo;
		if (!pUser)
		{
			ERROR_LOG("分配内存失败,userID = %d", userID);
			return false;
		}

		pUser->userID = userID;
		pUser->money = userData.money;
		pUser->jewels = userData.jewels;
		pUser->userStatus = userData.status;
		pUser->isVirtual = userData.isVirtual;
		memcpy(pUser->name, userData.name, sizeof(pUser->name));
		memcpy(pUser->headURL, userData.headURL, sizeof(pUser->headURL));
		pUser->sex = userData.sex;
		pUser->IsOnline = true;
		pUser->socketIdx = uIndex;
		memcpy(pUser->longitude, userData.Lng, sizeof(pUser->longitude));
		memcpy(pUser->latitude, userData.Lat, sizeof(pUser->latitude));
		memcpy(pUser->address, userData.address, sizeof(pUser->address));
		memcpy(pUser->motto, userData.motto, sizeof(pUser->motto));
		strcpy(pUser->ip, userData.logonIP);

		m_pGameUserManage->AddUser(pUser);

		LoaderResponseLogon msg;

		m_pGServerConnect->SendData(uIndex, &msg, sizeof(msg), MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, 0, userID);
	}

	// 记录登陆记录
	if (!userData.isVirtual)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_LOGON_LOGOUT, tableName, sizeof(tableName));

		BillManage()->WriteBill(&m_SQLDataManage, "INSERT INTO %s (userID,type,time,ip,platformType,macAddr) VALUES (%d,%d,%d,'%s',%d,'%s')",
			tableName, userID, 3, (int)time(NULL), userData.logonIP, GetRoomID(), "aa");
	}
	else
	{
		// 机器人登陆先记录redis中的roomID
		m_pRedis->SetUserRoomID(userID, GetRoomID());
	}

	return true;
}

bool CGameMainManage::OnMatchLogonLogic(void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, int userID)
{
	if (uSize != sizeof(LoaderRequestLogon))
	{
		ERROR_LOG("size is not match");
		return false;
	}

	LoaderRequestLogon* pMessage = (LoaderRequestLogon *)pData;
	if (!pMessage)
	{
		ERROR_LOG("message is NULL");
		return false;
	}

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("OnMoneyLogonLogic::GetUserData from redis failed: userID：%d", userID);
		return false;
	}

	if (userData.sealFinishTime != 0)
	{
		int iCurTime = (int)time(NULL);
		if (iCurTime < userData.sealFinishTime || userData.sealFinishTime < 0) //解封时间到了，解封账号
		{
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ACCOUNT_SEAL, userID);
			return false;
		}
	}

	if (!userData.isVirtual)
	{
		// 获取服务器状态
		int serverStatus = 0;
		m_pRedis->GetServerStatus(serverStatus);
		if (serverStatus == SERVER_PLATFROM_STATUS_CLOSE)
		{
			m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_SERVER_CLOSE, userID);
			return false;
		}
	}

	if (0 != userData.roomID && GetRoomID() != userData.roomID)
	{
		LoaderResponseLogon msg;
		msg.iRoomID = userData.roomID;
		m_pGServerConnect->SendData(uIndex, &msg, sizeof(msg), MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_ROOM_EXISTING, userID);
		return false;
	}

	//比赛场判断
	if (userData.roomID == 0)
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_NO_USER_DATA, userID);
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);

	if (pUser && pUser->deskIdx == INVALID_DESKIDX)
	{
		//INFO_LOG("用户桌子索引异常 uIndex=%d,userID = %d,pUser->socketIdx = %d,pUser->deskStation = %d,pUser->playStatus = %d ",
		//	uIndex, userID, pUser->socketIdx, pUser->deskStation, pUser->playStatus);

		// 内存和缓存中移除玩家数据
		if (m_pGameUserManage)
		{
			m_pGameUserManage->DelUser(userID);
		}
		pUser = NULL;
	}

	// 比赛场不能自动登录，必须系统拉进去
	if (pUser == NULL)
	{
		ERROR_LOG("没有报名该比赛,userID=%d", userID);
		m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		m_pRedis->SetUserRoomID(userID, 0);
		m_pRedis->SetUserMatchStatus(userID, MATCH_TYPE_NORMAL, USER_MATCH_STATUS_NORMAL);
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, ERROR_MATCH_OVER, userID);
		return false;
	}

	if (INVALID_DESKIDX != pUser->deskIdx)
	{
		pUser->money = userData.money;
		pUser->jewels = userData.jewels;
		pUser->IsOnline = true;
		pUser->socketIdx = uIndex;
		pUser->userStatus = userData.status;
		pUser->isVirtual = userData.isVirtual;

		memcpy(pUser->longitude, userData.Lng, sizeof(pUser->longitude));
		memcpy(pUser->latitude, userData.Lat, sizeof(pUser->latitude));
		memcpy(pUser->address, userData.address, sizeof(pUser->address));
		memcpy(pUser->motto, userData.motto, sizeof(pUser->motto));

		LoaderResponseLogon msg;
		msg.isInDesk = true;
		m_pGServerConnect->SendData(uIndex, &msg, sizeof(msg), MSG_MAIN_LOADER_LOGON, MSG_ASS_LOADER_LOGON, 0, userID);

		CGameDesk * pGameDesk = GetDeskObject(pUser->deskIdx);
		if (pGameDesk)
		{
			pGameDesk->UserSitDesk(pUser);
		}
		else
		{
			ERROR_LOG("用户桌子不存在,userID = %d,pUser->deskIdx = %d", userID, pUser->deskIdx);
			return false;
		}
	}
	else
	{
		ERROR_LOG("用户桌子索引异常,userID = %d", userID);
		return false;
	}

	// 记录登陆记录
	if (!userData.isVirtual)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_LOGON_LOGOUT, tableName, sizeof(tableName));

		BillManage()->WriteBill(&m_SQLDataManage, "INSERT INTO %s (userID,type,time,ip,platformType,macAddr) VALUES (%d,%d,%d,'%s',%d,'%s')",
			tableName, userID, 3, (int)time(NULL), userData.logonIP, GetRoomID(), "aa");
	}
	else
	{
		// 机器人登陆先记录redis中的roomID
		m_pRedis->SetUserRoomID(userID, GetRoomID());
	}

	return true;
}

////////////////////////////////登出相关//////////////////////////////////////////
bool CGameMainManage::OnUserRequestLogout(void* pData, UINT size, ULONG uAccessIP, UINT uIndex, int userID)
{
	// 玩家能否退出
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ADD_LOADER_LOGOUT, 0, userID);
		return false;
	}

	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		ERROR_LOG("比赛场不能登出");
		return false;
	}

	int status = pUser->playStatus;
	if (status != USER_STATUS_STAND)
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ADD_LOADER_LOGOUT, 0, userID);
		return false;
	}

	// 玩家必须不在桌子上才能登出
	if (pUser->deskIdx != INVALID_DESKIDX || pUser->deskStation != INVALID_DESKSTATION)
	{
		m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ADD_LOADER_LOGOUT, 0, userID);
		return false;
	}

	OnUserLogout(userID);

	// 发送登出成功
	m_pGServerConnect->SendData(uIndex, NULL, 0, MSG_MAIN_LOADER_LOGON, MSG_ADD_LOADER_LOGOUT, 0, userID);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleActionMessage(int userID, unsigned int assistID, void * pData, int size)
{
	switch (assistID)
	{
	case MSG_ASS_LOADER_ACTION_SIT:
	{
		return OnHandleUserRequestSit(userID, pData, size);
	}
	case MSG_ASS_LOADER_ACTION_STAND:
	{
		return OnHandleUserRequestStand(userID, pData, size);
	}
	case MSG_ASS_LOADER_ACTION_MATCH_SIT:
	{
		return OnHandleUserRequestMatchSit(userID, pData, size);
	}
	case MSG_ASS_LOADER_ROBOT_TAKEMONEY:
	{
		return OnHandleRobotRequestTakeMoney(userID, pData, size);
	}
	case MSG_ASS_LOADER_GAME_BEGIN:
	{
		return OnHandleUserRequestGameBegin(userID);
	}
	case MSG_ASS_LOADER_ACTION_COMBINE_SIT:
	{
		return OnHandleUserRequestCombineSit(userID, pData, size);
	}
	case MSG_ASS_LOADER_ACTION_CANCEL_SIT:
	{
		return OnHandleUserRequestCancelSit(userID, pData, size);
	}
	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleUserRequestSit(int userID, void* pData, int size)
{
	if (size != sizeof(LoaderRequestSit))
	{
		return false;
	}

	LoaderRequestSit* pMessage = (LoaderRequestSit *)pData;
	if (!pMessage)
	{
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	int deskIdx = pUser->deskIdx;
	CGameDesk * pDesk = GetDeskObject(deskIdx);
	if (!pDesk)
	{
		ERROR_LOG("GetDeskObject failed deskIdx:%d", deskIdx);
		return false;
	}

	// 设定选择的座位号
	pUser->choiceDeskStation = pMessage->deskStation;

	return pDesk->UserSitDesk(pUser);
}

bool CGameMainManage::OnHandleUserRequestMatchSit(int userID, void* pData, int size)
{
	if (size != sizeof(LoaderRequestMatchSit))
	{
		ERROR_LOG("size != sizeof(LoaderRequestMatchSit)");
		return false;
	}

	LoaderRequestMatchSit* pMessage = (LoaderRequestMatchSit *)pData;
	if (!pMessage)
	{
		ERROR_LOG("pMessage=null");
		return false;
	}

	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		ERROR_LOG("比赛场不能匹配坐桌");
		return false;
	}

	if (GetRoomType() != ROOM_TYPE_GOLD)
	{
		ERROR_LOG("只有金币场才能匹配坐桌，userID=%d", userID);
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	// 组桌模式不能搓桌坐下
	if (IsCanCombineDesk())
	{
		if (!pUser->isVirtual)
		{
			ERROR_LOG("玩家坐桌失败，组桌游戏不能搓桌坐下，userID=%d,name=%s", userID, pUser->name);
			SendErrNotifyMessage(userID, "组桌游戏不能搓桌坐下", SMT_EJECT);
		}
		return false;
	}

	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (m_pRedis->GetRoomBaseInfo(m_InitData.uRoomID, roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_InitData.uRoomID);
	}
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("m_InitData.uRoomID=%d不存在", m_InitData.uRoomID);
		return false;
	}

	//初始化待选桌子索引
	int iArrDeskIndex[G_CHANGEDESK_MAX_COUNT], iArrRealDeskIndex[G_CHANGEDESK_MAX_COUNT];
	int iArrDeskIndexCount = 0, iArrRealDeskIndexCount = 0;
	memset(iArrDeskIndex, INVALID_DESKIDX, sizeof(iArrDeskIndex));
	memset(iArrRealDeskIndex, INVALID_DESKIDX, sizeof(iArrRealDeskIndex));
	int iDeskIndex = INVALID_DESKIDX;

	CGameDesk * pGameDesk = NULL;
	if (INVALID_DESKIDX != pUser->deskIdx)
	{
		pGameDesk = GetDeskObject(pUser->deskIdx);
		if (pGameDesk && !pGameDesk->IsPlayGame(pUser->deskStation))
		{
			pGameDesk->UserLeftDesk(pUser);
		}
		else
		{
			ERROR_LOG("游戏中不能换桌,userID=%d", userID);
			return false;
		}
	}

	if (m_InitData.iRoomSort == ROOM_SORT_HUNDRED)
	{
		for (int i = 0; i < (int)m_uDeskCount; i++)
		{
			pGameDesk = GetDeskObject(i);
			if (pGameDesk && pGameDesk->IsCanSitDesk())
			{
				if (pUser->isVirtual)
				{
					int iRandRobotCount = CUtil::GetRandRange(GetPoolConfigInfo("minC"), GetPoolConfigInfo("maxC"));
					if (iRandRobotCount <= 0)
					{
						iRandRobotCount = CUtil::GetRandRange(5, 20);
					}

					if ((int)pGameDesk->GetRobotPeople() > iRandRobotCount)
					{
						break;
					}
				}

				iDeskIndex = i;
				break;
			}
		}
	}
	else if (m_InitData.iRoomSort == ROOM_SORT_SCENE)
	{
		int iMaxUserNum = -1;
		int iSearchCount = 0;
		for (int i = 0; i < (int)m_uDeskCount; i++)
		{
			if (iArrDeskIndexCount >= G_CHANGEDESK_MAX_COUNT || iSearchCount >= (G_CHANGEDESK_MAX_COUNT + 1))
			{
				break;
			}
			pGameDesk = GetDeskObject(i);
			if (!pGameDesk)
			{
				ERROR_LOG("GetDeskObject(%d) failed:", i);
				continue;
			}

			if (i == pMessage->deskIdx)
			{
				// 原来的桌子
				continue;
			}

			if (pGameDesk->IsCanSitDesk())
			{
				int iRealPeopleCount = pGameDesk->GetRealPeople();
				unsigned int uRobotPeopleCount = pGameDesk->GetRobotPeople();


				if (pUser->isVirtual && pRoomBaseInfo->robotCount > 0 && uRobotPeopleCount >= pRoomBaseInfo->robotCount)
				{
					iSearchCount++;
					continue;
				}

				if (iRealPeopleCount > 0)
				{
					iArrRealDeskIndex[iArrRealDeskIndexCount++] = i;
				}

				iArrDeskIndex[iArrDeskIndexCount++] = i;
			}
		}

		//随机选桌子
		if (iArrRealDeskIndexCount > 0)
		{
			iDeskIndex = iArrRealDeskIndex[CUtil::GetRandNum() % iArrRealDeskIndexCount];
		}
		else if (iArrDeskIndexCount > 0)
		{
			iDeskIndex = iArrDeskIndex[CUtil::GetRandNum() % iArrDeskIndexCount];
		}
	}
	else
	{
		int iMaxUserNum = -1;
		int iSearchCount = 0;
		for (int i = 0; i < (int)m_uDeskCount; i++)
		{
			if (iArrDeskIndexCount >= G_CHANGEDESK_MAX_COUNT || iSearchCount >= (G_CHANGEDESK_MAX_COUNT + 1))
			{
				break;
			}
			pGameDesk = GetDeskObject(i);
			if (!pGameDesk)
			{
				ERROR_LOG("GetDeskObject(%d) failed:", i);
				continue;
			}

			if (i == pMessage->deskIdx)
			{
				// 原来的桌子
				continue;
			}

			if (pGameDesk->IsCanSitDesk())
			{
				int iRealPeopleCount = pGameDesk->GetRealPeople();
				unsigned int uRobotPeopleCount = pGameDesk->GetRobotPeople();

				if (pUser->isVirtual && iRealPeopleCount <= 0 && CUtil::GetRandNum() % 100 < 85)
				{
					iSearchCount++;
					continue;
				}

				if (pUser->isVirtual && pRoomBaseInfo->robotCount > 0 && uRobotPeopleCount >= pRoomBaseInfo->robotCount)
				{
					iSearchCount++;
					continue;
				}

				if (iRealPeopleCount > 0)
				{
					iArrRealDeskIndex[iArrRealDeskIndexCount++] = i;
				}

				iArrDeskIndex[iArrDeskIndexCount++] = i;
			}
		}

		//随机选桌子
		if (iArrRealDeskIndexCount > 0)
		{
			iDeskIndex = iArrRealDeskIndex[CUtil::GetRandNum() % iArrRealDeskIndexCount];
		}
		else if (iArrDeskIndexCount > 0)
		{
			iDeskIndex = iArrDeskIndex[CUtil::GetRandNum() % iArrDeskIndexCount];
		}
	}

	if (INVALID_DESKIDX != iDeskIndex)
	{
		CGameDesk * pGameDesk = GetDeskObject(iDeskIndex);
		if (!pGameDesk)
		{
			ERROR_LOG("GetDeskObject failed deskIdx:%d", iDeskIndex);
			return false;
		}
		if (pGameDesk->UserSitDesk(pUser))
		{
			if (pGameDesk->GetRoomSort() == ROOM_SORT_NORMAL && pGameDesk->m_byBeginMode == FULL_BEGIN && pUser->deskStation != 255)
			{
				if (INVALID_DESKIDX != pMessage->deskIdx)
				{
					pGameDesk->UserAgreeGame(pUser->deskStation);
				}
			}
			return true;
		}
		else
		{
			ERROR_LOG("Fail");
			return false;
		}
	}
	else
	{
		m_pGServerConnect->SendData(pUser->socketIdx, NULL, 0, MSG_MAIN_LOADER_NOTIFY_USER, MSG_NTF_LOADER_DESK_USER_NOFIND_DESK, 0, pUser->userID);
		if (!pUser->isVirtual)
		{
			ERROR_LOG("没有给玩家配置到桌子,userID=%d", pUser->userID);
		}
		return false;
	}

	return true;
}

bool CGameMainManage::OnHandleUserRequestStand(int userID, void * pData, int size)
{
	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		ERROR_LOG("比赛场不能站起");
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return true;
	}

	// 只有在一个桌子中才能站起来
	if (pUser->deskIdx == INVALID_DESKIDX)
	{
		return false;
	}

	CGameDesk* pGameDesk = GetDeskObject(pUser->deskIdx);
	if (!pGameDesk)
	{
		ERROR_LOG("GetDeskObject failed deskIdx=%d", pUser->deskIdx);
		return false;
	}

	int roomSort = pGameDesk->GetRoomSort();
	int roomType = GetRoomType();
	if (roomSort == ROOM_SORT_NORMAL || roomSort == ROOM_SORT_SCENE)
	{
		if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
		{
			return pGameDesk->UserLeftDesk(pUser);
		}
		else
		{
			if (pGameDesk->IsPlayGame(pUser->deskStation))
			{
				return SendErrNotifyMessage(userID, "游戏中不能离开", SMT_EJECT);
			}
			else
			{
				return pGameDesk->UserLeftDesk(pUser);
			}
		}
	}
	else if (roomSort == ROOM_SORT_HUNDRED)
	{
		if (pGameDesk->IsPlayGame(pUser->deskStation))
		{
			return SendErrNotifyMessage(userID, "游戏中不能离开", SMT_EJECT);
		}
		else
		{
			return pGameDesk->UserLeftDesk(pUser);
		}
	}

	return true;
}

bool CGameMainManage::OnHandleRobotRequestTakeMoney(int userID, void* pData, int size)
{
	if (size != sizeof(_LoaderRobotTakeMoney))
	{
		ERROR_LOG("OnHandleRobotTakeMoney size match failed");
		return false;
	}

	_LoaderRobotTakeMoney* pMessage = (_LoaderRobotTakeMoney*)pData;
	if (!pMessage)
	{
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("OnHandleRobotRequestTakeMoney::GetUser(%d) failed", userID);
		return false;
	}

	if (!pUser->isVirtual || pMessage->iMoney <= 0)
	{
		return true;
	}

	//如果后台设置了金币范围直接按照后台来
	int iMinRobotHaveMoney_ = GetPoolConfigInfo("minM");
	int iMaxRobotHaveMoney_ = GetPoolConfigInfo("maxM");
	if (iMaxRobotHaveMoney_ > iMinRobotHaveMoney_ && m_InitData.iRoomSort != ROOM_SORT_HUNDRED)
	{
		pMessage->iMoney = CUtil::GetRandRange(iMinRobotHaveMoney_, iMaxRobotHaveMoney_);
	}

	//机器人从奖池中取金币
	long long _i64PoolMoney = 0;
	_i64PoolMoney = pUser->money - (long long)pMessage->iMoney;
	//m_pRedis->SetRoomPoolMoney(GetRoomID(), _i64PoolMoney, true);
	m_pRedis->SetUserMoney(userID, pMessage->iMoney);
	pUser->money = pMessage->iMoney;

	_LoaderRobotTakeMoneyRes _res;

	_res.byCode = 0;
	_res.iMoney = (int)pUser->money;

	m_pGServerConnect->SendData(pUser->socketIdx, &_res, sizeof(_res), MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ROBOT_TAKEMONEY, 0, pUser->userID);

	return true;
}

bool CGameMainManage::OnHandleUserRequestGameBegin(int userID)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	CGameDesk* pDesk = GetDeskObject(pUser->deskIdx);
	if (!pDesk)
	{
		return false;
	}

	return pDesk->OnUserRequsetGameBegin(userID);
}

bool CGameMainManage::OnHandleUserRequestCombineSit(int userID, void* pData, int size)
{
	if (size != 0)
	{
		return false;
	}

	if (m_InitData.iRoomSort == ROOM_SORT_HUNDRED || m_InitData.iRoomSort == ROOM_SORT_SCENE)
	{
		return false;
	}

	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		ERROR_LOG("比赛场不能组桌");
		return false;
	}

	if (GetRoomType() != ROOM_TYPE_GOLD)
	{
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	// 没有配置组桌
	if (!IsCanCombineDesk())
	{
		if (!pUser->isVirtual)
		{
			ERROR_LOG("没有配置组桌，组桌失败，userID=%d,name=%s", userID, pUser->name);
		}
		return false;
	}

	CGameDesk * pGameDesk = NULL;
	if (INVALID_DESKIDX != pUser->deskIdx)
	{
		pGameDesk = GetDeskObject(pUser->deskIdx);
		if (pGameDesk && !pGameDesk->IsPlayGame(pUser->deskStation))
		{
			pGameDesk->UserLeftDesk(pUser);
		}
		else
		{
			ERROR_LOG("游戏中不能组桌,userID=%d", userID);
			return false;
		}
	}

	// 添加到集合
	AddCombineDeskUser(userID, pUser->isVirtual);

	return true;
}

bool CGameMainManage::OnHandleUserRequestCancelSit(int userID, void* pData, int size)
{
	if (size != 0)
	{
		ERROR_LOG("size error");
		return false;
	}

	if (m_InitData.iRoomSort == ROOM_SORT_HUNDRED || m_InitData.iRoomSort == ROOM_SORT_SCENE)
	{
		return false;
	}

	if (GetRoomType() != ROOM_TYPE_GOLD)
	{
		ERROR_LOG("只有金币场才能匹配坐桌，userID=%d", userID);
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	// 没有配置组桌
	if (!IsCanCombineDesk())
	{
		if (!pUser->isVirtual)
		{
			ERROR_LOG("没有配置组桌，组桌失败，userID=%d,name=%s", userID, pUser->name);
		}
		return false;
	}

	if (INVALID_DESKIDX != pUser->deskIdx)
	{
		ERROR_LOG("已经在桌子内的玩家不能坐桌，userID=%d", userID);
		return false;
	}

	if (!DelCombineDeskUser(userID, pUser->isVirtual))
	{
		ERROR_LOG("排队列表中不存在该玩家,userID=%d", userID);
		return false;
	}

	m_pGServerConnect->SendData(pUser->socketIdx, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_CANCEL_SIT, 0, pUser->userID);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleFrameMessage(int userID, unsigned int assistID, void* pData, int size)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	if (pUser->deskIdx >= (int)m_uDeskCount)
	{
		return false;
	}

	// 选择桌子
	CGameDesk* pGameDesk = GetDeskObject(pUser->deskIdx);
	if (!pGameDesk)
	{
		return false;
	}

	if (pUser->deskStation == INVALID_DESKSTATION)
	{
		ERROR_LOG("user(%d) deskStation is invalid", userID);
		return false;
	}

	bool isWatch = false;
	if (pUser->playStatus == USER_STATUS_WATCH)
	{
		isWatch = true;
	}

	return pGameDesk->HandleFrameMessage(pUser->deskStation, assistID, pData, size, isWatch);
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleGameMessage(int userID, unsigned int assistID, void* pData, int size)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	CGameDesk * pGameDesk = GetDeskObject(pUser->deskIdx);
	if (!pGameDesk)
	{
		//ERROR_LOG("GetDeskObject:%d failed", pUser->deskIdx);
		return false;
	}

	if (pUser->deskStation == INVALID_DESKSTATION)
	{
		ERROR_LOG("user:%d deskStation is invalid", pUser->deskStation);
		return false;
	}

	// 记录玩家操作时间
	pUser->lastOperateTime = time(NULL);

	return pGameDesk->HandleNotifyMessage(pUser->deskStation, assistID, pData, size, pUser->playStatus == USER_STATUS_WATCH);
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleDismissMessage(int userID, unsigned int assistID, void * pData, int size)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	CGameDesk * pGameDesk = GetDeskObject(pUser->deskIdx);
	if (!pGameDesk)
	{
		return false;
	}

	return pGameDesk->HandleDissmissMessage(pUser->deskStation, assistID, pData, size);
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleVoiceAndTalkMessage(int userID, unsigned int assistID, void * pData, int size)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return false;
	}

	CGameDesk * pGameDesk = GetDeskObject(pUser->deskIdx);
	if (!pGameDesk)
	{
		return false;
	}

	switch (assistID)
	{
	case MSG_ASS_LOADER_TALK:
	{
		pGameDesk->OnHandleTalkMessage(pUser->deskStation, pData, size);
	}
	case MSG_ASS_LOADER_VOICE:
	{
		pGameDesk->OnHandleVoiceMessage(pUser->deskStation, pData, size);
	}
	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CGameMainManage::OnHandleMatchMessage(int userID, unsigned int assistID, void* pData, int size)
{
	switch (assistID)
	{
	case MSG_ASS_LOADER_MATCH_ALL_DESK_DATA:
	{
		return OnHandleMatchAllDeskStatusMessage(userID, pData, size);
	}
	case MSG_ASS_LOADER_MATCH_ENTER_WATCH_DESK:
	{
		return OnHandleMatchEnterWatchDeskMessage(userID, pData, size);
	}
	case MSG_ASS_LOADER_MATCH_QUIT_WATCH_DESK:
	{
		return OnHandleMatchQuitWatchDeskMessage(userID, pData, size);
	}
	default:
		break;
	}

	return false;
}

bool CGameMainManage::OnHandleMatchAllDeskStatusMessage(int userID, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, LoaderRequestMatchAllDeskDataInfo, pData, size);

	auto itr = m_matchGameDeskMap.find(pMessage->llPartOfMatchID);
	if (itr == m_matchGameDeskMap.end())
	{
		ERROR_LOG("没有该比赛:%lld", pMessage->llPartOfMatchID);
		return false;
	}

	auto itrUser = m_matchUserMap.find(pMessage->llPartOfMatchID);
	if (itrUser == m_matchUserMap.end())
	{
		ERROR_LOG("没有找到该比赛人数：%lld", pMessage->llPartOfMatchID);
		return false;
	}

	std::list<int> &deskList = itr->second;

	if (deskList.size() <= 0)
	{
		ERROR_LOG("比赛已经结束：%lld", pMessage->llPartOfMatchID);
		return false;
	}

	LoaderResponseMatchAllDeskDataInfo msg;
	msg.deskCount = 0;
	msg.iCurPeopleCount = deskList.size() * m_KernelData.uDeskPeople;
	msg.iMaxPeopleCount = itrUser->second.size();

	for (auto itr = deskList.begin(); itr != deskList.end(); itr++)
	{
		CGameDesk* pDesk = GetDeskObject(*itr);
		if (!pDesk)
		{
			continue;
		}

		msg.iCurMatchRound = pDesk->m_iCurMatchRound;
		msg.iMaxMatchRound = pDesk->m_iMaxMatchRound;
		msg.desk[msg.deskCount].deskIdx = pDesk->m_deskIdx;

		if (pDesk->m_bFinishMatch)
		{
			msg.desk[msg.deskCount].status = 1;
		}
		else if (pDesk->m_llStartMatchTime == 0)
		{
			msg.desk[msg.deskCount].status = 0;
		}
		else
		{
			msg.desk[msg.deskCount].status = 2;
		}

		msg.deskCount++;
	}

	int iSendSize = 20 + msg.deskCount * sizeof(LoaderResponseMatchAllDeskDataInfo::DeskInfo);
	SendData(userID, &msg, iSendSize, MSG_MAIN_LOADER_MATCH, MSG_ASS_LOADER_MATCH_ALL_DESK_DATA, 0);

	return true;
}

//旁观其它桌子
bool CGameMainManage::OnHandleMatchEnterWatchDeskMessage(int userID, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, LoaderRequestMatchEnterWatch, pData, size);

	GameUserInfo *pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("用户不存在，不能进入旁观,userID=%d", userID);
		return false;
	}

	CGameDesk* pDesk = GetDeskObject(pMessage->deskIdx);
	if (!pDesk)
	{
		ERROR_LOG("旁观的桌子不存在:%d", pMessage->deskIdx);
		return false;
	}

	pDesk->MatchEnterMyDeskWatch(pUser, pMessage->llPartOfMatchID);

	return true;
}

//退出旁观
bool CGameMainManage::OnHandleMatchQuitWatchDeskMessage(int userID, void* pData, int size)
{
	if (size != 0)
	{
		return false;
	}

	GameUserInfo *pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("用户不存在，不能退出旁观,userID=%d", userID);
		return false;
	}

	CGameDesk* pDesk = GetDeskObject(pUser->watchDeskIdx);
	if (!pDesk)
	{
		ERROR_LOG("玩家之前没有旁观其它桌子,watchDeskIdx=%d", pUser->watchDeskIdx);
		return false;
	}

	pDesk->MatchQuitMyDeskWatch(pUser, pUser->socketIdx, 1);

	return true;
}

//////////////////////////////////////////////////////////////////////
void CGameMainManage::SendData(int userID, void * pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode)
{
	if (!m_pGameUserManage)
	{
		ERROR_LOG("SendData m_pGameUserManage is NULL");
		return;
	}

	if (!m_pGServerConnect)
	{
		ERROR_LOG("SendData m_pGServerConnect is NULL");
		return;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser(%d) failed", userID);
		return;
	}

	if (!pUser->IsOnline)
	{
		return;
	}

	if (pUser->socketIdx == -1)
	{
		return;
	}

	m_pGServerConnect->SendData(pUser->socketIdx, pData, size, mainID, assistID, handleCode, userID);
}

bool CGameMainManage::SendErrNotifyMessage(int userID, const char * lpszMessage, int wType/* = SMT_EJECT*/)
{
	if (!lpszMessage || userID <= 0)
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

	SendData(userID, &msg, 8 + msg.sizeCount, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_ERR_MSG, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////
GameUserInfo * CGameMainManage::GetUser(int userID)
{
	if (!m_pGameUserManage)
	{
		return NULL;
	}

	return m_pGameUserManage->GetUser(userID);
}

bool CGameMainManage::OnUserLogout(int userID)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	// 清理内存和缓存数据
	DelUser(userID);

	return true;
}

//////////////////////////////////////////////////////////////////////
void CGameMainManage::DelUser(int userID)
{
	if (m_pGameUserManage)
	{
		m_pGameUserManage->DelUser(userID);
	}

	m_pRedis->SetUserRoomID(userID, 0);
	m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
}

//////////////////////////////////////////////////////////////////////
int CGameMainManage::GetRoomID()
{
	return m_InitData.uRoomID;
}

//////////////////////////////////////////////////////////////////////
bool CGameMainManage::RemoveOfflineUser(int userID)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	if (pUser->IsOnline)
	{
		ERROR_LOG("user is online userID=%d", userID);
		return false;
	}

	// 被移除的时候玩家必须处于站起状态
	if (pUser->playStatus != USER_STATUS_STAND)
	{
		ERROR_LOG("user status is invalid userID=%d, status=%d", userID, pUser->playStatus);
		return false;
	}

	// 清理内存和缓存数据
	DelUser(userID);

	return true;
}

bool CGameMainManage::RemoveWatchUser(int userID)
{
	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("GetUser failed userID=%d", userID);
		return false;
	}

	// 是否旁观状态
	if (pUser->playStatus != USER_STATUS_WATCH)
	{
		ERROR_LOG("user status is invalid userID=%d, status=%d", userID, pUser->playStatus);
		return false;
	}

	// 清理玩家内存
	DelUser(userID);

	return true;
}

void CGameMainManage::OnTimerCheckInvalidStatusUser()
{
	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return;
	}

	if (m_pGameUserManage)
	{
		m_pGameUserManage->CheckInvalidStatusUser(this);
	}
}

void CGameMainManage::CheckTimeOutDesk()
{
	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return;
	}

	if (!m_pRedis)
	{
		return;
	}

	int roomID = GetRoomID();

	time_t currTime = time(NULL);

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (!pDesk || !pDesk->IsEnable()) //调用IsEnable：只有玩家进过的桌子，10分钟才可以解散。代开没有进过的桌子不会解散
		{
			continue;
		}

		int deskMixID = MAKE_DESKMIXID(roomID, pDesk->m_deskIdx);

		PrivateDeskInfo privateDeskInfo;
		if (!m_pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
		{
			continue;
		}

		if (privateDeskInfo.masterID <= 0 || privateDeskInfo.roomID <= 0)
		{
			ERROR_LOG("检测到无效的桌子 deskMixID=%d", deskMixID);
			m_pRedis->DelPrivateDeskRecord(deskMixID);
			continue;
		}

		if (privateDeskInfo.friendsGroupID > 0 && privateDeskInfo.friendsGroupDeskNumber > 0)
		{
			continue;
		}

		//判断是否超时
		if (currTime - privateDeskInfo.checkTime <= PRIVATE_DESK_TIMEOUT_SECS)
		{
			continue;
		}

		// 当前没有玩家或者所有人都离线
		if (privateDeskInfo.currDeskUserCount <= 0 || pDesk->IsAllUserOffline())
		{
			INFO_LOG("清理超时桌子 roomID=%d,masterID=%d,passwd=%s",
				privateDeskInfo.roomID, privateDeskInfo.masterID, privateDeskInfo.passwd);
			pDesk->ProcessCostJewels();
			pDesk->ClearAllData(privateDeskInfo);
		}
	}
}

void CGameMainManage::OnCommonTimer()
{
	CheckTimeoutNotAgreeUser();

	CheckTimeoutNotOperateUser();

	if (GetRoomType() == ROOM_TYPE_MATCH)
	{
		CheckDeskStartMatch();
	}
}

void CGameMainManage::CheckRedisConnection()
{
	if (!m_pRedis)
	{
		ERROR_LOG("m_pRedis is NULL");
		return;
	}

	const RedisConfig& redisGameConfig = ConfigManage()->GetRedisConfig(REDIS_DATA_TYPE_GAME);

	m_pRedis->CheckConnection(redisGameConfig);

	if (m_pRedisPHP)
	{
		const RedisConfig& redisPHPConfig = ConfigManage()->GetRedisConfig(REDIS_DATA_TYPE_PHP);
		m_pRedisPHP->CheckConnection(redisPHPConfig);
	}
}

void CGameMainManage::CheckTimeoutNotAgreeUser()
{
	if (GetRoomType() != ROOM_TYPE_GOLD)
	{
		return;
	}

	if (m_InitData.iRoomSort == ROOM_SORT_HUNDRED || m_InitData.iRoomSort == ROOM_SORT_SCENE)
	{
		return;
	}

	time_t currTime = time(NULL);

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk && pDesk->IsEnable())
		{
			pDesk->CheckTimeoutNotAgreeUser(currTime);
		}
	}
}

// 检查超时不操作玩家
void CGameMainManage::CheckTimeoutNotOperateUser()
{
	if (GetRoomType() != ROOM_TYPE_GOLD)
	{
		return;
	}

	if (m_InitData.iRoomSort != ROOM_SORT_HUNDRED && m_InitData.iRoomSort != ROOM_SORT_SCENE)
	{
		return;
	}

	time_t currTime = time(NULL);

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk && pDesk->IsEnable())
		{
			pDesk->CheckTimeoutNotOperateUser(currTime);
		}
	}
}

void CGameMainManage::OnHundredGameStart()
{
	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(GetRoomID());
	if (pRoomBaseInfo->sort != ROOM_SORT_HUNDRED)
	{
		return;
	}

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk)
		{
			pDesk->OnStart();
		}
	}
}

void CGameMainManage::OnSceneGameStart()
{
	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(GetRoomID());
	if (pRoomBaseInfo->sort != ROOM_SORT_SCENE)
	{
		return;
	}

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk)
		{
			pDesk->OnStart();
		}
	}
}

// 定期保存当前房间人数
void CGameMainManage::OnSaveRoomPeopleCount()
{
	if (m_pRedis)
	{
		m_pRedis->SetRoomServerPeopleCount(GetRoomID(), m_pGameUserManage->GetUserCount());
	}
}

// 统计匹配桌子速度
void CGameMainManage::OnCombineDeskGameBegin()
{
	// 没有真人不开始游戏
	if (m_combineRealUserVec.size() <= 0)
	{
		return;
	}

	// 不够凑成一桌
	if (m_combineRealUserVec.size() + m_combineRobotUserVec.size() < m_KernelData.uDeskPeople)
	{
		return;
	}

	// 清空
	m_allCombineDeskUserVec.clear();

	// 将真人和机器人放入集合
	for (size_t i = 0; i < m_combineRealUserVec.size(); i++)
	{
		m_allCombineDeskUserVec.push_back(m_combineRealUserVec[i]);
	}
	UINT uNeedRobotNums = m_combineRealUserVec.size() * (m_KernelData.uDeskPeople - 1);
	for (size_t i = 0; i < m_combineRobotUserVec.size() && i < uNeedRobotNums; i++)
	{
		m_allCombineDeskUserVec.push_back(m_combineRobotUserVec[i]);
	}

	// 乱序集合
	random_shuffle(m_allCombineDeskUserVec.begin(), m_allCombineDeskUserVec.end());

	// 找到桌子，将玩家全部坐下
	for (int i = 0; i < (int)m_uDeskCount; i++)
	{
		if (m_allCombineDeskUserVec.size() < m_KernelData.uDeskPeople)
		{
			break;
		}

		CGameDesk *pGameDesk = GetDeskObject(i);
		if (!pGameDesk)
		{
			ERROR_LOG("GetDeskObject(%d) failed:", i);
			continue;
		}

		// 找到一张可以座的桌子
		if (pGameDesk->IsCanSitDesk())
		{
			while (pGameDesk->GetUserCount() < pGameDesk->m_iConfigCount && !pGameDesk->IsPlayGame(0))
			{
				int tempUserID = m_allCombineDeskUserVec[m_allCombineDeskUserVec.size() - 1];
				GameUserInfo* pTempUser = GetUser(tempUserID);
				if (!pTempUser)
				{
					ERROR_LOG("GetUser(%d) failed", tempUserID);
					DelCombineDeskUser(tempUserID, pTempUser->isVirtual);
					m_allCombineDeskUserVec.pop_back();
					continue;
				}

				// 给该玩家发送匹配坐桌成功
				m_pGServerConnect->SendData(pTempUser->socketIdx, NULL, 0, MSG_MAIN_LOADER_ACTION, MSG_ASS_LOADER_ACTION_COMBINE_SIT, 0, pTempUser->userID);

				if (!pGameDesk->UserSitDesk(pTempUser))
				{
					ERROR_LOG("UserSitDesk Fail");
				}

				// 排队列表中删除
				DelCombineDeskUser(tempUserID, pTempUser->isVirtual);
				m_allCombineDeskUserVec.pop_back();
			}

			if (!pGameDesk->IsPlayGame(0) && pGameDesk->GetUserCount() == pGameDesk->m_iConfigCount)
			{
				pGameDesk->GameBegin(0);
			}
		}
	}
}

int CGameMainManage::GetRoomType()
{
	return m_InitData.iRoomType;
}

//////////////////////////////////////////////////////////////////////////
// 组桌
bool CGameMainManage::AddCombineDeskUser(int userID, BYTE isVirtual)
{
	// 已经存在
	if (m_combineUserSet.count(userID) > 0)
	{
		return false;
	}

	m_combineUserSet.insert(userID);

	if (isVirtual)
	{
		m_combineRobotUserVec.push_back(userID);
	}
	else
	{
		m_combineRealUserVec.push_back(userID);
	}

	return true;
}

bool CGameMainManage::DelCombineDeskUser(int userID, BYTE isVirtual)
{
	if (m_combineUserSet.count(userID) <= 0)
	{
		return false;
	}

	m_combineUserSet.erase(userID);

	if (isVirtual)
	{
		for (auto itr = m_combineRobotUserVec.begin(); itr != m_combineRobotUserVec.end(); itr++)
		{
			if (*itr == userID)
			{
				m_combineRobotUserVec.erase(itr);
				break;
			}
		}
	}
	else
	{
		for (auto itr = m_combineRealUserVec.begin(); itr != m_combineRealUserVec.end(); itr++)
		{
			if (*itr == userID)
			{
				m_combineRealUserVec.erase(itr);
				break;
			}
		}
	}

	return true;
}

//////////////////////////////////中心服消息////////////////////////////////////////
bool CGameMainManage::SendMessageToCenterServer(UINT msgID, void * pData, UINT size, int userID/* = 0*/)
{
	if (m_pTcpConnect)
	{
		bool ret = m_pTcpConnect->Send(msgID, pData, size, userID);
		if (!ret)
		{
			ERROR_LOG("向中心服发送数据失败！！！");
		}
		return ret;
	}
	else
	{
		ERROR_LOG("向中心服发送数据失败：：m_pTcpConnect=NULL");
	}

	return false;
}

void CGameMainManage::SendResourcesChangeToLogonServer(int userID, int resourceType, long long value, int reason, long long changeValue)
{
	PlatformResourceChange msg;

	msg.resourceType = resourceType;
	msg.value = value;
	msg.reason = reason;
	msg.changeValue = changeValue;

	SendMessageToCenterServer(CENTER_MESSAGE_LOADER_RESOURCE_CHANGE, &msg, sizeof(msg), userID);
}

void CGameMainManage::SendFireCoinChangeToLogonServer(int friendsGroupID, int userID, long long value, int reason, long long changeValue)
{
	PlatformResourceChange msg;

	msg.reserveData = friendsGroupID;
	msg.value = value;
	msg.reason = reason;
	msg.changeValue = changeValue;

	SendMessageToCenterServer(CENTER_MESSAGE_LOADER_FIRECOIN_CHANGE, &msg, sizeof(msg), userID);
}

// 通知全服大奖或者公告
void CGameMainManage::SendRewardActivityNotify(const char * rewardMsg)
{
	if (rewardMsg == NULL)
	{
		ERROR_LOG("奖励消息不能为空");
		return;
	}

	LogonNotifyActivity msg;

	msg.sizeCount = strlen(rewardMsg) + 1;
	if (msg.sizeCount > sizeof(msg.content))
	{
		ERROR_LOG("奖励消息太长:%d", msg.sizeCount);
		return;
	}

	strcpy(msg.content, rewardMsg);

	SendMessageToCenterServer(CENTER_MESSAGE_LOADER_REWARD_ACTIVITY, &msg, sizeof(msg));
}

bool CGameMainManage::OnCenterServerMessage(UINT msgID, NetMessageHead * pNetHead, void* pData, UINT size, int userID)
{
	switch (msgID)
	{
	case CENTER_MESSAGE_COMMON_RESOURCE_CHANGE:
	{
		OnCenterMessageResourceChange(pData, size, userID);
		return true;
	}
	case PLATFORM_MESSAGE_FG_DISSMISS_DESK:
	{
		OnCenterMessageFGDissmissDesk(pData, size);
		return true;
	}
	case PLATFORM_MESSAGE_MASTER_DISSMISS_DESK:
	{
		OnCenterMessageMasterDissmissDesk(pData, size);
		return true;
	}
	case PLATFORM_MESSAGE_CLOSE_SERVER:
	{
		OnCenterCloseServerDissmissAllDesk(pData, size);
		return true;
	}
	case PLATFORM_MESSAGE_RELOAD_GAME_CONFIG:
	{
		OnCenterMessageReloadGameConfig(pData, size);
		return true;
	}
	case PLATFORM_MESSAGE_START_MATCH_PEOPLE:
	{
		OnCenterMessageStartMatchPeople(pData, size);
		return true;
	}
	case CENTER_MESSAGE_COMMON_START_MATCH_TIME:
	{
		OnCenterMessageStartMatchTime(pData, size);
		return true;
	}
	default:
		break;
	}
	return true;
}

bool CGameMainManage::OnCenterMessageResourceChange(void* pData, int size, int userID)
{
	if (size != sizeof(PlatformResourceChange))
	{
		return false;
	}

	PlatformResourceChange* pMessage = (PlatformResourceChange*)pData;
	if (!pMessage)
	{
		return false;
	}

	GameUserInfo* pUser = GetUser(userID);
	if (!pUser)
	{
		return true;
	}

	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_MATCH || roomType == ROOM_TYPE_PRIVATE)
	{
		return true;
	}

	if (pUser->deskIdx == INVALID_DESKIDX)
	{
		return true;
	}

	long long afterValue = 0;
	if (pMessage->resourceType == RESOURCE_TYPE_MONEY)
	{
		pUser->money += pMessage->changeValue;
		afterValue = pUser->money;
	}
	else if (pMessage->resourceType == RESOURCE_TYPE_FIRECOIN && roomType == ROOM_TYPE_FG_VIP)
	{
		pUser->fireCoin += (int)pMessage->changeValue;
		afterValue = pUser->fireCoin;
	}
	else if (pMessage->resourceType == RESOURCE_TYPE_JEWEL)
	{
		pUser->jewels += (int)pMessage->changeValue;
		afterValue = pUser->jewels;
	}
	else
	{
		return true;
	}

	if (roomType == ROOM_TYPE_MATCH || roomType == ROOM_TYPE_PRIVATE)
	{
		return true;
	}

	CGameDesk* pDesk = GetDeskObject(pUser->deskIdx);
	if (!pDesk)
	{
		ERROR_LOG("GetDeskObject failed deskIdx=%d", pUser->deskIdx);
		return false;
	}

	if (pMessage->resourceType == RESOURCE_TYPE_FIRECOIN && roomType == ROOM_TYPE_FG_VIP
		&& pDesk->m_friendsGroupMsg.friendsGroupID != pMessage->reserveData)
	{
		ERROR_LOG("俱乐部id不正确,桌子friendsGroupID=%d,reserveData=%d", pDesk->m_friendsGroupMsg.friendsGroupID, pMessage->reserveData);
		return false;
	}

	pDesk->NotifyUserResourceChange(pUser->userID, pMessage->resourceType, afterValue, pMessage->changeValue);

	return true;
}

bool CGameMainManage::OnCenterMessageFGDissmissDesk(void* pData, int size)
{
	if (size != sizeof(PlatformDissmissDesk))
	{
		return false;
	}

	PlatformDissmissDesk* pMessage = (PlatformDissmissDesk*)pData;
	if (!pMessage)
	{
		return false;
	}

	int userID = pMessage->userID;
	int deskIdx = pMessage->deskMixID % MAX_ROOM_HAVE_DESK_COUNT;


	CGameDesk* pDesk = GetDeskObject(deskIdx);
	if (!pDesk)
	{
		ERROR_LOG("大厅解散桌子GetDeskObject failed deskIdx=%d", deskIdx);
		return false;
	}

	//解散桌子
	pDesk->LogonDissmissDesk(userID, pMessage->bDelete);

	return true;
}

bool CGameMainManage::OnCenterMessageMasterDissmissDesk(void* pData, int size)
{
	if (size != sizeof(PlatformDissmissDesk))
	{
		return false;
	}

	PlatformDissmissDesk* pMessage = (PlatformDissmissDesk*)pData;
	if (!pMessage)
	{
		return false;
	}

	int deskIdx = pMessage->deskMixID % MAX_ROOM_HAVE_DESK_COUNT;

	CGameDesk* pDesk = GetDeskObject(deskIdx);
	if (!pDesk)
	{
		ERROR_LOG("大厅解散桌子GetDeskObject failed deskIdx=%d", deskIdx);
		return false;
	}

	//解散桌子
	pDesk->LogonDissmissDesk();

	return true;
}

// 关服，解散所有桌子
void CGameMainManage::OnCenterCloseServerDissmissAllDesk(void* pData, int size)
{
	if (size != 0)
	{
		ERROR_LOG("关服命令失败。数据包大小不对");
		return;
	}

	if (GetRoomType() == ROOM_TYPE_GOLD || GetRoomType() == ROOM_TYPE_MATCH)
	{
		return;
	}

	if (!m_pRedis)
	{
		return;
	}

	int roomID = GetRoomID();

	std::vector<int> vecRooms;
	m_pRedis->GetAllDesk(roomID, vecRooms);
	int iSaveCount = 0;
	for (size_t i = 0; i < vecRooms.size(); i++)
	{
		int deskIdx = vecRooms[i];
		int deskMixID = MAKE_DESKMIXID(roomID, deskIdx);

		PrivateDeskInfo privateDeskInfo;
		if (!m_pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
		{
			continue;
		}

		CGameDesk* pDesk = GetDeskObject(deskIdx);
		if (!pDesk)
		{
			continue;
		}

		if (privateDeskInfo.friendsGroupID > 0 && privateDeskInfo.friendsGroupDeskNumber > 0)
		{
			if (m_pRedis->SaveFGDeskRoom(privateDeskInfo))
			{
				iSaveCount++;
			}
		}

		//解散所有桌子（强行解散）
		pDesk->LogonDissmissDesk();
	}

	INFO_LOG("======== LoaderServer 关闭成功 ,关闭桌子数量%d，保存桌子数量%d=======", vecRooms.size(), iSaveCount);
}

// 重新加载配置数据
bool CGameMainManage::OnCenterMessageReloadGameConfig(void* pData, int size)
{
	if (size != 0)
	{
		ERROR_LOG("不匹配");
		return false;
	}

	OnUpdate();

	return true;
}

// 开始比赛(实时赛)
bool CGameMainManage::OnCenterMessageStartMatchPeople(void* pData, int size)
{
	// 统计消耗
	AUTOCOST("PHP请求开始比赛耗时：");

	SAFECHECK_MESSAGE(pMessage, PlatformPHPReqStartMatchPeople, pData, size);

	if (!m_pRedis)
	{
		return false;
	}

	if (GetRoomType() != ROOM_TYPE_MATCH)
	{
		ERROR_LOG("房间类型不正确,roomType=%d", GetRoomType());
		return false;
	}

	if (pMessage->gameID != m_uNameID)
	{
		ERROR_LOG("游戏id不匹配，gameID=%d,pMessage->gameID=%d", m_uNameID, pMessage->gameID);
		return false;
	}

	GameBaseInfo * pGameBaseInfo = ConfigManage()->GetGameBaseInfo(m_uNameID);
	if (!pGameBaseInfo)
	{
		ERROR_LOG("没有配置该游戏，gameID=%d", m_uNameID);
		return false;
	}

	//分配一个局部比赛id
	long long llPartOfMatchID = m_pRedis->GetPartOfMatchIndex();
	if (llPartOfMatchID <= 0)
	{
		ERROR_LOG("比赛开赛失败,llPartOfMatchID=%lld", llPartOfMatchID);
		return false;
	}

	//获取比赛人数
	std::vector<MatchUserInfo> vecPeople;
	if (!m_pRedis->GetFullPeopleMatchPeople(m_uNameID, pMessage->matchID, pMessage->peopleCount, vecPeople))
	{
		ERROR_LOG("开赛人数不够，无法开赛，当前人数=%d，需要人数=%d", vecPeople.size(), pMessage->peopleCount);
		return true;
	}

	//获取比赛的桌子
	std::list<int> matchDeskList;
	if (!GetMatchDesk(pMessage->peopleCount / pGameBaseInfo->deskPeople, matchDeskList))
	{
		return true;
	}

	//将相关比赛人员，全部拉进桌子
	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		int userID = vecPeople[i].userID;
		UserData userData;
		if (!m_pRedis->GetUserData(userID, userData))
		{
			ERROR_LOG("获取玩家数据失败，导致无法开赛,uerID=%d", userID);
			return false;
		}

		// 更新状态
		vecPeople[i].byMatchStatus = DESK_MATCH_STATUS_NORMAL;

		// 内存数据
		GameUserInfo* pUser = m_pGameUserManage->GetUser(userID);
		if (pUser)
		{
			m_pGameUserManage->DelUser(userID);
			pUser = NULL;
		}

		// 创建一个新玩家
		pUser = new GameUserInfo;
		if (!pUser)
		{
			ERROR_LOG("无法开赛，分配内存失败,userID = %d", userID);
			return false;
		}

		//初始化比赛信息
		pUser->matchSocre = 0;

		//填充内存数据
		pUser->userID = userID;
		pUser->money = userData.money;
		pUser->jewels = userData.jewels;
		pUser->userStatus = userData.status;
		pUser->isVirtual = userData.isVirtual;
		memcpy(pUser->name, userData.name, sizeof(pUser->name));
		memcpy(pUser->headURL, userData.headURL, sizeof(pUser->headURL));
		pUser->sex = userData.sex;
		pUser->IsOnline = false;
		pUser->socketIdx = -1;
		if (pUser->isVirtual == 1)
		{
			RobotPositionInfo positionInfo;
			int iRobotIndex = m_pRedis->GetRobotInfoIndex();
			ConfigManage()->GetRobotPositionInfo(iRobotIndex, positionInfo);
			strcpy(pUser->longitude, positionInfo.longitude.c_str());
			strcpy(pUser->latitude, positionInfo.latitude.c_str());
			strcpy(pUser->address, positionInfo.address.c_str());
			strcpy(pUser->ip, positionInfo.ip.c_str());
			strcpy(pUser->name, positionInfo.name.c_str());
			strcpy(pUser->headURL, positionInfo.headUrl.c_str());
			pUser->sex = positionInfo.sex;
		}
		else
		{
			memcpy(pUser->longitude, userData.Lng, sizeof(pUser->longitude));
			memcpy(pUser->latitude, userData.Lat, sizeof(pUser->latitude));
			memcpy(pUser->address, userData.address, sizeof(pUser->address));
			memcpy(pUser->motto, userData.motto, sizeof(pUser->motto));
		}

		m_pGameUserManage->AddUser(pUser);
	}

	time_t currTime = time(NULL);

	//初始化比赛桌子
	for (auto itr = matchDeskList.begin(); itr != matchDeskList.end(); itr++)
	{
		CGameDesk * pGameDesk = GetDeskObject(*itr);
		if (pGameDesk == NULL)
		{
			ERROR_LOG("### 桌子内存找不到,index=%d ###", *itr);
			return false;
		}

		pGameDesk->InitDeskMatchData();
		pGameDesk->m_llPartOfMatchID = llPartOfMatchID;
		pGameDesk->m_iCurMatchRound = 1;
		pGameDesk->m_iMaxMatchRound = pMessage->matchRound;
		pGameDesk->m_llStartMatchTime = currTime + 10; //设置开始比赛时间
	}

	//通知这些玩家过来参加比赛
	PlatformLoaderNotifyStartMatch notifyMsg;
	notifyMsg.gameID = m_uNameID;
	notifyMsg.matchType = MATCH_TYPE_PEOPLE;
	notifyMsg.matchID = pMessage->matchID;
	notifyMsg.peopleCount = pMessage->peopleCount;
	notifyMsg.roomID = GetRoomID();
	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		notifyMsg.userID[i] = vecPeople[i].userID;
	}
	SendMessageToCenterServer(CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH, &notifyMsg, sizeof(notifyMsg));

	//将桌子加入内存
	m_matchGameDeskMap.insert(std::make_pair(llPartOfMatchID, matchDeskList));

	//将比赛人员在内存中保存
	m_matchUserMap.insert(std::make_pair(llPartOfMatchID, vecPeople));

	//保存比赛大ID
	m_matchMainIDMap[llPartOfMatchID] = pMessage->matchID;

	//保存比赛类型
	m_matchTypeMap[llPartOfMatchID] = MATCH_TYPE_PEOPLE;

	//将这些参赛人员从redis中删除
	m_pRedis->DelFullPeopleMatchPeople(m_uNameID, pMessage->matchID, vecPeople);

	//开始比赛
	AllocDeskStartMatch(matchDeskList, vecPeople);

	return true;
}

// 开始比赛(定时赛)
bool CGameMainManage::OnCenterMessageStartMatchTime(void* pData, int size)
{
	// 统计消耗
	AUTOCOST("定时赛开始比赛耗时：");

	SAFECHECK_MESSAGE(pMessage, PlatformReqStartMatchTime, pData, size);

	time_t currTime = time(NULL);

	if (!m_pRedis)
	{
		return false;
	}

	if (GetRoomType() != ROOM_TYPE_MATCH)
	{
		ERROR_LOG("房间类型不正确,roomType=%d", GetRoomType());
		return false;
	}

	if (pMessage->gameID != m_uNameID)
	{
		ERROR_LOG("游戏id不匹配，gameID=%d,pMessage->gameID=%d", m_uNameID, pMessage->gameID);
		return false;
	}

	GameBaseInfo * pGameBaseInfo = ConfigManage()->GetGameBaseInfo(m_uNameID);
	if (!pGameBaseInfo)
	{
		ERROR_LOG("没有配置该游戏，gameID=%d", m_uNameID);
		SendMatchFailMail(MATCH_FAIL_REASON_SYSTEM_ERROR, 0, MATCH_TYPE_TIME, pMessage->matchID);
		return false;
	}

	//分配一个局部比赛id
	long long llPartOfMatchID = m_pRedis->GetPartOfMatchIndex();
	if (llPartOfMatchID <= 0)
	{
		ERROR_LOG("比赛开赛失败,llPartOfMatchID=%lld", llPartOfMatchID);
		SendMatchFailMail(MATCH_FAIL_REASON_SYSTEM_ERROR, 0, MATCH_TYPE_TIME, pMessage->matchID);
		return false;
	}

	//获取比赛人数
	std::vector<MatchUserInfo> vecPeople;
	if (!m_pRedis->GetTimeMatchPeople(pMessage->matchID, vecPeople))
	{
		ERROR_LOG("无法开赛，matchID=%d 当前人数=%d", pMessage->matchID, vecPeople.size());
		SendMatchFailMail(MATCH_FAIL_REASON_NOT_ENOUGH_PEOPLE, 0, MATCH_TYPE_TIME, pMessage->matchID);
		return true;
	}

	//最大人数限制
	if (vecPeople.size() > MAX_MATCH_PEOPLE_COUNT)
	{
		ERROR_LOG("比赛人数超过限制，当前人数=%d", vecPeople.size());
		SendMatchFailMail(MATCH_FAIL_REASON_SYSTEM_ERROR, 0, MATCH_TYPE_TIME, pMessage->matchID);
		return true;
	}

	//有效玩家加入内存。失败清理内存
	for (auto itr = vecPeople.begin(); itr != vecPeople.end();)
	{
		int userID = itr->userID;
		UserData userData;
		if (!m_pRedis->GetUserData(userID, userData))
		{
			ERROR_LOG("获取玩家数据失败,uerID=%d", userID);
			itr = vecPeople.erase(itr);
			continue;
		}

		//游戏中，视为弃权
		if (userData.roomID > 0)
		{
			SendMatchFailMail(MATCH_FAIL_REASON_PLAYING, userID, MATCH_TYPE_TIME, pMessage->matchID);
			ClearMatchStatus(MATCH_FAIL_REASON_PLAYING, userID, MATCH_TYPE_TIME, pMessage->matchID);
			itr = vecPeople.erase(itr);
			continue;
		}

		// 更新状态
		itr->byMatchStatus = DESK_MATCH_STATUS_NORMAL;

		// 内存数据
		GameUserInfo* pUser = m_pGameUserManage->GetUser(userID);
		if (pUser)
		{
			m_pGameUserManage->DelUser(userID);
			pUser = NULL;
		}

		// 创建一个新玩家
		pUser = new GameUserInfo;
		if (!pUser)
		{
			ERROR_LOG("分配内存失败,userID = %d", userID);
			itr = vecPeople.erase(itr);
			continue;
		}

		//初始化比赛信息
		pUser->matchSocre = 0;

		//填充内存数据
		pUser->userID = userID;
		pUser->money = userData.money;
		pUser->jewels = userData.jewels;
		pUser->userStatus = userData.status;
		pUser->isVirtual = userData.isVirtual;
		memcpy(pUser->name, userData.name, sizeof(pUser->name));
		memcpy(pUser->headURL, userData.headURL, sizeof(pUser->headURL));
		pUser->sex = userData.sex;
		pUser->IsOnline = false;
		pUser->socketIdx = -1;
		memcpy(pUser->longitude, userData.Lng, sizeof(pUser->longitude));
		memcpy(pUser->latitude, userData.Lat, sizeof(pUser->latitude));
		memcpy(pUser->address, userData.address, sizeof(pUser->address));
		memcpy(pUser->motto, userData.motto, sizeof(pUser->motto));

		m_pGameUserManage->AddUser(pUser);

		itr++;
	}

	//判断有效人数数量，是否小于最小值
	if (vecPeople.size() < (UINT)pMessage->minPeople || vecPeople.size() < m_KernelData.uDeskPeople)
	{
		ERROR_LOG("开始失败，比赛少于所需人数，最少人数=%d,当前人数=%d,matchID=%d", pMessage->minPeople, vecPeople.size(), pMessage->matchID);

		//发送比赛失败邮件通知，退报名费，清理比赛状态
		for (size_t i = 0; i < vecPeople.size(); i++)
		{
			ClearMatchStatus(MATCH_FAIL_REASON_NOT_ENOUGH_PEOPLE, vecPeople[i].userID, MATCH_TYPE_TIME, pMessage->matchID);
		}

		SendMatchFailMail(MATCH_FAIL_REASON_NOT_ENOUGH_PEOPLE, 0, MATCH_TYPE_TIME, pMessage->matchID);
		ClearMatchUser(pMessage->matchID, vecPeople);

		return false;
	}

	//添加机器人，满足比赛所需人数
	bool bStartMatchSuccess = true;
	int iNeedRobotCount = vecPeople.size() % m_KernelData.uDeskPeople == 0 ? 0 : (m_KernelData.uDeskPeople - (vecPeople.size() % m_KernelData.uDeskPeople));
	for (int i = 0; i < iNeedRobotCount; i++)
	{
		int userID = MatchGetRobotUserID();
		if (userID <= 0)
		{
			bStartMatchSuccess = false;
			ERROR_LOG("添加机器人失败");
			break;
		}

		// 内存数据
		GameUserInfo* pUser = new GameUserInfo;
		if (!pUser)
		{
			ERROR_LOG("分配内存失败,userID = %d", userID);
			bStartMatchSuccess = false;
			break;
		}

		//初始化比赛信息
		pUser->matchSocre = 0;
		MatchUserInfo robotMatchUser;
		robotMatchUser.userID = userID;
		robotMatchUser.byMatchStatus = DESK_MATCH_STATUS_NORMAL;
		robotMatchUser.signUpTime = currTime;
		vecPeople.push_back(robotMatchUser);

		//填充内存数据
		pUser->userID = userID;
		pUser->money = 0;
		pUser->jewels = 0;
		pUser->userStatus = 0;
		pUser->isVirtual = 1;
		pUser->IsOnline = true;
		pUser->socketIdx = -1;
		RobotPositionInfo positionInfo;
		int iRobotIndex = m_pRedis->GetRobotInfoIndex();
		ConfigManage()->GetRobotPositionInfo(iRobotIndex, positionInfo);
		strcpy(pUser->longitude, positionInfo.longitude.c_str());
		strcpy(pUser->latitude, positionInfo.latitude.c_str());
		strcpy(pUser->address, positionInfo.address.c_str());
		strcpy(pUser->ip, positionInfo.ip.c_str());
		strcpy(pUser->name, positionInfo.name.c_str());
		strcpy(pUser->headURL, positionInfo.headUrl.c_str());
		pUser->sex = positionInfo.sex;

		m_pGameUserManage->AddUser(pUser);
	}
	if (!bStartMatchSuccess)
	{
		//发送比赛失败邮件通知，退报名费，清理比赛状态
		for (size_t i = 0; i < vecPeople.size() - iNeedRobotCount; i++)
		{
			ClearMatchStatus(MATCH_FAIL_REASON_SYSTEM_ERROR, vecPeople[i].userID, MATCH_TYPE_TIME, pMessage->matchID);
		}

		SendMatchFailMail(MATCH_FAIL_REASON_SYSTEM_ERROR, 0, MATCH_TYPE_TIME, pMessage->matchID);
		ClearMatchUser(pMessage->matchID, vecPeople);

		ERROR_LOG("比赛开始失败，添加机器人失败");
		return false;
	}

	//获取比赛的桌子
	int iNeedDeskCount = vecPeople.size() / m_KernelData.uDeskPeople;
	std::list<int> matchDeskList;
	if (!GetMatchDesk(iNeedDeskCount, matchDeskList))
	{
		ERROR_LOG("无法开赛，桌子数量不够 iNeedDeskCount=%d", iNeedDeskCount);

		//发送比赛失败邮件通知，退报名费，清理比赛状态
		for (size_t i = 0; i < vecPeople.size() - iNeedRobotCount; i++)
		{
			ClearMatchStatus(MATCH_FAIL_REASON_SYSTEM_ERROR, vecPeople[i].userID, MATCH_TYPE_TIME, pMessage->matchID);
		}

		SendMatchFailMail(MATCH_FAIL_REASON_SYSTEM_ERROR, 0, MATCH_TYPE_TIME, pMessage->matchID);
		ClearMatchUser(pMessage->matchID, vecPeople);

		return false;
	}

	//全部验证通过可以开始比赛，计算比赛所需轮数
	int iMaxMatchRound = 0;
	for (iMaxMatchRound = 1; iMaxMatchRound <= MAX_MATCH_ROUND + 1; iMaxMatchRound++)
	{
		if (m_KernelData.uDeskPeople * (int)pow(2.0, iMaxMatchRound - 1) > vecPeople.size())
		{
			break;
		}
	}
	iMaxMatchRound = iMaxMatchRound - 1;

	//初始化比赛桌子
	for (auto itr = matchDeskList.begin(); itr != matchDeskList.end(); itr++)
	{
		CGameDesk * pGameDesk = GetDeskObject(*itr);
		if (pGameDesk == NULL)
		{
			ERROR_LOG("######  开始比赛失败，桌子内存找不到,index=%d  ######", *itr);
			return false;
		}

		pGameDesk->InitDeskMatchData();
		pGameDesk->m_llPartOfMatchID = llPartOfMatchID;
		pGameDesk->m_iCurMatchRound = 1;
		pGameDesk->m_iMaxMatchRound = iMaxMatchRound;
		pGameDesk->m_llStartMatchTime = currTime + 10; //设置开始比赛时间
	}

	//通知这些玩家过来参加比赛
	PlatformLoaderNotifyStartMatch notifyMsg;
	notifyMsg.gameID = m_uNameID;
	notifyMsg.matchType = MATCH_TYPE_TIME;
	notifyMsg.matchID = pMessage->matchID;
	notifyMsg.roomID = GetRoomID();
	notifyMsg.peopleCount = vecPeople.size();
	for (int i = 0; i < notifyMsg.peopleCount; i++)
	{
		notifyMsg.userID[i] = vecPeople[i].userID;
	}
	SendMessageToCenterServer(CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH, &notifyMsg, sizeof(notifyMsg));

	//设置比赛状态为比赛中
	if (m_pRedisPHP)
	{
		m_pRedisPHP->SetMatchStatus(pMessage->matchID, MATCH_STATUS_PLAYING);
	}

	//将桌子加入内存
	m_matchGameDeskMap.insert(std::make_pair(llPartOfMatchID, matchDeskList));

	//将比赛人员在内存中保存
	m_matchUserMap.insert(std::make_pair(llPartOfMatchID, vecPeople));

	//保存比赛大ID
	m_matchMainIDMap[llPartOfMatchID] = pMessage->matchID;

	//保存比赛类型
	m_matchTypeMap[llPartOfMatchID] = MATCH_TYPE_TIME;

	//开始比赛
	AllocDeskStartMatch(matchDeskList, vecPeople);

	return true;
}

//////////////////////////////////////////////////////////////////////
//比赛场

// 为比赛分配一定数量的桌子
bool CGameMainManage::GetMatchDesk(int needDeskCount, std::list<int> &listDesk)
{
	if (needDeskCount <= 0)
	{
		ERROR_LOG("参数错误,needDeskCount=%d", needDeskCount);
		return false;
	}

	listDesk.clear();

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk && !pDesk->IsPlayGame(0) && pDesk->m_llPartOfMatchID == 0)
		{
			listDesk.push_back(i);
		}

		if (listDesk.size() >= (UINT)needDeskCount)
		{
			break;
		}
	}

	if (listDesk.size() != needDeskCount)
	{
		ERROR_LOG("桌子分配错误,listDeskCount=%d,needDeskCount=%d", listDesk.size(), needDeskCount);
		return false;
	}

	return true;
}

// 为比赛人员分配桌子，并坐下
void CGameMainManage::AllocDeskStartMatch(const std::list<int> &matchDeskList, const std::vector<MatchUserInfo> &vecPeople)
{
	if (matchDeskList.size() <= 0 || vecPeople.size() <= 0)
	{
		ERROR_LOG("分配桌子失败，desklistCount=%d,peopleCount=%d", matchDeskList.size(), vecPeople.size());
		return;
	}

	int arrUserID[MAX_MATCH_PEOPLE_COUNT] = { 0 };
	memset(arrUserID, false, sizeof(arrUserID));
	int iRemainMatchPeopleCount = 0;

	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		if (vecPeople[i].userID > 0 && vecPeople[i].byMatchStatus == DESK_MATCH_STATUS_NORMAL)
		{
			arrUserID[iRemainMatchPeopleCount++] = vecPeople[i].userID;
		}
		else
		{
			break;
		}
	}

	if (iRemainMatchPeopleCount / m_KernelData.uDeskPeople != matchDeskList.size())
	{
		ERROR_LOG("####  剩余人数错误:iRemainMatchPeopleCount=%d,m_KernelData.uDeskPeople=%d,matchDeskList.size()=%d  ####",
			iRemainMatchPeopleCount, m_KernelData.uDeskPeople, matchDeskList.size());
		return;
	}

	//随机乱序，数组
	int temp = 0, data_ = 0;
	int iCount = 2;
	while (--iCount >= 0)
	{
		for (int i = 0; i < iRemainMatchPeopleCount; i++)
		{
			temp = CUtil::GetRandNum() % (iRemainMatchPeopleCount - i) + i;
			data_ = arrUserID[temp];
			arrUserID[temp] = arrUserID[i];
			arrUserID[i] = data_;
		}
	}

	for (auto itr = matchDeskList.begin(); itr != matchDeskList.end(); itr++)
	{
		CGameDesk* pDesk = GetDeskObject(*itr);
		if (!pDesk)
		{
			ERROR_LOG("无效的桌子索引:%d", *itr);
			continue;
		}

		//从数组中取玩家坐桌
		for (size_t i = 0; i < m_KernelData.uDeskPeople; i++)
		{
			if (iRemainMatchPeopleCount <= 0)
			{
				ERROR_LOG("#### 剩余人数不够 ####");
				return;
			}

			int userID = arrUserID[iRemainMatchPeopleCount - 1];

			GameUserInfo* pUser = m_pGameUserManage->GetUser(userID);
			if (!pUser)
			{
				ERROR_LOG("###### 重大错误,userID=%d #######", userID);
				return;
			}

			if (!pDesk->UserSitDesk(pUser))
			{
				ERROR_LOG("###### 重大错误,userID=%d #######", userID);
				return;
			}

			iRemainMatchPeopleCount--;
		}
	}
}

// 定时检查可以开始比赛的桌子
void CGameMainManage::CheckDeskStartMatch()
{
	time_t currTime = time(NULL);

	for (unsigned int i = 0; i < m_uDeskCount; i++)
	{
		CGameDesk* pDesk = GetDeskObject(i);
		if (pDesk &&  pDesk->m_llStartMatchTime > 0 && currTime >= pDesk->m_llStartMatchTime && !pDesk->IsPlayGame(0))
		{
			pDesk->GameBegin(0);
		}
	}
}

// 是否全部桌子都完成比赛
bool CGameMainManage::IsAllDeskFinishMatch(long long llPartOfMatchID)
{
	std::list<int> &deskList = m_matchGameDeskMap[llPartOfMatchID];

	if (deskList.size() <= 0)
	{
		ERROR_LOG("判断一轮比赛是否结束失败，desklistCount=%d", deskList.size());
		return false;
	}

	int iFinishCount = 0;
	for (auto itr = deskList.begin(); itr != deskList.end(); itr++)
	{
		CGameDesk* pDesk = GetDeskObject(*itr);
		if (pDesk && pDesk->m_bFinishMatch)
		{
			iFinishCount++;
		}
	}

	if (iFinishCount == deskList.size())
	{
		return true;
	}

	return false;
}

// 某一张桌子游戏结束
void CGameMainManage::MatchDeskFinish(long long llPartOfMatchID, int deskIdx)
{
	// 通知所有还没有被淘汰的玩家，一个桌子完成比赛
	LoaderNotifyDeskFinishMatch msg;
	msg.deskIdx = deskIdx;

	std::vector<MatchUserInfo> &vecPeople = m_matchUserMap[llPartOfMatchID];

	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		if (vecPeople[i].userID > 0 && vecPeople[i].byMatchStatus == DESK_MATCH_STATUS_NORMAL)
		{
			SendData(vecPeople[i].userID, &msg, sizeof(msg), MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_DESK_FINISH_MATCH, 0);
		}
	}
}

// 比赛进入下一轮
void CGameMainManage::MatchNextRound(long long llPartOfMatchID, int iCurMatchRound, int iMaxMatchRound)
{
	// 统计消耗
	AUTOCOST("进入下一轮比赛耗时：");

	std::vector<MatchUserInfo> &vecPeople = m_matchUserMap[llPartOfMatchID];
	std::list<int> &deskList = m_matchGameDeskMap[llPartOfMatchID];
	BYTE matchType = m_matchTypeMap[llPartOfMatchID];

	//根据积分排序
	int allUserCount = MatchSortUser(vecPeople, iCurMatchRound);

	//计算晋级玩家数量
	int iPromotionCount = 0;
	if (matchType == MATCH_TYPE_PEOPLE || matchType == MATCH_TYPE_TIME && iCurMatchRound > 1)
	{
		iPromotionCount = allUserCount / 2;
	}
	else
	{
		int iCount_ = 0;
		for (int i = 1; i <= MAX_MATCH_ROUND + 1; i++)
		{
			iCount_ = m_KernelData.uDeskPeople * (int)pow(2.0, i - 1);
			if (iCount_ > allUserCount)
			{
				break;
			}
		}

		iPromotionCount = iCount_ / 4;
	}

	LoaderNotifyMatchRank rankmsg;

	//将所有玩家排名填充
	rankmsg.peopleCount = allUserCount;
	for (int i = 0; i < allUserCount; i++)
	{
		rankmsg.user[i].userID = vecPeople[i].userID;
		rankmsg.user[i].rank = i + 1;
	}
	int iRankMsgSize = sizeof(LoaderNotifyMatchRank::LoaderNotifyMatchRankUser) * allUserCount + sizeof(rankmsg) - sizeof(rankmsg.user);

	//淘汰玩家
	for (int i = 0; i < allUserCount; i++)
	{
		int userID = vecPeople[i].userID;
		GameUserInfo *pUser = GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		//让玩家离开桌子
		CGameDesk* pDesk = GetDeskObject(pUser->deskIdx);
		if (pDesk)
		{
			pDesk->UserLeftDesk(pUser);
		}

		rankmsg.gameID = m_uNameID;
		rankmsg.gameMatchID = m_matchMainIDMap[llPartOfMatchID];
		rankmsg.iCurMatchRound = iCurMatchRound;
		rankmsg.iMaxMatchRound = iMaxMatchRound;

		if (i < iPromotionCount) //晋级玩家
		{
			vecPeople[i].byMatchStatus = DESK_MATCH_STATUS_NORMAL;
			rankmsg.type = 1;
			rankmsg.rankMatch = i + 1;

			//推送排名通知
			SendData(userID, &rankmsg, iRankMsgSize, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_MATCH_RANK, 0);

			//分数变成1/3，准备进入下一轮
			if (pUser->matchSocre > 0)
			{
				pUser->matchSocre /= 3;
			}
			else
			{
				pUser->matchSocre = 0;
			}
		}
		else //淘汰玩家
		{
			vecPeople[i].byMatchStatus = DESK_MATCH_STATUS_FAIL;
			rankmsg.type = 0;
			rankmsg.rankMatch = i + 1;

			//推送排名通知
			SendData(userID, &rankmsg, iRankMsgSize, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_MATCH_RANK, 0);

			//清理玩家内存和redis状态，目前不断socket，玩家还可以继续观战
			DelUser(userID);
			m_pRedis->SetUserMatchStatus(userID, MATCH_TYPE_NORMAL, USER_MATCH_STATUS_NORMAL);
			m_pRedis->SetUserMatchRank(userID, (long long)m_uNameID*MAX_GAME_MATCH_ID + rankmsg.gameMatchID, i + 1);

			//发送邮件通知
			SendMatchGiftMail(userID, m_uNameID, matchType, rankmsg.gameMatchID, i + 1);
		}
	}

	//计算需要清理的桌子数量
	int iDeskCount = deskList.size();
	int iClearDeskCount = 0;
	if (matchType == MATCH_TYPE_PEOPLE || matchType == MATCH_TYPE_TIME && iCurMatchRound > 1)
	{
		iClearDeskCount = iDeskCount / 2;
	}
	else
	{
		iClearDeskCount = (allUserCount - iPromotionCount) / m_KernelData.uDeskPeople;
	}

	//清理桌子，并设置桌子数据
	int iIndex = 0;
	time_t currTime = time(NULL);
	for (auto itr = deskList.begin(); itr != deskList.end(); iIndex++)
	{
		CGameDesk* pDesk = GetDeskObject(*itr);
		if (!pDesk)
		{
			deskList.erase(itr++);
			continue;
		}
		if (iIndex < iClearDeskCount)
		{
			pDesk->InitDeskMatchData();
			deskList.erase(itr++);
			continue;
		}

		pDesk->m_iCurMatchRound++;
		pDesk->m_llStartMatchTime = currTime + 15; //设置开始比赛时间
		pDesk->m_bFinishMatch = false;

		itr++;
	}

	// 所有晋级玩家重新坐桌
	AllocDeskStartMatch(deskList, vecPeople);
}

// 比赛结束
void CGameMainManage::MatchEnd(long long llPartOfMatchID, int iCurMatchRound, int iMaxMatchRound)
{
	// 统计消耗
	AUTOCOST("比赛结束耗时：");

	std::vector<MatchUserInfo> &vecPeople = m_matchUserMap[llPartOfMatchID];
	std::list<int> &deskList = m_matchGameDeskMap[llPartOfMatchID];
	BYTE matchType = m_matchTypeMap[llPartOfMatchID];
	int gameMatchID = m_matchMainIDMap[llPartOfMatchID];

	//根据积分排序
	int allUserCount = MatchSortUser(vecPeople, iCurMatchRound);

	LoaderNotifyMatchRank rankmsg;

	//将所有玩家排名填充
	rankmsg.peopleCount = allUserCount;
	for (int i = 0; i < allUserCount; i++)
	{
		rankmsg.user[i].userID = vecPeople[i].userID;
		rankmsg.user[i].rank = i + 1;
	}
	int iRankMsgSize = sizeof(LoaderNotifyMatchRank::LoaderNotifyMatchRankUser) * allUserCount + sizeof(rankmsg) - sizeof(rankmsg.user);

	//抉择最终名次
	for (int i = 0; i < allUserCount; i++)
	{
		int userID = vecPeople[i].userID;
		GameUserInfo *pUser = GetUser(userID);
		if (!pUser)
		{
			continue;
		}

		//推送最终排名通知
		rankmsg.gameID = m_uNameID;
		rankmsg.gameMatchID = gameMatchID;
		rankmsg.iCurMatchRound = iCurMatchRound;
		rankmsg.iMaxMatchRound = iMaxMatchRound;
		rankmsg.type = 2;
		rankmsg.rankMatch = i + 1;
		SendData(userID, &rankmsg, iRankMsgSize, MSG_MAIN_LOADER_NOTIFY, MSG_NTF_LOADER_MATCH_RANK, 0);

		//清理玩家比赛状态，设置排名
		m_pRedis->SetUserMatchStatus(userID, MATCH_TYPE_NORMAL, USER_MATCH_STATUS_NORMAL);
		m_pRedis->SetUserMatchRank(userID, (long long)m_uNameID*MAX_GAME_MATCH_ID + rankmsg.gameMatchID, i + 1);

		// 清除玩家缓存中的数据
		m_pRedis->SetUserRoomID(userID, 0);
		m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);

		// 最后才能清理玩家
		m_pGameUserManage->DelUser(userID);

		//发送邮件通知
		SendMatchGiftMail(userID, m_uNameID, matchType, rankmsg.gameMatchID, i + 1);
	}

	//设置比赛结束相关信息
	if (matchType == MATCH_TYPE_TIME)
	{
		//根据排名重新设置报名集合
		m_pRedis->SetTimeMatchPeopleRank(gameMatchID, vecPeople);

		//设置比赛状态为比赛结束
		if (m_pRedisPHP)
		{
			m_pRedisPHP->SetMatchStatus(gameMatchID, MATCH_STATUS_GAME_OVER);
		}
	}

	//清理所有桌子
	for (auto itr = deskList.begin(); itr != deskList.end(); itr++)
	{
		CGameDesk* pDesk = GetDeskObject(*itr);
		if (!pDesk)
		{
			continue;
		}

		pDesk->InitDeskMatchData();
	}
	m_matchGameDeskMap.erase(llPartOfMatchID);

	//清理所有比赛玩家
	m_matchUserMap.erase(llPartOfMatchID);

	//清理比赛记录
	m_matchMainIDMap.erase(llPartOfMatchID);

	//清理比赛类型
	m_matchTypeMap.erase(llPartOfMatchID);
}

// 根据积分，排序没有淘汰的玩家
int CGameMainManage::MatchSortUser(std::vector<MatchUserInfo> &vecPeople, int iCurMatchRound)
{
	int iSortSize = vecPeople.size();
	for (int i = 1; i < iCurMatchRound; i++)
	{
		iSortSize /= 2;
	}

	int iMax = 0;
	for (int i = 0; i < iSortSize - 1; i++)
	{
		iMax = i;
		for (int j = i + 1; j < iSortSize; j++)
		{
			int userID = vecPeople[j].userID;
			int maxUserID = vecPeople[iMax].userID;
			GameUserInfo *pUser = GetUser(userID);
			GameUserInfo *pMaxUser = GetUser(maxUserID);
			if (!pUser || !pMaxUser)
			{
				continue;
			}

			if (pUser->matchSocre > pMaxUser->matchSocre)
			{
				iMax = j;
			}
			else if (pUser->matchSocre == pMaxUser->matchSocre && vecPeople[j].signUpTime < vecPeople[iMax].signUpTime)
			{
				iMax = j;
			}
		}

		if (iMax != i)
		{
			MatchUserInfo byTemp = vecPeople[iMax];
			vecPeople[iMax] = vecPeople[i];
			vecPeople[i] = byTemp;
		}
	}

	return iSortSize;
}

// 安排机器人加入桌子和玩家比赛
int CGameMainManage::MatchGetRobotUserID()
{
	int userID = 0;

	//最多尝试500次获取
	for (int iCount = 0; iCount < 500; iCount++)
	{
		userID = m_pRedis->GetRobotUserID();
		if (userID <= 0)
		{
			return 0;
		}

		if (m_pGameUserManage->GetUser(userID) != NULL)
		{
			continue;
		}

		UserData userData;
		if (!m_pRedis->GetUserData(userID, userData))
		{
			continue;
		}

		if (userData.isVirtual == 0)
		{
			continue;
		}

		if (userID > 0)
		{
			break;
		}
	}

	return userID;
}

// 发送比赛邮件奖励
void CGameMainManage::SendMatchGiftMail(int userID, int gameID, BYTE matchType, int gameMatchID, int ranking)
{
	OtherConfig otherConfig;
	if (!m_pRedis->GetOtherConfig(otherConfig))
	{
		otherConfig = ConfigManage()->GetOtherConfig();
	}

	char bufUserID[20] = "";
	sprintf(bufUserID, "%d", userID);

	//组合生成URL
	std::string url = "http://";
	url += otherConfig.http;
	url += "/hm_ucenter/web/index.php?api=match&action=matchAward&userID=";
	url += bufUserID;

	//发送邮件通知
	AsyncEventMsgHTTP asyncEvent;
	strcpy(asyncEvent.url, url.c_str());

	m_SQLDataManage.PushLine(&asyncEvent.dataLineHead, sizeof(AsyncEventMsgHTTP), ASYNC_EVENT_HTTP, userID, HTTP_POST_TYPE_MATCH_GIFT);
}

// 发送比赛失败，退报名费，以及清理比赛状态
void CGameMainManage::SendMatchFailMail(BYTE failReason, int userID, BYTE matchType, int gameMatchID)
{
	OtherConfig otherConfig;
	if (!m_pRedis->GetOtherConfig(otherConfig))
	{
		otherConfig = ConfigManage()->GetOtherConfig();
	}

	// 发送邮件
	char bufURL[256] = "";

	if (userID == 0)
	{
		sprintf(bufURL, "http://%s/hm_ucenter/web/index.php?api=match&action=matchNotStart&matchID=%d&reason=%d",
			otherConfig.http, gameMatchID, failReason);
	}
	else
	{
		sprintf(bufURL, "http://%s/hm_ucenter/web/index.php?api=match&action=matchFail&userID=%d&matchID=%d&reason=%d",
			otherConfig.http, userID, gameMatchID, failReason);
	}

	AsyncEventMsgHTTP asyncEvent;
	memcpy(asyncEvent.url, bufURL, min(sizeof(asyncEvent.url), sizeof(bufURL)));

	m_SQLDataManage.PushLine(&asyncEvent.dataLineHead, sizeof(AsyncEventMsgHTTP), ASYNC_EVENT_HTTP, userID == 0 ? gameMatchID : userID, HTTP_POST_TYPE_MATCH_FAIL);
}

// 清理比赛状态
void CGameMainManage::ClearMatchStatus(BYTE failReason, int userID, BYTE matchType, int gameMatchID)
{
	// 清理报名状态
	m_pRedis->SetUserMatchStatus(userID, MATCH_TYPE_NORMAL, USER_MATCH_STATUS_NORMAL);

	//通知这个玩家比赛失败
	PlatformLoaderNotifyStartMatchFail notifyMsg;
	notifyMsg.gameID = m_uNameID;
	notifyMsg.matchType = MATCH_TYPE_TIME;
	notifyMsg.matchID = gameMatchID;
	notifyMsg.reason = failReason;
	SendMessageToCenterServer(CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH_FAIL, &notifyMsg, sizeof(notifyMsg), userID);
}

// 清理比赛玩家内存
void CGameMainManage::ClearMatchUser(int gameMatchID, const std::vector<MatchUserInfo> &vecPeople)
{
	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		m_pGameUserManage->DelUser(vecPeople[i].userID);
	}

	//开始比赛失败，设置比赛状态为比赛结束
	if (m_pRedisPHP)
	{
		m_pRedisPHP->SetMatchStatus(gameMatchID, MATCH_STATUS_GAME_OVER);
	}
}