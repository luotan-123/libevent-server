#ifndef TABLE_FRAME_SINK_HEAD_FILE
#define TABLE_FRAME_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "AfxTempl.h"
#include "GameLogic.h"
#include "PolygonShape.h"
#include <map>
using namespace std;

////////////////////////////////////////////////////////////////////////////////// 

// 捕鱼路径
struct tagFishPath
{
	CWHArray< tagBezierPoint * >		ArrayBezierPoint;
};

// 开炮信息
struct tagFireInfo
{
	int							nMultipleIndex;		// 倍数索引
	double						dBulletInvest;		// 子弹收入
};

// 捕获信息
struct tagCatchInfo
{
	UINT						nFishKey;
	int							nMultipleCount;
	LONGLONG					lScoreCount;

	tagCatchInfo()
	{
		nFishKey = 0;
		nMultipleCount = 0;
		lScoreCount = 0;
	}
	tagCatchInfo ( tagCatchInfo & TRight )
	{
		nFishKey = TRight.nFishKey;
		nMultipleCount = TRight.nMultipleCount;
		lScoreCount = TRight.lScoreCount;
	}
	void operator= ( const tagCatchInfo & TRight )
	{
		nFishKey = TRight.nFishKey;
		nMultipleCount = TRight.nMultipleCount;
		lScoreCount = TRight.lScoreCount;
	}
};

// 停留信息
struct tagStayInfo
{
	UINT						nStayStart;			// 停留开始
	UINT						nStayTime;			// 停留时间
};
typedef CWHArray< tagStayInfo >	CArrayStay;

// 鱼鱼结构
struct tagFishInfo
{
	// 基本信息
	UINT				nFishKey;				// 鱼索引
	BYTE				nFishType;				// 鱼种类
	int					nFishScore;				// 鱼分数
	UINT				wHitChair;				// 击杀位置

	// 时间信息
	UINT				unCreateTime;			// 创建时间
	UINT				unOverTime;				// 结束时间

	// 击杀信息
	int					nScoreChance;			// 分数几率
	double				dCurChance;				// 当前几率

	// 鱼状态
	EnumFishState		nFishState;				// 鱼状态

	// 重复创建
	bool				bRepeatCreate;			// 重复创建

	// 路径偏移
	float				fRotateAngle;			// 角度
	CShortPoint 		PointOffSet;			// 偏移

	// 路径信息
	int					nBezierCount;			// 路径数量
	tagBezierPoint		TBezierPoint[BEZIER_POINT_MAX];		// 路径信息

	// 初始化角度
	float				fInitialAngle;			// 角度

	// 停留数据
	CArrayStay			ArrayStayInfo;			// 停留开始

	tagFishInfo()
	{
		// 基本信息
		nFishKey = 0;
		nFishType = 0;			
		nFishScore = 0;	
		wHitChair = 0;	

		// 时间信息
		unCreateTime = 0;
		unOverTime = 0;

		// 击杀信息
		nScoreChance = 0;
		dCurChance = 0.0;

		// 鱼类型
		nFishState = FishState_Normal;

		// 重复创建
		bRepeatCreate = false;

		// 路径偏移
		fRotateAngle = 0.f;
		PointOffSet.SetPoint(0, 0);

		// 路径信息
		nBezierCount = 0;
		ZeroMemory(TBezierPoint, sizeof(TBezierPoint));
	}

	// 赋值函数
	void operator= ( const tagFishInfo & TRight )
	{
		// 基本信息
		nFishKey = TRight.nFishKey;
		nFishType = TRight.nFishType;
		nFishScore = TRight.nFishScore;	
		wHitChair = TRight.wHitChair;

		// 时间信息
		unCreateTime = TRight.unCreateTime;
		unOverTime = TRight.unOverTime;

		// 击杀信息
		nScoreChance = TRight.nScoreChance;
		dCurChance = TRight.dCurChance;

		// 鱼类型
		nFishState = TRight.nFishState;

		// 重复创建
		bRepeatCreate = TRight.bRepeatCreate;

		// 路径偏移
		fRotateAngle = TRight.fRotateAngle;
		PointOffSet = TRight.PointOffSet;

		// 路径信息
		nBezierCount = TRight.nBezierCount;
		CopyMemory(&TBezierPoint, TRight.TBezierPoint, sizeof(TBezierPoint));

		// 停留数据
		ArrayStayInfo.Append( TRight.ArrayStayInfo );
	}
};

// 鱼移动类型
enum EnumFishMoveType
{
	FishMoveType_Next,					// 下一个
	FishMoveType_Delete,				// 异常鱼
	FishMoveType_Normal,				// 正常鱼
};

// 捕获类型
enum EnumPlayCatchType
{
	EPCT_Bullet,				// 子弹
	EPCT_Laser,					// 激光
	EPCT_Killer,				// 杀手鱼
	EPCT_Explosion,				// 局部爆炸
	EPCT_FullScreen,			// 全屏爆炸
};

// 杀手类型
enum EnumKillerType
{
	KillerType_No,			// 无杀手
	KillerType_One,			// 一个
	KillerType_All,			// 全部
};

// 游戏桌子
class CTableFrameSink : public CGameDesk
{
	// 函数定义
public:
	// 构造函数
	CTableFrameSink();
	// 析构函数
	virtual ~CTableFrameSink();

	// 版权控制
protected:
	bool							m_bLegalOne;
	bool							m_bLagalTwo;
	//ICompilationSink *				m_pICompilationSink;			

	// 游戏变量
protected:
	bool							m_bCompetitionRoom;						// 比赛房间
	bool							m_bTimedCompetitionRoom;				// 定时赛房间
	bool							m_bFirstTime;							// 第一次
	bool							m_bFishSwimming;						// 鱼游动
	UINT							m_nAppearFishCount;						// 出现鱼数
	 int						    m_nFishMultiple[FishType_Max][2];		// 鱼的倍数

	// 创建信息
protected:
	static int						m_nCreateCount;							// 创建数量

	// 场景信息
protected:
	byte							m_cbBackIndex;							// 背景索引
	bool							m_bBackExchange;						// 背景交换
	static int						m_nSceneTime;							// 场景维持时间
	uint							m_nSceneBegin;							// 场景开始时间

	// 房间倍数
protected:
	 int						m_nMultipleValue[Multiple_Max];

	// 子弹
protected:
	static int						m_nBulletVelocity;						// 子弹速度
	static int						m_nBulletCoolingTime;					// 子弹冷却

	// 玩家信息
protected:
	DWORD 							m_dwPlayID[PlayChair_Max];								// 玩家ID
	LONGLONG						m_lPlayScore[PlayChair_Max];							// 玩家积分
	LONGLONG						m_lPlayStartScore[PlayChair_Max];						// 玩家积分
	LONGLONG						m_lplayCutMoney[PlayChair_Max];							// 抽水金额
	LONGLONG						m_lFireCount[PlayChair_Max];							// 开炮数量
	LONGLONG						m_lBulletConsume[PlayChair_Max];						// 子弹消耗
	DWORD							m_nPlayDownTime[PlayChair_Max];							// 玩家坐下时间
	DWORD							m_nRobotPlayTime[PlayChair_Max];						// 机器人游戏时间
	DWORD							m_nPlayDelay[PlayChair_Max];							// 玩家延迟
	long							m_lPlayFishCount[PlayChair_Max][FishType_Max];			// 玩家捕获鱼数
	int								m_nMultipleIndex[PlayChair_Max];						// 玩家倍数
	map< int, tagFireInfo >			m_ArrayBulletKey[PlayChair_Max];						// 子弹关键值

	// 玩家信息
protected:
	CPoint							m_PointPlay[PlayChair_Max];							// 玩家位置

	// 玩家补给
protected:
	static int						m_nSupplyCondition[2];					// 活动条件
	int								m_nSupplyValue[PlayChair_Max];			// 活动值
	int								m_nEnergyValue[PlayChair_Max];			// 能量值
	bool							m_bPlaySupply[PlayChair_Max];				// 激活补给箱
	int								m_nLaserPlayTime[PlayChair_Max];			// 激光时间
	int								m_nSpeedPlayTime[PlayChair_Max];			// 加速时间

	// 鱼信息
protected:
	CWHArray< tagFishInfo >			m_ArrayFishInfo;						// 鱼鱼信息
	DWORD							m_nStartTime;							// 同步时间
	DWORD							m_FishKingTime;							//鱼王刷新起始时间
	DWORD                           m_FishKingIntervalTime;					//鱼王的刷新间隔
	bool							m_bFushBoss;							//鱼王刷新公告阶段
	bool							m_bFushSence;							//刷新BOSS时间鱼阵不能刷新
	bool							m_bIsFushBoss;						    //BOSS刷新中
	bool							m_bIsFushBegin;							//刷新到BOSS消失期间

	// 阵列
protected:
	UINT							m_nFishTeamTime;							// 鱼队列数
	int								m_nFishKingTime;							// 渔王时间
	int								m_nFishKing[FishType_Small_Max];				// 鱼王信息

	// 消息控制
protected:
	int								m_nAwardMinMultiple;
	BOOL							m_bAwardChatBox;

	// 补给箱控制
protected:
	static int						m_nLaserTime;							// 激光时间
	static int						m_nLaserChance;							// 激光几率
	static int						m_nSpeedTime;							// 加速时间
	static int						m_nSpeedChance;							// 加速几率
	static int						m_nGiftScore[5];						// 赠送金币
	static int						m_nGiftChance[5];						// 赠送几率
	static int						m_nNullChance;							// 空箱几率

	// 爆炸控制
protected:
	static int						m_nExplosionProportion;					// 爆炸比例
	static int						m_nExplosionStart;						// 爆炸启动
	static LONGLONG					m_lExplosionCondition;					// 爆炸条件
	static EnumExplosionConditionType m_nExplosionConditionType;			// 条件类型
	static CMap<uint, uint, LONGLONG, LONGLONG> m_MapPlayExplosionCondition;// 爆炸条件

	// 库存
protected:
	static double					m_dTaxRatio;							// 税收比例
	static LONGLONG					m_nRoomWrite;							// 房间实际写分
	static double					m_dDartStock;						// 飞镖库存
	static double					m_dWholeTax[Multiple_Max];			// 全部税收
	static double					m_dRoomStock[Multiple_Max];			// 房间库存
	static double *					m_pdTableStock[Multiple_Max];		// 房间库存
	double							m_dPlayStock[PlayChair_Max][Multiple_Max];// 玩家库存
	
	// 难度
	static int						m_nDifficultyStart[Difficulty_Type];		// 难度类型
	static int						m_nRoomDifficultyCount[Difficulty_Max];
	static int						m_nTableDifficultyCount[Difficulty_Max];
	static int						m_nPlayDifficultyCount[Difficulty_Max];
	static double					m_dRoomDifficultyValue[Difficulty_Max];
	static double					m_dTableDifficultyValue[Difficulty_Max];
	static double					m_dPlayDifficultyValue[Difficulty_Max];

	// 个人难度控制
    CMapPersonalDifficulty	m_MapPersonalDifficulty;

	// 总库存
	static LONGLONG					m_lStockInitial;					// 初始库存
	static LONGLONG					m_lStockCurrent;					// 当前库存

	// 鱼路径
protected:
	static CWHArray< tagFishPath * >	m_ArrayFishPathPositive;			

	//其他配置
	int   m_iAddFinshCount;	   //增加人数增加的鱼数
	int   m_iFinshLeve;        //鱼群大小等级
	int   m_iFinshGroup;       //鱼阵类型
	bool  m_bOpenShoalFish;    //大鱼群是否开启
	int   m_iFishShoalLev;     //大鱼群数量等级
	int   m_iCreatJiLv;        //创建鱼的时候小鱼创建比例
	int   m_iTickTime;		  //玩家多久不操作会被踢出
	bool  m_IsOpenTick;	      //是否开启不操作踢出玩家
	int	  m_ipropNum;		  //大于多少索引的鱼可以爆出特殊道具
	int	  m_iBingDong;		  //冰冻技能概率
	int	  m_iLocking;		  //1元话费卡概率
	int	  m_iRate;			  //抽水率
	int   m_iRobotTickTime;   // 机器人多久不操作会被踢

	uint   m_iOptionTime[PLAY_COUNT];//玩家操作间隔时间
	// 组件变量
protected:
	CGameLogic						m_GameLogic;							// 游戏逻辑
	static tagCustomRule			m_CustomRule;							// 配置规则

	// 组件接口
protected:
	//ITableFrame	*					m_pITableFrame;							// 框架接口
	//tagGameServiceOption *			m_pGameServiceOption;					// 服务配置

	// 控制组件
public:
	HINSTANCE						m_hControlInst;
	//IServerControl *				m_pIServerControl;



	// 基础接口
public:
	// 释放对象
	virtual VOID Release();
	// 接口查询
	//virtual VOID * QueryInterface(REFGUID Guid, DWORD dwQueryVer);

	// 管理接口
public:
	// 配置桌子
	//virtual bool Initialization(IUnknownEx * pIUnknownEx);
	// 复位桌子
	virtual VOID RepositionSink();

	// 查询接口
public:
	// 查询是否扣服务费
	virtual bool QueryBuckleServiceCharge(WORD wChairID) { return false; }



	// 游戏事件
public:


	/// 初始游戏
	virtual bool InitDeskGameStation();
	//游戏开始
	//virtual bool OnStart();
	//游戏开始
	virtual bool GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//发送场景
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//	// 重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);
	//框架消息处理函数
	virtual bool HandleFrameMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);
	// 玩家坐下
	virtual bool UserSitDeskActionNotify(BYTE deskStation);

	// 事件接口
public:
	//定时器事件
	virtual bool OnTimer(UINT uTimerID);
	//游戏消息处理
	virtual bool HandleNotifyMessage(BYTE deskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);

	//用户断线
	virtual bool  UserNetCut(GameUserInfo* pUser);
	//用户起来
	virtual bool  UserLeftDesk(GameUserInfo* pUser);
	//游戏状态
	virtual bool IsPlayGame(BYTE bDeskStation);
	//用户被踢
	//virtual void  UserBeKicked(BYTE deskStation);
public:
	//加载配置
	virtual void LoadDynamicConfig();

	// 控制函数
public:
	// 返回配置
	virtual void GetCustomRule( tagCustomRule & nConfigInfo );
	// 设置配置
	virtual void SetCustomRule( tagCustomRule & nConfigInfo, bool bSaveFile );
	// 返回个人难度
	virtual void GetPersonalDifficulty( CMapPersonalDifficulty & MapPersonalDifficulty );
	// 设置个人难度
	virtual void SetPersonalDifficulty( unsigned long lPlayID, double dPersonalDifficulty );
	// 删除个人难度
	virtual void DeletePersonalDifficulty( unsigned long lPlayID );
	// 清空个人难度
	virtual void ClearPersonalDifficulty();
	//获取个人难度
	void GetUserDifficulty(unsigned long lPlayID, double& dPersonalDifficulty);
	// 获取库存
	
	// 设置桌子库存
	virtual bool SetTableStock( unsigned short TableID, int nTableStock );
	//被踢玩家处理
	virtual void UserBeKicked(BYTE deskStation);
	// 百人类游戏,判断这个玩家名字是否可变（包括机器人，以及上庄列表玩家）
	virtual bool HundredGameIsInfoChange(BYTE deskStation);

	// 游戏事件
protected:
	// 捕鱼事件
	bool OnSubCatchFish(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 开火
	bool OnSubFire(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 准备激光
	bool OnSubBeginLaser(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 激光
	bool OnSubLaser(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 延迟
	bool OnSubDelay(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 语音
	bool OnSubSpeech(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 倍数选择
	bool OnSubMultiple(BYTE deskStation, const void * pBuffer, WORD wDataSize);
	// 控制信息
	bool OnSubControl(BYTE deskStation, VOID * pData, WORD wDataSize );
	//技能释放
	bool OnSubSkill(BYTE deskStation, VOID * pData, WORD wDataSize);

	void GetStock(LONGLONG & lStockInitial, LONGLONG & lStockCurrent);
	//切换炮台
	bool OnChangeCannon(BYTE deskStation, VOID * pData, WORD wDataSize);

	// 功能函数
public:
	//给玩家发送数据
	bool SendTableData(WORD wChairID, WORD wSubCmdID, VOID * pData, WORD wDataSize, unsigned int handleCode = 0);
	//给玩家发送数据
	bool SendUserItemData(GameUserInfo * pIServerUserItem, WORD wSubCmdID, VOID * pData, WORD wDataSize);

	// 发送奖励提示
	bool SendAwardTip( WORD wChairID, byte nFishType, int nFishMultiple, LONGLONG lFishScore, EnumScoreType nScoreType );
	// 初始化鱼池
	void InitializationFishpond( uint nBeginTime = 0 );
	// 创建鱼
	uint CreateFish( byte cbCount, byte cbFishType = FishType_Max, uint nPathIndex = uint_max, uint unCreateTime = 0, uint unIntervalTime = 1000, float fRotateAngle = 0.f, bool bCanSpecial = true, bool bCanAquatic = true, EnumKillerType nKillerType = KillerType_No, WORD wHitChair = INVALID_CHAIR, CShortPoint & PointOffSet = CShortPoint(0,0), float fInitialAngle = 0.f, bool bRepeatCreate = true );
	// 创建鱼
	uint CreateFishEx( byte cbCount, byte cbFishType = FishType_Max, tagBezierPoint TBezierPoint[BEZIER_POINT_MAX] = NULL, int nBezierCount = 0, uint unCreateTime = 0, uint unIntervalTime = 1000, float fRotateAngle = 0.f, bool bCanSpecial = true, bool bCanAquatic = true, EnumKillerType nKillerType = KillerType_No, WORD wHitChair = INVALID_CHAIR, CShortPoint & PointOffSet = CShortPoint(0,0), float fInitialAngle = 0.f, bool bRepeatCreate = true );
	// 创建鱼
	uint ResetFish( tagFishInfo & TFishInfo );
	// 随机路径
	void RandomPath( byte cbFishType, tagBezierPoint TBezierPoint[BEZIER_POINT_MAX], int & nBezierCount );
	// 发送鱼信息
	void SendFish( tagFishInfo & TFishInfo );
	// 捕鱼
	LONGLONG PlayCatchFish( WORD wChairID, int nBulletSplit, uint nFishCatchKey, int nMultipleIndex, EnumPlayCatchType EPlayCatchType );
	// 比较鱼
	bool ISFishCatchKey( uint nFishKey, CWHArray< uint > & ArrayFishCatchKey );
	// 捕中鱼
	LONGLONG CatchInFish( WORD wChairID, int nMultipleIndex, EnumPlayCatchType nPlayCatchType, tagFishInfo & TFishInfo, CWHArray< tagCatchInfo > & ArrayCatchFishInfo );
	// 发送捕获消息
	void SendCatchFish( WORD wChairID, CWHArray< tagCatchInfo > & ArrayCatchFishInfo );
	// 游戏难度
	double DifficultyGame( WORD wChairID, int nMultipleIndex );
	// 游戏难度
	double DifficultyRoom( int nMultipleIndex );
	// 游戏难度
	double DifficultyTable( int nMultipleIndex );
	// 游戏难度
	double DifficultyPlay( WORD wChairID, int nMultipleIndex );
	// 内部函数
private:
	// 特殊鱼阵
	uint SpecialFishMatrix( byte cbFishType, CDoublePoint PointFish );
	// 鱼群建立
	uint GroupOfFish( uint nBeginTime = 0 );
	// 圆圈鱼
	uint CircleOfFish( byte cbFishType,	int nFishCount, int nPathIndex, uint unCreateTime, uint nRadius, uint unIntervalTime );
	// 获取路径时间
	uint PathTime( tagFishPath* pFishPath );
	// 计算当前鱼位置
	EnumFishMoveType FishMove( CDoublePoint & ptPosition, tagFishInfo & TFish, uint nCustomLossTime = uint_max );
	// 鱼快速消息
	void FishFastMove();
	// 获取鱼数量
	uint GetFishCount( uint nType );
	// 获取特殊鱼数量
	uint GetFishSpecialCount();
	// 鱼冰冻
	void FishFreeze( uint unLossTime );
	// 交换变量
	template< typename T > void SwapVariable( T & One, T & Two );
	// 判断机器人
	bool IsAndroidUser( BYTE wChairID );
	// 初始化玩家
	void InitializePlayer( WORD wChairID );

public:
	// 旋转点
	CDoublePoint Rotate( CDoublePoint & ptCircle, double dRadian, CDoublePoint & ptSome );
	// 旋转点
	CShortPoint Rotate( CShortPoint & ptCircle, double dRadian, CShortPoint & ptSome );
	// 已知圆心,弧度,半径 求圆上任意一
	CShortPoint RotatePoint( CShortPoint & ptCircle, double dRadian, double dRadius );
	// 贝塞尔曲线
	CDoublePoint PointOnCubicBezier( tagBezierPoint* cp, double t );
	// 两点距离
	double DistanceOfPoint( CDoublePoint & PointOne, CDoublePoint & PointTwo );
	// 随机区域
	int RandomArea(int nLen, ...);

public:
	// 使能关机特权 并关机
	bool EnableShutdownPrivilege();

	// 库存函数
public:
	// 玩家消耗
	void PlayerConsume( WORD wChairID, int nMultipleIndex, LONGLONG lConsumeCount ); 
	// 玩家收入
	void PlayerIncome( WORD wChairID, int nMultipleIndex, LONGLONG lIncomeCount );
	// 桌子库存
	double & _TableStock( int nMultipleIndex );	
	// 桌子库存
	double & _TableStock( unsigned short TableID, int nMultipleIndex );	
	// 玩家库存
	double & _PlayerStock( WORD wChairID, int nMultipleIndex );

	//新增
	//获取玩家信息
	GameUserInfo * GetTableUserItem(WORD wChairID);
	//打印日志文件-服务器
	void DebugPrintf(const char *p, ...);
	//初始化游戏配置
	void IniConfig();

	//新增变量
	bool IsBegin;              //游戏是否开始了
};

////////////////////////////////////////////////////////////////////////////////// 

#endif