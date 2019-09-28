#pragma once
#include "StdAfx.h"
#include "MJ/GameDataEx.h"
#include "MJ/CheckHuPaiEx.h"
#include "MJ/CheckCPGActionEx.h"
#include "iostream"
#include "string"
#include "json/json.h"
#include "fstream"
#pragma comment(lib,"json_vc71_libmt.lib")
#pragma warning(disable:4305)
#pragma warning(disable:4309)

//此处添加游戏定时器：游戏定时器
//特别注意定时器的ID范围1-50
#define TIME_ACTION				    30  //麻将动作定时器
#define TIME_THINK				    31  //思考定时器
#define TIME_TING_OUTCARD			32	//听牌出牌
#define TIME_TUOGUAN			    33	//托管出牌
#define TIME_HUAPAI                 34  //开局花牌
#define TIME_BUHUA                  35  //补花
#define TIME_CHECK_ACTION           36  //检测动作
#define TIME_DELAY_SEND_ACTION      38  //延时发送动作
#define TIME_TUOGUAN_ACTION			39	//托管动作（吃碰杠胡听等等）

/// 游戏桌子类
class CServerGameDesk : public CGameDesk
{
public:
	GameDataEx						m_GameData;							//游戏数据
	CheckCPGActionEx				m_CPGData;							//吃碰杠数据
	CheckHuPaiEx					m_CheckHuData;						//胡牌数据
	bool							m_bAgree[PLAY_COUNT];				//同意标示
	bool							m_bTrustee[PLAY_COUNT];				//是否托管
	BYTE                            m_byMJCardType;						//麻将牌样式
	int								m_iPlayingCount;					//游戏进行局数
	CMD_S_ZongResult				m_tZongResult;						//总战绩
	int								m_iVideoIndex;                      //战绩回放索引
	Json::Value						m_element;                          //战绩回放元素
	Json::Value						m_root;                             //战绩回放的根
	bool							m_bNetCut[PLAY_COUNT];				//断线
	BYTE                            m_byBeginTempCard;					//临时保存的牌
	int                             m_iOutCardChaoShiCount[PLAY_COUNT]; //超时出牌次数

	////////////////////////////////静态变量///////////////////////////////
	static bool                     gOutMessageSize; //输出协议大小

	////////////////////////////////超端///////////////////////////////
	bool							m_bSuperFlag[PLAY_COUNT];			//超端玩家结果
	BYTE							m_ChangeCard[PLAY_COUNT];			//超端玩家下次抓牌牌值

public:
	/// 构造函数
	CServerGameDesk();
	/// 析构函数
	virtual ~CServerGameDesk();
public:
	/// 初始游戏
	virtual bool InitDeskGameStation();

	/// 游戏开始
	virtual bool GameBegin(BYTE bBeginFlag);

	/// 游戏结束
	virtual bool GameFinish(BYTE bDeskStation, BYTE bCloseFlag);

	/// 判断是否正在游戏
	virtual bool IsPlayGame(BYTE bDeskStation);

	/// 游戏数据包处理函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation, unsigned int assistID, void* pData, int size, bool bWatchUser = false);

	/// 用户离开游戏桌
	virtual bool UserLeftDesk(GameUserInfo* pUser);

	//玩家断线处理
	virtual bool UserNetCut(GameUserInfo *pUser);

	/// 获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);

	/// 重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);

	/// 定时器消息
	virtual bool OnTimer(UINT uTimerID);

	//桌子成功解散
	virtual void OnDeskSuccessfulDissmiss(bool isDismissMidway = true);

	// 玩家请求托管
	virtual bool OnHandleUserRequestAuto(BYTE deskStation);

	// 玩家取消托管
	virtual bool OnHandleUserRequestCancelAuto(BYTE deskStation);

	//初始化游戏数据，非正常大结算初始化
	virtual void InitDeskGameData();

	// 动态加载配置文件数据
	virtual void LoadDynamicConfig();
public:
	///检测玩家station当前能做哪些动作
	bool CheckAction(int iStation, BYTE byCard, BYTE byType);

	//抓牌
	void ZhuaPai(BYTE byCurrentStation, bool bHead);

	//用户出牌
	bool OnUserOutCard(BYTE byDeskStation, CMD_C_OutCard *pOutCard);

	//用户操作
	bool OnUserOperate(BYTE byDeskStation, CMD_C_OperateCard *pAction);

	//吃牌
	bool OnChi(BYTE byDeskStation, CMD_C_OperateCard *pAction);

	//碰牌
	bool OnPeng(BYTE byDeskStation, CMD_C_OperateCard *pAction);

	//杠牌
	bool OnGang(BYTE byDeskStation, CMD_C_OperateCard *pAction);

	//胡牌
	bool OnHu(BYTE byDeskStation, CMD_C_OperateCard *pAction);

	//过
	bool OnPass(BYTE byDeskStation, CMD_C_OperateCard *pAction);

	///有玩家有动作发送拦牌信息给玩家
	bool HaveUserCanDoAction(BYTE byIgnoreStation);

	//发送吃碰杠听
	void SendChiPengGangHuMsg(int iDesk);

	////记录杠牌状态，拥于杠开和杠后炮
	void RecordGangStatus();

	//json存储，战绩回放
	void OnJson(void *pData, int iType);

	//配牌
	bool HandCard();

	//获取房间规则
	void GetSpecialRule();

	//根据tagStructGameRuler设置相应规则
	void SetSpecialRule();

	//自动选择一张牌出
	BYTE AutoOutCard(BYTE byStation);

	//自动动作
	bool AutoCanAction(BYTE byStation, BYTE byActionType);

	//发送出牌出错消息
	void SendOutCardError(BYTE byDeskStation, int iErrorCode);

	//超端玩家请求看门牌
	bool SuperUserLookMenPai(BYTE byDeskStation, CMD_C_SuperLook *SuperLook);

	//超端玩家请求要牌
	bool SuperUserChangePai(BYTE byDeskStation, CMD_C_Super_Request_Card *SuperChangeCard);
};