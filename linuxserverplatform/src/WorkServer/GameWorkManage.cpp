#include "CommonHead.h"
#include "ErrorCode.h"
#include "json/json.h"
#include "AsyncEventMsg.pb.h"
#include "GameWorkManage.h"
#include "GameWorkModule.h"

// protobuf 相关命名空间
using namespace AsyncEventMsg;

// 预编译选项
#define FRIENDSGROUP_CRATE_ROOM_MODE	// 俱乐部创建房间，房主消耗钻石


//////////////////////////////////////////////////////////////////////
CGameWorkManage::CGameWorkManage() : CBaseWorkServer()
{
	m_lastNormalTimerTime = 0;
	m_uGroupIndex = 0;
	m_uGroupCount = 1;
	m_uMainGroupIndex = 0;
}

//////////////////////////////////////////////////////////////////////
CGameWorkManage::~CGameWorkManage()
{
	m_buyRoomVec.clear();
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnSocketRead(NetMessageHead* pNetHead, void* pData, UINT uSize, UINT uIndex)
{
	if (!pNetHead)
	{
		return false;
	}

	// 性能统计
	AUTOCOST("message cost assistID: Mainid=%d AssistantID=%d", pNetHead->uMainID, pNetHead->uAssistantID);

	// 测试调用lua






	switch (pNetHead->uMainID)
	{
	case MSG_ASS_LOGON_DESK:
	{
		return OnHandleGameDeskMessage(pNetHead->uAssistantID, pData, uSize, uIndex, pNetHead->uIdentification);
	}
	case MSG_MAIN_LOGON_OTHER:
	{
		return OnHandleOtherMessage(pNetHead->uAssistantID, pData, uSize, uIndex, pNetHead->uIdentification);
	}
	default:
	{
		return false;
	}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnStart()
{
	INFO_LOG("GameWorkManage OnStart begin...");

	// 初始化lua相关
	if (!InitLua())
	{
		CON_ERROR_LOG("InitLua failed");
		return false;
	}

	// 注册lua调用c/c++函数
	CFuncRegister();

	// 加载lua文件
	if (!LoadAllLuaFile())
	{
		CON_ERROR_LOG("LoadAllLuaFile failed");
		return false;
	}

	m_socketIndexVec.clear();

	if (m_pRedis)
	{
		m_pRedis->SetDBManage(&m_SQLDataManage);

		//清理在线人数
		std::unordered_map<std::string, std::string> umap;
		umap["curPeople"] = "0";
		umap["socketCount"] = "0";
		umap["curWebSocketPeople"] = "0";
		umap["webSocketCount"] = "0";
		m_pRedis->hmset(TBL_BASE_LOGON, ConfigManage()->m_logonServerConfig.logonID, umap);
	}

	if (m_pRedisPHP)
	{
		m_pRedisPHP->SetDBManage(&m_SQLDataManage);
	}

	m_lastNormalTimerTime = time(NULL);

	InitRounteCheckEvent();

	//// 测试异步获取数据库数据的代码
	//AsyncEventMsgSqlStatement msg;
	//char sql[128] = "select userID,name,headURL,money from userInfo order by money desc limit 5";
	//memcpy(msg.sql, sql, sizeof(sql));
	//msg.bIsSelect = true;
	//m_SQLDataManage.PushLine(&msg.dataLineHead, sizeof(msg) - sizeof(msg.sql) + strlen(msg.sql), ASYNC_EVENT_SQL_STATEMENT, DB_TYPE_COMMON, ASYNC_MESSAGE_DATABASE_TEST);

	INFO_LOG("GameWorkManage OnStart end.");

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnStop()
{
	lua_close(m_pLuaState);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnTimerMessage(UINT uTimerID)
{
	AUTOCOST("timerID=%d", uTimerID);


	lua_getglobal(m_pLuaState, "g_isdubug");
	//printf("g_isdubug=%s\n", lua_tostring(m_pLuaState, -1));
	lua_pop(m_pLuaState, 1);

	// 是否 从新加载文件
	static int df = 0;
	//printf("计数：%d\n", df);
	if (df % 8 == 7)
	{
		//printf("加载文件\n");
		if (luaL_dofile(m_pLuaState, "./WorkServerTest.lua") != 0)
		{
			CON_ERROR_LOG("%s\n", lua_tostring(m_pLuaState, -1));
			return false;
		}
	}
	df++;

	// 测试调用lua
	lua_getglobal(m_pLuaState, "testload");
	lua_pushinteger(m_pLuaState, 123);
	if (lua_pcall(m_pLuaState, 1, 0, 0))
	{
		std::cout << "LUA_ERROR " << lua_tostring(m_pLuaState, -1) << std::endl;
		lua_pop(m_pLuaState, 1);
	}
	//printf("%d\n", lua_tointeger(m_pLuaState, -1));

	//printf("栈顶：%d\n", lua_gettop(m_pLuaState));

	lua_getglobal(m_pLuaState, "testreturn");
	if (lua_pcall(m_pLuaState, 0, 4, 0))
	{
		std::cout << "LUA_ERROR " << lua_tostring(m_pLuaState, -1) << std::endl;
		lua_pop(m_pLuaState, 1);
	}
	/*printf("%s\n", lua_tostring(m_pLuaState, 1));
	printf("%s\n", lua_tostring(m_pLuaState, 2));
	printf("%s\n", lua_tostring(m_pLuaState, 3));
	printf("%s\n", lua_tostring(m_pLuaState, 4));*/
	lua_pop(m_pLuaState, 4);



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
	case LOGON_TIMER_NORMAL:
	{
		OnNormalTimer();
		return true;
	}
	default:
		break;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnAsynThreadResult(AsynThreadResultLine* pResultData, void* pData, UINT uSize)
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
bool CGameWorkManage::PreInitParameter(ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData)
{
	if (!pInitData || !pKernelData)
	{
		return false;
	}

	int workID = ConfigManage()->GetWorkServerConfig().workID;




	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnSocketClose(ULONG uAccessIP, UINT socketIdx, UINT uConnectTime, BYTE socketType)
{
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnHandleGameDeskMessage(int assistID, void* pData, int size, unsigned int socketIdx, int userID)
{
	if (userID <= 0)
	{
		ERROR_LOG("没有绑定玩家id的异常 socketIdx:%d", socketIdx);
		return false;
	}

	switch (assistID)
	{
	case MSG_ASS_LOGON_BUY_DESK:
	{
		return OnHandleUserBuyDesk(userID, pData, size, socketIdx);
	}
	case MSG_ASS_LOGON_ENTER_DESK:
	{
		return OnHandleUserEnterDesk(userID, pData, size, socketIdx);
	}
	default:
		break;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnHandleUserBuyDesk(int userID, void* pData, int size, unsigned int socketIdx)
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_MATCH_PLAYING, userID);
		return true;
	}

	// 玩家在游戏中的话则无法创建房间，但是可以代开
	if (userData.roomID > 0 && pMessage->masterNotPlay == 0)
	{
		LogonResponseBuyDesk msg;
		msg.roomID = userData.roomID;
		SendData(socketIdx, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CANNOT_BUYDESK_INROOM, userID);

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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG, userID);
		return true;
	}

	ConfigManage()->GetBuyRoomInfo(pMessage->gameID, pMessage->roomType, m_buyRoomVec);
	if (m_buyRoomVec.size() <= 0)
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG, userID);
		return true;
	}
	int iBuyRoomID = m_buyRoomVec[CUtil::GetRandNum() % m_buyRoomVec.size()];

	RoomBaseInfo* pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(iBuyRoomID);
	if (!pRoomBaseInfo)
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG, userID);
		ERROR_LOG("购买桌子失败roomID=%d", iBuyRoomID);
		return true;
	}

	if (!IsRoomIDServerExists(iBuyRoomID))
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_GAME_NO_START, userID);
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_BUY_DESK_TOPLIMIT, userID);
		return true;
	}

	// 俱乐部vip房间只能俱乐部创建
	if (friendsGroupID > 0 && friendsGroupDeskNumber > MAX_FRIENDSGROUP_DESK_LIST_COUNT&& pRoomBaseInfo->type != ROOM_TYPE_FG_VIP)
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM, userID);
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_BUY_DESK_CONFIG, userID);
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_ILLEGAL_OPER, userID);
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
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_NOT_EXISTS, userID);
			return true;
		}

		// 获取俱乐部信息
		BYTE userPower = m_pRedisPHP->GetUserPower(friendsGroupID, userID);
		masterID = m_pRedisPHP->GetFriendsGroupMasterID(friendsGroupID);

		redisLockFG.Lock();

		int ret = m_pRedis->IsCanCreateFriendsGroupRoom(userID, friendsGroupID, userPower, friendsGroupDeskNumber);
		if (ret == 1)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_MANAGER, userID);
			return true;
		}
		else if (ret == 2)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_ERR_CREATE_ROOM, userID);
			return true;
		}
	}

	//特殊游戏判断，猜拳
	if (cqFriendsGroupID > 0 && pRoomBaseInfo->gameID == 37550102)
	{
		long long carryFireCoin = 0;
		if (!m_pRedisPHP->GetUserFriendsGroupMoney(cqFriendsGroupID, userID, carryFireCoin))
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_MANAGER, userID);
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
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_FRIENDSGROUP_ERR_CREATE_ROOM, userID);
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
				SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode, userID);
				return true;
			}
		}
		else
		{
			if (buyUserData.jewels < needCostNums)
			{
				UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_JEWELS : ERROR_NOT_ENOUGH_JEWELS;
				SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode, userID);
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
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM, userID);
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
					SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode, userID);
					return true;
				}
			}
			else
			{
				if (buyUserData.jewels < needCostNums)
				{
					UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_JEWELS : ERROR_NOT_ENOUGH_JEWELS;
					SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode, userID);
					return true;
				}
			}
		}

		// 金币房检查玩家的金币数量，够不够进入
		if (pMessage->masterNotPlay == 0 && pBuyGameDeskInfo->costResType == RESOURCE_TYPE_MONEY
			&& userData.money < (minPoint + needCostNums))
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_ROOM_NO_MONEYREQUIRE, userID);
			return true;
		}

		// 金币房检查玩家的金币数量，够不够进入
		if (pMessage->masterNotPlay == 0 && pBuyGameDeskInfo->costResType == RESOURCE_TYPE_JEWEL
			&& userData.money < minPoint)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_ROOM_NO_MONEYREQUIRE, userID);
			return true;
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FG_VIP)//俱乐部VIP房
	{
		if (minPoint < kickPoint || minPoint < basePoint)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM, userID);
			return true;
		}

		if (friendsGroupID <= 0) //vip房只有俱乐部可以创建
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM, userID);
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
					SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode, userID);
					return true;
				}
			}
			else
			{
				if (buyUserData.jewels < needCostNums)
				{
					UINT uErrorCode = friendsGroupID > 0 ? ERROR_FRIENDSGROUP_NOT_JEWELS : ERROR_NOT_ENOUGH_JEWELS;
					SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, uErrorCode, userID);
					return true;
				}
			}
		}
	}
	else
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_CREATE_ROOM, userID);
		return false;
	}

	int maxUserCount = pGameBaseInfo->deskPeople;
	// 特殊游戏的处理
	if (pGameBaseInfo->multiPeopleGame)
	{
		maxUserCount = iJsonRS;
		if (maxUserCount <= 0)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NO_SET_PEOPLE, userID);
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
		SendData(socketIdx, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, ERROR_NOVAILD_DESK, userID);
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

	SendData(socketIdx, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_BUY_DESK, 0, userID);

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGameWorkManage::OnHandleUserEnterDesk(int userID, void* pData, int size, unsigned int socketIdx)
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_MATCH_PLAYING, userID);
		return true;
	}

	// 已经在一个房间里面则无法加入房间
	if (userData.roomID > 0)
	{
		LogonResponseEnterDesk msg;
		msg.userID = userID;
		msg.roomID = userData.roomID;
		msg.deskIdx = userData.deskIdx;
		SendData(socketIdx, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_CANNOT_ENTERDESK_INROOM, userID);

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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NO_THIS_ROOM, userID);
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_GAME_NO_START, userID);
		return true;
	}

	//非俱乐部不能加入俱乐部房间
	int	friendsGroupID = privateDeskInfo.friendsGroupID;
	if (friendsGroupID != 0 && !m_pRedisPHP->IsCanJoinFriendsGroupRoom(userID, friendsGroupID))
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_FRIENDSGROUP_ERR_JOIN_ROOM, userID);
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
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_STOP_JOIN, userID);
		return true;
	}

	//特殊游戏判断，猜拳
	if (cqFriendsGroupID > 0 && pRoomBaseInfo->gameID == 37550102)
	{
		long long carryFireCoin = 0;
		if (!m_pRedisPHP->GetUserFriendsGroupMoney(cqFriendsGroupID, userID, carryFireCoin))
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_FRIENDSGROUP_ERR_JOIN_ROOM, userID);
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
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_ROOM_NO_MONEYREQUIRE, userID);
			return true;
		}
	}
	else if (pRoomBaseInfo->type == ROOM_TYPE_FG_VIP)
	{
		if (friendsGroupID <= 0)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NO_THIS_ROOM, userID);
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
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_FRIENDSGROUP_FIRECOIN_LIMIT, userID);
			return true;
		}
	}
	else
	{
		SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NO_THIS_ROOM, userID);
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
				SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NOT_ENOUGH_JEWELS, userID);
				return true;
			}
		}
		else
		{
			if (userData.money < pBuyGameDeskInfo->AAcostNums)
			{
				SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_NOT_ENOUGH_MONEY, userID);
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
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_DESK_FULL, userID);
			return true;
		}
	}
	else
	{
		if (privateDeskInfo.currDeskUserCount >= privateDeskInfo.maxDeskUserCount)
		{
			SendData(socketIdx, NULL, 0, MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, ERROR_DESK_FULL, userID);
			return true;
		}
	}

	// 记录玩家进入桌子
	m_pRedis->SetUserRoomID(userID, roomID);
	m_pRedis->SetUserDeskIdx(userID, deskIdx);

	msg.userID = userID;
	msg.roomID = roomID;
	msg.deskIdx = deskIdx;

	SendData(socketIdx, &msg, sizeof(msg), MSG_ASS_LOGON_DESK, MSG_ASS_LOGON_ENTER_DESK, 0, userID);

	return true;
}

bool CGameWorkManage::OnHandleOtherMessage(int assistID, void* pData, int size, unsigned int socketIdx, int userID)
{
	if (userID <= 0)
	{
		ERROR_LOG("没有绑定玩家id的异常 socketIdx:%d", socketIdx);
		return false;
	}

	switch (assistID)
	{
	case MSG_ASS_LOGON_OTHER_USERINFO_FLUSH:
	{
		return OnHandleUserInfoFlush(userID, pData, size, socketIdx);
	}
	case MSG_ASS_LOGON_OTHER_ROBOT_TAKEMONEY:
	{
		return OnHandleRobotTakeMoney(userID, pData, size, socketIdx);
	}
	case MSG_ASS_LOGON_OTHER_REQ_USERINFO:
	{
		return OnHandleReqUserInfo(userID, pData, size, socketIdx);
	}
	default:
		break;
	}

	return false;
}

bool CGameWorkManage::OnHandleUserInfoFlush(int userID, void* pData, int size, unsigned int socketIdx)
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

	SendData(socketIdx, &msg, sizeof(msg), MSG_MAIN_LOGON_OTHER, MSG_ASS_LOGON_OTHER_USERINFO_FLUSH, 0, userID);

	return true;
}

bool CGameWorkManage::OnHandleRobotTakeMoney(int userID, void* pData, int size, unsigned int socketIdx)
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

	SendData(socketIdx, &_res, sizeof(_res), MSG_MAIN_LOGON_OTHER, MSG_ASS_LOGON_OTHER_ROBOT_TAKEMONEY, 0, userID);

	return true;
}

bool CGameWorkManage::OnHandleReqUserInfo(int userID, void* pData, int size, unsigned int socketIdx)
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

	SendData(socketIdx, &msg, sizeof(msg), MSG_MAIN_LOGON_OTHER, MSG_ASS_LOGON_OTHER_REQ_USERINFO, 0, userID);

	return true;
}

bool CGameWorkManage::SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int uIdentification)
{
	if (m_pGServerConnect)
	{
		return m_pGServerConnect->SendData(index, pData, size, mainID, assistID, handleCode, uIdentification);
	}

	return true;
}

bool CGameWorkManage::SendData(int userID, void* pData, int size, int mainID, int assistID, int handleCode)
{
	return SendMessageToCenterServer(CENTER_MESSAGE_LOGON_RELAY_USER_MSG, pData, size, userID, mainID, assistID, handleCode);
}

void CGameWorkManage::NotifyResourceChange(int userID, int resourceType, long long value, int reason, long long changeValue)
{
	if (m_pRedis && m_pRedis->IsUserOnline(userID))
	{
		PlatformResourceChange msgToLoader;

		msgToLoader.resourceType = resourceType;
		msgToLoader.value = value;
		msgToLoader.reason = reason;
		msgToLoader.changeValue = changeValue;

		SendMessageToCenterServer(CENTER_MESSAGE_LOGON_RESOURCE_CHANGE, &msgToLoader, sizeof(msgToLoader), userID);
	}
}

// 向中心服务器发送消息
bool CGameWorkManage::SendMessageToCenterServer(UINT msgID, void* pData, UINT size, int userID/* = 0*/, UINT mainID/* = 0*/, UINT assistID/* = 0*/, UINT handleCode/* = 0*/)
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

void CGameWorkManage::InitRounteCheckEvent()
{
	// 设置检查redis连接定时器
	SetTimer(LOGON_TIMER_CHECK_REDIS_CONNECTION, CHECK_REDIS_CONNECTION_SECS * 1000);

	// 设置存储redis数据到DB定时器
	SetTimer(LOGON_TIMER_ROUTINE_SAVE_REDIS, CHECK_REDIS_SAVE_DB * 1000);

	// 设置通用定时器
	SetTimer(LOGON_TIMER_NORMAL, NORMAL_TIMER_SECS * 1000);
}

void CGameWorkManage::CheckRedisConnection()
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

void CGameWorkManage::RountineSaveRedisDataToDB(bool updateAll)
{
	if (m_pRedis)
	{
		m_pRedis->RountineSaveRedisDataToDB(updateAll);
	}
}

void CGameWorkManage::OnNormalTimer()
{
	if (!m_pRedis)
	{
		ERROR_LOG("### redis 指针为空 ###");
		return;
	}

	ExecuteSqlSecond();

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
bool CGameWorkManage::IsUserOnline(int userID)
{
	return m_pRedis->IsUserOnline(userID);
}

void CGameWorkManage::OnServerCrossDay()
{
	AUTOCOSTONCE("OnServerCrossDay");
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
	if (IsMainDistributedSystem())
	{
		m_pRedisPHP->ClearAllEmailInfo();

		m_pRedisPHP->ClearAllFGNotifyInfo();

		ClearDataBase();

		CleanAllUserMoneyJewelsRank();
	}

	//生成新表，记录账单数据
	if (IsMainDistributedSystem())
	{
		AutoCreateNewTable(false);
	}

	/*int currTime = (int)time(NULL);
	const auto& onlineUserMap = m_pUserManage->GetLogonUserInfoMap();
	for (auto iter = onlineUserMap.begin(); iter != onlineUserMap.end(); ++iter)
	{
		int userID = iter->first;
		OnUserCrossDay(userID, currTime);
	}*/

	// 生成内存分析
#ifdef JEMALLOC_PROFILE_MEMORY
	mallctl("prof.dump", NULL, NULL, NULL, 0);
#endif // JEMALLOC_PROFILE_MEMORY
}

// 凌晨12点
void CGameWorkManage::OnServerCrossDay12Hour()
{
	AUTOCOSTONCE("OnServerCrossDay12Hour");

	if (m_pRedis)
	{
		m_pRedis->CountOneDayPoolInfo();
	}
}

void CGameWorkManage::OnServerCrossWeek()
{
	AUTOCOSTONCE("OnServerCrossWeek");
	INFO_LOG("OnServerCrossWeek");

	//分布式处理
	if (IsMainDistributedSystem())
	{
		//清理胜局数
		CleanAllUserWinCount();
	}

	//清理过期文件，考虑到不同的登陆服，会部署到不同的主机，每个主机都需要执行
	DeleteExpireFile();
}

void CGameWorkManage::CleanAllUserWinCount()
{
	if (m_pRedis)
	{
		m_pRedis->ClearAllUserWinCount();
	}
}

// 清理金币钻石排行榜
void CGameWorkManage::CleanAllUserMoneyJewelsRank()
{
	m_pRedis->ClearAllUserRanking(TBL_RANKING_MONEY);

	m_pRedis->ClearAllUserRanking(TBL_RANKING_JEWELS);
}

void CGameWorkManage::OnUserCrossDay(int userID, int time)
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
void CGameWorkManage::ClearDataBase()
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
void CGameWorkManage::AutoCreateNewTable(bool start)
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
bool CGameWorkManage::IsRoomIDServerExists(int roomID)
{
	return m_pRedis->GetRoomServerStatus(roomID);
}

// 自动开房
bool CGameWorkManage::AutoCreateRoom(const SaveRedisFriendsGroupDesk& deskInfo)
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

// 删除程序生成的无效文件
void CGameWorkManage::DeleteExpireFile()
{
	char delJsonPath[128] = "";

	sprintf(delJsonPath, "%s%s", SAVE_JSON_PATH, "deletejson.sh");
	system(delJsonPath);
}

std::string CGameWorkManage::GetRandHeadURLBySex(BYTE sex)
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

bool CGameWorkManage::IsMainDistributedSystem()
{
	if (m_uGroupIndex == m_uMainGroupIndex)
	{
		return true;
	}

	return false;
}

bool CGameWorkManage::IsDistributedSystemCalculate(long long calcID)
{
	if (m_uGroupCount <= 0)
	{
		ERROR_LOG("########### 重大错误m_uGroupCount=%d##############", m_uGroupCount);
		return false;
	}

	if (calcID % m_uGroupCount == m_uGroupIndex)
	{
		return true;
	}

	return false;
}

////////////////////////////////处理中心服消息//////////////////////////////////////////
bool CGameWorkManage::OnCenterServerMessage(UINT msgID, NetMessageHead* pNetHead, void* pData, UINT size, int userID)
{
	// 性能统计
	AUTOCOST("OnCenterServerMessage cost assistID: msgID=%u", msgID);

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
	case PLATFORM_MESSAGE_CLOSE_SERVER:
	{
		return OnCenterCloseServerMessage(pData, size);
	}
	case PLATFORM_MESSAGE_OPEN_SERVER:
	{
		return OnCenterOpenServerMessage(pData, size);
	}
	case CENTER_MESSAGE_COMMON_AUTO_CREATEROOM:
	{
		return OnCenterAutoCreateRoomMessage(pData, size);
	}
	default:
		break;
	}
	return false;
}

// 服务器id重复处理
bool CGameWorkManage::OnCenterRepeatIDMessage(void* pData, int size)
{
	if (size != 0)
	{
		return false;
	}

	CON_ERROR_LOG("################ 服务器唯一的workID重复(workID = %d)，启动失败，请重新配置workID ！！！ ####################", ConfigManage()->GetWorkServerConfig().workID);

	exit(0);

	return true;
}

// 分布式系统信息
bool CGameWorkManage::OnCenterDistributedSystemInfoMessage(void* pData, int size)
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

	if (pMessage->logonGroupCount <= 0)
	{
		ERROR_LOG("中心服发送数据错误 pMessage->logonGroupCount%d", pMessage->logonGroupCount);
		return false;
	}

	// 设置本系统在集群的位置信息
	m_uGroupCount = pMessage->logonGroupCount;
	m_uGroupIndex = pMessage->logonGroupIndex;
	m_uMainGroupIndex = pMessage->mainLogonGroupIndex;

	INFO_LOG("集群信息：Count = %d , Index = %d , MainIndex = %d", m_uGroupCount, m_uGroupIndex, m_uMainGroupIndex);

	static bool bStart = false;
	// 启动生成分表
	if (m_uGroupIndex == 0 && !bStart)
	{
		bStart = true;
		AutoCreateNewTable(true);
	}

	return true;
}

// 关服处理
bool CGameWorkManage::OnCenterCloseServerMessage(void* pData, int size)
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

	//// 统计在线时长，以及发送关服通知
	//const auto& onlineUserMap = m_pUserManage->GetLogonUserInfoMap();
	//for (auto iter = onlineUserMap.begin(); iter != onlineUserMap.end(); ++iter)
	//{
	//	int userID = iter->first;
	//	if (!iter->second->isVirtual)
	//	{
	//		m_pRedis->AddUserResNums(userID, "allOnlineToTime", currTime - iter->second->logonTime);
	//		m_pRedis->SetUserInfo(userID, " IsOnline 0 ");

	//		//发送消息通知下线
	//		SendData(socketIdx, NULL, 0, MSG_MAIN_LOGON_NOTIFY, MSG_NTF_LOGON_CLOSE_SERVER, 0);
	//	}
	//}

	//保存所有数据到db
	RountineSaveRedisDataToDB(true);

	INFO_LOG("==========LogonServer 关闭成功===========");

	return true;
}

// 开服处理
bool CGameWorkManage::OnCenterOpenServerMessage(void* pData, int size)
{
	if (!m_pRedis)
	{
		return false;
	}

	AUTOCOSTONCE("开服耗时，数据恢复");

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

// 中心服自动开房
bool CGameWorkManage::OnCenterAutoCreateRoomMessage(void* pData, UINT size)
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


//////////////////////////////////////////////////////////////////////////
void CGameWorkManage::SendHTTPMessage(int userID, const std::string& url, BYTE postType)
{
	// HTTP请求
	AsyncEventMsgHTTP asyncEvent;
	strcpy(asyncEvent.url, url.c_str());

	m_SQLDataManage.PushLine(&asyncEvent.dataLineHead, sizeof(AsyncEventMsgHTTP), ASYNC_EVENT_HTTP, userID, postType);
}

//////////////////////////////////////////////////////////////////////////
void CGameWorkManage::ExecuteSqlSecond()
{
	// 每秒执行一次，每次取MAX_SAVE_DATA_COUNT这么多数据去保存
	const int MAX_SAVE_DATA_COUNT = 64;
	int iSaveCount = 0;
	AsyncEventMsgSqlStatement msg;

	while (true)
	{
		redisReply* pReply = (redisReply*)redisCommand(m_pRedis->GetContext(), "LPOP %s", TBL_SAVE_DATA_SECOND);
		if (pReply == NULL)
		{
			break;
		}

		if (pReply->type == REDIS_REPLY_NIL)
		{
			freeReplyObject(pReply);
			break;
		}

		const char* sql = pReply->str;
		if (!sql)
		{
			freeReplyObject(pReply);
			continue;
		}

		size_t len = strlen(sql);
		if (len >= MAX_SQL_STATEMENT_SIZE)
		{
			ERROR_LOG("sql太长：%s", sql);
			freeReplyObject(pReply);
			continue;
		}

		strcpy(msg.sql, sql);

		if (!m_SQLDataManage.PushLine(&msg.dataLineHead, sizeof(msg) - sizeof(msg.sql) + len, ASYNC_EVENT_SQL_STATEMENT, DB_TYPE_LOG, 0))
		{
			ERROR_LOG("投递队列失败：%s", sql);
		}

		iSaveCount++;

		freeReplyObject(pReply);

		if (iSaveCount >= MAX_SAVE_DATA_COUNT) //每次最多处理数据量
		{
			break;
		}
	}
}

//////////////////////////////////lua相关////////////////////////////////////////
bool CGameWorkManage::InitLua()
{
	m_pLuaState = luaL_newstate();
	if (!m_pLuaState)
	{
		CON_ERROR_LOG("luaL_newstate failed!\n");
		return false;
	}

	luaopen_base(m_pLuaState);

	// 打开使用pb库
	luaopen_pb(m_pLuaState);

	//// 打开lua使用mysql的库
	//luaopen_luasql_mysql(m_pLuaState);

	//// 打开lua使用redis的库
	//luaopen_socket_core(m_pLuaState);

	// 初始化lua内置所有库
	luaL_openlibs(m_pLuaState);

	return true;
}

void CGameWorkManage::CFuncRegister()
{
	lua_register(m_pLuaState, "c_rediscmd", l_redis);



}

bool CGameWorkManage::LoadAllLuaFile()
{
	if (luaL_dofile(m_pLuaState, "./WorkServerTest.lua") != 0)
	{
		CON_ERROR_LOG("%s\n", lua_tostring(m_pLuaState, -1));
		return false;
	}

	if (luaL_dofile(m_pLuaState, "./ProtobufTest.lua") != 0)
	{
		CON_ERROR_LOG("%s\n", lua_tostring(m_pLuaState, -1));
		return false;
	}

	/*if (luaL_dofile(m_pLuaState, "./executor.lua") != 0)
	{
		CON_ERROR_LOG("%s\n", lua_tostring(m_pLuaState, -1));
		return false;
	}*/

	///////////////////////////////
	lua_getglobal(m_pLuaState, "add");
	lua_pushinteger(m_pLuaState, 102);
	lua_pushinteger(m_pLuaState, 100);

	int val = lua_pcall(m_pLuaState, 2, 1, 0);
	if (val)
	{
		std::cout << "LUA_ERROR " << lua_tostring(m_pLuaState, -1) << std::endl;
		lua_pop(m_pLuaState, 1);
	}

	//printf("%s\n", lua_tostring(m_pLuaState, -1));


	/////////////////////////////////////////////////
	lua_getglobal(m_pLuaState, "luotan");
	lua_pushinteger(m_pLuaState, 123456);
	if (lua_pcall(m_pLuaState, 1, 1, 0))
	{
		std::cout << "LUA_ERROR " << lua_tostring(m_pLuaState, -1) << std::endl;
		lua_pop(m_pLuaState, 1);
	}

	//printf("栈顶：%d\n", lua_gettop(m_pLuaState));

	lua_settop(m_pLuaState, 0);

	INFO_LOG("load all lua file success.");

	return true;
}


//////////////////////////////////lua全局静态函数////////////////////////////////////////
int CGameWorkManage::l_redis(lua_State* l)
{
	RoomBaseInfo room;
	WorkServerModule()->m_pWorkManage->m_pRedis->GetRoomBaseInfo(1, room);

	const char* cmd = lua_tostring(l, -1);
	char buf[10] = "luotan";


	lua_pushstring(l, buf);

	return 1;
}