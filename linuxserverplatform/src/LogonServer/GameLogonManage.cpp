#include "CommonHead.h"
#include "ErrorCode.h"
#include "json/json.h"
#include "AsyncEventMsg.pb.h"
#include <arpa/inet.h>
#include "GameLogonManage.h"

using namespace AsyncEventMsg;

// 预编译选项
#define FRIENDSGROUP_CRATE_ROOM_MODE	// 俱乐部创建房间，房主消耗钻石
#define HAOTIAN							// 皓天游戏，没有定义就是皓玩游戏

//////////////////////////////////////////////////////////////////////
CGameLogonManage::CGameLogonManage() : CBaseLogonServer()
{
	m_nPort = 0;
	m_uMaxPeople = 0;
	m_pUserManage = NULL;
	m_pGServerManage = NULL;
	m_lastNormalTimerTime = 0;
}

//////////////////////////////////////////////////////////////////////
CGameLogonManage::~CGameLogonManage()
{
	SAFE_DELETE(m_pUserManage);
	SAFE_DELETE(m_pGServerManage);
	m_buyRoomVec.clear();
	m_scoketMatch.clear();
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnSocketRead(NetMessageHead* pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, UINT dwHandleID)
{
	if (!pNetHead)
	{
		return false;
	}

	AUTOCOST("statistics message cost mainID: %d assistID: %d uIdentification: %d", pNetHead->uMainID, pNetHead->uAssistantID, pNetHead->uIdentification);

	switch (pNetHead->uMainID)
	{
	case MSG_MAIN_LOGON_REGISTER:
	{
		return OnHandleUserRegister(pNetHead->uAssistantID, pData, uSize, uAccessIP, uIndex, dwHandleID);
	}
	case MSG_MAIN_LOGON_LOGON:
	{
		return OnHandleUserLogonMessage(pNetHead->uAssistantID, pData, uSize, uAccessIP, uIndex, dwHandleID);
	}
	case MSG_ASS_LOGON_DESK:
	{
		return OnHandleGameDeskMessage(pNetHead->uAssistantID, pData, uSize, uAccessIP, uIndex, dwHandleID);
	}
	case MSG_MAIN_LOGON_OTHER:
	{
		return OnHandleOtherMessage(pNetHead->uAssistantID, pData, uSize, uAccessIP, uIndex, dwHandleID);
	}
	case COMMON_VERIFY_MESSAGE:
	{
		return OnHandleGServerVerifyMessage(pData, uSize, uIndex);
	}
	default:
	{
		// 在游戏消息范围内
		if (pNetHead->uMainID >= MSG_MAIN_LOADER_LOGON && pNetHead->uMainID < MSG_MAIN_LOADER_MAX)
		{
			LogonServerSocket socketInfo = GetIdentityIDBySocketIdx(uIndex);
			if (socketInfo.type == LOGON_SERVER_SOCKET_TYPE_USER)
			{
				return OnHandleGServerToGameMessage(socketInfo.identityID, pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
			else if (socketInfo.type == LOGON_SERVER_SOCKET_TYPE_GAME_SERVER)
			{
				return OnHandleGServerToUserMessage(socketInfo.identityID, pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
		}

		return false;
	}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnStart()
{
	INFO_LOG("GameLogonManage OnStart begin...");

	//初始化socket
	m_socketInfoMap.clear();

	if (m_pRedis)
	{
		m_pRedis->SetDBManage(&m_SQLDataManage);

		//清理在线人数
		std::unordered_map<std::string, std::string> umap;
		umap["curPeople"] = "0";
		m_pRedis->hmset(TBL_BASE_LOGON, ConfigManage()->m_logonServerConfig.logonID, umap);
	}

	if (m_pRedisPHP)
	{
		m_pRedisPHP->SetDBManage(&m_SQLDataManage);
	}

	if (m_pUserManage)
	{
		ERROR_LOG("m_pUserMange is already exists");
		return false;
	}

	m_pUserManage = new CLogonUserManage;
	if (!m_pUserManage)
	{
		ERROR_LOG("new LogonUserManage failed");
		return false;
	}

	if (m_pGServerManage)
	{
		ERROR_LOG("m_pGServerManage is already exists");
		return false;
	}

	m_pGServerManage = new CLogonGServerManage;
	if (!m_pGServerManage)
	{
		ERROR_LOG("new LogonGServerManage failed");
		return false;
	}

	m_lastNormalTimerTime = time(NULL);
	m_lastSendHeartBeatTime = 0;

	InitRounteCheckEvent();

	// 测试异步获取数据库数据的代码
	AsyncEventMsgSqlStatement msg;
	char sql[128] = "select userID,name,headURL,money from userInfo order by money desc limit 5";
	memcpy(msg.sql, sql, sizeof(sql));
	msg.bIsSelect = true;
	m_SQLDataManage.PushLine(&msg.dataLineHead, sizeof(AsyncEventMsgSqlStatement), ASYNC_EVENT_SQL_STATEMENT, DB_TYPE_COMMON, ASYNC_MESSAGE_DATABASE_TEST);

	INFO_LOG("GameLogonManage OnStart end.");

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnTimerMessage(UINT uTimerID)
{
	AUTOCOST("定时器耗时timerID = %d", uTimerID);
	switch (uTimerID)
	{
	case LOGON_TIMER_CHECK_REDIS_CONNECTION:
	{
		CheckRedisConnection();
		return true;
	}
	case LOGON_TIMER_ROUTINE_SAVE_REDIS:
	{
		RountineSaveRedisDataToDB(false);
		return true;
	}
	case LOGON_TIMER_ROUTINE_CHECK_UNBINDID_SOCKET:
	{
		RoutineCheckUnbindIDSocket();
		return true;
	}
	case LOGON_TIMER_NORMAL:
	{
		OnNormalTimer();
		return true;
	}
	case LOGON_TIMER_CHECK_HEARTBEAT:
	{
		CheckHeartBeat(m_lastSendHeartBeatTime, CHECK_HEAETBEAT_SECS);
		m_lastSendHeartBeatTime = time(NULL);

		return true;
	}
	default:
		break;
	}

	return false;
}

void CGameLogonManage::NotifyUserInfo(const UserData& userData)
{
	int userID = userData.userID;

	UserRedSpot userRedspot;
	if (!m_pRedis->GetUserRedSpot(userID, userRedspot))
	{
		return;
	}

	// 通知小红点
	bool bHaveRedSpot = false;

	if (userRedspot.friendList > 0 || userRedspot.friendNotifyList > 0)
	{
		// 通知好友小红点
		LogonNotifyFriendRedSpot msgFriend;

		msgFriend.friendListRedSpotCount = userRedspot.friendList;
		msgFriend.notifyListRedSpotCount = userRedspot.friendNotifyList;

		SendData(userID, &msgFriend, sizeof(msgFriend), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_FRIEND_REDSPOT, 0);

		bHaveRedSpot = true;
	}

	if (userRedspot.FGNotifyList > 0)
	{
		// 通知俱乐部小红点
		LogonFriendsGroupPushRedSpot msgFG;

		msgFG.notifyListRedSpotCount = userRedspot.FGNotifyList;

		int iSendSize = 8 + sizeof(LogonFriendsGroupPushRedSpot::MsgRedSpot) * msgFG.friendsGroupCount;
		SendData(userID, &msgFG, iSendSize, MSG_MAIN_FRIENDSGROUP_NOTIFY, MSG_NTF_LOGON_FRIENDSGROUP_REDSPOT, 0);

		bHaveRedSpot = true;
	}

	if (userRedspot.notEMRead > 0 || userRedspot.notEMReceived > 0)
	{
		// 通知邮件小红点
		LogonNotifyEmailRedSpot msgEmail;

		msgEmail.notReadCount = userRedspot.notEMRead;
		msgEmail.notReceivedCount = userRedspot.notEMReceived;

		SendData(userID, &msgEmail, sizeof(msgEmail), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_EMAIL_REDSPOT, 0);

		bHaveRedSpot = true;
	}

	// 没有小红点删除redis键值
	if (!bHaveRedSpot)
	{
		m_pRedis->DelKey(CRedisLogon::MakeKey(TBL_USER_REDSPOT, userID).c_str());
	}
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnAsynThreadResult(AsynThreadResultLine* pResultData, void* pData, UINT uSize)
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
bool CGameLogonManage::OnStop()
{
	// 处理一些大厅管理器结束相关的事件
	if (m_pUserManage)
	{
		m_pUserManage->Release();
	}

	if (m_pGServerManage)
	{
		m_pGServerManage->Release();
	}

	m_socketInfoMap.clear();

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::PreInitParameter(ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData)
{
	if (!pInitData || !pKernelData)
	{
		return false;
	}

	int logonID = ConfigManage()->GetLogonServerConfig().logonID;
	LogonBaseInfo* pLogonBaseInfo = ConfigManage()->GetLogonBaseInfo(logonID);

	if (pLogonBaseInfo == NULL)
	{
		ERROR_LOG("logonserver config error: logonID=%d", logonID);
		return false;
	}

	m_uMaxPeople = pLogonBaseInfo->maxPeople;
	m_nPort = pLogonBaseInfo->port;

	pInitData->uListenPort = m_nPort;
	pInitData->uMaxPeople = m_uMaxPeople;

	return true;
}

//////////////////////////////////////////////////////////////////////
// 走到这里的一般是底层通知业务层关闭(比如客户端断线，或者主动关闭socket之类)
bool CGameLogonManage::OnSocketClose(ULONG uAccessIP, UINT socketIdx, UINT uConnectTime)
{
	AUTOCOST("socketIdx = %d", socketIdx);

	in_addr ip_;  ip_.s_addr = uAccessIP;
	std::string ip = inet_ntoa(ip_);

	// 取socket信息
	LogonServerSocket socketInfo = GetIdentityIDBySocketIdx(socketIdx);
	if (socketInfo.identityID <= 0 || socketInfo.type <= LOGON_SERVER_SOCKET_TYPE_NO)
	{
		// 取不到信息ID也正常，可能会有连接没有绑定ID
		return true;
	}

	// 删除socketIdx
	DelSocketIdx(socketIdx);

	// 处理游戏服掉线
	if (socketInfo.type == LOGON_SERVER_SOCKET_TYPE_GAME_SERVER)
	{
		m_pGServerManage->DelGServer(socketInfo.identityID);

		INFO_LOG("游戏服掉线：roomID=%d", socketInfo.identityID);
	}
	else if (socketInfo.type == LOGON_SERVER_SOCKET_TYPE_USER)
	{
		////////////////////////////////玩家掉线//////////////////////////////////////////
		int userID = socketInfo.identityID;

		LogonUserInfo* pUser = m_pUserManage->GetUser(userID);
		if (!pUser)
		{
			ERROR_LOG("GetUser failed userID=%d", userID);
			return false;
		}

		OnUserLogout(userID);

		// 移除在线玩家，向中心服务器发送消息
		PlatformUserLogonLogout logonToCenterUserStatus;
		logonToCenterUserStatus.userID = userID;
		logonToCenterUserStatus.type = 1;
		logonToCenterUserStatus.isVirtual = pUser->isVirtual;
		SendMessageToCenterServer(CENTER_MESSAGE_LOGON_USER_LOGON_OUT, &logonToCenterUserStatus, sizeof(logonToCenterUserStatus));

		// 如果玩家正在游戏中，向游戏服发送玩家掉线消息
		int roomID = (int)m_pRedis->GetUserResNums(userID, "roomID");
		if (roomID > 0)
		{
			LogonGServerInfo* pGServer = m_pGServerManage->GetGServer(roomID);
			if (pGServer)
			{
				m_TCPSocket.SendData(pGServer->socketIdx, NULL, 0, MSG_MAIN_USER_SOCKET_CLOSE, 0, 0, 0, userID);
			}
		}

		// 给PHP发送消息
		if (!pUser->isVirtual)
		{
			SendHTTPUserLogonLogout(userID, 1);
		}

		// 写登出记录
		if (pUser->isVirtual == 0)
		{
			char tableName[128] = "";
			ConfigManage()->GetTableNameByDate(TBL_STATI_LOGON_LOGOUT, tableName, sizeof(tableName));

			time_t currTime = time(NULL);
			BillManage()->WriteBill(&m_SQLDataManage, "INSERT INTO %s (userID,type,time,ip) VALUES (%d,%d,%d,'%s')",
				tableName, userID, 2, (int)currTime, ip.c_str());

			long long lastCalcOnlineToTime = 0;
			if ((lastCalcOnlineToTime = m_pRedis->GetUserResNums(userID, "lastCalcOnlineToTime")) > 0)
			{
				m_pRedis->AddUserResNums(userID, "allOnlineToTime", currTime - lastCalcOnlineToTime);
				m_pRedis->SetUserResNums(userID, "lastCalcOnlineToTime", 0);
			}

			m_pRedis->SetUserInfo(userID, " IsOnline 0 ");
		}

		// 清理玩家内存
		m_pUserManage->DelUser(userID);

		// redis记录数据
		m_pRedis->SetLogonServerCurrPeopleCount(ConfigManage()->GetLogonServerConfig().logonID,
			m_pUserManage->GetUserCount(), m_TCPSocket.GetCurSocketSize());

		//退出比赛场页面
		m_scoketMatch.erase(socketIdx);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnHandleUserRegister(unsigned int assistID, void* pData, int size, unsigned long accessIP, unsigned int socketIdx, long handleID)
{
	if (size != sizeof(LogonRequestRegister) && size != sizeof(LogonRequestRegister) - 1)
	{
		return false;
	}

	LogonRequestRegister* pMessage = (LogonRequestRegister*)pData;
	if (!pMessage)
	{
		return false;
	}

	BYTE byPlatformType = 0;//默认0   1是ios  2安卓   3、win32
	if (size == sizeof(LogonRequestRegister))
	{
		byPlatformType = pMessage->byPlatformType;
	}

	const char* ip = m_TCPSocket.GetSocketIP(socketIdx);

	if (pMessage->byFastRegister == LOGON_QUICK || pMessage->byFastRegister >= LOGON_NR)
	{
		ERROR_LOG("invalid Register type byFastRegister=%d ", pMessage->byFastRegister);
		return true;
	}

	// 是否第三方登录，第三方登陆验证账号唯一性
	bool isTrd = (pMessage->byFastRegister == LOGON_WEICHAT || pMessage->byFastRegister == LOGON_QQ) ? true : false;
	if (isTrd)
	{
		std::string accountMD5 = pMessage->szAccountInfo;
		CUtil::MD5(accountMD5);
		if (strcmp(accountMD5.c_str(), pMessage->szPswd))
		{
			ERROR_LOG("非法账号，因为szAccountInfo=%s和szPswd=%s不匹配", pMessage->szAccountInfo, pMessage->szPswd);
			return false;
		}
	}

	// 检查是否有非法字符
	if (!CUtil::CheckString(pMessage->szAccount, sizeof(pMessage->szAccount)))
	{
		ERROR_LOG("invalid string account=%s passwd=%s", pMessage->szAccount, pMessage->szPswd);
		return false;
	}

	int accountLen = strlen(pMessage->szAccount);
	if (accountLen < 1 || accountLen >= sizeof(pMessage->szAccount))
	{
		ERROR_LOG("账号长度异常 pMessage->szAccount=%s,len=%d", pMessage->szAccount, accountLen);
		m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_NOT_MATCH, 0);
		return false;
	}

	if (strlen(pMessage->szPswd) != USER_MD5_PASSWD_LEN)
	{
		ERROR_LOG("密码长度不正确:pMessage->szPswd=%s", pMessage->szPswd);
		m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_PASSWD_FALSE, 0);
		return false;
	}

	// 先生成一个token
	char token[MAX_TOKEN_BUF_SIZE + 1] = "";
	CUtil::GetUuid(token, MAX_TOKEN_BUF_SIZE);

	int userID = 0;
	BYTE byPhoneFastRegister = LOGON_TEL_PHONE;
	char szPhoneAccountInfo[64] = "";//手机绑定微信或者qq的唯一标识

	if (pMessage->byFastRegister == LOGON_NORMAL) //普通注册登录
	{
		if (pMessage->byStatus == LOGON_REGISTER) //注册
		{
			if (accountLen < MIN_USER_ACCOUNT_LEN || accountLen >= MAX_USER_ACCOUNT_LEN)
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_NOT_MATCH, 0);
				return false;
			}

			if (CUtil::IsContainDirtyWord(pMessage->szAccount) == true)
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_HAVE_DIRTYWORD, 0);
				return true;
			}

			if (strstr(pMessage->szAccount, "Phone") != NULL)	//Phone起头的字符用于qq和微信，不允许注册
			{
				m_TCPSocket.SendData(socketIdx, pMessage, sizeof(*pMessage), MSG_MAIN_LOGON_REGISTER, assistID, ERROR_HAVE_DIRTYWORD, 0);
				return true;
			}

			if (m_pRedis->IsAccountExists(pMessage->szAccount))
			{
				m_TCPSocket.SendData(socketIdx, pMessage, sizeof(*pMessage), MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_EXISTS, 0);
				return true;
			}
		}
		else   // 登录验证
		{
			UserData userData;

			userID = m_pRedis->GetUserIDByAccount(pMessage->szAccount);
			if (userID <= 0)
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_NOTEXISTS, 0);
				return true;
			}

			if (!m_pRedis->GetUserData(userID, userData))
			{
				ERROR_LOG("GetUserData(%d) failed", userID);
				return true;
			}

			if (strcmp(pMessage->szAccount, userData.account))
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_NOT_MATCH, 0);
				return true;
			}

			if (strcmp(userData.passwd, pMessage->szPswd))
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_PASSWD_FALSE, 0);
				return true;
			}
		}
	}
	else if (pMessage->byFastRegister == LOGON_TEL_PHONE) //手机登陆
	{
		if (pMessage->byStatus == LOGON_LOGIN) //登陆验证
		{
			UserData userData;

			userID = m_pRedis->GetUserIDByPhone(pMessage->szAccount);
			if (userID <= 0)
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_PHONE_NO_BIND, 0);
				return true;
			}

			if (!m_pRedis->GetUserData(userID, userData))
			{
				ERROR_LOG("GetUserData(%d) failed", userID);
				return true;
			}

			if (strcmp(pMessage->szAccount, userData.phone))
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_NOT_MATCH, 0);
				return true;
			}

			if (strcmp(userData.phonePasswd, pMessage->szPswd))
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_PASSWD_FALSE, 0);
				return true;
			}

			byPhoneFastRegister = userData.registerType;
			memcpy(szPhoneAccountInfo, userData.accountInfo, sizeof(szPhoneAccountInfo));
		}
		else
		{
			ERROR_LOG("不支持的登陆类型::szAccount = %s", pMessage->szAccount);
			return false;
		}
	}
	else if (pMessage->byFastRegister == LOGON_TEL_XIANLIAO)
	{
		if (pMessage->byStatus == LOGON_LOGIN) //登陆验证
		{
			UserData userData;

			userID = m_pRedis->GetUserIDByXianLiao(pMessage->szAccount);
			if (userID <= 0)
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_XIANLIAO_NO_BIND, 0);
				return true;
			}

			if (!m_pRedis->GetUserData(userID, userData))
			{
				ERROR_LOG("GetUserData(%d) failed", userID);
				return true;
			}

			if (strcmp(pMessage->szAccount, userData.xianliao))
			{
				m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_REGISTER, assistID, ERROR_ACCOUNT_NOT_MATCH, 0);
				return true;
			}

			byPhoneFastRegister = userData.registerType;
			memcpy(szPhoneAccountInfo, userData.accountInfo, sizeof(szPhoneAccountInfo));
		}
		else
		{
			ERROR_LOG("不支持的登陆类型::szAccount = %s", pMessage->szAccount);
			return false;
		}
	}

	//// 注册
	UserData newUserData;

	newUserData.userID = userID;
	memcpy(newUserData.account, pMessage->szAccount, sizeof(newUserData.account));
	memcpy(newUserData.accountInfo, pMessage->szAccountInfo, sizeof(newUserData.accountInfo));
	memcpy(newUserData.name, pMessage->szAccount, sizeof(newUserData.name));
	memcpy(newUserData.passwd, pMessage->szPswd, sizeof(newUserData.passwd));
	if (pMessage->szHeadUrl[0] != '\0')
	{
		pMessage->szHeadUrl[sizeof(pMessage->szHeadUrl) - 1] = '\0';
		memcpy(newUserData.headURL, pMessage->szHeadUrl, sizeof(newUserData.headURL));
	}
	newUserData.sex = pMessage->bySex;

	// 注册时间和ip
	newUserData.registerTime = (int)time(NULL);
	if (ip)
	{
		strcpy(newUserData.registerIP, ip);
	}
	newUserData.registerType = pMessage->byFastRegister;

	// 皓玩游戏有自己的游戏名字，无需每次自动更新微信信息
	if (isTrd)
	{
		newUserData.userID = m_pRedis->GetRelevanceTrdUid(pMessage->szPswd);
		if (newUserData.userID > 0) //更新第三方信息
		{
#ifdef HAOTIAN
			std::unordered_map<std::string, std::string> umap;
			umap["headURL"] = newUserData.headURL;
			umap["sex"] = CUtil::Tostring(newUserData.sex >= 2 ? 0 : newUserData.sex);
			umap["name"] = newUserData.name;
			umap["accountInfo"] = newUserData.accountInfo;
			m_pRedis->hmset(TBL_USER, newUserData.userID, umap);
#endif // HAOTIAN
		}
	}

	// 游客注册
	if (pMessage->byFastRegister == LOGON_VISITOR)
	{
		newUserData.userID = m_pRedis->GetVisitorID(pMessage->szPswd);
	}

	// 用户注册
	if (newUserData.userID == 0)
	{
		OtherConfig otherConfig;
		if (!m_pRedis->GetOtherConfig(otherConfig))
		{
			otherConfig = ConfigManage()->GetOtherConfig();
		}

		if (!IsIpRegister(otherConfig, newUserData.registerIP))
		{
			ERROR_LOG("此ip已经不能注册,ip=%s,name=%s,iplimitCount=%d", newUserData.registerIP, newUserData.name, otherConfig.IPRegisterLimitCount);
			return false;
		}

		newUserData.money = otherConfig.registerGiveMoney;
		newUserData.jewels = otherConfig.registerGiveJewels;

		if (isTrd == false)
		{
			std::string headURL = GetRandHeadURLBySex(newUserData.sex);
			strcpy(newUserData.headURL, headURL.c_str());
		}

		// 保存到redis
		int ret = m_pRedis->Register(newUserData, newUserData.registerType);
		if (ret < 0)
		{
			ERROR_LOG("m_pRedis->Register failed code:%d", ret);
			return false;
		}

		newUserData.userID = ret;

		OnUserRegister(newUserData);
	}

	m_pRedis->SetUserToken(newUserData.userID, token);

	LogonResponseRegister msg;
	msg.userID = newUserData.userID;
	memcpy(msg.szToken, token, strlen(token));
	if (pMessage->byFastRegister == LOGON_TEL_PHONE || pMessage->byFastRegister == LOGON_TEL_XIANLIAO) //手机登陆或者闲聊登陆
	{
		msg.byFastRegister = byPhoneFastRegister;
		memcpy(msg.szAccountInfo, szPhoneAccountInfo, sizeof(msg.szAccountInfo));
	}
	else
	{
		memcpy(msg.szAccountInfo, newUserData.accountInfo, sizeof(msg.szAccountInfo));
		msg.byFastRegister = newUserData.registerType;
	}

	m_TCPSocket.SendData(socketIdx, &msg, sizeof(msg), MSG_MAIN_LOGON_REGISTER, assistID, 0, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnHandleUserLogonMessage(int assistID, void* pData, int size, unsigned long accessIP, unsigned int socketIdx, long handleID)
{
	if (assistID == MSG_ASS_LOGON_LOGON)
	{
		return OnHanleUserLogon(pData, size, accessIP, socketIdx, 0);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnHanleUserLogon(void* pData, int size, unsigned long accessIP, unsigned int socketIdx, long handleID)
{
	if (size != sizeof(LogonRequestLogon) && size != sizeof(LogonRequestLogon) - 1)
	{
		return false;
	}

	LogonRequestLogon* pMessage = (LogonRequestLogon*)pData;
	if (!pMessage)
	{
		return false;
	}

	//保证缓冲区正常
	pMessage->szMacAddr[sizeof(pMessage->szMacAddr) - 1] = '\0';
	pMessage->szToken[sizeof(pMessage->szToken) - 1] = '\0';

	BYTE byPlatformType = 0;//默认0   1是ios  2安卓   3、win32
	if (size == sizeof(LogonRequestLogon))
	{
		byPlatformType = pMessage->byPlatformType;
	}

	int userID = pMessage->iUserID;

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("GetUserData(%d) failed", userID);
		return false;
	}

	const char* ip = m_TCPSocket.GetSocketIP(socketIdx);

	/*if (!userData.isVirtual && strlen(userData.macAddr) > 0 && strcmp(userData.macAddr, pMessage->szMacAddr))
	{
		ERROR_LOG("物理地址不匹配:new=%s,old=%s,userID=%d,ip=%s", pMessage->szMacAddr, userData.macAddr, userID, ip);
		m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_LOGON, MSG_ASS_LOGON_LOGON, ERROR_MAC_NOTMATCH, 0);
		return false;
	}*/

	if (!userData.isVirtual)
	{
		// 获取服务器状态
		int serverStatus = 0;
		m_pRedis->GetServerStatus(serverStatus);
		if (serverStatus == SERVER_PLATFROM_STATUS_CLOSE)
		{
			m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_LOGON, MSG_ASS_LOGON_LOGON, ERROR_SERVER_CLOSE, 0);
			return true;
		}
	}

	// 清理玩家状态，保证游戏正确
	RoomBaseInfo roomBasekInfo;
	RoomBaseInfo* pRoomBaseInfo = NULL;
	if (m_pRedis->GetRoomBaseInfo(userData.roomID, roomBasekInfo))
	{
		pRoomBaseInfo = &roomBasekInfo;
	}
	else
	{
		pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(userData.roomID);
	}
	int deskMixID = userData.roomID * MAX_ROOM_HAVE_DESK_COUNT + userData.deskIdx;
	PrivateDeskInfo privateDeskInfo;
	if ((userData.roomID != 0 || userData.deskIdx != INVALID_DESKIDX)
		&& pRoomBaseInfo && pRoomBaseInfo->type != ROOM_TYPE_GOLD && pRoomBaseInfo->type != ROOM_TYPE_MATCH
		&& !m_pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		INFO_LOG("Logon Fail deskMixID %d userID %d roomID %d deskIdx %d", deskMixID, userID, userData.roomID, userData.deskIdx);
		m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		m_pRedis->SetUserRoomID(userID, 0);
		userData.roomID = 0;
		userData.deskIdx = INVALID_DESKIDX;
	}
	if (pRoomBaseInfo && pRoomBaseInfo->type == ROOM_TYPE_GOLD && userData.roomID != 0)
	{
		//判断金币是否满足进入条件
		bool bCanSit = true;
		if (pRoomBaseInfo->minPoint > 0 && userData.money < pRoomBaseInfo->minPoint)
		{
			ERROR_LOG("user money is not enough usermoney=%lld minMoney=%d", userData.money, pRoomBaseInfo->minPoint);
			bCanSit = false;
		}
		else if (pRoomBaseInfo->minPoint > 0 && pRoomBaseInfo->maxPoint > pRoomBaseInfo->minPoint&& userData.money > pRoomBaseInfo->maxPoint)
		{
			ERROR_LOG("user money is too much usermoney=%lld maxMoney=%d", userData.money, pRoomBaseInfo->maxPoint);
			bCanSit = false;
		}
		if (!bCanSit)
		{
			m_pRedis->SetUserRoomID(userID, 0);
			userData.roomID = 0;
		}
	}
	if (userData.roomID > 0 && !IsRoomIDServerExists(userData.roomID))
	{
		m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		m_pRedis->SetUserRoomID(userID, 0);
		userData.roomID = 0;
		userData.deskIdx = INVALID_DESKIDX;
	}

	time_t currTime = time(NULL);

	//检查是否封号
	if (userData.sealFinishTime != 0)
	{
		int iCurTime = (int)currTime;
		if (iCurTime >= userData.sealFinishTime && userData.sealFinishTime > 0) //解封时间到了，解封账号
		{
			int iStatus = userData.status ^ USER_IDENTITY_TYPE_SEAL;
			std::string cmdLine;
			char statusBuff[60] = { 0 };
			memset(statusBuff, 0, sizeof(statusBuff));
			sprintf(statusBuff, " status %d sealFinishTime 0 ", iStatus);
			cmdLine = statusBuff;
			if (!m_pRedis->SetUserInfo(userID, cmdLine))
			{
				ERROR_LOG("SetUserInfo failed userID=%d, cmd=%s", userID, cmdLine.c_str());
			}
		}
		else
		{
			LogonResponseFail msg;
			msg.byType = 0;
			msg.iRemainTime = userData.sealFinishTime < 0 ? -1 : userData.sealFinishTime - iCurTime;
			m_TCPSocket.SendData(socketIdx, &msg, sizeof(msg), MSG_MAIN_LOGON_LOGON, MSG_ASS_LOGON_LOGON, ERROR_ACCOUNT_SEAL, 0);
			return false;
		}
	}

	// 检查token
	if (!userData.isVirtual && strcmp(pMessage->szToken, userData.token))
	{
		m_TCPSocket.SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_LOGON, MSG_ASS_LOGON_LOGON, ERROR_ROOM_TOKEN_NOTMATCH, 0);
		ERROR_LOG("token error userID=%d", userID);
		return false;
	}

	// 玩家是否登录了
	LogonUserInfo* pUser = m_pUserManage->GetUser(userID);
	if (pUser)
	{
		int oldSocketIdx = pUser->socketIdx;
		if (strcmp(pMessage->szMacAddr, userData.macAddr))
		{
			// 不同设备
			m_TCPSocket.SendData(oldSocketIdx, NULL, 0, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_USER_SQUEEZE, 0, 0);
		}
		m_TCPSocket.CloseSocket(oldSocketIdx);
		DelSocketIdx(oldSocketIdx);
	}
	else if (m_pRedis->IsUserOnline(userID))
	{
		//玩家已经在其它服务器登录，踢掉旧服务器的
		SendMessageToCenterServer(CENTER_MESSAGE_LOGON_REPEAT_USER_LOGON, NULL, 0, userID);

		pUser = new LogonUserInfo;
		m_pUserManage->AddUser(userID, pUser);
	}
	else
	{
		pUser = new LogonUserInfo;
		m_pUserManage->AddUser(userID, pUser);
	}

	if (pMessage->szMacAddr[0] == '\0')
	{
		strcpy(pMessage->szMacAddr, REDIS_STR_DEFAULT);
	}

	if (pMessage->szToken[0] == '\0')
	{
		strcpy(pMessage->szToken, REDIS_STR_DEFAULT);
	}

	// 保存玩家的mac地址、ip、token、在线状态
	std::string cmdLine = " macAddr ";
	cmdLine += pMessage->szMacAddr;
	cmdLine += " token ";
	cmdLine += pMessage->szToken;
	if (ip && strlen(ip) > 0)
	{
		cmdLine += " logonIP ";
		cmdLine += ip;
		strcpy(userData.logonIP, ip);
	}
	if (!userData.isVirtual)
	{
		cmdLine += " IsOnline 1 ";
		cmdLine += " lastCalcOnlineToTime ";
		cmdLine += CUtil::Tostring(currTime);
	}
	m_pRedis->SetUserInfo(userID, cmdLine);

	// 检查是否跨天
	int lastCrossDayTime = userData.lastCrossDayTime;
	if (CUtil::GetDateFromTimeStamp(currTime) > CUtil::GetDateFromTimeStamp(lastCrossDayTime))
	{
		OnUserCrossDay(userID, (int)currTime);
	}

	pUser->userID = userID;
	pUser->socketIdx = socketIdx;
	pUser->isVirtual = userData.isVirtual;

	// 将玩家的socketIdx和userID关联
	m_socketInfoMap[socketIdx] = LogonServerSocket(LOGON_SERVER_SOCKET_TYPE_USER, userID);

	// 向中心服发送消息
	PlatformUserLogonLogout logonToCenterUserStatus;
	logonToCenterUserStatus.userID = userID;
	logonToCenterUserStatus.type = 0;
	logonToCenterUserStatus.isVirtual = userData.isVirtual;
	SendMessageToCenterServer(CENTER_MESSAGE_LOGON_USER_LOGON_OUT, &logonToCenterUserStatus, sizeof(logonToCenterUserStatus));

	// 给PHP发送消息
	if (!pUser->isVirtual)
	{
		SendHTTPUserLogonLogout(userID, 0);
	}

	LogonResponseLogon msg;
	memcpy(msg.name, userData.name, sizeof(msg.name));
	memcpy(msg.phone, userData.phone, sizeof(msg.phone));
	msg.sex = userData.sex;
	msg.money = userData.money;
	msg.jewels = userData.jewels;
	msg.roomID = userData.roomID;
	msg.deskIdx = userData.deskIdx;
	memcpy(msg.logonIP, userData.logonIP, sizeof(msg.logonIP));
	memcpy(msg.headURL, userData.headURL, sizeof(msg.headURL));
	msg.bankMoney = userData.bankMoney;
	memcpy(msg.bankPasswd, userData.bankPasswd, sizeof(msg.bankPasswd));
	memcpy(msg.longitude, userData.Lng, sizeof(msg.longitude));
	memcpy(msg.latitude, userData.Lat, sizeof(msg.latitude));
	memcpy(msg.address, userData.address, sizeof(msg.address));
	memcpy(msg.motto, userData.motto, sizeof(msg.motto));
	msg.visitor = userData.registerType == LOGON_VISITOR ? 1 : 0;
	msg.combineMatchID = userData.combineMatchID;
	msg.matchStatus = userData.matchType == MATCH_TYPE_PEOPLE ? userData.matchStatus : USER_MATCH_STATUS_NORMAL;

	// 登录成功
	m_TCPSocket.SendData(socketIdx, &msg, sizeof(msg), MSG_MAIN_LOGON_LOGON, MSG_ASS_LOGON_LOGON, 0, 0);

	// 添加在线玩家，向中心服务器发送消息
	m_pRedis->AddOnlineUser(userID, userData.isVirtual);
	m_pRedis->SetLogonServerCurrPeopleCount(ConfigManage()->GetLogonServerConfig().logonID,
		m_pUserManage->GetUserCount(), m_TCPSocket.GetCurSocketSize());

	// 通知玩家信息
	OnUserLogon(userData);

	// 写登录记录
	if (userData.isVirtual == 0)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_LOGON_LOGOUT, tableName, sizeof(tableName));

		BillManage()->WriteBill(&m_SQLDataManage, "INSERT INTO %s (userID,type,time,ip,platformType,macAddr) VALUES (%d,%d,%d,'%s',%d,'%s')",
			tableName, userID, 1, (int)currTime, ip, byPlatformType, pMessage->szMacAddr);
	}

	return true;
}
//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnHandleGameDeskMessage(int assistID, void* pData, int size, unsigned long accessIP, unsigned int socketIdx, long handleID)
{
	LogonServerSocket socketInfo = GetIdentityIDBySocketIdx(socketIdx);
	if (socketInfo.type != LOGON_SERVER_SOCKET_TYPE_USER)
	{
		ERROR_LOG("非玩家socket，禁止发送此消息 socketIdx:%d", socketIdx);
		return false;
	}

	int userID = socketInfo.identityID;
	if (userID <= 0)
	{
		ERROR_LOG("没有绑定玩家id的异常 socketIdx:%d", socketIdx);
		return false;
	}

	switch (assistID)
	{
	case MSG_ASS_LOGON_BUY_DESK:
	{
		return OnHandleUserBuyDesk(userID, pData, size);
	}
	case MSG_ASS_LOGON_ENTER_DESK:
	{
		return OnHandleUserEnterDesk(userID, pData, size);
	}
	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnHandleUserBuyDesk(int userID, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, LogonRequestBuyDesk, pData, size);

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("GetUserData failed userID=%d", userID);
		return false;
	}

	// 比赛场判断
	if ((userData.matchType == MATCH_TYPE_TIME && userData.matchStatus == USER_MATCH_STATUS_AFTER_BEGIN
		|| userData.matchType == MATCH_TYPE_PEOPLE && userData.matchStatus != USER_MATCH_STATUS_NORMAL) && pMessage->masterNotPlay == 0)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_MATCH_PLAYING);
		return true;
	}

	// 玩家在游戏中的话则无法创建房间，但是可以代开
	if (userData.roomID > 0 && pMessage->masterNotPlay == 0)
	{
		LogonResponseBuyDesk msg;
		msg.roomID = userData.roomID;
		SendData(userID, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CANNOT_BUYDESK_INROOM);

		//告诉正确的位置
		RoomBaseInfo* pUserRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(userData.roomID);
		if (pUserRoomBaseInfo)
		{
			if (pUserRoomBaseInfo->type != ROOM_TYPE_GOLD && pUserRoomBaseInfo->type != ROOM_TYPE_MATCH)
			{
				if (userData.deskIdx == INVALID_DESKIDX)
				{
					m_pRedis->SetUserRoomID(userID, 0);
				}
				else
				{
					ERROR_LOG("用户已经在房间userID = %d,roomID:%d,deskIdx=%d", userID, userData.roomID, userData.deskIdx);
				}
			}
		}
		else
		{
			m_pRedis->SetUserRoomID(userID, 0);
			m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		}
		return true;
	}

	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(pMessage->gameID);
	if (!pGameBaseInfo)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG);
		return true;
	}

	ConfigManage()->GetBuyRoomInfo(pMessage->gameID, pMessage->roomType, m_buyRoomVec);
	if (m_buyRoomVec.size() <= 0)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG);
		return true;
	}
	int iBuyRoomID = m_buyRoomVec[CUtil::GetRandNum() % m_buyRoomVec.size()];

	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(iBuyRoomID);
	if (!pRoomBaseInfo)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG);
		ERROR_LOG("购买桌子失败roomID=%d", iBuyRoomID);
		return true;
	}

	if (!IsRoomIDServerExists(iBuyRoomID))
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_GAME_NO_START);
		return true;
	}

	int	friendsGroupID = pMessage->friendsGroupID;
	int friendsGroupDeskNumber = pMessage->friendsGroupDeskNumber;
	int masterID = 0;

	// 检查购买桌子次数
	OtherConfig otherConfig;
	if (!m_pRedis->GetOtherConfig(otherConfig))
	{
		otherConfig = ConfigManage()->GetOtherConfig();
	}
	if (friendsGroupID == 0 && userData.buyingDeskCount >= otherConfig.buyingDeskCount)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_BUY_DESK_TOPLIMIT);
		return true;
	}

	// 俱乐部vip房间只能俱乐部创建
	if (friendsGroupID > 0 && friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT&& pRoomBaseInfo->type != ROOM_TYPE_FG_VIP)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM);
		return true;
	}

	//查找是否有这个配置
	BuyGameDeskInfo buyGameDeskInfo;
	BuyGameDeskInfo* pBuyGameDeskInfo = NULL;
	BuyGameDeskInfoKey buyDekKey(pMessage->gameID, pMessage->count, pRoomBaseInfo->type);
	if (m_pRedis->GetBuyGameDeskInfo(buyDekKey, buyGameDeskInfo))
	{
		pBuyGameDeskInfo = &buyGameDeskInfo;
	}
	else
	{
		pBuyGameDeskInfo = ConfigManage()->GetBuyGameDeskInfo(buyDekKey);
	}
	if (!pBuyGameDeskInfo)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG);
		return false;
	}

	//解析创建房间规则
	int iJsonRS = 0;
	int buyGameCount = pBuyGameDeskInfo->count;
	int payType = PAY_TYPE_NORMAL;
	bool bAlreadyCost = false;
	int needCostNums = 0;
	int minPoint = 0;
	int kickPoint = 0;
	int basePoint = 0;
	int cqFriendsGroupID = 0;
	if (strlen(pMessage->gameRules) >= MAX_BUY_DESK_JSON_LEN)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_ILLEGAL_OPER);
		return false;
	}
	if (pMessage->gameRules[0] != '\0')
	{
		Json::Reader reader;
		Json::Value value;
		if (!reader.parse(pMessage->gameRules, value))
		{
			ERROR_LOG("parse gameRules(%s) failed", pMessage->gameRules);
			return false;
		}

		payType = value["pay"].asInt();
		minPoint = value["mPoint"].asInt();
		kickPoint = value["lPoint"].asInt();
		iJsonRS = value["rs"].asInt();
		basePoint = value["bPoint"].asInt();
		cqFriendsGroupID = value["FGID"].asInt();
	}

	////////////////////////////////同步数据(俱乐部不能同时购买牌桌)//////////////////////////////////////////
	char redisLockKey[48] = "";
	sprintf(redisLockKey, "%s|%d", TBL_FG_BUY_DESK, friendsGroupID);
	CRedisLock redisLockFG(m_pRedisPHP->GetRedisContext(), redisLockKey, 5, false);

	//创建俱乐部房间，返回俱乐部群主，创建vip房间会自动分配牌桌
	if (friendsGroupID != 0)
	{
		// 判断俱乐部是否存在
		if (!m_pRedisPHP->IsCanJoinFriendsGroupRoom(userID, friendsGroupID))
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_NOT_EXISTS);
			return true;
		}

		// 获取俱乐部信息
		BYTE userPower = m_pRedisPHP->GetUserPower(friendsGroupID, userID);
		masterID = m_pRedisPHP->GetFriendsGroupMasterID(friendsGroupID);

		redisLockFG.Lock();

		int ret = m_pRedis->IsCanCreateFriendsGroupRoom(userID, friendsGroupID, userPower, friendsGroupDeskNumber);
		if (ret == 1)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_MANAGER);
			return true;
		}
		else if (ret == 2)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_ERR_CREATE_ROOM);
			return true;
		}
	}

	//特殊游戏判断，猜拳
	if (cqFriendsGroupID > 0 && pRoomBaseInfo->gameID == 37550102)
	{
		long long carryFireCoin = 0;
		if (!m_pRedisPHP->GetUserFriendsGroupMoney(cqFriendsGroupID, userID, carryFireCoin))
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_MANAGER);
			return true;
		}
	}

	UserData masterData;
#ifdef FRIENDSGROUP_CRATE_ROOM_MODE
	if (friendsGroupID != 0)
	{
		if (!m_pRedis->GetUserData(masterID, masterData))
		{
			ERROR_LOG("创建俱乐部房间，但是群主不存在,masterID = %d", masterID);
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_ERR_CREATE_ROOM);
			return true;
		}
	}
#else
	masterID = 0;
#endif

	//判断房间归属权
	int buyUserID = masterID == 0 ? userID : masterID;
	UserData buyUserData = masterID == 0 ? userData : masterData;

	if (pBuyGameDeskInfo->costResType <= 0)
	{
		pBuyGameDeskInfo->costResType = RESOURCE_TYPE_JEWEL;
	}

	////////////////////////////////同步数据(一个人不能同时购买桌子)//////////////////////////////////////////
	memset(redisLockKey, 0, sizeof(redisLockKey));
	sprintf(redisLockKey, "%s|%d", TBL_USER_BUY_DESK, buyUserID);
	CRedisLock redisLock(m_pRedisPHP->GetRedisContext(), redisLockKey, 5);

	if (pRoomBaseInfo->type == ROOM_TYPE_PRIVATE) //积分房
	{
		if (payType != PAY_TYPE_AA)
		{
			payType = PAY_TYPE_NORMAL;
		}

		// 检查房卡
		if (payType == PAY_TYPE_NORMAL)
		{
			needCostNums = pBuyGameDeskInfo->costNums;
		}
		else
		{
			needCostNums = pBuyGameDeskInfo->AAcostNums;
		}

		if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY)
		{
			if (buyUserData.money < needCostNums) //消耗俱乐部群主的
			{
				UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_MONEY : ERROR_NOT_ENOUGH_MONEY;
				SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode);
				return true;
			}
		}
		else
		{
			if (buyUserData.jewels < needCostNums)
			{
				UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_JEWELS : ERROR_NOT_ENOUGH_JEWELS;
				SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode);
				return true;
			}
		}

		//房主支付先扣
		if (payType == PAY_TYPE_NORMAL)
		{
			bAlreadyCost = true;
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FRIEND)//金币房
	{
		if (minPoint < kickPoint || minPoint < basePoint)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM);
			return true;
		}

		if (payType == PAY_TYPE_NORMAL)
		{
			needCostNums = pBuyGameDeskInfo->costNums;
			bAlreadyCost = true;

			if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY)
			{
				if (buyUserData.money < needCostNums) //消耗俱乐部群主的
				{
					UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_MONEY : ERROR_NOT_ENOUGH_MONEY;
					SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode);
					return true;
				}
			}
			else
			{
				if (buyUserData.jewels < needCostNums)
				{
					UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_JEWELS : ERROR_NOT_ENOUGH_JEWELS;
					SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode);
					return true;
				}
			}
		}

		// 金币房检查玩家的金币数量，够不够进入
		if (pMessage->masterNotPlay == 0 && pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY
			&& userData.money < (minPoint + needCostNums))
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_ROOM_NO_MONEYREQUIRE);
			return true;
		}

		// 金币房检查玩家的金币数量，够不够进入
		if (pMessage->masterNotPlay == 0 && pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL
			&& userData.money < minPoint)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_ROOM_NO_MONEYREQUIRE);
			return true;
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FG_VIP)//俱乐部VIP房
	{
		if (minPoint < kickPoint || minPoint < basePoint)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM);
			return true;
		}

		if (friendsGroupID <= 0) //vip房只有俱乐部可以创建
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM);
			return true;
		}

		if (payType == PAY_TYPE_NORMAL)
		{
			needCostNums = pBuyGameDeskInfo->costNums;
			bAlreadyCost = true;

			if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY)
			{
				if (buyUserData.money < needCostNums) //消耗俱乐部群主的
				{
					UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_MONEY : ERROR_NOT_ENOUGH_MONEY;
					SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode);
					return true;
				}
			}
			else
			{
				if (buyUserData.jewels < needCostNums)
				{
					UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_JEWELS : ERROR_NOT_ENOUGH_JEWELS;
					SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode);
					return true;
				}
			}
		}
	}
	else
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM);
		return false;
	}

	int maxUserCount = pGameBaseInfo->deskPeople;
	// 特殊游戏的处理
	if (pGameBaseInfo->multiPeopleGame)
	{
		maxUserCount = iJsonRS;
		if (maxUserCount <= 0)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_SET_PEOPLE);
			return true;
		}
	}

	LogonResponseBuyDesk msg;

	// 创建房间
	std::string deskPasswd = m_pRedis->CreatePrivateDeskRecord(buyUserID, pRoomBaseInfo->roomID, pMessage->masterNotPlay, pRoomBaseInfo->deskCount,
		buyGameCount, pMessage->gameRules, maxUserCount, payType, pGameBaseInfo->watcherCount, friendsGroupID, friendsGroupDeskNumber);
	if (deskPasswd == "")
	{
		ERROR_LOG("CreatePrivateDeskRecord failed userID: %d, deskCount: %d", userID, pRoomBaseInfo->deskCount);
		SendData(userID, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NOVAILD_DESK);
		return true;
	}

	//房主支付，先扣
	if (bAlreadyCost && needCostNums > 0)
	{
		if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL)
		{
			long long newJewels = buyUserData.jewels - needCostNums;
			m_pRedis->SetUserJewelsEx(buyUserID, -needCostNums, true, RESOURCE_CHANGE_REASON_CREATE_ROOM, iBuyRoomID, 0, buyUserData.isVirtual, friendsGroupID);

			// 解锁
			redisLock.Unlock();

			// 通知资源变化
			NotifyResourceChange(buyUserID, RESOURCE_TYPE_JEWEL, newJewels, RESOURCE_CHANGE_REASON_CREATE_ROOM, -needCostNums);
		}
		else
		{
			long long newGolds = buyUserData.money - needCostNums;
			m_pRedis->SetUserMoneyEx(buyUserID, -needCostNums, true, RESOURCE_CHANGE_REASON_CREATE_ROOM, iBuyRoomID, 0, buyUserData.isVirtual, friendsGroupID);

			// 解锁
			redisLock.Unlock();

			//算房卡场系统赢钱
			m_pRedis->SetRoomGameWinMoney(iBuyRoomID, needCostNums, true);

			// 通知资源变化
			NotifyResourceChange(buyUserID, RESOURCE_TYPE_MONEY, newGolds, RESOURCE_CHANGE_REASON_CREATE_ROOM, -needCostNums);
		}
	}

	// 解锁
	redisLock.Unlock();

	//添加俱乐部牌桌消息
	if (friendsGroupID != 0)
	{
		OneFriendsGroupDeskInfo deskInfoMsg;
		bool bHaveRedSpot = false;
		if (m_pRedis->CreateFriendsGroupDeskInfo(friendsGroupID, friendsGroupDeskNumber, deskPasswd, pRoomBaseInfo->gameID, pRoomBaseInfo->type, deskInfoMsg, bHaveRedSpot))
		{
			// 解锁
			redisLockFG.Unlock();

			// 发给中心服务器，让中心服务器转发
			//推送给所有在线成员
			UINT iMessageID = friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT
				? MSG_NTF_LOGON_FRIENDSGROUP_NEW_VIPROOM_MSG : MSG_NTF_LOGON_FRIENDSGROUP_NEW_DESK_MSG;
			SendMessageToCenterServer(CENTER_MESSAGE_LOGON_RELAY_FG_MSG, &deskInfoMsg, sizeof(deskInfoMsg), friendsGroupID, MSG_MAIN_FRIENDSGROUP_NOTIFY, iMessageID, 0);

			if (bHaveRedSpot)
			{
				PlatformFriendsGroupPushRedSpot msgRedSpot;
				msgRedSpot.friendsGroupID = friendsGroupID;
				if (friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT)
				{
					msgRedSpot.VIPRoomMsgRedSpotCount = 1;
				}
				else
				{
					msgRedSpot.deskMsgRedSpotCount = 1;
				}

				SendMessageToCenterServer(CENTER_MESSAGE_LOGON_REQ_FG_REDSPOT_MSG, &msgRedSpot, sizeof(msgRedSpot), friendsGroupID, 0, 0, 0);
			}
		}
		else
		{
			// 解锁
			redisLockFG.Unlock();

			ERROR_LOG("创建牌桌失败,friendsGroupID=%d,friendsGroupDeskNumber=%d", friendsGroupID, friendsGroupDeskNumber);
		}
	}

	// 购买桌子次数
	if (friendsGroupID == 0)
	{
		m_pRedis->SetUserBuyingDeskCount(userID, 1, true);
	}

	msg.userID = userID;
	memcpy(msg.passwd, deskPasswd.c_str(), deskPasswd.size());
	msg.passwdLen = deskPasswd.size();
	msg.roomID = pRoomBaseInfo->roomID;

	SendData(userID, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameLogonManage::OnHandleUserEnterDesk(int userID, void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, LogonRequestEnterDesk, pData, size);

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("GetUserData(%d) failed", userID);
		return false;
	}

	// 比赛场判断
	if (userData.matchType == MATCH_TYPE_TIME && userData.matchStatus == USER_MATCH_STATUS_AFTER_BEGIN
		|| userData.matchType == MATCH_TYPE_PEOPLE && userData.matchStatus != USER_MATCH_STATUS_NORMAL)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_MATCH_PLAYING);
		return true;
	}

	// 已经在一个房间里面则无法加入房间
	if (userData.roomID > 0)
	{
		LogonResponseEnterDesk msg;
		msg.userID = userID;
		msg.roomID = userData.roomID;
		msg.deskIdx = userData.deskIdx;
		SendData(userID, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_CANNOT_ENTERDESK_INROOM);

		//告诉正确的位置
		RoomBaseInfo* pUserRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(userData.roomID);
		if (pUserRoomBaseInfo)
		{
			if (pUserRoomBaseInfo->type != ROOM_TYPE_GOLD && pUserRoomBaseInfo->type != ROOM_TYPE_MATCH)
			{
				if (userData.deskIdx == INVALID_DESKIDX)
				{
					m_pRedis->SetUserRoomID(userID, 0);
				}
				else
				{
					ERROR_LOG("用户已经在房间userID = %d,roomID:%d,deskIdx=%d", userID, userData.roomID, userData.deskIdx);
				}
			}
		}
		else
		{
			m_pRedis->SetUserRoomID(userID, 0);
			m_pRedis->SetUserDeskIdx(userID, INVALID_DESKIDX);
		}
		return true;
	}

	// 根据唯一的密码查询私有桌子
	int deskMixID = m_pRedis->GetDeskMixIDByPasswd(pMessage->passwd);
	if (deskMixID <= 0)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NO_THIS_ROOM);
		return true;
	}

	// 根据deskMixID取出桌子的数据
	PrivateDeskInfo privateDeskInfo;
	if (!m_pRedis->GetPrivateDeskRecordInfo(deskMixID, privateDeskInfo))
	{
		ERROR_LOG("GetPrivateDeskRecordInfo failed deskMixID %d", deskMixID);
		return false;
	}

	int roomID = deskMixID / MAX_ROOM_HAVE_DESK_COUNT;
	int deskIdx = deskMixID % MAX_ROOM_HAVE_DESK_COUNT;

	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(roomID);
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("GetRoomBaseInfo failed roomID: %d", roomID);
		return false;
	}

	if (!IsRoomIDServerExists(roomID))
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_GAME_NO_START);
		return true;
	}

	//非俱乐部不能加入俱乐部房间
	int	friendsGroupID = privateDeskInfo.friendsGroupID;
	if (friendsGroupID != 0 && !m_pRedisPHP->IsCanJoinFriendsGroupRoom(userID, friendsGroupID))
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_FRIENDSGROUP_ERR_JOIN_ROOM);
		return true;
	}

	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(pRoomBaseInfo->gameID);
	if (!pGameBaseInfo)
	{
		ERROR_LOG("GetGameBaseInfo failed gameID: %d", pRoomBaseInfo->gameID);
		return false;
	}

	//判断游戏是否开始 ERROR_STOP_JOIN
	int iStopJoin = 0;
	int minPoint = 0; //入场限制
	int cqFriendsGroupID = 0;
	if (privateDeskInfo.gameRules[0] != '\0')
	{
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(privateDeskInfo.gameRules, value))
		{
			iStopJoin = value["Stopjoin"].asInt();
			minPoint = value["mPoint"].asInt();
			cqFriendsGroupID = value["FGID"].asInt();
		}
	}
	if (iStopJoin && privateDeskInfo.curGameCount > 0)
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_STOP_JOIN);
		return true;
	}

	//特殊游戏判断，猜拳
	if (cqFriendsGroupID > 0 && pRoomBaseInfo->gameID == 37550102)
	{
		long long carryFireCoin = 0;
		if (!m_pRedisPHP->GetUserFriendsGroupMoney(cqFriendsGroupID, userID, carryFireCoin))
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_FRIENDSGROUP_ERR_JOIN_ROOM);
			return true;
		}
	}

	if (pRoomBaseInfo->type == ROOM_TYPE_PRIVATE)
	{
		// todo
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FRIEND)
	{
		// 私人房检查玩家的金币数量
		if (minPoint > 0 && userData.money < minPoint)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_ROOM_NO_MONEYREQUIRE);
			return true;
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FG_VIP)
	{
		if (friendsGroupID <= 0)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NO_THIS_ROOM);
			return true;
		}

		//判断火币够不够
		long long carryFireCoin = 0;
		if (!m_pRedisPHP->GetUserFriendsGroupMoney(friendsGroupID, userID, carryFireCoin))
		{
			ERROR_LOG("获取玩家俱乐部金币失败:userID = %d , friendsGroupID=%d", userID, friendsGroupID);
		}

		if ((int)carryFireCoin < minPoint)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_FRIENDSGROUP_FIRECOIN_LIMIT);
			return true;
		}
	}
	else
	{
		SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NO_THIS_ROOM);
		return false;
	}

	//判断资源是否足够假如房间
	if (privateDeskInfo.payType == PAY_TYPE_AA && (pRoomBaseInfo->type == ROOM_TYPE_PRIVATE || pRoomBaseInfo->type == ROOM_TYPE_FG_VIP))
	{
		BuyGameDeskInfo buyGameDeskInfo;
		BuyGameDeskInfo* pBuyGameDeskInfo = NULL;
		BuyGameDeskInfoKey buyDekKey(pRoomBaseInfo->gameID, privateDeskInfo.buyGameCount, pRoomBaseInfo->type);
		if (m_pRedis->GetBuyGameDeskInfo(buyDekKey, buyGameDeskInfo))
		{
			pBuyGameDeskInfo = &buyGameDeskInfo;
		}
		else
		{
			pBuyGameDeskInfo = ConfigManage()->GetBuyGameDeskInfo(buyDekKey);
		}
		if (!pBuyGameDeskInfo)
		{
			ERROR_LOG("积分房,GetBuyGameDeskInfo failed gameID(%d) gameCount(%d)", pRoomBaseInfo->gameID, privateDeskInfo.buyGameCount);
			return false;
		}

		if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL)
		{
			if (userData.jewels < pBuyGameDeskInfo->AAcostNums)
			{
				SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NOT_ENOUGH_JEWELS);
				return true;
			}
		}
		else
		{
			if (userData.money < pBuyGameDeskInfo->AAcostNums)
			{
				SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NOT_ENOUGH_MONEY);
				return true;
			}
		}
	}

	LogonResponseEnterDesk msg;

	if (pGameBaseInfo->canWatch)
	{
		// 旁观者人数是否已满
		if (privateDeskInfo.currWatchUserCount >= privateDeskInfo.maxWatchUserCount)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_DESK_FULL);
			return true;
		}
	}
	else
	{
		if (privateDeskInfo.currDeskUserCount >= privateDeskInfo.maxDeskUserCount)
		{
			SendData(userID, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_DESK_FULL);
			return true;
		}
	}

	// 记录玩家进入桌子
	m_pRedis->SetUserRoomID(userID, roomID);
	m_pRedis->SetUserDeskIdx(userID, deskIdx);

	msg.userID = userID;
	msg.roomID = roomID;
	msg.deskIdx = deskIdx;

	SendData(userID, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, 0);

	return true;
}

bool CGameLogonManage::OnHandleOtherMessage(int assistID, void* pData, int size, unsigned long accessIP, unsigned int socketIdx, long handleID)
{
	LogonServerSocket socketInfo = GetIdentityIDBySocketIdx(socketIdx);
	if (socketInfo.type != LOGON_SERVER_SOCKET_TYPE_USER)
	{
		ERROR_LOG("非玩家socket，禁止发送此消息 socketIdx:%d", socketIdx);
		return false;
	}

	int userID = socketInfo.identityID;
	if (userID <= 0)
	{
		ERROR_LOG("没有绑定玩家id的异常 socketIdx:%d", socketIdx);
		return false;
	}

	switch (assistID)
	{
	case MSG_ASS_LOGON_OTHER_USERINFO_FLUSH:
	{
		return OnHandleUserInfoFlush(userID, pData, size);
	}
	case MSG_ASS_LOGON_OTHER_ROBOT_TAKEMONEY:
	{
		return OnHandleRobotTakeMoney(userID, pData, size);
	}
	case MSG_ASS_LOGON_OTHER_REQ_USERINFO:
	{
		return OnHandleReqUserInfo(userID, pData, size);
	}
	case MSG_ASS_LOGON_OTHER_JOIN_MATCH_SCENE:
	{
		return OnHandleJoinMatchScene(socketIdx);
	}
	case MSG_ASS_LOGON_OTHER_EXIT_MATCH_SCENE:
	{
		return OnHandleExitMatchScene(socketIdx);
	}
	default:
		break;
	}

	return false;
}

bool CGameLogonManage::OnHandleUserInfoFlush(int userID, void* pData, int size)
{
	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("GetUserData failed userID=%d", userID);
		return true;
	}

	LogonUserInfoFlush msg;

	msg.iJewels = userData.jewels;
	msg.money = userData.money;

	SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOGON_OTHER, MSG_ASS_LOGON_OTHER_USERINFO_FLUSH, 0);

	return true;
}

bool CGameLogonManage::OnHandleRobotTakeMoney(int userID, void* pData, int size)
{
	if (size != sizeof(_LogonResponseRobotTakeMoney))
	{
		ERROR_LOG("OnHandleRobotTakeMoney size match failed");
		return false;
	}

	_LogonResponseRobotTakeMoney* pMessage = (_LogonResponseRobotTakeMoney*)pData;
	if (!pMessage)
	{
		return false;
	}

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("OnHandleRobotTakeMoney::GetUserData failed userID:%d", userID);
		return true;
	}

	if (!userData.isVirtual || pMessage->iMoney <= 0)
	{
		return true;
	}

	//机器人从奖池中取金币
	__int64 _i64PoolMoney = 0;
	_i64PoolMoney = userData.money - (__int64)pMessage->iMoney;
	m_pRedis->SetRoomPoolMoney(pMessage->iRoomID, _i64PoolMoney, true);
	m_pRedis->SetUserMoney(userID, pMessage->iMoney);

	_LogonRobotTakeMoneyRes _res;

	_res.iMoney = (int)userData.money;

	SendData(userID, &_res, sizeof(_res), MSG_MAIN_LOGON_OTHER, MSG_ASS_LOGON_OTHER_ROBOT_TAKEMONEY, 0);

	return true;
}

bool CGameLogonManage::OnHandleReqUserInfo(int userID, void* pData, int size)
{
	if (size != sizeof(LogonRequestUserInfo))
	{
		return false;
	}

	LogonRequestUserInfo* pMessage = (LogonRequestUserInfo*)pData;
	if (!pMessage)
	{
		return false;
	}

	int targetID = pMessage->userID;

	UserData userData;
	if (!m_pRedis->GetUserData(targetID, userData))
	{
		ERROR_LOG("请求个人信息失败 targetID=%d", targetID);
		return true;
	}

	LogonResponseUserInfo msg;

	msg.gameCount = userData.totalGameCount;
	msg.winCount = userData.totalGameWinCount;
	if (userData.isVirtual)
	{
		msg.winCount = int(msg.gameCount * ((targetID % 40 + 20) * 1.0 / 100));
	}
	msg.sex = userData.sex;
	memcpy(msg.name, userData.name, sizeof(msg.name));
	memcpy(msg.headURL, userData.headURL, sizeof(msg.headURL));
	msg.onlineStatus = m_pRedis == NULL ? IsUserOnline(userID) : m_pRedis->IsUserOnline(userID);
	memcpy(msg.longitude, userData.Lng, sizeof(msg.longitude));
	memcpy(msg.latitude, userData.Lat, sizeof(msg.latitude));
	memcpy(msg.address, userData.address, sizeof(msg.address));
	msg.jewels = userData.jewels;
	msg.money = userData.money;
	memcpy(msg.ip, userData.logonIP, sizeof(msg.ip));
	memcpy(msg.motto, userData.motto, sizeof(msg.motto));

	SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOGON_OTHER, MSG_ASS_LOGON_OTHER_REQ_USERINFO, 0);

	return true;
}

// 请求进入比赛场页面
bool CGameLogonManage::OnHandleJoinMatchScene(UINT uIndex)
{
	m_scoketMatch.insert(uIndex);

	return true;
}

// 请求退出比赛场页面
bool CGameLogonManage::OnHandleExitMatchScene(UINT uIndex)
{
	m_scoketMatch.erase(uIndex);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 游戏服相关消息

bool CGameLogonManage::OnHandleGServerVerifyMessage(void* pData, int size, unsigned int socketIdx)
{
	SAFECHECK_MESSAGE(pMessage, PlatformLogonServerVerify, pData, size);

	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(pMessage->roomID);
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("没有配置的roomID=%d", pMessage->roomID);
		return false;
	}

	// 容错
	pMessage->passwd[sizeof(pMessage->passwd) - 1] = '\0';

	// 认证密码
	if (strcmp(pMessage->passwd, ConfigManage()->m_loaderServerConfig.logonserverPasswd))
	{
		ERROR_LOG("密码错误，pMessage->passwd=%s,pwd=%s", pMessage->passwd, ConfigManage()->m_loaderServerConfig.logonserverPasswd);
		return false;
	}

	// 添加到内存
	LogonGServerInfo* pGServer = m_pGServerManage->GetGServer(pMessage->roomID);
	if (pGServer)
	{
		ERROR_LOG("gserver（%d）已经登录过了", pMessage->roomID);

		// 返回true不断socket
		return true;
	}
	else
	{
		pGServer = new LogonGServerInfo;
		pGServer->roomID = pMessage->roomID;
		pGServer->socketIdx = socketIdx;
		m_pGServerManage->AddGServer(pMessage->roomID, pGServer);
	}

	// socketIdx和gserver关联
	m_socketInfoMap[socketIdx] = LogonServerSocket(LOGON_SERVER_SOCKET_TYPE_GAME_SERVER, pMessage->roomID);

	// 记录到redis
	m_pRedis->SetLogonServerCurrPeopleCount(ConfigManage()->GetLogonServerConfig().logonID,
		m_pUserManage->GetUserCount(), m_TCPSocket.GetCurSocketSize());

	INFO_LOG("【roomID=%d】游戏服连接本服", pMessage->roomID);

	return true;
}

bool CGameLogonManage::OnHandleGServerToGameMessage(int userID, NetMessageHead* pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, UINT dwHandleID)
{
	LogonUserInfo* pUser = m_pUserManage->GetUser(userID);
	if (!pUser)
	{
		ERROR_LOG("玩家存在 userID=%d", userID);
		return false;
	}

	int roomID = pNetHead->uIdentification;

	LogonGServerInfo* pGServer = m_pGServerManage->GetGServer(roomID);
	if (!pGServer)
	{
		m_TCPSocket.SendData(pUser->socketIdx, NULL, 0, pNetHead->uMainID, pNetHead->uAssistantID, ERROR_GAME_NO_START, 0, roomID);

		// 返回true不断socket
		return true;
	}

	// 此处可以根据业务逻辑，拦截游戏消息
	// todo:



	// 转发到游戏服
	m_TCPSocket.SendData(pGServer->socketIdx, pData, uSize, pNetHead->uMainID, pNetHead->uAssistantID, pNetHead->uHandleCode, 0, userID);

	return true;
}


bool CGameLogonManage::OnHandleGServerToUserMessage(int roomID, NetMessageHead* pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, UINT dwHandleID)
{
	// 此处可以根据业务逻辑，拦截游戏消息
	// todo:


	// 转发消息
	SendData(pNetHead->uIdentification, pData, uSize, pNetHead->uMainID, pNetHead->uAssistantID, pNetHead->uHandleCode, roomID);

	return true;
}


//////////////////////////////////////////////////////////////////////////
LogonServerSocket CGameLogonManage::GetIdentityIDBySocketIdx(int socketIdx)
{
	auto iter = m_socketInfoMap.find(socketIdx);
	if (iter != m_socketInfoMap.end())
	{
		return iter->second;
	}

	return LogonServerSocket(LOGON_SERVER_SOCKET_TYPE_NO, 0);
}

// 删除socketIdx索引
void CGameLogonManage::DelSocketIdx(int socketIdx)
{
	auto iter = m_socketInfoMap.find(socketIdx);
	if (iter != m_socketInfoMap.end())
	{
		m_socketInfoMap.erase(iter);
	}
}

bool CGameLogonManage::SendData(int userID, void* pData, int size, unsigned int mainID, unsigned int assistID, unsigned int handleCode, unsigned int uIdentification/* = 0*/)
{
	LogonUserInfo* pUser = m_pUserManage->GetUser(userID);
	if (!pUser)
	{
		if (uIdentification > 0) // 转发游戏消息
		{
			//ERROR_LOG("转发游戏消息失败：userID=%d,mainID=%d,assistID=%d,uIdentification=%d", userID, mainID, assistID, uIdentification);

			// 可以将消息发送给中心服，进一步转发保证数据成功。
			// todo:

		}
		return false;
	}

	int ret = m_TCPSocket.SendData(pUser->socketIdx, pData, size, mainID, assistID, handleCode, 0, uIdentification);
	if (ret <= 0)
	{
		return false;
	}

	return true;
}

bool CGameLogonManage::SendDataBatch(void* pData, UINT uSize, UINT uMainID, UINT bAssistantID, UINT uHandleCode)
{
	for (auto iter = m_socketInfoMap.begin(); iter != m_socketInfoMap.end(); iter++)
	{
		if (iter->second.type == LOGON_SERVER_SOCKET_TYPE_USER)
		{
			m_TCPSocket.SendData(iter->first, pData, uSize, uMainID, bAssistantID, 0, 0, 0);
		}
	}

	return true;
}

// 向中心服务器发送消息
bool CGameLogonManage::SendMessageToCenterServer(UINT msgID, void* pData, UINT size, int userID/* = 0*/, UINT mainID/* = 0*/, UINT assistID/* = 0*/, UINT handleCode/* = 0*/)
{
	if (m_pTcpConnect)
	{
		NetMessageHead netHead;

		netHead.uMainID = mainID;
		netHead.uAssistantID = assistID;
		netHead.uHandleCode = handleCode;
		bool ret = m_pTcpConnect->Send(msgID, pData, size, userID, &netHead);
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

void CGameLogonManage::InitRounteCheckEvent()
{
	// 设置心跳定时器
	SetTimer(LOGON_TIMER_CHECK_HEARTBEAT, CHECK_HEAETBEAT_SECS * 1000);

	// 设置检查redis连接定时器
	SetTimer(LOGON_TIMER_CHECK_REDIS_CONNECTION, CHECK_REDIS_CONNECTION_SECS * 1000);

	// 设置存储redis数据到DB定时器
	SetTimer(LOGON_TIMER_ROUTINE_SAVE_REDIS, CHECK_REDIS_SAVE_DB * 1000);

	// 设置定期清理未登录的连接
	//SetTimer(LOGON_TIMER_ROUTINE_CHECK_UNBINDID_SOCKET, ROUTINE_CHECK_UNBINDID_SOCKET * 1000);

	// 设置通用定时器
	SetTimer(LOGON_TIMER_NORMAL, NORMAL_TIMER_SECS * 1000);
}

void CGameLogonManage::CheckRedisConnection()
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

void CGameLogonManage::CheckHeartBeat(time_t llLastSendHeartBeatTime, int iHeartBeatTime)
{
	int avtiveCount = 0;
	int negativeCount = 0;
	time_t currTime = time(NULL);

	const std::unordered_map<int, TCPSocketInfo>& socketInfoMap = m_TCPSocket.GetSocketMap();

	for (auto iter = socketInfoMap.begin(); iter != socketInfoMap.end(); iter++)
	{
		/*LogonServerSocket socketInfo = GetIdentityIDBySocketIdx(iter->first);
		if (socketInfo.type == LOGON_SERVER_SOCKET_TYPE_GAME_SERVER
			|| socketInfo.type == LOGON_SERVER_SOCKET_TYPE_NO)
		{
			continue;
		}*/

		if (iter->second.isConnect)
		{
			m_TCPSocket.SendData(iter->first, NULL, 0, MSG_MAIN_TEST, MSG_ASS_TEST, 0, 0);

			//if ((currTime - iter->second.lastRecvMsgTime) >= (iHeartBeatTime * (KEEP_ACTIVE_HEARTBEAT_COUNT - 1) + currTime - llLastSendHeartBeatTime + 1))
			//{
			//	m_TCPSocket.CloseSocket(iter->first);
			//	negativeCount++;
			//	CON_INFO_LOG("心跳踢人 currTime = %lld,lastRecvMsgTime=%lld,acceptMsgTime=%lld,ip=%s:%d",
			//		currTime, iter->second.lastRecvMsgTime, iter->second.acceptMsgTime, iter->second.ip, iter->second.port);
			//}
			//else
			//{
			//	// 发送心跳
			//	m_TCPSocket.SendData(iter->first, NULL, 0, MSG_MAIN_TEST, MSG_ASS_TEST, 0, 0);
			//	avtiveCount++;
			//}
		}
	}

	/*if (negativeCount > 0)
	{
		INFO_LOG("avtiveCount=%d negativeCount=%d", avtiveCount, negativeCount);
	}*/
}

void CGameLogonManage::RountineSaveRedisDataToDB(bool updateAll)
{
	if (m_pRedis)
	{
		m_pRedis->RountineSaveRedisDataToDB(updateAll);
	}
}

void CGameLogonManage::RoutineCheckUnbindIDSocket()
{
	time_t currTime = time(NULL);

	// 检查未绑定userID的异常socket
	const std::unordered_map<int, TCPSocketInfo>& socketInfoMap = m_TCPSocket.GetSocketMap();

	for (auto iter = socketInfoMap.begin(); iter != socketInfoMap.end(); iter++)
	{
		bool bIsConnect = iter->second.isConnect;
		time_t llAcceptMsgTime = iter->second.acceptMsgTime;

		// 没有绑定userID的socket
		if (bIsConnect && currTime - llAcceptMsgTime > BINDID_SOCKET_USERID_TIME&& GetIdentityIDBySocketIdx(iter->first).type <= 0)
		{
			INFO_LOG("清理没有绑定的连接【connect-time=%lld,ip=%s】", currTime - llAcceptMsgTime, iter->second.ip);
			m_TCPSocket.CloseSocket(iter->first);
		}
	}
}

void CGameLogonManage::OnNormalTimer()
{
	if (!m_pRedis)
	{
		ERROR_LOG("### redis 指针为空 ###");
		return;
	}

	// 检查是否跨天
	time_t currTime = time(NULL);
	int currHour = CUtil::GetHourTimeStamp(currTime);
	int lastHour = CUtil::GetHourTimeStamp(m_lastNormalTimerTime);

	//凌晨12点执行奖池统计
	if (currHour == 0 && currHour != lastHour)
	{
		OnServerCrossDay12Hour();
	}

	//凌晨5点执行清理数据
	if (currHour == 5 && currHour > lastHour)
	{
		OnServerCrossDay();

		int currWeekday = CUtil::GetWeekdayFromTimeStamp(currTime);

		if (currWeekday == 1)
		{
			OnServerCrossWeek();
		}
	}

	m_lastNormalTimerTime = currTime;
}

/***********************************************************************************************************/
bool CGameLogonManage::IsUserOnline(int userID)
{
	LogonUserInfo* pUser = m_pUserManage->GetUser(userID);
	if (pUser)
	{
		return true;
	}

	return false;
}

void CGameLogonManage::NotifyResourceChange(int userID, int resourceType, long long value, int reason, long long changeValue)
{
	if (IsUserOnline(userID))
	{
		LogonNotifyResourceChange msg;

		msg.resourceType = resourceType;
		msg.value = value;
		msg.reason = reason;
		msg.changeValue = changeValue;

		SendData(userID, &msg, sizeof(LogonNotifyResourceChange), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_RESOURCE_CHANGE, 0);
	}
	else if (m_pRedis && m_pRedis->IsUserOnline(userID))
	{
		PlatformResourceChange msgToLoader;

		msgToLoader.resourceType = resourceType;
		msgToLoader.value = value;
		msgToLoader.reason = reason;
		msgToLoader.changeValue = changeValue;

		SendMessageToCenterServer(CENTER_MESSAGE_LOGON_RESOURCE_CHANGE, &msgToLoader, sizeof(msgToLoader), userID);
	}
}

void CGameLogonManage::OnServerCrossDay()
{
	AUTOCOST("OnServerCrossDay");
	INFO_LOG("OnServerCrossDay");

	if (!m_pRedis || !m_pRedisPHP)
	{
		ERROR_LOG("m_pRedis = null || m_pRedisPHP = NULL");
		return;
	}

	m_pRedis->ClearGradeInfo();

	//重置每日奖池，添加到总奖池
	m_pRedis->ClearOneDayWinMoney();

	//清理过期数据
	if (m_pRedis->IsMainDistributedSystem())
	{
		m_pRedisPHP->ClearAllEmailInfo();

		m_pRedisPHP->ClearAllFGNotifyInfo();

		ClearDataBase();

		CleanAllUserMoneyJewelsRank();
	}

	//生成新表，记录账单数据
	if (m_pRedis->IsMainDistributedSystem())
	{
		AutoCreateNewTable(false);
	}

	int currTime = (int)time(NULL);
	const auto& onlineUserMap = m_pUserManage->GetLogonUserInfoMap();
	for (auto iter = onlineUserMap.begin(); iter != onlineUserMap.end(); ++iter)
	{
		int userID = iter->first;
		OnUserCrossDay(userID, currTime);
	}
}

// 凌晨12点
void CGameLogonManage::OnServerCrossDay12Hour()
{
	AUTOCOST("OnServerCrossDay12Hour");

	if (m_pRedis)
	{
		m_pRedis->CountOneDayPoolInfo();
	}
}

void CGameLogonManage::OnServerCrossWeek()
{
	AUTOCOST("OnServerCrossWeek");
	INFO_LOG("OnServerCrossWeek");

	//分布式处理
	if (m_pRedis->IsMainDistributedSystem())
	{
		//清理胜局数
		CleanAllUserWinCount();
	}

	//清理过期文件，考虑到不同的登陆服，会部署到不同的主机，每个主机都需要执行
	DeleteExpireFile();
}

void CGameLogonManage::CleanAllUserWinCount()
{
	if (m_pRedis)
	{
		m_pRedis->ClearAllUserWinCount();
	}
}

// 清理金币钻石排行榜
void CGameLogonManage::CleanAllUserMoneyJewelsRank()
{
	m_pRedis->ClearAllUserRanking(TBL_RANKING_MONEY);

	m_pRedis->ClearAllUserRanking(TBL_RANKING_JEWELS);
}

void CGameLogonManage::OnUserLogon(const UserData& userData)
{
	if (userData.isVirtual)
	{
		return;
	}

	//发送相关通知 
	NotifyUserInfo(userData);

	// 统计登陆次数
	if (m_pRedisPHP)
	{
		m_pRedisPHP->AddUserResNums(userData.userID, "loginLobbyCount", 1);
	}
}

void CGameLogonManage::OnUserLogout(int userID)
{

}

void CGameLogonManage::OnUserRegister(const UserData& userData)
{
	// 往数据库中立马插入一个玩家
	char key[48] = "";
	sprintf(key, "%s|%d", TBL_USER, userData.userID);
	if (!m_pRedis->SaveRedisDataToDB(key, TBL_USER, userData.userID, REDIS_EXTEND_MODE_INSERT))
	{
		ERROR_LOG("注册插入用户失败：userID=%d", userData.userID);
	}

	// 往任务表里插入一个玩家
	char skey[48] = "";
	sprintf(skey, "%s|%d", TBL_USER_BAG, userData.userID);
	if (!m_pRedis->SaveRedisDataToDB(skey, TBL_USER_BAG, userData.userID, REDIS_EXTEND_MODE_INSERT))
	{
		ERROR_LOG("注册背包插入用户失败：userID=%d", userData.userID);
	}

	if (userData.money > 0)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_USER_MONEY_CHANGE, tableName, sizeof(tableName));

		// 金币变化
		BillManage()->WriteBill(&m_SQLDataManage, "INSERT INTO %s (userID,time,money,changeMoney,reason,roomID,friendsGroupID,rateMoney) VALUES(%d,%d,%lld,%lld,%d,%d,%d,%d)",
			tableName, userData.userID, (int)time(NULL), userData.money, userData.money, RESOURCE_CHANGE_REASON_REGISTER, 0, 0, 0);

		// 金币排行榜
		m_pRedis->AddKeyToZSet(TBL_RANKING_MONEY, userData.money, userData.userID);
	}

	if (userData.jewels > 0)
	{
		char tableName[128] = "";
		ConfigManage()->GetTableNameByDate(TBL_STATI_USER_JEWELS_CHANGE, tableName, sizeof(tableName));

		// 钻石变化
		BillManage()->WriteBill(&m_SQLDataManage, "INSERT INTO %s (userID,time,jewels,changeJewels,reason,roomID,friendsGroupID) VALUES(%d,%d,%d,%d,%d,%d,%d)",
			tableName, userData.userID, (int)time(NULL), userData.jewels, userData.jewels, RESOURCE_CHANGE_REASON_REGISTER, 0, 0);

		// 钻石排行榜
		m_pRedis->AddKeyToZSet(TBL_RANKING_JEWELS, userData.jewels, userData.userID);
	}

	// post给php，发送邮件通知
	SendHTTPUserRegisterMessage(userData.userID);
}

void CGameLogonManage::OnUserCrossDay(int userID, int time)
{
	m_pRedis->SetUserCrossDayTime(userID, time);

	//清除玩家战绩相关
	if (m_pRedis)
	{
		m_pRedis->ClearUserGradeInfo(userID);
	}

	//清理玩家无效邮件和俱乐部通知
	if (m_pRedisPHP)
	{
		m_pRedisPHP->ClearUserEmailSet(userID);

		m_pRedisPHP->ClearUserFGNotifySet(userID);
	}
}

// 清理数据库过期数据
void CGameLogonManage::ClearDataBase()
{
	// 获取系统配置，看看是否需要分表
	OtherConfig otherConfig = ConfigManage()->GetOtherConfig();

	// 分表不清理数据
	if (otherConfig.byIsDistributedTable)
	{
		return;
	}

	time_t currTime = time(NULL);

	////"statistics_logonandlogout"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_STATI_LOGON_LOGOUT, currTime - iDataExpireTime);

	////"statistics_moneychange"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_STATI_USER_MONEY_CHANGE, currTime - iDataExpireTime);

	////"statistics_jewelschange"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_STATI_USER_JEWELS_CHANGE, currTime - iDataExpireTime);

	////"statistics_gamerecordinfo"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where finishedTime<%lld",
	//	TBL_STATI_GAME_RECORD_INFO, currTime - iDataExpireTime);

	////"statistics_rewardspool"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_STATI_REWARDS_POOL, currTime - iDataExpireTime);

	////"statistics_firecoinchange"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_STATI_USER_FIRECOIN_CHANGE, currTime - iDataExpireTime);

	////"friendsGroupAccounts"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_FG_RECORD_ACCOUNTS, currTime - iDataExpireTime);

	////"friendsGroupDeskListCost"
	//BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
	//	TBL_FG_COST_ACCOUNTS, currTime - iDataExpireTime);

	//"statistics_roomwincount"，保存7天的数据
	BillManage()->WriteBill(&m_SQLDataManage, "DELETE from %s where time<%lld",
		TBL_STATI_ROOM_WIN_COUNT, currTime - 7 * 24 * 60 * 60);
}

// 生成新表记录账单数据
void CGameLogonManage::AutoCreateNewTable(bool start)
{
	// 获取系统配置，看看是否需要分表
	OtherConfig otherConfig = ConfigManage()->GetOtherConfig();

	if (!otherConfig.byIsDistributedTable)
	{
		return;
	}

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char tableName[256] = "";
	char date[64] = "";

	if (start)  //创建本月的表
	{
		sprintf(date, "%d%02d", sys.wYear, sys.wMonth);
	}
	else //创建下个月的表
	{
		if (sys.wDay != 26 && sys.wDay != 27 && sys.wDay != 28) //每个月只在26，27，28进行生成表
		{
			return;
		}
		sprintf(date, "%d%02d", sys.wYear, sys.wMonth == 12 ? 1 : sys.wMonth + 1);
	}

	//以下就是需要分表的表

	//"statistics_logonandlogout"
	sprintf(tableName, "%s_%s", TBL_STATI_LOGON_LOGOUT, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_STATI_LOGON_LOGOUT);

	//"statistics_moneychange"
	sprintf(tableName, "%s_%s", TBL_STATI_USER_MONEY_CHANGE, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_STATI_USER_MONEY_CHANGE);

	//"statistics_jewelschange"
	sprintf(tableName, "%s_%s", TBL_STATI_USER_JEWELS_CHANGE, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_STATI_USER_JEWELS_CHANGE);

	//"statistics_gamerecordinfo"
	sprintf(tableName, "%s_%s", TBL_STATI_GAME_RECORD_INFO, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_STATI_GAME_RECORD_INFO);

	////"statistics_rewardspool"
	//sprintf(tableName, "%s_%s", TBL_STATI_REWARDS_POOL, date);
	//BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
	//	tableName, TBL_STATI_REWARDS_POOL);

	////"statistics_firecoinchange"
	//sprintf(tableName, "%s_%s", TBL_STATI_USER_FIRECOIN_CHANGE, date);
	//BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
	//	tableName, TBL_STATI_USER_FIRECOIN_CHANGE);

	////"friendsGroupAccounts"
	//sprintf(tableName, "%s_%s", TBL_FG_RECORD_ACCOUNTS, date);
	//BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
	//	tableName, TBL_FG_RECORD_ACCOUNTS);

	////"friendsGroupDeskListCost"
	//sprintf(tableName, "%s_%s", TBL_FG_COST_ACCOUNTS, date);
	//BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
	//	tableName, TBL_FG_COST_ACCOUNTS);

	//"web_agent_pump_jewels"
	sprintf(tableName, "%s_%s", TBL_WEB_AGENT_PUMP_JEWELS, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_WEB_AGENT_PUMP_JEWELS);

	//"web_agent_pump_money"
	sprintf(tableName, "%s_%s", TBL_WEB_AGENT_PUMP_MONEY, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_WEB_AGENT_PUMP_MONEY);

	//"statistics_roomwincount"
	sprintf(tableName, "%s_%s", TBL_STATI_ROOM_WIN_COUNT, date);
	BillManage()->WriteBill(&m_SQLDataManage, "CREATE TABLE %s LIKE %s;",
		tableName, TBL_STATI_ROOM_WIN_COUNT);
}

//////////////////////////////////////////////////////////////////////////

// 判断roomID的服务器是否存在
bool CGameLogonManage::IsRoomIDServerExists(int roomID)
{
	if (m_pGServerManage->GetGServer(roomID) != NULL)
	{
		return true;
	}

	return false;
}

// 自动开房
bool CGameLogonManage::AutoCreateRoom(const SaveRedisFriendsGroupDesk& deskInfo)
{
	int userID = deskInfo.userID;

	UserData userData;
	if (!m_pRedis->GetUserData(userID, userData))
	{
		ERROR_LOG("GetUserData failed userID=%d", userID);
		return false;
	}

	GameBaseInfo* pGameBaseInfo = ConfigManage()->GetGameBaseInfo(deskInfo.gameID);
	if (!pGameBaseInfo)
	{
		return false;
	}

	//获取roomID
	ConfigManage()->GetBuyRoomInfo(deskInfo.gameID, deskInfo.roomType, m_buyRoomVec);
	if (m_buyRoomVec.size() <= 0)
	{
		ERROR_LOG("没有roomID::gameID=%d,roomType=%d", deskInfo.gameID, deskInfo.roomType);
		return false;
	}
	int iBuyRoomID = m_buyRoomVec[CUtil::GetRandNum() % m_buyRoomVec.size()];

	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(iBuyRoomID);
	if (!pRoomBaseInfo)
	{
		ERROR_LOG("invalid roomID: %d", iBuyRoomID);
		return false;
	}

	//创建俱乐部房间,判断创建房间是否已经到达上限
	int	friendsGroupID = deskInfo.friendsGroupID;
	int friendsGroupDeskNumber = deskInfo.friendsGroupDeskNumber;
	if (friendsGroupID <= 0 || friendsGroupDeskNumber <= 0 || friendsGroupDeskNumber > (MAX_FRIENDSGROUP_DESK_LIST_COUNT + MAX_FRIENDSGROUP_VIP_ROOM_COUNT))
	{
		ERROR_LOG("俱乐部异常: friendsGroupID=%d,friendsGroupDeskNumber=%d", friendsGroupID, friendsGroupDeskNumber);
		return false;
	}

	// 判断俱乐部是否存在
	if (!m_pRedisPHP->IsCanJoinFriendsGroupRoom(userID, friendsGroupID))
	{
		ERROR_LOG("俱乐部不存在 friendsGroupID=%d,userID=%d", friendsGroupID, userID);
		return false;
	}

	//查找是否有这个配置
	BuyGameDeskInfo buyGameDeskInfo;
	BuyGameDeskInfo* pBuyGameDeskInfo = NULL;
	BuyGameDeskInfoKey buyDekKey(deskInfo.gameID, deskInfo.maxCount, pRoomBaseInfo->type);
	if (m_pRedis->GetBuyGameDeskInfo(buyDekKey, buyGameDeskInfo))
	{
		pBuyGameDeskInfo = &buyGameDeskInfo;
	}
	else
	{
		pBuyGameDeskInfo = ConfigManage()->GetBuyGameDeskInfo(buyDekKey);
	}
	if (!pBuyGameDeskInfo)
	{
		ERROR_LOG("配置不存在:gameID=%d,count=%d,roomType=%d", deskInfo.gameID, deskInfo.maxCount, pRoomBaseInfo->type);
		return false;
	}

	// 解析json
	int iJsonRS = 0;
	int buyGameCount = pBuyGameDeskInfo->count;
	int payType = PAY_TYPE_NORMAL;
	bool bAlreadyCost = false;
	int needCostNums = 0;
	// 解析支付方式
	if (deskInfo.gameRules[0] != '\0')
	{
		Json::Reader reader;
		Json::Value value;
		if (!reader.parse(deskInfo.gameRules, value))
		{
			ERROR_LOG("parse gameRules(%s) failed", deskInfo.gameRules);
			return false;
		}

		payType = value["pay"].asInt();
		iJsonRS = value["rs"].asInt();
	}
	if (pBuyGameDeskInfo->costResType <= 0)
	{
		pBuyGameDeskInfo->costResType = RESOURCE_TYPE_JEWEL;
	}

	////////////////////////////////同步数据(一个人不能同时购买桌子)//////////////////////////////////////////
	char redisLockKey[48] = "";
	sprintf(redisLockKey, "%s|%d", TBL_USER_BUY_DESK, userID);
	CRedisLock redisLock(m_pRedisPHP->GetRedisContext(), redisLockKey, 5);

	if (pRoomBaseInfo->type == ROOM_TYPE_PRIVATE)
	{
		if (payType != PAY_TYPE_AA)
		{
			payType = PAY_TYPE_NORMAL;
		}

		// 检查房卡
		if (payType == PAY_TYPE_NORMAL)
		{
			needCostNums = pBuyGameDeskInfo->costNums;
		}
		else
		{
			needCostNums = pBuyGameDeskInfo->AAcostNums;
		}

		if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY)
		{
			if (userData.money < needCostNums)
			{
				ERROR_LOG("房主金币不足，自动开房失败,userID = %d,userData.money=%lld,needCostNums=%d", userID, userData.money, needCostNums);
				return false;
			}
		}
		else
		{
			if (userData.jewels < needCostNums)
			{
				ERROR_LOG("房主钻石不足，自动开房失败,userID = %d,userData.jewels=%d,needCostNums=%d", userID, userData.jewels, needCostNums);
				return false;
			}
		}

		//房主支付先扣
		if (payType == PAY_TYPE_NORMAL)
		{
			bAlreadyCost = true;
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FRIEND)
	{
		if (payType == PAY_TYPE_NORMAL)
		{
			needCostNums = pBuyGameDeskInfo->costNums;
			bAlreadyCost = true;

			if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY)
			{
				if (userData.money < needCostNums) //消耗俱乐部群主的
				{
					ERROR_LOG("房主金币不足，自动开房失败,userID = %d,userData.money=%lld,needCostNums=%d", userID, userData.money, needCostNums);
					return false;
				}
			}
			else
			{
				if (userData.jewels < needCostNums)
				{
					ERROR_LOG("房主钻石不足，自动开房失败,userID = %d,userData.jewels=%d,needCostNums=%d", userID, userData.jewels, needCostNums);
					return false;
				}
			}
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FG_VIP)//俱乐部VIP房
	{
		if (payType == PAY_TYPE_NORMAL)
		{
			needCostNums = pBuyGameDeskInfo->costNums;
			bAlreadyCost = true;

			if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY)
			{
				if (userData.money < needCostNums) //消耗俱乐部群主的
				{
					ERROR_LOG("房主金币不足，自动开房失败,userID = %d,userData.money=%lld,needCostNums=%d", userID, userData.money, needCostNums);
					return false;
				}
			}
			else
			{
				if (userData.jewels < needCostNums)
				{
					ERROR_LOG("房主钻石不足，自动开房失败,userID = %d,userData.jewels=%d,needCostNums=%d", userID, userData.jewels, needCostNums);
					return false;
				}
			}
		}
	}
	else
	{
		ERROR_LOG("房间类型错误");
		return false;
	}

	int maxUserCount = pGameBaseInfo->deskPeople;
	// 特殊游戏的处理
	if (pGameBaseInfo->multiPeopleGame)
	{
		maxUserCount = iJsonRS;
	}

	// 创建房间
	std::string deskPasswd = m_pRedis->CreatePrivateDeskRecord(userID, pRoomBaseInfo->roomID, 1, pRoomBaseInfo->deskCount,
		buyGameCount, deskInfo.gameRules, maxUserCount, payType, pGameBaseInfo->watcherCount, friendsGroupID, friendsGroupDeskNumber);
	if (deskPasswd == "")
	{
		ERROR_LOG("CreatePrivateDeskRecord failed userID: %d, deskCount: %d", userID, pRoomBaseInfo->deskCount);
		return false;
	}

	//房主支付，先扣
	if (bAlreadyCost && needCostNums > 0)
	{
		if (pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL)
		{
			long long newJewels = userData.jewels - needCostNums;
			m_pRedis->SetUserJewelsEx(userID, -needCostNums, true, RESOURCE_CHANGE_REASON_CREATE_ROOM, iBuyRoomID, 0, userData.isVirtual, friendsGroupID);

			// 解锁
			redisLock.Unlock();

			// 通知资源变化
			NotifyResourceChange(userID, RESOURCE_TYPE_JEWEL, newJewels, RESOURCE_CHANGE_REASON_CREATE_ROOM, -needCostNums);
		}
		else
		{
			long long newGolds = userData.money - needCostNums;
			m_pRedis->SetUserMoneyEx(userID, -needCostNums, true, RESOURCE_CHANGE_REASON_CREATE_ROOM, iBuyRoomID, 0, userData.isVirtual, friendsGroupID);

			// 解锁
			redisLock.Unlock();

			//算房卡场系统赢钱
			m_pRedis->SetRoomGameWinMoney(iBuyRoomID, needCostNums, true);

			// 通知资源变化
			NotifyResourceChange(userID, RESOURCE_TYPE_MONEY, newGolds, RESOURCE_CHANGE_REASON_CREATE_ROOM, -needCostNums);
		}
	}

	// 解锁
	redisLock.Unlock();

	//添加俱乐部牌桌消息
	OneFriendsGroupDeskInfo deskInfoMsg;
	bool bHaveRedSpot = false;
	bool bRet = m_pRedis->CreateFriendsGroupDeskInfo(friendsGroupID, friendsGroupDeskNumber, deskPasswd, pRoomBaseInfo->gameID, pRoomBaseInfo->type, deskInfoMsg, bHaveRedSpot);
	if (bRet)
	{
		// 发给中心服务器，让中心服务器转发
		//推送给所有在线成员
		UINT iMessageID = friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT
			? MSG_NTF_LOGON_FRIENDSGROUP_NEW_VIPROOM_MSG : MSG_NTF_LOGON_FRIENDSGROUP_NEW_DESK_MSG;
		SendMessageToCenterServer(CENTER_MESSAGE_LOGON_RELAY_FG_MSG, &deskInfoMsg, sizeof(deskInfoMsg), friendsGroupID, MSG_MAIN_FRIENDSGROUP_NOTIFY, iMessageID, 0);

		if (bHaveRedSpot)
		{
			PlatformFriendsGroupPushRedSpot msgRedSpot;
			msgRedSpot.friendsGroupID = friendsGroupID;
			if (friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT)
			{
				msgRedSpot.VIPRoomMsgRedSpotCount = 1;
			}
			else
			{
				msgRedSpot.deskMsgRedSpotCount = 1;
			}

			SendMessageToCenterServer(CENTER_MESSAGE_LOGON_REQ_FG_REDSPOT_MSG, &msgRedSpot, sizeof(msgRedSpot), friendsGroupID, 0, 0, 0);
		}
	}
	else
	{
		ERROR_LOG("自动开房失败,masterID = %d , friendsGroupID = %d,friendsGroupDeskNumber = %d", userID, friendsGroupID, friendsGroupDeskNumber);
	}

	return  bRet;
}

// 判断此ip是否可以注册
bool CGameLogonManage::IsIpRegister(const OtherConfig& otherConfig, const char* ip)
{
	if (!ip)
	{
		return true;
	}

	if (!otherConfig.byIsIPRegisterLimit)
	{
		return true;
	}

	char sql[128] = "";
	sprintf(sql, "SELECT count(*) as ipcount from userinfo where registerIP='%s'", ip);

	CMysqlHelper::MysqlData dataSet;
	try
	{
		m_SQLDataManage.m_pMysqlHelper->queryRecord(sql, dataSet);
	}
	catch (MysqlHelper_Exception & excep)
	{
		ERROR_LOG("执行sql语句失败:%s", excep.errorInfo.c_str());
		return true;
	}

	if (dataSet.size() != 1)
	{
		return true;
	}

	int ipcount = 0;
	CConfigManage::sqlGetValue(dataSet[0], "ipcount", ipcount);

	if (ipcount >= otherConfig.IPRegisterLimitCount)
	{
		return false;
	}

	return true;
}

// 删除程序生成的无效文件
void CGameLogonManage::DeleteExpireFile()
{
	char delJsonPath[128] = "";

	sprintf(delJsonPath, "%s%s", SAVE_JSON_PATH, "DeleteJson.bat");
	system(delJsonPath);
}

// 判断某个玩家是否是游客
bool CGameLogonManage::IsVisitorUser(int userID)
{
	UserData userData;
	if (m_pRedis->GetUserData(userID, userData) && userData.registerType == LOGON_VISITOR)
	{
		return true;
	}

	return false;
}

std::string CGameLogonManage::GetRandHeadURLBySex(BYTE sex)
{
	int headURLID = 0;

	std::string prefix = "http://";
	std::string str = prefix + WEB_ADDRESS;

	if (sex == USER_SEX_MALE)
	{
		int diff = MAX_MAN_HEADURL_ID - MIN_MAN_HEADURL_ID;
		headURLID = CUtil::GetRandNum() % diff + MIN_MAN_HEADURL_ID;
	}
	else
	{
		int diff = MAX_WOMAN_HEADURL_ID - MIN_WOMAN_HEADURL_ID;
		headURLID = CUtil::GetRandNum() % diff + MIN_WOMAN_HEADURL_ID;
	}

	char buf[128] = "";
	sprintf(buf, "/%d.jpg", headURLID);

	str += buf;

	return str;
}

////////////////////////////////处理中心服消息//////////////////////////////////////////
bool CGameLogonManage::OnCenterServerMessage(UINT msgID, NetMessageHead* pNetHead, void* pData, UINT size, int userID)
{
	switch (msgID)
	{
	case CENTER_MESSAGE_COMMON_REPEAT_ID:
	{
		return OnCenterRepeatIDMessage(pData, size);
	}
	case CENTER_MESSAGE_COMMON_LOGON_GROUP_INFO:
	{
		return OnCenterDistributedSystemInfoMessage(pData, size);
	}
	case CENTER_MESSAGE_LOGON_REPEAT_USER_LOGON:
	{
		return OnCenterKickOldUserMessage(pData, size);
	}
	case PLATFORM_MESSAGE_IDENTUSER:
	{
		return OnCenterKickUserMessage(pData, size, userID);
	}
	case PLATFORM_MESSAGE_NOTICE:
	{
		return OnCenterNoticeMessage(pData, size);
	}
	case PLATFORM_MESSAGE_REQ_ALL_USER_MAIL:
	{
		return OnCenterAllUserMailMessage(pData, size);
	}
	case PLATFORM_MESSAGE_CLOSE_SERVER:
	{
		return OnCenterCloseServerMessage(pData, size);
	}
	case PLATFORM_MESSAGE_OPEN_SERVER:
	{
		return OnCenterOpenServerMessage(pData, size);
	}
	case PLATFORM_MESSAGE_SEND_HORN:
	{
		return OnCenterSendHornMessage(pData, size);
	}
	case PLATFORM_MESSAGE_NOTIFY_USERID:
	{
		return OnCenterNotifyUserMessage(pNetHead, pData, size, userID);
	}
	case CENTER_MESSAGE_COMMON_AUTO_CREATEROOM:
	{
		return OnCenterAutoCreateRoomMessage(pData, size);
	}
	case PLATFORM_MESSAGE_PHONE_INFO:
	{
		return OnCenterPhoneInfoMessage(pData, size);
	}
	case CENTER_MESSAGE_LOADER_REWARD_ACTIVITY:
	{
		return OnCenterActivityRewardsMessage(pData, size);
	}
	case PLATFORM_MESSAGE_SIGN_UP_MATCH_PEOPLE:
	{
		return OnCenterSignUpMatchMessage(pData, size);
	}
	case CENTER_MESSAGE_LOADER_NOTIFY_START_MATCH:
	{
		return OnCenterLoaderStartMatchMessage(pData, size);
	}
	case PLATFORM_MESSAGE_SIGN_UP_MATCH_TIME:
	{
		return OnCenterTimeMatchPeopleChangeMessage(pData, size);
	}
	default:
		break;
	}
	return false;
}

// 服务器id重复处理
bool CGameLogonManage::OnCenterRepeatIDMessage(void* pData, int size)
{
	if (size != 0)
	{
		return false;
	}

	ERROR_LOG("################ 服务器唯一的logonID重复(logonID = %d)，启动失败，请重新配置logonID ！！！ ####################", ConfigManage()->GetLogonServerConfig().logonID);

	exit(0);

	return true;
}

// 分布式系统信息
bool CGameLogonManage::OnCenterDistributedSystemInfoMessage(void* pData, int size)
{
	if (size != sizeof(PlatformDistributedSystemInfo))
	{
		return false;
	}

	PlatformDistributedSystemInfo* pMessage = (PlatformDistributedSystemInfo*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (!m_pRedis)
	{
		return false;
	}

	if (pMessage->logonGroupCount <= 0)
	{
		ERROR_LOG("中心服发送数据错误 pMessage->logonGroupCount%d", pMessage->logonGroupCount);
		return false;
	}

	// 设置本系统在集群的位置信息
	m_pRedis->m_uLogonGroupCount = pMessage->logonGroupCount;
	m_pRedis->m_uLogonGroupIndex = pMessage->logonGroupIndex;
	m_pRedis->m_uMainLogonGroupIndex = pMessage->mainLogonGroupIndex;

	INFO_LOG("集群信息：m_uLogonGroupCount = %d , m_uLogonGroupIndex = %d , m_uMainLogonGroupIndex = %d",
		m_pRedis->m_uLogonGroupCount, m_pRedis->m_uLogonGroupIndex, m_pRedis->m_uMainLogonGroupIndex);

	static bool bStart = false;
	// 启动生成分表
	if (m_pRedis->m_uLogonGroupIndex == 0 && !bStart)
	{
		bStart = true;
		AutoCreateNewTable(true);
	}

	return true;
}

// 踢掉本服旧玩家（不同设备同时登陆使用）
bool CGameLogonManage::OnCenterKickOldUserMessage(void* pData, UINT size)
{
	if (size != sizeof(PlatformRepeatUserLogon))
	{
		return false;
	}

	PlatformRepeatUserLogon* pMessage = (PlatformRepeatUserLogon*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (!m_pUserManage)
	{
		return false;
	}

	// 玩家是否登录了
	LogonUserInfo* pUser = m_pUserManage->GetUser(pMessage->userID);
	if (pUser)
	{
		int oldSocketIdx = pUser->socketIdx;

		// 不同设备
		m_TCPSocket.SendData(oldSocketIdx, NULL, 0, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_USER_SQUEEZE, 0, 0);
		//m_TCPSocket.CloseSocket(oldSocketIdx);

		// 清理玩家内存
		m_pUserManage->DelUser(pMessage->userID);
		DelSocketIdx(oldSocketIdx);
	}
	else
	{
		ERROR_LOG("旧服务器不存在玩家:userID=%d", pMessage->userID);
		return false;
	}

	return true;
}

// 踢人掉线消息(封号使用)
bool CGameLogonManage::OnCenterKickUserMessage(void* pData, int size, int userID)
{
	if (size != 0)
	{
		return false;
	}

	// 玩家是否登录了
	LogonUserInfo* pUser = m_pUserManage->GetUser(userID);
	if (pUser)
	{
		INFO_LOG("收到中心服踢人消息,userID=%d", userID);
		return m_TCPSocket.CloseSocket(pUser->socketIdx);
	}

	return true;
}

// 发布公告
bool CGameLogonManage::OnCenterNoticeMessage(void* pData, int size)
{
	if (size != sizeof(PlatformNoticeMessage))
	{
		return false;
	}

	PlatformNoticeMessage* pMessage = (PlatformNoticeMessage*)pData;
	if (!pMessage)
	{
		return false;
	}

	LogonNotifyNotice msg;

	memcpy(msg.tile, pMessage->tile, sizeof(msg.tile));
	memcpy(msg.content, pMessage->content, sizeof(msg.content));
	msg.interval = pMessage->interval;
	msg.times = pMessage->times;		// 告诉前端的都只有一次
	msg.type = pMessage->type;

	// 计算发送字节数量
	msg.sizeCount = min(strlen(msg.content) + 1, sizeof(msg.content));
	int iSendSize = 40 + msg.sizeCount;

	// 全服在线玩家发送公告
	SendDataBatch(&msg, iSendSize, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_NOTICE, 0);

	return true;
}

// 全服邮件通知
bool CGameLogonManage::OnCenterAllUserMailMessage(void* pData, int size)
{
	if (size != 0)
	{
		return false;
	}

	const auto& onlineUserMap = m_pUserManage->GetLogonUserInfoMap();
	for (auto iter = onlineUserMap.begin(); iter != onlineUserMap.end(); ++iter)
	{
		int userID = iter->first;

		UserRedSpot userRedspot;
		if (!m_pRedis->GetUserRedSpot(userID, userRedspot))
		{
			continue;
		}

		LogonNotifyEmailRedSpot msg;

		msg.notReadCount = userRedspot.notEMRead;
		msg.notReceivedCount = userRedspot.notEMReceived;

		SendData(userID, &msg, sizeof(msg), MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_EMAIL_REDSPOT, 0);
	}

	return true;
}

// 关服处理
bool CGameLogonManage::OnCenterCloseServerMessage(void* pData, int size)
{
	if (size != 0)
	{
		return false;
	}

	if (!m_pRedis)
	{
		return false;
	}

	time_t currTime = time(NULL);

	// 统计在线时长，以及发送关服通知
	const auto& onlineUserMap = m_pUserManage->GetLogonUserInfoMap();
	for (auto iter = onlineUserMap.begin(); iter != onlineUserMap.end(); ++iter)
	{
		int userID = iter->first;
		if (!iter->second->isVirtual)
		{
			long long lastCalcOnlineToTime = 0;
			if ((lastCalcOnlineToTime = m_pRedis->GetUserResNums(userID, "lastCalcOnlineToTime")) > 0)
			{
				m_pRedis->AddUserResNums(userID, "allOnlineToTime", currTime - lastCalcOnlineToTime);
				m_pRedis->SetUserResNums(userID, "lastCalcOnlineToTime", 0);
			}

			m_pRedis->SetUserInfo(userID, " IsOnline 0 ");

			//发送消息通知下线
			SendData(userID, NULL, 0, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_CLOSE_SERVER, 0);
		}
	}

	//保存所有数据到db
	RountineSaveRedisDataToDB(true);

	INFO_LOG("==========LogonServer 关闭成功===========");

	return true;
}

// 开服处理
bool CGameLogonManage::OnCenterOpenServerMessage(void* pData, int size)
{
	if (!m_pRedis)
	{
		return false;
	}

	AUTOCOST("开服耗时，数据恢复");

	int iAllData = 0, iSaveData = 0;

	// 获取保存的数据
	std::vector<SaveRedisFriendsGroupDesk> vecSaveRedisFriendsGroupDesk;
	iAllData = m_pRedis->GetAllTempFgDesk(vecSaveRedisFriendsGroupDesk);

	for (size_t i = 0; i < vecSaveRedisFriendsGroupDesk.size(); i++)
	{
		if (!AutoCreateRoom(vecSaveRedisFriendsGroupDesk[i]))
		{
			ERROR_LOG("恢复牌桌失败 userID = %d friendsGroupID = %d,friendsGroupDeskNumber = %d", vecSaveRedisFriendsGroupDesk[i].userID,
				vecSaveRedisFriendsGroupDesk[i].friendsGroupID, vecSaveRedisFriendsGroupDesk[i].friendsGroupDeskNumber);
			continue;
		}

		iSaveData++;
	}

	INFO_LOG("本服恢复了%d/%d个牌桌", iSaveData, iAllData);

	INFO_LOG("========LogonServer 恢复数据成功========");

	return true;
}

// 玩家发送喇叭
bool CGameLogonManage::OnCenterSendHornMessage(void* pData, UINT size)
{
	if (size != sizeof(PlatformHorn))
	{
		return false;
	}

	PlatformHorn* pMessage = (PlatformHorn*)pData;
	if (!pMessage)
	{
		return false;
	}

	LogonNotifyHorn msg;

	memcpy(msg.content, pMessage->content, sizeof(msg.content));
	memcpy(msg.userName, pMessage->userName, sizeof(msg.userName));
	msg.userID = pMessage->userID;

	// 计算发送字节数量
	msg.sizeCount = min(strlen(msg.content) + 1, sizeof(msg.content));
	int iSendSize = 72 + msg.sizeCount;

	// 广播这个公告
	SendDataBatch(&msg, iSendSize, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_HORN, 0);

	return true;
}

// 向某个玩家推送消息
bool CGameLogonManage::OnCenterNotifyUserMessage(NetMessageHead* pNetHead, void* pData, int size, int userID)
{
	// 直接发送数据给用户
	SendData(userID, pData, size, pNetHead->uMainID, pNetHead->uAssistantID, pNetHead->uHandleCode);

	return true;
}

// 中心服自动开房
bool CGameLogonManage::OnCenterAutoCreateRoomMessage(void* pData, UINT size)
{
	if (size != sizeof(PlatformAutoCreateRoom))
	{
		return false;
	}

	PlatformAutoCreateRoom* pMessage = (PlatformAutoCreateRoom*)pData;
	if (!pMessage)
	{
		return false;
	}

	int masterID = pMessage->masterID;
	int friendsGroupID = pMessage->friendsGroupID;
	int friendsGroupDeskNumber = pMessage->friendsGroupDeskNumber;

	SaveRedisFriendsGroupDesk deskInfo;

	deskInfo.userID = masterID;
	deskInfo.friendsGroupID = friendsGroupID;
	deskInfo.friendsGroupDeskNumber = friendsGroupDeskNumber;
	deskInfo.roomType = pMessage->roomType;
	deskInfo.gameID = pMessage->gameID;
	deskInfo.maxCount = pMessage->maxCount;
	memcpy(deskInfo.gameRules, pMessage->gameRules, sizeof(deskInfo.gameRules));

	bool bRet = AutoCreateRoom(deskInfo);
	if (!bRet)
	{
		// 删除redis中的牌桌
		m_pRedis->DelFGDeskRoom(friendsGroupID, friendsGroupDeskNumber);

		ERROR_LOG("自动开房失败,masterID = %d , friendsGroupID = %d,friendsGroupDeskNumber = %d", masterID, friendsGroupID, friendsGroupDeskNumber);
	}

	return true;
}

bool CGameLogonManage::OnCenterPhoneInfoMessage(void* pData, int size)
{
	if (size != sizeof(UserData))
	{
		return false;
	}

	UserData* pMessage = (UserData*)pData;
	if (!pMessage)
	{
		return false;
	}

	if (CUtil::IsContainDirtyWord(pMessage->name))
	{
		ERROR_LOG("name包含非法词：%s", pMessage->name);
		return false;
	}

	if (m_pRedis->GetUserIDByPhone(pMessage->phone) > 0)
	{
		ERROR_LOG("账号已经存在,phone=%s", pMessage->phone);
		return false;
	}

	UserData userData = *pMessage;

	// 保存到redis
	int ret = m_pRedis->Register(userData, LOGON_TEL_PHONE);
	if (ret < 0)
	{
		ERROR_LOG("注册账号失败:phone=%s,userID=%d,name=%s", userData.phone, userData.userID, userData.name);
		return false;
	}

	userData.userID = ret;

	OnUserRegister(userData);

	return true;
}

bool CGameLogonManage::OnCenterActivityRewardsMessage(void* pData, int size)
{
	SAFECHECK_MESSAGE(pMessage, LogonNotifyActivity, pData, size);

	// 全服推送
	LogonNotifyActivity msg;

	memcpy(msg.content, pMessage->content, sizeof(msg.content));
	msg.sizeCount = pMessage->sizeCount;

	// 计算发送字节数量
	int iSendSize = 4 + msg.sizeCount;

	// 广播这个公告
	SendDataBatch(&msg, iSendSize, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_REWARD_ACTIVITY, 0);

	return true;
}

// 有人报名或者退出比赛（实时赛）
bool CGameLogonManage::OnCenterSignUpMatchMessage(void* pData, UINT size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPSignUpMatchPeople, pData, size);

	LogonPeopleMatchSignUpPeopleChange msg;

	msg.gameID = pMessage->gameID;
	msg.matchID = pMessage->matchID;
	msg.curSignUpCount = pMessage->curSignUpCount;
	msg.peopleCount = pMessage->peopleCount;
	msg.allSignUpCount = pMessage->allSignUpCount;

	for (auto itr = m_scoketMatch.begin(); itr != m_scoketMatch.end(); itr++)
	{
		m_TCPSocket.SendData(*itr, &msg, sizeof(msg), MSG_MAIN_MATCH_NOTIFY, MSG_NTF_LOGON_MATCH_NOTIFY_SIGNUP_CHANGE_PEOPLE, 0, 0);
	}

	return true;
}

// 比赛场通知玩家进入比赛
bool CGameLogonManage::OnCenterLoaderStartMatchMessage(void* pData, UINT size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformLoaderNotifyStartMatch, pData, size);

	LogonMatchJoinRoom msg;
	msg.gameID = pMessage->gameID;
	msg.matchType = pMessage->matchType;
	msg.matchID = pMessage->matchID;
	msg.roomID = pMessage->roomID;

	for (int i = 0; i < pMessage->peopleCount; i++)
	{
		if (IsUserOnline(pMessage->userID[i]))
		{
			SendData(pMessage->userID[i], &msg, sizeof(msg), MSG_MAIN_MATCH_NOTIFY, MSG_NTF_LOGON_MATCH_NOTIFY_START_MATCH, 0);
		}
	}

	return true;
}

// 有人报名或者退出比赛（定时赛）
bool CGameLogonManage::OnCenterTimeMatchPeopleChangeMessage(void* pData, UINT size)
{
	SAFECHECK_MESSAGE(pMessage, PlatformPHPSignUpMatchTime, pData, size);

	LogonTimeMatchSignUpPeopleChange msg;

	msg.matchID = pMessage->matchID;
	msg.curSignUpCount = pMessage->curSignUpCount;

	for (auto itr = m_scoketMatch.begin(); itr != m_scoketMatch.end(); itr++)
	{
		m_TCPSocket.SendData(*itr, &msg, sizeof(msg), MSG_MAIN_MATCH_NOTIFY, MSG_NTF_LOGON_MATCH_NOTIFY_SIGNUP_CHANGE_TIME, 0, 0);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
void CGameLogonManage::SendHTTPMessage(int userID, const std::string& url, BYTE postType)
{
	// HTTP请求
	AsyncEventMsgHTTP asyncEvent;
	strcpy(asyncEvent.url, url.c_str());

	m_SQLDataManage.PushLine(&asyncEvent.dataLineHead, sizeof(AsyncEventMsgHTTP), ASYNC_EVENT_HTTP, userID, postType);
}

bool CGameLogonManage::SendHTTPUserRegisterMessage(int userID)
{
	if (userID <= 0)
	{
		return false;
	}

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
	url += "/hm_ucenter/web/index.php?api=c&action=userRegister&userID=";
	url += bufUserID;

	SendHTTPMessage(userID, url, HTTP_POST_TYPE_REGISTER);

	return true;
}

bool CGameLogonManage::SendHTTPUserLogonLogout(int userID, BYTE type)
{
	if (userID <= 0)
	{
		return false;
	}

	char bufUserID[20] = "";
	sprintf(bufUserID, "%d", userID);

	//组合生成URL
	std::string url = "http://";
	url += ConfigManage()->GetOtherConfig().http;
	if (type == 0)  // 登陆
	{
		url += "/hm_ucenter/web/index.php?api=c&action=userLogin&userID=";
	}
	else // 登出
	{
		url += "/hm_ucenter/web/index.php?api=c&action=userLogout&userID=";
	}
	url += bufUserID;

	SendHTTPMessage(userID, url, type == 0 ? HTTP_POST_TYPE_LOGON : HTTP_POST_TYPE_LOGOUT);

	return true;
}