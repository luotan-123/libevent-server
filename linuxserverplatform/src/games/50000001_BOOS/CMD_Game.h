#ifndef CMD_GAME_HEAD_FILE
#define CMD_GAME_HEAD_FILE

#include "..\Dexter\Dexter.h"
// 1字节对其
#pragma pack(1)

////////////////////////////////////////////////////////////////////////// 
// 服务定义

#define GAMENAME						TEXT("BOSS来了")
#define GAME_DLL_AUTHOR                 TEXT("深圳火妹网络科技")            //
#define GAME_DLL_NOTE                   TEXT("电玩 -- 游戏组件")

#define NAME_ID                         50000001
#define PLAY_COUNT                      4

// 版本定义
#define GAME_MAX_VER					1								        // 现在最高版本
#define GAME_LESS_VER					1								        // 现在最低版本
#define GAME_CHANGE_VER					0								        // 修改版本

// 支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME

// 游戏常量定义
#define INVALID_CHAIR				0xFFFF								//无效椅子

////////////////////////////////////////////////////////////////////////// 

//
#define  GAME_STATUS_FREE     0     //空闲状态
#define  GAME_STATUS_PLAY     1    //游戏状态

// 游戏消息
#define WM_D3D_RENDER				(WM_USER + 300)					// 渲染消息
#define WM_D3D_LOAD					(WM_USER + 301)					// 载入消息
#define WM_SHOW_CHAT				(WM_USER + 302)					// 聊天消息
#define WM_BEGIN_HELP				(WM_USER + 303)					// 开始帮助消息
#define IDM_CATCH_FISH				(WM_USER + 1000)				// 捕鱼信息
#define IDM_FIRE					(WM_USER + 1001)				// 开火信息
#define IDM_UPDATE_CONTROLS			(WM_USER + 1004)				// 更新控件信息
#define IDM_SET						(WM_USER + 1005)				// 设置消息
#define IDM_FIRE_SWIM_OVER			(WM_USER + 1007)				// 游动完结
#define IDM_TEMP_SET				(WM_USER + 1008)				// 临时设置消息
#define IDM_COLOR					(WM_USER + 1009)				// 颜色消息
#define IDM_BEGIN_LASER				(WM_USER + 1013)				// 准备激光
#define IDM_LASER					(WM_USER + 1014)				// 激光
#define IDM_GOLD_SELETE				(WM_USER + 1018)				// 金龟选择
#define IDM_SPEECH					(WM_USER + 1020)				// 语音消息
#define IDM_MULTIPLE				(WM_USER + 1021)				// 倍数消息
#define IDM_OPEN_CONTROL			(WM_USER + 1023)				// 打开控制

#define S_TOP_LEFT					0								// 服务器位置
#define S_TOP_CENTER				1								// 服务器位置
#define S_TOP_RIGHT					2								// 服务器位置
#define S_BOTTOM_LEFT				3								// 服务器位置
#define S_BOTTOM_CENTER				4								// 服务器位置
#define S_BOTTOM_RIGHT				5								// 服务器位置

#define C_TOP_LEFT					0								// 视图位置
#define C_TOP_CENTER				1								// 视图位置
#define C_TOP_RIGHT					2								// 视图位置
#define C_BOTTOM_LEFT				3								// 视图位置
#define C_BOTTOM_CENTER				4								// 视图位置
#define C_BOTTOM_RIGHT				5								// 视图位置

// 相对窗口
#define DEFAULE_WIDTH				(1280)							// 客户端相对宽
#define DEFAULE_HEIGHT				(800)							// 客户端相对高	
#define OBLIGATE_LENGTH				(300)							// 预留宽度

#define CAPTION_TOP_SIZE			25								// 标题大小
#define CAPTION_BOTTOM_SIZE			40								// 标题大小

// 音量
#define  MAX_VOLUME					(3000)

////////////////////////////////////////////////////////////////////////// 
// 时间变换
#define  SECOND_TO_MILLISECOND(A)	(A*1000)
#define  MINUTE_TO_MILLISECOND(A)	(A*1000*60)

// 区间随机数
#define	RAND_INDEX					2
#define RAND_INITIAL( nValue, nMin, nMax )		{ nValue[0] = nMin; nValue[1] = nMax; }
#define RAND_MIN_MAX( nValue, nMin, nMax )		{ if( (nMin) >= (nMax) ) { nValue = nMin;} else { nValue = (rand()%((nMax) - (nMin))) + (nMin); } }
#define RAND_EQUAL_MIN_MAX( nMin, nMax )		( ( (rand() + GetTickCount()) % (nMax - nMin) ) + nMin )
#define RAND_TRUE_FALSE( True, False )			( ( (rand() + GetTickCount()) % ((True) + (False)) ) < (uint)(True) )

// 最大路径
#define BEZIER_POINT_MAX			8

//定义道具的key
#define BAG_HONG_BAO		"redBag"			// 红包
#define BAG_PHONE_CARD1		"phoneBillCard1"	// 1元话费卡
#define BAG_PHONE_CARD5		"phoneBillCard5"	// 5元话费卡
#define BAG_BING_DONG		"skillFrozen"		// 冰冻技能
#define BAG_LOCK			"skillLocking"		// 锁定技能

// 游戏玩家
enum
{
	PlayChair_Max				= 4,
	PlayChair_Invalid			= 0xffff,
	PlayName_Len				= 32,
	QianPao_Bullet				= 1,							// 千炮消耗
	Multiple_Max				= 10,							// 最大倍数
};

// 鱼类型小鱼0-30
enum EnumFishTypeA
{
	FishType_BXiaoHuangYu		= 0,							// 小黄鱼
	FishType_BXiaoQingYu	    = 1,							// 小青鱼
	FishType_BCaiBanYu			= 2,							// 彩斑鱼							
	FishType_BHaiMa			    = 3,							// 海马
	FishType_BBXiaoChouYu	    = 4,							// 小丑鱼
	FishType_BHongWeiYu		    = 5,							// 红尾鱼
	FishType_BHaiLuo			= 6,							// 海螺
	FishType_BHaiLuoXie			= 7,							// 海螺蟹
	FishType_BShanHuYu		    = 8,							// 珊瑚鱼
	FishType_BHongJinYu			= 9,							// 红金鱼
	FishType_BLanWeiYu			= 10,							// 蓝尾鱼
	FishType_BDengLongYu		= 11,							// 灯笼鱼

	FishType_BQiPaoYu			= 12,							// 气泡鱼
	FishType_BLvQiPaoYu		    = 13,							// 绿色气泡鱼
	FishType_BKuiJiaYu			= 14,							// 盔甲鱼
	FishType_BNianYu			= 15,							// 鲶鱼
	FishType_BWuGui				= 16,							// 乌龟
	FishType_BMoGuiYu			= 17,							// 魔鬼鱼

	FishType_BJianYu			= 18,							// 剑鱼
	FishType_BHaiTun		    = 19,							// 海豚
	FishType_BDianYu			= 20,							// 电鱼
	FishType_BJingYu			= 21,							// 鲸鱼
	FishType_BShaYu				= 22,							// 鲨鱼

	FishType_BChuiTouSha		= 23,							// 锤头鲨
	FishType_BJinJuChiSha		= 24,							// 黄金甲巨尺鲨
	FishType_BJinKuiJiaYu		= 25,							// 黄金盔甲鱼
	FishType_BJinChuiTouSha	    = 26,							// 黄金锤头鲨
	FishType_BJinShaYu		    = 27,							// 黄金鲸鱼
	FishType_BJinHuSha			= 28,							// 黄金虎鲸

	FishType_BOSS			    = 29,							//BOSS

	FishType_Hong_Bao		    = 30,							//	红包 key值:redBag
	FishType_Hua_Fei1			= 31,							//  话费1元 key值:phoneBillCard1
	FishType_Hua_Fei2			= 32,							//  话费5元 key值:phoneBillCard5


	FishType_ShuiHuZhuan		= 35,							// 水浒传
	FishType_ZhongYiTang		= 36,							// 忠义堂
	FishType_BaoZhaFeiBiao		= 37,							// 爆炸飞镖
	FishType_BaoXiang			= 38,							// 宝箱
	FishType_General_Max		= 33,							// 普通鱼最大
	FishType_Normal_Max			= 33,							// 正常鱼最大
	FishType_Max				= 33,							// 最大数量
	FishType_Small_Max			= 11,							// 小鱼最大索引
	FishType_Moderate_Max		= 17,							// 中鱼索
	FishType_Moderate_Big_Max	= 22,							// 中大鱼索
	FishType_Big_Max			= 29,							// 大鱼索引
	FishType_Invalid			= -1,							// 无效鱼
};
//中鱼
enum EnumFishTypeS
{
	FishTypeS_One			    = 30,							//中鱼

};
//大鱼
enum EnumFishTypeSS
{
	FishTypeSS_One			   = 50,							//大鱼
};
//鱼王
enum EnumFishTypeSSS
{
	FishTypeSSS_One			  = 90,							   //鱼王
};
// 鱼状态
enum EnumFishState
{
	FishState_Normal,		// 普通鱼
	FishState_King,			// 鱼王
	FishState_Killer,		// 杀手鱼
	FishState_Aquatic,		// 水草鱼
};

// 帮助
enum EnumHelpType
{
	HelpType_Operation,		
	HelpType_Fish,		
	HelpType_Box,		
	HelpType_Max,	
};

////////////////////////////////////////////////////////////////////////// 

// 子弹定义
enum
{
	BulletTwo	= 0,			// 双炮子弹
	BulletThree = 1,			// 三炮子弹
	BulletFour	= 2,			// 四炮子弹
	BulletStyle = 3,			// 子弹索引
};

// 获取分数类型
enum EnumScoreType
{
	EST_Cold,					// 金币
	EST_Laser,					// 激光
	EST_Speed,					// 加速
	EST_Gift,					// 赠送
	EST_Null,					// 空
};

// 记录信息
enum EnumRecord
{
	Record_UserID		= 1,		
	Record_ServerID		= 2,			
	Record_MatchID		= 3,			
	Record_MatchNo		= 4,	
	Record_FireCount	= 5,
	Record_UseScore		= 6,	
};


// 顶点类
struct CDoublePoint
{
	double				x;
	double				y;

	CDoublePoint() {  x = 0.0; y = 0.0; }
	CDoublePoint( POINT& Par ) {  x = Par.x; y = Par.y; }
	CDoublePoint( CDoublePoint& Par ) {  x = Par.x; y = Par.y; }
	CDoublePoint( double ParX, double ParY ) { x = ParX; y = ParY; }
	CDoublePoint& operator= (const CDoublePoint& point) { this->x = point.x; this->y = point.y; return *this; }
	void operator+= (CDoublePoint& point) { x += point.x; y += point.y; }
	bool operator!= (CDoublePoint& point) { return (x != point.x || y != point.y); }
	inline void SetPoint( double ParX, double ParY ) { x = ParX; y = ParY; }
};

struct CShortPoint
{
	short				x;
	short				y;

	CShortPoint() {  x = 0; y = 0; }
	CShortPoint( POINT& Par ) {  x = (short)Par.x; y = (short)Par.y; }
	CShortPoint( CShortPoint& Par ) {  x = Par.x; y = Par.y; }
	CShortPoint( short ParX, short ParY ) { x = ParX; y = ParY; }
	void operator+= (CShortPoint& point) { x += point.x; y += point.y; }
	inline void SetPoint( short ParX, short ParY ) { x = ParX; y = ParY; }

};

struct tagBezierPoint
{
	CDoublePoint			BeginPoint;
	CDoublePoint			EndPoint;
	CDoublePoint			KeyOne;
	CDoublePoint			KeyTwo;
	uint					Time;
};

// 载入需要信息
struct tagLoad
{
	void *							pGLDevice;
	void *							pCWnd;
	void *							pHdc;
	void *							pHrc;
	int								nIndex;
};

// RGB 颜色
struct COLORRGBM
{
	byte							R;									// 颜色色相
	byte							G;									// 颜色饱和
	byte							B;									// 颜色亮度
};

// HSB 颜色
struct COLORHSBM
{
	WORD							H;									// 颜色色相
	double							S;									// 颜色饱和
	double							B;									// 颜色亮度
};

// 游戏场景
struct GameScene
{
	byte				cbBackIndex;							// 背景索引
	LONGLONG			lPlayScore;								// 玩家积分
	LONGLONG			lPlayCurScore[PlayChair_Max];				// 玩家积分
	LONGLONG			lPlayStartScore[PlayChair_Max];			// 玩家积分

	int					nBulletVelocity;						// 子弹速度
	int					nBulletCoolingTime;						// 子弹冷却
	int					nFishMultiple[FishType_Max][2];		// 鱼倍数

	LONGLONG			lBulletConsume[PlayChair_Max];			// 子弹消耗
	long				lPlayFishCount[PlayChair_Max][FishType_Max];	// 玩家捕获鱼数
	int					nMultipleValue[Multiple_Max];		// 房间倍数
	int					nMultipleIndex[PlayChair_Max];			// 当前倍数
	bool				bUnlimitedRebound;						// 无限反弹
//	tchar				szBrowseUrl[256];						// 充值地址
};

////////////////////////////////////////////////////////////////////////// 
// 服务器命令结构
enum
{
	SUB_S_SYNCHRONOUS = 101,				// 同步信息
	SUB_S_FISH_CREATE,						// 鱼创建
	SUB_S_FISH_CATCH,						// 捕获鱼
	SUB_S_FIRE,								// 开火
	SUB_S_EXCHANGE_SCENE,					// 转换场景
	SUB_S_OVER,								// 结算
	SUB_S_DELAY_BEGIN,						// 延迟
	SUB_S_DELAY,							// 延迟
	SUB_S_BEGIN_LASER,						// 准备激光
	SUB_S_LASER,							// 激光
	SUB_S_BANK_TAKE,						// 银行取款
	SUB_S_SPEECH,							// 语音消息
	SUB_S_SYSTEM,							// 系统消息
	SUB_S_MULTIPLE,							// 倍数消息
	SUB_S_SUPPLY_TIP,						// 补给提示
	SUB_S_SUPPLY,							// 补给消息
	SUB_S_AWARD_TIP,						// 分数提示
	SUB_S_CONTROL,							// 控制消息
	SUB_S_UPDATE_GAME,						// 更新游戏
	SUB_S_STAY_FISH,						// 停留鱼
	SUB_S_GAME_SENCE,                       //发送场景
	SUB_S_GAME_BOSS,						//BOSS刷新
};

#define  SUB_S_CREATE_FISHS           666    //创建鱼鱼

// 系统消息
struct CMD_S_System
{
	double					dRoomStock;			// 房间库存
	double					dTableStock;		// 房间库存
	double					dPlayStock;			// 玩家库存
	double					dDartStock;			// 空闲库存
	double					dGameDifficulty;	// 房间难度
};

// 鱼创建
struct CMD_S_FishCreate
{
	// 基本信息
	uint				nFishKey;				// 鱼关键值
	uint				unCreateTime;			// 创建时间
	WORD				wHitChair;				// 击杀位置
	byte				nFishType;				// 鱼种类
	EnumFishState		nFishState;				// 鱼状态

	// 路径偏移
	float				fRotateAngle;			// 角度
	CShortPoint 		PointOffSet;			// 偏移

	// 初始角度
	float				fInitialAngle;			// 角度

	// 随机路径
	int					nBezierCount;			// 路径数量
	tagBezierPoint		TBezierPoint[BEZIER_POINT_MAX];		// 路径信息
};

// 同步消息
struct CMD_S_Synchronous
{						 
	uint				nOffSetTime;			// 偏移时间	
};

// 捕获鱼
struct CMD_S_CatchFish
{
	uint				nFishKey;				// 鱼索引
	WORD				wChairID;				// 玩家位置
	int					nMultipleCount;			// 倍数数量
	LONGLONG			lScoreCount;			// 获得数量
};

// 开火
struct CMD_S_Fire
{
	uint				nBulletKey;				// 子弹关键值
	int					nBulletScore;			// 子弹分数
	int					nMultipleIndex;			// 倍数索引
	int					nTrackFishIndex;		// 追踪鱼索引
	WORD				wChairID;				// 玩家位置
	CShortPoint			ptPos;					// 位置
	LONGLONG			llPlayScore;			// 玩家实时积分
};


// 准备激光
struct CMD_S_BeginLaser
{
	WORD				wChairID;				// 玩家位置
	CShortPoint			ptPos;					// 位置
};

// 激光
struct CMD_S_Laser
{
	WORD				wChairID;				// 玩家位置
	CShortPoint			ptPos;					// 位置
};

// 激光奖励
struct CMD_S_LaserReward
{
	WORD				wChairID;				// 玩家位置
	LONGLONG			lScore;					// 分数奖励
};

// 转换场景
struct CMD_S_ExchangeScene
{
	byte				cbBackIndex;			// 背景索引
};

// 结算
struct CMD_S_Over
{
	WORD				wChairID;				// 玩家位置
};

// 延迟
struct CMD_S_Delay
{
	int					nDelay;					// 延迟
	WORD				wChairID;				// 玩家位置
};

// 银行查看
struct CMD_S_BankTake
{
	WORD				wChairID;				// 玩家位置
	LONGLONG			lPlayScore;				// 用户积分
};

// 语音消息
struct CMD_S_Speech
{
	WORD				wChairID;				// 玩家位置
	int					nSpeechIndex;			// 语音索引
};

// 倍数选择
struct CMD_S_Multiple
{
	WORD				wChairID;				// 玩家位置
	int					nMultipleIndex;			// 倍数索引
};

// 补给提示
struct CMD_S_SupplyTip
{
	WORD				wChairID;				// 玩家位置
};

// 补给信息
struct CMD_S_Supply
{
	WORD				wChairID;				// 玩家位置
	LONGLONG			lSupplyCount;			// 补给数量
	EnumScoreType		nSupplyType;			// 补给类型
};

// 提示消息
struct CMD_S_AwardTip
{
	WORD				wTableID;				// 桌子ID
	WORD				wChairID;				// 位置ID
	tchar				szPlayName[32];			// 用户名
	byte				nFishType;				// 鱼种类
	int					nFishMultiple;			// 鱼倍数
	LONGLONG			lFishScore;				// 具体分数
	EnumScoreType		nScoreType;				// 分数类型
};

// 结构信息
struct CMD_S_Control
{
	uint				nMessageID;
	uint				nMessageSize;
	byte				cbMessageInfo[1024];
};

// 更新游戏
struct CMD_S_UpdateGame
{
	int					nMultipleValue[Multiple_Max];						// 房间倍数
	int					nCatchFishMultiple[FishType_Max][RAND_INDEX];		// 捕鱼倍数
	int					nBulletVelocity;									// 子弹速度
	int					nBulletCoolingTime;									// 子弹冷却
};

// 停留鱼
struct CMD_S_StayFish
{
	uint				nFishKey;							// 鱼索引
	uint				nStayStart;							// 停留开始
	uint				nStayTime;							// 停留时间
};

////////////////////////////////////////////////////////////////////////// 
// 客户端命令结构
enum
{
	SUB_C_CATCH_FISH = 101,							// 捕鱼信息
	SUB_C_FIRE,										// 开火
	SUB_C_DELAY,									// 延迟
	SUB_C_BEGIN_LASER,								// 准备激光
	SUB_C_LASER,									// 激光
	SUB_C_SPEECH,									// 语音消息
	SUB_C_MULTIPLE,									// 倍数消息
	SUB_C_CONTROL,									// 控制消息
	SUB_C_SKILL,									// 释放技能
	SUB_C_CHANGE_CONNON,							// 切换炮台
};

// 捕中最大数量
#define FishCatch_Max			5

// 捕获鱼
struct CMD_C_CatchFish
{
	uint				nBulletKey;					// 子弹关键值
	uint				nFishKey[FishCatch_Max];	// 捕中鱼
};

// 开火
struct CMD_C_Fire
{
	int					nMultipleIndex;				// 倍数索引
	int					nTrackFishIndex;			// 追踪鱼索引
	uint				nBulletKey;					// 子弹关键值
	CShortPoint			ptPos;						// 位置
};

// 准备激光
struct CMD_C_BeginLaser
{
	CShortPoint			ptPos;						// 位置
};

// 激光
struct CMD_C_Laser
{
	CShortPoint			ptBeginPos;					// 开始位置
	CShortPoint			ptEndPos;					// 结束位置
	uint				unLossTime;					// 已过时间
};

// 语音消息
struct CMD_C_Speech
{
	int					nSpeechIndex;				// 语音索引
};

// 倍数消息
struct CMD_C_Multiple
{
	int					nMultipleIndex;			// 语音索引
};

// 结构信息
struct CMD_C_Control
{
	uint				nMessageID;
	uint				nMessageSize;
	byte				cbMessageInfo[1024];
};

//技能
struct CMD_C_Skill
{
	byte	SkillType; //技能类型 2,冰冻,3,锁定
};

//炮台切换
struct CMD_C_CANNON
{
	BYTE desksTation;   //座位号
	BYTE cannonIndex;	//炮台索引
	CMD_C_CANNON()
	{
		desksTation = 255;
		cannonIndex = 255;
	}
};
////////////////////////////////////////////////////////////////////////// 

// 难度索引
#define Difficulty_Type		3	
#define Difficulty_Max		10	

enum EnumExplosionConditionType
{
	ExplosionConditionType_Gun,
	ExplosionConditionType_Gold,
};

// 自定义配置
struct tagCustomRule
{
	// 倍数配置
	int						nMultipleValue[Multiple_Max];		// 房间倍数

	// 控制配置
	int						nTaxRatio;								// 抽水比例
	int						nInitialStock;							// 初始库存
	int						nRoomDifficultyCount[Difficulty_Max];	// 房间难度
	int						nTableDifficultyCount[Difficulty_Max];	// 桌子难度
	int						nPlayDifficultyCount[Difficulty_Max];	// 椅子难度
	double					dRoomDifficultyValue[Difficulty_Max];	// 房间难度
	double					dTableDifficultyValue[Difficulty_Max];	// 桌子难度
	double					dPlayDifficultyValue[Difficulty_Max];	// 椅子难度

	// 场景配置
	int						nCreateCount;							// 创建数量
	int						nSceneTime;								// 场景维持时间

	// 子弹配置
	int						nBulletVelocity;						// 子弹速度
	int						nBulletCoolingTime;						// 子弹冷却

	// 活动配置
	int						nSupplyCondition[2];					// 补给条件
	int						nLaserTime;								// 激光时间
	int						nLaserChance;							// 激光几率
	int						nSpeedTime;								// 加速时间
	int						nSpeedChance;							// 加速几率
	int						nGiftScore[5];							// 赠送金币
	int						nGiftChance[5];							// 赠送几率
	int						nNullChance;							// 空箱几率

	// 鱼配置
	int						nCatchFishMultiple[FishType_Max][RAND_INDEX];		// 捕鱼倍数

	// 2.0 拓展
	int						nDifficultyStart[Difficulty_Type];		// 难度类型

	// 3.0 拓展
	int						nExplosionProportion;					// 爆炸比例
	int						nExplosionStart;						// 爆炸启动
	LONGLONG				lExplosionCondition;					// 爆炸条件
	EnumExplosionConditionType nExplosionConditionType;				// 条件类型

	// 4.0 拓展
	int						nAwardMinMultiple;
	BOOL					nAwardChatBox;

	tagCustomRule()
	{
		DefaultCustomRule();
	}

	// 默认配置
	void DefaultCustomRule()
	{
		// 房间倍数
		int nTempMultipleValue[Multiple_Max] = { 1, 10, 100, 500, 1000, 5000 };
		CopyMemory( nMultipleValue, nTempMultipleValue, sizeof(nMultipleValue) );

		// 控制配置
		nInitialStock = 0;
		nTaxRatio = 3;

		// 难度控制
		nRoomDifficultyCount[0] = 50;
		nRoomDifficultyCount[1] = 100;
		nRoomDifficultyCount[2] = 200;
		nRoomDifficultyCount[3] = 400;
		nRoomDifficultyCount[4] = 800;
		nRoomDifficultyCount[5] = 1200;
		nRoomDifficultyCount[6] = 2000;
		nRoomDifficultyCount[7] = 3000;
		nRoomDifficultyCount[8] = 6000;
		nRoomDifficultyCount[9] = 10000;

		nTableDifficultyCount[0] = 50;
		nTableDifficultyCount[1] = 100;
		nTableDifficultyCount[2] = 200;
		nTableDifficultyCount[3] = 400;
		nTableDifficultyCount[4] = 800;
		nTableDifficultyCount[5] = 1200;
		nTableDifficultyCount[6] = 2000;
		nTableDifficultyCount[7] = 3000;
		nTableDifficultyCount[8] = 6000;
		nTableDifficultyCount[9] = 10000;

		nPlayDifficultyCount[0] = -10000;
		nPlayDifficultyCount[1] = -1000;
		nPlayDifficultyCount[2] = -200;
		nPlayDifficultyCount[3] = -100;
		nPlayDifficultyCount[4] = -50;
		nPlayDifficultyCount[5] = 0;
		nPlayDifficultyCount[6] = 50;
		nPlayDifficultyCount[7] = 200;
		nPlayDifficultyCount[8] = 5000;
		nPlayDifficultyCount[9] = 10000;

		dRoomDifficultyValue[0] = 0.05;
		dRoomDifficultyValue[1] = 0.15;
		dRoomDifficultyValue[2] = 0.25;
		dRoomDifficultyValue[3] = 0.50;
		dRoomDifficultyValue[4] = 0.75;
		dRoomDifficultyValue[5] = 1.00;
		dRoomDifficultyValue[6] = 1.25;
		dRoomDifficultyValue[7] = 1.70;
		dRoomDifficultyValue[8] = 2.50;
		dRoomDifficultyValue[9] = 3.50;

		dTableDifficultyValue[0] = 0.05;
		dTableDifficultyValue[1] = 0.15;
		dTableDifficultyValue[2] = 0.25;
		dTableDifficultyValue[3] = 0.50;
		dTableDifficultyValue[4] = 0.75;
		dTableDifficultyValue[5] = 1.00;
		dTableDifficultyValue[6] = 2.50;
		dTableDifficultyValue[7] = 2.75;
		dTableDifficultyValue[8] = 3.00;
		dTableDifficultyValue[9] = 3.50;

		dPlayDifficultyValue[0] = 2.50;
		dPlayDifficultyValue[1] = 2.15;
		dPlayDifficultyValue[2] = 1.75;
		dPlayDifficultyValue[3] = 1.15;
		dPlayDifficultyValue[4] = 0.80;
		dPlayDifficultyValue[5] = 0.60;
		dPlayDifficultyValue[6] = 0.40;
		dPlayDifficultyValue[7] = 0.20;
		dPlayDifficultyValue[8] = 0.15;
		dPlayDifficultyValue[9] = 0.10;

		// 场景配置
		nCreateCount = 12;
		nSceneTime = 60 * 8;

		// 子弹配置
		nBulletVelocity = 15000;					
		nBulletCoolingTime = 150;

		// 活动配置
		RAND_INITIAL(nSupplyCondition, 500, 700);
		nLaserTime = 30;
		nLaserChance = 10;
		nSpeedTime = 60;
		nSpeedChance = 60;
		nGiftScore[0] = 10;
		nGiftScore[1] = 30;
		nGiftScore[2] = 50;
		nGiftScore[3] = 80;
		nGiftScore[4] = 120;
		nGiftChance[0] = 5;
		nGiftChance[1] = 5;
		nGiftChance[2] = 5;
		nGiftChance[3] = 5;
		nGiftChance[4] = 5;
		nNullChance = 5;

		// 鱼配置
		int	nTempFishMultiple[FishType_Max][RAND_INDEX] = { { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 6 }, { 7, 7 }, { 8, 8 }, { 9, 9 }, { 10, 10 }, { 12, 12 }, { 15, 15 }, { 18, 18 }, { 20, 20 }, { 25, 25 }, { 30, 30 }, { 40, 40 }, { 150, 150 }, { 200, 200 }, { 300, 300 }, { 400, 400 }, { 400, 1000 }, { 20, 20 }, { 20, 20 }, { 20, 20 }, { 0, 0 } };
		CopyMemory(nCatchFishMultiple, nTempFishMultiple, sizeof(nCatchFishMultiple));

		// 难度开启
		nDifficultyStart[0] = TRUE;
		nDifficultyStart[1] = TRUE;
		nDifficultyStart[2] = FALSE;

		// 爆炸信息
		nExplosionProportion = 50;
		nExplosionStart = 5000;
		lExplosionCondition = 3000;
		nExplosionConditionType = ExplosionConditionType_Gun;

		// 中奖配置
		nAwardMinMultiple = 50;
		nAwardChatBox = TRUE;
	}
};




// 还原对其数
#pragma pack()

////////////////////////////////////////////////////////////////////////// 

#endif