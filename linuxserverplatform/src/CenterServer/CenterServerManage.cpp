#include "CommonHead.h"
#include "NewMessageDefine.h"
#include "ErrorCode.h"
#include "CenterServerManage.h"

//////////////////////////////////////////////////////////////////////
CCenterServerManage::CCenterServerManage() : CBaseCenterServer()
{
	m_nPort = 0;
	m_uMaxPeople = 0;
	m_lastNormalTimerTime = 0;
}

//////////////////////////////////////////////////////////////////////
CCenterServerManage::~CCenterServerManage()
{
	m_serverToSocketMap.clear();
	m_logonGroupSocket.clear();
	m_centerUserInfoMap.clear();
	m_timeMatchInfoMap.clear();
}

//////////////////////////////////////////////////////////////////////
bool CCenterServerManage::OnSocketRead(NetMessageHead* pNetHead, CenterServerMessageHead* pCenterHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	if (!pNetHead)
	{
		return false;
	}

	AUTOCOST("msgID: %d mainID: %d assistID: %d", pCenterHead->msgID, pNetHead->uMainID, pNetHead->uAssistantID, uIndex);

	if (pCenterHead->msgID == COMMON_VERIFY_MESSAGE)
	{
		return OnHandleCommonServerVerifyMessage(pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	else if (CENTER_MESSAGE_COMMON_BEGIN < pCenterHead->msgID && pCenterHead->msgID < CENTER_MESSAGE_COMMON_END)
	{
		return OnHandleCommonMessage(pNetHead, pCenterHead, pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	else if (CENTER_MESSAGE_LOGON_BEGIN < pCenterHead->msgID && pCenterHead->msgID < CENTER_MESSAGE_LOGON_END)
	{
		return OnHandleLogonMessage(pNetHead, pCenterHead, pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	else if (CENTER_MESSAGE_LOADER_BEGIN < pCenterHead->msgID && pCenterHead->msgID < CENTER_MESSAGE_LOADER_END)
	{
		return OnHandleLoaderMessage(pNetHead, pCenterHead, pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	else if (PLATFORM_MESSAGE_BEGIN < pCenterHead->msgID && pCenterHead->msgID < PLATFORM_MESSAGE_END)
	{
		return OnHandlePHPMessage(pNetHead, pCenterHead, pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	else
	{
		ERROR_LOG("unhandled message pCenterHead->msgID=%d", pCenterHead->msgID);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CCenterServerManage::OnStart()
{
	INFO_LOG("CenterServerManage OnStart begin...");

	//清理所有在线用户
	if (m_pRedis)
	{
		m_pRedis->ClearOnlineUser();
	}

	//加载比赛信息
	m_pRedisPHP->LoadAllMatchInfo(m_timeMatchInfoMap);

	m_lastNormalTimerTime = time(NULL);

	m_socketIndexVec.resize(m_uMaxPeople);

	//// 测试redis读写速度
	//m_pRedis->TestRedis("CenterServer-GameRedis-Speed");
	//m_pRedisPHP->TestRedis("CenterServer-PHPRedis-Speed");

	InitRounteCheckEvent();

	INFO_LOG("CenterServerManage OnStart end.");

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CCenterServerManage::OnTimerMessage(UINT uTimerID)
{
	AUTOCOST("timerID = %d", uTimerID);

	switch (uTimerID)
	{
	case CENTER_TIMER_CHECK_REDIS_CONNECTION:
	{
		CheckRedisConnection();
		return true;
	}
	case CENTER_TIMER_NORMAL:
	{
		OnNormalTimer();
		return true;
	}
	case CENTER_TIMER_ROUTINE_CHECK_UNBINDID_SOCKET:
	{
		RoutineCheckUnbindIDSocket();
		return true;
	}
	case CENTER_TIMER_LOAD_MATCH_INFO:
	{
		m_pRedisPHP->LoadAllMatchInfo(m_timeMatchInfoMap);
		return true;
	}
	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
bool CCenterServerManage::OnStop()
{
	m_socketToServerMap.clear();

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CCenterServerManage::PreInitParameter(ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData)
{
	if (!pInitData || !pKernelData)
	{
		return false;
	}

	const CenterServerConfig& config = ConfigManage()->GetCenterServerConfig();

	m_uMaxPeople = config.maxSocketCount;
	m_nPort = config.port;

	pInitData->uListenPort = m_nPort;
	pInitData->uMaxPeople = m_uMaxPeople;
	memcpy(pInitData->szCenterIP, config.ip, sizeof(pInitData->szCenterIP));

	return true;
}

//////////////////////////////////////////////////////////////////////
// 走到这里的一般是底层通知业务层关闭(比如客户端断线，或者主动关闭socket之类)
bool CCenterServerManage::OnSocketClose(ULONG uAccessIP, UINT socketIdx, UINT uConnectTime)
{
	// 考虑到PHP，这里不生成日志
	auto itrSocketToServerMap = m_socketToServerMap.find(socketIdx);
	if (itrSocketToServerMap == m_socketToServerMap.end())
	{
		return false;
	}

	ServerBaseInfo serverInfo = itrSocketToServerMap->second;

	// 删除这个索引
	m_socketToServerMap.erase(itrSocketToServerMap);

	auto itrServerToSocket = m_serverToSocketMap.find(serverInfo);
	if (itrServerToSocket == m_serverToSocketMap.end())
	{
		ERROR_LOG("服务器信息不存在 serverType=%d,serverID=%d", serverInfo.serverType, serverInfo.serverID);
		return false;
	}

	// 删除这个服务器
	m_serverToSocketMap.erase(itrServerToSocket);

	if (!m_pRedis)
	{
		ERROR_LOG("m_pRedis = NULL");
		return false;
	}

	if (serverInfo.serverType == SERVICE_TYPE_LOGON) //登录服退出集群
	{
		auto itrVecLogonGroup = m_logonGroupSocket.begin();
		for (; itrVecLogonGroup != m_logonGroupSocket.end(); itrVecLogonGroup++)
		{
			if (itrVecLogonGroup->socketIdx == socketIdx)
			{
				break;
			}
		}
		if (itrVecLogonGroup == m_logonGroupSocket.end())
		{
			ERROR_LOG("登录服socket索引不存在 socketIdx=%d", socketIdx);
			return false;
		}

		// 删除索引
		m_logonGroupSocket.erase(itrVecLogonGroup);

		//重新发送集群信息
		SendDistributedSystemInfo();

		//将此登录服在线玩家玩家移除
		for (auto iter = m_centerUserInfoMap.begin(); iter != m_centerUserInfoMap.end(); )
		{
			if (iter->second.socketIdx == socketIdx)
			{
				m_pRedis->RemoveOnlineUser(iter->first, iter->second.isVirtual);
				m_centerUserInfoMap.erase(iter++);
			}
			else
			{
				iter++;
			}
		}

		m_pRedis->SetLogonServerStatus(serverInfo.serverID, 0);
	}
	else if (serverInfo.serverType == SERVICE_TYPE_LOADER) //游戏服退出集群
	{
		m_pRedis->SetRoomServerStatus(serverInfo.serverID, 0);
	}

	INFO_LOG("====== serverType:%d,serverID:%d退出集群 登录服集群数量:%d=========", serverInfo.serverType, serverInfo.serverID, m_logonGroupSocket.size());

	return true;
}

//////////////////////////////发送数据(登陆服发送)////////////////////////////////////////////
bool CCenterServerManage::SendData(int userID, UINT msgID, void* pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode)
{
	auto itr = m_centerUserInfoMap.find(userID);
	if (itr == m_centerUserInfoMap.end())
	{
		//WARNNING_LOG("===用户 userID:%d，在中心服务器中没有===", userID);
		return true;
	}

	int ret = m_TCPSocket.CenterServerSendData(itr->second.socketIdx, msgID, pData, size, mainID, assistID, handleCode, userID, itr->second.pBufferevent);
	if (ret <= 0)
	{
		ERROR_LOG("##发送数据失败：msgID=%d##", msgID);
		return false;
	}

	return true;
}

// 向服务器发送数据
bool CCenterServerManage::SendData(ServerBaseInfo* pServer, UINT msgID, void* pData, UINT size, int userID /*= 0*/, UINT mainID/* = 0*/, UINT assistID /*= 0*/, UINT handleCode/* = 0*/)
{
	if (!pServer)
	{
		return false;
	}

	auto itr = m_serverToSocketMap.find(*pServer);
	if (itr == m_serverToSocketMap.end())
	{
		ERROR_LOG("服务器没有和中心服务器建立连接,serverType=%d,serverID=%d", pServer->serverType, pServer->serverID);
		return false;
	}

	int ret = m_TCPSocket.CenterServerSendData(itr->second.socketIdx, msgID, pData, size, mainID, assistID, handleCode, userID, itr->second.pBufferevent);
	if (ret <= 0)
	{
		ERROR_LOG("##发送数据失败：msgID=%d,mainID=%d,assistID=%d##", msgID, mainID, assistID);
		return false;
	}

	return true;
}

// 向服务器发送数据
bool CCenterServerManage::SendData(UINT uScoketIndex, UINT msgID, void* pData, UINT size, UINT handleCode, void* pBufferevent)
{
	int	ret = m_TCPSocket.CenterServerSendData(uScoketIndex, msgID, pData, size, 0, 0, handleCode, 0, pBufferevent);
	if (ret <= 0)
	{
		ERROR_LOG("##发送数据失败：uScoketIndex = %d msgID=%d ##", uScoketIndex, msgID);
		return false;
	}

	return true;
}

void CCenterServerManage::NotifyResourceChange(int userID, int resourceType, long long value, long long changeValue, int reason, int reserveData/* = 0*/, BYTE isNotifyRoom/* = 0*/)
{
	if (!IsUserOnline(userID))
	{
		return;
	}

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("玩家不存在,userID=%d", userID);
		return;
	}

	LogonNotifyResourceChange msg;

	msg.resourceType = resourceType;
	msg.value = value;
	msg.reason = reason;
	msg.changeValue = changeValue;

	SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(LogonNotifyResourceChange), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_RESOURCE_CHANGE, 0);

	// 如果玩家在游戏中，通知到游戏
	int roomID = userData.roomID;
	if (roomID == 0 || !isNotifyRoom)
	{
		// 玩家不在游戏
		return;
	}

	PlatformResourceChange msgToLoader;

	msgToLoader.resourceType = resourceType;
	msgToLoader.value = value;
	msgToLoader.changeValue = changeValue;
	msgToLoader.reason = reason;
	msgToLoader.reserveData = reserveData;
	msgToLoader.isNotifyRoom = isNotifyRoom;

	ServerBaseInfo serverInfo;
	serverInfo.serverType = SERVICE_TYPE_LOADER;
	serverInfo.serverID = roomID;

	if (!SendData(&serverInfo, CENTER_MESSAGE_COMMON_RESOURCE_CHANGE, &msgToLoader, sizeof(msgToLoader), userID))
	{
		ERROR_LOG("向游戏服发送数据失败 roomID=%d", roomID);
	}
}

// 给登录服服务器集群发送当前集群信息
void CCenterServerManage::SendDistributedSystemInfo()
{
	size_t iLogonGroupSocketSize = m_logonGroupSocket.size();
	if (iLogonGroupSocketSize <= 0)
	{
		return;
	}

	int iMainLogonServer = CUtil::GetRandNum() % iLogonGroupSocketSize;
	for (size_t i = 0; i < iLogonGroupSocketSize; i++)
	{
		PlatformDistributedSystemInfo msg;
		msg.logonGroupCount = iLogonGroupSocketSize;
		msg.logonGroupIndex = i;
		msg.mainLogonGroupIndex = iMainLogonServer;

		SendData(m_logonGroupSocket[i].socketIdx, CENTER_MESSAGE_COMMON_LOGON_GROUP_INFO, &msg, sizeof(msg), 0, m_logonGroupSocket[i].pBufferevent);
	}
}

// 比赛即将开始，给所有报名玩家发送消息通知
void CCenterServerManage::SendNotifyMatchStart(const MatchInfo& matchInfo)
{
	//获取比赛人数
	std::vector<MatchUserInfo> vecPeople;
	if (!m_pRedis->GetTimeMatchPeople(matchInfo.matchID, vecPeople))
	{
		return;
	}

	LogonNotifyStartMatchRightAway msg;
	msg.matchID = matchInfo.matchID;
	msg.gameID = matchInfo.gameID;
	msg.startTime = MATCH_START_NOTIFY_TIME;

	for (size_t i = 0; i < vecPeople.size(); i++)
	{
		SendData(vecPeople[i].userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_MATCH_NOTIFY, MSG_NTF_LOGON_MATCH_NOTIFY_START_MATCH_RIGHTAWAY, 0);
	}
}

////////////////////////////////内部系统工具//////////////////////////////////////////
void CCenterServerManage::InitRounteCheckEvent()
{
	// 设置检查redis连接定时器
	SetTimer(CENTER_TIMER_CHECK_REDIS_CONNECTION, CHECK_REDIS_CONNECTION_SECS * 1000);

	// 设置通用定时器
	SetTimer(CENTER_TIMER_NORMAL, NORMAL_TIMER_SECS * 1000);

	// 设置清理php连接定时器
	SetTimer(CENTER_TIMER_ROUTINE_CHECK_UNBINDID_SOCKET, ROUTINE_CHECK_UNBINDID_SOCKET * 1000);

	// 设置定时加载比赛信息
	SetTimer(CENTER_TIMER_LOAD_MATCH_INFO, CHECK_REDIS_LOAD_MATCH_INFO * 1000);
}

void CCenterServerManage::CheckRedisConnection()
{
	if (!m_pRedis)
	{
		ERROR_LOG("m_pRedis is NULL");
		return;
	}

	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP is NULL");
		return;
	}

	const RedisConfig& redisGameConfig = ConfigManage()->GetRedisConfig(REDIS_DATA_TYPE_GAME);

	m_pRedis->CheckConnection(redisGameConfig);

	const RedisConfig& redisPHPConfig = ConfigManage()->GetRedisConfig(REDIS_DATA_TYPE_PHP);

	m_pRedisPHP->CheckConnection(redisPHPConfig);
}

void CCenterServerManage::OnNormalTimer()
{
	//////////////////////////////中心服凌晨6点////////////////////////////////////////////
	// 检查是否跨天
	time_t currTime = time(NULL);
	int currHour = CUtil::GetHourTimeStamp(currTime);
	int lastHour = CUtil::GetHourTimeStamp(m_lastNormalTimerTime);
	if (currHour == 6 && currHour > lastHour)
	{
		OnServerCrossDay();

		int currWeekday = CUtil::GetWeekdayFromTimeStamp(currTime);

		if (currWeekday == 1)
		{
			OnServerCrossWeek();
		}
	}

	//检查是否有定时赛可以开赛
	CheckTimeMatch(currTime);

	m_lastNormalTimerTime = currTime;
}

void CCenterServerManage::RoutineCheckUnbindIDSocket()
{
	time_t currTime = time(NULL);

	m_TCPSocket.GetSocketSet(m_socketIndexVec);

	for (int i = 0; i < m_socketIndexVec.size(); i++)
	{
		int index = m_socketIndexVec[i];
		time_t llAcceptMsgTime = m_TCPSocket.GetTCPSocketInfo(index)->acceptMsgTime;

		// 清理不绑定的socket
		if (currTime - llAcceptMsgTime > CONNECT_TIME_SECS)
		{
			auto itrSocketToServerMap = m_socketToServerMap.find(index);
			if (itrSocketToServerMap == m_socketToServerMap.end() || itrSocketToServerMap->second.serverType == SERVICE_TYPE_BEGIN)
			{
				m_TCPSocket.CloseSocket(index);
			}
		}
	}
}

void CCenterServerManage::OnServerCrossDay()
{
	AUTOCOSTONCE("OnServerCrossDay");
	INFO_LOG("OnServerCrossDay");

	// 每天发送一次集群信息
	SendDistributedSystemInfo();

	// 生成内存分析
#ifdef JEMALLOC_PROFILE_MEMORY
	mallctl("prof.dump", NULL, NULL, NULL, 0);
#endif // JEMALLOC_PROFILE_MEMORY
}

void CCenterServerManage::OnServerCrossWeek()
{
	AUTOCOSTONCE("OnServerCrossWeek");
	INFO_LOG("OnServerCrossWeek");

}

// 玩家是否在线
bool CCenterServerManage::IsUserOnline(int userID)
{
	if (m_centerUserInfoMap.find(userID) != m_centerUserInfoMap.end())
	{
		return true;
	}

	return true;
}

// 检查定时赛
void CCenterServerManage::CheckTimeMatch(const time_t& currTime)
{
	std::vector<int> matchRoomVec;

	for (auto itr = m_timeMatchInfoMap.begin(); itr != m_timeMatchInfoMap.end();)
	{
		if (currTime >= itr->second.startTime)
		{
			if (llabs(currTime - itr->second.startTime) <= 120)
			{
				//时间到了，可以开始比赛了
				PlatformReqStartMatchTime msg;
				msg.gameID = itr->second.gameID;
				msg.matchID = itr->second.matchID;
				msg.minPeople = itr->second.minPeople;
				msg.costResType = itr->second.costResType;
				msg.costResNums = itr->second.costResNums;

				matchRoomVec.clear();
				ConfigManage()->GetBuyRoomInfo(msg.gameID, ROOM_TYPE_MATCH, matchRoomVec);
				if (matchRoomVec.size() <= 0)
				{
					ERROR_LOG("没有配置比赛场，gameID=%d", msg.gameID);
					m_timeMatchInfoMap.erase(itr++);
					return;
				}

				//分配一个比赛场
				int roomID = matchRoomVec[CUtil::GetRandNum() % matchRoomVec.size()];

				ServerBaseInfo serverInfo;
				serverInfo.serverType = SERVICE_TYPE_LOADER;
				serverInfo.serverID = roomID;

				if (!SendData(&serverInfo, CENTER_MESSAGE_COMMON_START_MATCH_TIME, &msg, sizeof(msg)))
				{
					ERROR_LOG("定时赛：开始比赛失败 roomID=%d", roomID);
				}
				else
				{
					INFO_LOG("开始定时赛：gameid=%d,starttime=%lld,matchID=%d", itr->second.gameID, itr->second.startTime, itr->second.matchID);
				}
			}

			m_timeMatchInfoMap.erase(itr++);
			continue;
		}
		else if (itr->second.startTime - currTime < MATCH_START_NOTIFY_TIME && itr->second.isNotified == 0)
		{
			// 比赛开始前通知一次
			SendNotifyMatchStart(itr->second);

			itr->second.isNotified = 1;
			itr++;
		}
		else if (itr->second.startTime - currTime < MATCH_START_NOTIFY_TIME)
		{
			itr++;
		}
		else
		{
			break;
		}
	}
}

/////////////////////////////////PHP/////////////////////////////////////////
bool CCenterServerManage::OnHandlePHPMessage(NetMessageHead* pNetHead, CenterServerMessageHead* pCenterHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	switch (pCenterHead->msgID)
	{
	case PLATFORM_MESSAGE_NOTICE:
	{
		return OnHandlePHPNoticeMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_REQ_ALL_USER_MAIL:
	{
		return OnHandlePHPAllUserMailMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_CLOSE_SERVER:
	{
		return OnHandlePHPCloseServerMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_OPEN_SERVER:
	{
		return OnHandlePHPOpenServerMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_SEND_HORN:
	{
		return OnHandlePHPHornMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_MASTER_DISSMISS_DESK:
	case PLATFORM_MESSAGE_FG_DISSMISS_DESK:
	{
		return OnHandlePHPDissmissDeskMessage(uIndex, pCenterHead->msgID, pData, uSize);
	}
	case PLATFORM_MESSAGE_RESOURCE_CHANGE:
	{
		return OnHandlePHPNotifyResChangeMessage(uIndex, pCenterHead->userID, pData, uSize);
	}
	case PLATFORM_MESSAGE_NOTIFY_USERID:
	{
		return OnHandlePHPNotifyOneUserMessage(uIndex, pCenterHead->userID, pData, uSize, pNetHead->uMainID, pNetHead->uAssistantID);
	}
	case PLATFORM_MESSAGE_NOTIFY_FG:
	{
		return OnHandlePHPNotifyFGMessage(uIndex, pCenterHead->userID, pData, uSize, pNetHead->uMainID, pNetHead->uAssistantID);
	}
	case PLATFORM_MESSAGE_IDENTUSER:
	{
		return OnHandlePHPSetUserMessage(uIndex, pCenterHead->userID, pData, uSize);
	}
	case PLATFORM_MESSAGE_RED_SPOT:
	{
		return OnHandlePHPNotifyUserRedspotMessage(uIndex, pCenterHead->userID, pData, uSize);
	}
	case PLATFORM_MESSAGE_RED_FG_SPOT:
	{
		return OnHandlePHPNotifyUserRedFGspotMessage(uIndex, pCenterHead->userID, pData, uSize);
	}
	case PLATFORM_MESSAGE_RELOAD_GAME_CONFIG:
	{
		return OnHandlePHPReloadGameConfigMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_PHONE_INFO:
	{
		return OnHandlePHPPhoneInfoMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_SIGN_UP_MATCH_PEOPLE:
	{
		return OnHandlePHPSignUpMatchMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_START_MATCH_PEOPLE:
	{
		return OnHandlePHPReqStartMatchMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_MODIFY_TIME_MATCH:
	{
		return OnHandlePHPReqModifyTimeMatchMessage(uIndex, pData, uSize);
	}
	case PLATFORM_MESSAGE_SIGN_UP_MATCH_TIME:
	{
		return OnHandlePHPTimeMatchPeopleChangeMessage(uIndex, pData, uSize);
	}
	default:
		break;
	}

	return false;
}

// 添加公告
bool CCenterServerManage::OnHandlePHPNoticeMessage(UINT socketIdx, void* pData, int size)
{
	if (size != sizeof(PlatformNoticeMessage))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_NOTICE, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformNoticeMessage* pMessage = (PlatformNoticeMessage*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_NOTICE, NULL, 0, PF_DATA_NULL);
		return false;
	}

	//给所有登录服发公告
	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, PLATFORM_MESSAGE_NOTICE, pData, size, 0, m_logonGroupSocket[i].pBufferevent);
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_NOTICE, NULL, 0, PF_SUCCESS);

	return true;
}

// 全服邮件通知
bool CCenterServerManage::OnHandlePHPAllUserMailMessage(UINT socketIdx, void* pData, int size)
{
	//SendData(socketIdx, PLATFORM_MESSAGE_REQ_ALL_USER_MAIL, NULL, 0, PF_SUCCESS);

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, PLATFORM_MESSAGE_REQ_ALL_USER_MAIL, NULL, 0, 0, m_logonGroupSocket[i].pBufferevent);
	}

	return true;
}

//将reids数据保存db中
bool CCenterServerManage::OnHandlePHPCloseServerMessage(UINT socketIdx, void* pData, int size)
{
	if (size != sizeof(PlatformCloseServerMessage))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_CLOSE_SERVER, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformCloseServerMessage* pMessage = (PlatformCloseServerMessage*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_CLOSE_SERVER, NULL, 0, PF_DATA_NULL);
		return false;
	}

	if (!m_pRedis)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_CLOSE_SERVER, NULL, 0, PF_REDIS_NULL);
		return false;
	}

	if (pMessage->status != SERVER_PLATFROM_STATUS_CLOSE && pMessage->status != SERVER_PLATFROM_STATUS_TEST)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_CLOSE_SERVER, NULL, 0, PF_CLOSE_STATUS_ERR);
		return false;
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_CLOSE_SERVER, NULL, 0, PF_SUCCESS);

	m_pRedis->SetServerStatus(pMessage->status);

	INFO_LOG("==========CenterServer 发送关服命令===========");

	for (auto itr = m_serverToSocketMap.begin(); itr != m_serverToSocketMap.end(); itr++)
	{
		SendData(itr->second.socketIdx, PLATFORM_MESSAGE_CLOSE_SERVER, NULL, 0, 0, itr->second.pBufferevent);
	}

	////清理所有在线玩家
	//m_centerUserInfoMap.clear();

	return true;
}

//从reids中读取数据恢复
bool CCenterServerManage::OnHandlePHPOpenServerMessage(UINT socketIdx, void* pData, int size)
{
	if (size != 0)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_OPEN_SERVER, NULL, 0, PF_SIZEERROR);
		return false;
	}

	if (!m_pRedis)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_OPEN_SERVER, NULL, 0, PF_REDIS_NULL);
		return false;
	}

	INFO_LOG("==========CenterServer 发送开服命令===========");

	m_pRedis->SetServerStatus(SERVER_PLATFROM_STATUS_NORMAL);

	//SendData(socketIdx, PLATFORM_MESSAGE_OPEN_SERVER, NULL, 0, PF_SUCCESS);

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, PLATFORM_MESSAGE_OPEN_SERVER, NULL, 0, 0, m_logonGroupSocket[i].pBufferevent);
	}

	return true;
}

// 发送喇叭
bool CCenterServerManage::OnHandlePHPHornMessage(UINT socketIdx, void* pData, int size)
{
	if (size != sizeof(PlatformHorn))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_SEND_HORN, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformHorn* pMessage = (PlatformHorn*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_SEND_HORN, NULL, 0, PF_DATA_NULL);
		return false;
	}

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, PLATFORM_MESSAGE_SEND_HORN, pData, size, 0, m_logonGroupSocket[i].pBufferevent);
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_SEND_HORN, NULL, 0, PF_SUCCESS);

	return true;
}

// 解散桌子
bool CCenterServerManage::OnHandlePHPDissmissDeskMessage(UINT socketIdx, UINT msgID, void* pData, int size)
{
	if (size != sizeof(PlatformDissmissDesk))
	{
		//SendData(socketIdx, msgID, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformDissmissDesk* pMessage = (PlatformDissmissDesk*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, msgID, NULL, 0, PF_DATA_NULL);
		return false;
	}

	if (pMessage->deskMixID <= 0 || pMessage->roomID <= 0)
	{
		//SendData(socketIdx, msgID, NULL, 0, PF_DISSMISS_DESK_ERR);
		return false;
	}

	ServerBaseInfo serverInfo;
	serverInfo.serverType = SERVICE_TYPE_LOADER;
	serverInfo.serverID = pMessage->roomID;

	if (!SendData(&serverInfo, msgID, pData, size))
	{
		//SendData(socketIdx, msgID, NULL, 0, PF_SEND_DATA_ERR);
		return false;
	}

	//SendData(socketIdx, msgID, NULL, 0, PF_DISSMISS_DESK_ERR);

	return true;
}

// 通知某个人资源变化
bool CCenterServerManage::OnHandlePHPNotifyResChangeMessage(UINT socketIdx, int userID, void* pData, int size)
{
	if (size != sizeof(PlatformResourceChange))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RESOURCE_CHANGE, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformResourceChange* pMessage = (PlatformResourceChange*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RESOURCE_CHANGE, NULL, 0, PF_DATA_NULL);
		return false;
	}

	NotifyResourceChange(userID, pMessage->resourceType, pMessage->value, pMessage->changeValue, pMessage->reason, pMessage->reserveData, pMessage->isNotifyRoom);

	return true;
}

// 向某个玩家推送消息
bool CCenterServerManage::OnHandlePHPNotifyOneUserMessage(UINT socketIdx, int userID, void* pData, int size, unsigned int mainID, unsigned int assistID)
{
	return SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, pData, size, mainID, assistID, 0);
}

// 向俱乐部玩家推送消息
bool CCenterServerManage::OnHandlePHPNotifyFGMessage(UINT socketIdx, int friendsGroupID, void* pData, int size, unsigned int mainID, unsigned int assistID)
{
	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP = NULL");
		return false;
	}

	m_pRedisPHP->GetFGMember(friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", friendsGroupID);
		return false;
	}

	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		SendData(m_memberUserIDVec[i], PLATFORM_MESSAGE_NOTIFY_USERID, pData, size, mainID, assistID, 0);
	}

	return true;
}

// 设置玩家身份
bool CCenterServerManage::OnHandlePHPSetUserMessage(UINT socketIdx, int userID, void* pData, int size)
{
	if (size != sizeof(PlatformIdentUser))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformIdentUser* pMessage = (PlatformIdentUser*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_DATA_NULL);
		return false;
	}

	if (!m_pRedis)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_REDIS_NULL);
		return false;
	}

	if (pMessage->statusValue <= 0)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_NOUSER);
		return false;
	}

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_NOUSER);
		ERROR_LOG("GetUserData failed userID:%d", userID);
		return false;
	}

	int iStatus = 0;
	if (pMessage->type == 1) //设置
	{
		iStatus = userData.status | pMessage->statusValue;
		if (pMessage->statusValue == USER_IDENTITY_TYPE_WIN && (userData.status & USER_IDENTITY_TYPE_FAIL) == USER_IDENTITY_TYPE_FAIL)
		{
			iStatus = iStatus ^ USER_IDENTITY_TYPE_FAIL;
		}
		else if (pMessage->statusValue == USER_IDENTITY_TYPE_FAIL && (userData.status & USER_IDENTITY_TYPE_WIN) == USER_IDENTITY_TYPE_WIN)
		{
			iStatus = iStatus ^ USER_IDENTITY_TYPE_WIN;
		}
	}
	else //取消
	{
		if ((userData.status & pMessage->statusValue) == pMessage->statusValue)
		{
			iStatus = userData.status ^ pMessage->statusValue;
		}
		else
		{
			//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_SUCCESS);
			return true;
		}
	}

	//设置封号功能，填写用户被封时间
	int iFinishTime = (int)time(NULL) + pMessage->otherValue;
	if (pMessage->statusValue == USER_IDENTITY_TYPE_SEAL)
	{
		if (pMessage->type == 1)
		{
			if (pMessage->otherValue <= 0)
			{
				iFinishTime = -1;
			}
			m_pRedis->SetUserSealTime(userID, iFinishTime);

			//让这个玩家断线
			SendData(userID, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, 0, 0, 0);
		}
		else if (pMessage->type == 0)
		{
			m_pRedis->SetUserSealTime(userID, 0);
		}
	}

	std::unordered_map<std::string, std::string> umap;
	umap["status"] = std::to_string(iStatus);

	if (!m_pRedis->hmset(TBL_USER, userID, umap))
	{
		ERROR_LOG("设置玩家身份失败:userID=%d", userID);
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_IDENTUSER, NULL, 0, PF_SUCCESS);

	return true;
}

// 通知小红点
bool CCenterServerManage::OnHandlePHPNotifyUserRedspotMessage(UINT socketIdx, int userID, void* pData, int size)
{
	if (size != sizeof(PlatformPHPRedSpotNotify))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_SPOT, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformPHPRedSpotNotify* pMessage = (PlatformPHPRedSpotNotify*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_SPOT, NULL, 0, PF_DATA_NULL);
		return false;
	}

	UserRedSpot userRedspot;
	if (!m_pRedis->GetUserRedSpot(userID, userRedspot))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_SPOT, NULL, 0, PF_REDSPOT_NOT_EXIST);
		return true;
	}

	if (pMessage->redspotType == PlatformPHPRedSpotNotify::PHP_REDSPOT_TYPE_EMAIL)
	{
		LogonNotifyEmailRedSpot msg;

		msg.notReadCount = userRedspot.notEMRead;
		msg.notReceivedCount = userRedspot.notEMReceived;

		SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_EMAIL_REDSPOT, 0);
	}
	else if (pMessage->redspotType == PlatformPHPRedSpotNotify::PHP_REDSPOT_TYPE_FIREND)
	{
		LogonNotifyFriendRedSpot msg;

		msg.friendListRedSpotCount = userRedspot.friendList;
		msg.notifyListRedSpotCount = userRedspot.friendNotifyList;

		SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_FRIEND_REDSPOT, 0);
	}
	else if (pMessage->redspotType == PlatformPHPRedSpotNotify::PHP_REDSPOT_TYPE_FG)
	{
		LogonFriendsGroupPushRedSpot msg;

		msg.notifyListRedSpotCount = userRedspot.FGNotifyList;

		int iSendSize = 8 + sizeof(LogonFriendsGroupPushRedSpot::MsgRedSpot) * msg.friendsGroupCount;
		SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, iSendSize, MSG_MAIN_FRIENDSGROUP_NOTIFY, MSG_NTF_LOGON_FRIENDSGROUP_REDSPOT, 0);
	}
	else
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_SPOT, NULL, 0, PF_SEND_DATA_ERR);
		return true;
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_RED_SPOT, NULL, 0, PF_SUCCESS);

	return true;
}

// 通知俱乐部小红点
bool CCenterServerManage::OnHandlePHPNotifyUserRedFGspotMessage(UINT socketIdx, int friendsGroupID, void* pData, int size)
{
	if (size != sizeof(PlatformPHPRedSpotNotify))
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_FG_SPOT, NULL, 0, PF_SIZEERROR);
		return false;
	}

	PlatformPHPRedSpotNotify* pMessage = (PlatformPHPRedSpotNotify*)pData;
	if (!pMessage)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_FG_SPOT, NULL, 0, PF_DATA_NULL);
		return false;
	}

	if (pMessage->redspotType != PlatformPHPRedSpotNotify::PHP_REDSPOT_TYPE_FG)
	{
		//SendData(socketIdx, PLATFORM_MESSAGE_RED_FG_SPOT, NULL, 0, PF_DATA_NULL);
		return false;
	}

	m_pRedisPHP->GetFGMember(friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", friendsGroupID);
		return false;
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_RED_FG_SPOT, NULL, 0, PF_SUCCESS);

	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		int userID = m_memberUserIDVec[i];
		if (!IsUserOnline(userID))
		{
			continue;
		}

		UserRedSpot userRedspot;
		if (!m_pRedis->GetUserRedSpot(userID, userRedspot))
		{
			continue;
		}

		LogonFriendsGroupPushRedSpot msg;

		msg.notifyListRedSpotCount = userRedspot.FGNotifyList;

		int iSendSize = 8 + sizeof(LogonFriendsGroupPushRedSpot::MsgRedSpot) * msg.friendsGroupCount;
		SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, iSendSize, MSG_MAIN_FRIENDSGROUP_NOTIFY, MSG_NTF_LOGON_FRIENDSGROUP_REDSPOT, 0);
	}

	return true;
}

// 加载游戏配置
bool CCenterServerManage::OnHandlePHPReloadGameConfigMessage(UINT socketIdx, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPReloadGameConfig, pData, size);

	ServerBaseInfo serverInfo;
	serverInfo.serverType = SERVICE_TYPE_LOADER;
	serverInfo.serverID = pMessage->roomID;

	if (!SendData(&serverInfo, PLATFORM_MESSAGE_RELOAD_GAME_CONFIG, NULL, 0))
	{
		ERROR_LOG("向游戏服发送数据失败 roomID=%d", pMessage->roomID);
	}

	return true;
}

// 手机注册验证成功，注册账号
bool CCenterServerManage::OnHandlePHPPhoneInfoMessage(UINT socketIdx, void* pData, int size)
{
	if (size != sizeof(PlatformPhoneInfo))
	{
		return false;
	}

	PlatformPhoneInfo* pMessage = (PlatformPhoneInfo*)pData;
	if (!pMessage)
	{
		return false;
	}

	INFO_LOG("phone=%s, passwd=%s, name=%s, headURL=%s", pMessage->phone, pMessage->passwd, pMessage->name, pMessage->headURL);

	if (strlen(pMessage->phone) >= sizeof(pMessage->phone) || strlen(pMessage->passwd) >= sizeof(pMessage->passwd)
		|| strlen(pMessage->name) >= sizeof(pMessage->name) || strlen(pMessage->headURL) >= sizeof(pMessage->headURL))
	{
		ERROR_LOG("参数异常 phone=%s, passwd=%s, name=%s, headURL=%s", pMessage->phone, pMessage->passwd, pMessage->name, pMessage->headURL);
		return false;
	}

	UserData userData;

	memcpy(userData.phone, pMessage->phone, sizeof(userData.phone));
	memcpy(userData.name, pMessage->name, sizeof(userData.name));
	memcpy(userData.headURL, pMessage->headURL, sizeof(userData.headURL));
	userData.sex = pMessage->sex;
	strcpy(userData.passwd, pMessage->passwd);
	memcpy(userData.phonePasswd, userData.passwd, sizeof(userData.phonePasswd));

	OtherConfig otherConfig;
	if (!m_pRedis->GetOtherConfig(otherConfig))
	{
		otherConfig = ConfigManage()->GetOtherConfig();
	}
	userData.money = otherConfig.registerGiveMoney;
	userData.jewels = otherConfig.registerGiveJewels;
	userData.registerType = LOGON_TEL_PHONE;
	userData.registerTime = (int)time(NULL);

	int iTempIndex = CUtil::GetRandNum() % m_logonGroupSocket.size();

	SendData(m_logonGroupSocket[iTempIndex].socketIdx, PLATFORM_MESSAGE_PHONE_INFO, &userData, sizeof(userData), 0, m_logonGroupSocket[iTempIndex].pBufferevent);

	return true;
}

// 有人报名或者退出比赛（实时赛）
bool CCenterServerManage::OnHandlePHPSignUpMatchMessage(UINT socketIdx, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPSignUpMatchPeople, pData, size);

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, PLATFORM_MESSAGE_SIGN_UP_MATCH_PEOPLE, pData, size, 0, m_logonGroupSocket[i].pBufferevent);
	}

	return true;
}

// 请求开始比赛
bool CCenterServerManage::OnHandlePHPReqStartMatchMessage(UINT socketIdx, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPReqStartMatchPeople, pData, size);

	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(pMessage->gameID);
	if (!pGameBaseInfo)
	{
		ERROR_LOG("没有配置该游戏，gameID=%d", pMessage->gameID);
		return false;
	}

	if (pGameBaseInfo->deskPeople <= 1 || pMessage->peopleCount <= 1 || pMessage->peopleCount > MAX_MATCH_PEOPLE_COUNT)
	{
		ERROR_LOG("数据配置错误,pGameBaseInfo->deskPeople=%d,pMessage->peopleCount=%d", pGameBaseInfo->deskPeople, pMessage->peopleCount);
		return false;
	}

	int iRound = 0;
	for (int i = 1; i <= MAX_MATCH_ROUND; i++)
	{
		if (pMessage->peopleCount == pGameBaseInfo->deskPeople * (int)pow(2.0, i - 1))
		{
			iRound = i;
			break;
		}
	}
	if (iRound == 0)
	{
		ERROR_LOG("比赛人数不正确,peopleCount=%d", pMessage->peopleCount);
		return false;
	}
	if (iRound != pMessage->matchRound)
	{
		ERROR_LOG("比赛轮数不正确,pMessage->matchRound=%d,iRound=%d", pMessage->matchRound, iRound);
		return false;
	}

	std::vector<int> matchRoomVec;
	ConfigManage()->GetBuyRoomInfo(pMessage->gameID, ROOM_TYPE_MATCH, matchRoomVec);
	if (matchRoomVec.size() <= 0)
	{
		ERROR_LOG("没有配置比赛场，gameID=%d", pMessage->gameID);
		return false;
	}

	//分配一个比赛场
	int roomID = matchRoomVec[CUtil::GetRandNum() % matchRoomVec.size()];

	ServerBaseInfo serverInfo;
	serverInfo.serverType = SERVICE_TYPE_LOADER;
	serverInfo.serverID = roomID;

	if (!SendData(&serverInfo, PLATFORM_MESSAGE_START_MATCH_PEOPLE, pData, size))
	{
		ERROR_LOG("向游戏服发送数据失败 roomID=%d", roomID);
	}

	//SendData(socketIdx, PLATFORM_MESSAGE_START_MATCH_PEOPLE, NULL, 0, PF_SUCCESS);

	return true;
}

// 创建、修改、删除，一个定时赛
bool CCenterServerManage::OnHandlePHPReqModifyTimeMatchMessage(UINT socketIdx, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPModifyTimeMatch, pData, size);

	INFO_LOG("比赛信息：matchID=%d,gameID=%d,startTime=%lld,type=%d",
		pMessage->matchID, pMessage->gameID, pMessage->startTime, pMessage->type);

	if (pMessage->type == 0)
	{
		MatchInfo matchInfo;
		matchInfo.matchID = pMessage->matchID;
		matchInfo.gameID = pMessage->gameID;
		matchInfo.minPeople = pMessage->minPeople;
		matchInfo.costResType = pMessage->costResType;
		matchInfo.costResNums = pMessage->costResNums;
		matchInfo.startTime = pMessage->startTime;

		if (m_timeMatchInfoMap.count(matchInfo.startTime) > 0)
		{
			ERROR_LOG("比赛时间重复，time=%lld,matchID=%d,gameID=%d", matchInfo.startTime, matchInfo.matchID, matchInfo.gameID);
		}

		m_timeMatchInfoMap[matchInfo.startTime] = matchInfo;
	}
	else if (pMessage->type == 1)
	{
		auto itr = m_timeMatchInfoMap.find(pMessage->startTime);
		if (itr == m_timeMatchInfoMap.end())
		{
			ERROR_LOG("删除的比赛信息不存在，time=%lld,matchID=%d,gameID=%d", pMessage->startTime, pMessage->matchID, pMessage->gameID);
			return false;
		}
		if (itr->second.matchID == pMessage->matchID)
		{
			m_timeMatchInfoMap.erase(itr);
		}
		else
		{
			ERROR_LOG("删除的比赛信息不存在，time=%lld,matchID=%d,gameID=%d", itr->second.startTime, itr->second.matchID, itr->second.gameID);
		}
	}
	else if (pMessage->type == 2)
	{
		auto itr = m_timeMatchInfoMap.begin();
		for (; itr != m_timeMatchInfoMap.end(); itr++)
		{
			if (itr->second.matchID == pMessage->matchID)
			{
				break;
			}
		}

		if (itr == m_timeMatchInfoMap.end())
		{
			ERROR_LOG("修改的比赛信息不存在，time=%lld,matchID=%d,gameID=%d", pMessage->startTime, pMessage->matchID, pMessage->gameID);
			return false;
		}

		//先删除
		m_timeMatchInfoMap.erase(itr);

		//再添加
		MatchInfo matchInfo;
		matchInfo.matchID = pMessage->matchID;
		matchInfo.gameID = pMessage->gameID;
		matchInfo.minPeople = pMessage->minPeople;
		matchInfo.costResType = pMessage->costResType;
		matchInfo.costResNums = pMessage->costResNums;
		matchInfo.startTime = pMessage->startTime;

		if (m_timeMatchInfoMap.count(matchInfo.startTime) > 0)
		{
			ERROR_LOG("比赛时间重复，time=%lld,matchID=%d,gameID=%d", pMessage->startTime, pMessage->matchID, pMessage->gameID);
		}

		m_timeMatchInfoMap[matchInfo.startTime] = matchInfo;
	}

	return true;
}

// 玩家报名或者退出报名（定时赛）
bool CCenterServerManage::OnHandlePHPTimeMatchPeopleChangeMessage(UINT socketIdx, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPSignUpMatchTime, pData, size);

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, PLATFORM_MESSAGE_SIGN_UP_MATCH_TIME, pData, size, 0, m_logonGroupSocket[i].pBufferevent);
	}

	return true;
}

////////////////////////////////处理通用消息//////////////////////////////////////////
bool CCenterServerManage::OnHandleCommonMessage(NetMessageHead* pNetHead, CenterServerMessageHead* pCenterHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	return true;
}

// 为每个服务器绑定socketIdx
bool CCenterServerManage::OnHandleCommonServerVerifyMessage(void* pData, UINT size, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	if (size != sizeof(PlatformCenterServerVerify))
	{
		return false;
	}

	PlatformCenterServerVerify* pMessage = (PlatformCenterServerVerify*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (!m_pRedis)
	{
		ERROR_LOG("m_pRedis is NULL");
		return false;
	}

	ServerBaseInfo serverInfo;
	serverInfo.serverType = pMessage->serverType;
	serverInfo.serverID = pMessage->serverID;

	if (m_serverToSocketMap.find(serverInfo) != m_serverToSocketMap.end())
	{
		ERROR_LOG("####登录服ID重复,pMessage->serverID = %d####", pMessage->serverID);
		SendData(uIndex, CENTER_MESSAGE_COMMON_REPEAT_ID, NULL, 0, 0, pBufferevent);
		return false;
	}

	m_socketToServerMap[uIndex] = serverInfo;
	SocketSimpleInfo socketInfo;
	socketInfo.socketIdx = uIndex;
	socketInfo.pBufferevent = pBufferevent;
	m_serverToSocketMap.insert(std::make_pair(serverInfo, socketInfo));

	//发送当前服务器集群，发送编号和数量，以及主要服务器编号
	if (pMessage->serverType == SERVICE_TYPE_LOGON)   //登录服
	{
		m_pRedis->SetLogonServerStatus(serverInfo.serverID, 1);

		m_logonGroupSocket.emplace_back(uIndex, pBufferevent);

		SendDistributedSystemInfo();
	}
	else if (pMessage->serverType == SERVICE_TYPE_LOADER)//游戏服
	{
		m_pRedis->SetRoomServerStatus(pMessage->serverID, 1);
	}

	INFO_LOG("====== serverType:%d,serverID:%d 加入集群系统 登录服集群数量:%d=========", serverInfo.serverType, serverInfo.serverID, m_logonGroupSocket.size());

	return true;
}

////////////////////////////////处理大厅服消息//////////////////////////////////////////
bool CCenterServerManage::OnHandleLogonMessage(NetMessageHead* pNetHead, CenterServerMessageHead* pCenterHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	switch (pCenterHead->msgID)
	{
	case CENTER_MESSAGE_LOGON_RESOURCE_CHANGE:
	{
		return OnHandleLogonResChangeMessage(pCenterHead->userID, pData, uSize);
	}
	case CENTER_MESSAGE_LOGON_USER_LOGON_OUT:
	{
		return OnHandleLogonUserStatusMessage(pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	case CENTER_MESSAGE_LOGON_REPEAT_USER_LOGON:
	{
		return OnHandleLogonRepeatUserMessage(pCenterHead, pData, uSize, uAccessIP, uIndex, pBufferevent);
	}
	case CENTER_MESSAGE_LOGON_RELAY_USER_MSG:
	{
		return OnHandleLogonRelayUserMessage(pNetHead, pData, uSize, pCenterHead->userID);
	}
	case CENTER_MESSAGE_LOGON_RELAY_FG_MSG:
	{
		return OnHandleLogonRelayFGMessage(pNetHead, pData, uSize, pCenterHead->userID);
	}
	case CENTER_MESSAGE_LOGON_REQ_FG_REDSPOT_MSG:
	{
		return OnHandleLogonReqFGRedSpotMessage(pData, uSize, pCenterHead->userID);
	}
	case CENTER_MESSAGE_LOGON_MASTER_DISSMISS_DESK:
	{
		return OnHandleLogonReqDissmissDeskessage(pData, uSize);
	}
	default:
	{
		WARNNING_LOG("中心服消息不存在:pCenterHead->msgID = %d", pCenterHead->msgID);
		break;
	}
	}

	return true;
}

// 登陆服资源变化
bool CCenterServerManage::OnHandleLogonResChangeMessage(int userID, void* pData, UINT size)
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

	if (!IsUserOnline(userID))
	{
		return false;
	}

	LogonNotifyResourceChange msg;

	msg.resourceType = pMessage->resourceType;
	msg.value = pMessage->value;
	msg.reason = pMessage->reason;
	msg.changeValue = pMessage->changeValue;

	SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(LogonNotifyResourceChange), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_RESOURCE_CHANGE, 0);

	return true;
}

// 玩家上下线
bool CCenterServerManage::OnHandleLogonUserStatusMessage(void* pData, UINT size, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	if (size != sizeof(PlatformUserLogonLogout))
	{
		return false;
	}

	PlatformUserLogonLogout* pMessage = (PlatformUserLogonLogout*)pData;
	if (!pMessage)
	{
		return false;
	}

	auto itrSocketToServerMap = m_socketToServerMap.find(uIndex);
	if (itrSocketToServerMap == m_socketToServerMap.end())
	{
		ERROR_LOG("#### redis移除在线玩家失败 m_socketToServerMap中登陆服索引不存在 uIndex = %d ####", uIndex);
		return false;
	}

	if (itrSocketToServerMap->second.serverType != SERVICE_TYPE_LOGON)
	{
		ERROR_LOG("#### redis移除在线玩家失败 serverType=%d 错误####", itrSocketToServerMap->second.serverType);
		return false;
	}

	if (pMessage->type == 0) //新增
	{
		CenterUserInfo userInfo;
		userInfo.userID = pMessage->userID;
		userInfo.socketIdx = uIndex;
		userInfo.isVirtual = pMessage->isVirtual;
		userInfo.pBufferevent = pBufferevent;

		m_centerUserInfoMap[pMessage->userID] = userInfo;
	}
	else  //删除
	{
		auto itr = m_centerUserInfoMap.find(pMessage->userID);
		if (itr == m_centerUserInfoMap.end())
		{
			ERROR_LOG("删除玩家不存在 pMessage->userID = %d", pMessage->userID);
			return true;
		}
		if (itr->second.socketIdx == uIndex)
		{
			m_centerUserInfoMap.erase(itr);
			m_pRedis->RemoveOnlineUser(pMessage->userID, pMessage->isVirtual);
		}
		else
		{
			WARNNING_LOG("新旧服务器不一致:新socketIdx=%d, 旧uIndex=%d", itr->second.socketIdx, uIndex);
		}
	}

	return true;
}

// 给其它登录服发送数据，踢掉旧服务器玩家
bool CCenterServerManage::OnHandleLogonRepeatUserMessage(CenterServerMessageHead* pCenterHead, void* pData, UINT size, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	if (size != 0)
	{
		ERROR_LOG("协议不对");
		return false;
	}

	auto itr = m_centerUserInfoMap.find(pCenterHead->userID);
	if (itr == m_centerUserInfoMap.end())
	{
		ERROR_LOG("踢旧玩家失败：userID = %d", pCenterHead->userID);
		return false;
	}

	if (itr->second.socketIdx == uIndex)
	{
		ERROR_LOG("踢旧玩家失败，新旧服务器一致：userID = %d", pCenterHead->userID);
		return false;
	}

	PlatformRepeatUserLogon msg;
	msg.userID = pCenterHead->userID;

	SendData(itr->second.socketIdx, CENTER_MESSAGE_LOGON_REPEAT_USER_LOGON, &msg, sizeof(msg), 0, itr->second.pBufferevent);

	return true;
}

// 转发消息给某个人
bool CCenterServerManage::OnHandleLogonRelayUserMessage(NetMessageHead* pNetHead, void* pData, UINT size, int userID)
{
	return SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, pData, size, pNetHead->uMainID, pNetHead->uAssistantID, pNetHead->uHandleCode);
}

// 转发消息给俱乐部
bool CCenterServerManage::OnHandleLogonRelayFGMessage(NetMessageHead* pNetHead, void* pData, UINT size, int friendsGroupID)
{
	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP = NULL");
		return false;
	}

	m_pRedisPHP->GetFGMember(friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", friendsGroupID);
		return false;
	}

	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		SendData(m_memberUserIDVec[i], PLATFORM_MESSAGE_NOTIFY_USERID, pData, size, pNetHead->uMainID, pNetHead->uAssistantID, pNetHead->uHandleCode);
	}

	return true;
}

// 请求给俱乐部所有玩家发送小红点
bool CCenterServerManage::OnHandleLogonReqFGRedSpotMessage(void* pData, UINT size, int friendsGroupID)
{
	if (size != sizeof(PlatformFriendsGroupPushRedSpot))
	{
		return false;
	}

	PlatformFriendsGroupPushRedSpot* pMessage = (PlatformFriendsGroupPushRedSpot*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP = NULL");
		return false;
	}

	m_pRedisPHP->GetFGMember(friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", friendsGroupID);
		return false;
	}

	LogonFriendsGroupPushRedSpot msg;
	msg.friendsGroupCount = 1;
	msg.msgRedSpot[0].friendsGroupID = pMessage->friendsGroupID;
	msg.msgRedSpot[0].deskMsgRedSpotCount = pMessage->deskMsgRedSpotCount;
	msg.msgRedSpot[0].VIPRoomMsgRedSpotCount = pMessage->VIPRoomMsgRedSpotCount;
	int iSendSize = 8 + sizeof(LogonFriendsGroupPushRedSpot::MsgRedSpot) * msg.friendsGroupCount;

	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		SendData(m_memberUserIDVec[i], PLATFORM_MESSAGE_NOTIFY_USERID, &msg, iSendSize, MSG_MAIN_FRIENDSGROUP_NOTIFY, MSG_NTF_LOGON_FRIENDSGROUP_REDSPOT, 0);
	}

	return true;
}

// 请求解散房间
bool CCenterServerManage::OnHandleLogonReqDissmissDeskessage(void* pData, UINT size)
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

	if (pMessage->deskMixID <= 0 || pMessage->roomID <= 0)
	{
		ERROR_LOG("房间不存在,deskMixID=%d,roomID=%d", pMessage->deskMixID, pMessage->roomID);
		return false;
	}

	ServerBaseInfo serverInfo;
	serverInfo.serverType = SERVICE_TYPE_LOADER;
	serverInfo.serverID = pMessage->roomID;

	return SendData(&serverInfo, PLATFORM_MESSAGE_MASTER_DISSMISS_DESK, pData, size);
}

////////////////////////////////处理游戏服消息//////////////////////////////////////////
bool CCenterServerManage::OnHandleLoaderMessage(NetMessageHead* pNetHead, CenterServerMessageHead* pCenterHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, void* pBufferevent)
{
	switch (pCenterHead->msgID)
	{
	case CENTER_MESSAGE_LOADER_RESOURCE_CHANGE:
	{
		return OnHandleLoaderResChangeMessage(pCenterHead->userID, pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_BUYDESKINFO_CHANGE:
	{
		return OnHandleLoaderBuyDeskInfoChangeMessage(pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_DESK_DISSMISS:
	{
		return OnHandleLoaderDeskDissmissMessage(pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_DESK_STATUS_CHANGE:
	{
		return OnHandleLoaderDeskStatusChangeMessage(pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_FIRECOIN_CHANGE:
	{
		return OnHandleLoaderFireCoinChangeMessage(pCenterHead->userID, pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_REWARD_ACTIVITY:
	{
		return OnHandleLoaderRewardMessage(pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH:
	{
		return OnHandleLoaderNotifyStartMatchMessage(pData, uSize);
	}
	case CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH_FAIL:
	{
		return OnHandleLoaderNotifyStartMatchFailMessage(pCenterHead->userID, pData, uSize);
	}
	default:
		WARNNING_LOG("中心服消息不存在:pCenterHead->msgID = %d", pCenterHead->msgID);
		break;
	}

	return true;
}

// 游戏服资源变化
bool CCenterServerManage::OnHandleLoaderResChangeMessage(int userID, void* pData, UINT size)
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

	if (!IsUserOnline(userID))
	{
		return true;
	}

	LogonNotifyResourceChange msg;

	msg.resourceType = pMessage->resourceType;
	msg.value = pMessage->value;
	msg.reason = pMessage->reason;
	msg.changeValue = pMessage->changeValue;

	SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(LogonNotifyResourceChange), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_RESOURCE_CHANGE, 0);

	return true;
}

// 开房列表信息变化(人数变化)
bool CCenterServerManage::OnHandleLoaderBuyDeskInfoChangeMessage(void* pData, UINT size)
{
	if (size != sizeof(PlatformDeskPeopleCountChange))
	{
		return false;
	}

	PlatformDeskPeopleCountChange* pMessage = (PlatformDeskPeopleCountChange*)pData;
	if (!pMessage)
	{
		return false;
	}

	int masterID = pMessage->masterID;
	LogonNotifyBuyDeskInfoChange msg;

	msg.changeType = LogonNotifyBuyDeskInfoChange::CHANGE_REASON_USERCOUNT_CHANGE;
	msg.currUserCount = pMessage->currUserCount;
	memcpy(msg.passwd, pMessage->roomPasswd, sizeof(msg.passwd));

	SendData(masterID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_BUYDESKINFO_CHANGE, 0);

	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP = NULL");
		return false;
	}

	if (pMessage->friendsGroupID <= 0)
	{
		return true;
	}

	m_pRedisPHP->GetFGMember(pMessage->friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", pMessage->friendsGroupID);
		return false;
	}

	// 向俱乐部所有成员推送消息
	LogonFriendsGroupDeskInfoChange msgChange;
	msgChange.friendsGroupID = pMessage->friendsGroupID;
	msgChange.operType = 0;
	msgChange.friendsGroupDeskNumber = pMessage->friendsGroupDeskNumber;
	msgChange.curPeopleCount = pMessage->currUserCount;
	msgChange.updateType = pMessage->updateType;
	msgChange.userID = pMessage->userID;
	UINT iMessageID = pMessage->friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT
		? MSG_NTF_LOGON_FRIENDSGROUP_VIPROOM_CHANGE : MSG_NTF_LOGON_FRIENDSGROUP_DESK_INFO_CHANGE;

	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		SendData(m_memberUserIDVec[i], PLATFORM_MESSAGE_NOTIFY_USERID, &msgChange, sizeof(msgChange), MSG_MAIN_FRIENDSGROUP_NOTIFY, iMessageID, 0);
	}

	return true;
}

// 游戏大结算
bool CCenterServerManage::OnHandleLoaderDeskDissmissMessage(void* pData, UINT size)
{
	if (size != sizeof(PlatformDeskDismiss))
	{
		return false;
	}

	PlatformDeskDismiss* pMessage = (PlatformDeskDismiss*)pData;
	if (!pMessage)
	{
		return false;
	}

	// 向开房列表里推送数据
	int masterID = pMessage->masterID;
	LogonNotifyBuyDeskInfoChange msg;

	msg.changeType = LogonNotifyBuyDeskInfoChange::CHANGE_REASON_DEL;
	memcpy(msg.passwd, pMessage->passwd, sizeof(msg.passwd));

	SendData(masterID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_BUYDESKINFO_CHANGE, 0);

	if (pMessage->friendsGroupID <= 0)
	{
		return true;
	}

	//更新俱乐部消息
	int friendsGroupID = pMessage->friendsGroupID;       //俱乐部id
	int friendsGroupDeskNumber = pMessage->friendsGroupDeskNumber; //俱乐部桌子号

	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP = NULL");
		return false;
	}

	m_pRedisPHP->GetFGMember(friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", friendsGroupID);
		return false;
	}

	// 推送牌桌数据
	LogonFriendsGroupDeskInfoChange msgChange;
	msgChange.friendsGroupID = friendsGroupID;
	msgChange.operType = 1;
	msgChange.friendsGroupDeskNumber = friendsGroupDeskNumber;

	//推送给所有在线成员
	UINT iMessageID = friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT
		? MSG_NTF_LOGON_FRIENDSGROUP_VIPROOM_CHANGE : MSG_NTF_LOGON_FRIENDSGROUP_DESK_INFO_CHANGE;
	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		SendData(m_memberUserIDVec[i], PLATFORM_MESSAGE_NOTIFY_USERID, &msgChange, sizeof(msgChange), MSG_MAIN_FRIENDSGROUP_NOTIFY, iMessageID, 0);
	}

	//需要自动开房，发给登陆服务器自动开房
	if (!pMessage->bDeleteDesk)
	{
		if (m_logonGroupSocket.size() <= 0)
		{
			ERROR_LOG("自动开房失败，没有登录服在线");
			return false;
		}

		PlatformAutoCreateRoom autoCreateMsg;

		autoCreateMsg.masterID = masterID;
		autoCreateMsg.friendsGroupID = friendsGroupID;
		autoCreateMsg.friendsGroupDeskNumber = friendsGroupDeskNumber;
		autoCreateMsg.roomType = pMessage->roomType;
		autoCreateMsg.gameID = pMessage->gameID;
		autoCreateMsg.maxCount = pMessage->maxCount;
		memcpy(autoCreateMsg.gameRules, pMessage->gameRules, sizeof(autoCreateMsg.gameRules));

		UINT uSendSocketIdx = 0;
		void* pBufferevent = nullptr;
		auto itr = m_centerUserInfoMap.find(masterID);
		if (itr == m_centerUserInfoMap.end()) // 房主不在，随机选择服务器
		{
			int iTempIndex = CUtil::GetRandNum() % m_logonGroupSocket.size();
			uSendSocketIdx = m_logonGroupSocket[iTempIndex].socketIdx;
			pBufferevent = m_logonGroupSocket[iTempIndex].pBufferevent;
		}
		else // 房主存在，自动开房任务交给房主的登陆服
		{
			uSendSocketIdx = itr->second.socketIdx;
			pBufferevent = itr->second.pBufferevent;
		}

		SendData(uSendSocketIdx, CENTER_MESSAGE_COMMON_AUTO_CREATEROOM, &autoCreateMsg, sizeof(autoCreateMsg), 0, pBufferevent);
	}

	return true;
}

// 局数变化
bool CCenterServerManage::OnHandleLoaderDeskStatusChangeMessage(void* pData, UINT size)
{
	if (size != sizeof(PlatformDeskGameCountChange))
	{
		return false;
	}

	PlatformDeskGameCountChange* pMessage = (PlatformDeskGameCountChange*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (!m_pRedisPHP)
	{
		ERROR_LOG("m_pRedisPHP = NULL");
		return false;
	}

	if (pMessage->friendsGroupID <= 0)
	{
		return true;
	}

	m_pRedisPHP->GetFGMember(pMessage->friendsGroupID, m_memberUserIDVec);

	if (m_memberUserIDVec.size() <= 0)
	{
		ERROR_LOG("俱乐部无成员 friendsGroupID=%d", pMessage->friendsGroupID);
		return false;
	}

	//推送数据
	LogonFriendsGroupDeskInfoChange msg;
	msg.friendsGroupID = pMessage->friendsGroupID;
	msg.operType = 0;
	msg.friendsGroupDeskNumber = pMessage->friendsGroupDeskNumber;
	msg.gameStatus = pMessage->gameStatus;
	msg.updateType = 2;

	//推送给所有在线成员
	UINT iMessageID = pMessage->friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT
		? MSG_NTF_LOGON_FRIENDSGROUP_VIPROOM_CHANGE : MSG_NTF_LOGON_FRIENDSGROUP_DESK_INFO_CHANGE;

	for (size_t i = 0; i < m_memberUserIDVec.size(); i++)
	{
		SendData(m_memberUserIDVec[i], PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_FRIENDSGROUP_NOTIFY, iMessageID, 0);
	}

	return true;
}

// 火币变化通知
bool CCenterServerManage::OnHandleLoaderFireCoinChangeMessage(int userID, void* pData, UINT size)
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

	if (!IsUserOnline(userID))
	{
		return true;
	}

	LogonFriendsGroupUserFireCoinChange msg;

	msg.friendsGroupID = pMessage->reserveData;
	msg.user[0].userID = userID;
	msg.user[0].newFireCoin = (int)pMessage->value;

	SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_FRIENDSGROUP_NOTIFY, MSG_NTF_LOGON_FRIENDSGROUP_FIRECOIN_CHANGE, 0);

	return true;
}

// 大奖通知
bool CCenterServerManage::OnHandleLoaderRewardMessage(void* pData, UINT size)
{
	SAFECHECK_MESSAGE(pMessage, LogonNotifyActivity, pData, size);

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, CENTER_MESSAGE_LOADER_REWARD_ACTIVITY, pData, size, 0, m_logonGroupSocket[i].pBufferevent);
	}

	return true;
}

// 开始比赛
bool CCenterServerManage::OnHandleLoaderNotifyStartMatchMessage(void* pData, UINT size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformLoaderNotifyStartMatch, pData, size);

	for (size_t i = 0; i < m_logonGroupSocket.size(); i++)
	{
		SendData(m_logonGroupSocket[i].socketIdx, CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH, pData, size, 0, m_logonGroupSocket[i].pBufferevent);
	}

	return true;
}

// 开始比赛失败
bool CCenterServerManage::OnHandleLoaderNotifyStartMatchFailMessage(int userID, void* pData, UINT size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformLoaderNotifyStartMatchFail, pData, size);

	LogonStartMatchFail msg;

	msg.gameID = pMessage->gameID;
	msg.matchID = pMessage->matchID;
	msg.matchType = pMessage->matchType;
	msg.reason = pMessage->reason;

	SendData(userID, PLATFORM_MESSAGE_NOTIFY_USERID, &msg, sizeof(msg), MSG_MAIN_MATCH_NOTIFY, MSG_NTF_LOGON_MATCH_NOTIFY_START_MATCH_FAIL, 0);

	return true;
}