#include "ServerManage.h"

//全局变量
bool CServerGameDesk::gOutMessageSize = true;
/*------------------------------------------------------------------------------*/
/**
* 构造函数
*/
CServerGameDesk::CServerGameDesk(void) :CGameDesk(FULL_BEGIN)
{
	m_byMJCardType = MJTABLE_CARDTYPE0;
	SetGameStation(GS_STATUS_FREE);
	ReSetGameState(0);
	m_GameData.m_byNtStation = 255;
	memset(m_GameData.m_iAddScore, 0, sizeof(m_GameData.m_iAddScore)); //每一把的加分
	m_GameData.m_tGameRuler.Init();
	m_GameData.m_iZhuangWinCount = 0;//庄家连赢次数
	memset(m_bSuperFlag, false, sizeof(m_bSuperFlag));
}
/*------------------------------------------------------------------------------*/
/**
* 析构函数
*/
CServerGameDesk::~CServerGameDesk(void) {}

/*-------------------------------------------------------------------------------------------------*/
/**
* 初始化桌子
* @return 返回布尔类型
*/
bool CServerGameDesk::InitDeskGameStation()
{
	LoadDynamicConfig();
	return true;
}

void CServerGameDesk::LoadDynamicConfig()
{
	//加载房间设置
	char szfile[256] = "";
	sprintf(szfile, "%s%d_s.ini", CINIFile::GetAppPath().c_str(), NAME_ID);
	CINIFile f(szfile);

	string sKey;
	sKey = "game";

	m_GameData.m_mjRule.byBlockTime = f.GetKeyVal(sKey, "BlockTime", 15);
	m_GameData.m_mjRule.byOutTime = f.GetKeyVal(sKey, "OutTime", 15);
	m_GameData.m_mjRule.byAutoOutTime = f.GetKeyVal(sKey, "AutoOutTime", 1);

	//读金币场配置文件
	if (IsGoldRoom() && m_pDataManage)
	{
		//根据不同场不同玩法
		char buf[128] = "";
		sprintf(buf, "%d_%d", NAME_ID, m_pDataManage->m_InitData.uRoomID);
		sKey = buf;
		m_GameData.m_tGameRuler.bDaHu = f.GetKeyVal(sKey, "bDaHu", m_GameData.m_tGameRuler.bDaHu) == 0 ? false : true;
		m_GameData.m_tGameRuler.bBaoTing = f.GetKeyVal(sKey, "bBaoTing", m_GameData.m_tGameRuler.bBaoTing) == 0 ? false : true;
		m_GameData.m_tGameRuler.iZiMoScore = f.GetKeyVal(sKey, "iZiMoScore", m_GameData.m_tGameRuler.iZiMoScore);
		m_GameData.m_tGameRuler.bZhiZiMo = f.GetKeyVal(sKey, "bZhiZiMo", m_GameData.m_tGameRuler.bZhiZiMo) == 0 ? false : true;
		m_GameData.m_tGameRuler.bFenPai = f.GetKeyVal(sKey, "bFenPai", m_GameData.m_tGameRuler.bFenPai) == 0 ? false : true;
		SetSpecialRule();
	}
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 游戏开始
* @param [in] bBeginFlasg 开始模式标志
*/
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (CGameDesk::GameBegin(bBeginFlag) == false)
	{
		GameFinish(0, GF_SALE);
		return false;
	}
	//if (IsGoldRoom()) //金币场全是机器人不开始游戏
	//{
	//	int iRobotCount = 0;
	//	for (int i = 0;i < PLAY_COUNT;i++)
	//	{
	//		GameUserInfo user;
	//		if (GetUserData(i, user) && user.isVirtual)
	//		{
	//			iRobotCount++;
	//		}
	//	}
	//	if (iRobotCount >= PLAY_COUNT)
	//	{
	//		GameFinish(0, GF_SALE);
	//		return false;
	//	}
	//}
	m_GameData.InitData();
	m_CPGData.InitData();
	m_CheckHuData.InitData();
	m_iPlayingCount++;
	memset(m_bNetCut, false, sizeof(m_bNetCut));
	memset(m_bTrustee, false, sizeof(m_bTrustee));
	memset(m_iOutCardChaoShiCount, 0, sizeof(m_iOutCardChaoShiCount));

	//获取奖池数据
	m_pDataManage->LoadPoolData();

	//////////////////////////////////////定庄和摇筛子确定棋牌位置
	//混乱骰子
	CLogicBase::GetSeziData(&m_GameData.T_TwoSeziDirAndGetPai.bySezi0, &m_GameData.T_TwoSeziDirAndGetPai.bySezi1);
	if (!IsGoldRoom())
	{
		if (255 == m_GameData.m_byNtStation)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (m_masterID == GetUserIDByDeskStation(i))
				{
					m_GameData.m_byNtStation = i;
					break;
				}
			}
		}
	}
	//如果没有房主，随机定庄
	srand((unsigned)time(NULL));
	if (255 == m_GameData.m_byNtStation)
	{
		m_GameData.m_byNtStation = rand() % PLAY_COUNT;//定庄
	}
	//先清空数据
	//掷筛子玩家的位置 = 庄家的位置
	m_GameData.T_TwoSeziDirAndGetPai.byUser = m_GameData.m_byNtStation;
	//记录起牌的方向(1为自己 逆时针数)
	//1,5,9是开自己门前的牌，2,6,10是右手边，3,7,11是对家，4,8,12是左手边
	m_GameData.m_MenPai.byGetPaiDir = (PLAY_COUNT - (m_GameData.T_TwoSeziDirAndGetPai.bySezi0 + m_GameData.T_TwoSeziDirAndGetPai.bySezi1 - 1) % PLAY_COUNT
		+ m_GameData.T_TwoSeziDirAndGetPai.byUser) % PLAY_COUNT;
	m_GameData.m_MenPai.byGetPai = min(m_GameData.T_TwoSeziDirAndGetPai.bySezi0, m_GameData.T_TwoSeziDirAndGetPai.bySezi1);
	///////////////////////////////////////////////////////////////////////
	if (!m_GameData.DisPatchCard())
	{
		return false;
	}
	//庄家多抓一张牌
	BYTE byCard = 255;
	BYTE byPai0 = 255, byPai1 = 255;
	if (m_GameData.m_mjRule.bIsMatchMJ && HandCard())
	{
		byCard = m_GameData.m_byArHandPai[m_GameData.m_byNtStation][HAND_CARD_NUM - 1];
		if (m_GameData.m_mjRule.bHaveJing)
		{
			m_GameData.TurningString(byPai0, byPai1);
			m_GameData.m_StrJing.byJingGen = byPai0;
			m_GameData.m_StrJing.AddJing(byPai1);
		}
	}
	else
	{
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			for (int j = 0;j < HAND_CARD_NUM - 1;j++)
			{
				byCard = m_GameData.GetPai(i, true, NULL);
				if (m_GameData.m_mjRule.bHaveFlower && m_GameData.CheckIsHuaPai(byCard))
				{
					m_GameData.m_vHuapaiTemp[i].push_back(byCard);
				}
			}
		}
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			m_GameData.SortHandPai(i, false);
		}
		byCard = m_GameData.GetPai(m_GameData.m_byNtStation, m_GameData.m_bZhuaPaiDir, NULL);//再庄家抓牌
		if (m_GameData.m_mjRule.bHaveFlower && m_GameData.CheckIsHuaPai(byCard))
		{
			m_GameData.m_vHuapaiTemp[m_GameData.m_byNtStation].push_back(byCard);
		}
		if (m_GameData.m_mjRule.bHaveJing)
		{
			m_GameData.TurningString(byPai0, byPai1);
			m_GameData.m_StrJing.byJingGen = byPai0;
			m_GameData.m_StrJing.AddJing(byPai1);
		}
	}
	m_GameData.T_ZhuaPai.byPs = byCard;//抓牌的值
	if (MJTABLE_CARDTYPE0 == m_byMJCardType)
	{
		m_byMJCardType = MJTABLE_CARDTYPE1;
	}
	else
	{
		m_byMJCardType = MJTABLE_CARDTYPE0;
	}
	m_GameData.m_byNowOutStation = m_GameData.m_byNtStation;//设置出牌玩家
	//设置状态
	SetGameStation(GS_STATUS_PLAYING);

	//构造数据，发送给客户端游戏开始数据
	CMD_S_GameStart GameStart;
	GameStart.bySezi0 = m_GameData.T_TwoSeziDirAndGetPai.bySezi0;
	GameStart.bySezi1 = m_GameData.T_TwoSeziDirAndGetPai.bySezi1;
	GameStart.byGetPaiDir = m_GameData.m_MenPai.byGetPaiDir;
	GameStart.byGetPai = m_GameData.m_MenPai.byGetPai;
	GameStart.byNtStation = m_GameData.m_byNtStation;
	GameStart.iMenNums = m_GameData.m_MenPai.GetMenPaiNum();
	GameStart.iPlayingCount = m_iPlayingCount;
	GameStart.byMJCardType = m_byMJCardType;
	GameStart.byNtCard = byCard;
	memcpy(GameStart.bSuperFlag, m_bSuperFlag, sizeof(GameStart.bSuperFlag));
	/*GameStart.iGameBase=m_GameData.m_mjRule.iGameBase;*/
	memcpy(GameStart.byCardCount, m_GameData.m_byArHandPaiCount, sizeof(GameStart.byCardCount));
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
	if (!m_GameData.m_mjAction.bTing)
	{
		//提取庄家听牌数据
		memset(m_GameData.m_byOutCanHuCard, 255, sizeof(m_GameData.m_byOutCanHuCard));
		memset(m_GameData.m_byHuCard, 255, sizeof(m_GameData.m_byHuCard));
		memset(m_GameData.m_byCardRemainNum, 0, sizeof(m_GameData.m_byCardRemainNum));
		m_CheckHuData.OutCardExtractHu(&m_GameData, m_GameData.m_byNowOutStation, m_GameData.m_byOutCanHuCard, m_GameData.m_byHuCard, m_GameData.m_byCardRemainNum);
	}
#endif
	//发送数据
	for (BYTE i = 0;i < PLAY_COUNT;i++)
	{
		memset(GameStart.byCard, 255, sizeof(GameStart.byCard));
		if (m_bSuperFlag[i])
		{
			m_GameData.CopyHandPai(GameStart.byCard, i, true);
		}
		else
		{
			m_GameData.CopyHandPai(GameStart.byCard, i, false);
		}

		if (m_GameData.m_byNowOutStation == i)
		{
			memcpy(GameStart.byOutCanHuCard, m_GameData.m_byOutCanHuCard, sizeof(GameStart.byOutCanHuCard));
			memcpy(GameStart.byHuCard, m_GameData.m_byHuCard, sizeof(GameStart.byHuCard));
			memcpy(GameStart.byCardRemainNum, m_GameData.m_byCardRemainNum, sizeof(GameStart.byCardRemainNum));
		}
		else
		{
			memset(GameStart.byOutCanHuCard, 255, sizeof(GameStart.byOutCanHuCard));
			memset(GameStart.byHuCard, 255, sizeof(GameStart.byHuCard));
			memset(GameStart.byCardRemainNum, 0, sizeof(GameStart.byCardRemainNum));
		}

		SendGameData(i, &GameStart, sizeof(GameStart), MSG_MAIN_LOADER_GAME, S_C_GAME_START, 0);
		//SendWatchData(i,&GameStart,sizeof(GameStart), MSG_MAIN_LOADER_GAME, S_C_GAME_START, 0);
	}
	OnJson(&GameStart, 1);
	if (m_GameData.m_mjAction.bBuHua)
	{
		//庄家先补花
		BYTE byDeskStation = m_GameData.m_byNtStation;
		m_GameData.m_bIsBuHua[byDeskStation] = true;
		for (int j = 0; j < (int)m_GameData.m_vHuapaiTemp[byDeskStation].size();j++)
		{
			CMD_C_OutCard Data;
			memset(&Data, 0, sizeof(Data));
			Data.byOutCardData = m_GameData.m_vHuapaiTemp[byDeskStation][j];
			OnUserOutCard(byDeskStation, &Data);
		}
		m_GameData.m_bIsBuAllHua[byDeskStation] = true;
	}

	////检测动作
#ifdef DELAY_CHECK_ACTION
	m_byBeginTempCard = byCard;
	SetTimer(TIME_CHECK_ACTION, 3200);
#else
	bool bBlock = false;
	if (CheckAction(m_GameData.m_byNowOutStation, byCard, 0))
	{
		//bBlock=true;
		//m_GameData.m_byCurrentOperatorUser = m_GameData.m_byNowOutStation;
		//CMD_S_OperateNotify ActionData;
		//// 进入拦牌状态
		//int i = m_GameData.m_byNowOutStation;
		//ActionData.byUsr=i;
		//memcpy(ActionData.byGangData,m_GameData.T_CPGNotify[i].iGangData,sizeof(ActionData.byGangData));
		//ActionData.bGang=m_GameData.T_CPGNotify[i].bGang;
		//ActionData.bHu=m_GameData.T_CPGNotify[i].bHu;
		//SendGameData(i,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		////SendWatchData(i,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		bBlock = true;
		/*	m_GameData.T_TingPai.bTing=m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bTing;
		memcpy(m_GameData.T_TingPai.byTingCanOut,m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byTingCanOut,sizeof(m_GameData.T_TingPai.byTingCanOut));*/
		CMD_S_OperateNotify ActionData;
		m_GameData.m_byCurrentOperatorUser = m_GameData.m_byNowOutStation;
		// 进入拦牌状态
		ActionData.byUsr = m_GameData.m_byNowOutStation;
		memcpy(ActionData.byGangData, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].iGangData, sizeof(ActionData.byGangData));
		memcpy(ActionData.byTingCanOut, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byTingCanOut, sizeof(ActionData.byTingCanOut));
		ActionData.bGang = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bGang;
		ActionData.bHu = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bHu;
		ActionData.bTing = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bTing;
		SendGameData(m_GameData.m_byNowOutStation, &ActionData, sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		//SendWatchData(m_GameData.m_byNowOutStation,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
	}
	if (bBlock)
	{
		if (m_bTrustee[m_GameData.m_byNowOutStation])
		{
			SetTimer(TIME_TUOGUAN_ACTION, m_GameData.m_mjRule.byAutoOutTime * 1000);
		}
		else
		{
			if (IsGoldRoom())
			{
				SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
			}
		}
	}
	else
	{
		if (m_bTrustee[m_GameData.m_byNowOutStation])
		{
			SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
		}
		else
		{
			if (IsGoldRoom())
			{
				SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
}
		}
	}
#endif
	return true;
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 游戏结束
* @param [in] bDeskStation 玩家位置
* @param [in] bCloseFlag 游戏结束标志
* @return 返回布尔类型
*/
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
	{
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (GetUserIDByDeskStation(i) <= 0)
			{
				continue;
			}
			OnHandleUserRequestCancelAuto(i);
			//金币场，断线托管
			CMD_C_Trustee Data;
			Data.bTrustee = false;
			HandleNotifyMessage(i, C_S_TRUSTEE, &Data, sizeof(Data));
		}
		int roomType = GetRoomType();

		//////////////////////////////////////////////////////////////////////////计算总数
		//变量定义
		CMD_S_GameEnd GameEnd;
		int iGangZongFen[PLAY_COUNT];//杠总分
		int iHuZongFen[PLAY_COUNT];//胡总分
		memset(iGangZongFen, 0, sizeof(iGangZongFen));
		memset(iHuZongFen, 0, sizeof(iHuZongFen));
		memcpy(GameEnd.byHuType, m_GameData.T_CountFen.byHuType, sizeof(GameEnd.byHuType));
		memcpy(GameEnd.iFanCount, m_GameData.T_CountFen.iFanCount, sizeof(GameEnd.iFanCount));
		memcpy(GameEnd.bTing, m_GameData.m_bTing, sizeof(GameEnd.bTing));

		//杠分，非流局才算杠分
		if (!m_GameData.T_HuPai.bIsLiuJu)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				iGangZongFen[i] = m_GameData.m_iGangScore[i];
			}
			memcpy(GameEnd.countGangFen, m_GameData.m_countGangFen, sizeof(GameEnd.countGangFen));
		}

		//胡分
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			int iScore = 0;
			for (int b = 0;b < MAX_HU_FEN_KIND;b++)
			{
				iScore += m_GameData.T_CountFen.iHuFenType[i][b];
			}
			iHuZongFen[i] = iScore;
		}
		memcpy(GameEnd.countHuFen, m_GameData.m_countHuFen, sizeof(GameEnd.countHuFen));

		//总分
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			m_GameData.T_CountFen.iZongFen[i] = iHuZongFen[i] + iGangZongFen[i];
		}

		GameEnd.bZimo = m_GameData.T_HuPai.bZimo;				//是否自摸
		GameEnd.bIsLiuJu = m_GameData.T_HuPai.bIsLiuJu;		//是否流局
		GameEnd.byFangPao = m_GameData.T_HuPai.byFangPao;	//放炮玩家位置
		GameEnd.byPs = m_GameData.T_HuPai.byPs;				//胡的牌
		memcpy(GameEnd.bIsHu, m_GameData.T_CountFen.bIsHu, sizeof(GameEnd.bIsHu)); //各个胡牌

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			//输赢倍数
			GameEnd.iWinFailDouble[i] = m_GameData.T_CountFen.iZongFen[i];
			//总分
			GameEnd.llGameScore[i] = m_GameData.T_CountFen.iZongFen[i] * GetBasePoint();
		}

		if (255 != m_GameData.T_HuPai.byFangPao)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (!m_GameData.T_CountFen.bIsHu[i])
				{
					continue;
				}
				m_GameData.AddToHandPai(i, m_GameData.T_HuPai.byPs);
			}
		}

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			//m_GameData.SortHandPai(i,false);
			GameEnd.byArHandPaiCount[i] = m_GameData.CopyOneUserHandPai(GameEnd.byArHandPai[i], i);
			TCPGStruct UserGCPData[5];
			m_GameData.CopyGCPData(i, UserGCPData);
			memcpy(GameEnd.UserGCPData[i], UserGCPData, sizeof(GameEnd.UserGCPData[i]));
		}
		//获得每个玩家的出牌数据和门牌数据
		/*memcpy(GameEnd.byArOutPai,m_GameData.m_byArOutPai,sizeof(GameEnd.byArOutPai));
		memcpy(GameEnd.byArOutPaiCount,m_GameData.m_byArOutPaiCount,sizeof(GameEnd.byArOutPaiCount));*/
		GameEnd.byRemainMenPaiCount = m_GameData.GetRemainMenPai(GameEnd.byRemainMenPai);

		long long   llReturnMoney[PLAY_COUNT];     //返回金币	
		bool      temp_cut[PLAY_COUNT];
		memset(llReturnMoney, 0, sizeof(llReturnMoney));
		memset(temp_cut, 0, sizeof(temp_cut));

		//系统算分
		if (roomType != ROOM_TYPE_GOLD)
		{
			char chfileName[100];
			memset(chfileName, 0, sizeof(chfileName));
			char szTempCode[20];
			memset(szTempCode, 0, sizeof(szTempCode));
			GetVideoCode(szTempCode, 20);
			//保存战绩回放
			sprintf(chfileName, "%s%s.json", SAVE_JSON_PATH, szTempCode);
			string fileName = chfileName;
			ofstream fVideoFile;                       //生成json文件用
			fVideoFile.open(fileName, ios::out);
			m_root["event"] = m_element;
			fVideoFile << m_root.toStyledString() << endl;
			fVideoFile.close();
			if (roomType == ROOM_TYPE_FRIEND)
			{
				ChangeUserPointGoldRoom(GameEnd.llGameScore, szTempCode, true);
			}
			else if (roomType == ROOM_TYPE_FG_VIP)
			{
				ChangeUserFireCoin(GameEnd.llGameScore, szTempCode, true);
			}
			else if (roomType == ROOM_TYPE_PRIVATE)
			{
				ChangeUserPointPrivate(GameEnd.llGameScore, temp_cut, szTempCode, true);
			}
		}
		else
		{
			//金币场结算
			ChangeUserPoint(GameEnd.llGameScore, temp_cut);
		}

		//发送总分结果
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			SendGameData(i, &GameEnd, sizeof(GameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
			//SendWatchData(i,&GameEnd,sizeof(GameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
		}

		////////////////统计战绩
		if (!IsGoldRoom())
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				/*BYTE an=0,ming=0,bu=0;
				m_GameData.GetUserGangNum(i,an,ming,bu);
				if(m_GameData.T_CountFen.bIsHu[i])
				{
				m_tZongResult.iHuCount[i]+=1;
				}
				m_tZongResult.iMingGangCount[i]+=ming+bu;
				m_tZongResult.iAnGangCount[i]+=an;*/
				m_tZongResult.llGameScore[i] += GameEnd.llGameScore[i];
			}
		}
		OnJson(&GameEnd, 5);

		//创建房间的为第一个庄，能连庄。后面谁赢谁做庄，荒局连庄
		if (!m_GameData.T_HuPai.bIsLiuJu)
		{
			if (m_GameData.T_CountFen.byUserNums > 1)
			{
				m_GameData.m_byNtStation = m_GameData.T_HuPai.byFangPao;
			}
			else
				m_GameData.m_byNtStation = m_GameData.T_HuPai.byHu;
		}
		else
		{
			m_GameData.m_byNtStation = m_GameData.m_byEndZhuaPaiPalyer;
		}
		bCloseFlag = GF_NORMAL;
		break;
	}
	case GF_SALE:
	case GFF_SAFE_FINISH:
	{
		/*CMD_S_GameEnd GameEnd;
		for(int i=0; i<PLAY_COUNT; i++)
		{
		if (m_bNetCut[i])
		{
		continue;
		}
		SendGameData(i,&GameEnd,sizeof(GameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
		//SendWatchData(i,&GameEnd,sizeof(GameEnd), MSG_MAIN_LOADER_GAME, S_C_GAME_END, 0);
		}*/
		bCloseFlag = GFF_SAFE_FINISH;
		break;
	}
	case GFF_FORCE_FINISH:
	{
		return true;
	}
	default:
	{
		break;
	}
	}
	// 重置数据
	SetGameStation(GS_STATUS_FREE);
	ReSetGameState(bCloseFlag);
	return CGameDesk::GameFinish(bDeskStation, bCloseFlag);
}
/*-----------------------------------------------------------------------------------*/
/**
* 判断是否正在游戏
* @param [in] bDeskStation 玩家位置
* @return 返回当前游戏状态
*/
bool	CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	return GS_STATUS_PLAYING == GetGameStation();
}

/*------------------------------------------------------------------------------*/
/**
* 获取游戏状态信息
* @param [in] bDeskStation 玩家位置
* @param [in] uSoketID Socket ID
* @param [in] bWatchUser 旁观
* @return 返回布尔类型
*/
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if (!IsGoldRoom())
	{
		GetSpecialRule();
	}
	m_bNetCut[bDeskStation] = false;
	m_bTrustee[bDeskStation] = false;
	if (bDeskStation >= 0 && bDeskStation < PLAY_COUNT)
	{
		GameUserInfo user;
		if (GetUserData(bDeskStation, user) && (user.userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER)
		{
			m_bSuperFlag[bDeskStation] = true;
		}
		else
		{
			m_bSuperFlag[bDeskStation] = false;
		}
	}
	switch (GetGameStation())
	{
	case GS_STATUS_FREE:	//空闲状态
	{
		//变量定义
		CMD_S_StatusFree StatusFree;
		//构造数据
		StatusFree.byBlockTime = m_GameData.m_mjRule.byBlockTime;
		StatusFree.byOutTime = m_GameData.m_mjRule.byOutTime;
		StatusFree.iPlayingCount = m_iPlayingCount;
		//memcpy(StatusFree.bAgree,m_bAgree,sizeof(StatusFree.bAgree));
		StatusFree.byMJCardType = m_byMJCardType;
		//StatusFree.iBaseFen=m_GameData.m_mjRule.iGameBase;
		StatusFree.bSuperFlag = m_bSuperFlag[bDeskStation]; //是否超端
		//游戏规则
		memcpy(&StatusFree.tGameRuler, &m_GameData.m_tGameRuler, sizeof(StatusFree.tGameRuler));
		//给自己发送数据
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &StatusFree, sizeof(StatusFree));
	}
	break;
	case GS_STATUS_PLAYING:	//游戏状态
	{
		//变量定义
		CMD_S_StatusPlay StatusPlay;
		StatusPlay.byBlockTime = m_GameData.m_mjRule.byBlockTime;
		StatusPlay.byOutTime = m_GameData.m_mjRule.byOutTime;
		memcpy(StatusPlay.bTuoGuan, m_bTrustee, sizeof(StatusPlay.bTuoGuan));
		memcpy(StatusPlay.bTing, m_GameData.m_bTing, sizeof(StatusPlay.bTing));
		if (m_bSuperFlag[bDeskStation])
		{
			m_GameData.CopyHandPai(StatusPlay.byArHandPai, bDeskStation, true);
		}
		else
		{
			m_GameData.CopyHandPai(StatusPlay.byArHandPai, bDeskStation, false);
		}
		memcpy(StatusPlay.byArHandPaiCount, m_GameData.m_byArHandPaiCount, sizeof(StatusPlay.byArHandPaiCount));
		memcpy(StatusPlay.byArOutPai, m_GameData.m_byArOutPai, sizeof(StatusPlay.byArOutPai));
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			StatusPlay.byArOutPaiCount[i] = m_GameData.GetOutPaiCount(i);
		}
		StatusPlay.bSuperFlag = m_bSuperFlag[bDeskStation]; //是否超端
		memcpy(StatusPlay.UserGCPData, m_GameData.m_UserGCPData, sizeof(StatusPlay.UserGCPData));
		StatusPlay.byNowOutStation = m_GameData.m_byNowOutStation;
		StatusPlay.iMenNums = m_GameData.m_MenPai.GetMenPaiNum();
		/*if (m_GameData.m_mjAction.bTing && m_GameData.m_bTing[bDeskStation])
		{
		memcpy(StatusPlay.byTingCanOut,m_GameData.T_TingPai.byTingCanOut,sizeof(StatusPlay.byTingCanOut));
		}*/
		//StatusPlay.bTinging=m_GameData.T_TingPai.bTing;
		memcpy(StatusPlay.byTingOutCardIndex, m_GameData.m_byTingOutCardIndex, sizeof(StatusPlay.byTingOutCardIndex));
		StatusPlay.bQiHu = m_GameData.m_bQiHu[bDeskStation];
		StatusPlay.iPlayingCount = m_iPlayingCount;
		StatusPlay.byMJCardType = m_byMJCardType;
		/*StatusPlay.byBaoGen=m_GameData.m_StrJing.byJingGen;
		StatusPlay.byBaoCard=m_GameData.m_StrJing.byPai[0];*/
		StatusPlay.byNtStation = m_GameData.m_byNtStation;
		//StatusPlay.iBaseFen=m_GameData.m_mjRule.iGameBase;
		//游戏规则
		memcpy(&StatusPlay.tGameRuler, &m_GameData.m_tGameRuler, sizeof(StatusPlay.tGameRuler));
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
		if (!m_GameData.m_mjAction.bTing && bDeskStation == m_GameData.m_byNowOutStation)
		{
			//胡牌数据
			memcpy(StatusPlay.byOutCanHuCard, m_GameData.m_byOutCanHuCard, sizeof(StatusPlay.byOutCanHuCard)); //出牌前用
			memcpy(StatusPlay.byHuCard, m_GameData.m_byHuCard, sizeof(StatusPlay.byHuCard));
			memcpy(StatusPlay.byCardRemainNum, m_GameData.m_byCardRemainNum, sizeof(StatusPlay.byCardRemainNum));
		}
#endif
#ifdef RULE_HAVE_HU_PAI_NOTIFY
		memcpy(StatusPlay.byOutAfterHuCard, m_GameData.m_byOutAfterHuCard[bDeskStation], sizeof(StatusPlay.byOutAfterHuCard));//出牌后用
#endif
			//给自己发送数据
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &StatusPlay, sizeof(StatusPlay));
		if (m_GameData.m_byCurrentOperatorUser == bDeskStation)
		{
			SendChiPengGangHuMsg(bDeskStation);
		}
	}
	break;
	}
	return true;
}

/*------------------------------------------------------------------------------*/
/**
* 定时器消息
* @param [in] uTimerID 定时器ID
* @return 返回布尔类型
*/
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	KillTimer(uTimerID);
	if (TIME_TUOGUAN == uTimerID && IsGoldRoom())
	{
		if (255 != m_GameData.m_byNowOutStation)
		{
			CMD_C_OutCard Data;
			Data.byOutCardData = AutoOutCard(m_GameData.m_byNowOutStation);
			//Data.byOutCardData = m_GameData.GetLastHandPai(m_GameData.m_byNowOutStation);
			//Data.iOutCardIndex=m_GameData.m_byArHandPaiCount[m_GameData.m_byNowOutStation]-1;
			OnUserOutCard(m_GameData.m_byNowOutStation, &Data);
		}
	}
	else if (TIME_TUOGUAN_ACTION == uTimerID && IsGoldRoom())
	{
		if (m_GameData.m_byCurrentOperatorUser != 255 &&
			m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bCanAction)
		{
			if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bHu)//替你胡牌就烧高香了，还要劳资干啥
			{
				CMD_C_OperateCard Data;
				Data.byOperateType = ACTION_HU;
				OnUserOperate(m_GameData.m_byCurrentOperatorUser, &Data);
			}
			else if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bTing)
			{
				CMD_C_OutCard Data;
				Data.bTing = true;
				Data.byOutCardData = m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].byTingCanOut[0];
				OnUserOutCard(m_GameData.m_byCurrentOperatorUser, &Data);
			}
			else if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bGang)
			{
				CMD_C_OperateCard Data;
				Data.byOperateType = ACTION_GANG;
				for (int i = 0; i < 4; i++)
				{
					Data.byActionData[i] = m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].iGangData[0][1];
				}
				OnUserOperate(m_GameData.m_byCurrentOperatorUser, &Data);
			}
			else if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bPeng &&
				AutoCanAction(m_GameData.m_byCurrentOperatorUser, ACTION_PENG))
			{
				CMD_C_OperateCard Data;
				Data.byOperateType = ACTION_PENG;
				OnUserOperate(m_GameData.m_byCurrentOperatorUser, &Data);
			}
			else//pass掉啦
			{
				CMD_C_OperateCard Data;
				Data.byOperateType = ACTION_PASS;
				OnUserOperate(m_GameData.m_byCurrentOperatorUser, &Data);
			}
		}
	}
	else if (TIME_ACTION == uTimerID && IsGoldRoom())
	{
		if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bCanAction)
		{
			if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bHu)//替你胡牌就烧高香了，还要劳资干啥
			{
				CMD_C_OperateCard Data;
				Data.byOperateType = ACTION_HU;
				OnUserOperate(m_GameData.m_byCurrentOperatorUser, &Data);
			}
			else if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bTing)
			{
				CMD_C_OutCard Data;
				Data.byOutCardData = AutoOutCard(m_GameData.m_byCurrentOperatorUser);
				OnUserOutCard(m_GameData.m_byCurrentOperatorUser, &Data);
			}
			/*else if (m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].bGang)
			{
			CMD_C_OperateCard Data;
			Data.byOperateType = ACTION_GANG;
			for(int i = 0 ; i < 4; i ++)
			{
			Data.byActionData[i] = m_GameData.T_CPGNotify[m_GameData.m_byCurrentOperatorUser].iGangData[0][1];
			}
			OnUserOperate(m_GameData.m_byCurrentOperatorUser,&Data);
			}*/
			else//pass掉啦
			{
				CMD_C_OperateCard Data;
				Data.byOperateType = ACTION_PASS;
				OnUserOperate(m_GameData.m_byCurrentOperatorUser, &Data);
			}
		}
		////金币场，动作选择超时超时托管
		//if(255!=m_GameData.m_byCurrentOperatorUser)
		//{
		//	m_bTrustee[m_GameData.m_byCurrentOperatorUser]=true;
		//	CMD_S_Trustee Trustee;
		//	Trustee.bTrustee=true;
		//	Trustee.byDeskStation = m_GameData.m_byCurrentOperatorUser;
		//	for(int i=0;i<PLAY_COUNT;i++)
		//	{
		//		SendGameData(i,&Trustee,sizeof(Trustee), MSG_MAIN_LOADER_GAME, S_C_TRUSTEE, 0);
		//		//SendWatchData(i,&Trustee,sizeof(Trustee), MSG_MAIN_LOADER_GAME, S_C_TRUSTEE, 0);
		//	}
		//}
	}
	else if (TIME_THINK == uTimerID  && IsGoldRoom())
	{
		if (255 != m_GameData.m_byNowOutStation)
		{
			CMD_C_OutCard Data;
			memset(&Data, 0, sizeof(Data));
			Data.byOutCardData = AutoOutCard(m_GameData.m_byNowOutStation);
			//Data.byOutCardData = m_GameData.GetLastHandPai(m_GameData.m_byNowOutStation);
			//Data.iOutCardIndex=m_GameData.m_byArHandPaiCount[m_GameData.m_byNowOutStation]-1;
			BYTE byStation = m_GameData.m_byNowOutStation;
			OnUserOutCard(m_GameData.m_byNowOutStation, &Data);
			m_iOutCardChaoShiCount[byStation] ++;
			//金币场，出牌超时托管
			GameUserInfo user;
			GetUserData(byStation, user);
			if (m_GameData.m_mjRule.bChaoShiTuoGuan && !m_bTrustee[byStation]
				&& m_iOutCardChaoShiCount[byStation] >= 1 && !user.isVirtual)
			{
				m_bTrustee[byStation] = true;
				CMD_S_Trustee Trustee;
				Trustee.bTrustee = true;
				Trustee.byDeskStation = byStation;
				for (int i = 0;i < PLAY_COUNT;i++)
				{
					SendGameData(i, &Trustee, sizeof(Trustee), MSG_MAIN_LOADER_GAME, S_C_TRUSTEE, 0);
					//SendWatchData(i,&Trustee,sizeof(Trustee), MSG_MAIN_LOADER_GAME, S_C_TRUSTEE, 0);
				}
			}
		}
	}
	else if (TIME_BUHUA == uTimerID)
	{
		if (255 != m_GameData.m_byNowOutStation)
		{
			for (int i = m_GameData.m_byArHandPaiCount[m_GameData.m_byNowOutStation] - 1; i >= 0; i--)
			{
				if (m_GameData.CheckIsHuaPai(m_GameData.m_byArHandPai[m_GameData.m_byNowOutStation][i]))
				{
					CMD_C_OutCard Data;
					memset(&Data, 0, sizeof(Data));
					Data.byOutCardData = m_GameData.m_byArHandPai[m_GameData.m_byNowOutStation][i];
					//Data.byOutCardIndex=i;
					OnUserOutCard(m_GameData.m_byNowOutStation, &Data);
					break;
				}
			}
		}
	}
	else if (TIME_TING_OUTCARD == uTimerID)
	{
		if (255 != m_GameData.m_byNowOutStation)
		{
			CMD_C_OutCard Data;
			Data.byOutCardData = m_GameData.GetLastHandPai(m_GameData.m_byNowOutStation);
			OnUserOutCard(m_GameData.m_byNowOutStation, &Data);
		}
	}
	else if (TIME_CHECK_ACTION == uTimerID)
	{
		bool bBlock = false;
		if (CheckAction(m_GameData.m_byNowOutStation, m_byBeginTempCard, 0))
		{
			bBlock = true;
			m_GameData.T_TingPai.bTing = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bTing;
			memcpy(m_GameData.T_TingPai.byTingCanOut, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byTingCanOut, sizeof(m_GameData.T_TingPai.byTingCanOut));
			CMD_S_OperateNotify ActionData;
			m_GameData.m_byCurrentOperatorUser = m_GameData.m_byNowOutStation;
			// 进入拦牌状态
			ActionData.byUsr = m_GameData.m_byNowOutStation;
			memcpy(ActionData.byGangData, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].iGangData, sizeof(ActionData.byGangData));
			ActionData.bGang = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bGang;
			ActionData.bHu = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bHu;
			if (ActionData.bHu)
			{
				ActionData.bTing = false;
			}
			else
			{
				ActionData.bTing = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bTing;
				memcpy(ActionData.byTingCanOut, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byTingCanOut, sizeof(ActionData.byTingCanOut));
				memcpy(ActionData.byHuCard, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byHuCard, sizeof(ActionData.byHuCard));
				memcpy(ActionData.byCardRemainNum, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byCardRemainNum, sizeof(ActionData.byCardRemainNum));
			}
			SendGameData(m_GameData.m_byNowOutStation, &ActionData, sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
			//SendWatchData(m_GameData.m_byNowOutStation,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		}
		if (bBlock)
		{
			if (m_bTrustee[m_GameData.m_byNowOutStation])
			{
				SetTimer(TIME_TUOGUAN_ACTION, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
				}
			}
		}
		else
		{
			if (m_bTrustee[m_GameData.m_byNowOutStation])
			{
				SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
				}
			}
		}
	}
	return CGameDesk::OnTimer(uTimerID);
}

//桌子成功解散
void CServerGameDesk::OnDeskSuccessfulDissmiss(bool isDismissMidway)
{
	if (m_iPlayingCount > 0)  //大于一局才发大结算
	{
		// 计算抽水值
		CGameDesk::SetDeskPercentageScore(m_tZongResult.llGameScore);

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			SendGameData(i, &m_tZongResult, sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, S_C_GAME_END_ALL, 0);
			//SendWatchData(i,&m_tZongResult,sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, S_C_GAME_END_ALL, 0);
		}
	}
	return CGameDesk::OnDeskSuccessfulDissmiss(isDismissMidway);
}

// 玩家请求托管
bool CServerGameDesk::OnHandleUserRequestAuto(BYTE deskStation)
{
	return CGameDesk::OnHandleUserRequestAuto(deskStation);
}

// 玩家取消托管
bool CServerGameDesk::OnHandleUserRequestCancelAuto(BYTE deskStation)
{
	return CGameDesk::OnHandleUserRequestCancelAuto(deskStation);
}

//初始化游戏数据，房卡场非正常结算的时候调用
void CServerGameDesk::InitDeskGameData()
{
	KillTimer(TIME_CHECK_ACTION);
	KillTimer(TIME_ACTION);
	KillTimer(TIME_DELAY_SEND_ACTION);
	KillTimer(TIME_THINK);
	KillTimer(TIME_TING_OUTCARD);
	KillTimer(TIME_TUOGUAN);
	KillTimer(TIME_TUOGUAN_ACTION);
	KillTimer(TIME_BUHUA);
	m_GameData.InitData();
	m_CPGData.InitData();
	m_CheckHuData.InitData();
	m_GameData.m_byNtStation = 255;
	SetGameStation(GS_STATUS_FREE);
	// 重置数据
	ReSetGameState(0);
	m_iPlayingCount = 0;
	m_GameData.m_iZhuangWinCount = 0;
	m_GameData.m_iHuangZhuangCount = 0;
	m_tZongResult.Init();						//总战绩
	m_GameData.m_tGameRuler.Init(); //初始化规则
	memset(m_GameData.m_iAddScore, 0, sizeof(m_GameData.m_iAddScore)); //每一把的加分
}

/**
* 重置游戏状态
* @param [in] bLastStation 游戏结束标志
* @return 返回布尔类型
*/
bool	CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	memset(m_bNetCut, false, sizeof(m_bNetCut));
	memset(m_bAgree, false, sizeof(m_bAgree));
	memset(m_bTrustee, false, sizeof(m_bTrustee));
	memset(m_ChangeCard, 255, sizeof(m_ChangeCard));
	m_iVideoIndex = 0;
	m_element.clear();
	m_root.clear();
	return true;
}

/*-------------------------------------------------------------------------------------------------*/
/**
* 游戏数据包处理函数
* @param [in] bDeskStation 玩家位置
* @param [in] pNetHead 消息头
* @param [in] pData 消息体
* @param [in] uSize 消息包的大小
* @param [in] uSoketID Socket ID
* @param [in] bWatchUser 旁观
* @return 返回布尔类型
*/
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser)
{
	switch (assistID)
	{
	case MSG_ASS_LOADER_GAME_AGREE:		//用户同意游戏
	{
		if (bWatchUser)
		{
			return true;
		}
		if (GS_STATUS_FREE != GetGameStation())
		{
			return true;
		}
		m_bAgree[bDeskStation] = true;
		return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
	}
	case C_S_OUT_CARD:		//出牌消息
	{
		//效验消息
		if (size != sizeof(CMD_C_OutCard)) return false;
		m_iOutCardChaoShiCount[bDeskStation] = 0;
		//消息处理
		CMD_C_OutCard * pOutCard = (CMD_C_OutCard *)pData;
		return OnUserOutCard(bDeskStation, pOutCard);
	}
	case C_S_OPERATE_CARD:	//操作消息
	{
		//效验消息
		if (size != sizeof(CMD_C_OperateCard)) return false;
		//消息处理
		CMD_C_OperateCard * pOperateCard = (CMD_C_OperateCard *)pData;
		return OnUserOperate(bDeskStation, pOperateCard);
	}
	case C_S_TRUSTEE: //托管消息
	{
		if (size != sizeof(CMD_C_Trustee)) return false;
		CMD_C_Trustee *pTrustee = (CMD_C_Trustee *)pData;
		if (!m_bTrustee[bDeskStation] && pTrustee->bTrustee
			&& m_GameData.m_byNowOutStation == bDeskStation)
		{
			SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
		}
		m_bTrustee[bDeskStation] = pTrustee->bTrustee;
		CMD_S_Trustee Trustee;
		Trustee.bTrustee = pTrustee->bTrustee;
		Trustee.byDeskStation = bDeskStation;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			SendGameData(i, &Trustee, sizeof(Trustee), MSG_MAIN_LOADER_GAME, S_C_TRUSTEE, 0);
			//SendWatchData(i,&Trustee,sizeof(Trustee), MSG_MAIN_LOADER_GAME, S_C_TRUSTEE, 0);
		}
		return true;
	}
	case C_S_SUPER_REQUEST_LOOK:		//超端看牌
	{
		if (GetGameStation() == GS_STATUS_PLAYING)
		{
			CMD_C_SuperLook *SuperLook = (CMD_C_SuperLook*)pData;
			SuperUserLookMenPai(bDeskStation, SuperLook);
		}
		break;
	}
	case C_S_SUPER_REQUEST_CRAD:		//超端要牌
	{
		if (GetGameStation() == GS_STATUS_PLAYING)
		{
			if (size != sizeof(CMD_C_Super_Request_Card))
				return false;
			CMD_C_Super_Request_Card *SuperChangeCard = (CMD_C_Super_Request_Card*)pData;
			if (SuperChangeCard == NULL || SuperChangeCard->CardDate == 255)
				return false;

			SuperUserChangePai(bDeskStation, SuperChangeCard);
		}
		break;
	}
	}
	return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
}

/*-----------------------------------------------------------------------------------*/
/**
* 用户离开游戏桌
* @return 返回用户离开状态
*/
bool CServerGameDesk::UserLeftDesk(GameUserInfo* pUser)
{
	if (m_GameData.m_byNtStation != 255)
	{
		m_GameData.m_byNtStation = 255;
	}
	if (m_GameData.m_iHuangZhuangCount != 0)
	{
		m_GameData.m_iHuangZhuangCount = 0;
	}
	m_bAgree[pUser->deskStation] = false;
	//m_bSuperFlag[pUser->deskStation]=false;
	return CGameDesk::UserLeftDesk(pUser);
}

/*-----------------------------------------------------------------------------------*/
//用户断线离开
bool	CServerGameDesk::UserNetCut(GameUserInfo *pUser)
{
	CGameDesk::UserNetCut(pUser);
	////金币场，断线托管
	//CMD_C_Trustee Data;
	//Data.bTrustee = true;
	//if (IsGoldRoom() && m_GameData.m_mjRule.bDuanXianTuoGuan)
	//{
	//	HandleNotifyMessage(pUser->deskStation, C_S_TRUSTEE, &Data, sizeof(Data), false);
	//}

	//记录断线玩家（在重连前部给该玩家发送消息）
	m_bNetCut[pUser->deskStation] = true;
	
	if (IsGoldRoom() && IsPlayGame(0) && IsAllUserOffline())
	{
		GameFinish(pUser->deskStation, GFF_SAFE_FINISH);
	}

	return true;
}

//用户出牌,出牌用户椅子号,牌面值
bool CServerGameDesk::OnUserOutCard(BYTE byDeskStation, CMD_C_OutCard * pOutCard)
{
	//效验状态
	if (GS_STATUS_PLAYING != GetGameStation())
	{
		SendOutCardError(byDeskStation, 1);
		return true;
	}
	if (!m_GameData.IsOutPaiPeople(byDeskStation))
	{
		SendOutCardError(byDeskStation, 2);
		return true;
	}
	if (!CMjRef::IsValidateMj(pOutCard->byOutCardData))
	{
		SendOutCardError(byDeskStation, 3);
		return true;
	}
	if (m_GameData.GetAHandPaiCount(byDeskStation, pOutCard->byOutCardData) <= 0)
	{
		SendOutCardError(byDeskStation, 4);
		return true;
	}
	if (pOutCard->bTing && m_GameData.m_bTing[byDeskStation])
	{
		SendOutCardError(byDeskStation, 5);
		return true;
	}
	if (m_GameData.T_CPGNotify[byDeskStation].bCanAction && !m_GameData.T_CPGNotify[byDeskStation].bTing)
	{
		SendOutCardError(byDeskStation, 6);
		return true;
	}
	KillTimer(TIME_CHECK_ACTION);
	KillTimer(TIME_ACTION);
	KillTimer(TIME_THINK);
	KillTimer(TIME_TING_OUTCARD);
	m_GameData.T_TingPai.Init();
	m_GameData.T_GangPai.Init();
	//m_GameData.m_tActionTurn[byDeskStation].Init();
	m_GameData.m_bHaveOut[byDeskStation] = true;
	m_GameData.m_byNowOutStation = 255;
	m_GameData.T_OutPai.byUser = byDeskStation;
	m_GameData.T_OutPai.byPs = pOutCard->byOutCardData;
	//删除一张手牌
	BYTE byOutIndex = m_GameData.DeleteAHandPai(byDeskStation, pOutCard->byOutCardData);

#ifdef RULE_HAVE_HU_PAI_NOTIFY //提取玩家可以胡的牌
	BYTE byHuCard[MAX_MJ_KIND_NUM];
	memset(byHuCard, 255, sizeof(byHuCard));
	m_CheckHuData.ExtractHu(&m_GameData, byDeskStation, byHuCard);
	memcpy(m_GameData.m_byOutAfterHuCard[byDeskStation], byHuCard, sizeof(m_GameData.m_byOutAfterHuCard[byDeskStation]));
#endif
	//构造数据
	CMD_S_OutCard OutCard;
	OutCard.byOutCardUser = byDeskStation;
	OutCard.byOutCardData = pOutCard->byOutCardData;
	OutCard.byOutCardDataIndex = byOutIndex;
#ifdef RULE_HAVE_HU_PAI_NOTIFY
	memcpy(OutCard.byHuCard, byHuCard, sizeof(OutCard.byHuCard));
#endif

	if (m_GameData.m_mjAction.bTing && pOutCard->bTing && m_GameData.T_CPGNotify[byDeskStation].bTing)
	{
		m_GameData.T_CPGNotify[byDeskStation].Init();
		m_GameData.m_bTing[byDeskStation] = true;
		OutCard.bTing = pOutCard->bTing;
	}
	//添加一张出牌
	m_GameData.AddToOutPai(byDeskStation, pOutCard->byOutCardData, OutCard.bTing);

	//出完牌，重置弃胡状态
	m_GameData.m_bQiHu[byDeskStation] = false;
	m_GameData.m_bQiPeng[byDeskStation] = false;
	m_GameData.m_vecQiPengCard[byDeskStation].clear();

	//花牌
	if (m_GameData.m_mjRule.bHaveFlower && m_GameData.CheckIsHuaPai(OutCard.byOutCardData))
	{
		m_GameData.AddToHuaPai(byDeskStation, OutCard.byOutCardData);
	}
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		memset(OutCard.byCardData, 255, sizeof(OutCard.byCardData));
		if (m_bSuperFlag[i])
		{
			m_GameData.SortHandPai(byDeskStation, false);
			OutCard.byCardCount = m_GameData.CopyOneUserHandPai(OutCard.byCardData, byDeskStation);
		}
		else
		{
			if (byDeskStation == i)
			{
				m_GameData.SortHandPai(byDeskStation, false);
				OutCard.byCardCount = m_GameData.CopyOneUserHandPai(OutCard.byCardData, byDeskStation);
			}
			else
			{
				OutCard.byCardCount = m_GameData.GetHandPaiCount(byDeskStation);
			}
		}
		if (byDeskStation == i)
		{
			OnJson(&OutCard, 2);
		}
		SendGameData(i, &OutCard, sizeof(OutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD, 0);
		//SendWatchData(i,&OutCard,sizeof(OutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD, 0);
	}

	//花牌
	if (m_GameData.m_mjAction.bBuHua && m_GameData.CheckIsHuaPai(OutCard.byOutCardData))
	{
		//byDeskStation = (byDeskStation + 1)%PLAY_COUNT; 
		//ZhuaPai(byDeskStation,true);//牌头补花
		ZhuaPai(byDeskStation, false);//牌尾补花 
		return true;
	}

	//检测动作，听牌倒扣，别人无法吃碰杠胡
	if (OutCard.bTing || !CheckAction(byDeskStation, pOutCard->byOutCardData, 2))
	{
		//没有动作就直接下一个玩家抓牌
		ZhuaPai(byDeskStation, true);
	}
	else
	{
		//发送了拦牌给客户端玩家 
		if (!HaveUserCanDoAction(byDeskStation))
		{
			//么有发送了拦牌给客户端玩家 就启动抓牌事件
			//没有动作就直接下一个玩家抓牌
			ZhuaPai(byDeskStation, true);
		}
	}
	return true;
}

//抓牌
void CServerGameDesk::ZhuaPai(BYTE byCurrentStation, bool bHead)
{
	if (m_GameData.m_MenPai.GetMenPaiNum() <= m_GameData.m_mjRule.byGamePassNum)//流局
	{
		GameFinish(byCurrentStation, GF_NORMAL);
	}
	else
	{
		if (bHead)
		{
			m_GameData.m_byNowOutStation = m_GameData.GetNextStation(byCurrentStation, false);//抓牌位置
		}
		else
		{
			m_GameData.m_byNowOutStation = byCurrentStation;//抓牌位置
		}
		m_GameData.m_bQiHu[m_GameData.m_byNowOutStation] = false;
		m_GameData.m_bQiPeng[m_GameData.m_byNowOutStation] = false;
		m_GameData.m_vecQiPengCard[m_GameData.m_byNowOutStation].clear();

		m_GameData.m_byCurrentOperatorUser = 255;
		BYTE byPaiIndex = 255;
		BYTE byCard = 255;
		//超端玩家换牌
		if (m_bSuperFlag[m_GameData.m_byNowOutStation] && m_ChangeCard[m_GameData.m_byNowOutStation] != 255)
		{
			byCard = m_GameData.GetPai(m_GameData.m_byNowOutStation, bHead, &byPaiIndex, m_ChangeCard[m_GameData.m_byNowOutStation]);//超端抓牌
			m_ChangeCard[m_GameData.m_byNowOutStation] = 255;
		}
		else if (IsVirtual(m_GameData.m_byNowOutStation) && CUtil::GetRandNum() % 1000 < abs(m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent))
		{
			BYTE byTempCard = 255;
			for (int i = 0; i < 34; i++)
			{
				byTempCard = m_GameData.m_byOutAfterHuCard[m_GameData.m_byNowOutStation][i];
				if (byTempCard == 255)
				{
					break;
				}

				if (m_GameData.m_MenPai.GetOnePaiNum(byTempCard) <= 0)
				{
					continue;
				}

				byCard = m_GameData.GetPai(m_GameData.m_byNowOutStation, bHead, &byPaiIndex, byTempCard);//超端抓牌
				break;
			}

			if (byCard == 255)
			{
				byCard = m_GameData.GetPai(m_GameData.m_byNowOutStation, bHead, &byPaiIndex);//正常抓牌
			}
		}
		else
		{
			byCard = m_GameData.GetPai(m_GameData.m_byNowOutStation, bHead, &byPaiIndex);//正常抓牌
		}
		m_GameData.T_ZhuaPai.byPs = byCard;		//抓牌的值
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
		if (!m_GameData.m_mjAction.bTing)
		{
			//提取庄家听牌数据
			memset(m_GameData.m_byOutCanHuCard, 255, sizeof(m_GameData.m_byOutCanHuCard));
			memset(m_GameData.m_byHuCard, 255, sizeof(m_GameData.m_byHuCard));
			memset(m_GameData.m_byCardRemainNum, 0, sizeof(m_GameData.m_byCardRemainNum));
			m_CheckHuData.OutCardExtractHu(&m_GameData, m_GameData.m_byNowOutStation, m_GameData.m_byOutCanHuCard, m_GameData.m_byHuCard, m_GameData.m_byCardRemainNum);
		}
#endif
		//构造数据
		CMD_S_SendCard SendCard;
		SendCard.iMenNums = m_GameData.m_MenPai.GetMenPaiNum();
		if (SendCard.iMenNums <= m_GameData.m_mjRule.byGamePassNum)
		{
			m_GameData.m_byEndZhuaPaiPalyer = m_GameData.m_byNowOutStation;
		}
		SendCard.byCurrentUser = m_GameData.m_byNowOutStation;

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			memset(SendCard.byCard, 255, sizeof(SendCard.byCard));
			if (m_bSuperFlag[i])
			{
				SendCard.byCardData = byCard;
				SendCard.byCardCount = m_GameData.CopyOneUserHandPai(SendCard.byCard, m_GameData.m_byNowOutStation);
			}
			else
			{
				if (m_GameData.m_byNowOutStation == i)
				{
					SendCard.byCardData = byCard;
					SendCard.byCardCount = m_GameData.CopyOneUserHandPai(SendCard.byCard, m_GameData.m_byNowOutStation);
				}
				else
				{
					SendCard.byCardCount = m_GameData.GetHandPaiCount(m_GameData.m_byNowOutStation);
				}
			}
			if (m_GameData.m_byNowOutStation == i)
			{
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
				if (!m_GameData.m_mjAction.bTing)
				{
					memcpy(SendCard.byOutCanHuCard, m_GameData.m_byOutCanHuCard, sizeof(SendCard.byOutCanHuCard));
					memcpy(SendCard.byHuCard, m_GameData.m_byHuCard, sizeof(SendCard.byHuCard));
					memcpy(SendCard.byCardRemainNum, m_GameData.m_byCardRemainNum, sizeof(SendCard.byCardRemainNum));
				}
#endif
				OnJson(&SendCard, 3);
			}
			else
			{
				memset(SendCard.byOutCanHuCard, 255, sizeof(SendCard.byOutCanHuCard));
				memset(SendCard.byHuCard, 255, sizeof(SendCard.byHuCard));
				memset(SendCard.byCardRemainNum, 0, sizeof(SendCard.byCardRemainNum));
			}
			SendGameData(i, &SendCard, sizeof(SendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
			//SendWatchData(i,&SendCard,sizeof(SendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
		}
		////记录杠牌状态，拥于杠开和杠后炮
		RecordGangStatus();
		if (m_GameData.m_mjAction.bBuHua)
		{
			//有花牌的麻将，进行的相关操作
			if (!m_GameData.m_bIsBuHua[m_GameData.m_byNowOutStation])
			{
				//闲家补花
				BYTE byDeskStation = m_GameData.m_byNowOutStation;
				m_GameData.m_bIsBuHua[byDeskStation] = true;
				for (int j = 0; j < (int)m_GameData.m_vHuapaiTemp[byDeskStation].size();j++)
				{
					CMD_C_OutCard Data;
					memset(&Data, 0, sizeof(Data));
					Data.byOutCardData = m_GameData.m_vHuapaiTemp[byDeskStation][j];
					//Data.byOutCardIndex=m_GameData.m_byArHandPaiCount[byDeskStation]-1;
					OnUserOutCard(byDeskStation, &Data);
				}
				m_GameData.m_bIsBuAllHua[byDeskStation] = true;
			}
			//抓到花牌
			if (m_GameData.CheckIsHuaPai(byCard))
			{
				if (m_GameData.m_bIsBuAllHua[m_GameData.m_byNowOutStation])
				{
					SetTimer(TIME_BUHUA, m_GameData.m_mjRule.byAutoOutTime * 1000);
				}
				else
				{
					CMD_C_OutCard Data;
					memset(&Data, 0, sizeof(Data));
					Data.byOutCardData = byCard;
					//Data.byOutCardIndex=m_GameData.m_byArHandPaiCount[m_GameData.m_byNowOutStation]-1;
					OnUserOutCard(m_GameData.m_byNowOutStation, &Data);
				}
				return;
			}
		}
		//检测动作
		bool bBlock = false;
		if (CheckAction(m_GameData.m_byNowOutStation, byCard, 0))
		{
			bBlock = true;
			m_GameData.T_TingPai.bTing = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bTing;
			memcpy(m_GameData.T_TingPai.byTingCanOut, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byTingCanOut, sizeof(m_GameData.T_TingPai.byTingCanOut));
			CMD_S_OperateNotify ActionData;
			m_GameData.m_byCurrentOperatorUser = m_GameData.m_byNowOutStation;
			// 进入拦牌状态
			ActionData.byUsr = m_GameData.m_byNowOutStation;
			memcpy(ActionData.byGangData, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].iGangData, sizeof(ActionData.byGangData));
			ActionData.bGang = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bGang;
			ActionData.bHu = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bHu;
			if (ActionData.bHu)
			{
				ActionData.bTing = false;
			}
			else
			{
				ActionData.bTing = m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].bTing;
				memcpy(ActionData.byTingCanOut, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byTingCanOut, sizeof(ActionData.byTingCanOut));
				memcpy(ActionData.byHuCard, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byHuCard, sizeof(ActionData.byHuCard));
				memcpy(ActionData.byCardRemainNum, m_GameData.T_CPGNotify[m_GameData.m_byNowOutStation].byCardRemainNum, sizeof(ActionData.byCardRemainNum));
			}
			SendGameData(m_GameData.m_byNowOutStation, &ActionData, sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
			//SendWatchData(m_GameData.m_byNowOutStation,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		}
		if (bBlock)
		{
			if (m_bTrustee[m_GameData.m_byNowOutStation])
			{
				SetTimer(TIME_TUOGUAN_ACTION, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
				}
			}
		}
		else
		{
			if (m_GameData.m_bTing[m_GameData.m_byNowOutStation])
			{
				SetTimer(TIME_TING_OUTCARD, 1000);
			}
			else if (m_bTrustee[m_GameData.m_byNowOutStation])
			{
				SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
				}
			}
		}
	} //非流局
}

//用户操作
bool CServerGameDesk::OnUserOperate(BYTE byDeskStation, CMD_C_OperateCard *pAction)
{
	//效验状态
	if (GS_STATUS_PLAYING != GetGameStation())
	{
		return true;
	}
	if (!m_GameData.T_CPGNotify[byDeskStation].bCanAction)
	{
		return true;
	}
	if (ACTION_CHI == pAction->byOperateType)
	{
		return OnChi(byDeskStation, pAction);
	}
	else if (ACTION_PENG == pAction->byOperateType)
	{
		return OnPeng(byDeskStation, pAction);
	}
	else if (ACTION_GANG == pAction->byOperateType)//杠
	{
		return OnGang(byDeskStation, pAction);
	}
	else if (ACTION_HU == pAction->byOperateType)
	{
		return OnHu(byDeskStation, pAction);
	}
	else if (ACTION_PASS == pAction->byOperateType)
	{
		return OnPass(byDeskStation, pAction);
	}
	return true;
}

//吃牌
bool CServerGameDesk::OnChi(BYTE byDeskStation, CMD_C_OperateCard *pAction)
{
	if (!m_GameData.T_CPGNotify[byDeskStation].bChi || m_GameData.m_bTing[byDeskStation])
	{
		return true;
	}
	int iChiIndex = -1;
	for (int i = 0;i < 3;i++)
	{
		if (pAction->byActionData[0] == m_GameData.T_CPGNotify[byDeskStation].byChi[i][0])
		{
			iChiIndex = i;
			for (int b = 0;b < 3;b++)
			{
				if (m_GameData.T_CPGNotify[byDeskStation].byChi[iChiIndex][b] != pAction->byActionData[b])
				{
					iChiIndex = -1;
					break;
				}
			}
			break;
		}
	}
	if (-1 == iChiIndex)
	{
		return true;
	}
	m_GameData.T_TingPai.Init();
	m_GameData.temp_ChiPai.byUser = byDeskStation;
	m_GameData.temp_ChiPai.byBeChi = m_GameData.T_OutPai.byUser;
	m_GameData.temp_ChiPai.byPs = m_GameData.T_OutPai.byPs;
	for (int i = 0;i < 3;i++)
	{
		m_GameData.temp_ChiPai.byChiPs[i] = m_GameData.T_CPGNotify[byDeskStation].byChi[iChiIndex][i];
	}
	m_GameData.temp_ChiPai.byType = ACTION_CHI;
	m_GameData.T_CPGNotify[byDeskStation].Init();
	bool bSuccess = true;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (byDeskStation == i || !m_GameData.T_CPGNotify[i].bCanAction)
		{
			continue;
		}
		if (m_GameData.T_CPGNotify[i].bPeng || m_GameData.T_CPGNotify[i].bGang || m_GameData.T_CPGNotify[i].bHu)
		{
			bSuccess = false;
			break;
		}
	}
	if (bSuccess)
	{
		KillTimer(TIME_ACTION);
		KillTimer(TIME_THINK);
		KillTimer(TIME_TING_OUTCARD);
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			m_GameData.m_bGangState[i] = false;
		}
		//吃牌事件
		TCPGStruct chi;
		chi.Init();
		chi.byType = m_GameData.temp_ChiPai.byType;
		chi.iStation = m_GameData.temp_ChiPai.byUser;
		chi.iBeStation = m_GameData.temp_ChiPai.byBeChi;
		chi.iOutpai = m_GameData.temp_ChiPai.byPs;
		for (int i = 0;i < 3;i++)
		{
			chi.byData[i] = m_GameData.temp_ChiPai.byChiPs[i];
		}
		m_GameData.AddToGCP(m_GameData.temp_ChiPai.byUser, &chi);
		for (int i = 0;i < 3;i++)
		{
			if (chi.iOutpai == chi.byData[i])
			{
				continue;
			}
			//删除一张手牌
			m_GameData.DeleteAHandPai(chi.iStation, chi.byData[i]);
		}
		m_GameData.m_byCurrentOperatorUser = 255;
		CMD_S_OperateResult OperateResult;
		OperateResult.byType = chi.byType;
		OperateResult.iStation = chi.iStation;
		OperateResult.iBeStation = chi.iBeStation;
		OperateResult.iOutpai = chi.iOutpai;
		m_GameData.DeleteLastOutPai(chi.iBeStation);//删除被吃的牌
		//m_GameData.CopyOutPai(chi.iBeStation,OperateResult.byArOutPai);
		for (int i = 0;i < 3;i++)
		{
			OperateResult.byData[i] = chi.byData[i];
		}

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
			if (m_bSuperFlag[i])
			{
				m_GameData.SortHandPai(chi.iStation, false);
				OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, chi.iStation);
			}
			else
			{
				if (chi.iStation == i)
				{
					m_GameData.SortHandPai(chi.iStation, false);
					OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, chi.iStation);
				}
				else
				{
					OperateResult.byCardCount = m_GameData.GetHandPaiCount(chi.iStation);
				}
			}
			if (chi.iStation == i)
			{
				OnJson(&OperateResult, 4);
			}
			SendGameData(i, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
			//SendWatchData(i,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
		}
		m_GameData.m_byNowOutStation = OperateResult.iStation;
		bool bBlock = false;
		if (CheckAction(byDeskStation, 255, 4))
		{
			bBlock = true;
			CMD_S_OperateNotify ActionData;
			m_GameData.m_byCurrentOperatorUser = m_GameData.m_byNowOutStation;
			// 进入拦牌状态
			ActionData.Init();
			ActionData.byUsr = byDeskStation;
			ActionData.bTing = m_GameData.T_CPGNotify[byDeskStation].bTing;
			memcpy(ActionData.byTingCanOut, m_GameData.T_CPGNotify[byDeskStation].byTingCanOut, sizeof(ActionData.byTingCanOut));
			memcpy(ActionData.byHuCard, m_GameData.T_CPGNotify[byDeskStation].byHuCard, sizeof(ActionData.byHuCard));
			memcpy(ActionData.byCardRemainNum, m_GameData.T_CPGNotify[byDeskStation].byCardRemainNum, sizeof(ActionData.byCardRemainNum));
			SendGameData(byDeskStation, &ActionData, sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
			//SendWatchData(byDeskStation,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		}
		if (bBlock)
		{
			if (m_bTrustee[byDeskStation])
			{
				SetTimer(TIME_TUOGUAN_ACTION, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
				}
			}
		}
		else
		{
			if (m_bTrustee[byDeskStation])
			{
				SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
				}
			}
		}
	}
	else
	{
		HaveUserCanDoAction(byDeskStation); //别人有优先级更高的动作，吃失败
	}
	return true;
}

//碰牌
bool CServerGameDesk::OnPeng(BYTE byDeskStation, CMD_C_OperateCard *pAction)
{
	if (!m_GameData.T_CPGNotify[byDeskStation].bPeng || m_GameData.m_bTing[byDeskStation])
	{
		return true;
	}
	m_GameData.T_TingPai.Init();
	m_GameData.temp_PengPai.byUser = byDeskStation;
	m_GameData.temp_PengPai.byBePeng = m_GameData.T_OutPai.byUser;
	m_GameData.temp_PengPai.byPs = m_GameData.T_CPGNotify[byDeskStation].byPeng;
	m_GameData.T_CPGNotify[byDeskStation].Init();
	bool bSuccess = true;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (byDeskStation == i || !m_GameData.T_CPGNotify[i].bCanAction)
		{
			continue;
		}
		if (m_GameData.T_CPGNotify[i].bHu)
		{
			bSuccess = false;
			break;
		}
	}
	if (bSuccess)
	{
		KillTimer(TIME_ACTION);
		KillTimer(TIME_THINK);
		KillTimer(TIME_TING_OUTCARD);
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			m_GameData.m_bGangState[i] = false;
		}
		//碰牌事件
		TCPGStruct peng;
		peng.Init();
		peng.byType = ACTION_PENG;
		peng.iStation = m_GameData.temp_PengPai.byUser;
		peng.iBeStation = m_GameData.temp_PengPai.byBePeng;
		peng.iOutpai = m_GameData.temp_PengPai.byPs;
		for (int i = 0;i < 3;i++)
		{
			peng.byData[i] = m_GameData.temp_PengPai.byPs;
		}
		m_GameData.AddToGCP(m_GameData.temp_PengPai.byUser, &peng);
		for (int i = 0;i < 2;i++)
		{
			//删除一张手牌
			m_GameData.DeleteAHandPai(peng.iStation, peng.iOutpai);
		}
		m_GameData.m_byCurrentOperatorUser = 255;
		CMD_S_OperateResult OperateResult;
		OperateResult.byType = peng.byType;
		OperateResult.iStation = peng.iStation;
		OperateResult.iBeStation = peng.iBeStation;
		OperateResult.iOutpai = peng.iOutpai;
		m_GameData.DeleteLastOutPai(peng.iBeStation);//删除被碰的牌
		//m_GameData.CopyOutPai(peng.iBeStation,OperateResult.byArOutPai);
		//碰一张，这张出牌数量加2
		m_GameData.m_byAllOutCardNum[peng.iOutpai] += 2;
		for (int i = 0;i < 3;i++)
		{
			OperateResult.byData[i] = peng.byData[i];
		}
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
			if (m_bSuperFlag[i])
			{
				m_GameData.SortHandPai(peng.iStation, false);
				OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, peng.iStation);
			}
			else
			{
				if (peng.iStation == i)
				{
					m_GameData.SortHandPai(peng.iStation, false);
					OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, peng.iStation);
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
					if (!m_GameData.m_mjAction.bTing)
					{
						//提取庄家听牌数据
						memset(m_GameData.m_byOutCanHuCard, 255, sizeof(m_GameData.m_byOutCanHuCard));
						memset(m_GameData.m_byHuCard, 255, sizeof(m_GameData.m_byHuCard));
						memset(m_GameData.m_byCardRemainNum, 0, sizeof(m_GameData.m_byCardRemainNum));
						m_CheckHuData.OutCardExtractHu(&m_GameData, peng.iStation, m_GameData.m_byOutCanHuCard, m_GameData.m_byHuCard, m_GameData.m_byCardRemainNum);
						memcpy(OperateResult.byOutCanHuCard, m_GameData.m_byOutCanHuCard, sizeof(OperateResult.byOutCanHuCard));
						memcpy(OperateResult.byHuCard, m_GameData.m_byHuCard, sizeof(OperateResult.byHuCard));
						memcpy(OperateResult.byCardRemainNum, m_GameData.m_byCardRemainNum, sizeof(OperateResult.byCardRemainNum));
					}
#endif
				}
				else
				{
					OperateResult.byCardCount = m_GameData.GetHandPaiCount(peng.iStation);
				}
			}
			if (peng.iStation == i)
			{
				OnJson(&OperateResult, 4);
			}
			SendGameData(i, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
			//SendWatchData(i,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
		}
		m_GameData.m_byNowOutStation = OperateResult.iStation;
		bool bBlock = false;
		if (CheckAction(byDeskStation, 255, 4)) //碰后听牌
		{
			bBlock = true;
			CMD_S_OperateNotify ActionData;
			m_GameData.m_byCurrentOperatorUser = m_GameData.m_byNowOutStation;
			// 进入拦牌状态
			ActionData.Init();
			ActionData.byUsr = byDeskStation;
			ActionData.bTing = m_GameData.T_CPGNotify[byDeskStation].bTing;
			memcpy(ActionData.byTingCanOut, m_GameData.T_CPGNotify[byDeskStation].byTingCanOut, sizeof(ActionData.byTingCanOut));
			memcpy(ActionData.byHuCard, m_GameData.T_CPGNotify[byDeskStation].byHuCard, sizeof(ActionData.byHuCard));
			memcpy(ActionData.byCardRemainNum, m_GameData.T_CPGNotify[byDeskStation].byCardRemainNum, sizeof(ActionData.byCardRemainNum));
			SendGameData(byDeskStation, &ActionData, sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
			//SendWatchData(byDeskStation,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
		}
		if (bBlock)
		{
			if (m_bTrustee[byDeskStation])
			{
				SetTimer(TIME_TUOGUAN_ACTION, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
				}
			}
		}
		else
		{
			if (m_bTrustee[byDeskStation])
			{
				SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
				}
			}
		}
	}
	else
	{
		HaveUserCanDoAction(byDeskStation); //别人有胡，碰失败
	}
	return true;
}

//杠牌
bool CServerGameDesk::OnGang(BYTE byDeskStation, CMD_C_OperateCard *pAction)
{
	if (!m_GameData.T_CPGNotify[byDeskStation].bGang || pAction->byActionData[0] == 255)
	{
		return true;
	}
	int iGaneIndex = -1;
	for (int i = 0;i < 4;i++)
	{
		if (pAction->byActionData[0] == m_GameData.T_CPGNotify[byDeskStation].iGangData[i][1])
		{
			iGaneIndex = i;
			break;
		}
	}
	if (-1 == iGaneIndex)
	{
		return true;
	}
	m_GameData.T_TingPai.Init();
	m_GameData.temp_GangPai.byUser = byDeskStation;
	m_GameData.temp_GangPai.byPs = m_GameData.T_CPGNotify[byDeskStation].iGangData[iGaneIndex][1];
	m_GameData.temp_GangPai.byType = m_GameData.T_CPGNotify[byDeskStation].iGangData[iGaneIndex][0];
	if (ACTION_AN_GANG == m_GameData.temp_GangPai.byType)
	{
		m_GameData.temp_GangPai.byBeGang = 255;
	}
	else if (ACTION_BU_GANG == m_GameData.temp_GangPai.byType)
	{
		//补杠，byBeGang为放碰玩家
		for (int i = 0;i < 5;i++)
		{
			if (ACTION_PENG == m_GameData.m_UserGCPData[byDeskStation][i].byType &&
				m_GameData.m_UserGCPData[byDeskStation][i].iOutpai == m_GameData.temp_GangPai.byPs)
			{
				m_GameData.temp_GangPai.byBeGang = m_GameData.m_UserGCPData[byDeskStation][i].iBeStation;
				break;
			}
		}
	}
	else
	{
		m_GameData.temp_GangPai.byBeGang = m_GameData.T_OutPai.byUser;
	}
	for (int i = 0;i < 4;i++)
	{
		m_GameData.temp_GangPai.byGangPai[i] = m_GameData.T_CPGNotify[byDeskStation].iGangData[iGaneIndex][1];
	}
	m_GameData.T_CPGNotify[byDeskStation].Init();
	bool bSuccess = true;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (byDeskStation == i || !m_GameData.T_CPGNotify[i].bCanAction)
		{
			continue;
		}
		if (m_GameData.T_CPGNotify[i].bHu)
		{
			bSuccess = false;
			break;
		}
	}
	if (bSuccess)
	{
		KillTimer(TIME_ACTION);
		KillTimer(TIME_THINK);
		KillTimer(TIME_TING_OUTCARD);
		m_GameData.m_bGangState[byDeskStation] = true;
		m_GameData.T_GangPai.Init();
		m_GameData.T_GangPai.byUser = m_GameData.temp_GangPai.byUser;
		m_GameData.T_GangPai.byBeGang = m_GameData.temp_GangPai.byBeGang;
		m_GameData.T_GangPai.byPs = m_GameData.temp_GangPai.byPs;
		m_GameData.T_GangPai.byType = m_GameData.temp_GangPai.byType;
		//杠牌事件
		TCPGStruct gang;
		gang.Init();
		gang.byType = m_GameData.temp_GangPai.byType;
		gang.iStation = m_GameData.temp_GangPai.byUser;
		gang.iBeStation = m_GameData.temp_GangPai.byBeGang;
		gang.iOutpai = m_GameData.temp_GangPai.byPs;
		memcpy(gang.byData, m_GameData.temp_GangPai.byGangPai, sizeof(gang.byData));
		m_GameData.AddToGCP(m_GameData.temp_GangPai.byUser, &gang);
		m_CPGData.CountGangFen(&m_GameData, gang.iStation, gang.iBeStation, gang.byType);
		//杠牌，这张牌全部打出去了
		m_GameData.m_byAllOutCardNum[gang.iOutpai] = 4;
		if (!IsGoldRoom())
		{
			//统计战绩
			m_tZongResult.iGang[gang.iStation] ++;
			if (gang.iBeStation != 255)
			{
				m_tZongResult.iDianGang[gang.iBeStation] ++;
			}
		}

		CMD_S_OperateResult OperateResult;
		if (ACTION_AN_GANG == gang.byType)
		{
			for (int i = 0;i < 4;i++)
			{
				//删除一张手牌
				m_GameData.DeleteAHandPai(gang.iStation, gang.iOutpai);
			}
		}
		else if (ACTION_BU_GANG == gang.byType)
		{
			m_GameData.DeleteAGCPData(gang.iStation, ACTION_PENG, gang.iOutpai);
			//删除一张手牌
			m_GameData.DeleteAHandPai(gang.iStation, gang.iOutpai);
		}
		else if (ACTION_MING_GANG == gang.byType)
		{
			for (int i = 0;i < 3;i++)
			{
				//删除一张手牌
				m_GameData.DeleteAHandPai(gang.iStation, gang.iOutpai);
			}
			//删除一张出牌
			m_GameData.DeleteLastOutPai(gang.iBeStation);//删除被杠的牌
		}
		for (int i = 0;i < 4;i++)
		{
			OperateResult.byData[i] = gang.byData[i];
		}
		OperateResult.byType = gang.byType;
		OperateResult.iStation = gang.iStation;
		OperateResult.iBeStation = gang.iBeStation;
		OperateResult.iOutpai = gang.iOutpai;
		if (255 != gang.iBeStation)
		{
			m_GameData.DeleteLastOutPai(gang.iBeStation);//删除被杠的牌
			//m_GameData.CopyOutPai(gang.iBeStation,OperateResult.byArOutPai);
		}
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
			if (m_bSuperFlag[i])
			{
				m_GameData.SortHandPai(gang.iStation, false);
				OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, gang.iStation);
			}
			else
			{
				if (gang.iStation == i)
				{
					m_GameData.SortHandPai(gang.iStation, false);
					OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, gang.iStation);
				}
				else
				{
					OperateResult.byCardCount = m_GameData.GetHandPaiCount(gang.iStation);
				}
			}
			if (gang.iStation == i)
			{
				OnJson(&OperateResult, 4);
			}
			SendGameData(i, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
			//SendWatchData(i,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
		}
		m_GameData.m_byCurrentOperatorUser = 255;
		bool bAction = false;
		if (ACTION_BU_GANG == gang.byType)//是否有抢杠胡
		{
			if (CheckAction(gang.iStation, gang.iOutpai, 3))
			{
				//发送了拦牌给客户端玩家 
				if (HaveUserCanDoAction(byDeskStation))
				{
					bAction = true;
				}
			}
		}
		if (!bAction)
		{
			ZhuaPai(OperateResult.iStation, false);
		}
	}
	else
	{
		HaveUserCanDoAction(byDeskStation); //别人有胡，杠失败
	}
	return true;
}

//胡牌
bool CServerGameDesk::OnHu(BYTE byDeskStation, CMD_C_OperateCard *pAction)
{
	// 统计消耗
	//AUTOCOST("OnHu 耗时统计玩家%d", byDeskStation);
	if (!m_GameData.T_CPGNotify[byDeskStation].bHu)
	{
		return true;
	}
	KillTimer(TIME_ACTION);
	KillTimer(TIME_THINK);
	KillTimer(TIME_TING_OUTCARD);
	m_GameData.T_TingPai.Init();
	m_GameData.m_bIsHu[byDeskStation] = true;
	m_GameData.temp_HuPai[byDeskStation].byHu = byDeskStation;
	m_GameData.temp_HuPai[byDeskStation].byFangPao = m_GameData.T_CPGNotify[byDeskStation].byFangPao;
	m_GameData.temp_HuPai[byDeskStation].byPs = m_GameData.T_CPGNotify[byDeskStation].iHuPs;
	m_GameData.T_CPGNotify[byDeskStation].Init();
	bool bSuccess = true;
	//发送胡通知
	CMD_S_OperateResult OperateResult;//客户端消除动作界面用
	OperateResult.byType = ACTION_HU;
	OperateResult.iStation = byDeskStation;
	OperateResult.iBeStation = m_GameData.temp_HuPai[byDeskStation].byFangPao;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
		if (m_bSuperFlag[i])
		{
			m_GameData.SortHandPai(i, false);
			OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, byDeskStation);
		}
		else
		{
			if (byDeskStation == i)
			{
				m_GameData.SortHandPai(i, false);
				OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, byDeskStation);
			}
			else
			{
				OperateResult.byCardCount = m_GameData.GetHandPaiCount(byDeskStation);
			}
		}
		SendGameData(i, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
		//SendWatchData(i,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
	}
	OnJson(&OperateResult, 4);
	if (m_GameData.m_mjRule.bIsDuoXiang)
	{
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (byDeskStation == i || !m_GameData.T_CPGNotify[i].bCanAction)
			{
				continue;
			}
			if (m_GameData.T_CPGNotify[i].bHu)
			{
				bSuccess = false;
				break;
			}
		}
	}
	//检测是否还有其它玩家可以胡牌
	if (!m_GameData.m_mjRule.bIsDuoXiang || bSuccess)
	{
		KillTimer(TIME_ACTION);
		KillTimer(TIME_THINK);
		KillTimer(TIME_TING_OUTCARD);
		int iDelete = 0;
		int iPlayer = m_GameData.m_byNowOutStation == 255 ? m_GameData.T_OutPai.byUser : m_GameData.m_byNowOutStation;
		bool bFirst = true;
		bool bHu = false;
		for (int iCount = 0;iCount < PLAY_COUNT;iCount++)
		{
			if (!bFirst)
			{
				iPlayer = m_GameData.GetNextStation(iPlayer, false);
			}
			else
			{
				bFirst = !bFirst;
			}
			if (255 != m_GameData.temp_HuPai[iPlayer].byHu)
			{
				if (255 != m_GameData.temp_HuPai[iPlayer].byFangPao && iDelete <= 0)
				{
					iDelete++;
					m_GameData.DeleteLastOutPai(m_GameData.temp_HuPai[iPlayer].byFangPao);//删除被胡的牌
				}
				//计算胡牌数据
				m_GameData.T_HuPai.bIsLiuJu = false;
				m_GameData.T_HuPai.bZimo = (m_GameData.temp_HuPai[iPlayer].byFangPao == 255) ? true : false;
				m_GameData.T_HuPai.byHu = iPlayer;
				m_GameData.T_HuPai.byFangPao = m_GameData.temp_HuPai[iPlayer].byFangPao;
				m_GameData.T_HuPai.byPs = m_GameData.temp_HuPai[iPlayer].byPs;
				m_GameData.T_HuPai.bHaveHu[iPlayer] = true;
				m_GameData.T_HuPai.iUserNums++;
				bHu = true;
				if (!IsGoldRoom())
				{
					//统计战绩
					if (m_GameData.T_HuPai.bZimo)
					{
						m_tZongResult.iZimo[iPlayer] ++;
					}
					else
					{
						m_tZongResult.iJiePao[iPlayer] ++;
					}
				}
				if (!m_GameData.m_mjRule.bIsDuoXiang)
					break;
			}
		}
		if (bHu)
		{
			m_CheckHuData.CountHuFen(&m_GameData);
			if (!IsGoldRoom())
			{
				//统计点炮次数
				if (!m_GameData.T_HuPai.bZimo && m_GameData.T_HuPai.byFangPao != 255)
				{
					m_tZongResult.iDianPao[m_GameData.T_HuPai.byFangPao] ++;
				}
			}
			//抢杠胡，除去杠分
			if (!m_GameData.T_HuPai.bZimo && m_GameData.T_HuPai.byFangPao != 255 && ACTION_BU_GANG == m_GameData.T_GangPai.byType
				&& m_GameData.m_bGangState[m_GameData.T_HuPai.byFangPao])
			{
				//将杠分去掉
				m_CPGData.MinusGangFen(&m_GameData, m_GameData.T_GangPai.byUser, m_GameData.T_GangPai.byBeGang);
			}
			GameFinish(byDeskStation, GF_NORMAL);
		}
	}
	else //一炮多响，继续发送动作
	{
		HaveUserCanDoAction(byDeskStation);
	}
	return true;
}

//过牌
bool CServerGameDesk::OnPass(BYTE byDeskStation, CMD_C_OperateCard *pAction)
{
	if (!m_GameData.T_CPGNotify[byDeskStation].bCanAction)
	{
		return true;
	}
	m_GameData.T_TingPai.Init();
	bool bTing = m_GameData.T_CPGNotify[byDeskStation].bTing;
	//记录弃胡和弃碰
	if (m_GameData.T_CPGNotify[byDeskStation].bHu)
	{
		m_GameData.m_bQiHu[byDeskStation] = true;
	}
	if (m_GameData.T_CPGNotify[byDeskStation].bPeng)
	{
		m_GameData.m_bQiPeng[byDeskStation] = true;
		m_GameData.m_vecQiPengCard[byDeskStation].push_back(m_GameData.T_CPGNotify[byDeskStation].byPeng);
	}
	//清理记录
	m_GameData.T_CPGNotify[byDeskStation].Init();
	if (IsGoldRoom())
	{
		CMD_S_OperateResult OperateResult;//客户端消除动作界面用
		OperateResult.byType = ACTION_PASS;
		OperateResult.iStation = byDeskStation;
		/*if(bTing)
		{
		OperateResult.bZhua=false;
		}
		else
		{
		if(m_GameData.m_byNowOutStation==byDeskStation)
		{
		OperateResult.bZhua=true;
		}
		}*/
		OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, byDeskStation);
		SendGameData(byDeskStation, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
		//SendWatchData(byDeskStation,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
	}
	if (!HaveUserCanDoAction(byDeskStation)) //没有发任何动作
	{
		bool bEnd = false;
		if (!bEnd)
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			//逆时针胡牌
			int iDelete = 0;
			int iPlayer = m_GameData.m_byNowOutStation == 255 ? m_GameData.T_OutPai.byUser : m_GameData.m_byNowOutStation;
			bool bFirst = true;
			for (int iCount = 0;iCount < PLAY_COUNT;iCount++)
			{
				if (!bFirst)
				{
					iPlayer = m_GameData.GetNextStation(iPlayer, false);
				}
				else
				{
					bFirst = !bFirst;
				}
				if (255 != m_GameData.temp_HuPai[iPlayer].byHu)
				{
					if (255 != m_GameData.temp_HuPai[iPlayer].byFangPao && iDelete <= 0)
					{
						iDelete++;
						m_GameData.DeleteLastOutPai(m_GameData.temp_HuPai[iPlayer].byFangPao);//删除被胡的牌
					}
					//计算胡牌数据
					m_GameData.T_HuPai.bIsLiuJu = false;
					m_GameData.T_HuPai.bZimo = (m_GameData.temp_HuPai[iPlayer].byFangPao == 255) ? true : false;
					m_GameData.T_HuPai.byHu = iPlayer; //胡牌玩家位置
					m_GameData.T_HuPai.byFangPao = m_GameData.temp_HuPai[iPlayer].byFangPao; //放炮玩家位置
					m_GameData.T_HuPai.byPs = m_GameData.temp_HuPai[iPlayer].byPs;
					m_GameData.T_HuPai.bHaveHu[iPlayer] = true;
					m_GameData.T_HuPai.iUserNums++;
					bEnd = true;
					if (!IsGoldRoom())
					{
						//统计战绩
						if (m_GameData.T_HuPai.bZimo)
						{
							m_tZongResult.iZimo[iPlayer] ++;
						}
						else
						{
							m_tZongResult.iJiePao[iPlayer] ++;
						}
					}
					if (!m_GameData.m_mjRule.bIsDuoXiang)
						break;
				}
			}
			if (bEnd)
			{
				m_CheckHuData.CountHuFen(&m_GameData);
				if (!IsGoldRoom())
				{
					//统计点炮次数
					if (!m_GameData.T_HuPai.bZimo && m_GameData.T_HuPai.byFangPao != 255)
					{
						m_tZongResult.iDianPao[m_GameData.T_HuPai.byFangPao] ++;
					}
				}
				//抢杠胡，除去杠分
				if (!m_GameData.T_HuPai.bZimo && m_GameData.T_HuPai.byFangPao != 255 && ACTION_BU_GANG == m_GameData.T_GangPai.byType
					&& m_GameData.m_bGangState[m_GameData.T_HuPai.byFangPao])
				{
					//将杠分去掉
					m_CPGData.MinusGangFen(&m_GameData, m_GameData.T_GangPai.byUser, m_GameData.T_GangPai.byBeGang);
				}
				GameFinish(byDeskStation, GF_NORMAL);
				return true;
			}
		}

		////////////////////////////杠
		if (!bEnd && ACTION_NO != m_GameData.temp_GangPai.byType)
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			bEnd = true;
			//杠牌事件
			m_GameData.m_bGangState[m_GameData.temp_GangPai.byUser] = true;
			m_GameData.T_GangPai.Init();
			m_GameData.T_GangPai.byUser = m_GameData.temp_GangPai.byUser;
			m_GameData.T_GangPai.byBeGang = m_GameData.temp_GangPai.byBeGang;
			m_GameData.T_GangPai.byPs = m_GameData.temp_GangPai.byPs;
			m_GameData.T_GangPai.byType = m_GameData.temp_GangPai.byType;

			TCPGStruct gang;
			gang.Init();
			gang.byType = m_GameData.temp_GangPai.byType;
			gang.iStation = m_GameData.temp_GangPai.byUser;
			gang.iBeStation = m_GameData.temp_GangPai.byBeGang;
			gang.iOutpai = m_GameData.temp_GangPai.byPs;
			memcpy(gang.byData, m_GameData.temp_GangPai.byGangPai, sizeof(gang.byData));
			m_GameData.AddToGCP(m_GameData.temp_GangPai.byUser, &gang);
			m_CPGData.CountGangFen(&m_GameData, gang.iStation, gang.iBeStation, gang.byType);
			//杠牌，这张牌全部打出去了
			m_GameData.m_byAllOutCardNum[gang.iOutpai] = 4;
			if (!IsGoldRoom())
			{
				//统计战绩
				m_tZongResult.iGang[gang.iStation] ++;
				if (gang.iBeStation != 255)
				{
					m_tZongResult.iDianGang[gang.iBeStation] ++;
				}
			}

			if (ACTION_AN_GANG == gang.byType)
			{
				for (int i = 0;i < 4;i++)
				{
					//删除一张手牌
					m_GameData.DeleteAHandPai(gang.iStation, gang.iOutpai);
				}
			}
			else if (ACTION_BU_GANG == gang.byType)
			{
				m_GameData.DeleteAGCPData(gang.iStation, ACTION_PENG, gang.iOutpai);
				//删除一张手牌
				m_GameData.DeleteAHandPai(gang.iStation, gang.iOutpai);
			}
			else if (ACTION_MING_GANG == gang.byType)
			{
				for (int i = 0;i < 3;i++)
				{
					//删除一张手牌
					m_GameData.DeleteAHandPai(gang.iStation, gang.iOutpai);
				}
				//删除一张出牌
				m_GameData.DeleteLastOutPai(gang.iBeStation);//删除被杠的牌
			}
			CMD_S_OperateResult OperateResult;
			OperateResult.byType = gang.byType;
			OperateResult.iStation = gang.iStation;
			OperateResult.iBeStation = gang.iBeStation;
			OperateResult.iOutpai = gang.iOutpai;
			if (255 != gang.iBeStation)
			{
				m_GameData.DeleteLastOutPai(gang.iBeStation);//删除被杠的牌
				//m_GameData.CopyOutPai(gang.iBeStation,OperateResult.byArOutPai);
			}
			for (int b = 0;b < 4;b++)
			{
				OperateResult.byData[b] = gang.byData[b];
			}
			for (int b = 0;b < PLAY_COUNT;b++)
			{
				memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
				if (m_bSuperFlag[b])
				{
					m_GameData.SortHandPai(gang.iStation, false);
					OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, gang.iStation);
				}
				else
				{
					if (gang.iStation == b)
					{
						m_GameData.SortHandPai(gang.iStation, false);
						OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, gang.iStation);
					}
					else
					{
						OperateResult.byCardCount = m_GameData.GetHandPaiCount(gang.iStation);
					}
				}
				if (gang.iStation == b)
				{
					OnJson(&OperateResult, 4);
				}
				SendGameData(b, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
				//SendWatchData(b,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
			}
			ZhuaPai(OperateResult.iStation, false);
		}

		if (!bEnd && 255 != m_GameData.temp_PengPai.byUser)
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			bEnd = true;
			//碰牌事件
			TCPGStruct peng;
			peng.Init();
			peng.byType = ACTION_PENG;
			peng.iStation = m_GameData.temp_PengPai.byUser;
			peng.iBeStation = m_GameData.temp_PengPai.byBePeng;
			peng.iOutpai = m_GameData.temp_PengPai.byPs;
			for (int b = 0;b < 3;b++)
			{
				peng.byData[b] = m_GameData.temp_PengPai.byPs;
			}
			m_GameData.AddToGCP(m_GameData.temp_PengPai.byUser, &peng);
			for (int b = 0;b < 2;b++)
			{
				//删除一张手牌
				m_GameData.DeleteAHandPai(peng.iStation, peng.iOutpai);
			}
			CMD_S_OperateResult OperateResult;
			OperateResult.byType = peng.byType;
			OperateResult.iStation = peng.iStation;
			OperateResult.iBeStation = peng.iBeStation;
			OperateResult.iOutpai = peng.iOutpai;
			m_GameData.DeleteLastOutPai(peng.iBeStation);//删除被碰的牌
			//m_GameData.CopyOutPai(peng.iBeStation,OperateResult.byArOutPai);
			//碰一张，这张出牌数量加2
			m_GameData.m_byAllOutCardNum[peng.iOutpai] += 2;
			for (int b = 0;b < 3;b++)
			{
				OperateResult.byData[b] = peng.byData[b];
			}
			for (int b = 0;b < PLAY_COUNT;b++)
			{
				memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
				if (m_bSuperFlag[b])
				{
					m_GameData.SortHandPai(b, false);
					OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, peng.iStation);
				}
				else
				{
					if (peng.iStation == b)
					{
						m_GameData.SortHandPai(b, false);
						OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, peng.iStation);
#ifdef RULE_HAVE_TING_PAI_NOTIFY //是否有听牌提示
						if (!m_GameData.m_mjAction.bTing)
						{
							//提取庄家听牌数据
							memset(m_GameData.m_byOutCanHuCard, 255, sizeof(m_GameData.m_byOutCanHuCard));
							memset(m_GameData.m_byHuCard, 255, sizeof(m_GameData.m_byHuCard));
							memset(m_GameData.m_byCardRemainNum, 0, sizeof(m_GameData.m_byCardRemainNum));
							m_CheckHuData.OutCardExtractHu(&m_GameData, peng.iStation, m_GameData.m_byOutCanHuCard, m_GameData.m_byHuCard, m_GameData.m_byCardRemainNum);
							memcpy(OperateResult.byOutCanHuCard, m_GameData.m_byOutCanHuCard, sizeof(OperateResult.byOutCanHuCard));
							memcpy(OperateResult.byHuCard, m_GameData.m_byHuCard, sizeof(OperateResult.byHuCard));
							memcpy(OperateResult.byCardRemainNum, m_GameData.m_byCardRemainNum, sizeof(OperateResult.byCardRemainNum));
						}
#endif
					}
					else
					{
						OperateResult.byCardCount = m_GameData.GetHandPaiCount(peng.iStation);
					}
				}
				if (peng.iStation == b)
				{
					OnJson(&OperateResult, 4);
				}
				SendGameData(b, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
				//SendWatchData(b,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
			}
			m_GameData.m_byNowOutStation = OperateResult.iStation;
			if (IsGoldRoom())
			{
				SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
			}
		}

		if (!bEnd && ACTION_NO != m_GameData.temp_ChiPai.byType)
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			bEnd = true;
			//吃牌事件
			TCPGStruct chi;
			chi.Init();
			chi.byType = m_GameData.temp_ChiPai.byType;
			chi.iStation = m_GameData.temp_ChiPai.byUser;
			chi.iBeStation = m_GameData.temp_ChiPai.byBeChi;
			chi.iOutpai = m_GameData.temp_ChiPai.byPs;
			for (int b = 0;b < 3;b++)
			{
				chi.byData[b] = m_GameData.temp_ChiPai.byChiPs[b];
			}
			m_GameData.AddToGCP(m_GameData.temp_ChiPai.byUser, &chi);
			for (int b = 0;b < 3;b++)
			{
				if (chi.iOutpai == chi.byData[b])
				{
					continue;
				}
				//删除一张手牌
				m_GameData.DeleteAHandPai(chi.iStation, chi.byData[b]);
			}
			CMD_S_OperateResult OperateResult;
			OperateResult.byType = chi.byType;
			OperateResult.iStation = chi.iStation;
			OperateResult.iBeStation = chi.iBeStation;
			OperateResult.iOutpai = chi.iOutpai;
			m_GameData.DeleteLastOutPai(chi.iBeStation);//删除被吃的牌
			//m_GameData.CopyOutPai(chi.iBeStation,OperateResult.byArOutPai);
			for (int b = 0;b < 3;b++)
			{
				OperateResult.byData[b] = chi.byData[b];
			}
			for (int b = 0;b < PLAY_COUNT;b++)
			{
				memset(OperateResult.byCard, 255, sizeof(OperateResult.byCard));
				if (m_bSuperFlag[b])
				{
					m_GameData.SortHandPai(chi.iStation, false);
					OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, chi.iStation);
				}
				else
				{
					if (chi.iStation == b)
					{
						m_GameData.SortHandPai(chi.iStation, false);
						OperateResult.byCardCount = m_GameData.CopyOneUserHandPai(OperateResult.byCard, chi.iStation);
					}
					else
					{
						OperateResult.byCardCount = m_GameData.GetHandPaiCount(chi.iStation);
					}
				}
				if (chi.iStation == b)
				{
					OnJson(&OperateResult, 4);
				}
				SendGameData(b, &OperateResult, sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
				//SendWatchData(b,&OperateResult,sizeof(OperateResult), MSG_MAIN_LOADER_GAME, S_C_OPERATE_RESULT, 0);
			}
			m_GameData.m_byNowOutStation = OperateResult.iStation;
			if (IsGoldRoom())
			{
				SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
			}
		}

		if (!bEnd && 255 == m_GameData.m_byNowOutStation)//开始摸牌
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			bEnd = true;
			ZhuaPai(m_GameData.T_OutPai.byUser, true);
		}

		if (!bEnd && m_GameData.m_byNowOutStation == byDeskStation)//自己摸牌/吃碰后听牌有动作pass以后客服端重新计时出牌
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			bEnd = true;
			if (m_bTrustee[m_GameData.m_byNowOutStation])
			{
				SetTimer(TIME_TUOGUAN, m_GameData.m_mjRule.byAutoOutTime * 1000);
			}
			else
			{
				if (IsGoldRoom())
				{
					SetTimer(TIME_THINK, (m_GameData.m_mjRule.byOutTime + 2) * 1000);
				}
			}
		}

		if (!bEnd && m_GameData.m_byNowOutStation != 255)//抢杠胡点过，开始摸牌
		{
			KillTimer(TIME_ACTION);
			KillTimer(TIME_THINK);
			KillTimer(TIME_TING_OUTCARD);
			bEnd = true;
			ZhuaPai(m_GameData.m_byNowOutStation, false);
		}
	}
	return true;
}

///检测玩家station当前能做哪些动作
bool CServerGameDesk::CheckAction(int iStation, BYTE byCard, BYTE byType)
{
	// 统计消耗
	//AUTOCOST("CheckAction 耗时统计玩家%d", iStation);
	for (int i = 0;i < PLAY_COUNT;++i)
	{
		m_GameData.T_CPGNotify[i].Init();//清零
	}
	m_GameData.T_CountFen.Init();
	m_GameData.T_TingPai.Init();
	bool bAction = false;
	switch (byType)
	{
	case 0://自摸
	case 1://杠开(杠牌以后摸到牌胡)
	{
		//if (m_bTrustee[iStation]) //玩家托管不检测动作
		//{
		//	break;
		//}

		//糊
		if (m_CheckHuData.CanHu(&m_GameData, iStation, 255, byCard, true))
		{
			m_GameData.T_CPGNotify[iStation].bHu = true;
			m_GameData.T_CPGNotify[iStation].bZhuaPai = true;
			m_GameData.T_CPGNotify[iStation].iHuPs = byCard;
			m_GameData.T_CPGNotify[iStation].byHu = iStation;
			m_GameData.T_CPGNotify[iStation].byMaxThing = ACTION_HU;
		}

		//听
		if (!m_GameData.m_bTing[iStation] && m_GameData.m_mjAction.bTing && !m_GameData.T_CPGNotify[iStation].bHu &&
			m_CheckHuData.OutCardExtractHu(&m_GameData, iStation, m_GameData.T_CPGNotify[iStation].byTingCanOut,
				m_GameData.T_CPGNotify[iStation].byHuCard, m_GameData.T_CPGNotify[iStation].byCardRemainNum))
		{
			m_GameData.T_CPGNotify[iStation].bTing = true;
			m_GameData.T_CPGNotify[iStation].byMaxThing = ACTION_TING_PAI;
		}

		//杠
		if (m_CPGData.CanGang(&m_GameData, iStation, byCard, m_GameData.T_CPGNotify[iStation].iGangData, true, &m_CheckHuData))
		{
			m_GameData.T_CPGNotify[iStation].bGang = true;
			m_GameData.T_CPGNotify[iStation].bZhuaPai = true;
			m_GameData.T_CPGNotify[iStation].byMaxThing = ACTION_AN_GANG;
		}
		if (m_GameData.T_CPGNotify[iStation].bTing || m_GameData.T_CPGNotify[iStation].bHu || m_GameData.T_CPGNotify[iStation].bGang)
		{
			bAction = true;
			m_GameData.T_CPGNotify[iStation].bCanAction = true;
		}
	}
	break;
	case 2://点炮
	{
		for (int i = 0;i < PLAY_COUNT;++i)
		{
			if (m_GameData.m_bIsHu[i])        // 已胡牌玩家,或者断线状态
			{
				continue;
			}
			//if (m_bTrustee[i]) //玩家托管不检测动作
			//{
			//	continue;
			//}
			if (i == iStation)
			{
				continue;
			}
			//碰
			if (!m_GameData.m_bTing[i] && m_CPGData.CanPeng(&m_GameData, i, byCard))
			{
				m_GameData.T_CPGNotify[i].bPeng = true;
				m_GameData.T_CPGNotify[i].byPeng = byCard;
				m_GameData.T_CPGNotify[i].byMaxThing = ACTION_PENG;
			}
			//杠
			if (m_CPGData.CanGang(&m_GameData, i, byCard, m_GameData.T_CPGNotify[i].iGangData, false, &m_CheckHuData))
			{
				m_GameData.T_CPGNotify[i].bGang = true;
				m_GameData.T_CPGNotify[i].byMaxThing = ACTION_MING_GANG;
			}
			//糊
			if (m_GameData.m_mjAction.bFangPao&&m_CheckHuData.CanHu(&m_GameData, i, iStation, byCard, false))
			{

				m_GameData.T_CPGNotify[i].bHu = true;
				m_GameData.T_CPGNotify[i].byMaxThing = ACTION_HU;
				m_GameData.T_CPGNotify[i].iHuPs = byCard;
				m_GameData.T_CPGNotify[i].byHu = i;
				m_GameData.T_CPGNotify[i].byFangPao = iStation;
			}
			if (m_GameData.T_CPGNotify[i].bHu || m_GameData.T_CPGNotify[i].bPeng || m_GameData.T_CPGNotify[i].bGang
				|| m_GameData.T_CPGNotify[i].bTing)
			{
				bAction = true;
				m_GameData.T_CPGNotify[i].bCanAction = true;
			}
		}
	}
	break;
	case 3://抢杠胡
	{
		for (int i = 0;i < PLAY_COUNT;++i)
		{
			if (m_GameData.m_bIsHu[i])        // 已胡牌玩家,或者断线状态
			{
				continue;
			}
			//if (m_bTrustee[i]) //玩家托管不检测动作
			//{
			//	continue;
			//}
			if (i == iStation)
			{
				continue;
			}
			//糊
			if (m_GameData.m_mjAction.bQiangGangHu && m_CheckHuData.CanHu(&m_GameData, i, iStation, byCard, false))
			{
				m_GameData.T_CPGNotify[i].bHu = true;
				m_GameData.T_CPGNotify[i].iHuPs = byCard;//记录胡的牌
				m_GameData.T_CPGNotify[i].byHu = i;
				m_GameData.T_CPGNotify[i].byFangPao = iStation;
				m_GameData.T_CPGNotify[i].byMaxThing = ACTION_HU;
			}
			if (m_GameData.T_CPGNotify[i].bHu)
			{
				bAction = true;
				m_GameData.T_CPGNotify[i].bCanAction = true;
			}
		}
	}
	break;
	case 4: //检测玩家是否听牌
	{
		//if (m_bTrustee[iStation]) //玩家托管不检测动作
		//{
		//	break;
		//}
		//听
		if (!m_GameData.m_bTing[iStation] && m_GameData.m_mjAction.bTing && m_CheckHuData.OutCardExtractHu(&m_GameData, iStation, m_GameData.T_CPGNotify[iStation].byTingCanOut,
			m_GameData.T_CPGNotify[iStation].byHuCard, m_GameData.T_CPGNotify[iStation].byCardRemainNum))
		{
			m_GameData.T_CPGNotify[iStation].bTing = true;
			m_GameData.T_CPGNotify[iStation].byMaxThing = ACTION_TING_PAI;
		}

		if (m_GameData.T_CPGNotify[iStation].bTing)
		{
			bAction = true;
			m_GameData.T_CPGNotify[iStation].bCanAction = true;
		}
	}
	break;
	}
	if (bAction)
	{
		m_GameData.temp_ChiPai.Init();
		m_GameData.temp_PengPai.Init();
		m_GameData.temp_GangPai.Init();
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			m_GameData.temp_HuPai[i].Init();
		}
	}
	return bAction;
}

///发送拦牌信息给玩家
bool CServerGameDesk::HaveUserCanDoAction(BYTE byIgnoreStation)
{
	bool bHaveSend = false;	//是否已经发送了消息
	BYTE byCanActionDesk = 255;	//可以做动作的玩家
	m_GameData.m_byCurrentOperatorUser = 255;
	bool bIsHaveHuPai = false;
	int iCanHuCount = 0; //已经胡
	//检测此时是否有玩家已经胡牌
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_GameData.m_bIsHu[i])
		{
			bIsHaveHuPai = true;
			break;
		}
	}
	if (!bHaveSend)
	{
		//动作优先级大的玩家先发
		for (int j = 0;j < PLAY_COUNT - 1;j++)
		{
			int i = (byIgnoreStation + PLAY_COUNT - 1 - j) % PLAY_COUNT;
			if (m_GameData.T_CPGNotify[i].bCanAction == false/* || m_bNetCut[i]*/)
			{
				continue;
			}

			//优先给能胡牌的玩家发送动作消息 
			if (m_GameData.T_CPGNotify[i].bHu)
			{
				if (bIsHaveHuPai) //已经有人胡，不能再发送碰杠吃
				{
					m_GameData.T_CPGNotify[i].bGang = false;
					m_GameData.T_CPGNotify[i].bPeng = false;
					m_GameData.T_CPGNotify[i].bChi = false;
				}
				SendChiPengGangHuMsg(i);
				byCanActionDesk = i;
				m_GameData.m_byCurrentOperatorUser = byCanActionDesk;
				bHaveSend = true;
				break;
			}
		}
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (m_GameData.T_CPGNotify[i].bCanAction == false/* || m_bNetCut[i]*/)
			{
				continue;
			}
			if (m_GameData.T_CPGNotify[i].bHu)
				iCanHuCount++;
		}
		if (bHaveSend && !m_bTrustee[byCanActionDesk])
		{
			if (IsGoldRoom())
			{
				SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
			}
		}
	}
	//没有发送胡的消息 那么再优先发送杠 或者碰的消息
	if (!bHaveSend && !bIsHaveHuPai)
	{
		//动作优先级大的玩家先发
		for (int j = 0;j < PLAY_COUNT - 1;j++)
		{
			int i = (byIgnoreStation + PLAY_COUNT - 1 - j) % PLAY_COUNT;
			if (m_GameData.T_CPGNotify[i].bCanAction == false/* || m_bNetCut[i]*/)
			{
				continue;
			}
			//再优先给能杠、碰牌的玩家发送动作消息 
			if (m_GameData.T_CPGNotify[i].bGang || m_GameData.T_CPGNotify[i].bPeng)
			{
				SendChiPengGangHuMsg(i);
				byCanActionDesk = i;
				bHaveSend = true;
				m_GameData.m_byCurrentOperatorUser = byCanActionDesk;
				break;
			}
		}
		if (bHaveSend && !m_bTrustee[byCanActionDesk])
		{
			if (IsGoldRoom())
			{
				SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
			}
		}
	}

	//没有发送胡、杠、碰消息 再检查发送吃牌消息
	if (!bHaveSend && !bIsHaveHuPai)
	{
		//动作优先级大的玩家先发
		for (int j = 0;j < PLAY_COUNT;j++)
		{
			int i = (byIgnoreStation + PLAY_COUNT - 1 - j) % PLAY_COUNT;
			if (m_GameData.T_CPGNotify[i].bCanAction == false/* || m_bNetCut[i]*/)
			{
				continue;
			}
			//再优先给能杠、碰牌的玩家发送动作消息 
			if (m_GameData.T_CPGNotify[i].bChi)
			{
				m_GameData.T_CPGNotify[i].bCanAction = true;
				SendChiPengGangHuMsg(i);
				byCanActionDesk = i;
				m_GameData.m_byCurrentOperatorUser = byCanActionDesk;
				bHaveSend = true;
				break;
			}
		}
		if (bHaveSend && !m_bTrustee[byCanActionDesk])
		{
			if (IsGoldRoom())
			{
				SetTimer(TIME_ACTION, m_GameData.m_mjRule.byBlockTime * 1000);
			}
		}
	}
	return bHaveSend;
}

///功能: 发送吃碰杠消息
void CServerGameDesk::SendChiPengGangHuMsg(int iDesk)
{
	if (m_bTrustee[iDesk]) //如果玩家处于托管状态，服务器直接处理
	{
		SetTimer(TIME_TUOGUAN_ACTION, m_GameData.m_mjRule.byAutoOutTime * 1000);
		return;
	}
	CMD_S_OperateNotify ActionData;
	ActionData.byUsr = iDesk;
	ActionData.bChi = m_GameData.T_CPGNotify[iDesk].bChi;
	memcpy(ActionData.byChi, m_GameData.T_CPGNotify[iDesk].byChi, sizeof(ActionData.byChi));
	ActionData.bGang = m_GameData.T_CPGNotify[iDesk].bGang;
	memcpy(ActionData.byGangData, m_GameData.T_CPGNotify[iDesk].iGangData, sizeof(ActionData.byGangData));
	ActionData.bHu = m_GameData.T_CPGNotify[iDesk].bHu;
	ActionData.bPeng = m_GameData.T_CPGNotify[iDesk].bPeng;
	ActionData.byPeng = m_GameData.T_CPGNotify[iDesk].byPeng;
	if (!ActionData.bHu)
	{
		ActionData.bTing = m_GameData.T_CPGNotify[iDesk].bTing;
		memcpy(ActionData.byTingCanOut, m_GameData.T_CPGNotify[iDesk].byTingCanOut, sizeof(ActionData.byTingCanOut));
		memcpy(ActionData.byHuCard, m_GameData.T_CPGNotify[iDesk].byHuCard, sizeof(ActionData.byHuCard));
		memcpy(ActionData.byCardRemainNum, m_GameData.T_CPGNotify[iDesk].byCardRemainNum, sizeof(ActionData.byCardRemainNum));
	}
	SendGameData(iDesk, &ActionData, sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
	//SendWatchData(iDesk,&ActionData,sizeof(ActionData), MSG_MAIN_LOADER_GAME, S_C_OPERATE_NOTIFY, 0);
}

////记录杠牌状态，拥于杠开和杠后炮
void CServerGameDesk::RecordGangStatus()
{
	//记录杠牌状态，拥于杠开和杠后炮
	for (int i = 0;i < PLAY_COUNT; ++i)
	{
		//记录杠上自摸流程，步骤0
		m_GameData.m_bGangZiMo[i] = 0;
		if (i != m_GameData.m_byNowOutStation)
		{
			m_GameData.m_bGangState[i] = false;
		}
	}
}

//json存储,战绩回放
void CServerGameDesk::OnJson(void *pData, int iType)
{
	if (IsGoldRoom())
	{
		return;
	}
	if (1 == iType) //游戏开始
	{
		CMD_S_GameStart *pChangeData = (CMD_S_GameStart *)pData;
		if (!pChangeData)
		{
			return;
		}
		m_root["gameID"] = m_pDataManage->m_InitData.uNameID;
		m_root["bBankerPos"] = m_GameData.m_byNtStation;
		m_root["bMenRestNum"] = pChangeData->iMenNums;
		m_root["bRoomPassward"] = m_szDeskPassWord;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (m_masterID == GetUserIDByDeskStation(i))
			{
				GameUserInfo userData;
				GetUserData(i, userData);
				m_root["bRoomMasterName"] = userData.name;
				break;
			}
		}
		m_root["bCurPlayCount"] = m_iRunGameCount;
		m_root["bAllPlayCount"] = m_iVipGameCount;

		/////房间规则
		m_root["bDaHu"] = m_GameData.m_tGameRuler.bDaHu; //大胡
		m_root["bBaoTing"] = m_GameData.m_tGameRuler.bBaoTing; //报听
		m_root["iZiMoScore"] = m_GameData.m_tGameRuler.iZiMoScore; //自摸分数
		m_root["bZhiZiMo"] = m_GameData.m_tGameRuler.bZhiZiMo; //只自摸
		m_root["bFenPai"] = m_GameData.m_tGameRuler.bFenPai; //是否带风牌
		////////////////

		Json::Value _index;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key[10];
			memset(key, 0, sizeof(key));
			sprintf(key, "%d", i);
			Json::Value _info;
			GameUserInfo userData;
			GetUserData(i, userData);
			_info["nickName"] = Json::Value(userData.name);
			_info["dwUserID"] = Json::Value(GetUserIDByDeskStation(i));
			_info["szHeadUrl"] = Json::Value(userData.headURL);
			_info["iScore"] = Json::Value((int)m_tZongResult.llGameScore[i]);
			_info["sex"] = Json::Value(userData.sex);
			_index[key] = Json::Value(_info);
		}
		m_root["user"] = _index;
		Json::Value array_1;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			char szkey[10];
			memset(szkey, 0, sizeof(szkey));
			sprintf(szkey, "%d", i);

			Json::Value array_2;
			BYTE byCard[HAND_CARD_NUM];
			memset(byCard, 255, sizeof(byCard));
			m_GameData.CopyOneUserHandPai(byCard, i);
			for (int j = 0;j < HAND_CARD_NUM;j++)
			{
				char key_2[10];
				memset(key_2, 0, sizeof(key_2));
				sprintf(key_2, "%d", j);
				array_2[key_2] = Json::Value(byCard[j]);
			}
			array_1[szkey] = Json::Value(array_2);
		}
		m_root["handArrData"] = array_1;
	}
	else if (2 == iType) //出牌
	{
		CMD_S_OutCard *pChangeData = (CMD_S_OutCard *)pData;
		if (!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex, 0, sizeof(szEventIndex));
		sprintf(szEventIndex, "%d", m_iVideoIndex);
		Json::Value element;
		element["eventType"] = Json::Value("out");
		element["bUser"] = Json::Value(pChangeData->byOutCardUser);
		element["cardValue"] = Json::Value(pChangeData->byOutCardData);
		element["bTing"] = Json::Value(pChangeData->bTing);
		element["byOutCardDataIndex"] = Json::Value(pChangeData->byOutCardDataIndex);
		Json::Value array_1;
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			array_1[key_1] = Json::Value(pChangeData->byCardData[i]);
		}
		element["handData"] = Json::Value(array_1);
		m_element[szEventIndex] = Json::Value(element);
		m_iVideoIndex++;
	}
	else if (3 == iType) //抓牌
	{
		CMD_S_SendCard *pChangeData = (CMD_S_SendCard *)pData;
		if (!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex, 0, sizeof(szEventIndex));
		sprintf(szEventIndex, "%d", m_iVideoIndex);
		Json::Value element;
		element["eventType"] = Json::Value("get");
		element["bUser"] = Json::Value(pChangeData->byCurrentUser);
		element["cardValue"] = Json::Value(pChangeData->byCardData);
		element["menpainum"] = Json::Value(pChangeData->iMenNums);
		m_element[szEventIndex] = Json::Value(element);
		m_iVideoIndex++;
	}
	else if (4 == iType) //吃碰杠胡动作
	{
		CMD_S_OperateResult *pChangeData = (CMD_S_OperateResult *)pData;
		if (!pChangeData)
		{
			return;
		}
		char szEventIndex[10];
		memset(szEventIndex, 0, sizeof(szEventIndex));
		sprintf(szEventIndex, "%d", m_iVideoIndex);
		if (ACTION_CHI == pChangeData->byType)
		{
			Json::Value element;
			element["eventType"] = Json::Value("chi");
			element["bUser"] = Json::Value(pChangeData->iStation);
			element["bBeUser"] = Json::Value(pChangeData->iBeStation);
			element["cardValue"] = Json::Value(pChangeData->iOutpai);
			Json::Value array_chiList1;
			for (int i = 0;i < 3;i++)
			{
				char key_1[10];
				memset(key_1, 0, sizeof(key_1));
				sprintf(key_1, "%d", i);
				array_chiList1[key_1] = Json::Value(pChangeData->byData[i]);
			}
			element["chiList"] = Json::Value(array_chiList1);
			Json::Value array_handData1;
			for (int i = 0;i < HAND_CARD_NUM;i++)
			{

				char key_1[10];
				memset(key_1, 0, sizeof(key_1));
				sprintf(key_1, "%d", i);
				array_handData1[key_1] = Json::Value(pChangeData->byCard[i]);
			}
			element["handData"] = Json::Value(array_handData1);
			m_element[szEventIndex] = Json::Value(element);
		}
		else if (ACTION_PENG == pChangeData->byType)
		{
			Json::Value element;
			element["eventType"] = Json::Value("peng");
			element["bUser"] = Json::Value(pChangeData->iStation);
			element["bBeUser"] = Json::Value(pChangeData->iBeStation);
			element["cardValue"] = Json::Value(pChangeData->iOutpai);
			Json::Value array_handData1;
			for (int i = 0;i < HAND_CARD_NUM;i++)
			{

				char key_1[10];
				memset(key_1, 0, sizeof(key_1));
				sprintf(key_1, "%d", i);
				array_handData1[key_1] = Json::Value(pChangeData->byCard[i]);
			}
			element["handData"] = Json::Value(array_handData1);
			m_element[szEventIndex] = Json::Value(element);
		}
		else if (ACTION_HU == pChangeData->byType)
		{
			Json::Value element;
			element["eventType"] = Json::Value("hu");
			element["bUser"] = Json::Value(pChangeData->iStation);
			element["bBeUser"] = Json::Value(pChangeData->iBeStation);
			m_element[szEventIndex] = Json::Value(element);
		}
		else if (ACTION_AN_GANG == pChangeData->byType || ACTION_MING_GANG == pChangeData->byType
			|| ACTION_BU_GANG == pChangeData->byType)
		{
			Json::Value element;
			element["eventType"] = Json::Value("gang");
			element["bUser"] = Json::Value(pChangeData->iStation);
			element["bBeUser"] = Json::Value(pChangeData->iBeStation);
			element["cardValue"] = Json::Value(pChangeData->iOutpai);
			element["gangType"] = Json::Value(pChangeData->byType);
			Json::Value array_handData1;
			for (int i = 0;i < HAND_CARD_NUM;i++)
			{
				char key_1[10];
				memset(key_1, 0, sizeof(key_1));
				sprintf(key_1, "%d", i);
				array_handData1[key_1] = Json::Value(pChangeData->byCard[i]);
			}
			element["handData"] = Json::Value(array_handData1);
			m_element[szEventIndex] = Json::Value(element);
		}
		m_iVideoIndex++;
	}
	else if (5 == iType)
	{
		CMD_S_GameEnd *pChangeData = (CMD_S_GameEnd *)pData;
		if (pChangeData == NULL)
		{
			return;
		}

		char szEventIndex[10];
		memset(szEventIndex, 0, sizeof(szEventIndex));
		sprintf(szEventIndex, "%d", m_iVideoIndex);
		Json::Value element;
		element["eventType"] = Json::Value("GameEnd");
		element["bZimo"] = Json::Value(pChangeData->bZimo);
		element["bIsLiuJu"] = Json::Value(pChangeData->bIsLiuJu);
		//bIsHu
		Json::Value array_bIsHu;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			array_bIsHu[key_1] = Json::Value(pChangeData->bIsHu[i]);
		}
		element["bIsHu"] = Json::Value(array_bIsHu);
		element["byFangPao"] = Json::Value(pChangeData->byFangPao);
		element["byPs"] = Json::Value(pChangeData->byPs);

		//byArHandPai
		Json::Value array_byArHandPai;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			Json::Value array_HandCard;
			for (int j = 0; j < HAND_CARD_NUM;j++)
			{
				char key_2[10];
				memset(key_2, 0, sizeof(key_2));
				sprintf(key_2, "%d", j);
				array_HandCard[key_2] = Json::Value(pChangeData->byArHandPai[i][j]);
			}
			array_byArHandPai[key_1] = Json::Value(array_HandCard);
		}
		element["byArHandPai"] = Json::Value(array_byArHandPai);

		//byArHandPaiCount
		Json::Value array_byArHandPaiCount;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			array_byArHandPaiCount[key_1] = Json::Value(pChangeData->byArHandPaiCount[i]);
		}
		element["byArHandPaiCount"] = Json::Value(array_byArHandPaiCount);

		////iGang
		//Json::Value array_iGang;
		//for(int i=0;i<PLAY_COUNT;i++)
		//{
		//	char key_1[10];
		//	memset(key_1,0,sizeof(key_1));
		//	sprintf(key_1,"%d",i);
		//	array_iGang[key_1]=Json::Value(pChangeData->iGang[i]);
		//}
		//element["iGang"]=Json::Value(array_iGang);

		//llGameScore
		Json::Value array_llGameScore;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			array_llGameScore[key_1] = Json::Value((int)pChangeData->llGameScore[i]);
		}
		element["llGameScore"] = Json::Value(array_llGameScore);

		//UserGCPData
		Json::Value array_UserGCPData;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			Json::Value array_GCPData;
			for (int j = 0; j < 5;j++)
			{
				char key_2[10];
				memset(key_2, 0, sizeof(key_2));
				sprintf(key_2, "%d", j);
				Json::Value tCPGStructMes;
				tCPGStructMes["byType"] = Json::Value(pChangeData->UserGCPData[i][j].byType);
				tCPGStructMes["iStation"] = Json::Value(pChangeData->UserGCPData[i][j].iStation);
				tCPGStructMes["iBeStation"] = Json::Value(pChangeData->UserGCPData[i][j].iBeStation);
				tCPGStructMes["iOutpai"] = Json::Value(pChangeData->UserGCPData[i][j].iOutpai);
				Json::Value _array_byData_;
				for (int k = 0; k < 4; k++)
				{
					char key_3[10];
					memset(key_3, 0, sizeof(key_3));
					sprintf(key_3, "%d", k);
					_array_byData_[key_3] = Json::Value(pChangeData->UserGCPData[i][j].byData[k]);
				}
				tCPGStructMes["byData"] = Json::Value(_array_byData_);
				array_GCPData[key_2] = Json::Value(tCPGStructMes);
			}
			array_UserGCPData[key_1] = Json::Value(array_GCPData);
		}
		element["UserGCPData"] = Json::Value(array_UserGCPData);

		////iFanCount
		//Json::Value array_iFanCount;
		//for(int i=0;i<PLAY_COUNT;i++)
		//{
		//	char key_1[10];
		//	memset(key_1,0,sizeof(key_1));
		//	sprintf(key_1,"%d",i);
		//	array_iFanCount[key_1]=Json::Value(pChangeData->iFanCount[i]);
		//}
		//element["iFanCount"]=Json::Value(array_iFanCount);
		//element["bIsLiuJuDouble"]=Json::Value(pChangeData->bIsLiuJuDouble);

		//byHuType
		Json::Value array_byHuType;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			char key_1[10];
			memset(key_1, 0, sizeof(key_1));
			sprintf(key_1, "%d", i);
			Json::Value array_Type;
			for (int j = 0; j < MAX_HUPAI_TYPE;j++)
			{
				char key_2[10];
				memset(key_2, 0, sizeof(key_2));
				sprintf(key_2, "%d", j);
				array_Type[key_2] = Json::Value(pChangeData->byHuType[i][j]);
			}
			array_byHuType[key_1] = Json::Value(array_Type);
		}
		element["byHuType"] = Json::Value(array_byHuType);

		m_element[szEventIndex] = Json::Value(element);
		m_iVideoIndex++;
	}
}

//配牌器
bool CServerGameDesk::HandCard()
{
	//读取超端数据
	char cFileName[64];
	memset(cFileName, 0, sizeof(cFileName));
	sprintf(cFileName, "%s%d.json", SAVE_JSON_PATH, NAME_ID);
	ifstream fin;
	fin.open(cFileName);
	if (!fin.is_open())
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
	if (0 == iStatus)
	{
		return false;
	}

	//获取手牌数据
	int iCardCount[PLAY_COUNT];
	memset(iCardCount, 0, sizeof(iCardCount));
	int arraySize = root["CardCout"].size();
	arraySize = (arraySize > PLAY_COUNT) ? PLAY_COUNT : arraySize;
	for (int i = 0; i < arraySize; i++)
	{
		char _index[10];
		memset(_index, 0, sizeof(_index));
		sprintf(_index, "%d", i);
		Json::Value valKeyValue = root["CardCout"][i][_index];
		int tempKeyValue = valKeyValue.asInt();
		if ((HAND_CARD_NUM - 1) == tempKeyValue)
		{
			iCardCount[i] = tempKeyValue;
		}
	}

	BYTE byCardData[PLAY_COUNT][HAND_CARD_NUM];
	memset(byCardData, 255, sizeof(byCardData));
	bool bHandFlag[PLAY_COUNT];
	memset(bHandFlag, false, sizeof(bHandFlag));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if ((HAND_CARD_NUM - 1) != iCardCount[i])
		{
			continue;
		}
		char _array[10];
		memset(_array, 0, sizeof(_array));
		sprintf(_array, "%d", i);
		for (int j = 0;j < iCardCount[i];j++)
		{
			char _index[10];
			memset(_index, 0, sizeof(_index));
			sprintf(_index, "%d", j);
			Json::Value valKeyValue = root["CardData"][_array][j][_index];
			int tempKeyValue = valKeyValue.asInt();

			byCardData[i][j] = tempKeyValue;
			bHandFlag[i] = true;
		}
	}

	//获取门牌数据
	int iMCardCount[2];
	memset(iMCardCount, 0, sizeof(iMCardCount));
	int iMSize = root["MCardCout"].size();
	iMSize = (iMSize > 2) ? 2 : iMSize;
	for (int i = 0; i < iMSize; i++)
	{
		char _index[10];
		memset(_index, 0, sizeof(_index));
		sprintf(_index, "%d", i);
		Json::Value valKeyValue = root["MCardCout"][i][_index];
		int tempKeyValue = valKeyValue.asInt();

		iMCardCount[i] = tempKeyValue;
	}

	BYTE byMCardData[2][10];
	memset(byMCardData, 255, sizeof(byMCardData));
	for (int i = 0; i < 2; i++)
	{
		char _array[10];
		memset(_array, 0, sizeof(_array));
		sprintf(_array, "%d", i);
		for (int j = 0;j < iMCardCount[i];j++)
		{
			char _index[10];
			memset(_index, 0, sizeof(_index));
			sprintf(_index, "%d", j);
			Json::Value valKeyValue = root["MCardData"][_array][j][_index];
			int tempKeyValue = valKeyValue.asInt();

			byMCardData[i][j] = tempKeyValue;
		}
	}

	fin.close();

	//把选择的手牌从门牌中删除
	int iChangeNums = 0;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (!bHandFlag[i])
		{
			continue;
		}
		for (int b = 0;b < HAND_CARD_NUM;b++)
		{
			if (255 == byCardData[i][b])
			{
				continue;
			}
			for (int c = 0;c < MAX_MJ_PAI_COUNT;c++)
			{
				if (255 == m_GameData.m_MenPai.byMenPai[c])
				{
					continue;
				}
				if (byCardData[i][b] == m_GameData.m_MenPai.byMenPai[c])
				{
					m_GameData.m_MenPai.byMenPai[c] = 255;
					iChangeNums++;
					break;
				}
			}
		}
	}

	//把选择的门牌从门牌中删除
	for (int i = 0;i < 2;i++)
	{
		for (int j = 0;j < 10;j++)
		{
			if (255 == byMCardData[i][j])
			{
				continue;
			}
			for (int k = 0;k < MAX_MJ_PAI_COUNT;k++)
			{
				if (255 == m_GameData.m_MenPai.byMenPai[k])
				{
					continue;
				}
				if (byMCardData[i][j] == m_GameData.m_MenPai.byMenPai[k])
				{
					m_GameData.m_MenPai.byMenPai[k] = 255;
					break;
				}
			}
		}
	}

	int index = 0;
	BYTE byTempMenPai[MAX_MJ_PAI_COUNT];
	memset(byTempMenPai, 255, sizeof(byTempMenPai));

	for (int i = 0;i < 10;i++)//先放置配置的门牌头的牌
	{
		if (255 != byMCardData[0][i])
		{
			byTempMenPai[index++] = byMCardData[0][i];
		}
	}

	for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
	{
		if (255 != m_GameData.m_MenPai.byMenPai[i])
		{
			byTempMenPai[index++] = m_GameData.m_MenPai.byMenPai[i];
		}
	}

	for (int i = 0;i < 10;i++)//最后放置配置的门牌尾的牌
	{
		if (255 != byMCardData[1][i])
		{
			byTempMenPai[index++] = byMCardData[1][i];
		}
	}

	//配置门牌和索引
	memcpy(m_GameData.m_MenPai.byMenPai, byTempMenPai, sizeof(m_GameData.m_MenPai.byMenPai));
	m_GameData.m_MenPai.byStartIndex = 0;
	m_GameData.m_MenPai.byEndIndex = index - 1;

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (bHandFlag[i])
		{
			for (int j = 0;j < iCardCount[i];j++)
			{
				m_GameData.AddToHandPai(i, byCardData[i][j]);
			}
			m_GameData.SortHandPai(i, false);
		}
		else
		{
			for (int j = 0;j < HAND_CARD_NUM - 1;j++)
			{
				m_GameData.GetPai(i, true, NULL);
			}
			m_GameData.SortHandPai(i, false);
		}
	}

	Json::Value ntKeyValue = root["first"];
	int iNt = ntKeyValue.asInt();
	if (iNt > 0 && iNt <= PLAY_COUNT)
	{
		m_GameData.m_byNtStation = iNt - 1;
	}
	//庄家多拿一张牌
	m_GameData.GetPai(m_GameData.m_byNtStation, true, NULL);
	return true;
}

//获取房卡数据消息
void CServerGameDesk::GetSpecialRule()
{
	if (!m_GameData.m_tGameRuler.bAlreadSet)
	{
		m_GameData.m_tGameRuler.bAlreadSet = true;
		Json::Reader reader;
		Json::Value value;
		if (reader.parse(m_szGameRules, value))
		{
			m_GameData.m_tGameRuler.bDaHu = value["Hplx"].asInt() == 1;
			m_GameData.m_tGameRuler.bBaoTing = value["Wfxz_1"].asInt() == 1;
			m_GameData.m_tGameRuler.iZiMoScore = value["Zmfs"].asInt();
			m_GameData.m_tGameRuler.bFenPai = value["Wfxz_2"].asInt() == 1;
			m_GameData.m_tGameRuler.bZhiZiMo = value["Wfxz_3"].asInt() == 1;
			if (m_GameData.m_tGameRuler.iZiMoScore <= 0)
			{
				m_GameData.m_tGameRuler.iZiMoScore = 3;
			}
		}
		
		INFO_LOG("规则：%s\n====[大胡%d] [报听%d] [风牌%d] [只能自摸胡%d] [自摸分数%d]===",
			m_szGameRules, m_GameData.m_tGameRuler.bDaHu, m_GameData.m_tGameRuler.bBaoTing,
			m_GameData.m_tGameRuler.bFenPai, m_GameData.m_tGameRuler.bZhiZiMo, m_GameData.m_tGameRuler.iZiMoScore);

		//if (IsFriendRoom()) // 金币房设置踢人下限
		//{
		//	SetRemovePlayerMinPoint(8 * GetBasePoint());
		//}

		SetSpecialRule();
	}
}

//根据tagStructGameRuler设置相应规则
void CServerGameDesk::SetSpecialRule()
{
	if (m_GameData.m_tGameRuler.bZhiZiMo)
	{
		m_GameData.m_mjAction.bFangPao = 0;
		m_GameData.m_mjAction.bQiangGangHu = 0;
	}
	else
	{
		m_GameData.m_mjAction.bFangPao = 1;
		m_GameData.m_mjAction.bQiangGangHu = 1;
	}

	if (m_GameData.m_tGameRuler.bFenPai)
	{
		m_GameData.m_mjRule.bHaveHongZhong = 1;			//是否有红中
		m_GameData.m_mjRule.bHaveFaCai = 1;			//是否有发财
		m_GameData.m_mjRule.bHaveBaiBan = 1;			//是否有白板
		m_GameData.m_mjRule.bHaveFengPai = 1;			//是否有东南西北
		m_GameData.m_mjRule.byAllMjCount = 136;
	}
	else
	{
		m_GameData.m_mjRule.bHaveHongZhong = 0;			//是否有红中
		m_GameData.m_mjRule.bHaveFaCai = 0;			//是否有发财
		m_GameData.m_mjRule.bHaveBaiBan = 0;			//是否有白板
		m_GameData.m_mjRule.bHaveFengPai = 0;			//是否有东南西北
		m_GameData.m_mjRule.byAllMjCount = 108;
	}

	if (m_GameData.m_tGameRuler.bBaoTing)
	{
		m_GameData.m_mjAction.bTing = true;
	}
	else
	{
		m_GameData.m_mjAction.bTing = false;
	}

	if (m_GameData.m_tGameRuler.bDaHu)
	{
		m_GameData.m_mjRule.bIsDuoXiang = true;
	}
	else
	{
		m_GameData.m_mjRule.bIsDuoXiang = false;
	}

	memset(m_GameData.m_byCardType, 255, sizeof(m_GameData.m_byCardType));
	int index = 0;
	if (m_GameData.m_mjRule.bHaveWan)
	{
		for (int i = 0;i < 9;i++)
		{
			m_GameData.m_byCardType[index] = stcArr_A_Mj_W[i];
			index++;
		}
	}
	if (m_GameData.m_mjRule.bHaveTiao)
	{
		for (int i = 0;i < 9;i++)
		{
			m_GameData.m_byCardType[index] = stcArr_A_Mj_T[i];
			index++;
		}
	}
	if (m_GameData.m_mjRule.bHaveBing)
	{
		for (int i = 0;i < 9;i++)
		{
			m_GameData.m_byCardType[index] = stcArr_A_Mj_B[i];
			index++;
		}
	}
	if (m_GameData.m_mjRule.bHaveHongZhong)
	{
		m_GameData.m_byCardType[index] = CMjEnum::MJ_TYPE_ZHONG;
		index++;
	}
	if (m_GameData.m_mjRule.bHaveFaCai)
	{
		m_GameData.m_byCardType[index] = CMjEnum::MJ_TYPE_FA;
		index++;
	}
	if (m_GameData.m_mjRule.bHaveBaiBan)
	{
		m_GameData.m_byCardType[index] = CMjEnum::MJ_TYPE_BAI;
		index++;
	}
	if (m_GameData.m_mjRule.bHaveFengPai)
	{
		for (int i = 0;i < 4;i++)
		{
			m_GameData.m_byCardType[index] = stcArr_A_Mj_FENG[i];
			index++;
		}
	}
}

//自动选择一张牌出
BYTE CServerGameDesk::AutoOutCard(BYTE byStation)
{
	BYTE byCard = m_GameData.GetLastHandPai(byStation);
	CLogicBase::SelectSort(m_GameData.m_byArHandPai[byStation], HAND_CARD_NUM, false);
	int iCardCount = m_GameData.m_byArHandPaiCount[byStation]; //玩家的有效手牌数量
	//出这张牌可以听牌
	BYTE byCanTingCard = 255;
	if ((byCanTingCard = m_CheckHuData.GetCanTingCard(&m_GameData, byStation)) != 255)
	{
		return byCanTingCard;
	}

	//顺子判断
	for (int i = iCardCount - 1; i >= 0; i--)
	{
		if (i == 0)
		{
			if (abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i + 1]) > 1
				|| (m_GameData.m_byArHandPai[byStation][i] >= CMjEnum::MJ_TYPE_FD && abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i + 1]) != 0)
				)
			{
				return m_GameData.m_byArHandPai[byStation][i];
			}
		}
		else if (i == iCardCount - 1)
		{
			if (abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i - 1]) > 1
				|| (m_GameData.m_byArHandPai[byStation][i] >= CMjEnum::MJ_TYPE_FD && abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i - 1]) != 0)
				)
			{
				return m_GameData.m_byArHandPai[byStation][i];
			}
		}
		else
		{
			if (abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i + 1]) > 1
				&& abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i - 1]) > 1
				|| (m_GameData.m_byArHandPai[byStation][i] >= CMjEnum::MJ_TYPE_FD
					&& abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i - 1]) != 0
					&& abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i + 1]) != 0)
				)
			{
				return m_GameData.m_byArHandPai[byStation][i];
			}
		}
	}

	return byCard;
}

//自动动作
bool CServerGameDesk::AutoCanAction(BYTE byStation, BYTE byActionType)
{
	if (byActionType == ACTION_PENG && m_GameData.T_CPGNotify[byStation].bPeng)
	{
		CLogicBase::SelectSort(m_GameData.m_byArHandPai[byStation], HAND_CARD_NUM, false);
		int iCardCount = m_GameData.m_byArHandPaiCount[byStation]; //玩家的有效手牌数量
		int iDuiZiCount = 0;
		for (int i = 0; i < iCardCount - 1;)
		{
			if (i < iCardCount - 2)
			{
				if (m_GameData.m_byArHandPai[byStation][i] == m_GameData.m_byArHandPai[byStation][i + 1])
				{
					if (m_GameData.m_byArHandPai[byStation][i] != m_GameData.m_byArHandPai[byStation][i + 2])
					{
						iDuiZiCount++;
					}
					i += 2;
				}
				else
				{
					i++;
				}
			}
			else
			{
				if (m_GameData.m_byArHandPai[byStation][i] == m_GameData.m_byArHandPai[byStation][i + 1])
				{
					iDuiZiCount++;
				}
				break;
			}
		}
		if (iDuiZiCount <= 1)
		{
			return false;
		}
		for (int i = 0; i < iCardCount;i++)
		{
			if (m_GameData.m_byArHandPai[byStation][i] == m_GameData.T_CPGNotify[byStation].byPeng)
			{
				if (i == 0)
				{
					if (abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i + 2]) > 1
						|| m_GameData.m_byArHandPai[byStation][i] >= CMjEnum::MJ_TYPE_FD
						)
					{
						return true;
					}
				}
				else if (i == iCardCount - 1)
				{
					if (abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i - 1]) > 1
						|| m_GameData.m_byArHandPai[byStation][i] >= CMjEnum::MJ_TYPE_FD
						)
					{
						return true;
					}
				}
				else
				{
					if (abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i + 2]) > 1
						&& abs(m_GameData.m_byArHandPai[byStation][i] - m_GameData.m_byArHandPai[byStation][i - 1]) > 1
						|| m_GameData.m_byArHandPai[byStation][i] >= CMjEnum::MJ_TYPE_FD
						)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

//发送出牌出错消息
void CServerGameDesk::SendOutCardError(BYTE byDeskStation, int iErrorCode)
{
	CMD_S_OutCard_Result rsOutCard;
	rsOutCard.bResult = false;
	rsOutCard.byErrorCode = iErrorCode;
	SendGameData(byDeskStation, &rsOutCard, sizeof(rsOutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD_RESULT, 0);
	//SendWatchData(byDeskStation,&rsOutCard,sizeof(rsOutCard), MSG_MAIN_LOADER_GAME, S_C_OUT_CARD_RESULT, 0);
}

//超端玩家请求看门牌
bool CServerGameDesk::SuperUserLookMenPai(BYTE byDeskStation, CMD_C_SuperLook *SuperLook)
{
	if (m_bSuperFlag[byDeskStation] == false)
		return false;
	CMD_S_SuperLookResult SuperLookResult;
	BYTE    m_byRemainPaiDate[MAX_REMAIN_MEN_PAI_NUM];
	memset(m_byRemainPaiDate, 255, sizeof(m_byRemainPaiDate));
	int CardNum = 0;
	CardNum = m_GameData.GetRemainMenPai(m_byRemainPaiDate);

	for (int i = 0;i < 34;++i)
	{
		for (int j = 0;j < CardNum; ++j)
		{
			if (m_byRemainPaiDate[j] == 255)
				continue;
			if (stcArr_A_Mj[i] == m_byRemainPaiDate[j])
			{
				SuperLookResult.byRemainPaiDate[stcArr_A_Mj[i]]++;
			}
		}
	}

	SendGameData(byDeskStation, &SuperLookResult, sizeof(SuperLookResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_REQUEST_LOOK_RESULT, 0);
	return true;
}

//超端玩家请求要牌
bool CServerGameDesk::SuperUserChangePai(BYTE byDeskStation, CMD_C_Super_Request_Card *SuperChangeCard)
{
	CMD_S_Super_Request_Card_Result ChangeCardResult;

	if (m_bSuperFlag[byDeskStation] == false)
	{
		ChangeCardResult.ECode = 1;
		SendGameData(byDeskStation, &ChangeCardResult, sizeof(ChangeCardResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_REQUEST_CRAD_RESULT, 0);
		return false;
	}
	BYTE    m_byRemainPaiDate[MAX_REMAIN_MEN_PAI_NUM];
	memset(m_byRemainPaiDate, 255, sizeof(m_byRemainPaiDate));
	//获得剩余的门牌数据
	m_GameData.GetRemainMenPai(m_byRemainPaiDate);
	//检测是否门牌中是否指定的这张牌
	if (m_GameData.IsHaveAMenPai(SuperChangeCard->CardDate) == false)
	{
		ChangeCardResult.ECode = 2;
		SendGameData(byDeskStation, &ChangeCardResult, sizeof(ChangeCardResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_REQUEST_CRAD_RESULT, 0);
		//重新刷新剩余手牌数据
		CMD_C_SuperLook SuperLook;
		SuperUserLookMenPai(byDeskStation, &SuperLook);
		return false;
	}
	//保存下次该玩家抓的牌
	m_ChangeCard[byDeskStation] = SuperChangeCard->CardDate;
	ChangeCardResult.ECode = 0;
	SendGameData(byDeskStation, &ChangeCardResult, sizeof(ChangeCardResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_REQUEST_CRAD_RESULT, 0);
	return true;
}