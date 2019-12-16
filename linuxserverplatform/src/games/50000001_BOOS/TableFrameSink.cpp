#include "TableFrameSink.h"


////////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////////// 
//定时器
// 鱼游时间
#define IDI_FISH_CREATE				10									// 鱼游时间
#define IDI_DELAY					11									// 延迟时间
#define IDI_SECOND					12									// 秒计时
#define TIME_FUSH_BOSS				13									//BOSS刷新公告
#define TIME_WRITE_SCORE            14                                 //延时写分定时器

#define TIME_DELAY					10000								// 延迟时间






// 消息定义
#define	IPC_PACKAGE					4096 

// 鱼位置信息
#define FISH_POS_NEXT				0									// 下一个
#define FISH_POS_DELETE				1									// 删除当前
#define FISH_POS_NORMAL				2									// 正常

// 度数转弧度
#define AngleToRadian( Angle )     ( (float)(Angle) / 360.f ) * ( GL_PI * 2 )

// 记录转换
#define NumberToString( String, nSize, Number )	 tchar String[nSize] = { "" }; _sntprintf( String, nSize, TEXT("%I64d"), (LONGLONG)Number );	

////////////////////////////////////////////////////////////////////////////////// 

CWHArray< tagFishPath * >	CTableFrameSink::m_ArrayFishPathPositive;			// 鱼路径保存(正)

// 库存控制
LONGLONG					CTableFrameSink::m_nRoomWrite = 0;
double						CTableFrameSink::m_dDartStock = 0.0;
double						CTableFrameSink::m_dWholeTax[Multiple_Max] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };	// 全部税收
double						CTableFrameSink::m_dRoomStock[Multiple_Max] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };	// 房间库存
double *					CTableFrameSink::m_pdTableStock[Multiple_Max] = { NULL, NULL, NULL, NULL, NULL, NULL };
//CMapPersonalDifficulty		CTableFrameSink::m_MapPersonalDifficulty;

double CTableFrameSink::m_dTaxRatio;
// 空箱几率
int CTableFrameSink::m_nNullChance;
// 赠送几率
int CTableFrameSink::m_nGiftChance[5];
// 赠送金币
int CTableFrameSink::m_nGiftScore[5];
// 加速几率
int CTableFrameSink::m_nSpeedChance;
// 加速时间
int CTableFrameSink::m_nSpeedTime;
// 激光几率
int CTableFrameSink::m_nLaserChance;
// 激光时间
int CTableFrameSink::m_nLaserTime;
// 补给条件
int CTableFrameSink::m_nSupplyCondition[2];
// 子弹冷却
int CTableFrameSink::m_nBulletCoolingTime;
// 子弹速度
int CTableFrameSink::m_nBulletVelocity;
// 场景维持时间
int CTableFrameSink::m_nSceneTime;
// 创建鱼数量
int CTableFrameSink::m_nCreateCount;
//鱼配置(倍数)
int CTableFrameSink::m_nDifficultyStart[Difficulty_Type];
int CTableFrameSink::m_nRoomDifficultyCount[Difficulty_Max];
int CTableFrameSink::m_nTableDifficultyCount[Difficulty_Max];
int CTableFrameSink::m_nPlayDifficultyCount[Difficulty_Max];
double CTableFrameSink::m_dRoomDifficultyValue[Difficulty_Max];
double CTableFrameSink::m_dTableDifficultyValue[Difficulty_Max];
double CTableFrameSink::m_dPlayDifficultyValue[Difficulty_Max];
tagCustomRule CTableFrameSink::m_CustomRule;
// 爆炸比例
int CTableFrameSink::m_nExplosionProportion;
// 爆炸启动
int CTableFrameSink::m_nExplosionStart;
// 爆炸条件
LONGLONG CTableFrameSink::m_lExplosionCondition;
// 条件类型
EnumExplosionConditionType CTableFrameSink::m_nExplosionConditionType;
map<uint, LONGLONG> CTableFrameSink::m_MapPlayExplosionCondition;
LONGLONG CTableFrameSink::m_lStockInitial;
LONGLONG CTableFrameSink::m_lStockCurrent;
  


//初始化游戏
bool CTableFrameSink::InitDeskGameStation()
{
	IsBegin = false;
	IniConfig();
	return true;
}

// 构造函数
CTableFrameSink::CTableFrameSink() : CGameDesk(ALL_ARGEE)
{
	// 游戏变量
	m_bFirstTime = true;
	m_bFishSwimming = false;
	m_nAppearFishCount = 0;
	ZeroMemory( m_nFishMultiple, sizeof(m_nFishMultiple) );
	
	// 创建信息
	m_nCreateCount = 0;

	// 场景信息
	m_bCompetitionRoom = false;
	m_bTimedCompetitionRoom = false;
	m_cbBackIndex = 0;
	m_bBackExchange = false;
	m_nSceneTime = 0;
	m_nSceneBegin = 0;

	// 房间倍数
	ZeroMemory(m_nMultipleValue, sizeof(m_nMultipleValue));

	// 子弹
	m_nBulletVelocity = 10000;
	m_nBulletCoolingTime = 100;
	
	// 玩家信息
	ZeroMemory(m_dwPlayID,sizeof(m_dwPlayID));
	ZeroMemory(m_lPlayScore,sizeof(m_lPlayScore));
	ZeroMemory(m_lPlayStartScore,sizeof(m_lPlayStartScore));
	ZeroMemory(m_lplayCutMoney,sizeof(m_lplayCutMoney));
	ZeroMemory(m_lFireCount, sizeof(m_lFireCount));
	ZeroMemory(m_lBulletConsume,sizeof(m_lBulletConsume));
	ZeroMemory(m_nPlayDownTime,sizeof(m_nPlayDownTime));
	ZeroMemory(m_nRobotPlayTime, sizeof(m_nRobotPlayTime));
	ZeroMemory(m_nPlayDelay,sizeof(m_nPlayDelay));
	ZeroMemory(m_lPlayFishCount,sizeof(m_lPlayFishCount));
	ZeroMemory(m_nMultipleIndex,sizeof(m_nMultipleIndex));

	// 玩家补给
	ZeroMemory(m_nSupplyCondition,sizeof(m_nSupplyCondition));
	ZeroMemory(m_nSupplyValue,sizeof(m_nSupplyValue));
	ZeroMemory(m_nEnergyValue,sizeof(m_nEnergyValue));
	ZeroMemory(m_bPlaySupply,sizeof(m_bPlaySupply));
	ZeroMemory(m_nLaserPlayTime,sizeof(m_nLaserPlayTime));
	ZeroMemory(m_nSpeedPlayTime,sizeof(m_nSpeedPlayTime));

	// 鱼信息
	m_nStartTime = 0;

	// 阵列
	m_nFishTeamTime = 0;
	m_nFishKingTime = 30;
	ZeroMemory(m_nFishKing, sizeof(m_nFishKing));

	// 补给箱控制
	m_nLaserTime = 0;
	m_nLaserChance = 0;
	m_nSpeedTime = 0;
	m_nSpeedChance = 0;
	ZeroMemory(m_nGiftScore, sizeof(m_nGiftScore));
	ZeroMemory(m_nGiftChance, sizeof(m_nGiftChance));
	m_nNullChance = 0;

	// 库存
	m_dTaxRatio = 0.0;
	ZeroMemory(m_dPlayStock, sizeof(m_dPlayStock));


	return;
}

// 析构函数
CTableFrameSink::~CTableFrameSink()
{
	// 释放接口
//	ReleaseInterface( m_hControlInst, m_pIServerControl );
	for ( int nIndex = 0; nIndex < Multiple_Max; ++nIndex )
		SafeDeleteArray( m_pdTableStock[nIndex] );
	
}

// 释放对象
VOID CTableFrameSink::Release()
{
	for(UINT i = 0; i < m_ArrayFishPathPositive.GetCount(); ++i )
	{
		if( m_ArrayFishPathPositive[i] )
		{
			for(UINT j = 0; j < m_ArrayFishPathPositive[i]->ArrayBezierPoint.GetCount(); ++j )
			{

				if( m_ArrayFishPathPositive[i]->ArrayBezierPoint[j] )
				{
					delete m_ArrayFishPathPositive[i]->ArrayBezierPoint[j];
					m_ArrayFishPathPositive[i]->ArrayBezierPoint[j] = NULL;
				}
			}
			m_ArrayFishPathPositive[i]->ArrayBezierPoint.RemoveAll();
			delete m_ArrayFishPathPositive[i];
			m_ArrayFishPathPositive[i] = NULL;
		}
	}

	m_ArrayFishPathPositive.RemoveAll();

	delete this;
}

void CTableFrameSink::IniConfig()
{
	m_FishKingTime = 0;
	m_bFushBoss = false;
	m_bIsFushBoss = false;
	m_bIsFushBegin = false;
	//房间倍数
	m_nMultipleValue[0] = 1;
	m_nMultipleValue[1] = 10;
	m_nMultipleValue[2] = 100;
	m_nMultipleValue[3] = 500;
	m_nMultipleValue[4] = 1000;
	m_nMultipleValue[5] = 5000;

	//房间库存 暂时没用
	for (int nIndex = 0; nIndex < Multiple_Max; ++nIndex)
	{
		m_dRoomStock[nIndex] =(10 *(double)m_nMultipleValue[nIndex]);
		m_lStockInitial += (10* (LONGLONG)m_nMultipleValue[nIndex]);
		m_lStockCurrent += (10* (LONGLONG)m_nMultipleValue[nIndex]);
	}

	//难度类型
	m_nDifficultyStart[0] = TRUE;   //房间难度开启
	m_nDifficultyStart[1] = false;  //桌子难度
	m_nDifficultyStart[1] = false;  //椅子难度

	//房间难度
	m_nRoomDifficultyCount[0] = 50;
	m_nRoomDifficultyCount[1] = 100;
	m_nRoomDifficultyCount[2] = 200;
	m_nRoomDifficultyCount[3] = 400;
	m_nRoomDifficultyCount[4] = 800;
	m_nRoomDifficultyCount[5] = 1200;
	m_nRoomDifficultyCount[6] = 2000;
	m_nRoomDifficultyCount[7] = 3000;
	m_nRoomDifficultyCount[8] = 6000;
	m_nRoomDifficultyCount[9] = 10000;

	//桌子难度
	m_nTableDifficultyCount[0] = 50;
	m_nTableDifficultyCount[1] = 100;
	m_nTableDifficultyCount[2] = 200;
	m_nTableDifficultyCount[3] = 400;
	m_nTableDifficultyCount[4] = 800;
	m_nTableDifficultyCount[5] = 1200;
	m_nTableDifficultyCount[6] = 2000;
	m_nTableDifficultyCount[7] = 3000;
	m_nTableDifficultyCount[8] = 6000;
	m_nTableDifficultyCount[9] = 10000;

	//椅子难度
	m_nPlayDifficultyCount[0] = -10000;
	m_nPlayDifficultyCount[1] = -1000;
	m_nPlayDifficultyCount[2] = -200;
	m_nPlayDifficultyCount[3] = -100;
	m_nPlayDifficultyCount[4] = -50;
	m_nPlayDifficultyCount[5] = 0;
	m_nPlayDifficultyCount[6] = 50;
	m_nPlayDifficultyCount[7] = 200;
	m_nPlayDifficultyCount[8] = 5000;
	m_nPlayDifficultyCount[9] = 10000;

	//房间难度系数(用到有用)
	m_dRoomDifficultyValue[0] = 0.05;
	m_dRoomDifficultyValue[1] = 0.15;
	m_dRoomDifficultyValue[2] = 0.25;
	m_dRoomDifficultyValue[3] = 0.50;
	m_dRoomDifficultyValue[4] = 0.75;
	m_dRoomDifficultyValue[5] = 1.00;
	m_dRoomDifficultyValue[6] = 1.25;
	m_dRoomDifficultyValue[7] = 1.70;
	m_dRoomDifficultyValue[8] = 2.50;
	m_dRoomDifficultyValue[9] = 3.50;

	//桌子难度系数
	m_dTableDifficultyValue[0] = 0.05;
	m_dTableDifficultyValue[1] = 0.15;
	m_dTableDifficultyValue[2] = 0.25;
	m_dTableDifficultyValue[3] = 0.50;
	m_dTableDifficultyValue[4] = 0.75;
	m_dTableDifficultyValue[5] = 1.00;
	m_dTableDifficultyValue[6] = 2.50;
	m_dTableDifficultyValue[7] = 2.75;
	m_dTableDifficultyValue[8] = 3.00;
	m_dTableDifficultyValue[9] = 3.50;

	//椅子难度系数
	m_dPlayDifficultyValue[0] = 2.50;
	m_dPlayDifficultyValue[1] = 2.15;
	m_dPlayDifficultyValue[2] = 1.75;
	m_dPlayDifficultyValue[3] = 1.15;
	m_dPlayDifficultyValue[4] = 0.80;
	m_dPlayDifficultyValue[5] = 0.60;
	m_dPlayDifficultyValue[6] = 0.40;
	m_dPlayDifficultyValue[7] = 0.20;
	m_dPlayDifficultyValue[8] = 0.15;
	m_dPlayDifficultyValue[9] = 0.10;

	//抽水比例
	m_dTaxRatio = 0.3;

	//场景配置
	m_nCreateCount = 12; 
	m_nSceneTime = 60 * 8;

	// 子弹信息
	m_nBulletCoolingTime = 150;
	m_nBulletVelocity = 15000;

	// 活动配置
	m_nSupplyCondition[0] = 500;
	m_nSupplyCondition[1] = 700;
	m_nLaserTime = 30;
	m_nLaserChance = 10;
	m_nSpeedTime = 60;
	m_nSpeedChance = 60;
	m_nGiftScore[0] = 10;
	m_nGiftScore[1] = 30;
	m_nGiftScore[2] = 50;
	m_nGiftScore[3] = 80;
	m_nGiftScore[4] = 120;
	m_nGiftChance[0] = 5;
	m_nGiftChance[1] = 5;
	m_nGiftChance[2] = 5;
	m_nGiftChance[3] = 5;
	m_nGiftChance[4] = 5;
	m_nNullChance = 5;

     //鱼配置
	m_nFishMultiple[0][0] = 2;
	m_nFishMultiple[0][1] = 2;
	m_nFishMultiple[1][0] = 3;
	m_nFishMultiple[1][1] = 3;
	m_nFishMultiple[2][0] = 4;
	m_nFishMultiple[2][1] = 4;
	m_nFishMultiple[3][0] = 5;
	m_nFishMultiple[3][1] = 5;
	m_nFishMultiple[4][0] = 6;
	m_nFishMultiple[4][1] = 6;
	m_nFishMultiple[5][0] = 7;
	m_nFishMultiple[5][1] = 7;
	m_nFishMultiple[6][0] = 8;
	m_nFishMultiple[6][1] = 8;
	m_nFishMultiple[7][0] = 9;
	m_nFishMultiple[7][1] = 9;
	m_nFishMultiple[8][0] = 10;
	m_nFishMultiple[8][1] = 10;
	m_nFishMultiple[9][0] = 12;
	m_nFishMultiple[9][1] = 12;
	m_nFishMultiple[10][0] = 15;
	m_nFishMultiple[10][1] = 15;
	m_nFishMultiple[11][0] = 18;
	m_nFishMultiple[11][1] = 18;
	m_nFishMultiple[12][0] = 20;
	m_nFishMultiple[12][1] = 20;
	m_nFishMultiple[13][0] = 25;
	m_nFishMultiple[13][1] = 25;
	m_nFishMultiple[14][0] = 30;
	m_nFishMultiple[14][1] = 30;
	m_nFishMultiple[15][0] = 40;
	m_nFishMultiple[15][1] = 40;
	m_nFishMultiple[16][0] = 150;
	m_nFishMultiple[16][1] = 150;
	m_nFishMultiple[17][0] = 200;
	m_nFishMultiple[17][1] = 200;
	m_nFishMultiple[18][0] = 300;
	m_nFishMultiple[18][1] = 300;
	m_nFishMultiple[19][0] = 400;
	m_nFishMultiple[19][1] = 400;
	m_nFishMultiple[20][0] = 400;
	m_nFishMultiple[20][1] = 1000;
	m_nFishMultiple[21][0] = 20;
	m_nFishMultiple[21][1] = 20;
	m_nFishMultiple[22][0] = 20;
	m_nFishMultiple[22][1] = 20;
	m_nFishMultiple[23][0] = 20;
	m_nFishMultiple[23][1] = 20;
	m_nFishMultiple[24][0] = 0;
	m_nFishMultiple[24][1] = 0;

	// 爆炸配置
	m_nExplosionProportion = 50;
	m_nExplosionStart = 5000;
	m_lExplosionCondition = 3000;
	m_nExplosionConditionType = ExplosionConditionType_Gun;
	m_nExplosionProportion = Max_(m_nExplosionProportion, 0);
	m_nExplosionProportion = Min_(m_nExplosionProportion, 1000);

	// 中奖配置
	m_nAwardMinMultiple = 50;
	m_bAwardChatBox = true;

	// 添加桌子库存
	for (int nIndex = 0; nIndex < Multiple_Max; ++nIndex)
	{
		if (m_pdTableStock[nIndex] == NULL)
		{
			m_pdTableStock[nIndex] = new double[6];
			ZeroMemory(m_pdTableStock[nIndex], sizeof(double) * 6);
		}
	}

	

#ifdef _DEBUG

	// 测试给爆炸
	int nMultipleMax = 0;
	for (int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex)
	{
		nMultipleMax = Max_(nMultipleMax, m_nMultipleValue[nMultipleIndex]);
	}

	// 创建飞镖
	m_dDartStock = nMultipleMax * 5000;

#endif

	// 获取当前路径
	TCHAR szDirectoryPath[MAX_PATH + 1] = { "" };
	strcpy(szDirectoryPath, CINIFile::GetAppPath().c_str());
	
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
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
		CON_ERROR_LOG("roomID 错误");
		return;
	}

	// 读取鱼路径
	if (m_ArrayFishPathPositive.GetCount() == 0)
	{
		// 打开配置
		tchar szFishMermaidPath[MAX_PATH] = "";
		sprintf(szFishMermaidPath, "%sFishLKPath.pat", szDirectoryPath);

		// 载入路径
		fstream f(szFishMermaidPath);

		// 读取信息
		string strReadPrior;
		int nStrCount = 10;
		int nPathCount = 0;
		BOOL bHand = FALSE;
		BOOL bBeging = TRUE;
		int jD = 0;
		while (std::getline(f, strReadPrior))
		{
			// 判断是否明文
			if (bBeging && strReadPrior.length() == 9)
			{
				// 头信息比较
				TCHAR cHand[9] = { 1, 9, 8, 9, 7, 3, 2, 2, 'S' };
				for (; jD < strReadPrior.length() && jD < 9; ++jD)
				{
					if (strReadPrior.at(jD) != cHand[jD])
					{
						break;
					}
				}

				if (jD == 9)
				{
					bHand = TRUE;
					bBeging = FALSE;
					continue;
				}
			}
			bBeging = FALSE;

			// 最后信息
			string strRead;

			// 被加密
			if (bHand)
			{
				int nPriorCount = strReadPrior.length();
				TCHAR* pData = new TCHAR[nPriorCount + 1];
				for (int jD = 0; jD < nPriorCount; ++jD)
				{
					pData[jD] = strReadPrior.at(jD);
					if (pData[jD] != '\n')
						pData[jD] -= nStrCount % 99;

					nStrCount++;
				}
				pData[nPriorCount] = '\0';
				nStrCount++;
				strRead = pData;
				delete[] pData;
			}
			else
			{
				strRead = strReadPrior;
			}


			// 转换多字节
			char szReadByte[1024];
			strcpy(szReadByte, strRead.c_str());

			if (szReadByte[0] == 'B')
			{
				int nPathIndexT = 0;
				int nPathCountT = 0;
				sscanf(szReadByte, "B PathIndex:%d PathCount:%d", &nPathIndexT, &nPathCountT);

				// 保存成节点
				tagFishPath* pNode = new tagFishPath;
				m_ArrayFishPathPositive.Add(pNode);
			}
			else if (szReadByte[0] == 'S' && nPathCount < (int)m_ArrayFishPathPositive.GetCount())
			{
				CPoint BeginPoint;
				CPoint EndPoint;
				CPoint KeyOne;
				CPoint KeyTwo;
				int	   nTime;
				sscanf(szReadByte, "S BeginPoint:%d,%d EndPoint:%d,%d KeyOne:%d,%d KeyTwo:%d,%d Time:%d",
					&BeginPoint.x, &BeginPoint.y, &EndPoint.x, &EndPoint.y,
					&KeyOne.x, &KeyOne.y, &KeyTwo.x, &KeyTwo.y, &nTime);

				// 保存成节点
				tagBezierPoint* pFishPath = new tagBezierPoint;
				pFishPath->BeginPoint = BeginPoint;
				pFishPath->EndPoint = EndPoint;
				pFishPath->KeyOne = KeyOne;
				pFishPath->KeyTwo = KeyTwo;
				pFishPath->Time = nTime;
				m_ArrayFishPathPositive[nPathCount]->ArrayBezierPoint.Add(pFishPath);
			}
			else if (strRead =="End")
			{
				// 数量增加
				nPathCount++;
			}
			else
			{
				CON_ERROR_LOG("FishLKPath.pat 文件格式不正确");
				ASSERT(FALSE);
			}
		}

		f.close();
	}

	// 玩家位置
	m_PointPlay[S_TOP_LEFT].x = 230;
	m_PointPlay[S_TOP_LEFT].y = 50;
	m_PointPlay[S_TOP_CENTER].x = DEFAULE_WIDTH / 2;
	m_PointPlay[S_TOP_CENTER].y = 50;
	m_PointPlay[S_TOP_RIGHT].x = DEFAULE_WIDTH - 230;
	m_PointPlay[S_TOP_RIGHT].y = 50;
	m_PointPlay[S_BOTTOM_LEFT].x = 230;
	m_PointPlay[S_BOTTOM_LEFT].y = DEFAULE_HEIGHT - 23;
	//m_PointPlay[S_BOTTOM_CENTER].x = DEFAULE_WIDTH / 2;
	//m_PointPlay[S_BOTTOM_CENTER].y = DEFAULE_HEIGHT - 50;
//	m_PointPlay[S_BOTTOM_RIGHT].x = DEFAULE_WIDTH - 230;
//	m_PointPlay[S_BOTTOM_RIGHT].y = DEFAULE_HEIGHT - 50;
	return;
}

// 配置桌子
void CTableFrameSink::LoadDynamicConfig()
{
	string nameID = std::to_string(NAME_ID);
	CINIFile f(CINIFile::GetAppPath() + nameID + "_s.ini");

	string key = "game";

	//获取倍率
	int Multiple = GetPlatformMultiple();

	//子弹间隔
	m_nBulletCoolingTime = f.GetKeyVal(key, "nBulletCoolingTime", 150);
	//子弹速度
	m_nBulletVelocity = f.GetKeyVal(key, "nBulletVelocity", 15000);
	//创建鱼数量
	m_nCreateCount = f.GetKeyVal(key, "nCreateCount", 12);
	//场景维持时间
	m_nSceneTime = f.GetKeyVal(key, "nSceneTime", 480);
	//增加人数鱼增加数量
	m_iAddFinshCount = f.GetKeyVal(key, "AddFinshCount", 6);
	//鱼群数量
	m_iFinshLeve = f.GetKeyVal(key, "FinshLeve", 3);
	//是否开启不操作会被踢出
	m_IsOpenTick = f.GetKeyVal(key, "IsOpenTick", 1);
	//玩家多久不操作会被踢出房间(秒)
	m_iTickTime = f.GetKeyVal(key, "TickTime", 120);
	//大于多少索引的鱼可以爆出特殊道具
	m_ipropNum = f.GetKeyVal(key, "propNum", 23);
	m_iBingDong = f.GetKeyVal(key, "BingDong", 20);
	m_iLocking = f.GetKeyVal(key, "Locking", 20);
	m_iRate	= f.GetKeyVal(key, "Rate", 5);
	m_iRobotTickTime = f.GetKeyVal(key, "RobotTickTime", 60);

	if (m_iFinshLeve > 3 || m_iFinshLeve < 0)
	{
		m_iFinshLeve = 3;
	}
	//大鱼群是否开启
	m_bOpenShoalFish= f.GetKeyVal(key, "OpenFishShoal", 1);
	//大鱼群数量
	m_iFishShoalLev = f.GetKeyVal(key, "FishShoalLev", 10);
	//小于次倍数不公告
	m_nAwardMinMultiple = f.GetKeyVal(key, "AwardMinMultiple", 30);

	//鱼阵类型
	m_iFinshGroup = f.GetKeyVal(key, "FinshGroup", 255);
	if (m_iFinshGroup < 0 || m_iFinshGroup > 4)
	{
		m_iFinshGroup = 255;
	}
	//小鱼鱼群创建几率
	m_iCreatJiLv = f.GetKeyVal(key, "CreatJiLv", 10);
	if (m_iCreatJiLv < 0 || m_iCreatJiLv >= 90)
	{
		m_iCreatJiLv = 60;
	}

	m_FishKingIntervalTime = f.GetKeyVal(key, "FishKingIntervalTime", 120);
	if (m_FishKingIntervalTime > 0)
	{
		m_FishKingIntervalTime = m_FishKingIntervalTime * 1000;
	}
	else
	{
		m_FishKingIntervalTime = 120 * 1000;
	}


	m_dRoomDifficultyValue[0] = f.GetKeyVal(key, "dRoomDifficultyValue_1_0", 5);
	m_dRoomDifficultyValue[1] = f.GetKeyVal(key, "dRoomDifficultyValue_1_1", 15);
	m_dRoomDifficultyValue[2] = f.GetKeyVal(key, "dRoomDifficultyValue_1_2", 25);
	m_dRoomDifficultyValue[3] = f.GetKeyVal(key, "dRoomDifficultyValue_1_3", 50);
	m_dRoomDifficultyValue[4] = f.GetKeyVal(key, "dRoomDifficultyValue_1_4", 75);
	m_dRoomDifficultyValue[5] = f.GetKeyVal(key, "dRoomDifficultyValue_1_5", 100);
	m_dRoomDifficultyValue[6] = f.GetKeyVal(key, "dRoomDifficultyValue_1_6", 125);
	m_dRoomDifficultyValue[7] = f.GetKeyVal(key, "dRoomDifficultyValue_1_7", 170);
	m_dRoomDifficultyValue[8] = f.GetKeyVal(key, "dRoomDifficultyValue_1_8", 250);
	m_dRoomDifficultyValue[9] = f.GetKeyVal(key, "dRoomDifficultyValue_1_9", 350);

	//个人难度系数
	m_dPlayDifficultyValue[9] = f.GetKeyVal(key, "dPlayDifficultyValue_1_9", 250);
	m_dPlayDifficultyValue[8] = f.GetKeyVal(key, "dPlayDifficultyValue_1_8", 215);
	m_dPlayDifficultyValue[7] = f.GetKeyVal(key, "dPlayDifficultyValue_1_7", 175);
	m_dPlayDifficultyValue[6] = f.GetKeyVal(key, "dPlayDifficultyValue_1_6", 115);
	m_dPlayDifficultyValue[5] = f.GetKeyVal(key, "dPlayDifficultyValue_1_5", 80);
	m_dPlayDifficultyValue[4] = f.GetKeyVal(key, "dPlayDifficultyValue_1_4", 60);
	m_dPlayDifficultyValue[3] = f.GetKeyVal(key, "dPlayDifficultyValue_1_3", 40);
	m_dPlayDifficultyValue[2] = f.GetKeyVal(key, "dPlayDifficultyValue_1_2", 20);
	m_dPlayDifficultyValue[1] = f.GetKeyVal(key, "dPlayDifficultyValue_1_1", 15);
	m_dPlayDifficultyValue[0] = f.GetKeyVal(key, "dPlayDifficultyValue_1_0", 10);
	for (int i = 0;i<10;i++)
	{
		if (m_dRoomDifficultyValue[i] > 1000 && m_dRoomDifficultyValue[i] <= 0)
		{
			m_dRoomDifficultyValue[i] = 1;
			continue;
		}
		m_dRoomDifficultyValue[i] = m_dRoomDifficultyValue[i] / 100;
	}
	for (int i = 0; i < 10; i++)
	{
		if (m_dPlayDifficultyValue[i] > 1000 && m_dPlayDifficultyValue[i] <= 0)
		{
			m_dPlayDifficultyValue[i] = 1;
			continue;
		}
		m_dPlayDifficultyValue[i] = m_dPlayDifficultyValue[i] / 100;
	}
	if (GetRoomLevel() == 1)
	{
		m_nMultipleValue[0] = f.GetKeyVal(key, "MultipleValue_1_0", 1) * Multiple;
		m_nMultipleValue[1] = f.GetKeyVal(key, "MultipleValue_1_1", 10) * Multiple;
		m_nMultipleValue[2] = f.GetKeyVal(key, "MultipleValue_1_2", 100) * Multiple;
		m_nMultipleValue[3] = f.GetKeyVal(key, "MultipleValue_1_3", 500) * Multiple;
		m_nMultipleValue[4] = f.GetKeyVal(key, "MultipleValue_1_4", 1000) * Multiple;
		m_nMultipleValue[5] = f.GetKeyVal(key, "MultipleValue_1_5", 5000) * Multiple;
		m_nMultipleValue[6] = f.GetKeyVal(key, "MultipleValue_1_6", 6000) * Multiple;
		m_nMultipleValue[7] = f.GetKeyVal(key, "MultipleValue_1_7", 7000) * Multiple;
		m_nMultipleValue[8] = f.GetKeyVal(key, "MultipleValue_1_8", 8000) * Multiple;
		m_nMultipleValue[9] = f.GetKeyVal(key, "MultipleValue_1_9", 10000) * Multiple;

		m_nFishMultiple[0][0] = f.GetKeyVal(key, "m_nFishMultiple_1_0_0", 2);
		m_nFishMultiple[0][1] = f.GetKeyVal(key, "m_nFishMultiple_1_0_1", 2);
		m_nFishMultiple[1][0] = f.GetKeyVal(key, "m_nFishMultiple_1_1_0", 2);
		m_nFishMultiple[1][1] = f.GetKeyVal(key, "m_nFishMultiple_1_1_1", 2);
		m_nFishMultiple[2][0] = f.GetKeyVal(key, "m_nFishMultiple_1_2_0", 3);
		m_nFishMultiple[2][1] = f.GetKeyVal(key, "m_nFishMultiple_1_2_1", 3);
		m_nFishMultiple[3][0] = f.GetKeyVal(key, "m_nFishMultiple_1_3_0", 3);
		m_nFishMultiple[3][1] = f.GetKeyVal(key, "m_nFishMultiple_1_3_1", 3);
		m_nFishMultiple[4][0] = f.GetKeyVal(key, "m_nFishMultiple_1_4_0", 4);
		m_nFishMultiple[4][1] = f.GetKeyVal(key, "m_nFishMultiple_1_4_1", 4);
		m_nFishMultiple[5][0] = f.GetKeyVal(key, "m_nFishMultiple_1_5_0", 4);
		m_nFishMultiple[5][1] = f.GetKeyVal(key, "m_nFishMultiple_1_5_1", 4);
		m_nFishMultiple[6][0] = f.GetKeyVal(key, "m_nFishMultiple_1_6_0", 5);
		m_nFishMultiple[6][1] = f.GetKeyVal(key, "m_nFishMultiple_1_6_1", 5);
		m_nFishMultiple[7][0] = f.GetKeyVal(key, "m_nFishMultiple_1_7_0", 5);
		m_nFishMultiple[7][1] = f.GetKeyVal(key, "m_nFishMultiple_1_7_0", 5);
		m_nFishMultiple[8][0] = f.GetKeyVal(key, "m_nFishMultiple_1_8_0", 6);
		m_nFishMultiple[8][1] = f.GetKeyVal(key, "m_nFishMultiple_1_8_1", 6);
		m_nFishMultiple[9][0] = f.GetKeyVal(key, "m_nFishMultiple_1_9_0", 6);
		m_nFishMultiple[9][1] = f.GetKeyVal(key, "m_nFishMultiple_1_9_1", 6);
		m_nFishMultiple[10][0] = f.GetKeyVal(key, "m_nFishMultiple_1_10_0", 6);
		m_nFishMultiple[10][1] = f.GetKeyVal(key, "m_nFishMultiple_1_10_1", 6);
		m_nFishMultiple[11][0] = f.GetKeyVal(key, "m_nFishMultiple_1_11_0", 7);
		m_nFishMultiple[11][1] = f.GetKeyVal(key, "m_nFishMultiple_1_11_1", 7);
		m_nFishMultiple[12][0] = f.GetKeyVal(key, "m_nFishMultiple_1_12_0", 8);
		m_nFishMultiple[12][1] = f.GetKeyVal(key, "m_nFishMultiple_1_12_1", 8);
		m_nFishMultiple[13][0] = f.GetKeyVal(key, "m_nFishMultiple_1_13_0", 8);
		m_nFishMultiple[13][1] = f.GetKeyVal(key, "m_nFishMultiple_1_13_1", 8);
		m_nFishMultiple[14][0] = f.GetKeyVal(key, "m_nFishMultiple_1_14_0", 9);
		m_nFishMultiple[14][1] = f.GetKeyVal(key, "m_nFishMultiple_1_14_1", 9);
		m_nFishMultiple[15][0] = f.GetKeyVal(key, "m_nFishMultiple_1_15_0", 10);
		m_nFishMultiple[15][1] = f.GetKeyVal(key, "m_nFishMultiple_1_15_1", 10);
		m_nFishMultiple[16][0] = f.GetKeyVal(key, "m_nFishMultiple_1_16_0", 12);
		m_nFishMultiple[16][1] = f.GetKeyVal(key, "m_nFishMultiple_1_16_1", 12);
		m_nFishMultiple[17][0] = f.GetKeyVal(key, "m_nFishMultiple_1_17_0", 15);
		m_nFishMultiple[17][1] = f.GetKeyVal(key, "m_nFishMultiple_1_17_1", 15);
		m_nFishMultiple[18][0] = f.GetKeyVal(key, "m_nFishMultiple_1_18_0", 20);
		m_nFishMultiple[18][1] = f.GetKeyVal(key, "m_nFishMultiple_1_18_1", 20);
		m_nFishMultiple[19][0] = f.GetKeyVal(key, "m_nFishMultiple_1_19_0", 25);
		m_nFishMultiple[19][1] = f.GetKeyVal(key, "m_nFishMultiple_1_19_1", 25);
		m_nFishMultiple[20][0] = f.GetKeyVal(key, "m_nFishMultiple_1_20_0", 30);
		m_nFishMultiple[20][1] = f.GetKeyVal(key, "m_nFishMultiple_1_20_1", 30);
		m_nFishMultiple[21][0] = f.GetKeyVal(key, "m_nFishMultiple_1_21_0", 30);
		m_nFishMultiple[21][1] = f.GetKeyVal(key, "m_nFishMultiple_1_21_1", 30);
		m_nFishMultiple[22][0] = f.GetKeyVal(key, "m_nFishMultiple_1_22_0", 30);
		m_nFishMultiple[22][1] = f.GetKeyVal(key, "m_nFishMultiple_1_22_1", 30);
		m_nFishMultiple[23][0] = f.GetKeyVal(key, "m_nFishMultiple_1_23_0", 50);
		m_nFishMultiple[23][1] = f.GetKeyVal(key, "m_nFishMultiple_1_23_1", 50);
		m_nFishMultiple[24][0] = f.GetKeyVal(key, "m_nFishMultiple_1_24_0", 50);
		m_nFishMultiple[24][1] = f.GetKeyVal(key, "m_nFishMultiple_1_24_1", 50);
		m_nFishMultiple[25][0] = f.GetKeyVal(key, "m_nFishMultiple_1_25_0", 80);
		m_nFishMultiple[25][1] = f.GetKeyVal(key, "m_nFishMultiple_1_25_1", 100);
		m_nFishMultiple[26][0] = f.GetKeyVal(key, "m_nFishMultiple_1_26_0", 80);
		m_nFishMultiple[26][1] = f.GetKeyVal(key, "m_nFishMultiple_1_26_1", 100);
		m_nFishMultiple[27][0] = f.GetKeyVal(key, "m_nFishMultiple_1_27_0", 120);
		m_nFishMultiple[27][1] = f.GetKeyVal(key, "m_nFishMultiple_1_27_1", 150);
		m_nFishMultiple[28][0] = f.GetKeyVal(key, "m_nFishMultiple_1_28_0", 120);
		m_nFishMultiple[28][1] = f.GetKeyVal(key, "m_nFishMultiple_1_28_1", 150);
		m_nFishMultiple[29][0] = f.GetKeyVal(key, "m_nFishMultiple_1_29_0", 400);
		m_nFishMultiple[29][1] = f.GetKeyVal(key, "m_nFishMultiple_1_29_1", 600);

		m_nFishMultiple[30][0] = f.GetKeyVal(key, "m_nFishMultiple_1_30_0", 400);
		m_nFishMultiple[30][1] = f.GetKeyVal(key, "m_nFishMultiple_1_30_1", 600);
		m_nFishMultiple[31][0] = f.GetKeyVal(key, "m_nFishMultiple_1_31_0", 400);
		m_nFishMultiple[31][1] = f.GetKeyVal(key, "m_nFishMultiple_1_31_1", 600);
		m_nFishMultiple[32][0] = f.GetKeyVal(key, "m_nFishMultiple_1_32_0", 400);
		m_nFishMultiple[32][1] = f.GetKeyVal(key, "m_nFishMultiple_1_32_1", 600);
	}
	else if (GetRoomLevel() == 2)
	{
		m_nMultipleValue[0] = f.GetKeyVal(key, "MultipleValue_2_0", 1) * Multiple;
		m_nMultipleValue[1] = f.GetKeyVal(key, "MultipleValue_2_1", 10) * Multiple;
		m_nMultipleValue[2] = f.GetKeyVal(key, "MultipleValue_2_2", 100) * Multiple;
		m_nMultipleValue[3] = f.GetKeyVal(key, "MultipleValue_2_3", 500) * Multiple;
		m_nMultipleValue[4] = f.GetKeyVal(key, "MultipleValue_2_4", 1000) * Multiple;
		m_nMultipleValue[5] = f.GetKeyVal(key, "MultipleValue_2_5", 5000) * Multiple;
		m_nMultipleValue[6] = f.GetKeyVal(key, "MultipleValue_2_6", 6000) * Multiple;
		m_nMultipleValue[7] = f.GetKeyVal(key, "MultipleValue_2_7", 7000) * Multiple;
		m_nMultipleValue[8] = f.GetKeyVal(key, "MultipleValue_2_8", 8000) * Multiple;
		m_nMultipleValue[9] = f.GetKeyVal(key, "MultipleValue_2_9", 10000) * Multiple;

		m_nFishMultiple[0][0] = f.GetKeyVal(key, "m_nFishMultiple_2_0_0", 2);
		m_nFishMultiple[0][1] = f.GetKeyVal(key, "m_nFishMultiple_2_0_1", 2);
		m_nFishMultiple[1][0] = f.GetKeyVal(key, "m_nFishMultiple_2_1_0", 2);
		m_nFishMultiple[1][1] = f.GetKeyVal(key, "m_nFishMultiple_2_1_1", 2);
		m_nFishMultiple[2][0] = f.GetKeyVal(key, "m_nFishMultiple_2_2_0", 3);
		m_nFishMultiple[2][1] = f.GetKeyVal(key, "m_nFishMultiple_2_2_1", 3);
		m_nFishMultiple[3][0] = f.GetKeyVal(key, "m_nFishMultiple_2_3_0", 3);
		m_nFishMultiple[3][1] = f.GetKeyVal(key, "m_nFishMultiple_2_3_1", 3);
		m_nFishMultiple[4][0] = f.GetKeyVal(key, "m_nFishMultiple_2_4_0", 4);
		m_nFishMultiple[4][1] = f.GetKeyVal(key, "m_nFishMultiple_2_4_1", 4);
		m_nFishMultiple[5][0] = f.GetKeyVal(key, "m_nFishMultiple_2_5_0", 4);
		m_nFishMultiple[5][1] = f.GetKeyVal(key, "m_nFishMultiple_2_5_1", 4);
		m_nFishMultiple[6][0] = f.GetKeyVal(key, "m_nFishMultiple_2_6_0", 5);
		m_nFishMultiple[6][1] = f.GetKeyVal(key, "m_nFishMultiple_2_6_1", 5);
		m_nFishMultiple[7][0] = f.GetKeyVal(key, "m_nFishMultiple_2_7_0", 5);
		m_nFishMultiple[7][1] = f.GetKeyVal(key, "m_nFishMultiple_2_7_0", 5);
		m_nFishMultiple[8][0] = f.GetKeyVal(key, "m_nFishMultiple_2_8_0", 6);
		m_nFishMultiple[8][1] = f.GetKeyVal(key, "m_nFishMultiple_2_8_1", 6);
		m_nFishMultiple[9][0] = f.GetKeyVal(key, "m_nFishMultiple_2_9_0", 6);
		m_nFishMultiple[9][1] = f.GetKeyVal(key, "m_nFishMultiple_2_9_1", 6);
		m_nFishMultiple[10][0] = f.GetKeyVal(key, "m_nFishMultiple_2_10_0", 6);
		m_nFishMultiple[10][1] = f.GetKeyVal(key, "m_nFishMultiple_2_10_1", 6);
		m_nFishMultiple[11][0] = f.GetKeyVal(key, "m_nFishMultiple_2_11_0", 7);
		m_nFishMultiple[11][1] = f.GetKeyVal(key, "m_nFishMultiple_2_11_1", 7);
		m_nFishMultiple[12][0] = f.GetKeyVal(key, "m_nFishMultiple_2_12_0", 8);
		m_nFishMultiple[12][1] = f.GetKeyVal(key, "m_nFishMultiple_2_12_1", 8);
		m_nFishMultiple[13][0] = f.GetKeyVal(key, "m_nFishMultiple_2_13_0", 8);
		m_nFishMultiple[13][1] = f.GetKeyVal(key, "m_nFishMultiple_2_13_1", 8);
		m_nFishMultiple[14][0] = f.GetKeyVal(key, "m_nFishMultiple_2_14_0", 9);
		m_nFishMultiple[14][1] = f.GetKeyVal(key, "m_nFishMultiple_2_14_1", 9);
		m_nFishMultiple[15][0] = f.GetKeyVal(key, "m_nFishMultiple_2_15_0", 10);
		m_nFishMultiple[15][1] = f.GetKeyVal(key, "m_nFishMultiple_2_15_1", 10);
		m_nFishMultiple[16][0] = f.GetKeyVal(key, "m_nFishMultiple_2_16_0", 12);
		m_nFishMultiple[16][1] = f.GetKeyVal(key, "m_nFishMultiple_2_16_1", 12);
		m_nFishMultiple[17][0] = f.GetKeyVal(key, "m_nFishMultiple_2_17_0", 15);
		m_nFishMultiple[17][1] = f.GetKeyVal(key, "m_nFishMultiple_2_17_1", 15);
		m_nFishMultiple[18][0] = f.GetKeyVal(key, "m_nFishMultiple_2_18_0", 20);
		m_nFishMultiple[18][1] = f.GetKeyVal(key, "m_nFishMultiple_2_18_1", 20);
		m_nFishMultiple[19][0] = f.GetKeyVal(key, "m_nFishMultiple_2_19_0", 25);
		m_nFishMultiple[19][1] = f.GetKeyVal(key, "m_nFishMultiple_2_19_1", 25);
		m_nFishMultiple[20][0] = f.GetKeyVal(key, "m_nFishMultiple_2_20_0", 30);
		m_nFishMultiple[20][1] = f.GetKeyVal(key, "m_nFishMultiple_2_20_1", 30);
		m_nFishMultiple[21][0] = f.GetKeyVal(key, "m_nFishMultiple_2_21_0", 30);
		m_nFishMultiple[21][1] = f.GetKeyVal(key, "m_nFishMultiple_2_21_1", 30);
		m_nFishMultiple[22][0] = f.GetKeyVal(key, "m_nFishMultiple_2_22_0", 30);
		m_nFishMultiple[22][1] = f.GetKeyVal(key, "m_nFishMultiple_2_22_1", 30);
		m_nFishMultiple[23][0] = f.GetKeyVal(key, "m_nFishMultiple_2_23_0", 50);
		m_nFishMultiple[23][1] = f.GetKeyVal(key, "m_nFishMultiple_2_23_1", 50);
		m_nFishMultiple[24][0] = f.GetKeyVal(key, "m_nFishMultiple_2_24_0", 50);
		m_nFishMultiple[24][1] = f.GetKeyVal(key, "m_nFishMultiple_2_24_1", 50);
		m_nFishMultiple[25][0] = f.GetKeyVal(key, "m_nFishMultiple_2_25_0", 80);
		m_nFishMultiple[25][1] = f.GetKeyVal(key, "m_nFishMultiple_2_25_1", 100);
		m_nFishMultiple[26][0] = f.GetKeyVal(key, "m_nFishMultiple_2_26_0", 80);
		m_nFishMultiple[26][1] = f.GetKeyVal(key, "m_nFishMultiple_2_26_1", 100);
		m_nFishMultiple[27][0] = f.GetKeyVal(key, "m_nFishMultiple_2_27_0", 120);
		m_nFishMultiple[27][1] = f.GetKeyVal(key, "m_nFishMultiple_2_27_1", 150);
		m_nFishMultiple[28][0] = f.GetKeyVal(key, "m_nFishMultiple_2_28_0", 120);
		m_nFishMultiple[28][1] = f.GetKeyVal(key, "m_nFishMultiple_2_28_1", 150);
		m_nFishMultiple[29][0] = f.GetKeyVal(key, "m_nFishMultiple_2_29_0", 400);
		m_nFishMultiple[29][1] = f.GetKeyVal(key, "m_nFishMultiple_2_29_1", 600);

		m_nFishMultiple[30][0] = f.GetKeyVal(key, "m_nFishMultiple_2_30_0", 400);
		m_nFishMultiple[30][1] = f.GetKeyVal(key, "m_nFishMultiple_2_30_1", 600);
		m_nFishMultiple[31][0] = f.GetKeyVal(key, "m_nFishMultiple_2_31_0", 400);
		m_nFishMultiple[31][1] = f.GetKeyVal(key, "m_nFishMultiple_2_31_1", 600);
		m_nFishMultiple[32][0] = f.GetKeyVal(key, "m_nFishMultiple_2_32_0", 400);
		m_nFishMultiple[32][1] = f.GetKeyVal(key, "m_nFishMultiple_2_32_1", 600);
	}
	else if (GetRoomLevel() == 3)
	{
		m_nMultipleValue[0] = f.GetKeyVal(key, "MultipleValue_3_0", 1) * Multiple;
		m_nMultipleValue[1] = f.GetKeyVal(key, "MultipleValue_3_1", 10) * Multiple;
		m_nMultipleValue[2] = f.GetKeyVal(key, "MultipleValue_3_2", 100) * Multiple;
		m_nMultipleValue[3] = f.GetKeyVal(key, "MultipleValue_3_3", 500) * Multiple;
		m_nMultipleValue[4] = f.GetKeyVal(key, "MultipleValue_3_4", 1000) * Multiple;
		m_nMultipleValue[5] = f.GetKeyVal(key, "MultipleValue_3_5", 5000) * Multiple;
		m_nMultipleValue[6] = f.GetKeyVal(key, "MultipleValue_3_6", 6000) * Multiple;
		m_nMultipleValue[7] = f.GetKeyVal(key, "MultipleValue_3_7", 7000) * Multiple;
		m_nMultipleValue[8] = f.GetKeyVal(key, "MultipleValue_3_8", 8000) * Multiple;
		m_nMultipleValue[9] = f.GetKeyVal(key, "MultipleValue_3_9", 10000) * Multiple;

		m_nFishMultiple[0][0] = f.GetKeyVal(key, "m_nFishMultiple_3_0_0", 2);
		m_nFishMultiple[0][1] = f.GetKeyVal(key, "m_nFishMultiple_3_0_1", 2);
		m_nFishMultiple[1][0] = f.GetKeyVal(key, "m_nFishMultiple_3_1_0", 2);
		m_nFishMultiple[1][1] = f.GetKeyVal(key, "m_nFishMultiple_3_1_1", 2);
		m_nFishMultiple[2][0] = f.GetKeyVal(key, "m_nFishMultiple_3_2_0", 3);
		m_nFishMultiple[2][1] = f.GetKeyVal(key, "m_nFishMultiple_3_2_1", 3);
		m_nFishMultiple[3][0] = f.GetKeyVal(key, "m_nFishMultiple_3_3_0", 3);
		m_nFishMultiple[3][1] = f.GetKeyVal(key, "m_nFishMultiple_3_3_1", 3);
		m_nFishMultiple[4][0] = f.GetKeyVal(key, "m_nFishMultiple_3_4_0", 4);
		m_nFishMultiple[4][1] = f.GetKeyVal(key, "m_nFishMultiple_3_4_1", 4);
		m_nFishMultiple[5][0] = f.GetKeyVal(key, "m_nFishMultiple_3_5_0", 4);
		m_nFishMultiple[5][1] = f.GetKeyVal(key, "m_nFishMultiple_3_5_1", 4);
		m_nFishMultiple[6][0] = f.GetKeyVal(key, "m_nFishMultiple_3_6_0", 5);
		m_nFishMultiple[6][1] = f.GetKeyVal(key, "m_nFishMultiple_3_6_1", 5);
		m_nFishMultiple[7][0] = f.GetKeyVal(key, "m_nFishMultiple_3_7_0", 5);
		m_nFishMultiple[7][1] = f.GetKeyVal(key, "m_nFishMultiple_3_7_0", 5);
		m_nFishMultiple[8][0] = f.GetKeyVal(key, "m_nFishMultiple_3_8_0", 6);
		m_nFishMultiple[8][1] = f.GetKeyVal(key, "m_nFishMultiple_3_8_1", 6);
		m_nFishMultiple[9][0] = f.GetKeyVal(key, "m_nFishMultiple_3_9_0", 6);
		m_nFishMultiple[9][1] = f.GetKeyVal(key, "m_nFishMultiple_3_9_1", 6);
		m_nFishMultiple[10][0] = f.GetKeyVal(key, "m_nFishMultiple_3_10_0", 6);
		m_nFishMultiple[10][1] = f.GetKeyVal(key, "m_nFishMultiple_3_10_1", 6);
		m_nFishMultiple[11][0] = f.GetKeyVal(key, "m_nFishMultiple_3_11_0", 7);
		m_nFishMultiple[11][1] = f.GetKeyVal(key, "m_nFishMultiple_3_11_1", 7);
		m_nFishMultiple[12][0] = f.GetKeyVal(key, "m_nFishMultiple_3_12_0", 8);
		m_nFishMultiple[12][1] = f.GetKeyVal(key, "m_nFishMultiple_3_12_1", 8);
		m_nFishMultiple[13][0] = f.GetKeyVal(key, "m_nFishMultiple_3_13_0", 8);
		m_nFishMultiple[13][1] = f.GetKeyVal(key, "m_nFishMultiple_3_13_1", 8);
		m_nFishMultiple[14][0] = f.GetKeyVal(key, "m_nFishMultiple_3_14_0", 9);
		m_nFishMultiple[14][1] = f.GetKeyVal(key, "m_nFishMultiple_3_14_1", 9);
		m_nFishMultiple[15][0] = f.GetKeyVal(key, "m_nFishMultiple_3_15_0", 10);
		m_nFishMultiple[15][1] = f.GetKeyVal(key, "m_nFishMultiple_3_15_1", 10);
		m_nFishMultiple[16][0] = f.GetKeyVal(key, "m_nFishMultiple_3_16_0", 12);
		m_nFishMultiple[16][1] = f.GetKeyVal(key, "m_nFishMultiple_3_16_1", 12);
		m_nFishMultiple[17][0] = f.GetKeyVal(key, "m_nFishMultiple_3_17_0", 15);
		m_nFishMultiple[17][1] = f.GetKeyVal(key, "m_nFishMultiple_3_17_1", 15);
		m_nFishMultiple[18][0] = f.GetKeyVal(key, "m_nFishMultiple_3_18_0", 20);
		m_nFishMultiple[18][1] = f.GetKeyVal(key, "m_nFishMultiple_3_18_1", 20);
		m_nFishMultiple[19][0] = f.GetKeyVal(key, "m_nFishMultiple_3_19_0", 25);
		m_nFishMultiple[19][1] = f.GetKeyVal(key, "m_nFishMultiple_3_19_1", 25);
		m_nFishMultiple[20][0] = f.GetKeyVal(key, "m_nFishMultiple_3_20_0", 30);
		m_nFishMultiple[20][1] = f.GetKeyVal(key, "m_nFishMultiple_3_20_1", 30);
		m_nFishMultiple[21][0] = f.GetKeyVal(key, "m_nFishMultiple_3_21_0", 30);
		m_nFishMultiple[21][1] = f.GetKeyVal(key, "m_nFishMultiple_3_21_1", 30);
		m_nFishMultiple[22][0] = f.GetKeyVal(key, "m_nFishMultiple_3_22_0", 30);
		m_nFishMultiple[22][1] = f.GetKeyVal(key, "m_nFishMultiple_3_22_1", 30);
		m_nFishMultiple[23][0] = f.GetKeyVal(key, "m_nFishMultiple_3_23_0", 50);
		m_nFishMultiple[23][1] = f.GetKeyVal(key, "m_nFishMultiple_3_23_1", 50);
		m_nFishMultiple[24][0] = f.GetKeyVal(key, "m_nFishMultiple_3_24_0", 50);
		m_nFishMultiple[24][1] = f.GetKeyVal(key, "m_nFishMultiple_3_24_1", 50);
		m_nFishMultiple[25][0] = f.GetKeyVal(key, "m_nFishMultiple_3_25_0", 80);
		m_nFishMultiple[25][1] = f.GetKeyVal(key, "m_nFishMultiple_3_25_1", 100);
		m_nFishMultiple[26][0] = f.GetKeyVal(key, "m_nFishMultiple_3_26_0", 80);
		m_nFishMultiple[26][1] = f.GetKeyVal(key, "m_nFishMultiple_3_26_1", 100);
		m_nFishMultiple[27][0] = f.GetKeyVal(key, "m_nFishMultiple_3_27_0", 120);
		m_nFishMultiple[27][1] = f.GetKeyVal(key, "m_nFishMultiple_3_27_1", 150);
		m_nFishMultiple[28][0] = f.GetKeyVal(key, "m_nFishMultiple_3_28_0", 120);
		m_nFishMultiple[28][1] = f.GetKeyVal(key, "m_nFishMultiple_3_28_1", 150);
		m_nFishMultiple[29][0] = f.GetKeyVal(key, "m_nFishMultiple_3_29_0", 400);
		m_nFishMultiple[29][1] = f.GetKeyVal(key, "m_nFishMultiple_3_29_1", 600);

		m_nFishMultiple[30][0] = f.GetKeyVal(key, "m_nFishMultiple_3_30_0", 400);
		m_nFishMultiple[30][1] = f.GetKeyVal(key, "m_nFishMultiple_3_30_1", 600);
		m_nFishMultiple[31][0] = f.GetKeyVal(key, "m_nFishMultiple_3_31_0", 400);
		m_nFishMultiple[31][1] = f.GetKeyVal(key, "m_nFishMultiple_3_31_1", 600);
		m_nFishMultiple[32][0] = f.GetKeyVal(key, "m_nFishMultiple_3_32_0", 400);
		m_nFishMultiple[32][1] = f.GetKeyVal(key, "m_nFishMultiple_3_32_1", 600);
	}
	else
	{
		m_nMultipleValue[0] = f.GetKeyVal(key, "MultipleValue_1_0", 1);
		m_nMultipleValue[1] = f.GetKeyVal(key, "MultipleValue_1_1", 10);
		m_nMultipleValue[2] = f.GetKeyVal(key, "MultipleValue_1_2", 100);
		m_nMultipleValue[3] = f.GetKeyVal(key, "MultipleValue_1_3", 500);
		m_nMultipleValue[4] = f.GetKeyVal(key, "MultipleValue_1_4", 1000);
		m_nMultipleValue[5] = f.GetKeyVal(key, "MultipleValue_1_5", 5000);
		m_nMultipleValue[6] = f.GetKeyVal(key, "MultipleValue_1_6", 6000);
		m_nMultipleValue[7] = f.GetKeyVal(key, "MultipleValue_1_7", 7000);
		m_nMultipleValue[8] = f.GetKeyVal(key, "MultipleValue_1_8", 8000);
		m_nMultipleValue[9] = f.GetKeyVal(key, "MultipleValue_1_9", 10000);

		m_nFishMultiple[0][0] = f.GetKeyVal(key, "m_nFishMultiple_3_0_0", 2);
		m_nFishMultiple[0][1] = f.GetKeyVal(key, "m_nFishMultiple_3_0_1", 2);
		m_nFishMultiple[1][0] = f.GetKeyVal(key, "m_nFishMultiple_3_1_0", 2);
		m_nFishMultiple[1][1] = f.GetKeyVal(key, "m_nFishMultiple_3_1_1", 2);
		m_nFishMultiple[2][0] = f.GetKeyVal(key, "m_nFishMultiple_3_2_0", 3);
		m_nFishMultiple[2][1] = f.GetKeyVal(key, "m_nFishMultiple_3_2_1", 3);
		m_nFishMultiple[3][0] = f.GetKeyVal(key, "m_nFishMultiple_3_3_0", 3);
		m_nFishMultiple[3][1] = f.GetKeyVal(key, "m_nFishMultiple_3_3_1", 3);
		m_nFishMultiple[4][0] = f.GetKeyVal(key, "m_nFishMultiple_3_4_0", 4);
		m_nFishMultiple[4][1] = f.GetKeyVal(key, "m_nFishMultiple_3_4_1", 4);
		m_nFishMultiple[5][0] = f.GetKeyVal(key, "m_nFishMultiple_3_5_0", 4);
		m_nFishMultiple[5][1] = f.GetKeyVal(key, "m_nFishMultiple_3_5_1", 4);
		m_nFishMultiple[6][0] = f.GetKeyVal(key, "m_nFishMultiple_3_6_0", 5);
		m_nFishMultiple[6][1] = f.GetKeyVal(key, "m_nFishMultiple_3_6_1", 5);
		m_nFishMultiple[7][0] = f.GetKeyVal(key, "m_nFishMultiple_3_7_0", 5);
		m_nFishMultiple[7][1] = f.GetKeyVal(key, "m_nFishMultiple_3_7_0", 5);
		m_nFishMultiple[8][0] = f.GetKeyVal(key, "m_nFishMultiple_3_8_0", 6);
		m_nFishMultiple[8][1] = f.GetKeyVal(key, "m_nFishMultiple_3_8_1", 6);
		m_nFishMultiple[9][0] = f.GetKeyVal(key, "m_nFishMultiple_3_9_0", 6);
		m_nFishMultiple[9][1] = f.GetKeyVal(key, "m_nFishMultiple_3_9_1", 6);
		m_nFishMultiple[10][0] = f.GetKeyVal(key, "m_nFishMultiple_3_10_0", 6);
		m_nFishMultiple[10][1] = f.GetKeyVal(key, "m_nFishMultiple_3_10_1", 6);
		m_nFishMultiple[11][0] = f.GetKeyVal(key, "m_nFishMultiple_3_11_0", 7);
		m_nFishMultiple[11][1] = f.GetKeyVal(key, "m_nFishMultiple_3_11_1", 7);
		m_nFishMultiple[12][0] = f.GetKeyVal(key, "m_nFishMultiple_3_12_0", 8);
		m_nFishMultiple[12][1] = f.GetKeyVal(key, "m_nFishMultiple_3_12_1", 8);
		m_nFishMultiple[13][0] = f.GetKeyVal(key, "m_nFishMultiple_3_13_0", 8);
		m_nFishMultiple[13][1] = f.GetKeyVal(key, "m_nFishMultiple_3_13_1", 8);
		m_nFishMultiple[14][0] = f.GetKeyVal(key, "m_nFishMultiple_3_14_0", 9);
		m_nFishMultiple[14][1] = f.GetKeyVal(key, "m_nFishMultiple_3_14_1", 9);
		m_nFishMultiple[15][0] = f.GetKeyVal(key, "m_nFishMultiple_3_15_0", 10);
		m_nFishMultiple[15][1] = f.GetKeyVal(key, "m_nFishMultiple_3_15_1", 10);
		m_nFishMultiple[16][0] = f.GetKeyVal(key, "m_nFishMultiple_3_16_0", 12);
		m_nFishMultiple[16][1] = f.GetKeyVal(key, "m_nFishMultiple_3_16_1", 12);
		m_nFishMultiple[17][0] = f.GetKeyVal(key, "m_nFishMultiple_3_17_0", 15);
		m_nFishMultiple[17][1] = f.GetKeyVal(key, "m_nFishMultiple_3_17_1", 15);
		m_nFishMultiple[18][0] = f.GetKeyVal(key, "m_nFishMultiple_3_18_0", 20);
		m_nFishMultiple[18][1] = f.GetKeyVal(key, "m_nFishMultiple_3_18_1", 20);
		m_nFishMultiple[19][0] = f.GetKeyVal(key, "m_nFishMultiple_3_19_0", 25);
		m_nFishMultiple[19][1] = f.GetKeyVal(key, "m_nFishMultiple_3_19_1", 25);
		m_nFishMultiple[20][0] = f.GetKeyVal(key, "m_nFishMultiple_3_20_0", 30);
		m_nFishMultiple[20][1] = f.GetKeyVal(key, "m_nFishMultiple_3_20_1", 30);
		m_nFishMultiple[21][0] = f.GetKeyVal(key, "m_nFishMultiple_3_21_0", 30);
		m_nFishMultiple[21][1] = f.GetKeyVal(key, "m_nFishMultiple_3_21_1", 30);
		m_nFishMultiple[22][0] = f.GetKeyVal(key, "m_nFishMultiple_3_22_0", 30);
		m_nFishMultiple[22][1] = f.GetKeyVal(key, "m_nFishMultiple_3_22_1", 30);
		m_nFishMultiple[23][0] = f.GetKeyVal(key, "m_nFishMultiple_3_23_0", 50);
		m_nFishMultiple[23][1] = f.GetKeyVal(key, "m_nFishMultiple_3_23_1", 50);
		m_nFishMultiple[24][0] = f.GetKeyVal(key, "m_nFishMultiple_3_24_0", 50);
		m_nFishMultiple[24][1] = f.GetKeyVal(key, "m_nFishMultiple_3_24_1", 50);
		m_nFishMultiple[25][0] = f.GetKeyVal(key, "m_nFishMultiple_3_25_0", 80);
		m_nFishMultiple[25][1] = f.GetKeyVal(key, "m_nFishMultiple_3_25_1", 100);
		m_nFishMultiple[26][0] = f.GetKeyVal(key, "m_nFishMultiple_3_26_0", 80);
		m_nFishMultiple[26][1] = f.GetKeyVal(key, "m_nFishMultiple_3_26_1", 100);
		m_nFishMultiple[27][0] = f.GetKeyVal(key, "m_nFishMultiple_3_27_0", 120);
		m_nFishMultiple[27][1] = f.GetKeyVal(key, "m_nFishMultiple_3_27_1", 150);
		m_nFishMultiple[28][0] = f.GetKeyVal(key, "m_nFishMultiple_3_28_0", 120);
		m_nFishMultiple[28][1] = f.GetKeyVal(key, "m_nFishMultiple_3_28_1", 150);
		m_nFishMultiple[29][0] = f.GetKeyVal(key, "m_nFishMultiple_3_29_0", 400);
		m_nFishMultiple[29][1] = f.GetKeyVal(key, "m_nFishMultiple_3_29_1", 600);

		m_nFishMultiple[30][0] = f.GetKeyVal(key, "m_nFishMultiple_3_30_0", 400);
		m_nFishMultiple[30][1] = f.GetKeyVal(key, "m_nFishMultiple_3_30_1", 600);
		m_nFishMultiple[31][0] = f.GetKeyVal(key, "m_nFishMultiple_3_31_0", 400);
		m_nFishMultiple[31][1] = f.GetKeyVal(key, "m_nFishMultiple_3_31_1", 600);
		m_nFishMultiple[32][0] = f.GetKeyVal(key, "m_nFishMultiple_3_32_0", 400);
		m_nFishMultiple[32][1] = f.GetKeyVal(key, "m_nFishMultiple_3_32_1", 600);
	}
	
	return;
}

// 复位桌子
VOID CTableFrameSink::RepositionSink()
{
	return;
}


// 游戏结束
bool CTableFrameSink::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	// 解散写分
	if (bCloseFlag == GF_NORMAL)
	{
		//设置状态
		SetGameStation(GAME_STATUS_FREE);

		// 清空玩家
		for ( int nSite = 0; nSite < PlayChair_Max; ++nSite )
		{
			// 判断用户
			long userID = GetUserIDByDeskStation(nSite);
			if( userID > 0 )
			{
				// 玩家指针
				GameUserInfo UserInfo;
				GetUserData(nSite,UserInfo);

				// 写分
				LONGLONG lScore = ( m_lPlayScore[nSite] - m_lPlayStartScore[nSite] );
				if ( lScore != 0 )
				{
					// 积分为负， 不能超过最大分数
					if ( lScore < 0 && (-lScore) > UserInfo.money)
					{
						lScore = -UserInfo.money;
					}
				}

				// 游戏时间
				DWORD dwEndTime = (DWORD)time(NULL) - m_nPlayDownTime[nSite];

				ChangeUserPoint(nSite, lScore, m_lplayCutMoney[nSite]);

				m_lplayCutMoney[nSite] = 0;

				// 保存写分
				m_nRoomWrite += (-lScore);

				// 归零
				InitializePlayer(nSite);
			}
		}

		// 全局爆炸还原
		if ( GetFishCount(FishType_BaoZhaFeiBiao) > 0 )
		{
			// 最大倍数
			int nMultipleMax = 0;
			for ( int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex )
			{
				nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nMultipleIndex] );
			}

			// 飞镖消耗
			m_dDartStock += nMultipleMax * m_nExplosionStart;
		}

		// 记录库存
		//FishMermaidStock();

		// 删除定时器
		KillTimer(IDI_FISH_CREATE);
		KillTimer(IDI_DELAY);
		KillTimer(IDI_SECOND);
		KillTimer(TIME_WRITE_SCORE);
		//清空玩家的所有个人控制
		//删除全部个人难度
		ClearPersonalDifficulty();

		// 鱼信息
		m_ArrayFishInfo.RemoveAll();
		m_cbBackIndex = 0;
		m_nAppearFishCount = 0;
		m_bFishSwimming = FALSE;
		m_bBackExchange = FALSE;
		m_nStartTime = 0;

		// 阵列
		m_nFishTeamTime = 0;
		m_nFishKingTime = 30;
		ZeroMemory(m_nFishKing, sizeof(m_nFishKing));

		IsBegin = false;
		return CGameDesk::GameFinish(bDeskStation, bCloseFlag);
	}
	// 清空玩家
	for (int nSite = 0; nSite < PlayChair_Max; ++nSite)
	{
		// 归零
		InitializePlayer(nSite);
	}

	IsBegin = false;
	return CGameDesk::GameFinish(bDeskStation, bCloseFlag);
}

 //发送场景
bool CTableFrameSink::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{

	BYTE cbGameStatus = GetGameStation();

	switch(cbGameStatus)
	{
	case GAME_STATUS_FREE:		
	case GAME_STATUS_PLAY:
		{



			// 基本信息
			GameScene Scene;
			ZeroMemory(&Scene, sizeof(Scene));
			Scene.lPlayScore = m_lPlayScore[bDeskStation];
			Scene.cbBackIndex = m_cbBackIndex;
			CopyMemory( Scene.lPlayCurScore, m_lPlayScore, sizeof(Scene.lPlayCurScore) );
			CopyMemory( Scene.lPlayStartScore, m_lPlayStartScore, sizeof(Scene.lPlayStartScore) );
			Scene.nBulletVelocity = m_nBulletVelocity;
			Scene.nBulletCoolingTime = m_nBulletCoolingTime;
			CopyMemory( Scene.nFishMultiple, m_nFishMultiple, sizeof(Scene.nFishMultiple) );
			CopyMemory( Scene.lBulletConsume, m_lBulletConsume, sizeof(Scene.lBulletConsume) );
			CopyMemory( Scene.lPlayFishCount, m_lPlayFishCount, sizeof(Scene.lPlayFishCount) );
			CopyMemory( Scene.nMultipleValue, m_nMultipleValue, sizeof(Scene.nMultipleValue) );
			CopyMemory( Scene.nMultipleIndex, m_nMultipleIndex, sizeof(Scene.nMultipleIndex) );
			Scene.bUnlimitedRebound = false;
			//_sntprintf(Scene.szBrowseUrl, CountArray(Scene.szBrowseUrl), TEXT("%s/Pay/PayIndex.aspx"),szPlatformLink);
			SendGameStation(bDeskStation, uSocketID, bWatchUser, &Scene, sizeof(Scene));
			//printf("子弹速度:%d,冷却:%d", Scene.nBulletVelocity, Scene.nBulletCoolingTime);
			// 发送鱼信息
			DWORD dwTime = GetSysMilliseconds();
			DWORD Time = dwTime - m_nStartTime;
		//	printf("%d,%d,%d\n", Time, dwTime, m_nStartTime);
			// 时间同步
			CMD_S_Synchronous CMDSSynchronous;
			CMDSSynchronous.nOffSetTime = dwTime - m_nStartTime;
			//printf("%d,%d,%d\n", CMDSSynchronous.nOffSetTime, dwTime, m_nStartTime);
		     SendTableData(bDeskStation, SUB_S_SYNCHRONOUS, &CMDSSynchronous, sizeof(CMDSSynchronous));

			//// 发送鱼
			//for( int nIndex = 0; nIndex < m_ArrayFishInfo.GetCount(); ++nIndex )
			//{
			//	 发送信息
			//	SendFish( m_ArrayFishInfo.GetAt( nIndex) );
			//}
			//
			return true;
		}
	}

	return true;
}

// 定时器事件
bool CTableFrameSink::OnTimer(UINT uTimerID)
{
	switch (uTimerID)
	{
		// 创建鱼定时
	case IDI_FISH_CREATE:
		{
			// 关闭定时器
		KillTimer(IDI_FISH_CREATE);
			
			// 检测是否有人
			bool bHavePeople = false;
			for ( WORD i = 0; i < PlayChair_Max; ++i )
			{
				long  GameUser = GetUserIDByDeskStation(i);
				if (GameUser <= 0) continue;
				//if(IsAndroidUser(i)) continue;
				
				bHavePeople = true;
				
				break;
			}
			
			// 无人返回
			if ( !bHavePeople )
			{
				// 全局爆炸还原
				if ( GetFishCount(FishType_BaoZhaFeiBiao) > 0 )
				{
					// 最大倍数
					int nMultipleMax = 0;
					for ( int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex )
					{
						nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nMultipleIndex] );
					}

					// 飞镖消耗
					m_dDartStock += nMultipleMax * m_nExplosionStart;
				}

				// 鱼信息
				m_ArrayFishInfo.RemoveAll();

				// 设置信息
				m_cbBackIndex = 0;
				m_nAppearFishCount = 0;
				m_bFishSwimming = FALSE;
				m_bBackExchange = FALSE;

				GameFinish(0, GF_NORMAL);

				return true;
			}

			// 计算当前鱼
			int nFishIndex = 0;
			while ( nFishIndex < m_ArrayFishInfo.GetCount() )
			{
				// 获取鱼信息
				tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt(nFishIndex);

				// 计算鱼位置
				CDoublePoint PointFish;
				if( FishMove(PointFish, TFishInfo) == FishMoveType_Delete )
				{
					// 重新创建
					if( TFishInfo.bRepeatCreate && !m_bBackExchange )
					{
						// 重新生成鱼
						ResetFish( TFishInfo );

						// 下一条
						++nFishIndex;
					}
					else
					{

						// 盒子爆炸
						if( TFishInfo.nFishType == FishType_BaoXiang && TFishInfo.wHitChair != INVALID_CHAIR )
						{
							// 活动重载
							m_bPlaySupply[TFishInfo.wHitChair] = false;
						}

						// 全局爆炸还原
						if ( TFishInfo.nFishType == FishType_BaoZhaFeiBiao )
						{
							// 最大倍数
							int nMultipleMax = 0;
							for ( int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex )
							{
								nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nMultipleIndex] );
							}

							// 飞镖消耗
							m_dDartStock += nMultipleMax * m_nExplosionStart;
						}

						// 删除鱼
						m_ArrayFishInfo.RemoveAt( nFishIndex );

						// 继续
						continue;
					}
				}

				// 下一条
				++nFishIndex;
			}

			// 当前时间
			const uint nCurrentTime = GetSysMilliseconds();

			// 换地图
			if( !m_bBackExchange && nCurrentTime - m_nSceneBegin > (uint)m_nSceneTime * 1000 && !m_bFushSence)
			{
				m_bBackExchange = TRUE;
				m_nSceneBegin = nCurrentTime;
			}

			// 等待换地图 不添加鱼
			if ( !m_bBackExchange )
			{
				// 添加鱼
				if ( m_nAppearFishCount == 0 && !m_bBackExchange && !m_bFirstTime )
				{
					// 创建阵列
					DWORD GroupTime = GroupOfFish();
					m_nFishTeamTime = GetTickCount() + GroupTime;
					//BOSS时间去除鱼阵时间
					m_FishKingTime += GroupTime;
				}
				else if ( !m_bBackExchange && ( GetTickCount() > m_nFishTeamTime || m_ArrayFishInfo.GetCount() == 0 ) )
				{
					// 过滤第一次
					m_bFirstTime = false;

					// 创建鱼群
					InitializationFishpond();

					// 最大倍数
					int nMultipleMax = 0;
					for ( int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex )
					{
						nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nMultipleIndex] );
					}

					// 创建飞镖
					if( m_dDartStock >= nMultipleMax * m_nExplosionStart && GetFishCount(FishType_BaoZhaFeiBiao) == 0 )
					{
						// 飞镖消耗
						m_dDartStock -= nMultipleMax * m_nExplosionStart;

						// 创建飞镖
						CreateFish( 1, FishType_BaoZhaFeiBiao );
					}
				}
			}
	
			// 换场景发送消息
			BOOL bChangeScene = FALSE;
			if ( m_bBackExchange && m_ArrayFishInfo.GetCount() < 5 )
			{
				m_cbBackIndex = (m_cbBackIndex + 1)%3;
				m_bBackExchange = FALSE;
				bChangeScene = TRUE;
				m_nAppearFishCount = 0;

				// 鱼快速消息
				FishFastMove();

				// 换图消息
				CMD_S_ExchangeScene ExchangeScene;
				ExchangeScene.cbBackIndex = m_cbBackIndex;

				// 发送消息
				SendTableData(INVALID_CHAIR, SUB_S_EXCHANGE_SCENE, &ExchangeScene, sizeof(ExchangeScene) );
			}

			// 启动定时器
			if ( bChangeScene )
			{
				SetTimer(IDI_FISH_CREATE, 5000);
			}
			else
			{
				SetTimer(IDI_FISH_CREATE, 200);
			}
		}
		return true;
		// 延迟定时
	case IDI_DELAY:
		{
			// 关闭定时器
			KillTimer(IDI_DELAY);

			// 检测是否有人
			bool bHavePeople = false;
			GameUserInfo *GameUser = NULL;
			for ( WORD i = 0; i < PlayChair_Max; ++i )
			{
				GameUser = GetTableUserItem(i);
				if (GameUser == NULL) continue;

				// 开启定时器
				bHavePeople = true;
				SetTimer(IDI_DELAY, TIME_DELAY);
				break;
			}

			// 发送消息
			for (WORD i = 0; i < PlayChair_Max && bHavePeople; ++i )
			{
				//IServerUserItem *pIServerUserItem = m_pITableFrame->GetTableUserItem( i );
				GameUser = GetTableUserItem(i);
				if (GameUser == NULL ) continue;
				//if ( !pIServerUserItem->IsClientReady() ) continue;

				m_nPlayDelay[i] = GetSysMilliseconds();
				SendUserItemData(GameUser, SUB_S_DELAY_BEGIN, NULL, NULL);
			}
		}
		return true;

		// 秒钟定时
	case IDI_SECOND:
		{
			// 关闭定时器
			KillTimer(IDI_SECOND);

			// 检测是否有人
			bool bHavePeople = false;
			WORD wSite = INVALID_CHAIR;
			GameUserInfo *GameUser = NULL;
			for ( WORD i = 0; i < PlayChair_Max; ++i )
			{
				GameUser = GetTableUserItem(i);
				if (GameUser == NULL) continue;

				// 开启定时器
				bHavePeople = true;
				SetTimer(IDI_SECOND, 1000);
				wSite = i;
				break;
			}
			
			// 无人无倍数
			if( !bHavePeople )
				return true;

			// 减少鱼阵时间
			if ( m_nFishKingTime > 0 )
			{
				m_nFishKingTime--;
			}

			// 减少鱼阵时间
			for( int nIndex = 0; nIndex < CountArray(m_nFishKing); ++nIndex )
			{
				if ( m_nFishKing[nIndex] > 0 )
				{
					m_nFishKing[nIndex]--;
				}
			}

			// 游戏时间
			DWORD dwCurrentime = (DWORD)time(NULL);

			uint NowTime = GetSysMilliseconds();
			//判断玩家多长时间不操作
			//if (m_IsOpenTick)
			//{
			//	for (int i = 0; i < PLAY_COUNT; i++)
			//	{
			//		if (GetUserIDByDeskStation(i) <= 0) continue;

			//		if (NowTime - m_iOptionTime[i] > m_iTickTime * 1000)
			//		{
			//			//调用踢出该玩家
			//			KickOutUser(i);
			//		}
			//	}
			//}

			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (GetUserIDByDeskStation(i) <= 0) continue;
				if (IsAndroidUser(i))
				{


				}
			}
			// 机器人游戏时间
			for( int nSite = 0; nSite < PlayChair_Max; ++nSite )
			{
				// 玩家接口
				//IServerUserItem * pServerRobot = m_pITableFrame->GetTableUserItem(nSite);

				if (IsAndroidUser(nSite))
				{
					//超时不操作踢出
					if (NowTime - m_iOptionTime[nSite] > m_iRobotTickTime * 1000)
					{
						//printf("机器人不操作踢出\n");
						//调用踢出该玩家
						KickOutUser(nSite, REASON_KICKOUT_STAND_MINLIMIT);
					}

					if (dwCurrentime - m_nPlayDownTime[nSite] > m_nRobotPlayTime[nSite])
					{
						//printf("机器人游戏超时被踢出:%d\n", m_nRobotPlayTime[nSite]);
						//调用踢出该玩家
						KickOutUser(nSite, REASON_KICKOUT_STAND_MINLIMIT);
					}
					else if(m_lPlayScore[nSite] < m_nMultipleValue[m_nMultipleIndex[nSite]])
					{
						printf("机器人钱不足被踢出，房间等级：%d**:%lld", GetRoomLevel(), m_lPlayScore[nSite]);
						printf(",%d,%d\n", m_nMultipleIndex[nSite], m_nMultipleValue[m_nMultipleIndex[nSite]]);
						//调用踢出该玩家
						KickOutUser(nSite, REASON_KICKOUT_STAND_MINLIMIT);
					}
				}
			}
			return true;
		}
	case TIME_WRITE_SCORE: //定时写分
		{
			//关闭定时器
			KillTimer(TIME_WRITE_SCORE);
			// 检测是否有人
			for (WORD i = 0; i < PlayChair_Max; ++i)
			{
				long  userid = GetUserIDByDeskStation(i);
				if (userid <= 0) continue;

				//有人那么就结算一下
				GameUserInfo UserInfo;
				GetUserData(i, UserInfo);

				// 写分
				LONGLONG lScore = (m_lPlayScore[i] - m_lPlayStartScore[i]);
				if (lScore != 0)
				{
					// 积分为负， 不能超过最大分数
					if (lScore < 0 && (-lScore) > UserInfo.money)
					{
						lScore = -UserInfo.money;
					}
				}

				// 游戏时间
				DWORD dwEndTime = (DWORD)time(NULL) - m_nPlayDownTime[i];
				ChangeUserPoint(i, lScore, m_lplayCutMoney[i]);
				m_lplayCutMoney[i] = 0;

				//初始化分
				GetUserData(i, UserInfo);
				m_lPlayScore[i] = UserInfo.money;
				m_lPlayStartScore[i] = UserInfo.money;

				//更新玩家的个人难度
				//设置玩家个人难度
				long Userid = GetUserIDByDeskStation(i);
				if (Userid > 0)
				{
					
					// 个人难度
					int UserDifficulty = GetUserControlParam(i);
					//printf("玩家：%d,个人难度：%d\n", Userid, UserDifficulty);
					double dGameDifficulty = 0;
					if (UserDifficulty >= 0 && UserDifficulty < 1000)
					{
						int Lev = UserDifficulty / 100;
						dGameDifficulty = m_dPlayDifficultyValue[Lev];
					}
					else if (UserDifficulty >= 1000)
					{
						dGameDifficulty = m_dPlayDifficultyValue[9];
					}
				//	printf("设置个人难度：%llf", dGameDifficulty);
					SetPersonalDifficulty(Userid, dGameDifficulty);
				}

				// 保存写分
				m_nRoomWrite += (-lScore);		
			}
			SetTimer(TIME_WRITE_SCORE, 30000);
			return true;
		}
	case TIME_FUSH_BOSS:
	{
		KillTimer(TIME_FUSH_BOSS);
		m_bFushBoss = true;
		return true;
	}
		return true;
	}
	return false;
}


//框架消息处理函数
bool CTableFrameSink::HandleFrameMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser)
{
	return CGameDesk::HandleFrameMessage(bDeskStation, assistID, pData, size, bWatchUser);
}


// 游戏消息
bool CTableFrameSink::HandleNotifyMessage(BYTE deskStation, unsigned int assistID, void* pData, int size, bool bWatchUser)
{
	// 异常检测
	try
	{
		//记录玩家操作时间
		m_iOptionTime[deskStation] = GetSysMilliseconds();

		// 消息处理
		bool bSuccess = false;
		switch(assistID)
		{
		case SUB_C_CATCH_FISH:
			bSuccess = OnSubCatchFish(deskStation, pData, size);
			break;
		case SUB_C_FIRE:
			bSuccess = OnSubFire(deskStation, pData, size);
			break;
		case SUB_C_BEGIN_LASER:
			bSuccess = OnSubBeginLaser(deskStation, pData, size);
			break;
		case SUB_C_LASER:
			bSuccess = OnSubLaser(deskStation, pData, size);
			break;
		case SUB_C_DELAY:
			bSuccess = OnSubDelay(deskStation, pData, size);
			break;
		case SUB_C_SPEECH:
			bSuccess = OnSubSpeech(deskStation, pData, size);
			break;
		case SUB_C_MULTIPLE:
			bSuccess = OnSubMultiple(deskStation, pData, size);
			break;
		case SUB_C_CONTROL:
			bSuccess = OnSubControl(deskStation, pData, size);
			break;
		case SUB_C_SKILL:
			bSuccess = OnSubSkill(deskStation, pData, size);
			break;
		case SUB_C_CHANGE_CONNON:
			bSuccess = OnChangeCannon(deskStation, pData, size);
			break;
		}

		// 判断执行状况
		if( !bSuccess )
			ERROR_LOG( " %d执行消息 [ %d ] 失败！", GetUserIDByDeskStation(deskStation),assistID);

		return CGameDesk::HandleNotifyMessage(deskStation, assistID, pData, size, bWatchUser);
	}
	catch ( CException * pException )
	{
		// 获取错误信息
		tchar szErrorMessage[1024] = { "" };
		pException->GetErrorMessage(szErrorMessage, 1024);
		pException->Delete();


		return true;
	}
	catch ( ... )
	{
		ERROR_LOG("严重错误 执行消息 [ %d ] 失败！", assistID);
		return true;
	}
	return CGameDesk::HandleNotifyMessage(deskStation, assistID, pData, size, bWatchUser);
}

 
 //用户坐下
bool CTableFrameSink::UserSitDeskActionNotify(BYTE deskStation)
{

	// 记录库存
	//FishMermaidStock();
	bool bHavePeople = false;
	for (int i = 0;i<PlayChair_Max;i++)
	{
		long  GameUser = GetUserIDByDeskStation(i);
		if (GameUser <= 0) continue;
		//if (IsAndroidUser(i)) continue;

		bHavePeople = true;

		break;
	}
	//没人或者机器人直接结束
	//if (!bHavePeople)
	//{
	//	GameFinish(0, GF_NORMAL);
	//}

	SetGameStation(GAME_STATUS_PLAY);
	// 启动捕鱼
	if( !m_bFishSwimming)
	{
		// 设置种子
		srand(GetTickCount());

		// 第一次
		m_bFirstTime = TRUE;

		// 设置时间
		m_bFishSwimming = TRUE;

		// 开启定时器
		SetTimer(IDI_FISH_CREATE, 100);

		// 延迟定时
		SetTimer(IDI_DELAY, TIME_DELAY);

		// 秒定时
		SetTimer(IDI_SECOND, 1000);
	
		// 保存时间
		m_nStartTime = GetSysMilliseconds();
		m_nSceneBegin = m_nStartTime;
	}
	//记录玩家坐下时间
	m_iOptionTime[deskStation] = GetSysMilliseconds();

	//获取玩家信息
	GameUserInfo UserData;
	GetUserData(deskStation, UserData);
	// 属性设置
	if( m_dwPlayID[deskStation] == 0 )
	{
		// 初始化玩家
		InitializePlayer(deskStation);

		// 更新玩家信息
		m_dwPlayID[deskStation] = GetUserIDByDeskStation(deskStation);
		m_lPlayScore[deskStation] = (LONGLONG)UserData.money;
		m_lPlayStartScore[deskStation] = m_lPlayScore[deskStation];
		m_nPlayDownTime[deskStation] = (DWORD)time(NULL);
		m_nRobotPlayTime[deskStation] = ( (rand()%15) + 15 ) * 60;

		//设置玩家个人难度
		long Userid = GetUserIDByDeskStation(deskStation);
		if (Userid > 0)
		{
			// 个人难度
			int UserDifficulty = GetUserControlParam(deskStation);
			double dGameDifficulty = 0;
			if (UserDifficulty >= 0 && UserDifficulty < 1000)
			{
				int Lev = UserDifficulty / 100;
				dGameDifficulty = m_dPlayDifficultyValue[Lev];
				//printf("个人难度:%d,难度等级:%d,难度系数%lf\n", UserDifficulty, Lev, RoomDifficultyCount[Lev]);
			}
			else if (UserDifficulty >= 1000)
			{
				dGameDifficulty = m_dPlayDifficultyValue[9];
			}
			SetPersonalDifficulty(Userid, dGameDifficulty);
		}
	}
	// 设置状态
	if (!IsBegin)
	{
		IsBegin = true;

		// 开始游戏
		GameBegin(0);
	}
	return true;
}

// 用户起立
bool CTableFrameSink::UserLeftDesk(GameUserInfo* pUser)
{
		
	// 检测是否有人
	for ( WORD i = 0; i < PlayChair_Max; ++i )
	{
	   if (pUser->deskStation == i) continue;
		long  userid = GetUserIDByDeskStation(i);
		if (userid <= 0) continue;

		//有人那么就结算一下
		// 判断用户
		long PlayeruserID = pUser->userID;
		if (PlayeruserID > 0)
		{
			// 玩家指针
			//IServerUserItem * pIServerDismiss = m_pITableFrame->GetTableUserItem(nSite);
			GameUserInfo UserInfo;
			GetUserData(pUser->deskStation, UserInfo);

			// 写分
			LONGLONG lScore = (m_lPlayScore[pUser->deskStation] - m_lPlayStartScore[pUser->deskStation]);
			if (lScore != 0)
			{
				// 积分为负， 不能超过最大分数
				if (lScore < 0 && (-lScore) > UserInfo.money)
				{
					lScore = -UserInfo.money;
				}
			}

			// 游戏时间
			DWORD dwEndTime = (DWORD)time(NULL) - m_nPlayDownTime[pUser->deskStation];

			ChangeUserPoint(pUser->deskStation, lScore, m_lplayCutMoney[i]);
			m_lplayCutMoney[i] = 0;

			// 保存写分
			m_nRoomWrite += (-lScore);
			// 清空玩家信息
			InitializePlayer(pUser->deskStation);

		}
		return CGameDesk::UserLeftDesk(pUser);
	}

	// 删除定时器
	KillTimer(IDI_FISH_CREATE);
	KillTimer(IDI_DELAY);
	KillTimer(IDI_SECOND);
	//删除个人难度
	DeletePersonalDifficulty(pUser->userID);

	// 全局爆炸还原
	if ( GetFishCount(FishType_BaoZhaFeiBiao) > 0 )
	{
		// 最大倍数
		int nMultipleMax = 0;
		for ( int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex )
		{
			nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nMultipleIndex] );
		}

		// 飞镖消耗
		m_dDartStock += nMultipleMax * m_nExplosionStart;
	}

	// 鱼信息
	m_ArrayFishInfo.RemoveAll();
	m_cbBackIndex = 0;
	m_nAppearFishCount = 0;
	m_bFishSwimming = FALSE;
	m_bBackExchange = FALSE;
	m_nStartTime = 0;

	// 阵列
	m_nFishTeamTime = 0;
	m_nFishKingTime = 30;
	ZeroMemory(m_nFishKing, sizeof(m_nFishKing));

	if (IsBegin)
	{
		// 结束游戏
		GameFinish(pUser->deskStation, GF_NORMAL);
	}

	return CGameDesk::UserLeftDesk(pUser);
}

// 返回配置
void CTableFrameSink::GetCustomRule( tagCustomRule & nConfigInfo )
{
	// 更新数值
	CopyMemory( &nConfigInfo, &m_CustomRule, sizeof(tagCustomRule) );
}

// 设置配置
void CTableFrameSink::SetCustomRule( tagCustomRule & nConfigInfo, bool bSaveFile )
{
	// 更新数值
	CopyMemory( &m_CustomRule, &nConfigInfo, sizeof(tagCustomRule) );
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
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
		return;
	}
	// 保存信息
	if ( bSaveFile )
	{
		// 获取当前路径
		TCHAR szDirectoryPath[MAX_PATH + 1] = { "" }; 
		strcpy(szDirectoryPath, CINIFile::GetAppPath().c_str());

		// 打开配置
		tchar szConfigPath[MAX_PATH] = "";
		sprintf(szConfigPath, "%s%s.fdx", szDirectoryPath, pRoomBaseInfo->name);
		file * pFile = NULL;
		pFile = fopen(szConfigPath, "wb+" );
		if ( pFile != NULL )
		{
			// 保存配置
			fwrite( &m_CustomRule, sizeof(m_CustomRule), 1, pFile );
			fflush( pFile );
			fclose( pFile );
		}
	}

	// 倍数配置
	CopyMemory( m_nMultipleValue, nConfigInfo.nMultipleValue, sizeof(m_nMultipleValue) ); 

	// 库存计算
	LONGLONG lStockSetAmount = 0;
	for( int nIndex = 0; nIndex < Multiple_Max; ++nIndex )
	{
		m_dRoomStock[nIndex] = (double)((double)nConfigInfo.nInitialStock * (double)m_nMultipleValue[nIndex]);
		lStockSetAmount += (LONGLONG)(((LONGLONG)nConfigInfo.nInitialStock) * ((LONGLONG)m_nMultipleValue[nIndex]));
	}
	CopyMemory(m_nDifficultyStart, nConfigInfo.nDifficultyStart, sizeof(m_nDifficultyStart));
	CopyMemory(m_nRoomDifficultyCount, nConfigInfo.nRoomDifficultyCount, sizeof(m_nRoomDifficultyCount)); 
	CopyMemory(m_nTableDifficultyCount, nConfigInfo.nTableDifficultyCount, sizeof(m_nTableDifficultyCount)); 
	CopyMemory(m_nPlayDifficultyCount, nConfigInfo.nPlayDifficultyCount, sizeof(m_nPlayDifficultyCount)); 
	CopyMemory(m_dRoomDifficultyValue, nConfigInfo.dRoomDifficultyValue, sizeof(m_dRoomDifficultyValue)); 
	CopyMemory(m_dTableDifficultyValue, nConfigInfo.dTableDifficultyValue, sizeof(m_dTableDifficultyValue)); 
	CopyMemory(m_dPlayDifficultyValue, nConfigInfo.dPlayDifficultyValue, sizeof(m_dPlayDifficultyValue)); 

	// 库存更新
	m_lStockInitial = m_lStockInitial + (lStockSetAmount - m_lStockCurrent);
	m_lStockCurrent = lStockSetAmount;

	// 控制配置
	m_dTaxRatio = Min_( (double)nConfigInfo.nTaxRatio / 1000.0, 0.90 );

	// 场景配置
	m_nCreateCount = nConfigInfo.nCreateCount;
	m_nSceneTime = nConfigInfo.nSceneTime;

	// 子弹信息
	m_nBulletCoolingTime = nConfigInfo.nBulletCoolingTime;
	m_nBulletVelocity = nConfigInfo.nBulletVelocity;

	// 活动配置
	CopyMemory(m_nSupplyCondition, nConfigInfo.nSupplyCondition, sizeof(m_nSupplyCondition)); 
	m_nLaserTime = nConfigInfo.nLaserTime;
	m_nLaserChance = nConfigInfo.nLaserChance;
	m_nSpeedTime = nConfigInfo.nSpeedTime;
	m_nSpeedChance = nConfigInfo.nSpeedChance;
	CopyMemory(m_nGiftScore, nConfigInfo.nGiftScore, sizeof(m_nGiftScore)); 
	CopyMemory(m_nGiftChance, nConfigInfo.nGiftChance, sizeof(m_nGiftChance)); 
	m_nNullChance = Max_(nConfigInfo.nNullChance, 2);

	// 鱼配置
	CopyMemory(m_nFishMultiple, nConfigInfo.nCatchFishMultiple, sizeof(m_nFishMultiple)); 

	// 爆炸配置
	m_nExplosionProportion = nConfigInfo.nExplosionProportion;
	m_nExplosionStart = nConfigInfo.nExplosionStart;
	m_lExplosionCondition = nConfigInfo.lExplosionCondition;
	m_nExplosionConditionType = nConfigInfo.nExplosionConditionType;
	m_nExplosionProportion = Max_( m_nExplosionProportion, 0 );
	m_nExplosionProportion = Min_( m_nExplosionProportion, 1000 );

	// 更新信息
	CMD_S_UpdateGame CMDSUpdateGame;
	CopyMemory( CMDSUpdateGame.nMultipleValue, m_nMultipleValue, sizeof(CMDSUpdateGame.nMultipleValue) );
	CopyMemory( CMDSUpdateGame.nCatchFishMultiple, m_nFishMultiple, sizeof(CMDSUpdateGame.nCatchFishMultiple) );
	CMDSUpdateGame.nBulletVelocity = m_nBulletVelocity;
	CMDSUpdateGame.nBulletCoolingTime = m_nBulletCoolingTime;

	// 全房间发送
	SendTableData(INVALID_CHAIR, SUB_S_UPDATE_GAME, &CMDSUpdateGame, sizeof(CMD_S_UpdateGame) );
}

// 返回个人难度
void CTableFrameSink::GetPersonalDifficulty( CMapPersonalDifficulty & MapPersonalDifficulty )
{
	/*POSITION Pos = m_MapPersonalDifficulty.GetStartPosition();
	while ( Pos )
	{
		unsigned long lPlayID = 0;
		double dPersonalDifficulty = 0.0;
		m_MapPersonalDifficulty.GetNextAssoc( Pos, lPlayID, dPersonalDifficulty );
		MapPersonalDifficulty.SetAt( lPlayID, dPersonalDifficulty );
	}*/
}
//获取玩家的难度
void CTableFrameSink::GetUserDifficulty(unsigned long lPlayID, double& dPersonalDifficulty)
{
	//m_MapPersonalDifficulty.Lookup(lPlayID, dPersonalDifficulty);
	auto iter = m_MapPersonalDifficulty.find(lPlayID);
	if (iter != m_MapPersonalDifficulty.end())
	{
		dPersonalDifficulty = iter->second;
	}
}
// 设置个人难度
void CTableFrameSink::SetPersonalDifficulty( unsigned long lPlayID, double dPersonalDifficulty )
{
	//m_MapPersonalDifficulty.SetAt( lPlayID, dPersonalDifficulty );
	m_MapPersonalDifficulty[lPlayID] = dPersonalDifficulty;
}

// 删除个人难度
void CTableFrameSink::DeletePersonalDifficulty( unsigned long lPlayID )
{
	//m_MapPersonalDifficulty.RemoveKey( lPlayID );
	m_MapPersonalDifficulty.erase(lPlayID);
}

// 清空个人难度
void CTableFrameSink::ClearPersonalDifficulty()
{
	//m_MapPersonalDifficulty.RemoveAll();
	m_MapPersonalDifficulty.clear();
}

// 获取库存
void CTableFrameSink::GetStock( LONGLONG & lStockInitial, LONGLONG & lStockCurrent )
{
	lStockInitial = m_lStockInitial;
	lStockCurrent = m_lStockCurrent;
}

// 设置桌子库存
bool CTableFrameSink::SetTableStock( unsigned short TableID, int nTableStock )
{
	// 判断有效
	//if( TableID >= m_pGameServiceOption->wTableCount )
	//{
	//	return false;
	//}

	// 设置库存
	for ( int nIndex = 0; nIndex < Multiple_Max; ++nIndex )
	{
		// 桌子库存
		double & dTableStock = _TableStock(TableID, nIndex);

		// 设置库存
		dTableStock = nTableStock * m_nMultipleValue[nIndex];
	}

	return true;
}



// 捕鱼事件
bool CTableFrameSink::OnSubCatchFish(BYTE deskStation, const void * pBuffer, WORD wDataSize )
{
	// 效验数据
	ASSERT(wDataSize==sizeof(CMD_C_CatchFish));
	if (wDataSize!=sizeof(CMD_C_CatchFish)) return false;
	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		return true;
	}
	// 消息处理
	CMD_C_CatchFish * pCatchFish = (CMD_C_CatchFish *)pBuffer;
	if (!pCatchFish)
	{
		ERROR_LOG("错误：玩家:%d,OnSubCatchFish（捕鱼事件）空数据包",GetUserIDByDeskStation(deskStation));
		return false;
	}
	// 定义变量
	//WORD wChairID = pIServerUserItem->GetChairID();

	// 查找子弹
	map<int, tagFireInfo>::iterator itr = m_ArrayBulletKey[deskStation].find(pCatchFish->nBulletKey);

	if( itr == m_ArrayBulletKey[deskStation].end() )
	{
		//ASSERT(FALSE);

		return true;
	}

	// 获取子弹信息
	tagFireInfo & TFireInfo = itr->second;

	// 子弹拆分
	int nBulletSplit = 0;
	for ( int nIndex = 0; nIndex < FishCatch_Max; ++nIndex )
	{
		// 有效捕获
		if ( pCatchFish->nFishKey[nIndex] != 0 )
		{
			
			nBulletSplit++;
		}
	}

	// 计算捕获
	for ( int nIndex = 0; nIndex < FishCatch_Max; ++nIndex )
	{
		// 有效捕获
		if ( pCatchFish->nFishKey[nIndex] != 0 )
		{
			// 鱼信息
			bool				bRealKill = false;
			byte				nFishType = FishType_Max;				
			EnumFishState		nFishState = FishState_Normal;

			// 获取鱼信息
			for( int nDeathIndex = 0; nDeathIndex < m_ArrayFishInfo.GetCount(); nDeathIndex++ )
			{
				// 获取鱼信息
				tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt(nDeathIndex);

				// 捕获鱼
				if ( TFishInfo.nFishKey == pCatchFish->nFishKey[nIndex] )
				{
					//printf("捕中鱼的key:%d\n", pCatchFish->nFishKey[nIndex]);
					// 保存信息
					nFishType = TFishInfo.nFishType;
					nFishState = TFishInfo.nFishState;

					// 有效击杀
					if( TFishInfo.wHitChair == deskStation || TFishInfo.wHitChair == INVALID_CHAIR )
					{
						bRealKill = true;
					}

					break;
				}
			}

			// 有效击杀
			if ( bRealKill )
			{
				// 计算捕获数量
				LONGLONG lCatchScore = PlayCatchFish(deskStation, nBulletSplit, pCatchFish->nFishKey[nIndex], TFireInfo.nMultipleIndex, EPCT_Bullet);

				// 判断鱼信息
				if(nFishState >= FishType_BChuiTouSha /*nFishState == FishState_Killer || nFishType == FishType_BaoZhaFeiBiao || nFishType == FishType_ZhongYiTang */)
				{
					
					// 大倍发送信息
					int nCatchMultiple = (int)(lCatchScore / m_nMultipleValue[TFireInfo.nMultipleIndex]);
					SendAwardTip(deskStation, nFishType, nCatchMultiple, lCatchScore, EST_Cold);
					//printf("有效击杀发送:%d\n", nCatchMultiple);
				}
			}
		}
	}

	// 删除子弹
	m_ArrayBulletKey[deskStation].erase(itr);

	return true;
}


// 开火
bool CTableFrameSink::OnSubFire(BYTE deskStation, const void * pBuffer, WORD wDataSize)
{
	// 效验数据
	static int paoCount;
	if (IsAndroidUser(deskStation))
	{
		//printf("机器人开炮:%d\n", paoCount);
		paoCount++;
	}
	if (paoCount > 999)
	{
		paoCount = 0;
	}
	ASSERT(wDataSize==sizeof(CMD_C_Fire));
	if (wDataSize!=sizeof(CMD_C_Fire)) return false;
	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		ERROR_LOG("玩家ID不存在");
		return true;
	}
	// 消息处理
	CMD_C_Fire * pFire = (CMD_C_Fire *)pBuffer;
	if (!pFire)
	{
		ERROR_LOG("错误：玩家%d,OnSubFire发送空数据包", GetUserIDByDeskStation(deskStation));
		return false;
	}

	// 定义变量
	//WORD wChairID = pIServerUserItem->GetChairID();

	// 无效倍数
	if ( pFire->nMultipleIndex != m_nMultipleIndex[deskStation] )
	{
		return true;
	}

	// 子弹操作信息
	int nBulletScore = 0;
	byte cbBulletIndex = 0;
	LONGLONG lBulletInvest = 0;
	lBulletInvest = m_nMultipleValue[m_nMultipleIndex[deskStation]] * QianPao_Bullet;
	//printf("房间等级:%d,打炮倍数:%d,%d,%lld\n",GetRoomLevel() ,m_nMultipleIndex[deskStation], m_nMultipleValue[m_nMultipleIndex[deskStation]], lBulletInvest);
	// 扣除金币
	if ( m_lPlayScore[deskStation] < lBulletInvest )
	{
	//	FishMermaidLogo( "普通子弹金币扣除失败 %d, [P %I64d] [L %I64d]", ( pIServerUserItem->IsAndroidUser() ? 1 : 0 ), m_lPlayScore[wChairID], lBulletInvest );

		ASSERT( FALSE );

		return true;
	}

	// 修改开炮
	LONGLONG lPlayExplosionCondition = 0;

	auto iter = m_MapPlayExplosionCondition.find(GetUserIDByDeskStation(deskStation));
	if (iter != m_MapPlayExplosionCondition.end())
	{
		lPlayExplosionCondition = iter->second;
	}

	//m_MapPlayExplosionCondition.Lookup(GetUserIDByDeskStation(deskStation), lPlayExplosionCondition );
	
	// 未满足条件
	if ( lPlayExplosionCondition < m_lExplosionCondition )
	{
		if( m_nExplosionConditionType == ExplosionConditionType_Gun )
		{
			lPlayExplosionCondition += 1;
			m_MapPlayExplosionCondition[GetUserIDByDeskStation(deskStation)] = lPlayExplosionCondition;
			//m_MapPlayExplosionCondition.SetAt(GetUserIDByDeskStation(deskStation), lPlayExplosionCondition );
		}
		else
		{
			lPlayExplosionCondition += lBulletInvest;
			m_MapPlayExplosionCondition[GetUserIDByDeskStation(deskStation)] = lPlayExplosionCondition;
			//m_MapPlayExplosionCondition.SetAt(GetUserIDByDeskStation(deskStation), lPlayExplosionCondition );
		}
	}

	// 修改金币
	m_lPlayScore[deskStation] -= lBulletInvest;

	// 记录消耗
	nBulletScore = (int)lBulletInvest;
	m_lFireCount[deskStation] += 1;
	m_lBulletConsume[deskStation] += (long)lBulletInvest;

	// 添加能量
	if ( !m_bPlaySupply[deskStation] )
		m_nEnergyValue[deskStation] += 1;
	
	// 瓜分收入
	PlayerConsume(deskStation, m_nMultipleIndex[deskStation], lBulletInvest );

	// 添加子弹
	tagFireInfo FireInfo;
	FireInfo.nMultipleIndex = m_nMultipleIndex[deskStation];
	FireInfo.dBulletInvest = (double)lBulletInvest;
	m_ArrayBulletKey[deskStation].insert( map<int, tagFireInfo>::value_type(pFire->nBulletKey, FireInfo) );
	//printf("子弹关键值:%d,倍数索引:%d,子弹收入:%d", pFire->nBulletKey, FireInfo.nMultipleIndex, FireInfo.dBulletInvest);
	// 设置信息
	CMD_S_Fire CMDSFire;
	CMDSFire.wChairID = deskStation;
	CMDSFire.nBulletScore = nBulletScore;
	CMDSFire.nMultipleIndex = m_nMultipleIndex[deskStation];
	CMDSFire.nTrackFishIndex = pFire->nTrackFishIndex;
	CMDSFire.ptPos = pFire->ptPos;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (GetUserIDByDeskStation(i) <= 0) continue;
		CMDSFire.llPlayScore = m_lPlayScore[i];
		SendTableData(i, SUB_S_FIRE, &CMDSFire, sizeof(CMDSFire));
	}

	// 补给箱判断(暂时未开放)
	if ( false && !m_bPlaySupply[deskStation] && m_nSupplyValue[deskStation] != 0 && m_nEnergyValue[deskStation] >= m_nSupplyValue[deskStation] && !m_bBackExchange )
	{
		// 生成路径
		uint nPathIndex = deskStation;

		// 创建补给箱
		CreateFish( 1, FishType_BaoXiang, nPathIndex, 0, 0, 0.f, false, false, KillerType_No, deskStation, CShortPoint(0, 0), 0.f, false );

		// 清空能量
		m_nEnergyValue[deskStation] = 0;

		// 补给发送
		m_bPlaySupply[deskStation] = true;

		// 更新数量
		m_nSupplyValue[deskStation] = RAND_EQUAL_MIN_MAX(m_nSupplyCondition[0], m_nSupplyCondition[1]);

		// 发送提示消息
		CMD_S_SupplyTip CMDSSupplyTip;
		CMDSSupplyTip.wChairID = deskStation;
		SendTableData(INVALID_CHAIR, SUB_S_SUPPLY_TIP, &CMDSSupplyTip, sizeof(CMDSSupplyTip) );
	}

	return true;
}

// 准备激光
bool CTableFrameSink::OnSubBeginLaser(BYTE deskStation, const void * pBuffer, WORD wDataSize)
{
	// 效验数据
	ASSERT(wDataSize==sizeof(CMD_C_BeginLaser));
	if (wDataSize!=sizeof(CMD_C_BeginLaser) ) return false;

	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		ERROR_LOG("错误：玩家不存在");
		return false;
	}
	// 激光数值判断
	//if ( m_nLaserPlayTime[pIServerUserItem->GetChairID()] <= 0 )
	//{
	//	FishMermaidLogo( "激光使用超时 - 1。" );

	//	ASSERT(FALSE);

	//	return true;
	//}

	// 消息处理
	CMD_C_BeginLaser * pBeginLaser = (CMD_C_BeginLaser *)pBuffer;
	if (!pBeginLaser)
	{
		ERROR_LOG("错误:玩家%d，OnSubBeginLaser存在空指针", GetUserIDByDeskStation(deskStation));
		return false;
	}
	// 发送消息
	CMD_S_BeginLaser BeginLaser;
	BeginLaser.wChairID = deskStation;
	BeginLaser.ptPos = pBeginLaser->ptPos;

	// 发送消息
	SendTableData(INVALID_CHAIR, SUB_S_BEGIN_LASER, &BeginLaser, sizeof(BeginLaser) );

	return true;
}

// 激光
bool CTableFrameSink::OnSubLaser(BYTE deskStation, const void * pBuffer, WORD wDataSize)
{
	// 效验数据
	ASSERT(wDataSize==sizeof(CMD_C_Laser));
	if (wDataSize!=sizeof(CMD_C_Laser)) return false;

	// 激光数值判断
	//if ( m_nLaserPlayTime[pIServerUserItem->GetChairID()] <= 0 )
	//{
	//	FishMermaidLogo( "激光使用超时 - 2。" );

	//	ASSERT(FALSE);

	//	return true;
	//}
	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		ERROR_LOG("错误：找不到玩家%d", deskStation);
		return false;
	}
	// 消息处理
	CMD_C_Laser * pLaser = (CMD_C_Laser *)pBuffer;

	if (!pLaser)
	{
		ERROR_LOG("错误：玩家 :%d,OnSubLaser空指针", GetUserIDByDeskStation(deskStation));
		return false;
	}
	// 定义变量
	CMD_S_Laser Laser;
	WORD wChairID = deskStation;

	// 设置信息
	Laser.wChairID = deskStation;
	Laser.ptPos = pLaser->ptEndPos;

	// 激光完毕
	m_nLaserPlayTime[deskStation] = 0;

	// 发送消息
	SendTableData(INVALID_CHAIR, SUB_S_LASER, &Laser, sizeof(Laser) );

	// 角计算
	FLOAT fRadian = atan2((FLOAT)( pLaser->ptEndPos.y - pLaser->ptBeginPos.y ) , (FLOAT)( pLaser->ptBeginPos.x - pLaser->ptEndPos.x ));
	fRadian = -fRadian - GL_PI / 2;

	// 生成激光范围
	CFloatVector2 FVBullet;
	CFloatRotate  FRBullet;
	CPolygonShape SPBullet;
	FRBullet.Set(fRadian);
	FVBullet.Set(pLaser->ptBeginPos.x, pLaser->ptBeginPos.y);
	CFloatVector2 FVLaser[4];
	FVLaser[0].Set( -50, -(Max_(DEFAULE_WIDTH, DEFAULE_HEIGHT)) );
	FVLaser[1].Set(  50, -(Max_(DEFAULE_WIDTH, DEFAULE_HEIGHT)) );
	FVLaser[2].Set(  50, 0 );
	FVLaser[3].Set( -50, 0 );
	SPBullet.Set( FVLaser, 4 );

	// 计算捕获
	CWHArray< uint > ArrayFishCatchKey;
	for( int nFishIndex = 0; nFishIndex < m_ArrayFishInfo.GetCount(); ++nFishIndex )
	{
		// 获取信息
		tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt( nFishIndex );

		// 计算鱼位置
		CDoublePoint PointFish;
		if( FishMove( PointFish, TFishInfo, pLaser->unLossTime ) == FishMoveType_Normal )
		{
			// 无效位置
			if ( PointFish.x < 0 || PointFish.x > DEFAULE_WIDTH || PointFish.y < 0 || PointFish.y > DEFAULE_HEIGHT )
			{
				continue;
			}

			// 比较位置
			if( SPBullet.PointInShape( CFloatTransform( FVBullet, FRBullet ), CFloatVector2(PointFish.x, PointFish.y) ) )
			{
				ArrayFishCatchKey.Add( TFishInfo.nFishKey );
			}
		}
	}

	// 计算捕获数量
	LONGLONG lCatchScore = 0;

	// 计算捕获
	for ( int nFishCatchKeyIndex = 0; nFishCatchKeyIndex < ArrayFishCatchKey.GetCount(); nFishCatchKeyIndex++ )
	{
		// 计算捕获
		lCatchScore += PlayCatchFish(deskStation, (int)ArrayFishCatchKey.GetCount(), ArrayFishCatchKey[nFishCatchKeyIndex], m_nMultipleIndex[deskStation], EPCT_Laser);
	}

	// 大倍发送信息
	int nCatchMultiple = (int)(lCatchScore / m_nMultipleValue[m_nMultipleIndex[deskStation]]);
	SendAwardTip(deskStation, 0, nCatchMultiple, lCatchScore, EST_Laser);
	//printf("大倍发送:%d\n", nCatchMultiple);
	return true;
}

// 延迟
bool CTableFrameSink::OnSubDelay(BYTE deskStation, const void * pBuffer, WORD wDataSize)
{
	ASSERT(wDataSize == 0);
	if (wDataSize != 0) return false;

	// 定义变量
	WORD wChairID = deskStation;

	// 定义消息
	CMD_S_Delay Delay;

	// 计算延迟
	DWORD dwTime = GetSysMilliseconds();
	if ( dwTime < m_nPlayDelay[deskStation] )
		Delay.nDelay = ULONG_MAX - m_nPlayDelay[deskStation] + dwTime;
	else
		Delay.nDelay = dwTime - m_nPlayDelay[deskStation];

	// 设置位置
	Delay.wChairID = deskStation;

	// 机器人随机延时
	if( IsAndroidUser(deskStation) )
	{
		Delay.nDelay = rand()%50 + 50;
	}

	// 发送消息
	SendTableData(INVALID_CHAIR, SUB_S_DELAY, &Delay, sizeof(CMD_S_Delay) );

	return true;
}


// 语音
bool CTableFrameSink::OnSubSpeech(BYTE deskStation, const void * pBuffer, WORD wDataSize)
{
	// 效验数据
	ASSERT(wDataSize==sizeof(CMD_C_Speech));
	if (wDataSize!=sizeof(CMD_C_Speech)) return false;

	//// 消息处理
	//CMD_C_Speech * pSpeech = (CMD_C_Speech *)pBuffer;

	//// 判断信息
	//if ( pSpeech->nSpeechIndex >= SPEECH_INDEX_MAX )
	//	return true;
	//
	//// 发送消息
	//CMD_S_Speech Speech;
	//Speech.wChairID = pIServerUserItem->GetChairID();
	//Speech.nSpeechIndex = pSpeech->nSpeechIndex;
	//SendTableData(SUB_S_SPEECH, &Speech, sizeof(Speech));

	return true;
}

// 倍数选择
bool CTableFrameSink::OnSubMultiple(BYTE deskStation, const void * pBuffer, WORD wDataSize)
{
	// 效验数据
	ASSERT(wDataSize==sizeof(CMD_C_Multiple));
	if (wDataSize!=sizeof(CMD_C_Multiple)) return false;
	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		ERROR_LOG("错误:玩家%d不存在!", deskStation);
		return false;
	}
	// 消息处理
	CMD_C_Multiple * pMultiple = (CMD_C_Multiple *)pBuffer;
	if (!pMultiple)
	{
		ERROR_LOG("错误:玩家%d,倍数选择OnSubMultiple空指针", GetUserIDByDeskStation(deskStation));
		return false;
	}
	// 定义变量
	WORD wChairID = deskStation;

	// 判断信息
	if ( pMultiple->nMultipleIndex >= Multiple_Max || pMultiple->nMultipleIndex < 0 )
	{
		ERROR_LOG("玩家%d设置倍数失败, 超过有效值。", deskStation);

		//ASSERT(FALSE);

		return false;
	}

	// 激光和宝箱过滤
	if ( m_nLaserPlayTime[deskStation] > 0 || m_bPlaySupply[deskStation] )
	{
		ERROR_LOG("活动期间，玩家%d,ID:%d设置倍数无效,激光：%d,补给箱:%d。", deskStation, GetUserIDByDeskStation(deskStation), m_nLaserPlayTime[deskStation], m_bPlaySupply[deskStation]);

		//ASSERT(FALSE);

		return true;
	}

	// 设置变量
	m_nMultipleIndex[deskStation] = pMultiple->nMultipleIndex;

	// 发送消息
	CMD_S_Multiple Multiple;
	Multiple.wChairID = deskStation;
	Multiple.nMultipleIndex = pMultiple->nMultipleIndex;
	SendTableData(INVALID_CHAIR, SUB_S_MULTIPLE, &Multiple, sizeof(Multiple));

	return true;
}

// 控制信息
bool CTableFrameSink::OnSubControl(BYTE deskStation, VOID * pData, WORD wDataSize )
{
	// 效验数据
	ASSERT(wDataSize==sizeof(CMD_C_Control));
	if (wDataSize!=sizeof(CMD_C_Control)) return false;

	// 定义变量
	CMD_C_Control * pControl = (CMD_C_Control*)pData;
	if (GetUserIDByDeskStation(deskStation) <= 0)
	{
		ERROR_LOG("错误:玩家%d不存在!", deskStation);
		return false;
	}
	if (!pControl)
	{
		ERROR_LOG("错误:玩家%d,控制信息OnSubControl空指针", GetUserIDByDeskStation(deskStation));
		return false;
	}
	// 判断权限
	//if( !CUserRight::IsGameCheatUser(pIServerUserItem->GetUserRight()) )
	return true;

}

// 创建鱼
uint CTableFrameSink::CreateFishEx( byte cbCount, 
								   byte cbFishType /*= FishType_Max*/, 
								   tagBezierPoint TBezierPoint[BEZIER_POINT_MAX] /*= NULL*/, 
								   int nBezierCount /*= 0*/, 
								   uint unCreateTime /*= 0*/, 
								   uint unIntervalTime /*= 1000*/, 
								   float fRotateAngle /*= 0.f*/, 
								   bool bCanSpecial /*= true*/, 
								   bool bCanAquatic /* = true */, 
								   EnumKillerType nKillerType /*= KillerType_No*/, 
								   WORD wHitChair /*= INVALID_CHAIR*/, 
								   CShortPoint PointOffSet /*= CShortPoint(0,0)*/, 
								   float fInitialAngle /*= 0.f*/, 
								   bool bRepeatCreate /*= true */ )
{
	// 无效路径
	if ( TBezierPoint == NULL || nBezierCount == 0 )
	{
		//_Assert( false );

		return 0;
	}

	// 玩家数量
	int nPlayCount = 0;
	for( int nSite = 0; nSite < PlayChair_Max; ++nSite )
	{
		if( GetUserIDByDeskStation(nSite) > 0 )
			nPlayCount++;
	}

	// 创建种类
	if ( cbFishType == FishType_Max )
	{
		// 大鱼几率
		int ChuiTouSha = 4;
		int JinJuChiSha = 4;
		int JinKuiJiaYu = 4;
		int JinChuiTouSha = 4;
		int JinShaYu = 4;
		int JinHuSha = 4;
		int BOSS = 4;
		int HongBao = 40;
		int HuaFei1 = 40;
		int HuaFei2 = 40;

		if( GetFishCount(FishType_BChuiTouSha) >= 1 ) ChuiTouSha = 0;
		if (GetFishCount(FishType_BJinJuChiSha) >= 1) JinJuChiSha = 0;
		if (GetFishCount(FishType_BJinKuiJiaYu) >= 1) JinKuiJiaYu = 0;
		if (GetFishCount(FishType_BJinChuiTouSha) >= 1) JinChuiTouSha = 0;
		if (GetFishCount(FishType_BJinShaYu) >= 1) JinShaYu = 0;
		if (GetFishCount(FishType_BJinHuSha) >= 1) JinHuSha = 0;
		if (GetFishCount(FishType_BOSS) >= 1) BOSS = 0;

		if (GetFishCount(FishType_BChuiTouSha) + GetFishCount(FishType_BJinJuChiSha) + GetFishCount(FishType_BJinKuiJiaYu) + GetFishCount(FishType_BJinChuiTouSha) + GetFishCount(FishType_BJinShaYu)
			+ GetFishCount(FishType_BJinHuSha) > 2)
		{
			int ChuiTouSha = 0;
			int JinJuChiSha = 0;
			int JinKuiJiaYu = 0;
			int JinChuiTouSha = 0;
			int JinShaYu = 0;
			int JinHuSha = 0;
		}

		if (GetFishCount(FishType_BOSS) >= 1)
		{
			int ChuiTouSha = 0;
			int JinJuChiSha = 0;
			int JinKuiJiaYu = 0;
			int JinChuiTouSha = 0;
			int JinShaYu = 0;
			int JinHuSha = 0;
		}
		if (GetFishCount(FishType_Hong_Bao)  >= 1)
		{
			HongBao = 0;
		}
		if (GetFishCount(FishType_Hua_Fei1) + GetFishCount(FishType_Hua_Fei2) >= 1)
		{
			HuaFei1 = 0;
			HuaFei2 = 0;
		}
		// 随机鱼
		cbFishType = RandomArea(FishType_Normal_Max, 2, 5, 5, 20, 20, 20, 20, 20, 10, 10, 10, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, ChuiTouSha, JinJuChiSha, JinKuiJiaYu, JinChuiTouSha, JinShaYu, JinHuSha, 0,HongBao, HuaFei1,HuaFei2);
		//鱼王刷新时间
		if (m_FishKingTime == 0)
		{
			m_FishKingTime = GetSysMilliseconds();
			m_bFushBoss = false;
			m_bFushSence = false;
		}
		else if (!m_bIsFushBoss && m_FishKingTime > 0 && (GetSysMilliseconds() - m_FishKingTime) >m_FishKingIntervalTime && GetFishCount(FishType_BOSS) <= 0)
		{
			m_bFushSence = true;
			m_bIsFushBoss = true;
			SetTimer(TIME_FUSH_BOSS, 5000);
			SendTableData(INVALID_CHAIR, SUB_S_GAME_BOSS, 0, 0);
		}

		if (m_bFushBoss)
		{
			m_bFushBoss = false;
			m_bFushSence = true;
			m_bIsFushBegin = true;
			cbFishType = FishType_BOSS;
			m_FishKingTime = GetSysMilliseconds();
		}

		if (GetFishCount(FishType_BOSS) <= 0 && m_bIsFushBegin && cbFishType!= FishType_BOSS)
		{
			m_bFushSence = false;
			m_bIsFushBoss = false;
			m_bIsFushBegin = false;
		}
	}

	// 鱼分数
	int nFishScore = 0;
	RAND_MIN_MAX(nFishScore, m_nFishMultiple[cbFishType][0], m_nFishMultiple[cbFishType][1] );

	//  宝箱保存当前倍数
	if ( cbFishType == FishType_BaoXiang && wHitChair != INVALID_CHAIR )
	{
		nFishScore = m_nMultipleValue[m_nMultipleIndex[wHitChair]];
	}

	// 游动时间
	uint unSwimmiTime = 0;
	for( int nBezierIndex = 0; nBezierIndex < nBezierCount; ++nBezierIndex )
		unSwimmiTime += TBezierPoint[nBezierIndex].Time;

	// 杀手鱼限制
	int nKillerIndex = int_max;
	if( nKillerType == KillerType_One && cbFishType < FishType_Small_Max )
	{
		nKillerIndex = rand()%cbCount;
	}

	// 死亡几率
	double dCurChance = 0.0;

	// 循环创建
	int nIndex = 0;
	const uint unCurrentTime = GetSysMilliseconds();
	ASSERT( unCurrentTime >= m_nStartTime );
	while ( nIndex < cbCount )
	{
		// 特殊鱼
		bool bSpecial = false;				
		if( GetFishSpecialCount() < 2 && bCanSpecial && m_nFishKingTime == 0 && m_nFishKing[cbFishType] == 0 && cbFishType < FishType_Small_Max && nFishScore > 0 && nFishScore <= 10 && RandomArea(2, 3, 19) == 0 )
		{
			m_nFishKing[cbFishType] = 30;
			m_nFishKingTime = 45 - (nPlayCount * 5);
			bSpecial = true;
		}

		// 停留鱼鱼
		bool bAquatic = false;	
		if( bCanAquatic && nFishScore < 20 && RandomArea(2, 1, 30) == 0 && cbFishType != FishType_BaoXiang )
		{
			bAquatic = true;
		}

		// 基本信息
		tagFishInfo FishInfo;
		FishInfo.nFishKey = Max_( 1, m_nAppearFishCount + 1);
		FishInfo.nFishType = cbFishType;
		FishInfo.unCreateTime = (unCurrentTime - m_nStartTime) + unCreateTime + nIndex * unIntervalTime;
		FishInfo.unOverTime = unSwimmiTime;
		FishInfo.nScoreChance = nFishScore;
		FishInfo.dCurChance	= dCurChance;
		FishInfo.fRotateAngle = fRotateAngle;
		FishInfo.PointOffSet = PointOffSet;
		FishInfo.wHitChair = wHitChair;
		FishInfo.nFishScore = nFishScore;
		FishInfo.nBezierCount = nBezierCount; 
		FishInfo.bRepeatCreate = bRepeatCreate;
		FishInfo.fInitialAngle = fInitialAngle;
		CopyMemory( FishInfo.TBezierPoint, TBezierPoint, sizeof(FishInfo.TBezierPoint) );

		if ( nIndex == nKillerIndex || nKillerType == KillerType_All )
		{
			FishInfo.nFishState = FishState_Killer;
		}
		else if ( bSpecial )
		{
			FishInfo.nFishState = FishState_King;
		}
		else if ( bAquatic )
		{
			FishInfo.nFishState = FishState_Aquatic;
		}
		else
		{
			FishInfo.nFishState = FishState_Normal;
		}

		// 鱼数增加
		m_nAppearFishCount++;

		// 添加数据
		m_ArrayFishInfo.Add(FishInfo);

		// 发送鱼信息
		SendFish(FishInfo);

		// 下一个
		nIndex++;
	}

	return unCreateTime + nIndex * unIntervalTime + unSwimmiTime;
}


// 创建鱼
uint CTableFrameSink::CreateFish( byte cbCount, 
								  byte cbFishType		/* = FishType_Max */, 
								  uint nPathIndex		/* = uint_max */,
								  uint unCreateTime		/* = 0 */,
								  uint unIntervalTime	/* = 1000 */,
								  float fRotateAngle	/* = 0.f */, 
								  bool bCanSpecial		/* = true */, 
								  bool bCanAquatic		/* = true */, 
								  EnumKillerType nKillerType /* = KillerType_No */,
								  WORD wHitChair		/* = INVALID_CHAIR */,
								  CShortPoint PointOffSet /*= CShortPoint(0,0)*/, 
								  float fInitialAngle /*= 0.f*/, 
								  bool bRepeatCreate /*= true*/)
{

	// 随机路径
	int nBezierCount = 0;
	tagBezierPoint TBezierPoint[BEZIER_POINT_MAX];
	if ( (int64)nPathIndex >= m_ArrayFishPathPositive.GetCount() )
	{
		// 无指定
		RandomPath( cbFishType, TBezierPoint, nBezierCount );
	}
	else
	{
		// 获取固定路径
		ASSERT(nPathIndex < (uint)m_ArrayFishPathPositive.GetCount() );
		CWHArray< tagBezierPoint * > * pArrayBezierPoint = &(m_ArrayFishPathPositive[nPathIndex]->ArrayBezierPoint);

		// 赋值路径
		ASSERT( pArrayBezierPoint->GetCount() > 0 && pArrayBezierPoint->GetCount() <= BEZIER_POINT_MAX );
		nBezierCount = (int)pArrayBezierPoint->GetCount();
		for ( int nIndex = 0; nIndex < nBezierCount; ++nIndex )
		{
			// 获取路径
			tagBezierPoint * pBezierPoint = pArrayBezierPoint->GetAt(nIndex);
			if (!pBezierPoint)
			{
				ERROR_LOG("错误:获取鱼路径错误");
				return false;
			}
			// 赋值信息
			CopyMemory( &TBezierPoint[nIndex], pBezierPoint, sizeof(tagBezierPoint) );
		}
	}

	return CreateFishEx( cbCount, cbFishType, TBezierPoint, nBezierCount, unCreateTime, unIntervalTime, fRotateAngle, bCanSpecial, bCanAquatic, nKillerType, wHitChair, PointOffSet, fInitialAngle, bRepeatCreate );
}

// 创建鱼
uint CTableFrameSink::ResetFish( tagFishInfo & TFishInfo )
{
	// 当前时间
	int nIndex = 0;
	const uint nCurrentTime = GetSysMilliseconds();
	ASSERT( nCurrentTime >= m_nStartTime );

	// 基本信息
	tagFishInfo TFishCreateInfo;
	TFishCreateInfo.nFishKey = Max_( 1, m_nAppearFishCount + 1);
	TFishCreateInfo.nFishType = TFishInfo.nFishType;
	TFishCreateInfo.unCreateTime = nCurrentTime - m_nStartTime;
	TFishCreateInfo.unOverTime = TFishInfo.unOverTime;
	TFishCreateInfo.nScoreChance = TFishInfo.nFishScore;
	TFishCreateInfo.dCurChance	= TFishInfo.dCurChance;
	TFishCreateInfo.fRotateAngle = TFishInfo.fRotateAngle;
	TFishCreateInfo.PointOffSet = TFishInfo.PointOffSet;
	TFishCreateInfo.nFishState = TFishInfo.nFishState;
	TFishCreateInfo.wHitChair = TFishInfo.wHitChair;
	TFishCreateInfo.nFishScore = TFishInfo.nFishScore;
	TFishCreateInfo.nBezierCount = TFishInfo.nBezierCount;
	TFishCreateInfo.fInitialAngle = TFishInfo.fInitialAngle;
	//TFishCreateInfo.PSFish.Set( MonsterVec[TFishInfo.nFishType], MONSTER_VEC );

	// 修改路径
	for ( int nIndex = TFishInfo.nBezierCount - 1, nAntiIndex = 0; nIndex >= 0; --nIndex, ++nAntiIndex )
	{
		// 修改路径信息
		TFishCreateInfo.TBezierPoint[nAntiIndex].BeginPoint = TFishInfo.TBezierPoint[nIndex].EndPoint;
		TFishCreateInfo.TBezierPoint[nAntiIndex].EndPoint = TFishInfo.TBezierPoint[nIndex].BeginPoint;
		TFishCreateInfo.TBezierPoint[nAntiIndex].KeyOne = TFishInfo.TBezierPoint[nIndex].KeyTwo;
		TFishCreateInfo.TBezierPoint[nAntiIndex].KeyTwo = TFishInfo.TBezierPoint[nIndex].KeyOne;
		TFishCreateInfo.TBezierPoint[nAntiIndex].Time = TFishInfo.TBezierPoint[nIndex].Time;
	}

	// 更新鱼
	TFishInfo = TFishCreateInfo;

	// 鱼数增加
	m_nAppearFishCount++;

	// 发送鱼信息
	SendFish(TFishCreateInfo);

	return nCurrentTime - m_nStartTime + TFishInfo.unOverTime;
}


// 随机路径
void CTableFrameSink::RandomPath( byte cbFishType, tagBezierPoint TBezierPoint[BEZIER_POINT_MAX], int & nBezierCount )
{
	// BOSS
	if (m_bIsFushBoss && cbFishType >= FishType_Moderate_Max)
	{
		// 创建路径
		TBezierPoint[0].BeginPoint.x = -200;
		TBezierPoint[0].BeginPoint.y = RAND_EQUAL_MIN_MAX(100, 700);
		TBezierPoint[0].EndPoint.x = 1480;
		TBezierPoint[0].EndPoint.y = RAND_EQUAL_MIN_MAX(100, 700);
		TBezierPoint[0].KeyOne.x = RAND_EQUAL_MIN_MAX(100, 1280);
		TBezierPoint[0].KeyOne.y = RAND_EQUAL_MIN_MAX(100, 800);
		TBezierPoint[0].KeyTwo.x = RAND_EQUAL_MIN_MAX(100, 1280);
		TBezierPoint[0].KeyTwo.y = RAND_EQUAL_MIN_MAX(100, 800);
		TBezierPoint[0].Time = 30000;

		// 路径数量
		nBezierCount = 1;
	}
	else if( cbFishType >= FishType_Moderate_Max )
	{
		// 创建路径
		TBezierPoint[0].BeginPoint.x = -200;
		TBezierPoint[0].BeginPoint.y = RAND_EQUAL_MIN_MAX( 100, 700 );
		TBezierPoint[0].EndPoint.x = 1480;
		TBezierPoint[0].EndPoint.y = RAND_EQUAL_MIN_MAX( 100, 700 );
		TBezierPoint[0].KeyOne.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
		TBezierPoint[0].KeyOne.y = RAND_EQUAL_MIN_MAX( 0, 800 );
		TBezierPoint[0].KeyTwo.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
		TBezierPoint[0].KeyTwo.y = RAND_EQUAL_MIN_MAX( 0, 800 );
		TBezierPoint[0].Time = 30000;

		// 路径数量
		nBezierCount = 1;
	}
	// 中鱼
	else if( cbFishType >= FishType_Small_Max )
	{
		// 创建路径
		if ( RAND_TRUE_FALSE(50, 50) )
		{
			TBezierPoint[0].BeginPoint.x = -200;
			TBezierPoint[0].BeginPoint.y = RAND_EQUAL_MIN_MAX( 100, 700 );
			TBezierPoint[0].EndPoint.x = 1480;
			TBezierPoint[0].EndPoint.y = RAND_EQUAL_MIN_MAX( 100, 700 );
		}
		else
		{
			TBezierPoint[0].BeginPoint.x = RAND_EQUAL_MIN_MAX( 100, 1180 );
			TBezierPoint[0].BeginPoint.y = -200;
			TBezierPoint[0].EndPoint.x = RAND_EQUAL_MIN_MAX( 100, 1180 );
			TBezierPoint[0].EndPoint.y = 1000;
		}

		TBezierPoint[0].KeyOne.x = RAND_EQUAL_MIN_MAX( 200, 1080 );
		TBezierPoint[0].KeyOne.y = RAND_EQUAL_MIN_MAX( 200, 600 );
		TBezierPoint[0].KeyTwo.x = RAND_EQUAL_MIN_MAX( 200, 1080 );
		TBezierPoint[0].KeyTwo.y = RAND_EQUAL_MIN_MAX( 200, 600 );
		TBezierPoint[0].Time = RAND_EQUAL_MIN_MAX( 15000, 23000 );

		// 路径数量
		nBezierCount = 1;
	}
	// 小鱼
	else
	{

		// 第一种模式
		if ( RAND_TRUE_FALSE(20, 80) )
		{
			// 创建路径
			if ( RAND_TRUE_FALSE(50, 50) )
			{
				TBezierPoint[0].BeginPoint.x = -200;
				TBezierPoint[0].BeginPoint.y = RAND_EQUAL_MIN_MAX( 0, 800 );
				TBezierPoint[0].EndPoint.x = 1480;
				TBezierPoint[0].EndPoint.y = RAND_EQUAL_MIN_MAX( 0, 800 );
			}
			else
			{
				TBezierPoint[0].BeginPoint.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
				TBezierPoint[0].BeginPoint.y = -200;
				TBezierPoint[0].EndPoint.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
				TBezierPoint[0].EndPoint.y = 1000;
			}

			TBezierPoint[0].KeyOne.x = RAND_EQUAL_MIN_MAX( 200, 1080 );
			TBezierPoint[0].KeyOne.y = RAND_EQUAL_MIN_MAX( 200, 600 );
			TBezierPoint[0].KeyTwo.x = RAND_EQUAL_MIN_MAX( 200, 1080 );
			TBezierPoint[0].KeyTwo.y = RAND_EQUAL_MIN_MAX( 200, 600 );
			TBezierPoint[0].Time = RAND_EQUAL_MIN_MAX( 10000, 15000 );

			// 路径数量
			nBezierCount = 1;
		}
		// 第二种模式
		else
		{
			// 创建路径
			if ( RAND_TRUE_FALSE(50, 50) )
			{
				TBezierPoint[0].BeginPoint.x = -200;
				TBezierPoint[0].BeginPoint.y = RAND_EQUAL_MIN_MAX( 0, 800 );
				TBezierPoint[1].EndPoint.x = 1480;
				TBezierPoint[1].EndPoint.y = RAND_EQUAL_MIN_MAX( 0, 800 );
			}
			else
			{
				TBezierPoint[0].BeginPoint.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
				TBezierPoint[0].BeginPoint.y = -200;
				TBezierPoint[1].EndPoint.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
				TBezierPoint[1].EndPoint.y = 1000;
			}

			TBezierPoint[0].EndPoint.x = RAND_EQUAL_MIN_MAX( 200, 1208 );
			TBezierPoint[0].EndPoint.y = RAND_EQUAL_MIN_MAX( 100, 800 );
			TBezierPoint[1].BeginPoint.x = TBezierPoint[0].EndPoint.x;
			TBezierPoint[1].BeginPoint.y = TBezierPoint[0].EndPoint.y;

			TBezierPoint[0].KeyOne.x = RAND_EQUAL_MIN_MAX( 200, 1080 );
			TBezierPoint[0].KeyOne.y = RAND_EQUAL_MIN_MAX( 200, 600 );
			TBezierPoint[0].KeyTwo.x = RAND_EQUAL_MIN_MAX( 200, 1080 );
			TBezierPoint[0].KeyTwo.y = RAND_EQUAL_MIN_MAX( 200, 600 );
			TBezierPoint[0].Time = RAND_EQUAL_MIN_MAX( 7000, 9000 );

			TBezierPoint[1].KeyOne.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
			TBezierPoint[1].KeyOne.y = RAND_EQUAL_MIN_MAX( 0, 800 );
			TBezierPoint[1].KeyTwo.x = RAND_EQUAL_MIN_MAX( 0, 1280 );
			TBezierPoint[1].KeyTwo.y = RAND_EQUAL_MIN_MAX( 0, 800 );
			TBezierPoint[1].Time = RAND_EQUAL_MIN_MAX( 7000, 15000 );

			// 路径数量
			nBezierCount = 2;
		}
	}

	// 随机反转路径
	if( RAND_TRUE_FALSE(50, 50) )
	{
		// 遍历路径
		tagBezierPoint TBezierPointAnti[BEZIER_POINT_MAX];
		for ( int nIndex = nBezierCount - 1, nAntiIndex = 0; nIndex >= 0; --nIndex, ++nAntiIndex )
		{
			// 修改路径信息
			TBezierPointAnti[nAntiIndex].BeginPoint = TBezierPoint[nIndex].EndPoint;
			TBezierPointAnti[nAntiIndex].EndPoint = TBezierPoint[nIndex].BeginPoint;
			TBezierPointAnti[nAntiIndex].KeyOne = TBezierPoint[nIndex].KeyTwo;
			TBezierPointAnti[nAntiIndex].KeyTwo = TBezierPoint[nIndex].KeyOne;
			TBezierPointAnti[nAntiIndex].Time = TBezierPoint[nIndex].Time;
		}
		CopyMemory(TBezierPoint, TBezierPointAnti, sizeof(tagBezierPoint) * nBezierCount);
	}
}

// 发送鱼信息
void CTableFrameSink::SendFish( tagFishInfo & TFishInfo )
{
	// 构造消息
	CMD_S_FishCreate FishCreate;
	FishCreate.nFishKey = TFishInfo.nFishKey;
	FishCreate.unCreateTime = TFishInfo.unCreateTime;
	FishCreate.wHitChair = TFishInfo.wHitChair;
	FishCreate.nFishType = TFishInfo.nFishType;
	FishCreate.nFishState = TFishInfo.nFishState;
	FishCreate.fRotateAngle = TFishInfo.fRotateAngle;
	FishCreate.PointOffSet = TFishInfo.PointOffSet;
	FishCreate.nBezierCount = TFishInfo.nBezierCount;
	FishCreate.fInitialAngle = TFishInfo.fInitialAngle;
	CopyMemory( &FishCreate.TBezierPoint, &TFishInfo.TBezierPoint, sizeof(FishCreate.TBezierPoint) );
	//printf("创建鱼时间:%d", FishCreate.unCreateTime);
	// 发送消息
	SendTableData(INVALID_CHAIR, SUB_S_CREATE_FISHS, &FishCreate, sizeof(CMD_S_FishCreate) );

	// 发送停留消息
	for( int nIndex = 0; nIndex < TFishInfo.ArrayStayInfo.GetCount(); ++nIndex )
	{
		// 获取停留信息
		tagStayInfo & TStayInfo = TFishInfo.ArrayStayInfo.GetAt( nIndex );

		// 定义消息
		CMD_S_StayFish CMDSStayFish;
		CMDSStayFish.nFishKey = TFishInfo.nFishKey;
		CMDSStayFish.nStayStart = TStayInfo.nStayStart;
		CMDSStayFish.nStayTime = TStayInfo.nStayTime;

		// 发送消息
		SendTableData(INVALID_CHAIR, SUB_S_STAY_FISH, &CMDSStayFish, sizeof(CMD_S_StayFish));
	}
}

// 特殊鱼阵
uint CTableFrameSink::SpecialFishMatrix( byte cbFishType, CDoublePoint PointFish )
{
	// 鱼位置
	int nPathIndex = RAND_TRUE_FALSE(50, 50) ? 6 : 7;

	// 偏移位置
	CShortPoint PointOffSet;
	tagBezierPoint * pBezierPoint = m_ArrayFishPathPositive.GetAt(nPathIndex)->ArrayBezierPoint.GetAt(0);
	if (!pBezierPoint)
	{
		ERROR_LOG("错误:特殊鱼阵获取错误");
		return false;
	}
	PointOffSet.x = (short)(PointFish.x - pBezierPoint->BeginPoint.x);
	PointOffSet.y = (short)(PointFish.y - pBezierPoint->BeginPoint.y);

	// 杀手鱼
	bool bCreateKiller = false;

	// 36个
	int nCountMax = 36;
	if (m_iFishShoalLev > 0 && m_iFishShoalLev <50)
	{
		nCountMax = m_iFishShoalLev;
	}
	for( int nCount = 0; nCount < nCountMax; ++nCount )
	{
		// 创建杀手
		bool bKiller = !bCreateKiller && (RandomArea( 2, 2, nCountMax - nCount - 1 ) == 0);
		if ( bKiller ) bCreateKiller = true;

		// 创建3圈鱼
		CreateFish( 3, cbFishType, nPathIndex, 0, 1000, GL_PI * 2.f / nCountMax * nCount, false, false, bKiller ? KillerType_One : KillerType_No, INVALID_CHAIR, PointOffSet, 0.f, false );
	}

	return 0;
}
// 鱼群建立
uint CTableFrameSink::GroupOfFish( uint nBeginTime /* = 0 */ )
{
	BYTE Grop = rand() % 5;
	if (m_iFinshGroup >=0 && m_iFinshGroup<5)
	{
		Grop = m_iFinshGroup;
	}
	//ERROR_LOG("创建鱼阵类型：%d,鱼阵等级:%d", Grop, m_iFinshLeve);
	// 判断种类
	switch(Grop)
	{
	case 0:
		{
			// 创建大
			int nFishTime[10] = {0, 3000, 6000, 9000, 12000, 15500, 18000, 22000, 27500, 32000 };
			for ( int nFishIndex = FishType_BDengLongYu; nFishIndex <= FishType_BMoGuiYu; ++nFishIndex )
			{
				CreateFish( 1, nFishIndex, 8, nFishTime[nFishIndex - FishType_BDengLongYu], 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(0, 50), 0.f, false );
				CreateFish( 1, nFishIndex, 9, nFishTime[nFishIndex - FishType_BDengLongYu], 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(0, -50), 0.f, false );
			}
			int nFishCount = 32;
			// 创建鱼
			if (m_iFinshLeve == 3)
			{
				nFishCount = 32;
			}
			else if (m_iFinshLeve == 2)
			{
				nFishCount = 20;
			}
			else if (m_iFinshLeve == 1)
			{
				nFishCount = 12;
			}

			for( int nIndex = 1; nIndex < nFishCount; ++nIndex )
			{
				CreateFish( 1, FishType_BHaiLuo, 16, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(DEFAULE_WIDTH * nIndex / nFishCount, -40), 0.f, false );
				CreateFish( 1, FishType_BHaiLuo, 17, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(DEFAULE_WIDTH * nIndex / nFishCount, 40), 0.f, false );
			}

			nFishCount = 16;
			if (m_iFinshLeve == 3)
			{
				nFishCount = 16;
			}
			else if (m_iFinshLeve == 2)
			{
				nFishCount = 12;
			}
			else if (m_iFinshLeve == 1)
			{
				nFishCount = 7;
			}
			for( int nIndex = 1; nIndex < nFishCount; ++nIndex )
			{
				CreateFish( 1, FishType_BHongWeiYu, 16, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(DEFAULE_WIDTH * nIndex / nFishCount, 70), 0.f, false );
				CreateFish( 1, FishType_BHongWeiYu, 17, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(DEFAULE_WIDTH * nIndex / nFishCount, -70), 0.f, false );
			}
		}
		return 52000;
	case 1:
		{

			// 创建大
			CreateFish( 1, FishType_BMoGuiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(640, 400), 0.f, false );
			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(440, 400), 0.f, false );
			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(840, 400), GL_PI, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(415, 199), 0.f, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(537, 199), 0.f, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(661, 199), 0.f, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(784, 199), 0.f, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(866, 199), 0.f, false );


			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(415, 600), GL_PI, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(537, 600), GL_PI, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(661, 600), GL_PI, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(784, 600), GL_PI, false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(866, 600), GL_PI, false );
			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(364, 206), AngleToRadian(-15), false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(242, 298), AngleToRadian(-60), false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(222, 451), AngleToRadian(-105), false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(317, 573), AngleToRadian(-150), false );



			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(918, 206), AngleToRadian(15), false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1039, 298), AngleToRadian(60), false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1059, 451), AngleToRadian(105), false );

			CreateFish( 1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(966, 573), AngleToRadian(150), false );


			//下边一排的鱼
			//CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(413, 125), 0.f, false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(518, 125), 0.f, false );	

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(641, 125), 0.f, false );	

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(770, 125), 0.f, false );	

			//CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(869, 125), 0.f, false );	

			//上边一排
		//	CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(413, 672), GL_PI, false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(490, 672), GL_PI, false );	
	
			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(641, 672), GL_PI, false );	
	
			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(800, 672), GL_PI, false );	
	
		//	CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(869, 672), GL_PI, false );	
			//弧形
			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(365, 129), AngleToRadian(-10), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(237, 190), AngleToRadian(-40), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(156, 305), AngleToRadian(-70), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(144, 446), AngleToRadian(-100), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(203, 575), AngleToRadian(-130), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(319, 656), AngleToRadian(-160), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(916, 129), AngleToRadian(10), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1044, 190), AngleToRadian(40), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1125, 305), AngleToRadian(70), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1137, 446), AngleToRadian(100), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1078, 575), AngleToRadian(130), false );

			CreateFish( 1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(962, 656), AngleToRadian(160), false );

		



			if (m_iFinshLeve == 3)
			{
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(456, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(497, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(579, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(620, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(702, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(743, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(825, 199), 0.f, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(456, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(497, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(579, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(620, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(702, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(743, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(825, 600), GL_PI, false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(317, 226), AngleToRadian(-30), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(274, 257), AngleToRadian(-45), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(222, 346), AngleToRadian(-75), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(215, 399), AngleToRadian(-90), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(242, 500), AngleToRadian(-120), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(274, 540), AngleToRadian(-135), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(364, 593), AngleToRadian(-165), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(966, 226), AngleToRadian(30), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1008, 257), AngleToRadian(45), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1059, 346), AngleToRadian(75), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1067, 399), AngleToRadian(90), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1039, 500), AngleToRadian(120), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1008, 540), AngleToRadian(135), false);
				CreateFish(1, FishType_BCaiBanYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(918, 593), AngleToRadian(165), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(451, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(489, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(565, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(603, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(679, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(717, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(793, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(831, 125), 0.f, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(451, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(489, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(565, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(603, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(679, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(717, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(793, 672), GL_PI, false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(831, 672), GL_PI, false);

				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(319, 142), AngleToRadian(-20), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(276, 162), AngleToRadian(-30), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(204, 223), AngleToRadian(-50), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(176, 262), AngleToRadian(-60), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(144, 352), AngleToRadian(-80), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(139, 399), AngleToRadian(-90), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(156, 492), AngleToRadian(-110), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(176, 535), AngleToRadian(-120), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(237, 608), AngleToRadian(-140), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(276, 636), AngleToRadian(-150), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(365, 668), AngleToRadian(-170), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(962, 142), AngleToRadian(20), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1005, 162), AngleToRadian(30), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1078, 223), AngleToRadian(50), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1105, 262), AngleToRadian(60), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1137, 352), AngleToRadian(80), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1141, 399), AngleToRadian(90), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1125, 492), AngleToRadian(110), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1105, 535), AngleToRadian(120), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1044, 608), AngleToRadian(140), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1005, 636), AngleToRadian(150), false);
				CreateFish(1, FishType_BBXiaoChouYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(916, 668), AngleToRadian(170), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(487, 36), 0.f, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(548, 36), 0.f, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(670, 36), 0.f, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(731, 36), 0.f, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(853, 36), 0.f, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(487, 760), GL_PI, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(548, 760), GL_PI, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(670, 760), GL_PI, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(731, 760), GL_PI, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(853, 760), GL_PI, false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(303, 58), AngleToRadian(-20), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(245, 85), AngleToRadian(-30), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(149, 166), AngleToRadian(-50), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(113, 217), AngleToRadian(-60), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(70, 335), AngleToRadian(-80), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(64, 399), AngleToRadian(-90), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(86, 523), AngleToRadian(-110), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(113, 580), AngleToRadian(-120), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(194, 676), AngleToRadian(-140), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(245, 712), AngleToRadian(-150), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(364, 755), AngleToRadian(-170), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(977, 58), AngleToRadian(20), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1034, 85), AngleToRadian(30), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1131, 166), AngleToRadian(50), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1167, 217), AngleToRadian(60), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1210, 335), AngleToRadian(80), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1216, 399), AngleToRadian(90), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1194, 523), AngleToRadian(110), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1167, 580), AngleToRadian(120), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1086, 676), AngleToRadian(140), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1034, 712), AngleToRadian(150), false);
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(916, 755), AngleToRadian(170), false);



				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(426, 36), 0.f, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(609, 36), 0.f, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(792, 36), 0.f, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(426, 760), GL_PI, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(609, 760), GL_PI, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(792, 760), GL_PI, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(364, 42), AngleToRadian(-10), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(194, 121), AngleToRadian(-40), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(86, 275), AngleToRadian(-70), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(70, 462), AngleToRadian(-100), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(149, 631), AngleToRadian(-130), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(303, 739), AngleToRadian(-160), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(916, 42), AngleToRadian(10), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1086, 121), AngleToRadian(40), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1194, 275), AngleToRadian(70), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1210, 462), AngleToRadian(100), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1131, 631), AngleToRadian(130), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(977, 739), AngleToRadian(160), false);
			}
			else if (m_iFinshLeve == 2)
			{
				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(426, 36), 0.f, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(609, 36), 0.f, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(792, 36), 0.f, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(426, 760), GL_PI, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(609, 760), GL_PI, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(792, 760), GL_PI, false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(364, 42), AngleToRadian(-10), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(194, 121), AngleToRadian(-40), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(86, 275), AngleToRadian(-70), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(70, 462), AngleToRadian(-100), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(149, 631), AngleToRadian(-130), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(303, 739), AngleToRadian(-160), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(916, 42), AngleToRadian(10), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1086, 121), AngleToRadian(40), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1194, 275), AngleToRadian(70), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1210, 462), AngleToRadian(100), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(1131, 631), AngleToRadian(130), false);

				CreateFish(1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(977, 739), AngleToRadian(160), false);
			}
			else if (m_iFinshLeve == 1)
			{
				
			}
		}
		return 45000;
	case 2:
		{
			// 创建小鱼
			byte cbFishSmallIndex[3] = { FishType_BXiaoHuangYu, FishType_BHaiMa, FishType_BCaiBanYu };
			uint unFishCount[3] = { 72, 48, 32 };
			uint unFishSmallCreateTime[3] = { 0, 1000, 2000 };
			uint fFishSmallRadius[3] = { 300, 240, 170 };
			uint fFishSmallIntervalTime[3] = { 9000, 7000, 5000 };

			if (m_iFinshLeve == 2)
			{
				unFishCount[0] = 35;
				unFishCount[1] = 24;
				unFishCount[2] = 16;
			}
			else if (m_iFinshLeve == 1)
			{
				unFishCount[0] = 20;
				unFishCount[1] = 18;
				unFishCount[2] = 12;
			}
			// 简单鱼阵
			for( int nIndex = 0; nIndex < 3; ++nIndex )
			{
				CircleOfFish( cbFishSmallIndex[nIndex], unFishCount[nIndex], 8, unFishSmallCreateTime[nIndex], fFishSmallRadius[nIndex], fFishSmallIntervalTime[nIndex] );
				CircleOfFish( cbFishSmallIndex[nIndex], unFishCount[nIndex], 9, unFishSmallCreateTime[nIndex], fFishSmallRadius[nIndex], fFishSmallIntervalTime[nIndex] );
			}

			// 创建中鱼
			byte cbFishBigIndex = ( rand() % 6 ) + FishType_BNianYu;
			CreateFish( 1, cbFishBigIndex, 8, 4500, 0, 0.f, false, false );
			CreateFish( 1, cbFishBigIndex, 9, 4500, 0, 0.f, false, false );
		}
		return 17000 + 6000;
	case 3:
		{
			// 创建BOSS
			CreateFish( 1, ( rand() % 4 ) + FishType_BMoGuiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(DEFAULE_WIDTH / 2, DEFAULE_HEIGHT / 2), 0.f, false );

			// 鱼位置
			int nPathIndex6 = 6;
			int nPathIndex7 = 7;

			// 偏移位置
			CShortPoint PointOffSet6;
			CShortPoint PointOffSet7;
			tagBezierPoint * pBezierPoint6 = m_ArrayFishPathPositive.GetAt(nPathIndex6)->ArrayBezierPoint.GetAt(0);
			tagBezierPoint * pBezierPoint7 = m_ArrayFishPathPositive.GetAt(nPathIndex7)->ArrayBezierPoint.GetAt(0);
			PointOffSet6.x = (short)(DEFAULE_WIDTH / 2 - pBezierPoint6->BeginPoint.x);
			PointOffSet6.y = (short)(DEFAULE_HEIGHT / 2 - pBezierPoint6->BeginPoint.y);
			PointOffSet7.x = (short)(DEFAULE_WIDTH / 2 - pBezierPoint7->BeginPoint.x);
			PointOffSet7.y = (short)(DEFAULE_HEIGHT / 2 - pBezierPoint7->BeginPoint.y);

			// 36个
			int nCountMax = 36;
			if (m_iFinshLeve == 2)
			{
				nCountMax = 20;
			}
			else if (m_iFinshLeve == 1)
			{
				nCountMax = 12;
			}
			for( int nCount = 0; nCount < nCountMax; ++nCount )
			{
				// 创建5圈鱼
				CreateFish( 1, FishType_BBXiaoChouYu, nPathIndex7, 0, 0, GL_PI * 2.f / nCountMax * nCount, false, false, KillerType_No, INVALID_CHAIR, PointOffSet7, 0.f, false );
				CreateFish( 1, FishType_BHaiLuo, nPathIndex6, 3000, 0, GL_PI * 2.f / nCountMax * nCount, false, false, KillerType_No, INVALID_CHAIR, PointOffSet6, 0.f, false );
				CreateFish( 1, FishType_BHongWeiYu, nPathIndex7, 6000, 0, GL_PI * 2.f / nCountMax * nCount, false, false, KillerType_No, INVALID_CHAIR, PointOffSet7, 0.f, false );
				CreateFish( 1, FishType_BHaiLuo, nPathIndex6, 9000, 0, GL_PI * 2.f / nCountMax * nCount, false, false, KillerType_No, INVALID_CHAIR, PointOffSet6, 0.f, false );
				CreateFish( 1, FishType_BHongWeiYu, nPathIndex7, 12000, 0, GL_PI * 2.f / nCountMax * nCount, false, false, KillerType_No, INVALID_CHAIR, PointOffSet7, 0.f, false );
			}

		}
		return 45000;
	case 4:
		{
			// 创建BOSS
			byte cbFishBigIndex = ( rand() % 4 ) + FishType_BMoGuiYu;
			CreateFish( 1, cbFishBigIndex, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(318, 400), 0.f, false );
			CreateFish( 1, cbFishBigIndex, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, CShortPoint(957, 400), 0.f, false );

			// 创建鱼
			int nCountMax = 18;
			if (m_iFinshLeve == 2)
			{
				nCountMax = 14;
			}
			else if (m_iFinshLeve == 1)
			{
				nCountMax = 10;
			}
			for( int nIndex = 0; nIndex < nCountMax; ++nIndex )
			{
				double fAngle = GL_PI * 2.0 / nCountMax * nIndex;
				CreateFish( 1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, Rotate( CShortPoint(318, 400), fAngle, CShortPoint(318, 400 + 140) ), fAngle + GL_PI, false );
				CreateFish( 1, FishType_BHongWeiYu, 15, 0, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, Rotate( CShortPoint(957, 400), fAngle, CShortPoint(957, 400 + 140) ), fAngle + GL_PI, false );
			}

			// 创建鱼
			nCountMax = 36;
			if (m_iFinshLeve == 2)
			{
				nCountMax = 22;
			}
			else if (m_iFinshLeve == 1)
			{
				nCountMax = 11;
			}
			for( int nIndex = 0; nIndex < nCountMax; ++nIndex )
			{
				double fAngle = GL_PI * 2.0 / nCountMax * nIndex;
				CreateFish( 1, FishType_BCaiBanYu, 15, 500, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, Rotate( CShortPoint(318, 400), fAngle, CShortPoint(318, 400 + 225) ), fAngle + GL_PI, false );
				CreateFish( 1, FishType_BCaiBanYu, 15, 500, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, Rotate( CShortPoint(957, 400), fAngle, CShortPoint(957, 400 + 225) ), fAngle + GL_PI, false );
			}

			// 创建鱼
			nCountMax = 72;
			if (m_iFinshLeve == 2)
			{
				nCountMax = 32;
			}
			else if (m_iFinshLeve == 1)
			{
				nCountMax = 20;
			}
			for( int nIndex = 0; nIndex < nCountMax; ++nIndex )
			{
				double fAngle = GL_PI * 2.0 / nCountMax * nIndex;
				CreateFish( 1, FishType_BXiaoHuangYu, 15, 1000, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, Rotate( CShortPoint(318, 400), fAngle, CShortPoint(318, 400 + 295) ), fAngle + GL_PI, false );
				CreateFish( 1, FishType_BXiaoHuangYu, 15, 1000, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, Rotate( CShortPoint(957, 400), fAngle, CShortPoint(957, 400 + 295) ), fAngle + GL_PI, false );
			}
		}
		return 45000;
	}

	return 0;
}


// 圆圈鱼
uint CTableFrameSink::CircleOfFish( byte cbFishType, int nFishCount, int nPathIndex, uint unCreateTime, uint nRadius, uint unIntervalTime )
{
	// 创建鱼
	for( int nIndex = 0; nIndex < nFishCount; ++nIndex )
	{
		float fRadius = (float)nRadius;
		CShortPoint PointOffset(0, (short)(fRadius * sin( GL_PI * 2.f / (float)nFishCount * (float)nIndex )));
		uint unCreateOffsetTime = (uint)(( fRadius - (fRadius * cos( GL_PI * 2.f / (float)nFishCount * (float)nIndex )) ) / (fRadius * 2.f) * unIntervalTime);
		CreateFish( 1, cbFishType, nPathIndex, unCreateTime + unCreateOffsetTime, 0, 0.f, false, false, KillerType_No, INVALID_CHAIR, PointOffset );
	}

	return nFishCount;
}


// 获取路径时间
uint CTableFrameSink::PathTime( tagFishPath * pFishPath )
{
	uint unTime = 0;
	for( int i = 0; i < pFishPath->ArrayBezierPoint.GetCount(); ++i )
	{
		unTime += pFishPath->ArrayBezierPoint[i]->Time;
	}
	return unTime;
}

// 计算当前鱼位置
EnumFishMoveType CTableFrameSink::FishMove( CDoublePoint & ptPos, tagFishInfo & TFish, uint nCustomLossTime /*= uint_max*/ )
{
	// 判断当前时间
	uint nCurrentTime = GetSysMilliseconds();

	// 流逝时间
	uint nLossTime = nCurrentTime - m_nStartTime;

	// 更新时间
	if ( nCustomLossTime != uint_max )
	{
		nLossTime = nCustomLossTime;
	}

	// 未到创建时间
	if ( nLossTime < TFish.unCreateTime )
	{
		return FishMoveType_Next;
	}

	// 以前停留时间
	uint nBeforeStayTime = 0;

	// 停留时间
	for( int nStayIndex = 0; nStayIndex < TFish.ArrayStayInfo.GetCount(); ++nStayIndex )
	{
		// 获取信息
		tagStayInfo & nStayInfo = TFish.ArrayStayInfo.GetAt(nStayIndex);

		// 获取时间
		uint nStayStart = nStayInfo.nStayStart;				
		uint nStayTime = nStayInfo.nStayTime;

		// 比较信息
		if( nLossTime <= nStayStart )
		{
			break;
		}
		else if( nLossTime > nStayStart && nLossTime < nStayStart + nStayTime )
		{
			nLossTime = nStayStart;

			break;
		}
		else if ( nLossTime >= nStayStart + nStayTime )
		{
			nBeforeStayTime += nStayTime;
		}
	}

	// 减去时间
	nBeforeStayTime -= nBeforeStayTime;

	// 游动时间
	uint unSwimTime = nLossTime - TFish.unCreateTime;
	uint unSwimCurTime = unSwimTime;
	uint unAllTime = 0;
	int  nMoveIndex = 0;
	bool bSuccess = false;

	// 获取游动索引
	for ( int nBezierIndex = 0; nBezierIndex < TFish.nBezierCount ; ++nBezierIndex )
	{
		// 获取信息
		tagBezierPoint * pBezierPoint = &TFish.TBezierPoint[nBezierIndex];
		if (!pBezierPoint)
		{
			ERROR_LOG("错误:FishMove获取鱼信息错误");
		}
		// 路径耗时
		unAllTime += pBezierPoint->Time;

		// 查找当前路径
		if ( unSwimTime <= unAllTime )
		{
			nMoveIndex = nBezierIndex;
			bSuccess = true;
			break;
		}

		// 计算当前时间
		unSwimCurTime -= pBezierPoint->Time;
	}

	// 路径结束
	if ( !bSuccess )
	{
		return FishMoveType_Delete;
	}

	// 计算贝塞尔
	double dProportion = (double)(unSwimCurTime)/(double)(TFish.TBezierPoint[nMoveIndex].Time);
	CDoublePoint ptPosition(0.0, 0.0);
	ptPosition = PointOnCubicBezier( &TFish.TBezierPoint[nMoveIndex], dProportion );

	// 计算旋转
	ptPos = Rotate(TFish.TBezierPoint[0].BeginPoint, TFish.fRotateAngle, ptPosition );

	// 偏移
	ptPos.x += TFish.PointOffSet.x;
	ptPos.y += TFish.PointOffSet.y;

	return FishMoveType_Normal;
}

// 鱼快速消息
void CTableFrameSink::FishFastMove()
{
	// 全局爆炸还原
	if ( GetFishCount(FishType_BaoZhaFeiBiao) > 0 )
	{
		// 最大倍数
		int nMultipleMax = 0;
		for ( int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex )
		{
			nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nMultipleIndex] );
		}

		// 飞镖消耗
		m_dDartStock += nMultipleMax * m_nExplosionStart;
	}

	// 清除鱼
	m_nFishTeamTime = 0;
	m_ArrayFishInfo.RemoveAll();

	// 清空宝箱信息
	for ( int nSite = 0; nSite < PlayChair_Max; ++nSite )
	{
		// 宝箱启动中
		if( m_bPlaySupply[nSite] )
		{
			m_nEnergyValue[nSite] = 0;
			m_bPlaySupply[nSite] = false;
		}
	}

	// 更新时间
	m_nStartTime = GetSysMilliseconds();
}

// 获取鱼数量
uint CTableFrameSink::GetFishCount( uint nType )
{
	// 判断类型
	if( nType >= FishType_Max )
		return 0;

	// 鱼数量
	uint nFishCount = 0;
	for ( int nIndex = 0; nIndex < m_ArrayFishInfo.GetCount(); ++nIndex )
	{
		if( m_ArrayFishInfo.GetAt(nIndex).nFishType == nType )
		{
			nFishCount++;
		}
	}

	return nFishCount;
}

// 获取特殊鱼数量
uint CTableFrameSink::GetFishSpecialCount()
{
	// 鱼数量
	uint nFishCount = 0;
	for ( int nIndex = 0; nIndex < m_ArrayFishInfo.GetCount(); ++nIndex )
	{
		if( m_ArrayFishInfo.GetAt(nIndex).nFishState == FishState_King )
		{
			nFishCount++;
		}
	}

	return nFishCount;
}

// 鱼冰冻
void CTableFrameSink::FishFreeze( uint unLossTime )
{
	// 设置停留
	uint nStayStart = unLossTime;
	uint nStayTime = 5000;

	// 鱼信息
	for( int nIndex = 0; nIndex < m_ArrayFishInfo.GetCount(); ++nIndex )
	{
		// 获取鱼信息
		tagFishInfo & TFish = m_ArrayFishInfo.GetAt(nIndex);

		// 未到时间
		if ( nStayStart < TFish.unCreateTime )
		{
			continue;
		}

		// 获取鱼数据
		CArrayStay & ArrayStayInfo = TFish.ArrayStayInfo;

		// 对比最后一个
		bool bUpLast = false;
		if ( ArrayStayInfo.GetCount() > 0 )
		{
			// 获取最后一个
			tagStayInfo & nStaylast = ArrayStayInfo.GetAt( ArrayStayInfo.GetCount() - 1 );

			// 相同时间
			if ( nStaylast.nStayStart + nStaylast.nStayTime >= nStayStart )
			{
				// 更新时间
				nStaylast.nStayTime = nStayStart - nStaylast.nStayStart + 5000;

				// 更新信息
				bUpLast = true;
				nStayStart = nStaylast.nStayStart;
				nStayTime = nStaylast.nStayTime;
			}
		}
	
		// 添加信息
		if( !bUpLast )
		{
			// 设置信息
			tagStayInfo nStayInfo;
			nStayInfo.nStayStart = nStayStart;
			nStayInfo.nStayTime = nStayTime;

			// 添加信息
			ArrayStayInfo.Add(nStayInfo);
		}

		// 定义消息
		CMD_S_StayFish CMDSStayFish;
		CMDSStayFish.nFishKey = TFish.nFishKey;
		CMDSStayFish.nStayStart = nStayStart;
		CMDSStayFish.nStayTime = nStayTime;

		// 发送消息
		SendTableData(INVALID_CHAIR, SUB_S_STAY_FISH, &CMDSStayFish, sizeof(CMD_S_StayFish));
	}
}

// 发送消息
bool CTableFrameSink::SendTableData(WORD wChairID, WORD wSubCmdID, VOID * pData, WORD wDataSize, unsigned int handleCode/* = 0*/)
{
	// 判断桌子
	if (wChairID == INVALID_CHAIR)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (GetUserIDByDeskStation(i) <= 0)
			{
				continue;
			}
			SendGameData(i, pData, wDataSize, MSG_MAIN_LOADER_GAME, wSubCmdID, handleCode);
		}
		return true;
	}
	return SendGameData(wChairID, pData, wDataSize, MSG_MAIN_LOADER_GAME, wSubCmdID, handleCode);
}

// 发送奖励提示
bool CTableFrameSink::SendAwardTip( WORD wChairID, byte nFishType, int nFishMultiple, LONGLONG lFishScore, EnumScoreType nScoreType )
{
	// 有效倍数才发送
	if ( nFishMultiple <= 0 || lFishScore <= 0 )
		return false;

	// 获取玩家
	//IServerUserItem * pIServerUserItem = m_pITableFrame->GetTableUserItem(wChairID);
	//if( pIServerUserItem == NULL )
	//	return false;
	CRedisLoader* pRedis = m_pDataManage->GetRedis();
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
		return false;
	}
	// 比赛房间不发送
	if ( m_bCompetitionRoom )
		return true;

	// 倍数太小
//	printf("倍数大小:%d,%d\n", nFishMultiple, m_nAwardMinMultiple);
	if (nFishMultiple < m_nAwardMinMultiple)
		return true;

	// 声明消息
	//CMD_S_AwardTip CMDSAwardTip;
	//CMDSAwardTip.wTableID = /*pIServerUserItem->GetTableID()*/ 1;
	//CMDSAwardTip.wChairID = wChairID;
	//_sntprintf( CMDSAwardTip.szPlayName, CountArray(CMDSAwardTip.szPlayName), TEXT("%s"), pRoomBaseInfo->name);
	//CMDSAwardTip.nFishType = nFishType;
	//CMDSAwardTip.nFishMultiple = nFishMultiple;
	//CMDSAwardTip.lFishScore = lFishScore;
	//CMDSAwardTip.nScoreType = nScoreType;

	//// 发送消息
	//SendTableData(INVALID_CHAIR, SUB_S_AWARD_TIP, &CMDSAwardTip, sizeof(CMD_S_AwardTip) );

	// 发送桌子消息
	if( m_bAwardChatBox && nFishMultiple >= m_nAwardMinMultiple && lFishScore >= 0 )
	{
		// 鱼信息
		tchar szFishName[FishType_Max][22] = {
			"小黄鱼", "小青鱼", "彩斑鱼", "海马", "小丑鱼",
			"红尾鱼", "海螺", "海螺蟹", "珊瑚鱼", "红金鱼",
			"蓝尾鱼", "灯笼鱼", "气泡鱼", "绿色气泡鱼", "盔甲鱼",
			"鲶鱼", "乌龟", "魔鬼鱼", "剑鱼", "海豚",
			"电鱼", "鲸鱼", "鲨鱼", "黄金锤头鲨", "黄金巨尺鲨",
			"黄金盔甲鱼", "黄金锤头鲨", "黄金鲸鱼", "黄金虎鲸", "史前巨鳄" };


		GameUserInfo data;
		
		if (!GetUserData(wChairID, data))
		{
			return false;
		}


		// 生成信息
		char StrAwardTip[1024];
		//if (GetRoomLevel() <= 1)
		//{
		//	StrAwardTip.Format("恭喜玩家#FF0000 | 【%s】#00FAFA | 在【BOOS来了初级场】捕获了#FF12FA | 【%s】#FFFAFA | 获得#FFFA12 | 金币#I1 | %d #C1！",
		//		data.name, szFishName[nFishType], /*nFishMultiple,*/ lFishScore);
		//}
		//else if (GetRoomLevel() == 2)
		//{
		//	StrAwardTip.Format("恭喜玩家#FF0000 | 【%s】#00FAFA | 在【BOOS来了中级场】捕获了#FF12FA | 【%s】#FFFAFA | 获得#FFFA12 | 金币#I1 | %d #C1！",
		//		data.name, szFishName[nFishType], /*nFishMultiple,*/ lFishScore);
		//}
		//else if (GetRoomLevel() == 3)
		//{
		//	StrAwardTip.Format("恭喜玩家#FF0000 | 【%s】#00FAFA | 在【BOOS来了高级场】捕获了#FF12FA | 【%s】#FFFAFA | 获得#FFFA12 | 金币#I1 | %d #C1！",
		//		data.name, szFishName[nFishType], /*nFishMultiple,*/ lFishScore);
		//}
		//else
		//{
		//获取倍率
		int Multiple = 1;
		if (Multiple <= 0)
		{
			Multiple = 1;
		}
			sprintf(StrAwardTip,"恭喜玩家#FF0000|【%s】#00FAFA|在【打BOOS】捕获了#FF12FA|【%s】#FFFAFA|获得#FFFA12|金币#I1|%2f #C1",
				data.name, szFishName[nFishType], /*nFishMultiple,*/ (double)(lFishScore)/ Multiple);
		//}

		
		// 金币捕获
		//if( nScoreType == EST_Cold )
		//{
		//	StrAwardTip.Format( "%s玩家 %s 捕中了%s，获得 %d倍 %I64d分数！", pRoomBaseInfo->name, data.name, szFishName[nFishType], nFishMultiple, lFishScore);
		//}
		//else if ( nScoreType == EST_Laser )
		//{
		//	StrAwardTip.Format( "%s玩家 %s 使用激光，获得 %d倍 %I64d分数！", StrTable, CMDSAwardTip.szPlayName, nFishMultiple, lFishScore);
		//}

		//// 末尾提示
		//if ( nFishMultiple >= 500 )
		//{
		//	StrAwardTip += "超神了！！！";
		//}
		//else if ( nFishType == FishType_BaoXiang )
		//{
		//	StrAwardTip += "运气爆表！！！";
		//}
		//else
		//{
		//	StrAwardTip += "实力超群！！！";
		//}
		m_pDataManage->SendRewardActivityNotify(StrAwardTip);
		//printf("%s\n", StrAwardTip);
		// 发送消息
	//	m_pITableFrame->SendRoomMessage( NULL, StrAwardTip, SMT_CHAT );
	}

	return true;
}

// 初始化鱼池
void CTableFrameSink::InitializationFishpond( uint nBeginTime /*= 0*/ )
{

	// 玩家数量
	int nPlayCount = 0;
	for( int nSite = 0; nSite < PlayChair_Max; ++nSite )
	{
		if( GetUserIDByDeskStation(nSite) != NULL )
			nPlayCount++;
	}

	// 更新数量
	int nCreateCount = 0;
	if (nPlayCount > 1)
	{
		nCreateCount = Min_(nPlayCount-1, 4) * m_iAddFinshCount + m_nCreateCount;
	}
	else
	{
		nCreateCount = Min_(nPlayCount, 4) * m_iAddFinshCount + m_nCreateCount;
	}
	
	
	// 获取有效数量
	int nValidCount = 0;
	for( int nFishIndex = 0; nFishIndex < m_ArrayFishInfo.GetCount(); ++nFishIndex )
	{
		nValidCount += (m_ArrayFishInfo.GetAt(nFishIndex).bRepeatCreate) ? 1 : 0;
	}

	// 判断数量
	if( nValidCount > nCreateCount )
	{
		return;
	}

	// 创建时间
	uint unCreateTime = nBeginTime;

	// 小鱼群
	if( (m_nAppearFishCount % 16) == 0 )
	{
		// 小鱼数量
		int nNumber = (rand() % 3) + 3;
		byte cbFishType = RandomArea(6, 21, 19, 18, 16,13,12);

		// 正常鱼群
		if ( rand() % 2 == 0 )
		{
			CreateFish( nNumber, cbFishType, uint_max, unCreateTime, 1000, 0.f, true, true, KillerType_No, INVALID_CHAIR, CShortPoint(0, 0), 0.f, false );
		}
		// 爆炸鱼群
		else
		{
			// 随机路径
			int nBezierCount = 0;
			tagBezierPoint TBezierPoint[BEZIER_POINT_MAX];
			RandomPath( cbFishType, TBezierPoint, nBezierCount );
			CreateFishEx( 1, cbFishType, TBezierPoint, nBezierCount, unCreateTime, 0, 0.f, true, false, KillerType_All, INVALID_CHAIR, CShortPoint(-50, 0), 0.f, false );
			CreateFishEx( 1, cbFishType, TBezierPoint, nBezierCount, unCreateTime, 0, 0.f, true, false, KillerType_All, INVALID_CHAIR, CShortPoint(50, 0), 0.f, false );
			CreateFishEx( 1, cbFishType, TBezierPoint, nBezierCount, unCreateTime, 0, 0.f, true, false, KillerType_All, INVALID_CHAIR, CShortPoint(0, 50), 0.f, false );
			CreateFishEx( 1, cbFishType, TBezierPoint, nBezierCount, unCreateTime, 0, 0.f, true, false, KillerType_All, INVALID_CHAIR, CShortPoint(0, -50), 0.f, false );
		}
	}
	// 单条鱼
	else
	{
		if (rand()%100 < m_iCreatJiLv)
		{
			CreateFish(1, RandomArea(7, 30, 30,25,20,10,10,10), uint_max, unCreateTime, 0, 0.f, true, true, (RandomArea(2, 1, 99) == 0) ? KillerType_One : KillerType_No);
		}
		else
		{
			CreateFish(1, FishType_Max, uint_max, unCreateTime, 0, 0.f, true, true, (RandomArea(2, 1, 99) == 0) ? KillerType_One : KillerType_No);
		}

	}
}

// 捕获到鱼
LONGLONG CTableFrameSink::PlayCatchFish( WORD wChairID, int nBulletSplit, uint nFishCatchKey, int nMultipleIndex, EnumPlayCatchType nPlayCatchType )
{
	// 暂无倍数
	if( nMultipleIndex == Multiple_Max )
	{
		_Assert( false && "捕获鱼倍数异常。" );

		return 0;
	}
	if (GetUserIDByDeskStation(wChairID) <= 0)
	{
		return 0;
	}
	// 死鱼信息
	int nDeathScore = 0;
	int nDeathCount = 0;

	// 更新分数几率
	for( int nFishIndex = 0; nFishIndex < m_ArrayFishInfo.GetCount(); ++nFishIndex )
	{
		// 获取信息
		tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt(nFishIndex);

		// 更新分数几率
		TFishInfo.nScoreChance = TFishInfo.nFishScore;
	}

	// 遍历鱼
	for( int nFishIndex = 0; nFishIndex < m_ArrayFishInfo.GetCount(); ++nFishIndex )
	{
		// 获取信息
		tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt(nFishIndex);

		// 非捕获鱼
		if ( TFishInfo.nFishKey != nFishCatchKey )
		{
			continue;
		}

		// 修改信息
		nDeathScore += TFishInfo.nFishScore;
		nDeathCount += 1;

		// 特殊鱼过滤
		if ( TFishInfo.nFishState != FishState_Killer /*&& TFishInfo.nFishType != FishType_ZhongYiTang*/ )
		{
			// 返回
			break;
		}

		// 有效位置
		CDoublePoint PointFish;
		if ( FishMove( PointFish, TFishInfo ) != FishMoveType_Normal )
		{
			// 返回
			break;
		}

		// 计算周边鱼
		for( int nFishPeripheryIndex = 0; nFishPeripheryIndex < m_ArrayFishInfo.GetCount(); ++nFishPeripheryIndex )
		{
			// 获取信息
			tagFishInfo & TPeripheryInfo = m_ArrayFishInfo.GetAt(nFishPeripheryIndex);

			// 重复比较
			if ( TPeripheryInfo.nFishKey == nFishCatchKey )
			{
				continue;
			}

			// 有效位置
			CDoublePoint PointPeriphery;
			if ( FishMove( PointPeriphery, TPeripheryInfo ) != FishMoveType_Normal )
			{
				continue;
			}

			// 连带鱼
			bool bRelatedFish = false;

			// 杀手鱼
			if ( TFishInfo.nFishState == FishState_Killer )
			{
				// 同类鱼
				bRelatedFish = TFishInfo.nFishType == TPeripheryInfo.nFishType;
			}
			//else if ( TFishInfo.nFishType == FishType_ZhongYiTang )
			//{
			//	// 短距离
			//	bRelatedFish = DistanceOfPoint(PointFish, PointPeriphery) < 300.0;
			//}

			// 有效连带
			if( bRelatedFish )
			{
				// 修改几率
				TFishInfo.nScoreChance += TPeripheryInfo.nFishScore;

				// 添加分数
				nDeathScore += TPeripheryInfo.nFishScore;
				nDeathCount += 1;
			}
		}

		// 返回
		break;
	}

	// 无效捕获
	if( nDeathScore == 0 || nDeathCount == 0 )
	{
		return 0;
	}

	// 玩家指针
	//IServerUserItem * pIServerUserItem = m_pITableFrame->GetTableUserItem( wChairID );
	
	// 难度值
	double dModifyChance = DifficultyGame( wChairID, nMultipleIndex );
	//printf("捕到鱼:索引%d,总难度%llf\n", nMultipleIndex, dModifyChance);
	// 分数信息
	double dBulletScore = static_cast<double>(m_nMultipleValue[nMultipleIndex] * QianPao_Bullet);
	double dDeathScore = static_cast<double>(m_nMultipleValue[nMultipleIndex] * nDeathScore);

	// 几率信息
	nBulletSplit = Max_( 1, nBulletSplit );
	double dAppendChance = dBulletScore / (double)nBulletSplit / dDeathScore;
	double dBulletScale = (double)QianPao_Bullet / (double)nBulletSplit / (double)nDeathCount;

	// 捕获总分数
	LONGLONG lTotalCatchScore = 0;

	// 杀手鱼
	bool bFishKiller = false;
	byte nFishTypeKiller = FishType_Invalid;

	// 特殊鱼
	bool bFishSpecial = false;
	byte cbFishTypeSpecial = 0;
	CDoublePoint PointSpecial;

	// 停留鱼死亡
	bool bStayFish = false;

	// 全屏死亡
	bool bFishFullScreen = false;

	// 爆炸鱼死亡
	bool bFishExplosion = false;
	CDoublePoint PointExplosion;

	// 捕获鱼数据
	Static CWHArray< tagCatchInfo > ArrayCatchFishInfo;	
	ArrayCatchFishInfo.RemoveAll();

	// 防止负数, 增加难度
	//if ( m_dRoomStock[nMultipleIndex] < dDeathScore )
	//{
	//	dModifyChance = Min_( 0.15, dModifyChance );
	//}

	//// 负数太厉害
	//if ( m_dRoomStock[nMultipleIndex] < -100 * m_nMultipleValue[nMultipleIndex] )
	//{
	//	dModifyChance = Min_( 0.05, dModifyChance );
	//}

	// 机器人默认难度
	if ( IsAndroidUser(wChairID) )
	{
		// 默认难度
		dModifyChance = 1.0;

		// 最大倍数
		int nMultipleMax = 0;
		for ( int nIndex = 0; nIndex < Multiple_Max; ++nIndex )
		{
			nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nIndex] );
		}

		// 机器人赢得太多
		if ( m_lPlayScore[wChairID] - m_lPlayStartScore[wChairID] > 2000 * nMultipleMax )
		{
			dModifyChance = 0.5;
		}
	}

	// 比赛房间统一难度
	if( m_bCompetitionRoom )
	{
		dModifyChance = Max_(1.2, dModifyChance);
	}

	// 遍历鱼
	for( int nDeathIndex = 0; nDeathIndex < m_ArrayFishInfo.GetCount(); nDeathIndex++ )
	{
		// 获取鱼信息
		tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt(nDeathIndex);

		// 非捕获鱼
		if ( TFishInfo.nFishKey != nFishCatchKey )
		{
			continue;
		}

		// 全局爆炸判断
		LONGLONG lPlayExplosionCondition = 0;

		auto iter = m_MapPlayExplosionCondition.find(GetUserIDByDeskStation(wChairID));
		if (iter != m_MapPlayExplosionCondition.end())
		{
			lPlayExplosionCondition = iter->second;
		}

		//m_MapPlayExplosionCondition.Lookup( GetUserIDByDeskStation(wChairID), lPlayExplosionCondition );

		if( TFishInfo.nFishType == FishType_BaoZhaFeiBiao && lPlayExplosionCondition < m_lExplosionCondition )
		{
			continue;
		}

		// 特殊必死
		bool bSpecialDie = false;

		// 如果是宝箱, 宝箱几率直接20%
		if ( TFishInfo.nFishType == FishType_BaoXiang && RAND_TRUE_FALSE(20, 80) )
		{
			bSpecialDie = true;
		}

		// 如果是激光必中
		if ( nPlayCatchType == EPCT_Laser )
		{
			bSpecialDie = true;
		}

		// 捕获几率
		int nCaptureChance = (int)(dBulletScale * 100.0 * dModifyChance);
		double dWillBeIn = 1.2 / dModifyChance;

		// 优化几率
		nCaptureChance = Max_( nCaptureChance, 1 );

		// 非特殊处理, 系统防止负数
		//if ( !bSpecialDie && m_dRoomStock[nMultipleIndex] < dDeathScore && !m_bCompetitionRoom )
		//{
		//	dWillBeIn = Max_( 2.0, dWillBeIn );
		//	nCaptureChance = 1;
		//}

		// 添加基数
		TFishInfo.dCurChance += dAppendChance;

		// 捕获到鱼
		if ( bSpecialDie || TFishInfo.dCurChance > dWillBeIn || ( RAND_TRUE_FALSE( nCaptureChance, TFishInfo.nScoreChance * 100 ) ) )
		{
			// 捕获爆炸鱼
			if( TFishInfo.nFishType == FishType_BaoZhaFeiBiao )
			{
				// 清空数据
				//m_MapPlayExplosionCondition.SetAt(GetUserIDByDeskStation(wChairID), 0 );
				m_MapPlayExplosionCondition[GetUserIDByDeskStation(wChairID)] = 0;
			}

			// 捕获中鱼
			lTotalCatchScore += CatchInFish( wChairID, nMultipleIndex, nPlayCatchType, TFishInfo, ArrayCatchFishInfo );

			// 记录杀手鱼
			if ( TFishInfo.nFishState == FishState_Killer && !bFishKiller )
			{
				// 记录信息
				CDoublePoint PointKeller;
				bFishKiller = (FishMove( PointKeller, TFishInfo ) == FishMoveType_Normal);
				nFishTypeKiller = TFishInfo.nFishType;
			}

			// 记录鱼王
			if ( TFishInfo.nFishState == FishState_King && !bFishSpecial )
			{
				// 记录信息
				bFishSpecial = (FishMove( PointSpecial, TFishInfo ) == FishMoveType_Normal);
				cbFishTypeSpecial = TFishInfo.nFishType;
			}

			// 记录停留鱼
			//if ( TFishInfo.nFishType == FishType_ShuiHuZhuan && !bStayFish )
			//{
			//	// 记录信息
			//	CDoublePoint PointStay;
			//	bStayFish = (FishMove( PointStay, TFishInfo ) == FishMoveType_Normal);
			//}

			//// 记录全屏爆炸
			//if( TFishInfo.nFishType == FishType_BaoZhaFeiBiao && !bFishFullScreen )
			//{
			//	// 记录信息
			//	CDoublePoint PointFullScreen;
			//	bFishFullScreen = (FishMove( PointFullScreen, TFishInfo ) == FishMoveType_Normal);
			//}

			//// 记录爆炸鱼
			//if ( TFishInfo.nFishType == FishType_ZhongYiTang && !bFishExplosion )
			//{
			//	// 记录信息
			//	bFishExplosion = (FishMove( PointExplosion, TFishInfo ) == FishMoveType_Normal);
			//}

			// 删除鱼
			m_ArrayFishInfo.RemoveAt(nDeathIndex);
		}

		// 退出
		break;
	}

	// 杀手鱼计算
	if ( bFishKiller )
	{
		// 遍历鱼
		int nDeathIndex = 0; 
		while( nDeathIndex < m_ArrayFishInfo.GetCount() )
		{
			// 获取信息
			tagFishInfo & TPeripheryInfo = m_ArrayFishInfo.GetAt(nDeathIndex);

			// 周边鱼距离
			CDoublePoint PointPeriphery;
			if( FishMove( PointPeriphery, TPeripheryInfo ) == FishMoveType_Normal && nFishTypeKiller == TPeripheryInfo.nFishType )
			{
				// 捕获中鱼
				lTotalCatchScore += CatchInFish( wChairID, nMultipleIndex, EPCT_Killer, TPeripheryInfo, ArrayCatchFishInfo );

				// 删除鱼
				m_ArrayFishInfo.RemoveAt(nDeathIndex);

				// 继续
				continue;
			}

			// 下一个
			++nDeathIndex;
		}
	}

	// 特殊鱼计算
	if ( bFishSpecial && m_bOpenShoalFish)
	{
		// 创建鱼阵
		SpecialFishMatrix( cbFishTypeSpecial, PointSpecial );
	}

	// 全屏爆炸
	if ( bFishFullScreen )
	{
		// 遍历鱼
		LONGLONG lFishFullScreenScore = 0;
		int nDeathIndex = 0; 
		while( nDeathIndex < m_ArrayFishInfo.GetCount() )
		{
			// 获取信息
			tagFishInfo & TPeripheryInfo = m_ArrayFishInfo.GetAt(nDeathIndex);

			// 周边鱼距离
			CDoublePoint PointPeriphery;
			if( FishMove( PointPeriphery, TPeripheryInfo ) == FishMoveType_Normal && PointPeriphery.x > 0 && PointPeriphery.x < DEFAULE_WIDTH && PointPeriphery.y > 0 && PointPeriphery.y < DEFAULE_HEIGHT )
			{
				// 捕获中鱼
				LONGLONG lCatchScore = CatchInFish( wChairID, nMultipleIndex, EPCT_FullScreen, TPeripheryInfo, ArrayCatchFishInfo );
				lTotalCatchScore += lCatchScore;
				lFishFullScreenScore += lCatchScore;

				// 删除鱼
				m_ArrayFishInfo.RemoveAt(nDeathIndex);

				// 继续
				continue;
			}

			// 下一个
			++nDeathIndex;
		}

		// 最大倍数
		int nMultipleMax = 0;
		for ( int nIndex = 0; nIndex < Multiple_Max; ++nIndex )
		{
			nMultipleMax = Max_( nMultipleMax, m_nMultipleValue[nIndex] );
		}

		// 还原分数
		if( !IsAndroidUser(wChairID) )
		{
			m_dDartStock += nMultipleMax * m_nExplosionStart - lFishFullScreenScore;
			//m_dRoomStock[nMultipleIndex] += lFishFullScreenScore;
		}
		else
		{
			m_dDartStock += nMultipleMax * m_nExplosionStart;
		}
	}

	// 局部爆炸
	if( bFishExplosion )
	{
		// 遍历鱼
		int nDeathIndex = 0; 
		while( nDeathIndex < m_ArrayFishInfo.GetCount() )
		{
			// 获取信息
			tagFishInfo & TPeripheryInfo = m_ArrayFishInfo.GetAt(nDeathIndex);

			// 周边鱼距离
			CDoublePoint PointPeriphery;
			if( FishMove( PointPeriphery, TPeripheryInfo ) == FishMoveType_Normal && DistanceOfPoint(PointExplosion, PointPeriphery) < 300.0 && TPeripheryInfo.nFishType != FishType_BaoZhaFeiBiao )
			{
				// 捕获中鱼
				lTotalCatchScore += CatchInFish( wChairID, nMultipleIndex, EPCT_Explosion, TPeripheryInfo, ArrayCatchFishInfo );

				// 删除鱼
				m_ArrayFishInfo.RemoveAt(nDeathIndex);

				// 继续
				continue;
			}

			// 下一个
			++nDeathIndex;
		}
	}

	// 发送捕获消息
	SendCatchFish( wChairID, ArrayCatchFishInfo );

	// 停留消息
	if ( bStayFish )
	{
		//FishFreeze( GetSysMilliseconds() - m_nStartTime );
	}

	// 返回信息
	return lTotalCatchScore;
}

// 比较鱼
bool CTableFrameSink::ISFishCatchKey( uint nFishKey, CWHArray< uint > & ArrayFishCatchKey )
{
	// 遍历捕获
	for ( int nCatchIndex = 0; nCatchIndex < ArrayFishCatchKey.GetCount(); ++nCatchIndex )
	{
		// 判断有效捕获
		if ( ArrayFishCatchKey[nCatchIndex] != 0 && ArrayFishCatchKey[nCatchIndex] == nFishKey )
		{
			return true;
		}
	}

	return false;
}

// 捕中鱼
LONGLONG CTableFrameSink::CatchInFish( WORD wChairID, int nMultipleIndex, EnumPlayCatchType nPlayCatchType, tagFishInfo & TFishInfo, CWHArray< tagCatchInfo > & ArrayCatchFishInfo )
{
	// 鱼分数
	int nFishScore = TFishInfo.nFishScore;

	// 添加水草
	if( TFishInfo.nFishState == FishState_Aquatic )
	{
		nFishScore += 8;
	}

	// 盒子爆炸
	if( TFishInfo.nFishType == FishType_BaoXiang )
	{
		nFishScore = 0;
	}

	//如果是道具直接加入背包
	if (TFishInfo.nFishType == FishType_Hong_Bao)
	{
		nFishScore = 0;
		ChangeUserBage(wChairID,BAG_HONG_BAO);
	}

	if (TFishInfo.nFishType == FishType_Hua_Fei1)
	{
		nFishScore = 0;
		ChangeUserBage(wChairID, BAG_PHONE_CARD1);
	}

	if (TFishInfo.nFishType == FishType_Hua_Fei2)
	{
		nFishScore = 0;
		ChangeUserBage(wChairID, BAG_PHONE_CARD5);
	}

	if (TFishInfo.nFishType > m_ipropNum)
	{
		int Tmp = CUtil::GetRandRange(0, 100);
		if (Tmp > 50)
		{
			if (CUtil::GetRandRange(1, 1000) < m_iBingDong)
			{
				ChangeUserBage(wChairID, BAG_BING_DONG);
			}
		}
		else
		{
			if (CUtil::GetRandRange(1, 1000) < m_iLocking)
			{
				ChangeUserBage(wChairID, BAG_LOCK);
			}
		}	
	}
	// 捕获信息
	LONGLONG lCatchScore = (LONGLONG)((LONGLONG)nFishScore * (LONGLONG)m_nMultipleValue[nMultipleIndex]);

	// 玩家收入
	PlayerIncome( wChairID, nMultipleIndex, lCatchScore );

	// 盒子爆炸
	if( TFishInfo.nFishType == FishType_BaoXiang )
	{
		// 活动重载
		m_bPlaySupply[wChairID] = false;

		// 如果是机器人，就不要出现激光
		int nLaserChance = m_nLaserChance;
		if ( IsAndroidUser(wChairID) )
		{
			nLaserChance = 0;
		}

		// 补给活动
		int nBoxActivity = RandomArea(8, nLaserChance, m_nSpeedChance, m_nGiftChance[0], m_nGiftChance[1], m_nGiftChance[2], m_nGiftChance[3], m_nGiftChance[4], m_nNullChance);

		// 如果库存为过低，只出强化类
		//if( m_dRoomStock[nMultipleIndex] < m_nMultipleValue[nMultipleIndex] * 100.0 )
		//{
		//	nBoxActivity = RandomArea(8, 0, m_nSpeedChance, 0, 0, 0, 0, 0, m_nNullChance);
		//}

		// 判断活动
		switch (nBoxActivity)
		{
		case 0:	// 激光
			{
				// 更新激光时间（5秒误差时间）
				m_nLaserPlayTime[wChairID] = m_nLaserTime + 5;

				// 发送激光消息
				CMD_S_Supply CMDSSupply;
				CMDSSupply.wChairID = wChairID;
				CMDSSupply.lSupplyCount = m_nLaserTime;
				CMDSSupply.nSupplyType = EST_Laser;
				SendTableData(INVALID_CHAIR, SUB_S_SUPPLY, &CMDSSupply, sizeof(CMDSSupply) );
			}
			break;
		case 1:	// 加速
			{
				// 更新加速时间（2秒误差时间）
				m_nSpeedPlayTime[wChairID] = m_nSpeedTime + 2;

				// 发送加速消息
				CMD_S_Supply CMDSSupply;
				CMDSSupply.wChairID = wChairID;
				CMDSSupply.lSupplyCount = m_nSpeedTime;
				CMDSSupply.nSupplyType = EST_Speed;
				SendTableData(INVALID_CHAIR, SUB_S_SUPPLY, &CMDSSupply, sizeof(CMDSSupply) );
			}
			break;
		case 2:	// 赠送金币
		case 3:
		case 4:
		case 5:
		case 6:
			{
				// 更新捕获分数
				lCatchScore = m_nGiftScore[ nBoxActivity - 2 ] * TFishInfo.nFishScore;

				// 发送赠送消息
				CMD_S_Supply CMDSSupply;
				CMDSSupply.wChairID = wChairID;
				CMDSSupply.lSupplyCount = lCatchScore;
				CMDSSupply.nSupplyType = EST_Gift;
				SendTableData(INVALID_CHAIR, SUB_S_SUPPLY, &CMDSSupply, sizeof(CMDSSupply) );

				// 玩家收入
				PlayerIncome( wChairID, nMultipleIndex, lCatchScore );

				// 大倍发送信息
				SendAwardTip(wChairID, FishType_BaoXiang, m_nGiftScore[ nBoxActivity - 2 ], lCatchScore, EST_Cold);
			//	printf("箱子发送:%d\n", m_nGiftScore[nBoxActivity - 2]);
			}
			break;
		case 7:	// 空宝箱
			{
				// 发送空宝箱消息
				CMD_S_Supply CMDSSupply;
				CMDSSupply.wChairID = wChairID;
				CMDSSupply.lSupplyCount = 0;
				CMDSSupply.nSupplyType = EST_Null;
				SendTableData(INVALID_CHAIR, SUB_S_SUPPLY, &CMDSSupply, sizeof(CMDSSupply) );
			}
			break;
		}
	}
	else
	{
		
		// 大倍发送信息
		if (m_nFishMultiple[TFishInfo.nFishScore][0] >= m_nAwardMinMultiple && nPlayCatchType == EPCT_Bullet )
		{	
			SendAwardTip( wChairID, TFishInfo.nFishType, m_nFishMultiple[TFishInfo.nFishType][0], lCatchScore, EST_Cold );
			//printf("大倍数发送:%d,鱼种类:%d,最小公告倍数:%d\n", m_nFishMultiple[TFishInfo.nFishType][0], TFishInfo.nFishType, m_nAwardMinMultiple);
		}
	}

	// 添加抽水
	if (lCatchScore > 0)
	{
		m_lplayCutMoney[wChairID] += lCatchScore * m_iRate / 100;
		lCatchScore -= lCatchScore * m_iRate / 100;
	}


	// 添加分数
	m_lPlayScore[wChairID] += lCatchScore;

	// 玩家捕获数量
	m_lPlayFishCount[wChairID][TFishInfo.nFishType]++;

	// 设置捕获
	tagCatchInfo TCatchInfo;
	TCatchInfo.nMultipleCount = TFishInfo.nFishScore;
	TCatchInfo.nFishKey = TFishInfo.nFishKey;
	TCatchInfo.lScoreCount = lCatchScore;

	// 记录分数
	ArrayCatchFishInfo.Add(TCatchInfo);

	return lCatchScore;
}

// 发送捕获消息
void CTableFrameSink::SendCatchFish( WORD wChairID, CWHArray< tagCatchInfo > & ArrayCatchFishInfo )
{
	// 发送死亡消息
	WORD wBufferSize = 0;
	byte cbBuffer[IPC_PACKAGE];
	int nSendIndex = 0;
	while( nSendIndex < ArrayCatchFishInfo.GetCount() )
	{
		// 超出界限
		if ( (wBufferSize + sizeof(CMD_S_CatchFish) + 1) > sizeof(cbBuffer) )
		{
			// 发送消息
			SendTableData(INVALID_CHAIR, SUB_S_FISH_CATCH, cbBuffer, wBufferSize);

			// 归零消息
			wBufferSize = 0;
		}

		// 获取信息
		tagCatchInfo & TCatchInfo = ArrayCatchFishInfo.GetAt(nSendIndex);

		// 定义消息
		CMD_S_CatchFish CMDCatchFish;
		CMDCatchFish.wChairID = wChairID;
		CMDCatchFish.nFishKey = TCatchInfo.nFishKey;
		CMDCatchFish.nMultipleCount = TCatchInfo.nMultipleCount;
		CMDCatchFish.lScoreCount = TCatchInfo.lScoreCount;

		// 构造消息
		CopyMemory(cbBuffer + wBufferSize, &CMDCatchFish, sizeof(CMD_S_CatchFish) );
		wBufferSize += sizeof(CMD_S_CatchFish);

		// 下一个
		nSendIndex++;
	}

	// 有效消息
	if ( wBufferSize > 0 ) 
	{
		// 发送消息
		SendTableData(INVALID_CHAIR, SUB_S_FISH_CATCH, cbBuffer, wBufferSize);
	}

	return ;
}

// 游戏难度
double CTableFrameSink::DifficultyGame( WORD wChairID, int nMultipleIndex )
{
	// 房间难度
	double dRoomDifficulty = DifficultyRoom( nMultipleIndex );

	// 桌子难度
	//double dTableDifficulty = DifficultyTable( nMultipleIndex );

	// 个人难度
	//double dPlayDifficulty = DifficultyPlay( wChairID, nMultipleIndex );

	// 游戏难度
	double dGameDifficulty = 0.0;
	int nDifficultyCount = 0;
	//个人难度
	double dUserdifficulty = 1;
	long userID = GetUserIDByDeskStation(wChairID);
	if (userID > 0)
	{
		GetUserDifficulty(userID, dUserdifficulty);
	}
	//printf("个人难度:%llf\n", dUserdifficulty);
	// 有效难度
	if (m_nDifficultyStart[0])
	{
		dGameDifficulty += dUserdifficulty;
		nDifficultyCount += 1;
	}

	// 有效难度
	if( m_nDifficultyStart[0] )
	{
		dGameDifficulty += dRoomDifficulty;
		nDifficultyCount += 1;
	}


	// 无效启动
	if ( nDifficultyCount == 0 )
	{
		dGameDifficulty = 0.5;
	}
	else
	{
		dGameDifficulty /= nDifficultyCount;
	}


	// 个人难度
	//int UserDifficulty = GetUserControlParam(wChairID);
	//if (UserDifficulty >=0 && UserDifficulty<1000)
	//{
	//	int Lev = UserDifficulty / 100;
	//	dGameDifficulty = m_dPlayDifficultyValue[Lev];
	//	//printf("个人难度:%d,难度等级:%d,难度系数%lf\n", UserDifficulty, Lev, RoomDifficultyCount[Lev]);
	//}
	//else if (UserDifficulty >= 1000)
	//{
	//	dGameDifficulty = m_dPlayDifficultyValue[9];
	//}
	// 难度值
	//printf("难度值%lf\n", dGameDifficulty);
	return dGameDifficulty;
}

// 游戏难度
double CTableFrameSink::DifficultyRoom( int nMultipleIndex )
{
	// 房间难度
	double dRoomDifficulty = 1.0;
	//难度系数
	//double m_dRoomDifficultyValue[10] = { 0.05,0.15,0.25,0.5,0.75,1,1.25,1.7,2.5,3.5 };
	//printf("游戏难度:%lf\n", m_pooInfo.platformCtrlPercent);
	if (m_pDataManage->LoadPoolData())
	{
		if (m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent <= 0)
		{
			dRoomDifficulty  == 1;
		}
		else
		{
			int Lev = 5;
			if (m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent >= 1000 )
			{
				m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent = 999;
			}
			else
			{
				Lev = m_pDataManage->m_rewardsPoolInfo.platformCtrlPercent / 100;
			}
			if (Lev >= 0 && Lev < Difficulty_Max)
			{
				dRoomDifficulty = m_dRoomDifficultyValue[Lev];
			}
			//printf("房间难度:%d,难度等级:%d,难度系数%lf\n", m_pooInfo.platformCtrlPercent, Lev, RoomDifficultyCount[Lev]);
		}
	}
	
	// 难度索引
	//int nDifficultyIndex = 0;

	//// 计算房间难度
	//for ( nDifficultyIndex = 0; nDifficultyIndex < Difficulty_Max; ++nDifficultyIndex )
	//{
	//	// 比较信息
	//	if( m_dRoomStock[nMultipleIndex] < m_nRoomDifficultyCount[nDifficultyIndex] * m_nMultipleValue[nMultipleIndex] )
	//	{
	//		// 设置难度
	//		dRoomDifficulty = m_dRoomDifficultyValue[nDifficultyIndex];

	//		break;
	//	}
	//}

	//// 无效难度
	//if( nDifficultyIndex == Difficulty_Max )
	//{
	//	// 设置难度
	//	dRoomDifficulty = m_dRoomDifficultyValue[Difficulty_Max - 1];
	//}

	return dRoomDifficulty;
}

// 游戏难度
double CTableFrameSink::DifficultyTable( int nMultipleIndex )
{
	// 桌子难度
	double dTableDifficulty = 1.0;

	// 难度索引
	int nDifficultyIndex = 0;

	// 计算桌子难度
	for ( nDifficultyIndex = 0; nDifficultyIndex < Difficulty_Max; ++nDifficultyIndex )
	{
		// 比较信息
		if( _TableStock(nMultipleIndex) < m_nTableDifficultyCount[nDifficultyIndex] * m_nMultipleValue[nMultipleIndex] )
		{
			// 设置难度
			dTableDifficulty = m_dTableDifficultyValue[nDifficultyIndex];

			break;
		}
	}

	// 无效难度
	if( nDifficultyIndex == Difficulty_Max )
	{
		// 设置难度
		dTableDifficulty = m_dTableDifficultyValue[Difficulty_Max - 1];
	}

	return dTableDifficulty;
}

// 游戏难度
double CTableFrameSink::DifficultyPlay( WORD wChairID, int nMultipleIndex )
{
	// 个人难度
	double dPlayDifficulty = 1.0;

	// 难度索引
	int nDifficultyIndex = 0;

	// 计算个人难度
	for ( nDifficultyIndex = 0; nDifficultyIndex < Difficulty_Max; ++nDifficultyIndex )
	{
		// 比较信息
		if( _PlayerStock(wChairID, nMultipleIndex) < m_nPlayDifficultyCount[nDifficultyIndex] * m_nMultipleValue[nMultipleIndex] )
		{
			// 设置难度
			dPlayDifficulty = m_dPlayDifficultyValue[nDifficultyIndex];

			break;
		}
	}

	// 无效难度
	if( nDifficultyIndex == Difficulty_Max )
	{
		// 设置难度
		dPlayDifficulty = m_dPlayDifficultyValue[Difficulty_Max - 1];
	}

	return dPlayDifficulty;
}


// 交换变量
template< typename T > void CTableFrameSink::SwapVariable( T & One, T & Two )
{
	T Temp = One;
	One = Two;
	Two = Temp;
}

// 旋转点
CDoublePoint CTableFrameSink::Rotate(const CDoublePoint & ptCircle, double dRadian, const CDoublePoint & ptSome )
{ 
	CDoublePoint temp; 
	CDoublePoint tempPtSome(ptSome);
	tempPtSome.x -= ptCircle.x; 
	tempPtSome.y -= ptCircle.y;
	temp.x = tempPtSome.x*cos(dRadian) - tempPtSome.y*sin(dRadian) + ptCircle.x;
	temp.y = tempPtSome.y*cos(dRadian) + tempPtSome.x*sin(dRadian) + ptCircle.y;
	return temp; 
} 

// 旋转点
CShortPoint CTableFrameSink::Rotate(const CShortPoint & ptCircle, double dRadian, const CShortPoint & ptSome )
{ 
	CShortPoint temp; 
	CShortPoint tempPtSome(ptSome);
	tempPtSome.x -= ptCircle.x;
	tempPtSome.y -= ptCircle.y;
	temp.x = (short)(tempPtSome.x*cos(dRadian) - tempPtSome.y*sin(dRadian) + ptCircle.x);
	temp.y = (short)(tempPtSome.y*cos(dRadian) + tempPtSome.x*sin(dRadian) + ptCircle.y);
	return temp; 
} 

// 已知圆心,弧度,半径 求圆上任意一
CShortPoint CTableFrameSink::RotatePoint( CShortPoint & ptCircle, double dRadian, double dRadius )
{
	CShortPoint temp;
	temp.x = (short)(dRadius*cos(dRadian) + ptCircle.x);
	temp.y = (short)(dRadius*sin(dRadian) + ptCircle.y);
	return temp;
}

// 贝塞尔曲线
CDoublePoint CTableFrameSink::PointOnCubicBezier( tagBezierPoint * cp, double t )
{
	double   ax = 0.f, bx = 0.f, cx = 0.f;
	double   ay = 0.f, by = 0.f, cy = 0.f;
	double   tSquared = 0.f, tCubed = 0.f;
	CDoublePoint result;

	// 计算系数
	cx = 3.f * (cp->KeyOne.x - cp->BeginPoint.x);
	bx = 3.f * (cp->KeyTwo.x - cp->KeyOne.x) - cx;
	ax = cp->EndPoint.x - cp->BeginPoint.x - cx - bx;

	cy = 3.f * (cp->KeyOne.y - cp->BeginPoint.y);
	by = 3.f * (cp->KeyTwo.y - cp->KeyOne.y) - cy;
	ay = cp->EndPoint.y - cp->BeginPoint.y - cy - by;

	// 计算曲线点
	tSquared = t * t;
	tCubed = tSquared * t;

	result.x = (ax * tCubed) + (bx * tSquared) + (cx * t) + cp->BeginPoint.x;
	result.y = (ay * tCubed) + (by * tSquared) + (cy * t) + cp->BeginPoint.y;

	return result;
}

// 两点距离
double CTableFrameSink::DistanceOfPoint( CDoublePoint & PointOne, CDoublePoint & PointTwo )
{
	return sqrt( pow( abs(PointOne.x - PointTwo.x), 2 ) + pow( abs(PointOne.y - PointTwo.y), 2 ) );
}

// 随机区域
int CTableFrameSink::RandomArea( int nLen, ... )
{ 
	ASSERT( nLen > 0 );
	if ( nLen <= 0 )
		return 0;

	va_list argptr;
	va_start(argptr, nLen);
	Static CWHArray<int> Array;
	Array.RemoveAll();
	for ( byte i = 0; i < nLen; ++i )
	{
		int nTemp = va_arg( argptr, int );
		if( nTemp >= 0 && nTemp < 100000 )
		{
			Array.Add(nTemp);
		}
		else
		{
			//ASSERT(FALSE);
			break;
		}
	}
	va_end(argptr);


	int nIndex = 0;
	int nACTotal = 0;
	for (int i = 0; i < Array.GetCount(); i++)
		nACTotal += Array[i];

	ASSERT( nACTotal > 0  );
	if ( nACTotal > 0 )
	{
		int nRandNum = ( rand() + GetTickCount() + m_nAppearFishCount ) % nACTotal;
		for (int i = 0; i < Array.GetCount(); i++)
		{
			nRandNum -= Array[i];
			if (nRandNum < 0)
			{
				nIndex = i;
				break;
			}
		}
	}
	else
	{
		nIndex = rand()%nLen;
	}


	ASSERT( nIndex < nLen );
	return nIndex;
}


// 判断机器人
bool CTableFrameSink::IsAndroidUser(BYTE wChairID )
{
	//IServerUserItem * pIAndroidUserItem = m_pITableFrame->GetTableUserItem( wChairID );
	//return pIAndroidUserItem && pIAndroidUserItem->IsAndroidUser();

	/*GameUserInfo user;
	if (GetUserData(wChairID, user))
	{
		return user.isVirtual;
	}*/

	return IsVirtual(wChairID);
}

// 初始化玩家
void CTableFrameSink::InitializePlayer( WORD wChairID )
{
	// 初始化信息
	m_dwPlayID[wChairID] = 0;
	m_lPlayScore[wChairID] = 0;
	m_lPlayStartScore[wChairID] = 0;
	m_lplayCutMoney[wChairID] = 0;
	m_lBulletConsume[wChairID] = 0;
	m_lFireCount[wChairID] = 0;
	m_nPlayDownTime[wChairID] = 0;
	m_nRobotPlayTime[wChairID] = 0;
	m_nPlayDelay[wChairID] = 0;
	ZeroMemory(m_lPlayFishCount[wChairID], sizeof(m_lPlayFishCount[wChairID]));
	m_nMultipleIndex[wChairID] = 0;
	m_ArrayBulletKey[wChairID].clear();

	m_nEnergyValue[wChairID] = 0;
	m_nSupplyValue[wChairID] = RAND_EQUAL_MIN_MAX(m_nSupplyCondition[0], m_nSupplyCondition[1]);
	m_bPlaySupply[wChairID] = false;
	m_nLaserPlayTime[wChairID] = 0;
	m_nSpeedPlayTime[wChairID] = 0;

	// 删除特定鱼
	int nFishIndex = 0;
	while ( nFishIndex < m_ArrayFishInfo.GetCount() )
	{
		// 获取鱼信息
		tagFishInfo & TFishInfo = m_ArrayFishInfo.GetAt(nFishIndex);

		// 对比位置
		if ( TFishInfo.wHitChair == wChairID )
		{
			// 删除鱼
			m_ArrayFishInfo.RemoveAt( nFishIndex );

			continue;
		}

		// 下一个
		nFishIndex++;
	}
}

// 玩家消耗
void CTableFrameSink::PlayerConsume( WORD wChairID, int nMultipleIndex, LONGLONG lConsumeCount )
{
	// 机器人不参与
	if( IsAndroidUser(wChairID) )
	{
		return ;
	}

	// 无效倍数
	if( nMultipleIndex == Multiple_Max )
	{
		return ;
	}

	// 无分累计
	if( lConsumeCount <= 0 )
	{
		return ;
	}

	// 库存记录
	m_lStockCurrent += lConsumeCount;

	// 转换消耗
	double dConsumption = static_cast<double>(lConsumeCount);

	// 抽水抽取
	double dExtractTax = dConsumption * m_dTaxRatio;
	dConsumption -= dExtractTax;
	m_dWholeTax[nMultipleIndex] += dExtractTax;

	// 房间累计
	double dExtractDart = dConsumption * ( (double)m_nExplosionProportion / 1000.0 );
	dConsumption -= dExtractDart;
	m_dDartStock += dExtractDart;

	// 房间累计
	//m_dRoomStock[nMultipleIndex] += dConsumption;

	// 桌子累计
	double & dTableStock  = _TableStock( nMultipleIndex );
	dTableStock += dConsumption;

	// 个人消耗
	double & dPlayStock = _PlayerStock( wChairID, nMultipleIndex );
	dPlayStock -= static_cast<double>(lConsumeCount);

	// 发送消息
	GameUserInfo * pIServerUserItem = GetTableUserItem(wChairID);
	// 判断权限
	if( pIServerUserItem != NULL &&((pIServerUserItem->userStatus&USER_IDENTITY_TYPE_SUPER) == USER_IDENTITY_TYPE_SUPER))
	{
		// 定义消息
		CMD_S_System CMDSSystem;
		CMDSSystem.dRoomStock = m_dRoomStock[nMultipleIndex];
		CMDSSystem.dTableStock = dTableStock;
		CMDSSystem.dPlayStock = dPlayStock;
		CMDSSystem.dDartStock = m_dDartStock;
		CMDSSystem.dGameDifficulty = DifficultyGame( wChairID, nMultipleIndex );

		// 发送消息
		SendUserItemData( pIServerUserItem, SUB_S_SYSTEM, &CMDSSystem, sizeof(CMD_S_System) );
	}

	return ;
}

// 玩家收入
void CTableFrameSink::PlayerIncome( WORD wChairID, int nMultipleIndex, LONGLONG lIncomeCount )
{
	// 机器人不参与
	if( IsAndroidUser(wChairID) )
	{
		return ;
	}

	// 无效倍数
	if( nMultipleIndex == Multiple_Max )
	{
		return ;
	}

	// 无分累计
	if( lIncomeCount <= 0 )
	{
		return ;
	}

	// 库存记录
	m_lStockCurrent -= lIncomeCount;

	// 转换消耗
	double dIncomeCount = static_cast<double>(lIncomeCount);

	// 房间消耗
	//m_dRoomStock[nMultipleIndex] -= dIncomeCount;

	// 桌子消耗
	double & dTableStock = _TableStock( nMultipleIndex );
	dTableStock -= dIncomeCount;

	// 个人累计
	double & dPlayStock = _PlayerStock( wChairID, nMultipleIndex );
	dPlayStock += dIncomeCount;

	return ;
}

// 桌子库存
double & CTableFrameSink::_TableStock( int nMultipleIndex )
{
	return m_pdTableStock[nMultipleIndex][/*m_pITableFrame->GetTableID()*/1];
}

// 桌子库存
double & CTableFrameSink::_TableStock( unsigned short TableID, int nMultipleIndex )
{
	return m_pdTableStock[nMultipleIndex][TableID];
}

// 玩家库存
double & CTableFrameSink::_PlayerStock( WORD wChairID, int nMultipleIndex )
{
	return m_dPlayStock[wChairID][nMultipleIndex];
}


//获取玩家信息
GameUserInfo * CTableFrameSink::GetTableUserItem(WORD wChairID)
{
	if (!m_pDataManage)
	{
		return NULL;
	}

	long userID = GetUserIDByDeskStation(wChairID);
	if (userID <= 0)
	{
		return NULL;
	}

	return m_pDataManage->GetUser(userID);
}


//给玩家发送数据
bool CTableFrameSink::SendUserItemData(GameUserInfo * pIServerUserItem, WORD wSubCmdID, VOID * pData, WORD wDataSize)
{
	if (!pIServerUserItem)
	{
		INFO_LOG("pIServerUserItem=NULL");
		return true;
	}

	return SendGameData(pIServerUserItem->deskStation, pData, wDataSize, MSG_MAIN_LOADER_GAME, wSubCmdID, 0);
}
////////////////////////////////////////////////////////////////////////////////// 

bool CTableFrameSink::ReSetGameState(BYTE bLastStation)
{
	return true;
}

bool CTableFrameSink::GameBegin(BYTE bBeginFlag)
{
	SetTimer(TIME_WRITE_SCORE, 30000);
	IniConfig();
	LoadDynamicConfig();
	return CGameDesk::GameBegin(bBeginFlag);
}

//用户断线
bool CTableFrameSink::UserNetCut(GameUserInfo* pUser)
{
	//UserLeftDesk(pUser);
	//调用踢出该玩家
	KickOutUser(pUser->deskStation, REASON_KICKOUT_STAND_MINLIMIT);

	return true;
}

bool CTableFrameSink::IsPlayGame(BYTE bDeskStation)
{

	return false;
}


void CTableFrameSink::UserBeKicked(BYTE deskStation)
{
	// 检测是否有人
	for (WORD i = 0; i < PlayChair_Max; ++i)
	{
		if (deskStation == i) continue;
		long  userid = GetUserIDByDeskStation(i);
		if (userid <= 0) continue;

		//有人那么就结算一下
		// 判断用户
		long PlayeruserID = GetUserIDByDeskStation(deskStation);
		if (PlayeruserID > 0)
		{
			// 玩家指针
			//IServerUserItem * pIServerDismiss = m_pITableFrame->GetTableUserItem(nSite);
			GameUserInfo UserInfo;
			GetUserData(deskStation, UserInfo);

			// 写分
			LONGLONG lScore = (m_lPlayScore[deskStation] - m_lPlayStartScore[deskStation]);
			if (lScore != 0)
			{
				// 积分为负， 不能超过最大分数
				if (lScore < 0 && (-lScore) > UserInfo.money)
				{
					lScore = -UserInfo.money;
				}
			}

			// 游戏时间
			DWORD dwEndTime = (DWORD)time(NULL) - m_nPlayDownTime[deskStation];

			ChangeUserPoint(deskStation, lScore, m_lplayCutMoney[i]);
			m_lplayCutMoney[i] = 0;

			// 保存写分
			m_nRoomWrite += (-lScore);
			// 清空玩家信息
			InitializePlayer(deskStation);

		}

		return;
	}
	//只有他一个人 那么游戏就直接结束了

	// 删除定时器
	KillTimer(IDI_FISH_CREATE);
	KillTimer(IDI_DELAY);
	KillTimer(IDI_SECOND);
	//删除个人难度
	long userID = GetUserIDByDeskStation(deskStation);
	DeletePersonalDifficulty(userID);

	// 全局爆炸还原
	if (GetFishCount(FishType_BaoZhaFeiBiao) > 0)
	{
		// 最大倍数
		int nMultipleMax = 0;
		for (int nMultipleIndex = 0; nMultipleIndex < Multiple_Max; ++nMultipleIndex)
		{
			nMultipleMax = Max_(nMultipleMax, m_nMultipleValue[nMultipleIndex]);
		}

		// 飞镖消耗
		m_dDartStock += nMultipleMax * m_nExplosionStart;
	}

	// 鱼信息
	m_ArrayFishInfo.RemoveAll();
	m_cbBackIndex = 0;
	m_nAppearFishCount = 0;
	m_bFishSwimming = FALSE;
	m_bBackExchange = FALSE;
	m_nStartTime = 0;

	// 阵列
	m_nFishTeamTime = 0;
	m_nFishKingTime = 30;
	ZeroMemory(m_nFishKing, sizeof(m_nFishKing));

	if (IsBegin)
	{
		// 结束游戏
		GameFinish(deskStation, GF_NORMAL);
	}
}

// 技能
bool CTableFrameSink::OnSubSkill(BYTE deskStation, VOID * pData, WORD wDataSize)
{
	if (sizeof(CMD_C_Skill) != wDataSize) return true;

	CMD_C_Skill *Skill = (CMD_C_Skill*)pData;
	if (!Skill)
	{
		ERROR_LOG("错误:玩家%d，技能错误", GetUserIDByDeskStation(deskStation));
		return false;
	}


	if (Skill->SkillType == 2)
	{
		int skillCount = GetUserBagDataByKey(deskStation, BAG_BING_DONG);
		if (skillCount > 0)
		{
			ChangeUserBage(deskStation, BAG_BING_DONG, -1);
			FishFreeze(5 * 1000);
		}
		else
		{
			SendGameMessage(deskStation, "您的冰冻技能道具不足！");
			return true;
		}
	}

	if (Skill->SkillType == 3)
	{
		int skillCount = GetUserBagDataByKey(deskStation, BAG_LOCK);
		if (skillCount > 0)
		{
			ChangeUserBage(deskStation, BAG_LOCK, -1);
		}
		else
		{
			SendGameMessage(deskStation, "您的锁定技能道具不足！");
			return true;
		}
	}

	SendTableData(INVALID_CHAIR, SUB_C_SKILL, pData, wDataSize);
	
	return true;
}


bool CTableFrameSink::HundredGameIsInfoChange(BYTE deskStation)
{
	return false;
}

bool CTableFrameSink::OnChangeCannon(BYTE deskStation, VOID * pData, WORD wDataSize)
{
	if (sizeof(CMD_C_CANNON) != wDataSize) return false;

	CMD_C_CANNON *cannon = (CMD_C_CANNON*)pData;

	long userID = GetUserIDByDeskStation(deskStation);
	if (userID <= 0) return false;

	SendTableData(INVALID_CHAIR, SUB_C_CHANGE_CONNON, pData, wDataSize);
	return true;
}