#include "ServerManage.h"

#ifndef USER_IDENTITY_TYPE_SUPER
#define USER_IDENTITY_TYPE_SUPER 1
#endif


//构造函数
CServerGameDesk::CServerGameDesk(void) :CGameDesk(ALL_ARGEE)
{
	SetGameStation(GS_WAIT_ARGEE);//游戏状态
	m_iAllCardCount = 52;
	m_byUpGradePeople = 255;
	m_iOutCardPeople = -1;
	m_byLastTurnBanker = 255;
	m_iShangJuQiangZhuang = 1;
	m_i64CallScoreMaxValue = 0;
	ReSetGameState(0);
	m_tZongResult.Init();
	memset(m_iLaserUserOpenShap, 0, sizeof(m_iLaserUserOpenShap));
	memset(m_byLastUserCard, 0, sizeof(m_byLastUserCard));
	memset(m_bMingPai, false, sizeof(m_bMingPai));
	memset(m_bSuperFlag, false, sizeof(m_bSuperFlag));
	memset(m_byLastTurnNote, 1, sizeof(m_byLastTurnNote));// 上一轮玩家下注值 取1,2,3
	m_byLastCallScore = 0;
	m_byLastPlayerCount = 0;
	memset(m_byLastUserCard, 0, sizeof(m_byLastUserCard));
	memset(m_i64LastTurePoint, 0, sizeof(m_i64LastTurePoint));
	memset(m_i64JiaoFenValue, 0, sizeof(m_i64JiaoFenValue));
	memset(m_byCardShapeBase, 1, sizeof(m_byCardShapeBase));
	m_bySuperWinStation = 255;
	m_bySuperFailStation = 255;
	m_bSuperResult = false;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		m_vWinMoney[i].clear();
	}
	m_iAIWantWinMoneyPercent = 1000;
	m_iHumanFailMoneyPercent = 150;
	m_iGiveHumanMoneyPercent = 100;
	m_iRobotGetMoneyPercent = 200;
	m_bSystemWin = true;
	memset(m_bTuoGuan, false, sizeof(m_bTuoGuan));
}

//析构函数
CServerGameDesk::~CServerGameDesk()
{
}

//重置游戏状态和数据
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	memset(m_i64UserNoteLimite, 0, sizeof(m_i64UserNoteLimite));
	SetGameStation(GS_WAIT_ARGEE);
	memset(m_byUserNoteStation, 0, sizeof(m_byUserNoteStation));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_byUserCardCount[i] = 0;
		m_iCallScore[i] = -1;
		m_byOpenUnderCount[i] = 0;
		m_bMingPai[i] = false;
	}
	memset(m_bNetCut, false, sizeof(m_bNetCut));
	// 基本数据
	memset(m_i64PerJuTotalNote, 0, sizeof(m_i64PerJuTotalNote));
	memset(m_byUserCard, 255, sizeof(m_byUserCard));
	memset(m_byUserCardCount, 0, sizeof(m_byUserCardCount));
	memset(m_iTotalCard, 0, sizeof(m_iTotalCard));
	memset(m_byOpenUpCard, 0, sizeof(m_byOpenUpCard));
	memset(m_byBullCard, 0, sizeof(m_byBullCard));
	memset(m_iCallScore, -1, sizeof(m_iCallScore));
	memset(m_bUserReady, false, sizeof(m_bUserReady));
	memset(m_bReadyOpenCard, false, sizeof(m_bReadyOpenCard));
	memset(m_i64UserCallScore, -1, sizeof(m_i64UserCallScore));
	m_bSuperResult = false;

	// 推注数据
	memset(m_byUserCallStation, 255, sizeof(m_byUserCallStation));
	memset(m_byOpenShape, 0, sizeof(m_byOpenShape));
	memset(m_byCurTurnNote, 1, sizeof(m_byCurTurnNote));// 本轮轮玩家下注值,取1,2,3

	// 计时数据
	m_byRemainingTimer = 0;
	m_i64CallScoreMaxValue = 0;
	m_byCurrOperateUser = 255;
	m_bCallScoreFinish = false;
	return true;
}

//初始化
bool CServerGameDesk::InitDeskGameStation()
{
	LoadDynamicConfig();	
	m_bAlreadSet = false;
	return true;
}

void CServerGameDesk::LoadDynamicConfig()
{
	//加载配置文件
	LoadIni();
	//重新加载配置文件里面的
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);
}

//加载ini配置文件
int	CServerGameDesk::LoadIni()
{
	char szfile[256] = "";
	sprintf(szfile, "%s%d_s.ini", CINIFile::GetAppPath().c_str(), NAME_ID);
	CINIFile f(szfile);

	string key = "game";

	m_byBeginTime = f.GetKeyVal(key, "BeginTime", 15);
	m_byThinkTime = f.GetKeyVal(key, "ThinkTime", 15);
	m_byCallScoreTime = f.GetKeyVal(key, "CallScoreTime", 15);
	m_byXiaZhuTime = f.GetKeyVal(key, "XiaZhuTime", 15);
	m_bySendCardTime = f.GetKeyVal(key, "SendCardTime", 15);
	m_iGoodCard = f.GetKeyVal(key, "iGoodCard", 5);
	m_bHaveKing = f.GetKeyVal(key, "HaveKing", 0);				//是否有王
	m_bTurnRule = f.GetKeyVal(key, "TurnRule", 0);
	m_byTuoGuanTime = f.GetKeyVal(key, "TuoGuan", 1);

	//金币场读取房间配置
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD)
	{
		memset(m_byCardShapeBase, 1, sizeof(m_byCardShapeBase));
		RoomBaseInfo * pRoomBaseInfo = ConfigManage()->GetRoomBaseInfo(m_pDataManage->GetRoomID());
		m_tRoomRuler.iBaseScore = pRoomBaseInfo->basePoint > 0 ? pRoomBaseInfo->basePoint : f.GetKeyVal(key, "iBaseScore", 1);
		m_tRoomRuler.byFangZhu = 0;
		m_tRoomRuler.iGameModel = f.GetKeyVal(key, "iGameModel", 4); // 游戏模式
		m_Difen = f.GetKeyVal(key, "Difen", 1);
		if (m_Difen == 1)
		{
			m_tRoomRuler.byBaseScoreLow = 1;
			m_tRoomRuler.byBaseScoreUpper = 2;
		}
		if (m_Difen == 2)
		{
			m_tRoomRuler.byBaseScoreLow = 2;
			m_tRoomRuler.byBaseScoreUpper = 4;
		}
		if (m_Difen == 3)
		{
			m_tRoomRuler.byBaseScoreLow = 4;
			m_tRoomRuler.byBaseScoreUpper = 8;

		}
		if (m_Difen == 4)
		{
			m_tRoomRuler.byBaseScoreLow = 5;
			m_tRoomRuler.byBaseScoreUpper = 10;
		}
		m_Guize = f.GetKeyVal(key, "Guize", 1);
		if (m_Guize == 1)
		{
			m_byCardShapeBase[10] = 4;
			m_byCardShapeBase[9] = 3;
			m_byCardShapeBase[8] = 2;
			m_byCardShapeBase[7] = 2;
			m_tRoomRuler.iGuize = m_Guize;
		}
		if (m_Guize == 2)
		{
			m_byCardShapeBase[10] = 3;
			m_byCardShapeBase[9] = 2;
			m_byCardShapeBase[8] = 2;
			m_tRoomRuler.iGuize = m_Guize;
		}
		m_Tesu_1 = f.GetKeyVal(key, "Tesu_1", 1);
		if (m_Tesu_1 == 0)
		{
			m_tRoomRuler.bHuaNiu = false;
		}
		if (m_Tesu_1 == 1)
		{
			m_tRoomRuler.bHuaNiu = true;
		}
		m_Tesu_2 = f.GetKeyVal(key, "Tesu_2", 1);
		if (m_Tesu_2 == 0)
		{
			m_tRoomRuler.bBombNiu = false;
		}
		if (m_Tesu_2 == 1)
		{
			m_tRoomRuler.bBombNiu = true;
		}
		m_Tesu_3 = f.GetKeyVal(key, "Tesu_3", 1);
		if (m_Tesu_3 == 0)
		{
			m_tRoomRuler.bSmallNiu = false;
		}
		if (m_Tesu_3 == 1)
		{
			m_tRoomRuler.bSmallNiu = true;
		}

		//添加顺子牛，同花牛，葫芦牛
		m_Tesu_4 = f.GetKeyVal(key, "Tesu_4", 1);
		if (m_Tesu_4 == 0)
		{
			m_tRoomRuler.bShunNiu = false;
		}
		if (m_Tesu_4 == 1)
		{
			m_tRoomRuler.bShunNiu = true;
		}
		m_Tesu_5 = f.GetKeyVal(key, "Tesu_5", 1);
		if (m_Tesu_5 == 0)
		{
			m_tRoomRuler.bTongHuaNiu = false;
		}
		if (m_Tesu_5 == 1)
		{
			m_tRoomRuler.bTongHuaNiu = true;
		}
		m_Tesu_6 = f.GetKeyVal(key, "Tesu_6", 1);
		if (m_Tesu_6 == 0)
		{
			m_tRoomRuler.bHuLuNiu = false;
		}
		if (m_Tesu_6 == 1)
		{
			m_tRoomRuler.bHuLuNiu = true;
		}

		m_Tz = f.GetKeyVal(key, "Tz", 1);
		if (m_Tz == 0)
		{
			m_tRoomRuler.bTuiZhu = false;
		}
		if (m_Tz == 1)
		{
			m_tRoomRuler.bTuiZhu = true;
		}
		m_Stopjoin = f.GetKeyVal(key, "Stopjoin", 0);
		if (m_Stopjoin == 0)
		{
			m_tRoomRuler.bStopJoin = false;
		}
		if (m_Stopjoin == 1)
		{
			m_tRoomRuler.bStopJoin = true;
		}
		m_Jzcp = f.GetKeyVal(key, "Jzcp", 0);
		if (m_Jzcp == 0)
		{
			m_tRoomRuler.bStopShuffle = false;
		}
		if (m_Jzcp == 1)
		{
			m_tRoomRuler.bStopShuffle = true;
		}

		if (m_tRoomRuler.iGameModel == GM_PERMANENT_NT)
		{
			m_Shangzhuan = f.GetKeyVal(key, "Shangzhuan", 1);

			if (m_Shangzhuan == 1)
			{
				m_tRoomRuler.iShangZhuangFen = 0;
			}
			if (m_Shangzhuan == 2)
			{
				m_tRoomRuler.iShangZhuangFen = 100 * m_tRoomRuler.byBaseScoreLow;
			}
			if (m_Shangzhuan == 3)
			{
				m_tRoomRuler.iShangZhuangFen = 150 * m_tRoomRuler.byBaseScoreLow;
			}
			if (m_Shangzhuan == 4)
			{
				m_tRoomRuler.iShangZhuangFen = 200 * m_tRoomRuler.byBaseScoreLow;
			}
		}

		if (m_tRoomRuler.iGameModel == GM_SHOW_NT)
		{
			m_i64JiaoFenValue[0] = 0;
			m_Qiangzhuan = f.GetKeyVal(key, "Qiangzhuan", 1);
			if (m_Qiangzhuan == 1)
			{
				m_tRoomRuler.iMaxQiangZhuang = 1;
				m_i64JiaoFenValue[1] = 1;
			}
			if (m_Qiangzhuan == 2)
			{
				m_tRoomRuler.iMaxQiangZhuang = 2;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
			}
			if (m_Qiangzhuan == 3)
			{
				m_tRoomRuler.iMaxQiangZhuang = 3;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
				m_i64JiaoFenValue[3] = 3;
			}
			if (m_Qiangzhuan == 4)
			{
				m_tRoomRuler.iMaxQiangZhuang = 4;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
				m_i64JiaoFenValue[3] = 3;
				m_i64JiaoFenValue[4] = 4;
			}
		}
		// 牌型倍率设置, 注牛将军没有金牛银牛之分, 金牛就是五花牛, 设置游戏规则时:未勾选五花牛的话就只是牛牛牌型

		if (m_tRoomRuler.bHuaNiu == true)
		{
			m_byCardShapeBase[UG_BULL_GOLD] = 5;
		}
		if (m_tRoomRuler.bBombNiu == true)
		{
			m_byCardShapeBase[UG_BULL_BOMB] = 8;
		}

		if (m_tRoomRuler.bSmallNiu == true)
		{
			m_byCardShapeBase[UG_FIVE_SMALL] = 10;
		}


		//新增顺子牛，同花牛，葫芦牛

		if (m_tRoomRuler.bShunNiu == true)
		{
			m_byCardShapeBase[UG_SHUN_BULL] = 5;
		}

		if (m_tRoomRuler.bTongHuaNiu == true)
		{
			m_byCardShapeBase[UG_TONGHUA_BULL] = 6;
		}

		if (m_tRoomRuler.bHuLuNiu == true)
		{
			m_byCardShapeBase[UG_HULU_BULL] = 7;
		}

		m_Logic.SetCardShape(m_tRoomRuler);
	}

	return true;
}

//根据房间ID加载配置文件
int CServerGameDesk::LoadExtIni(int iRoomID)
{
	char szKey[20];
	char nid[20] = "";
	sprintf(nid, "%d", NAME_ID);
	sprintf(szKey, "%s_%d", nid, iRoomID);
	string key = szKey;
	string s = CINIFile::GetAppPath();/////本地路径
	CINIFile f(s + nid + "_s.ini");

	m_byBeginTime = f.GetKeyVal(key, "BeginTime", m_byBeginTime);
	m_byThinkTime = f.GetKeyVal(key, "ThinkTime", m_byThinkTime);
	m_byCallScoreTime = f.GetKeyVal(key, "CallScoretime", m_byCallScoreTime);
	m_byXiaZhuTime = f.GetKeyVal(key, "XiaZhutime", m_byXiaZhuTime);
	m_bySendCardTime = f.GetKeyVal(key, "SendCardTime", m_bySendCardTime);
	m_iGoodCard = f.GetKeyVal(key, "iGoodCard", m_iGoodCard);
	m_bHaveKing = f.GetKeyVal(key, "HaveKing", m_bHaveKing);
	m_bTurnRule = f.GetKeyVal(key, "TurnRule", m_bTurnRule);

	//金币场读取房间配置
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_GOLD)
	{
		m_tRoomRuler.iGameModel = f.GetKeyVal(key, "iGameModel", m_tRoomRuler.iGameModel); // 游戏模式
		m_Difen = f.GetKeyVal(key, "Difen", m_Difen);
		if (m_Difen == 1)
		{
			m_tRoomRuler.byBaseScoreLow = 1;
			m_tRoomRuler.byBaseScoreUpper = 2;
		}
		if (m_Difen == 2)
		{
			m_tRoomRuler.byBaseScoreLow = 2;
			m_tRoomRuler.byBaseScoreUpper = 4;
		}
		if (m_Difen == 3)
		{
			m_tRoomRuler.byBaseScoreLow = 4;
			m_tRoomRuler.byBaseScoreUpper = 8;

		}
		if (m_Difen == 4)
		{
			m_tRoomRuler.byBaseScoreLow = 5;
			m_tRoomRuler.byBaseScoreUpper = 10;
		}
		m_Guize = f.GetKeyVal(key, "Guize", m_Guize);
		if (m_Guize == 1)
		{
			m_byCardShapeBase[10] = 4;
			m_byCardShapeBase[9] = 3;
			m_byCardShapeBase[8] = 2;
			m_byCardShapeBase[7] = 2;
			m_tRoomRuler.iGuize = m_Guize;
		}
		if (m_Guize == 2)
		{
			m_byCardShapeBase[10] = 3;
			m_byCardShapeBase[9] = 2;
			m_byCardShapeBase[8] = 2;
			m_tRoomRuler.iGuize = m_Guize;
		}
		m_Tesu_1 = f.GetKeyVal(key, "Tesu_1", m_Tesu_1);
		if (m_Tesu_1 == 0)
		{
			m_tRoomRuler.bHuaNiu = false;
		}
		if (m_Tesu_1 == 1)
		{
			m_tRoomRuler.bHuaNiu = true;
		}
		m_Tesu_2 = f.GetKeyVal(key, "Tesu_2", m_Tesu_2);
		if (m_Tesu_2 == 0)
		{
			m_tRoomRuler.bBombNiu = false;
		}
		if (m_Tesu_2 == 1)
		{
			m_tRoomRuler.bBombNiu = true;
		}
		m_Tesu_3 = f.GetKeyVal(key, "Tesu_3", m_Tesu_3);
		if (m_Tesu_3 == 0)
		{
			m_tRoomRuler.bSmallNiu = false;
		}
		if (m_Tesu_3 == 1)
		{
			m_tRoomRuler.bSmallNiu = true;
		}

		//新增顺子牛，同花牛，葫芦牛

		m_Tesu_4 = f.GetKeyVal(key, "Tesu_4", m_Tesu_4);
		if (m_Tesu_4 == 0)
		{
			m_tRoomRuler.bShunNiu = false;
		}
		if (m_Tesu_4 == 1)
		{
			m_tRoomRuler.bShunNiu = true;
		}

		m_Tesu_5 = f.GetKeyVal(key, "Tesu_3", m_Tesu_5);
		if (m_Tesu_5 == 0)
		{
			m_tRoomRuler.bTongHuaNiu = false;
		}
		if (m_Tesu_5 == 1)
		{
			m_tRoomRuler.bTongHuaNiu = true;
		}

		m_Tesu_6 = f.GetKeyVal(key, "Tesu_6", m_Tesu_6);
		if (m_Tesu_6 == 0)
		{
			m_tRoomRuler.bHuLuNiu = false;
		}
		if (m_Tesu_6 == 1)
		{
			m_tRoomRuler.bHuLuNiu = true;
		}

		m_Tz = f.GetKeyVal(key, "Tz", m_Tz);
		if (m_Tz == 0)
		{
			m_tRoomRuler.bTuiZhu = false;
		}
		if (m_Tz == 1)
		{
			m_tRoomRuler.bTuiZhu = true;
		}
		m_Stopjoin = f.GetKeyVal(key, "Stopjoin", m_Stopjoin);
		if (m_Stopjoin == 0)
		{
			m_tRoomRuler.bStopJoin = false;
		}
		if (m_Stopjoin == 1)
		{
			m_tRoomRuler.bStopJoin = true;
		}
		m_Jzcp = f.GetKeyVal(key, "Jzcp", m_Jzcp);
		if (m_Jzcp == 0)
		{
			m_tRoomRuler.bStopShuffle = false;
		}
		if (m_Jzcp == 1)
		{
			m_tRoomRuler.bStopShuffle = true;
		}

		if (m_tRoomRuler.iGameModel == GM_PERMANENT_NT)
		{
			m_Shangzhuan = f.GetKeyVal(key, "Shangzhuan", m_Shangzhuan);

			if (m_Shangzhuan == 1)
			{
				m_tRoomRuler.iShangZhuangFen = 0;
			}
			if (m_Shangzhuan == 2)
			{
				m_tRoomRuler.iShangZhuangFen = 100 * m_tRoomRuler.byBaseScoreLow;
			}
			if (m_Shangzhuan == 3)
			{
				m_tRoomRuler.iShangZhuangFen = 150 * m_tRoomRuler.byBaseScoreLow;
			}
			if (m_Shangzhuan == 4)
			{
				m_tRoomRuler.iShangZhuangFen = 200 * m_tRoomRuler.byBaseScoreLow;
			}
		}

		if (m_tRoomRuler.iGameModel == GM_SHOW_NT)
		{
			m_i64JiaoFenValue[0] = 0;
			m_Qiangzhuan = f.GetKeyVal(key, "Qiangzhuan", m_Qiangzhuan);
			if (m_Qiangzhuan == 1)
			{
				m_tRoomRuler.iMaxQiangZhuang = 1;
				m_i64JiaoFenValue[1] = 1;
			}
			if (m_Qiangzhuan == 2)
			{
				m_tRoomRuler.iMaxQiangZhuang = 2;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
			}
			if (m_Qiangzhuan == 3)
			{
				m_tRoomRuler.iMaxQiangZhuang = 3;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
				m_i64JiaoFenValue[3] = 3;
			}
			if (m_Qiangzhuan == 4)
			{
				m_tRoomRuler.iMaxQiangZhuang = 4;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
				m_i64JiaoFenValue[3] = 3;
				m_i64JiaoFenValue[4] = 4;
			}
		}
		// 牌型倍率设置, 注牛将军没有金牛银牛之分, 金牛就是五花牛, 设置游戏规则时:未勾选五花牛的话就只是牛牛牌型
		if (m_tRoomRuler.bHuaNiu == true)
		{
			m_byCardShapeBase[UG_BULL_GOLD] = 5;
		}
		if (m_tRoomRuler.bBombNiu == true)
		{
			m_byCardShapeBase[UG_BULL_BOMB] = 8;
		}

		if (m_tRoomRuler.bSmallNiu == true)
		{
			m_byCardShapeBase[UG_FIVE_SMALL] = 10;
		}

		//新增顺子牛，同花牛，葫芦牛

		if (m_tRoomRuler.bShunNiu == true)
		{
			m_byCardShapeBase[UG_SHUN_BULL] = 5;
		}

		if (m_tRoomRuler.bTongHuaNiu == true)
		{
			m_byCardShapeBase[UG_TONGHUA_BULL] = 6;
		}

		if (m_tRoomRuler.bHuLuNiu == true)
		{
			m_byCardShapeBase[UG_HULU_BULL] = 7;
		}

		m_Logic.SetCardShape(m_tRoomRuler);
	}

	return true;
}

//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	switch (uTimerID)
	{
	case TIME_GAME_TUOGUAN:
	{
		KillTimer(TIME_GAME_TUOGUAN);
		BYTE byStatus = GetGameStation();
		if (byStatus == GS_WAIT_ARGEE)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (m_bTuoGuan[i])
				{
					HandleNotifyMessage(i, MSG_ASS_LOADER_GAME_AGREE, NULL, 0, false);
				}
			}
		}
		else if (byStatus == GS_ROB_NT)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (m_byUserStation[i] != STATE_NULL)
				{
					if (m_byUserCallStation[i] == 255 && m_bTuoGuan[i])
					{
						UserCallScoreResult(i, 0);
					}
				}
			}
		}
		else if (byStatus == GS_NOTE)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUpGradePeople != i && m_bTuoGuan[i])
				{
					if (m_byUserNoteStation[i] == 0 && (m_byUserStation[i] == STATE_PLAY_GAME || m_byUserStation[i] == STATE_CALL_SCORE))
					{
						UserNoteResult(i, 1);
						m_i64PerJuTotalNote[i] = m_i64UserNoteLimite[i][0];
					}
				}
			}
		}
		else if (byStatus == GS_OPEN_CARD)
		{
			for (int i = 0; i < PLAY_COUNT; ++i)
			{
				if (m_byUserStation[i] == STATE_NULL)
				{
					continue;
				}
				if (m_bTuoGuan[i])
				{
					m_bReadyOpenCard[i] = true;
					AutoBull(i);
				}
			}
		}
	}
	return true;
	case TIME_GAME_AUTO_BEGIN:
	{
		KillTimer(TIME_GAME_AUTO_BEGIN);
		if (IsPlayGame(0))
		{
			return true;
		}
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (GetUserIDByDeskStation(i) <= 0 || m_bTuoGuan[i])
			{
				continue;
			}
			HandleNotifyMessage(i, MSG_ASS_LOADER_GAME_AGREE, NULL, 0, false);
		}
	}
	return true;
	case TIME_GAME_COUNT_FINISH:
	{
		KillTimer(TIME_GAME_COUNT_FINISH);
		CGameDesk::GameFinish(0, GF_NORMAL);
	}
	return true;
	case TIME_CALL_NT:	//叫庄
	{
		KillTimer(TIME_CALL_NT);
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (m_byUserStation[i] != STATE_NULL)
			{
				if (m_byUserCallStation[i] == 255)
				{
					// 时间到了默认不叫庄
					UserCallScoreResult(i, 0);
					m_bTuoGuan[i] = true;
					OnHandleUserRequestAuto(i);
				}
			}
		}
		return true;
	}
	case TIME_NOTE:	//下注计时器
	{
		KillTimer(TIME_NOTE);
		if (GetGameStation() == GS_NOTE)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUpGradePeople != i)
				{
					if (m_byUserNoteStation[i] == 0 && (m_byUserStation[i] == STATE_PLAY_GAME || m_byUserStation[i] == STATE_CALL_SCORE))
					{
						// 时间到了默认下注值为最小下注值
						UserNoteResult(i, 1);
						m_i64PerJuTotalNote[i] = m_i64UserNoteLimite[i][0];
						m_bTuoGuan[i] = true;
						OnHandleUserRequestAuto(i);
					}
				}
			}
		}
		return true;
	}
	case TIME_NOTE_FINISH:	//下注完成定时器
	{
		KillTimer(TIME_NOTE_FINISH);
		if (GetGameStation() == GS_NOTE)
		{
			if (m_tRoomRuler.iGameModel != GM_SHOW_NT)
			{
				SendCard();
			}
			else
			{
				SendLastCard();
			}
		}
		return true;
	}
	case TIME_SEND_CARD_FINISH:	//发牌定时器
	{
		KillTimer(TIME_SEND_CARD_FINISH);
		if (GetGameStation() == GS_SEND_CARD)
		{
			SendCardFinish();
		}
		return true;
	}
	case TIME_AUTO_BULL:	//自动摆牛计时器
	{
		KillTimer(TIME_AUTO_BULL);       // 到时自动摆牛
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}
			if (!m_bTuoGuan[i] && m_byUserStation[i] != STATE_OPEN_CARD)
			{
				m_bReadyOpenCard[i] = true;
				AutoBull(i);
			}
		}
		return true;
	}
	case TIME_GAME_FINISH:	//结束定时器
	{
		KillTimer(TIME_GAME_FINISH);
		GameFinish(0, GF_NORMAL);
		return true;
	}
	case TIME_RAND_BANKER:       // 随机庄
	{
		KillTimer(TIME_RAND_BANKER);
		CallScoreFinish();
		return true;
	}
	case TIME_REMAIN_TIMER:       // 用于计时的计时器
	{
		if (GetGameStation() != GS_WAIT_NEXT && GetGameStation() != GS_WAIT_ARGEE)
		{
			if (m_byRemainingTimer > 0)
			{
				--m_byRemainingTimer;
			}
			return true;
		}
		else
		{
			return true;
		}
	}
	}

	return CGameDesk::OnTimer(uTimerID);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser/* = false*/)
{
	if (bWatchUser || bDeskStation >= PLAY_COUNT)
	{
		return true;
	}
	switch (assistID)
	{
	case MSG_ASS_LOADER_GAME_AGREE:		//用户同意游戏
	{
		m_bUserReady[bDeskStation] = true;
		if (IsPlayGame(bDeskStation))
		{
			return true;
		}
		return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
	}
	case C_S_SUPER_USER:        // 超端玩家设置消息
	{
		SAFECHECK_STRUCT(pSuperUserState, C_S_SuperUserState, pData, size);
		if (1 != m_bSuperFlag[bDeskStation])
		{
			return true;
		}
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (pSuperUserState->byOperatorValue[i] == 1)
			{
				m_bySuperWinStation = i;
			}
			else if (pSuperUserState->byOperatorValue[i] == 0)
			{
				m_bySuperFailStation = i;
			}
		}
		// 返回超端设置结果
		// 记录超端数据
		// 尝试立即换牌，但是会失败
		return SuperSetResult(bDeskStation);
	}
	case C_S_USER_BET:	//玩家下注
	{
		if (STATE_HAVE_NOTE == m_byUserStation[bDeskStation] || STATE_NULL == m_byUserStation[bDeskStation] || GS_NOTE != GetGameStation())
		{
			return true;
		}
		SAFECHECK_STRUCT(pBetValue, C_S_UserBetValue, pData, size)
			return UserNoteResult(bDeskStation, pBetValue->byBetBtnNum);
	}
	case C_S_USER_OPER:    // 玩家摆牌状态
	{
		if (GetGameStation() != GS_OPEN_CARD)
		{
			return true;
		}
		if (m_byUserStation[bDeskStation] == STATE_NULL)
		{
			return true;
		}
		SAFECHECK_STRUCT(pUserOpenCard, C_S_UserOpenCard, pData, size)
			m_bReadyOpenCard[bDeskStation] = true;          // 牛将军这里是庄家最后亮牌, 如果不需要庄家最后亮牌, 删除所有包含m_bReadyOpenCard相关的内容
		AutoBull(bDeskStation);
		m_bMingPai[bDeskStation] = true;
		return true;
	}
	case C_S_CALL_BANKER:	//玩家叫分
	{
		m_byCurrOperateUser = bDeskStation;
		SAFECHECK_STRUCT(pUserCallBanker, C_S_UserCallBanker, pData, size)
			return UserCallScoreResult(bDeskStation, pUserCallBanker->byValue);
	}
	case C_S_CARDSHAPE:  // 玩家请求得到当前牌型
	{
		SAFECHECK_STRUCT(pCardShape, C_S_GetCardShape, pData, size);
		return GetUserCardShape(bDeskStation);
	}
	}
	return CGameDesk::HandleNotifyMessage(bDeskStation, assistID, pData, size, bWatchUser);
}

//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	int roomType = GetRoomType();
	if (roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_FG_VIP)
	{
		SetGameRuler();
	}

	if (bDeskStation < PLAY_COUNT)
	{
		m_bNetCut[bDeskStation] = false;
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
	GameStation_Base tBaseData;
	//游戏版本核对
	//游戏版本核对
	tBaseData.iVersion = DEV_HEIGHT_VERSION;			//游戏版本号
	tBaseData.iVersion2 = DEV_LOW_VERSION;
	//游戏基础信息
	tBaseData.byGameStation = GetGameStation();		//游戏状态

	// 恢复当前游戏时间
	tBaseData.byThinkTime = m_byThinkTime;			//思考时间就是出牌时间
	tBaseData.byBeginTime = m_byBeginTime;			//准备时间
	tBaseData.bySendCardTime = m_bySendCardTime;		//剩余发牌时间
	tBaseData.byCuoPaiTime = m_byThinkTime;        // 剩余搓牌时间
	tBaseData.byCallScoreTime = m_byCallScoreTime;	//剩余叫庄时间
	tBaseData.byXiaZhuTime = m_byXiaZhuTime;		//剩余下注时间
	tBaseData.byRemainingTimer = m_byRemainingTimer;
	tBaseData.byAllCardCount = 52;
	memcpy(&tBaseData.tRoomRuler, &m_tRoomRuler, sizeof(tBaseData.tRoomRuler));
	tBaseData.tRoomRuler.byFangZhu = 0;            // 房主
	// 检测是否存在超端部分
	memcpy(tBaseData.bIsSuperUser, m_bSuperFlag, sizeof(tBaseData.bIsSuperUser));

	switch (GetGameStation())
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
	case GS_WAIT_NEXT:			//等待下一盘游戏开始
	{
		GameStation_WaiteAgree TGameStation;
		::memcpy(&TGameStation, &tBaseData, sizeof(GameStation_Base));
		::memcpy(TGameStation.bUserReady, m_bUserReady, sizeof(TGameStation.bUserReady));

		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}

	case GS_ROB_NT:		//抢庄状态
	{
		GameStation_RobNt TGameStation;
		::memcpy(&TGameStation, &tBaseData, sizeof(GameStation_Base));
		::memcpy(TGameStation.i64UserCallScore, m_i64UserCallScore, sizeof(TGameStation.i64UserCallScore));
		::memcpy(TGameStation.byUserCallStation, m_byUserCallStation, sizeof(TGameStation.byUserCallStation));

		// 根据游戏模式设置玩家手牌
		if (GM_SHOW_NT == m_tRoomRuler.iGameModel)
		{
			for (int i = 0; i < PLAY_COUNT; ++i)
			{
				if (m_byUserStation[i] == STATE_NULL)
				{
					continue;
				}
				for (int j = 0; j < SH_USER_CARD - 1; ++j)
				{
					TGameStation.byUserCard[i][j] = m_byUserCard[i][j];
				}
				TGameStation.byUserCardCount[i] = SH_USER_CARD - 1;
			}
		}
		else
		{
			memcpy(TGameStation.byUserCardCount, m_byUserCardCount, sizeof(TGameStation.byUserCardCount));
		}
		//中途加入玩家不能抢庄
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				TGameStation.byUserCallStation[i] = 1;
			}
		}
		memcpy(TGameStation.byUserStation, m_byUserStation, sizeof(TGameStation.byUserStation));
		//发送数据
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}
	case GS_NOTE:		//下注状态
	{
		GameStation_Note TGameStation;
		::memcpy(&TGameStation, &tBaseData, sizeof(GameStation_Base));
		TGameStation.byBankerCallValue = m_i64CallScoreMaxValue;
		TGameStation.byNtStation = m_byUpGradePeople;				//庄家位置
		::memcpy(TGameStation.byUserNoteStation, m_byUserNoteStation, sizeof(TGameStation.byUserNoteStation));
		::memcpy(TGameStation.i64UserNoteLimite, m_i64UserNoteLimite, sizeof(TGameStation.i64UserNoteLimite));

		// 得到玩家的下注值
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (i != m_byUpGradePeople)
			{
				TGameStation.i64PerJuTotalNote[i] = m_i64PerJuTotalNote[i];
			}
			// 避免断线重连时推注值与下注值一致
			if (TGameStation.i64UserNoteLimite[i][1] == TGameStation.i64UserNoteLimite[i][2])
			{
				TGameStation.i64UserNoteLimite[i][2] = 0;
			}

			long userID = GetUserIDByDeskStation(i);
			if (userID <= 0)
			{
				TGameStation.iUserStation[i] = 0;
			}
			else
			{
				TGameStation.iUserStation[i] = 1;
			}
		}

		if (GM_SHOW_NT == m_tRoomRuler.iGameModel)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUserStation[i] == STATE_NULL)
				{
					continue;
				}
				if (i != m_byUpGradePeople)
				{
					TGameStation.i64PerJuTotalNote[i] = m_i64PerJuTotalNote[i];			//用户每下注情况
				}
				for (int j = 0; j < SH_USER_CARD - 1; ++j)
				{
					TGameStation.byUserCard[i][j] = m_byUserCard[i][j];
				}
				TGameStation.byUserCardCount[i] = SH_USER_CARD - 1;
			}
		}
		else
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (i != m_byUpGradePeople)
				{
					TGameStation.i64PerJuTotalNote[i] = m_i64PerJuTotalNote[i];			//用户每下注情况
				}
				for (int j = 0; j < SH_USER_CARD; ++j)
				{
					TGameStation.byUserCard[i][j] = m_byUserCard[i][j];
				}
				TGameStation.byUserCardCount[i] = m_byUserCardCount[i];
			}
		}
		//中途加入玩家不能下注
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				TGameStation.byUserNoteStation[i] = 1;
			}
		}
		//发送数据
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}
	case GS_SEND_CARD:
	{
		GameStation_SendCard TGameStation;
		::memcpy(&TGameStation, &tBaseData, sizeof(GameStation_Base));
		TGameStation.byBankerCallValue = m_i64CallScoreMaxValue;
		TGameStation.byNtStation = m_byUpGradePeople;		//庄家位置
		if (bDeskStation < PLAY_COUNT)
		{
			//用户手上的扑克
			::memcpy(TGameStation.byUserCard[bDeskStation], m_byUserCard[bDeskStation], sizeof(TGameStation.byUserCard[bDeskStation]));
		}
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			TGameStation.byUserCardCount[i] = m_byUserCardCount[i];			//用户手上扑克数目
			TGameStation.i64PerJuTotalNote[i] = m_i64PerJuTotalNote[i];			//用户每下注情况
			//个玩家的状态 标记是否中途加入的
			TGameStation.iUserStation[i] = m_byUserStation[i];
		}
		//发送数据
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}
	case GS_OPEN_CARD:	//游戏中状态(开牌状态)
	{
		GameStation_OpenCard TGameStation;
		::memcpy(&TGameStation, &tBaseData, sizeof(GameStation_Base));
		TGameStation.byBankerCallValue = m_i64CallScoreMaxValue;
		TGameStation.byNtStation = m_byUpGradePeople;		//庄家位置
		//用户手中牌
		::memcpy(TGameStation.byUserCardCount, m_byUserCardCount, sizeof(TGameStation.byUserCardCount));
		if (bDeskStation < PLAY_COUNT)
		{
			::memcpy(TGameStation.byUserCard[bDeskStation], m_byUserCard[bDeskStation], sizeof(TGameStation.byUserCard[bDeskStation]));
		}
		//当前各用户下注总数
		::memcpy(TGameStation.i64PerJuTotalNote, m_i64PerJuTotalNote, sizeof(TGameStation.i64PerJuTotalNote));
		::memcpy(TGameStation.bMingPai, m_bMingPai, sizeof(TGameStation.bMingPai));

		//玩家摆牛数据
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			//个玩家的状态 标记是否中途加入的
			TGameStation.iUserStation[i] = m_byUserStation[i];
		}

		::memcpy(TGameStation.byOpenUnderCount, m_byOpenUnderCount, sizeof(m_byOpenUnderCount));
		::memcpy(TGameStation.byOpenUnderCard, m_byBullCard, sizeof(TGameStation.byOpenUnderCard));
		::memcpy(TGameStation.byOpenUpCard, m_byOpenUpCard, sizeof(TGameStation.byOpenUpCard));
		::memcpy(TGameStation.bReadyOpenCard, m_bReadyOpenCard, sizeof(TGameStation.bReadyOpenCard));

		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			TGameStation.iOpenShape[i] = m_byOpenShape[i];
		}
		//发送数据
		SendGameStation(bDeskStation, uSocketID, bWatchUser, &TGameStation, sizeof(TGameStation));
		return true;
	}
	}

	return false;
}

//游戏开始
bool CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (CGameDesk::GameBegin(bBeginFlag) == false)
	{
		GameFinish(0, GF_SALE);
		return false;
	}
	//初始奖值数据
	memset(m_byRobot, 0, sizeof(m_byRobot));
	memset(m_byHuman, 0, sizeof(m_byHuman));
	memset(m_bIsRobot, false, sizeof(m_bIsRobot));
	m_iRobotCount = 0;
	m_iHumanCount = 0;
	m_byRobotDesk = 255;	//随便找一个机器人的位置
	m_byUserDesk = 255;		//随便找一个真实玩家的位置
	memset(m_byUserStation, STATE_NULL, sizeof(m_byUserStation));
	memset(m_bNetCut, false, sizeof(m_bNetCut));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		long userID = GetUserIDByDeskStation(i);
		if (userID > 0)
		{
			m_byUserStation[i] = STATE_PLAY_GAME;
			GameUserInfo * pUser = m_pDataManage->GetUser(userID);
			if (pUser == NULL)
			{
				continue;
			}
			if (pUser->isVirtual)
			{
				m_byRobot[m_iRobotCount++] = i;
				m_bIsRobot[i] = true;
			}
			else
			{
				m_byHuman[m_iHumanCount++] = i;
			}
		}
	}
	//无真人本局游戏结束
	if (m_iHumanCount <= 0)
	{
		GameFinish(0, GF_SALE);
		return false;
	}

	KillTimer(TIME_GAME_AUTO_BEGIN);
	SetGameStation(GS_ROB_NT);

	//分发扑克
	m_Logic.RandCard(m_iTotalCard, m_iAllCardCount, m_bHaveKing);

	//配牌
	BrandCard();

	//发送游戏开始消息
	S_C_BeginPlayStruct Begin;
	Begin.iOutDeskStation = m_iOutCardPeople;

	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		GameUserInfo user;
		if (GetUserData(i, user) && (user.userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER)
		{
			m_bSuperFlag[i] = true;
			Begin.bIsSuperUser[i] = true;
		}
	}

	for (int i = 0;i < PLAY_COUNT; i++)
	{
		SendGameData(i, &Begin, sizeof(Begin), MSG_MAIN_LOADER_GAME, S_C_GAME_PLAY, 0);
	}

	memset(Begin.bIsSuperUser, 0, sizeof(Begin.bIsSuperUser));
	SendWatchData(&Begin, sizeof(Begin), MSG_MAIN_LOADER_GAME, S_C_GAME_PLAY, 0);
	// 这里的SendWatchData函数针对存在旁观的游戏, 这里无法更改成支持不存在旁观的功能, 需要平台支持

	srand((unsigned)(time(NULL)));
	if (GM_NIUNIU_NT == m_tRoomRuler.iGameModel)              // 牛牛上庄模式
	{
		int roomType = GetRoomType();
		if (m_iRunGameCount == 1 && (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP)
			|| m_byUpGradePeople == 255 || m_byUserStation[m_byUpGradePeople] == STATE_NULL)
		{
			m_vecIntTemp.clear();
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUserStation[i] != STATE_NULL)
				{
					m_vecIntTemp.push_back(i);
				}
			}
			m_byUpGradePeople = m_vecIntTemp[CUtil::GetRandNum() % m_vecIntTemp.size()];
		}
		NoticeUserNote();
	}

	if (GM_PERMANENT_NT == m_tRoomRuler.iGameModel)				// 固定庄家模式
	{
		bool bHaveyUpGrade = false;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			long userID = GetUserIDByDeskStation(i);
			if (userID > 0 && userID == m_masterID)
			{
				m_byUpGradePeople = i;
				bHaveyUpGrade = true;
				break;
			}
		}

		//这种情况是代开时会出现(房主不时来玩时的)
		if (!bHaveyUpGrade)
		{
			m_vecIntTemp.clear();
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUserStation[i] != STATE_NULL)
				{
					m_vecIntTemp.push_back(i);
				}
			}
			m_byUpGradePeople = m_vecIntTemp[CUtil::GetRandNum() % m_vecIntTemp.size()];
		}
		NoticeUserNote();
	}

	if (GM_SHOW_NT == m_tRoomRuler.iGameModel)                // 明牌抢庄
	{
		SendCard();
	}

	if (GM_FREE_NT == m_tRoomRuler.iGameModel)                // 自由抢庄
	{
		NoticeUserScore();
	}

	if (GM_ONE_WIN == m_tRoomRuler.iGameModel)                 // 通比牛牛
	{
		NoticeUserNote();
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_byUserStation[i] != STATE_NULL)
			{
				UserNoteResult(i, 1);
			}
		}
	}
	SetTimer(TIME_REMAIN_TIMER, 1000);
	return true;
}

//发送扑克给用户
int	CServerGameDesk::SendCard()
{
	KillTimer(TIME_NOTE_FINISH);
	// 根据不同的游戏模式进入不同的游戏状态
	SetGameStation(GS_SEND_CARD);

	::memset(m_byUserCardCount, 0, sizeof(m_byUserCardCount));
	::memset(m_byUserCard, 0, sizeof(m_byUserCard));

	//分发扑克牌 
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}
		m_byUserCardCount[i] = (m_tRoomRuler.iGameModel == GM_SHOW_NT) ? SH_USER_CARD - 1 : SH_USER_CARD;
		for (int j = 0; j < SH_USER_CARD; j++)
		{
			m_byUserCard[i][j] = m_iTotalCard[i * SH_USER_CARD + j];
		}
	}

	//////////////////////////////奖值////////////////////////////////////////////
	//执行奖值结果
	//机器人奖池控制
	//获取平台奖池数据，确定本局控制概率
	bool bRet = true;
	if (m_bySuperWinStation == 255 && m_bySuperFailStation == 255 && m_pDataManage->LoadPoolData())
	{
		if (m_iRobotCount > 0 && GM_SHOW_NT != m_tRoomRuler.iGameModel)
		{
			bRet = AiWinAutoCtrl();
		}
		m_iHumanFailMoneyPercent = m_pDataManage->m_rewardsPoolInfo.realPeopleFailPercent;
		m_iGiveHumanMoneyPercent = m_pDataManage->m_rewardsPoolInfo.realPeopleWinPercent;
		if (m_iRobotCount == 0 || !bRet) //真人之间对战
		{
			//个人控制
			AutoRecoverMoney();
			AutoGiveMoney();
		}
	}

	//////////////////////////////超端///////////////////////////////////////////
	//执行上一局超端设置
	SuperSetResult(255);

	S_C_SendCard tSendCardDdata;

	if (255 != m_byUpGradePeople)
	{
		tSendCardDdata.byStartPos = m_byUpGradePeople;
	}
	else
	{
		int iRand = CUtil::GetRandNum() % PLAY_COUNT;
		for (int i = iRand;i < iRand + PLAY_COUNT;i++)
		{
			tSendCardDdata.byStartPos = i % PLAY_COUNT;
			if (m_byUserStation[i] != STATE_NULL)
			{
				break;
			}
		}
	}

	::memcpy(tSendCardDdata.byCardCount, m_byUserCardCount, sizeof(tSendCardDdata.byCardCount));

	for (int i = 0;i < PLAY_COUNT; i++)
	{
		if (m_byUserStation[i] != STATE_NULL)
		{
			memcpy(tSendCardDdata.byCardData[i], m_byUserCard[i], sizeof(BYTE) * m_byUserCardCount[i]);

			SendGameData(i, &tSendCardDdata, sizeof(S_C_SendCard), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
			SendWatchData(&tSendCardDdata, sizeof(tSendCardDdata), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
			::memset(tSendCardDdata.byCardData[i], 0, sizeof(tSendCardDdata.byCardData[i]));
		}
	}

	if (m_tRoomRuler.iGameModel == GM_SHOW_NT)
	{
		// 通知用户叫庄

		//平台相关代码
		SendWatchData(&tSendCardDdata, sizeof(tSendCardDdata), MSG_MAIN_LOADER_GAME, S_C_SEND_CARD, 0);
		NoticeUserScore();
		return true;
	}
	SetTimer(TIME_SEND_CARD_FINISH, 400);
	return true;
}

//通知用户叫庄
int CServerGameDesk::NoticeUserScore()
{
	SetGameStation(GS_ROB_NT);
	if (m_tRoomRuler.iGameModel == GM_SHOW_NT || m_tRoomRuler.iGameModel == GM_FREE_NT)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_NULL == m_byUserStation[i])
			{
				continue;
			}
			// 设置每个玩家的叫庄分数
			if (m_tRoomRuler.iGameModel == GM_FREE_NT)
			{
				m_tRoomRuler.iMaxQiangZhuang = 1;
			}
			for (int j = 0; j < 1 + m_tRoomRuler.iMaxQiangZhuang; ++j)
			{
				m_i64CallValue[i][j] = j;
			}
		}

		S_C_NoticCallBanker    tNoticeCallBanker;
		tNoticeCallBanker.byCallUser = m_byCurrOperateUser;

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (STATE_NULL != m_byUserStation[i])
			{
				tNoticeCallBanker.byCallUser = i;
				SendGameData(i, &tNoticeCallBanker, sizeof(S_C_NoticCallBanker), MSG_MAIN_LOADER_GAME, S_C_CALL_BANKER, 0);
			}
		}

		//平台相关代码
		SendWatchData(&tNoticeCallBanker, sizeof(tNoticeCallBanker), MSG_MAIN_LOADER_GAME, S_C_CALL_BANKER, 0);

		SetTimer(TIME_CALL_NT, (m_byCallScoreTime + 1) * 1000);
		SetTimer(TIME_GAME_TUOGUAN, m_byTuoGuanTime * 1000 + 500);
		m_byRemainingTimer = m_byCallScoreTime;
	}
	return true;
}

//玩家叫庄
int CServerGameDesk::UserCallScoreResult(BYTE bDeskStation, BYTE byBetBtnNum)
{
	//用户状态错误，不能叫庄
	if (m_byUserStation[bDeskStation] == STATE_NULL || byBetBtnNum >= JIAO_FEN_COUNT)
	{
		ERROR_LOG("叫庄错误%d\n", byBetBtnNum);
		return true;
	}

	if (byBetBtnNum > m_tRoomRuler.iMaxQiangZhuang)
	{
		ERROR_LOG("叫庄错误%d,m_tRoomRuler.iMaxQiangZhuang = %d\n", byBetBtnNum, m_tRoomRuler.iMaxQiangZhuang);
		return true;
	}

	//叫过庄，不能再叫
	if (m_byUserCallStation[bDeskStation] != 255)
	{
		return true;
	}
	
	m_byUserStation[bDeskStation] = STATE_CALL_SCORE;
	m_i64UserCallScore[bDeskStation] = m_i64CallValue[bDeskStation][byBetBtnNum];
	if (byBetBtnNum == 0)
	{
		m_byUserCallStation[bDeskStation] = 0;
	}
	if (byBetBtnNum == 1 || byBetBtnNum == 2 || byBetBtnNum == 3 || byBetBtnNum == 4)
	{
		m_byUserCallStation[bDeskStation] = 1;
	}

	S_C_UserCallBankerResult tUserCallBankerResult;
	tUserCallBankerResult.byCallBankerUser = bDeskStation;
	tUserCallBankerResult.byValue = m_i64CallValue[bDeskStation][byBetBtnNum];
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}
		SendGameData(i, &tUserCallBankerResult, sizeof(S_C_UserCallBankerResult), MSG_MAIN_LOADER_GAME, S_C_CALL_BANKER_RESULT, 0);
	}

	SendWatchData(&tUserCallBankerResult, sizeof(S_C_UserCallBankerResult), MSG_MAIN_LOADER_GAME, S_C_CALL_BANKER_RESULT, 0);

	if (0 == CountNoCallScorePeople())
	{
		KillTimer(TIME_CALL_NT);

		int iMaxCallScore = -2;
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_i64UserCallScore[i] > iMaxCallScore)
			{
				iMaxCallScore = m_i64UserCallScore[i];
			}
		}
		m_i64CallScoreMaxValue = iMaxCallScore;
		if (m_i64CallScoreMaxValue == -2)
		{
			ERROR_LOG("牛牛抢庄分数异常m_i64CallScoreMaxValue = %d", m_i64CallScoreMaxValue);
		}
		S_C_NoticRandSelectBanker tNoticRandSelectBanker;
		memset(tNoticRandSelectBanker.bRandSelectUser, false, sizeof(tNoticRandSelectBanker.bRandSelectUser));

		BYTE byUpGradeRand[PLAY_COUNT];
		memset(byUpGradeRand, 0, sizeof(byUpGradeRand));
		int byUpGradeRandCount = 0;

		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_i64CallScoreMaxValue == m_i64UserCallScore[i])
			{
				tNoticRandSelectBanker.bRandSelectUser[i] = true;
				byUpGradeRand[byUpGradeRandCount++] = i;
			}
		}
		srand((unsigned)(time(NULL)));
		if (GM_SHOW_NT == m_tRoomRuler.iGameModel && m_byRobotDesk != 255)
		{
			m_byUpGradePeople = m_byRobotDesk;
		}
		else
		{
			if (byUpGradeRandCount <= 0)
			{
				for (int i = 0; i < PLAY_COUNT; ++i)
				{
					if (m_byUserStation[i] != STATE_NULL)
					{
						m_byUpGradePeople = i;
						break;
					}
				}
				ERROR_LOG("抢庄异常，选第一个玩家为庄家 m_byUpGradePeople = %d", m_byUpGradePeople);
			}
			else
			{
				m_byUpGradePeople = byUpGradeRand[CUtil::GetRandNum() % byUpGradeRandCount];
			}
		}
		tNoticRandSelectBanker.byBanker = m_byUpGradePeople;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}
			SendGameData(i, &tNoticRandSelectBanker, sizeof(S_C_NoticRandSelectBanker), MSG_MAIN_LOADER_GAME, S_C_NOTIC_RAND_SELECT_BANKER, 0);
		}

		SetTimer(TIME_RAND_BANKER, 500 * CountPlayer());
	}
	return true;
}

//叫庄结束
int	CServerGameDesk::CallScoreFinish()
{
	NoticeUserNote();
	return true;
}

//获取推注值
//iQiang抢庄，iBaseScore上一局下注底分，iCardShape上一局牌型，iReserve预留
void CServerGameDesk::GetTuiZhu(long long &iTuiZhuScore, long long &iTuiZhuScoreEx, int iQiang, long long iBaseScore, BYTE iCardShape, int iReserve)
{
	if (iQiang == 1)
	{
		if (iCardShape >= UG_BULL_NINE)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 3;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 5;
			}
		}
		else if (iCardShape >= UG_BULL_SEVEN)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 3;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 5;
			}
		}
		else
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = 0;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 3;
			}
		}
	}
	else if (iQiang == 2)
	{
		if (iCardShape >= UG_BULL_NINE)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 5;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 6;
			}
		}
		else if (iCardShape >= UG_BULL_SEVEN)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 5;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 6;
			}
		}
		else
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 3;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 5;
			}
		}
	}
	else if (iQiang == 3)
	{
		if (iCardShape >= UG_BULL_NINE)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 10;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 16;
			}
		}
		else if (iCardShape >= UG_BULL_SEVEN)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 6;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 9;
			}
		}
		else
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 4;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 6;
			}
		}
	}
	else if (iQiang == 4)
	{
		if (iCardShape >= UG_BULL_NINE)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 16;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 16;
			}
		}
		else if (iCardShape >= UG_BULL_SEVEN)
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 10;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 10;
			}
		}
		else
		{
			if (iBaseScore == m_tRoomRuler.byBaseScoreLow)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 5;
			}
			else if (iBaseScore == m_tRoomRuler.byBaseScoreUpper)
			{
				iTuiZhuScore = m_tRoomRuler.byBaseScoreLow * 10;
			}
		}
	}
}

//通知用户下注
int	CServerGameDesk::NoticeUserNote()
{
	SetGameStation(GS_NOTE);
	//清空相关数据
	memset(m_i64UserNoteLimite, 0, sizeof(m_i64UserNoteLimite));
	memset(m_i64NoteValue, 0, sizeof(m_i64NoteValue));
	if (m_tRoomRuler.iGameModel != GM_ONE_WIN)
	{
		// 设置玩家可下注的值
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (i == m_byUpGradePeople || m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}
			m_i64NoteValue[i][0] = m_tRoomRuler.byBaseScoreLow;
			m_i64NoteValue[i][1] = m_tRoomRuler.byBaseScoreUpper;
			m_i64UserNoteLimite[i][0] = m_tRoomRuler.byBaseScoreLow;
			m_i64UserNoteLimite[i][1] = m_tRoomRuler.byBaseScoreUpper;

			if (m_tRoomRuler.bTuiZhu == false)
			{
				m_i64NoteValue[i][2] = 0;
			}
			else     // 允许推注时的处理
			{
				if (m_vWinMoney[i].size() >= 1 && m_byLastTurnBanker != 255 && m_byLastTurnBanker != i)
				{
					if (m_byLastTurnNote[i] != 3)
					{
						//m_i64LastNoteValue：上一轮下注值
						///////////////////////////////计算推注值///////////////////////////////////////////
						GetTuiZhu(m_i64NoteValue[i][2], m_i64NoteValue[i][2], m_iShangJuQiangZhuang, m_i64LastNoteValue[i], m_iLaserUserOpenShap[i]);
						m_i64UserNoteLimite[i][2] = m_i64NoteValue[i][2];
					}
				}
			}
		}

		S_C_NoticBet tNoticBet;
		tNoticBet.byBanker = m_byUpGradePeople;
		tNoticBet.byBankerCallValue = m_i64CallScoreMaxValue;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (STATE_NULL != m_byUserStation[i])
			{
				memcpy(tNoticBet.i64CanBetValue, m_i64NoteValue[i], sizeof(tNoticBet.i64CanBetValue));
				SendGameData(i, &tNoticBet, sizeof(S_C_NoticBet), MSG_MAIN_LOADER_GAME, S_C_NOTIC_BET, 0);
			}
		}

		SendWatchData(&tNoticBet, sizeof(tNoticBet), MSG_MAIN_LOADER_GAME, S_C_NOTIC_BET, 0);

		SetTimer(TIME_NOTE, (m_byXiaZhuTime + 1) * 1000);
		SetTimer(TIME_GAME_TUOGUAN, m_byTuoGuanTime * 1000 + 500);
		m_byRemainingTimer = m_byXiaZhuTime;
		return true;
	}
	else   // 通比牛牛模式下的处理
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			long userID = GetUserIDByDeskStation(i);
			if (i == m_byUpGradePeople || userID <= 0)
			{
				continue;
			}
			// 设置下注值
			m_i64NoteValue[i][1] = m_tRoomRuler.byBaseScoreLow;
			m_i64NoteValue[i][0] = m_tRoomRuler.byBaseScoreLow;
		}

		S_C_NoticBet tNoticBet;
		tNoticBet.byBanker = m_byUpGradePeople;

		// 广播下注结果
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (STATE_NULL != m_byUserStation[i])
			{
				memcpy(tNoticBet.i64CanBetValue, m_i64NoteValue[i], sizeof(tNoticBet.i64CanBetValue));
				SendGameData(i, &tNoticBet, sizeof(S_C_NoticBet), MSG_MAIN_LOADER_GAME, S_C_NOTIC_BET, 0);
			}
		}

		SendWatchData(&tNoticBet, sizeof(tNoticBet), MSG_MAIN_LOADER_GAME, S_C_NOTIC_BET, 0);

		SetTimer(TIME_NOTE, 100);
	}
	return true;
}

//玩家下注
int	CServerGameDesk::UserNoteResult(BYTE bDeskStation, BYTE byBetBtnNum)
{
	if (bDeskStation == m_byUpGradePeople) //庄家不能下注
	{
		return true;
	}
	if (m_byUserNoteStation[bDeskStation] == 1) //已经下过注，不能再下注
	{
		return true;
	}
	if (byBetBtnNum == 0 || byBetBtnNum > sizeof(m_i64NoteValue[0]))
	{
		ERROR_LOG("下注值错误：%d", byBetBtnNum);
		return true;
	}
	m_i64PerJuTotalNote[bDeskStation] = m_i64NoteValue[bDeskStation][byBetBtnNum - 1];
	m_byCurTurnNote[bDeskStation] = byBetBtnNum;
	m_byUserNoteStation[bDeskStation] = 1;

	//标记该玩家已经下注了
	m_byUserStation[bDeskStation] = STATE_HAVE_NOTE;
	S_C_UserBetValueResult tUserBetValueResult;
	tUserBetValueResult.byBetUser = bDeskStation;
	tUserBetValueResult.i64BetValue = m_i64NoteValue[bDeskStation][byBetBtnNum - 1];

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}
		SendGameData(i, &tUserBetValueResult, sizeof(S_C_UserBetValueResult), MSG_MAIN_LOADER_GAME, S_C_USER_BET_RESULT, 0);
	}

	SendWatchData(&tUserBetValueResult, sizeof(S_C_UserBetValueResult), MSG_MAIN_LOADER_GAME, S_C_USER_BET_RESULT, 0);

	//发牌消息
	if (CountNoNotePeople() == 1)
	{
		m_bCallScoreFinish = true;
		KillTimer(TIME_NOTE);
		SetTimer(TIME_NOTE_FINISH, 200);

		//////////////////////////////奖值////////////////////////////////////////////
		//执行奖值结果
		//机器人奖池控制
		//明牌抢庄模式控制胜率
		if (m_bSuperResult == false)
		{
			if (m_iRobotCount > 0 && GM_SHOW_NT == m_tRoomRuler.iGameModel)
			{
				AiWinAutoCtrl();
			}
		}

		return true;
	}
	return true;
}

//发最后一张牌
bool CServerGameDesk::SendLastCard()
{
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byUserStation[i] != STATE_NULL)
		{
			m_byUserCardCount[i] = SH_USER_CARD;
		}
	}
	S_C_SendLastCard tSendLastCard;
	tSendLastCard.byPoint = m_byUpGradePeople;
	memcpy(tSendLastCard.byCardCount, m_byUserCardCount, sizeof(m_byUserCardCount));
	// 发送玩家最后一张手牌数据
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byUserStation[i] != STATE_NULL)
		{
			tSendLastCard.byCardData = m_byUserCard[i][4];
			m_byLastCard[i] = m_byUserCard[i][4];
			SendGameData(i, &tSendLastCard, sizeof(S_C_SendLastCard), MSG_MAIN_LOADER_GAME, S_C_SEND_LAST_CARD, 0);
		}
	}

	SendWatchData(&tSendLastCard, sizeof(tSendLastCard), MSG_MAIN_LOADER_GAME, S_C_SEND_LAST_CARD, 0);

	SendCardFinish();
	return true;
}

//发牌结束
int	CServerGameDesk::SendCardFinish()
{
	BeginOpenCard();
	return true;
}

//开始摆牛
int	CServerGameDesk::BeginOpenCard()
{
	SetGameStation(GS_OPEN_CARD);
	//通知摆牌
	S_C_NotOpenCard tNotOpenCard;

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}

		//排序手牌
		//m_Logic.SortCard(m_byUserCard[i], NULL, SH_USER_CARD);

		SendGameData(i, &tNotOpenCard, sizeof(S_C_NotOpenCard), MSG_MAIN_LOADER_GAME, S_C_NOTIC_OPER, 0);
	}

	SendWatchData(&tNotOpenCard, sizeof(tNotOpenCard), MSG_MAIN_LOADER_GAME, S_C_NOTIC_OPER, 0);

	SetTimer(TIME_AUTO_BULL, (m_byThinkTime + 1) * 1000);
	SetTimer(TIME_GAME_TUOGUAN, m_byTuoGuanTime * 1000 + 500);
	m_byRemainingTimer = m_byThinkTime;
	return true;
}

//玩家摆牛
bool CServerGameDesk::UserOpenCard(BYTE byDeskStation, void * pData, int iSize)
{
	C_S_UserOpenCard *pUserOpenCardReslut = (C_S_UserOpenCard *)pData;
	if (NULL == pUserOpenCardReslut || iSize != sizeof(C_S_UserOpenCard))
	{
		return true;
	}
	m_byUserStation[byDeskStation] = STATE_OPEN_CARD;
	m_bMingPai[byDeskStation] = true;

	//计算升起来的两张牌
	BYTE byCard2[2];
	memset(byCard2, 0, sizeof(byCard2));
	int iIndex = 0;
	for (int i = 0; i < SH_USER_CARD; i++)
	{
		bool bHave = false;
		for (int j = 0; j < 3; j++)
		{
			if (m_byUserCard[byDeskStation][i] == m_byBullCard[byDeskStation][j])
			{
				bHave = true;
			}
		}
		if (!bHave)
		{
			byCard2[iIndex++] = m_byUserCard[byDeskStation][i];
		}
	}
	if (iIndex != 2)
	{
		ERROR_LOG("算牛错误");
	}

	int iShape = UG_NO_POINT;
	iShape = m_Logic.GetShape(m_byUserCard[byDeskStation], m_byUserCardCount[byDeskStation], m_byBullCard[byDeskStation]);
	// 玩家摊牌结果
	S_C_UserOpenCardReslut TUserTanPaiResult;
	TUserTanPaiResult.byCardShap = iShape;
	TUserTanPaiResult.byDeskStation = byDeskStation;
	//升起的两张牌
	memcpy(TUserTanPaiResult.byUp, byCard2, sizeof(TUserTanPaiResult.byUp));
	//底牌牛牌张数和数据
	TUserTanPaiResult.byDownCount = 3;
	memcpy(TUserTanPaiResult.byDown, m_byBullCard[byDeskStation], sizeof(TUserTanPaiResult.byDown));
	memcpy(TUserTanPaiResult.byTanPaiCard, m_byUserCard[byDeskStation], sizeof(TUserTanPaiResult.byTanPaiCard));

	//这里记录玩家摆牛信息(断线重连的时候需要用到这些数据)
	//保存牌型，用于轮庄模式
	m_byOpenShape[byDeskStation] = iShape;
	//底牌张数
	m_byOpenUnderCount[byDeskStation] = TUserTanPaiResult.byDownCount;
	//底牌的三张牌
	memcpy(m_byOpenUnderCard[byDeskStation], TUserTanPaiResult.byDown, sizeof(m_byOpenUnderCard[byDeskStation]));
	//升起来的2张牌
	memcpy(m_byOpenUpCard[byDeskStation], TUserTanPaiResult.byUp, sizeof(m_byOpenUpCard[byDeskStation]));
	//已经摆牌的玩家
	memcpy(TUserTanPaiResult.bReadyOpenCard, m_bReadyOpenCard, sizeof(TUserTanPaiResult.bReadyOpenCard));
	TUserTanPaiResult.byBanker = m_byUpGradePeople;

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}
		SendGameData(i, &TUserTanPaiResult, sizeof(S_C_UserOpenCardReslut), MSG_MAIN_LOADER_GAME, S_C_USER_OPER_RESULT, 0);
	}

	SendWatchData(&TUserTanPaiResult, sizeof(S_C_UserOpenCardReslut), MSG_MAIN_LOADER_GAME, S_C_USER_OPER_RESULT, 0);

	//检查是否都摆牛了
	if (CheckFinish())
	{
		KillTimer(TIME_AUTO_BULL);
		//结束, 动画效果延迟
		SetTimer(TIME_GAME_FINISH, 2000);
		m_byRemainingTimer = 2;
	}
	return true;
}

// 玩家自动摆牛
void CServerGameDesk::AutoBull(BYTE byDeskStation)
{
	long userID = GetUserIDByDeskStation(byDeskStation);
	if (userID > 0 && m_byUserStation[byDeskStation] != STATE_NULL && m_byUserStation[byDeskStation] != STATE_OPEN_CARD)
	{
		BYTE bResult[3];
		memset(bResult, 0, sizeof(bResult));
		if (m_Logic.GetBull(m_byUserCard[byDeskStation], 5, bResult))
		{
			::memcpy(m_byBullCard[byDeskStation], bResult, sizeof(bResult));
		}
		else
		{
			memset(m_byBullCard[byDeskStation], 0, sizeof(m_byBullCard[byDeskStation]));
			m_byBullCard[byDeskStation][0] = m_byUserCard[byDeskStation][0];
			m_byBullCard[byDeskStation][1] = m_byUserCard[byDeskStation][1];
			m_byBullCard[byDeskStation][2] = m_byUserCard[byDeskStation][2];
		}
		C_S_UserOpenCard tUserOpenCardReslut;
		UserOpenCard(byDeskStation, &tUserOpenCardReslut, sizeof(C_S_UserOpenCard));
	}
}

//检测是否结束
int CServerGameDesk::CheckFinish()
{
	int iOpenCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_byUserStation[i] == STATE_OPEN_CARD)
		{
			iOpenCount++;
		}
	}
	if (iOpenCount >= CountPlayer())
	{
		return true;
	}

	return false;
}

//统计玩游戏玩家
int CServerGameDesk::CountPlayer()
{
	int iCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_byUserStation[i] != STATE_NULL)
		{
			iCount++;
		}
	}
	return iCount;
}

//统计没有叫庄玩家人数
BYTE CServerGameDesk::CountNoCallScorePeople()
{
	int iCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}

		if (m_byUserCallStation[i] == 255)
		{
			iCount++;
		}
	}
	return iCount;
}

//统计没有下注玩家人数
BYTE CServerGameDesk::CountNoNotePeople()
{
	int iCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_byUserStation[i] == STATE_NULL)
		{
			continue;
		}

		if (m_byUserNoteStation[i] == 0)
		{
			iCount++;
		}
	}
	return iCount;
}

//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
	{
		KillAllTimer();
		//设置数据 
		SetGameStation(GS_WAIT_ARGEE);
		int roomType = GetRoomType();

		// 记录玩家本轮下注结果, 得到上一轮的数据..   推注需要使用
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}
			m_i64LastNoteValue[i] = m_i64PerJuTotalNote[i];
		}
		// 通比牛牛模式下得到牌最大的玩家是谁
		if (GM_ONE_WIN == m_tRoomRuler.iGameModel)
		{
			BYTE  iWinCount[PLAY_COUNT] = { 0 };           // 记录每个玩家的胜利次数, 索引0表示第0个玩家, 数组值表示胜利次数
			// 比较每个玩家之间的胜利次数
			for (int i = 0; i < PLAY_COUNT; ++i)
			{
				for (int j = i + 1; j < PLAY_COUNT; ++j)
				{
					if (m_byUserStation[i] == STATE_NULL || m_byUserStation[j] == STATE_NULL)
					{
						continue;
					}
					if (m_Logic.CompareCard(m_byUserCard[i], m_byUserCardCount[i], m_byBullCard[i],
						m_byUserCard[j], m_byUserCardCount[j], m_byBullCard[j]) == 1)
					{
						++iWinCount[i];
					}
					else
					{
						++iWinCount[j];
					}
				}
			}
			// 找到胜利次数最多的玩家位置
			int iMaxWinCount = 0;
			int ipos = 0;
			for (int i = 0; i < PLAY_COUNT; ++i)
			{
				if (m_byUserStation[i] == STATE_NULL)
				{
					continue;
				}
				if (iWinCount[i] > iMaxWinCount)
				{
					iMaxWinCount = iWinCount[i];
					ipos = i;
				}
			}
			// 设置庄家
			m_byUpGradePeople = ipos;
		}

		//特殊情况特殊处理
		if (GM_SHOW_NT == m_tRoomRuler.iGameModel && m_byUpGradePeople == 255)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_byUserStation[i] == STATE_NULL)
				{
					continue;
				}
				if (m_i64PerJuTotalNote[i] == 0)
				{
					m_byUpGradePeople = i;
					break;
				}
			}
		}

		// 游戏结束数据包
		S_C_GameFinish t_S_C_GameFinish;
		::memcpy(t_S_C_GameFinish.byUpData, &m_byOpenUpCard, sizeof(m_byOpenUpCard));
		::memcpy(t_S_C_GameFinish.byDownData, &m_byBullCard, sizeof(m_byBullCard));
		::memcpy(t_S_C_GameFinish.byCardData, &m_byUserCard, sizeof(m_byUserCard));
		memcpy(t_S_C_GameFinish.byUserState, m_byUserStation, sizeof(t_S_C_GameFinish.byUserState));
		memcpy(t_S_C_GameFinish.byCardShap, m_byOpenShape, sizeof(t_S_C_GameFinish.byCardShap));

		__int64 i64TempRecord[PLAY_COUNT];
		::memset(i64TempRecord, 0, sizeof(i64TempRecord));
		__int64 iNtLoseMoney = 0;//庄家输赢的钱
		__int64 i64TempMoney = 0; //金币场结算用
		__int64 i64AllXianJiaWinMoney = 0, i64AllXianJiaFailMoney = 0;
		long long resNums = 0;

		int isetMultiple = 0;
		// 记录游戏倍数
		if (m_i64CallScoreMaxValue == 0 || m_i64CallScoreMaxValue == 1 || m_i64CallScoreMaxValue == -1)
		{
			isetMultiple = 1;
		}
		else
		{
			isetMultiple = m_i64CallScoreMaxValue;
		}
		m_iShangJuQiangZhuang = isetMultiple;
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}

			if (i != m_byUpGradePeople)
			{
				i64TempMoney = ComputePoint(i) * m_i64PerJuTotalNote[i] * isetMultiple * m_tRoomRuler.iBaseScore;
				if (roomType != ROOM_TYPE_PRIVATE && GetRoomResNums(i, resNums))
				{
					////控制赢，不能赢多于身上金币
					//if (i64TempMoney > 0 && i64TempMoney > resNums)
					//{
					//	i64TempMoney = resNums;
					//}

					//控制输，不能输多于身上金币
					if (i64TempMoney < 0 && resNums < llabs(i64TempMoney))
					{
						i64TempMoney = -resNums;
					}
				}
				// 玩家输赢分数
				t_S_C_GameFinish.i64TurePoint[i] = i64TempMoney;
				iNtLoseMoney += t_S_C_GameFinish.i64TurePoint[i];
				if (i64TempMoney > 0)
				{
					i64AllXianJiaWinMoney += i64TempMoney;
				}
				else
				{
					i64AllXianJiaFailMoney += llabs(i64TempMoney);
				}
			}
		}
		if (roomType != ROOM_TYPE_PRIVATE && GetRoomResNums(m_byUpGradePeople, resNums) && iNtLoseMoney != 0)
		{
			////控制赢，不能赢多于身上金币
			//if (iNtLoseMoney < 0 && resNums < llabs(iNtLoseMoney))
			//{
			//	//退金币
			//	long long llTuiMoney = llabs(iNtLoseMoney)-resNums;
			//	for (int i = 0; i < PLAY_COUNT; ++i)
			//	{
			//		if (m_byUserStation[i] == STATE_NULL || i == m_byUpGradePeople)
			//		{
			//			continue;
			//		}

			//		if (t_S_C_GameFinish.i64TurePoint[i] < 0)
			//		{
			//			t_S_C_GameFinish.i64TurePoint[i] = t_S_C_GameFinish.i64TurePoint[i] + floor( llabs(t_S_C_GameFinish.i64TurePoint[i]) * 1.0 / i64AllXianJiaFailMoney * llTuiMoney );
			//		}
			//	}
			//	iNtLoseMoney = -resNums;
			//}

			//控制输，不能输多于身上金币
			if (iNtLoseMoney > 0 && resNums < iNtLoseMoney)
			{
				for (int i = 0; i < PLAY_COUNT; ++i)
				{
					if (m_byUserStation[i] == STATE_NULL || i == m_byUpGradePeople)
					{
						continue;
					}

					if (t_S_C_GameFinish.i64TurePoint[i] > 0)
					{
						t_S_C_GameFinish.i64TurePoint[i] = (long long)floor(t_S_C_GameFinish.i64TurePoint[i] * 1.0 / i64AllXianJiaWinMoney * (resNums + i64AllXianJiaFailMoney));
					}
				}
				iNtLoseMoney = resNums;
			}
		}

		t_S_C_GameFinish.i64TurePoint[m_byUpGradePeople] = -iNtLoseMoney;

		//计算抽水
		long long UserPumpGold[PLAY_COUNT] = { 0 };
		memset(UserPumpGold, 0, sizeof(UserPumpGold));
		if (roomType == ROOM_TYPE_GOLD)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (t_S_C_GameFinish.i64TurePoint[i] > 0)
				{
					UserPumpGold[i] = t_S_C_GameFinish.i64TurePoint[i] * 0 / 100;
					t_S_C_GameFinish.i64TurePoint[i] -= UserPumpGold[i];
				}
			}
		}

		/*ERROR_LOG("###第%d局，算分:[%I64d] [%I64d] [%I64d] [%I64d] [%I64d] [%I64d]，庄家：%d###",m_iRunGameCount,t_S_C_GameFinish.i64TurePoint[0],t_S_C_GameFinish.i64TurePoint[1]
		,t_S_C_GameFinish.i64TurePoint[2],t_S_C_GameFinish.i64TurePoint[3],t_S_C_GameFinish.i64TurePoint[4],t_S_C_GameFinish.i64TurePoint[5],m_byUpGradePeople);*/

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}
			// 如果玩家本轮输了, 清空历史记录
			if (t_S_C_GameFinish.i64TurePoint[i] < 0)
			{
				m_vWinMoney[i].clear();
			}
			// 如果玩家本轮赢了, 记录下该局的输赢值
			else
			{
				m_vWinMoney[i].push_back(t_S_C_GameFinish.i64TurePoint[i]);
			}
		}
		if (m_tRoomRuler.iGameModel == GM_ONE_WIN)
		{
			m_byUpGradePeople = 255;
		}
		memcpy(m_byLastTurnNote, m_byCurTurnNote, sizeof(m_byLastTurnNote));
		t_S_C_GameFinish.byUpGradeStation = m_byUpGradePeople;
		m_byLastTurnBanker = m_byUpGradePeople;        // 记录该庄家位置
		m_byLastPlayerCount = CountPlayer();
		::memcpy(m_byLastUserCard, m_byUserCard, sizeof(m_byUserCard));
		::memcpy(m_i64LastTurePoint, t_S_C_GameFinish.i64TurePoint, sizeof(t_S_C_GameFinish.i64TurePoint));
		::memcpy(m_iLaserUserOpenShap, m_byOpenShape, sizeof(m_byOpenShape));
		m_byLastCallScore = m_i64CallScoreMaxValue;    // 记录上一轮叫分最大的人, 需要查看上一轮的数据

		// 保存牛牛的牌型
		char gameData[1024];
		memset(gameData, 0, sizeof(gameData));
		sprintf(gameData + strlen(gameData), "%d|", m_byUpGradePeople);
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (GetUserIDByDeskStation(i) <= 0)
			{
				continue;
			}
			sprintf(gameData + strlen(gameData), "%d,%d,%d,%d,%d,%d,%d,%lld|", GetUserIDByDeskStation(i), m_byUserCard[i][0], m_byUserCard[i][1],
				m_byUserCard[i][2], m_byUserCard[i][3], m_byUserCard[i][4], m_byOpenShape[i], m_i64PerJuTotalNote[i]);
		}

		//写入数据库				
		bool temp_cut[PLAY_COUNT];
		memset(temp_cut, 0, sizeof(temp_cut));

		if (roomType == ROOM_TYPE_PRIVATE)
		{
			// 如需使用一般斗牛模式下结算部分, 注释掉上面部分的代码, 使用下面的这行代码
			ChangeUserPointPrivate(t_S_C_GameFinish.i64TurePoint, temp_cut, "", true, gameData);
		}
		else if (roomType == ROOM_TYPE_GOLD)
		{
			ChangeUserPoint(t_S_C_GameFinish.i64TurePoint, temp_cut, UserPumpGold);
		}
		else if (roomType == ROOM_TYPE_FRIEND)
		{
			ChangeUserPointGoldRoom(t_S_C_GameFinish.i64TurePoint, "", true, gameData);
		}
		else if (roomType == ROOM_TYPE_FG_VIP)
		{
			ChangeUserFireCoin(t_S_C_GameFinish.i64TurePoint, "", true, gameData);
		}

		memcpy(t_S_C_GameFinish.i64ChangeMoney, t_S_C_GameFinish.i64TurePoint, sizeof(t_S_C_GameFinish.i64ChangeMoney));

		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_byUserStation[i] == STATE_NULL)
			{
				continue;
			}
			if (roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP)
			{
				if (m_tZongResult.iBestWin[i] < t_S_C_GameFinish.i64ChangeMoney[i])
				{
					m_tZongResult.iBestWin[i] = t_S_C_GameFinish.i64ChangeMoney[i];
				}
				m_tZongResult.iMoney[i] += t_S_C_GameFinish.i64ChangeMoney[i]; //统计大结算
			}
		}

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (GetUserIDByDeskStation(i) <= 0)
			{
				continue;
			}
			SendGameData(i, &t_S_C_GameFinish, sizeof(S_C_GameFinish), MSG_MAIN_LOADER_GAME, S_C_GAME_FINISH, 0);
		}

		SendWatchData(&t_S_C_GameFinish, sizeof(t_S_C_GameFinish), MSG_MAIN_LOADER_GAME, S_C_GAME_FINISH, 0);

		////////////////////////////////确定庄家位置/////////////////////////////////////
		// 游戏模式是牛牛上庄模式
		if (GM_NIUNIU_NT == m_tRoomRuler.iGameModel)
		{
			vector<BYTE> ivNiuNiuCard;
			ivNiuNiuCard.clear();
			// 记录牌型是牛牛的玩家位置
			for (int i = 0; i < PLAY_COUNT; ++i)
			{
				if (m_byUserStation[i] == STATE_NULL)
				{
					continue;
				}
				if (m_byOpenShape[i] == UG_BULL_BULL)
				{
					ivNiuNiuCard.push_back(i);
				}
			}
			int iNiuSize = ivNiuNiuCard.size();
			// 得到牌型是牛牛且牛牛牌中最大的玩家位置
			if (iNiuSize >= 2)
			{
				int iMaxStation = 0;
				for (int i = 1;i < iNiuSize; i++)
				{
					if (m_Logic.NewCompareCard(m_byUserCard[ivNiuNiuCard[i]], SH_USER_CARD, m_byUserCard[ivNiuNiuCard[iMaxStation]], SH_USER_CARD) == 1)
					{
						iMaxStation = i;
					}
				}
				m_byUpGradePeople = ivNiuNiuCard[iMaxStation];
			}
			// 如果只有一个人的牌型是牛牛
			if (iNiuSize == 1)
			{
				m_byUpGradePeople = ivNiuNiuCard[0];
			}
		}

		ReSetGameState(bCloseFlag);
		if ((roomType == ROOM_TYPE_FRIEND || roomType == ROOM_TYPE_PRIVATE || roomType == ROOM_TYPE_FG_VIP) && m_iRunGameCount == m_iVipGameCount)
		{
			SetTimer(TIME_GAME_COUNT_FINISH, 2000);
		}
		else
		{
			if (roomType == ROOM_TYPE_PRIVATE)
			{
				SetTimer(TIME_GAME_AUTO_BEGIN, (m_byBeginTime + 2) * 1000);
				SetTimer(TIME_GAME_TUOGUAN, (m_byTuoGuanTime + 4) * 1000);
			}
			else
			{
				for (int i = 0; i < PLAY_COUNT; ++i)
				{
					if (GetUserIDByDeskStation(i) > 0)
					{
						OnHandleUserRequestCancelAuto(i);
					}
				}
			}
			CGameDesk::GameFinish(bDeskStation, bCloseFlag);
		}
		return true;
	}
	case GF_SALE:			//游戏安全结束
	case GFF_SAFE_FINISH:
	{
		KillAllTimer();
		SetGameStation(GS_WAIT_ARGEE);
		bCloseFlag = GFF_SAFE_FINISH;
		ReSetGameState(bCloseFlag);
		CGameDesk::GameFinish(bDeskStation, bCloseFlag);
		return true;
	}
	case GFF_FORCE_FINISH:		//用户断线离开
	{
		return true;
	}
	// switch结束
	}
	return true;
}

//计算各家分数
int CServerGameDesk::ComputePoint(BYTE DeskStation)
{
	int Shape;
	int Point = 0;
	int winer = -1;

	if (m_Logic.CompareCard(m_byUserCard[DeskStation], m_byUserCardCount[DeskStation], m_byBullCard[DeskStation],
		m_byUserCard[m_byUpGradePeople], m_byUserCardCount[m_byUpGradePeople], m_byBullCard[m_byUpGradePeople]) == 1)
	{
		winer = DeskStation;
		Point = 1;
	}
	else
	{
		winer = m_byUpGradePeople;
		Point = -1;
	}

	Shape = m_Logic.GetShape(m_byUserCard[winer], m_byUserCardCount[winer], m_byBullCard[winer]);
	Point = Point * m_byCardShapeBase[Shape];

	return Point;
}

//获取下一个玩家位置
BYTE CServerGameDesk::GetNextDeskStation(BYTE bDeskStation)
{
	if (!m_bTurnRule)//顺时针
	{
		int i = 1;
		for (; i < PLAY_COUNT; i++)
		{
			BYTE next = (bDeskStation + i) % PLAY_COUNT;
			long nextUserID = GetUserIDByDeskStation(next);
			if (nextUserID > 0 && m_byUserStation[next] != STATE_NULL)
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
		if (nextUserID > 0 && m_byUserStation[next] != STATE_NULL)
		{
			return next;
		}
	}

	return 255;
}

///清理所有计时器
void CServerGameDesk::KillAllTimer()
{
	KillTimer(TIME_NOTE_FINISH);			//发牌定时器
	KillTimer(TIME_CALL_NT);				//叫庄
	KillTimer(TIME_GAME_FINISH);			//游戏结束定时器
	KillTimer(TIME_AUTO_BULL);				//断线自动摆牛
	KillTimer(TIME_NOTE);					//下注计时器
	KillTimer(TIME_OUT_CARD);                // 出牌定时器
	KillTimer(TIME_RAND_BANKER);
	KillTimer(TIME_GAME_TUOGUAN);
	KillTimer(TIME_REMAIN_TIMER);
}

//桌子成功解散
void CServerGameDesk::OnDeskSuccessfulDissmiss(bool isDismissMidway)
{
	if (m_iRunGameCount > 0) //大于一局才发大结算
	{
		// 计算抽水值
		CGameDesk::SetDeskPercentageScore(m_tZongResult.iMoney);

		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (GetUserIDByDeskStation(i) <= 0)
			{
				continue;
			}
			SendGameData(i, &m_tZongResult, sizeof(TZongResult), MSG_MAIN_LOADER_GAME, SUB_S_GAME_END_ALL, 0);
		}

		SendWatchData(&m_tZongResult, sizeof(m_tZongResult), MSG_MAIN_LOADER_GAME, SUB_S_GAME_END_ALL, 0);
	}
	/*ERROR_LOG("======大结算:[%I64d] [%I64d] [%I64d] [%I64d] [%I64d] [%I64d]======", m_tZongResult.iMoney[0], m_tZongResult.iMoney[1]
		, m_tZongResult.iMoney[2], m_tZongResult.iMoney[3], m_tZongResult.iMoney[4], m_tZongResult.iMoney[5]);*/

	CGameDesk::OnDeskSuccessfulDissmiss(isDismissMidway);
}

//初始化游戏数据，房卡场大结算的时候调用
void CServerGameDesk::InitDeskGameData()
{
	KillAllTimer();
	//设置数据 
	SetGameStation(GS_WAIT_ARGEE);
	m_bAlreadSet = false;
	m_tRoomRuler.Init();
	ReSetGameState(0);
	memset(m_byLastTurnNote, 1, sizeof(m_byLastTurnNote));// 上一轮玩家下注值 取1,2,3      
	memset(m_iLaserUserOpenShap, 0, sizeof(m_iLaserUserOpenShap));
	memset(m_bSuperFlag, 0, sizeof(m_bSuperFlag));
	memset(m_bTuoGuan, false, sizeof(m_bTuoGuan));
	m_byUpGradePeople = 255;
	m_byLastTurnBanker = 255;
	m_iShangJuQiangZhuang = 1;
	m_tZongResult.Init();
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		m_vWinMoney[i].clear();
	}
	m_bySuperWinStation = 255;
	m_bySuperFailStation = 255;
}

// 玩家请求托管
bool CServerGameDesk::OnHandleUserRequestAuto(BYTE deskStation)
{
	if (deskStation >= 0 && deskStation < PLAY_COUNT && m_byUserStation[deskStation] != STATE_NULL)
	{
		if (m_bTuoGuan[deskStation] == false)
		{
			BYTE byStatus = GetGameStation();
			if (byStatus == GS_WAIT_ARGEE)
			{
				HandleNotifyMessage(deskStation, MSG_ASS_LOADER_GAME_AGREE, NULL, 0, false);
			}
			else if (byStatus == GS_ROB_NT)
			{
				if (m_byUserCallStation[deskStation] == 255)
				{
					UserCallScoreResult(deskStation, 0);
				}
			}
			else if (byStatus == GS_NOTE)
			{
				if (m_byUpGradePeople != deskStation)
				{
					if (m_byUserNoteStation[deskStation] == 0 &&
						(m_byUserStation[deskStation] == STATE_PLAY_GAME || m_byUserStation[deskStation] == STATE_CALL_SCORE))
					{
						UserNoteResult(deskStation, 1);
						m_i64PerJuTotalNote[deskStation] = m_i64UserNoteLimite[deskStation][0];
					}
				}
			}
			else if (byStatus == GS_OPEN_CARD)
			{
				m_bReadyOpenCard[deskStation] = true;
				AutoBull(deskStation);
			}
		}
		m_bTuoGuan[deskStation] = true;
	}
	return CGameDesk::OnHandleUserRequestAuto(deskStation);
}

// 玩家取消托管
bool CServerGameDesk::OnHandleUserRequestCancelAuto(BYTE deskStation)
{
	if (deskStation >= 0 && deskStation < PLAY_COUNT)
	{
		m_bTuoGuan[deskStation] = false;
	}
	return CGameDesk::OnHandleUserRequestCancelAuto(deskStation);
}

//设置游戏规则
void CServerGameDesk::SetGameRuler()
{
	if (m_bAlreadSet)
	{
		return;
	}
	m_bAlreadSet = true;
	// 特殊牌型和高级选项里面, 勾上了是1, 没勾上是0.  其他选项均是从1开始
	Json::Reader reader;
	Json::Value vGameRulerValue;
	m_tRoomRuler.byFangZhu = 0;
	int roomType = GetRoomType();
	if (reader.parse(m_szGameRules, vGameRulerValue))
	{
		//获取金币房和俱乐部vip房间底分
		m_tRoomRuler.iBaseScore = GetBasePoint();

		int iTemp = 0;
		iTemp = vGameRulerValue["gameIdx"].asInt();
		if (iTemp == 1)
			m_tRoomRuler.iGameModel = GM_NIUNIU_NT;
		if (iTemp == 2)
			m_tRoomRuler.iGameModel = GM_PERMANENT_NT;
		if (iTemp == 3)
			m_tRoomRuler.iGameModel = GM_FREE_NT;
		if (iTemp == 4)
			m_tRoomRuler.iGameModel = GM_SHOW_NT;
		if (iTemp == 5)
			m_tRoomRuler.iGameModel = GM_ONE_WIN;
		iTemp = vGameRulerValue["Difen"].asInt();
		if (iTemp == 1)
		{
			m_tRoomRuler.byBaseScoreLow = 1;
			m_tRoomRuler.byBaseScoreUpper = 2;
		}
		if (iTemp == 2)
		{
			m_tRoomRuler.byBaseScoreLow = 2;
			m_tRoomRuler.byBaseScoreUpper = 4;
		}
		if (iTemp == 3)
		{
			m_tRoomRuler.byBaseScoreLow = 4;
			m_tRoomRuler.byBaseScoreUpper = 8;
		}
		if (iTemp == 4)
		{
			m_tRoomRuler.byBaseScoreLow = 5;
			m_tRoomRuler.byBaseScoreUpper = 10;
		}

		memset(m_byCardShapeBase, 1, sizeof(m_byCardShapeBase));
		iTemp = vGameRulerValue["Guize"].asInt();
		if (iTemp == 1)
		{
			m_byCardShapeBase[10] = 4;
			m_byCardShapeBase[9] = 3;
			m_byCardShapeBase[8] = 2;
			m_byCardShapeBase[7] = 2;
			m_tRoomRuler.iGuize = iTemp;
		}
		if (iTemp == 2)
		{
			m_byCardShapeBase[10] = 3;
			m_byCardShapeBase[9] = 2;
			m_byCardShapeBase[8] = 2;
			m_tRoomRuler.iGuize = iTemp;
		}
		iTemp = vGameRulerValue["Tesu_1"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bHuaNiu = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bHuaNiu = true;
		}
		iTemp = vGameRulerValue["Tesu_2"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bBombNiu = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bBombNiu = true;
		}
		iTemp = vGameRulerValue["Tesu_3"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bSmallNiu = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bSmallNiu = true;
		}

		//新增顺子牛，同花牛，葫芦牛
		iTemp = vGameRulerValue["Tesu_4"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bShunNiu = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bShunNiu = true;
		}

		iTemp = vGameRulerValue["Tesu_5"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bTongHuaNiu = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bTongHuaNiu = true;
		}

		iTemp = vGameRulerValue["Tesu_6"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bHuLuNiu = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bHuLuNiu = true;
		}

		bool flag = vGameRulerValue["Tz"].asBool();
		if (flag == false)
		{
			m_tRoomRuler.bTuiZhu = false;
		}
		if (flag == true)
		{
			m_tRoomRuler.bTuiZhu = true;
		}
		iTemp = vGameRulerValue["Stopjoin"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bStopJoin = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bStopJoin = true;
		}
		iTemp = vGameRulerValue["Jzcp"].asInt();
		if (iTemp == 0)
		{
			m_tRoomRuler.bStopShuffle = false;
		}
		if (iTemp == 1)
		{
			m_tRoomRuler.bStopShuffle = true;
		}

		if (m_tRoomRuler.iGameModel == GM_PERMANENT_NT)
		{
			iTemp = vGameRulerValue["Shangzhuan"].asInt();

			if (iTemp == 1)
			{
				m_tRoomRuler.iShangZhuangFen = 0;
			}
			if (iTemp == 2)
			{
				m_tRoomRuler.iShangZhuangFen = 100 * m_tRoomRuler.byBaseScoreLow;
			}
			if (iTemp == 3)
			{
				m_tRoomRuler.iShangZhuangFen = 150 * m_tRoomRuler.byBaseScoreLow;
			}
			if (iTemp == 4)
			{
				m_tRoomRuler.iShangZhuangFen = 200 * m_tRoomRuler.byBaseScoreLow;
			}
		}
		if (m_tRoomRuler.iGameModel == GM_SHOW_NT)
		{
			m_i64JiaoFenValue[0] = 0;
			iTemp = vGameRulerValue["Qiangzhuan"].asInt();
			if (iTemp == 1)
			{
				m_tRoomRuler.iMaxQiangZhuang = 1;
				m_i64JiaoFenValue[1] = 1;
			}
			if (iTemp == 2)
			{
				m_tRoomRuler.iMaxQiangZhuang = 2;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
			}
			if (iTemp == 3)
			{
				m_tRoomRuler.iMaxQiangZhuang = 3;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
				m_i64JiaoFenValue[3] = 3;
			}
			if (iTemp == 4)
			{
				m_tRoomRuler.iMaxQiangZhuang = 4;
				m_i64JiaoFenValue[1] = 1;
				m_i64JiaoFenValue[2] = 2;
				m_i64JiaoFenValue[3] = 3;
				m_i64JiaoFenValue[4] = 4;
			}
		}

		m_tRoomRuler.byZhiFuFangShi = m_payType;

		// 牌型倍率设置, 注牛将军没有金牛银牛之分, 金牛就是五花牛, 设置游戏规则时:未勾选五花牛的话就只是牛牛牌型
		if (m_tRoomRuler.bHuaNiu == true)
		{
			m_byCardShapeBase[UG_BULL_GOLD] = 5;
		}

		if (m_tRoomRuler.bBombNiu == true)
		{
			m_byCardShapeBase[UG_BULL_BOMB] = 8;
		}

		if (m_tRoomRuler.bSmallNiu == true)
		{
			m_byCardShapeBase[UG_FIVE_SMALL] = 10;
		}

		//新增顺子牛，同花牛，葫芦牛

		if (m_tRoomRuler.bShunNiu == true)
		{
			m_byCardShapeBase[UG_SHUN_BULL] = 5;
		}

		if (m_tRoomRuler.bTongHuaNiu == true)
		{
			m_byCardShapeBase[UG_TONGHUA_BULL] = 6;
		}

		if (m_tRoomRuler.bHuLuNiu == true)
		{
			m_byCardShapeBase[UG_HULU_BULL] = 7;
		}
	}
	m_Logic.SetCardShape(m_tRoomRuler);
}

//玩家掉线
bool CServerGameDesk::UserNetCut(GameUserInfo *pUser)
{
	CGameDesk::UserNetCut(pUser);
	if (pUser->deskStation >= 0 && pUser->deskStation < PLAY_COUNT)
	{
		m_bNetCut[pUser->deskStation] = true;
	}
	if (GetRoomType() == ROOM_TYPE_GOLD && IsPlayGame(0))
	{
		int iCutNums = 0;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			if (m_bNetCut[i])
			{
				iCutNums++;
			}
		}
		if (CountPlayer() == iCutNums)
		{
			GameFinish(0, GF_SALE);
		}
	}
	return true;
}

// 玩家离开桌子，直接返回大厅
bool CServerGameDesk::UserLeftDesk(GameUserInfo* pUser)
{
	if (pUser->deskStation >= 0 && pUser->deskStation < PLAY_COUNT)
	{
		m_bNetCut[pUser->deskStation] = false;
		m_bUserReady[pUser->deskStation] = false;
		m_vWinMoney[pUser->deskStation].clear();
		if (pUser->deskStation == m_byLastTurnBanker)
		{
			m_byLastTurnBanker = 255;
		}
		m_bSuperFlag[pUser->deskStation] = false;
		m_byLastTurnNote[pUser->deskStation] = 1;
		m_bTuoGuan[pUser->deskStation] = false;
	}
	return CGameDesk::UserLeftDesk(pUser);
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if (GS_ROB_NT == GetGameStation() || GS_NOTE == GetGameStation()
		|| GS_SEND_CARD == GetGameStation() || GS_OPEN_CARD == GetGameStation())
	{
		return true;
	}
	return false;
}

//执行超端设置
bool CServerGameDesk::SuperSetResult(BYTE byStation)
{
	// 发送超端设置结果
	S_C_SuperUserResult tSuperUserResult;
	if (byStation != 255)
	{
		tSuperUserResult.byDeskStation = byStation;
		tSuperUserResult.byOperatorResult = 1;
		SendGameData(byStation, &tSuperUserResult, sizeof(S_C_SuperUserResult), MSG_MAIN_LOADER_GAME, S_C_SUPER_USER_RESULT, 0);
		return true;
	}
	//设置赢家
	if (m_bySuperWinStation >= 0 && m_bySuperWinStation < PLAY_COUNT && STATE_NULL != m_byUserStation[m_bySuperWinStation])
	{
		BYTE byTmpMax = 255;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_NULL != m_byUserStation[i])
			{
				if (255 == byTmpMax)
				{
					byTmpMax = i;
					break;
				}
			}
		}

		//找最大
		if (byTmpMax != 255)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (i == byTmpMax)
				{
					continue;
				}
				if (STATE_NULL != m_byUserStation[i])
				{
					if (m_Logic.NewCompareCard(m_byUserCard[i], SH_USER_CARD, m_byUserCard[byTmpMax], SH_USER_CARD) == 1)
					{
						byTmpMax = i;
					}
				}
			}
		}
		if (byTmpMax != m_bySuperWinStation)
		{
			Change2UserCard(byTmpMax, m_bySuperWinStation);
		}
		m_bySuperWinStation = 255;
		m_bSuperResult = true;
	}

	//设置输家
	if (m_bySuperFailStation >= 0 && m_bySuperFailStation < PLAY_COUNT && STATE_NULL != m_byUserStation[m_bySuperFailStation])
	{
		BYTE  byTmpMin = 255;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_NULL != m_byUserStation[i])
			{
				if (255 == byTmpMin)
				{
					byTmpMin = i;
					break;
				}
			}
		}
		//找最小
		if (byTmpMin != 255)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (i == byTmpMin)
				{
					continue;
				}
				if (STATE_NULL != m_byUserStation[i])
				{
					if (m_Logic.NewCompareCard(m_byUserCard[i], SH_USER_CARD, m_byUserCard[byTmpMin], SH_USER_CARD) != 1)
					{
						byTmpMin = i;
					}
				}
			}
		}

		//把最小的牌给玩家
		if (byTmpMin != m_bySuperFailStation)
		{
			Change2UserCard(byTmpMin, m_bySuperFailStation);
		}
		m_bySuperFailStation = 255;
		m_bSuperResult = true;
	}

	return true;
}

// 玩家请求查看玩家的牌型
bool CServerGameDesk::GetUserCardShape(BYTE byDeskStation)
{
	int iShape = UG_NO_POINT;
	iShape = m_Logic.GetShape(m_byUserCard[byDeskStation], m_byUserCardCount[byDeskStation]);
	BYTE bResult[3];
	memset(bResult, 0, sizeof(bResult));
	if (!m_Logic.GetBull(m_byUserCard[byDeskStation], 5, bResult))
	{
		bResult[0] = m_byUserCard[byDeskStation][0];
		bResult[1] = m_byUserCard[byDeskStation][1];
		bResult[2] = m_byUserCard[byDeskStation][2];
	}
	S_C_GetCardShapeResult tCardShapeResult;
	tCardShapeResult.byDeskStation = byDeskStation;
	tCardShapeResult.byCardShape = iShape;
	memcpy(tCardShapeResult.byCard3, bResult, sizeof(tCardShapeResult.byCard3));
	SendGameData(byDeskStation, &tCardShapeResult, sizeof(S_C_GetCardShapeResult), MSG_MAIN_LOADER_GAME, S_C_CARDSHAPE_RESULT, 0);
	return true;
}

//配牌器
bool CServerGameDesk::BrandCard()
{
	//读取超端数据
	char cFileName[64];
	memset(cFileName, 0, sizeof(cFileName));
	snprintf(cFileName, 64, "C:/web/json/%d.json", NAME_ID);
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

	//获取手牌数（手牌有几张）
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
		if (SH_USER_CARD == tempKeyValue)
		{
			iCardCount[i] = tempKeyValue;
		}
	}

	//获取手牌数据（手牌的具体值）
	BYTE byCardData[PLAY_COUNT][SH_USER_CARD];
	memset(byCardData, 0, sizeof(byCardData));
	bool bHandFlag[PLAY_COUNT];
	memset(bHandFlag, false, sizeof(bHandFlag));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (SH_USER_CARD != iCardCount[i])
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
			//bHandFlag[i] = true;
		}
		bHandFlag[i] = true;
	}

	fin.close();

	//把选择的手牌删除
	int iChangeNums = 0;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (!bHandFlag[i])
		{
			continue;
		}
		for (int b = 0;b < SH_USER_CARD;b++)
		{
			if (0 == byCardData[i][b])
			{
				continue;
			}
			for (int c = 0;c < 52;c++)
			{
				if (0 == m_iTotalCard[c])
				{
					continue;
				}
				if (byCardData[i][b] == m_iTotalCard[c])
				{
					m_iTotalCard[c] = 0;
					iChangeNums++;
					break;
				}
			}
		}
	}

	//先把配的牌配在指定的位置
	int index = 0;
	BYTE byTempMenPai[52];
	memset(byTempMenPai, 0, sizeof(byTempMenPai));
	for (int j = 0;j < PLAY_COUNT;j++)
	{
		if (bHandFlag[j])
		{
			for (int k = 0;k < SH_USER_CARD;k++)
			{
				byTempMenPai[j*SH_USER_CARD + k] = byCardData[j][k];
			}
		}
	}

	//把牌补齐
	for (int i = 0;i < 52;i++)
	{
		if (byTempMenPai[i] == 0)
		{
			for (int k = 0;k < 52;k++)
			{
				if (m_iTotalCard[k] != 0)
				{
					byTempMenPai[i] = m_iTotalCard[k];
					m_iTotalCard[k] = 0;
					break;
				}
			}
		}
	}

	memset(m_iTotalCard, 0, sizeof(m_iTotalCard));
	memcpy(m_iTotalCard, byTempMenPai, sizeof(m_iTotalCard));

	return true;
}


///////////////////////////////////////////////////////////////奖值//////////////////////////////////////////////////////////////////////
//交换两个人的牌
void CServerGameDesk::Change2UserCard(BYTE byFirstDesk, BYTE bySecondDesk)
{
	if (byFirstDesk >= 0 && byFirstDesk < PLAY_COUNT && bySecondDesk >= 0 && bySecondDesk < PLAY_COUNT)
	{
		if (m_byUserStation[byFirstDesk] == STATE_NULL || m_byUserStation[bySecondDesk] == STATE_NULL || byFirstDesk == bySecondDesk)
		{
			return;
		}
		BYTE byTmpCard[SH_USER_CARD];
		memcpy(byTmpCard, m_byUserCard[byFirstDesk], sizeof(byTmpCard));
		memcpy(m_byUserCard[byFirstDesk], m_byUserCard[bySecondDesk], sizeof(m_byUserCard[byFirstDesk]));
		memcpy(m_byUserCard[bySecondDesk], byTmpCard, sizeof(m_byUserCard[bySecondDesk]));
	}
}

//改变两个玩家的手牌，让bySecondDesk手牌比byFirstDesk大
bool CServerGameDesk::Change2UserCardEx(BYTE byFirstDesk, BYTE bySecondDesk)
{
	BYTE byFirstInitCard = m_byUserCard[byFirstDesk][4], bySecondInitCard = m_byUserCard[bySecondDesk][4];
	if (byFirstDesk >= 0 && byFirstDesk < PLAY_COUNT && bySecondDesk >= 0 && bySecondDesk < PLAY_COUNT)
	{
		if (m_byUserStation[byFirstDesk] == STATE_NULL || m_byUserStation[bySecondDesk] == STATE_NULL || byFirstDesk == bySecondDesk)
		{
			return false;
		}

		//第一种
		int iFirstShape = m_Logic.GetShape(m_byUserCard[byFirstDesk], SH_USER_CARD);
		int iSecondShape = m_Logic.GetShape(m_byUserCard[bySecondDesk], SH_USER_CARD);
		if (iFirstShape < UG_BULL_SIX)
		{
			//byFirstDesk的牌型不变，增大bySecondDesk牌型
			for (int i = PLAY_COUNT * SH_USER_CARD; i < m_iAllCardCount - 1; i++)
			{
				m_byUserCard[bySecondDesk][4] = m_iTotalCard[i];
				if (m_Logic.NewCompareCard(m_byUserCard[bySecondDesk], SH_USER_CARD, m_byUserCard[byFirstDesk], SH_USER_CARD) == 1)
				{
					//交换门牌
					m_iTotalCard[i] = bySecondInitCard;
					return true;
				}
			}
		}
		else
		{
			//bySecondDesk牌型不变，减少byFirstDesk的牌型
			for (int i = PLAY_COUNT * SH_USER_CARD; i < m_iAllCardCount - 1; i++)
			{
				m_byUserCard[byFirstDesk][4] = m_iTotalCard[i];
				if (m_Logic.NewCompareCard(m_byUserCard[bySecondDesk], SH_USER_CARD, m_byUserCard[byFirstDesk], SH_USER_CARD) == 1)
				{
					//交换门牌
					m_iTotalCard[i] = byFirstInitCard;
					return true;
				}
			}
		}

		ERROR_LOG("第一种算法失败，执行第二种算法...");
		//第二种
		for (int i = PLAY_COUNT * SH_USER_CARD; i < m_iAllCardCount - 1; i++)
		{
			m_byUserCard[bySecondDesk][4] = m_iTotalCard[i];
			for (int j = i + 1; j < m_iAllCardCount; j++)
			{
				m_byUserCard[byFirstDesk][4] = m_iTotalCard[j];
				if (m_Logic.NewCompareCard(m_byUserCard[bySecondDesk], SH_USER_CARD, m_byUserCard[byFirstDesk], SH_USER_CARD) == 1)
				{
					//交换门牌
					m_iTotalCard[i] = bySecondInitCard;
					m_iTotalCard[j] = byFirstInitCard;
					return true;
				}
			}
		}

		//寻找失败恢复手牌
		m_byUserCard[byFirstDesk][4] = byFirstInitCard;
		m_byUserCard[bySecondDesk][4] = bySecondInitCard;
		ERROR_LOG("尝试交换手牌失败,byFirstInitCard=%d,bySecondInitCard=%d", byFirstInitCard, bySecondInitCard);
	}
	return false;
}

///机器人输赢自动控制,
bool CServerGameDesk::AiWinAutoCtrl()
{
	//根据m_pooInfo值确定概率
	//int	m_iAIWantWinMoneyPercent;   //每张桌子，机器人赢钱概率，0~1000以内的值
	//int	m_iHumanFailMoneyPercent;   //每张桌子，真人输钱概率,0~1000以内的值
	//int	m_iGiveHumanMoneyPercent;   //赠送玩家金币概率,0~1000以内的值
	//int	m_iRobotGetMoneyPercent;	//机器人贪钱概率，0~1000
	int roomType = GetRoomType();
	if (roomType != ROOM_TYPE_GOLD)
	{
		return false;
	}
	//if (m_pooInfo.platformCtrlType == 0)
	//{
	//	m_bSystemWin = true;
	//	int iGradeNum = 5;
	//	if (m_pooInfo.rewardsCount > 0 && m_pooInfo.rewardsCount <= 1000)
	//	{
	//		iGradeNum = m_pooInfo.rewardsCount;
	//	}
	//	int iGrade = 1000/iGradeNum;
	//	if (m_pooInfo.gameWinMoney <= m_pooInfo.rewards1)
	//	{
	//		m_iAIWantWinMoneyPercent = 1000;
	//	}
	//	else if (m_pooInfo.gameWinMoney > m_pooInfo.rewards1 && m_pooInfo.gameWinMoney <= m_pooInfo.rewards2)
	//	{
	//		m_iAIWantWinMoneyPercent = 1000 - iGrade;
	//	}
	//	else if (m_pooInfo.gameWinMoney > m_pooInfo.rewards2 && m_pooInfo.gameWinMoney <= m_pooInfo.rewards3)
	//	{
	//		m_iAIWantWinMoneyPercent = 1000 - 2 * iGrade;
	//	}
	//	else if (m_pooInfo.gameWinMoney > m_pooInfo.rewards3 && m_pooInfo.gameWinMoney <= m_pooInfo.rewards4)
	//	{
	//		m_iAIWantWinMoneyPercent = 1000 - 3 * iGrade;
	//	}
	//	else if (m_pooInfo.gameWinMoney > m_pooInfo.rewards4 && m_pooInfo.gameWinMoney <= m_pooInfo.rewards5)
	//	{
	//		m_iAIWantWinMoneyPercent = 1000 - 4 * iGrade;
	//	}
	//	else
	//	{
	//		m_iAIWantWinMoneyPercent = 0;
	//	}
	//}
	//else
	//{
	//	int iTempWinPer = m_pooInfo.platformCtrlPercent;
	//	if (m_pooInfo.gameWinMoney <= m_pooInfo.platformCtrlValue)
	//	{
	//		m_bSystemWin = true;
	//	}
	//	else
	//	{
	//		m_bSystemWin = false;
	//	}

	//	//智能设置参数
	//	if (m_pooInfo.platformCtrlValue >= 0)
	//	{
	//		if (m_pooInfo.platformCtrlPercent == 1000)
	//		{
	//			if (m_pooInfo.gameWinMoney == 0)
	//			{
	//				iTempWinPer = 270;
	//			}
	//		}
	//		else if (m_pooInfo.gameWinMoney >= m_pooInfo.platformCtrlValue)
	//		{
	//			iTempWinPer = 1000;
	//		}
	//		else 
	//		{
	//			if (m_pooInfo.gameWinMoney <= -m_pooInfo.platformCtrlValue)
	//			{
	//				iTempWinPer = 650;
	//			}
	//			else if (m_pooInfo.gameWinMoney <= -m_pooInfo.platformCtrlValue/4)
	//			{
	//				iTempWinPer = 440;
	//			}
	//			else if (m_pooInfo.gameWinMoney <= 4*m_pooInfo.platformCtrlValue/5)
	//			{
	//				iTempWinPer = 270;
	//			}
	//			else
	//			{
	//				iTempWinPer = 420;
	//			}
	//		}

	//		//设置当前概率
	//		SetServerRoomPoolData(iTempWinPer);
	//	}

	//	m_iAIWantWinMoneyPercent = iTempWinPer;
	//}

	m_iAIWantWinMoneyPercent = m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent;

	if (m_iAIWantWinMoneyPercent >= 0)
	{
		m_bSystemWin = true;
	}
	else
	{
		m_bSystemWin = false;
	}

	m_iRobotGetMoneyPercent = 80;

	//首先去判断 这一桌是否既有机器人又有真人 
	if (m_iRobotCount > 0)
	{
		//本局是否自动控制
		if (CUtil::GetRandNum() % 1000 >= abs(m_iAIWantWinMoneyPercent))
		{
			return false;
		}

		////////////////////////////////////本局系统必赢或者必输//////////////////////////////////////
		if (GM_ONE_WIN == m_tRoomRuler.iGameModel)
		{
			if (m_bSystemWin)
			{
				int iRobotDesk_ = m_byRobot[CUtil::GetRandNum() % m_iRobotCount];
				BYTE byTmpMax = 255;
				FindMax(byTmpMax);
				if (byTmpMax != iRobotDesk_)
				{
					Change2UserCard(byTmpMax, iRobotDesk_);
				}
			}
			else
			{
				int iHumanDesk_ = m_byHuman[CUtil::GetRandNum() % m_iHumanCount];
				BYTE byTmpMax = 255;
				FindMax(byTmpMax);
				if (byTmpMax != iHumanDesk_)
				{
					Change2UserCard(byTmpMax, iHumanDesk_);
				}
			}
			return true;
		}
		else if (GM_FREE_NT == m_tRoomRuler.iGameModel)
		{
			AiRobNtAutoCtrl(false);
			return true;
		}
		else if (GM_SHOW_NT == m_tRoomRuler.iGameModel)
		{
			//////////////复杂算法//////////////
			AiRobNtAutoCtrl(true);
			return true;
		}
	}
	return false;
}

//抢庄模式下控制输赢(分明牌和不明牌)，主要控制自由抢庄和明牌抢庄
void CServerGameDesk::AiRobNtAutoCtrl(bool bMingPai)
{
	if (!m_bCallScoreFinish || m_byUpGradePeople == 255)
	{
		return;
	}

	//判断是机器人座庄还是真人坐庄
	bool IsUpGradeRobot = false;
	for (int i = 0; i < m_iRobotCount; i++)
	{
		if (m_byRobot[i] == m_byUpGradePeople)
		{
			IsUpGradeRobot = true;
			break;
		}
	}

	//机器人做庄
	if (IsUpGradeRobot)
	{
		//根据真人的数量，让机器人必赢算法
		switch (m_iHumanCount)
		{
		case 1:
		{
			int iValue = m_bSystemWin ? 1 : -1;
			if (m_Logic.NewCompareCard(m_byUserCard[m_byHuman[0]], SH_USER_CARD, m_byUserCard[m_byUpGradePeople], SH_USER_CARD) == iValue)
			{
				if (bMingPai)
				{
					if (m_bSystemWin)
					{
						Change2UserCardEx(m_byHuman[0], m_byUpGradePeople);
					}
					else
					{
						Change2UserCardEx(m_byUpGradePeople, m_byHuman[0]);
					}
				}
				else
				{
					Change2UserCard(m_byHuman[0], m_byUpGradePeople);
				}
			}
		}
		break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		{
			BYTE byDesk[PLAY_COUNT];
			memcpy(byDesk, m_byHuman, sizeof(byDesk));
			byDesk[m_iHumanCount] = m_byUpGradePeople;
			int iWinScore = 0;
			//计算输赢分数
			CountLocal(byDesk, m_iHumanCount + 1, iWinScore);
			if (m_bSystemWin)
			{
				if (iWinScore >= 0)
				{
					break;
				}
			}
			else
			{
				if (iWinScore <= 0)
				{
					break;
				}
			}

			int iAllSize[PLAY_COUNT];
			memset(iAllSize, 0, sizeof(iAllSize));
			FindAllSize(byDesk, m_iHumanCount + 1, iAllSize);

			if (m_bSystemWin) //系统赢
			{
				//调整机器人手牌
				for (int i = iAllSize[m_iHumanCount] + 1; i <= m_iHumanCount; i++)
				{
					BYTE bySation_ = 255;
					for (int j = 0; j < m_iHumanCount + 1; j++)
					{
						if (iAllSize[j] == i)
						{
							bySation_ = byDesk[j];
							break;
						}
					}
					if (bySation_ == 255)
					{
						continue;
					}
					if (bMingPai)
					{
						if (!Change2UserCardEx(bySation_, m_byUpGradePeople))
						{
							continue;
						}
					}
					else
					{
						Change2UserCard(bySation_, m_byUpGradePeople);
					}

					iWinScore = 0;
					//计算输赢分数
					CountLocal(byDesk, m_iHumanCount + 1, iWinScore);
					if (iWinScore >= 0 && (bMingPai || !bMingPai && CUtil::GetRandNum() % 1000 > m_iRobotGetMoneyPercent))
					{
						break;
					}
				}
			}
			else //系统输
			{
				//调整机器人手牌
				for (int i = iAllSize[m_iHumanCount] - 1; i >= 0; i--)
				{
					BYTE bySation_ = 255;
					for (int j = 0; j < m_iHumanCount + 1; j++)
					{
						if (iAllSize[j] == i)
						{
							bySation_ = byDesk[j];
							break;
						}
					}
					if (bySation_ == 255)
					{
						continue;
					}
					if (bMingPai)
					{
						if (!Change2UserCardEx(m_byUpGradePeople, bySation_))
						{
							continue;
						}
					}
					else
					{
						Change2UserCard(bySation_, m_byUpGradePeople);
					}

					iWinScore = 0;
					//计算输赢分数
					CountLocal(byDesk, m_iHumanCount + 1, iWinScore);
					if (iWinScore <= 0 && (bMingPai || !bMingPai && CUtil::GetRandNum() % 1000 > m_iRobotGetMoneyPercent))
					{
						break;
					}
				}
			}
		}
		break;
		default:
			break;
		}
	}
	else ///////////本局真人座庄//////////////
	{
		//根据机器人数量，让机器人赢或者输
		switch (m_iRobotCount)
		{
		case 1:
		{
			int iValue = m_bSystemWin ? -1 : 1;
			if (m_Logic.NewCompareCard(m_byUserCard[m_byRobot[0]], SH_USER_CARD, m_byUserCard[m_byUpGradePeople], SH_USER_CARD) == iValue)
			{
				if (bMingPai)
				{
					if (m_bSystemWin)
					{
						Change2UserCardEx(m_byUpGradePeople, m_byRobot[0]);
					}
					else
					{
						Change2UserCardEx(m_byRobot[0], m_byUpGradePeople);
					}
				}
				else
				{
					Change2UserCard(m_byRobot[0], m_byUpGradePeople);
				}
			}
		}
		break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		{
			BYTE byDesk[PLAY_COUNT];
			memcpy(byDesk, m_byRobot, sizeof(byDesk));
			byDesk[m_iRobotCount] = m_byUpGradePeople;
			int iWinScore = 0;
			//计算输赢分数
			CountLocal(byDesk, m_iRobotCount + 1, iWinScore);
			if (m_bSystemWin)
			{
				if (iWinScore <= 0)
				{
					break;
				}
			}
			else
			{
				if (iWinScore >= 0)
				{
					break;
				}
			}

			int iAllSize[PLAY_COUNT];
			memset(iAllSize, 0, sizeof(iAllSize));
			FindAllSize(byDesk, m_iRobotCount + 1, iAllSize);

			if (m_bSystemWin)
			{
				//调整真人手牌
				for (int i = iAllSize[m_iRobotCount] - 1; i >= 0; i--)
				{
					BYTE bySation_ = 255;
					for (int j = 0; j < m_iRobotCount + 1; j++)
					{
						if (iAllSize[j] == i)
						{
							bySation_ = byDesk[j];
							break;
						}
					}
					if (bySation_ == 255)
					{
						continue;
					}
					if (bMingPai)
					{
						if (!Change2UserCardEx(m_byUpGradePeople, bySation_))
						{
							continue;
						}
					}
					else
					{
						Change2UserCard(bySation_, m_byUpGradePeople);
					}

					iWinScore = 0;
					//计算输赢分数
					CountLocal(byDesk, m_iRobotCount + 1, iWinScore);
					if (iWinScore <= 0 && (bMingPai || !bMingPai && CUtil::GetRandNum() % 1000 > m_iRobotGetMoneyPercent))
					{
						break;
					}
				}
			}
			else
			{
				//调整真人手牌
				for (int i = iAllSize[m_iRobotCount] + 1; i <= m_iRobotCount; i++)
				{
					BYTE bySation_ = 255;
					for (int j = 0; j < m_iRobotCount + 1; j++)
					{
						if (iAllSize[j] == i)
						{
							bySation_ = byDesk[j];
							break;
						}
					}
					if (bySation_ == 255)
					{
						continue;
					}
					if (bMingPai)
					{
						if (!Change2UserCardEx(bySation_, m_byUpGradePeople))
						{
							continue;
						}
					}
					else
					{
						Change2UserCard(bySation_, m_byUpGradePeople);
					}

					iWinScore = 0;
					//计算输赢分数
					CountLocal(byDesk, m_iRobotCount + 1, iWinScore);
					if (iWinScore >= 0 && (bMingPai || !bMingPai && CUtil::GetRandNum() % 1000 > m_iRobotGetMoneyPercent))
					{
						break;
					}
				}
			}
		}
		break;
		default:
			break;
		}
	}
}

//只找最大
void CServerGameDesk::FindMax(BYTE &byTmpMax)
{
	byTmpMax = 255;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (STATE_NULL != m_byUserStation[i])
		{
			if (255 == byTmpMax)
			{
				byTmpMax = i;
				break;
			}
		}
	}

	//找最大
	if (byTmpMax != 255)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (i == byTmpMax)
			{
				continue;
			}
			if (STATE_NULL != m_byUserStation[i])
			{
				if (m_Logic.NewCompareCard(m_byUserCard[i], SH_USER_CARD, m_byUserCard[byTmpMax], SH_USER_CARD) == 1)
				{
					byTmpMax = i;
				}
			}
		}
	}
}

//获得所有牌的大小，按照索引排序
void CServerGameDesk::FindAllSize(BYTE byDeak[], int iCount, int iAllSize[])
{
	for (int i = 0; i < iCount - 1; i++)
	{
		for (int j = i + 1; j < iCount; j++)
		{
			int iDesk1 = byDeak[i];
			int iDesk2 = byDeak[j];
			if (STATE_NULL != m_byUserStation[iDesk1] && STATE_NULL != m_byUserStation[iDesk2])
			{
				if (m_Logic.NewCompareCard(m_byUserCard[iDesk1], SH_USER_CARD, m_byUserCard[iDesk2], SH_USER_CARD) == 1)
				{
					iAllSize[i]++;
				}
				else
				{
					iAllSize[j]++;
				}
			}
		}
	}
}

//从一组数据中计算庄家得分
void CServerGameDesk::CountLocal(BYTE byDeak[], int iCount, int &iWinScore)
{
	if (!m_bCallScoreFinish || m_byUpGradePeople == 255)
	{
		return;
	}
	iWinScore = 0;
	__int64 i64TempMoney = 0, iNtLoseMoney = 0;
	for (int i = 0; i < iCount; i++)
	{
		int j = byDeak[i];
		if (m_byUserStation[j] == STATE_NULL)
		{
			continue;
		}
		i64TempMoney = 0;
		if (j != m_byUpGradePeople)
		{
			int Shape = 0;
			int winer = -1;
			if (m_Logic.NewCompareCard(m_byUserCard[j], SH_USER_CARD, m_byUserCard[m_byUpGradePeople], SH_USER_CARD) == 1)
			{
				winer = j;
			}
			else
			{
				winer = m_byUpGradePeople;
			}
			Shape = m_Logic.GetShape(m_byUserCard[winer], SH_USER_CARD);
			if (winer == m_byUpGradePeople)
			{
				iNtLoseMoney += m_byCardShapeBase[Shape] * m_i64PerJuTotalNote[j];
			}
			else
			{
				iNtLoseMoney -= m_byCardShapeBase[Shape] * m_i64PerJuTotalNote[j];
			}
		}
	}
	iWinScore = (int)iNtLoseMoney;
}

//回收赢金币数量很多的玩家
void CServerGameDesk::AutoRecoverMoney()
{
	if (CUtil::GetRandNum() % 1000 < m_iHumanFailMoneyPercent)
	{
		BYTE byDesk[PLAY_COUNT];
		int iDeskCount = 0;
		memset(byDesk, 255, sizeof(byDesk));
		//判断是机器人座庄还是真人坐庄
		bool IsUpGradeHuman = false;
		for (int i = 0; i < m_iHumanCount; i++)
		{
			if (m_byHuman[i] == m_byUpGradePeople)
			{
				IsUpGradeHuman = true;
				break;
			}
		}
		if (m_byUpGradePeople != 255 && IsUpGradeHuman && IsRecoverPlayer(m_byUpGradePeople))
		{
			byDesk[iDeskCount++] = m_byUpGradePeople;
		}
		else
		{
			for (int i = 0; i < m_iHumanCount; i++)
			{
				if (IsRecoverPlayer(m_byHuman[i]))
				{
					byDesk[iDeskCount++] = m_byHuman[i];
				}
			}
		}

		//没有满足条件的真人玩家
		if (iDeskCount <= 0)
		{
			return;
		}
		BYTE byTmpMin = 255;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (STATE_NULL != m_byUserStation[i])
			{
				if (255 == byTmpMin)
				{
					byTmpMin = i;
					break;
				}
			}
		}
		//找最小
		if (byTmpMin != 255)
		{
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (i == byTmpMin)
				{
					continue;
				}
				if (STATE_NULL != m_byUserStation[i])
				{
					if (m_Logic.NewCompareCard(m_byUserCard[i], SH_USER_CARD, m_byUserCard[byTmpMin], SH_USER_CARD) != 1)
					{
						byTmpMin = i;
					}
				}
			}
		}
		BYTE byHuman = byDesk[CUtil::GetRandNum() % iDeskCount];
		//把最小的牌给玩家
		if (byTmpMin != byHuman)
		{
			Change2UserCard(byTmpMin, byHuman);
		}
	}
}

//让符合一定条件的玩家赢钱
void CServerGameDesk::AutoGiveMoney()
{
	if (CUtil::GetRandNum() % 1000 < m_iGiveHumanMoneyPercent)
	{
		BYTE byDesk[PLAY_COUNT];
		int iDeskCount = 0;
		memset(byDesk, 255, sizeof(byDesk));
		for (int i = 0; i < m_iHumanCount; i++)
		{
			if (IsWinMoneyPlayer(m_byHuman[i]))
			{
				byDesk[iDeskCount++] = m_byHuman[i];
			}
		}

		//没有满足条件的真人玩家
		if (iDeskCount <= 0)
		{
			return;
		}
		BYTE byTmpMax = 255;
		FindMax(byTmpMax);
		BYTE byHuman = byDesk[CUtil::GetRandNum() % iDeskCount];
		//把最小的牌给玩家
		if (byTmpMax != byHuman)
		{
			Change2UserCard(byTmpMax, byHuman);
		}
	}
}

//这个玩家是否满足回收条件
bool CServerGameDesk::IsRecoverPlayer(BYTE byDeskStation)
{
	GameUserInfo user;
	if (GetUserData(byDeskStation, user) && (user.userStatus&USER_IDENTITY_TYPE_FAIL) == USER_IDENTITY_TYPE_FAIL)
	{
		return true;
	}
	return false;
}

//这个玩家是否满足赢钱条件
bool CServerGameDesk::IsWinMoneyPlayer(BYTE byDeskStation)
{
	GameUserInfo user;
	if (GetUserData(byDeskStation, user) && (user.userStatus&USER_IDENTITY_TYPE_WIN) == USER_IDENTITY_TYPE_WIN)
	{
		return true;
	}
	return false;
}
