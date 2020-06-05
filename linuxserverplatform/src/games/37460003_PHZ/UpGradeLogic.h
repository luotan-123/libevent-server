#pragma once
#include "GameData.h"

//操作掩码
#define	UG_HUA_MASK					0xF0			//1111 0000
#define	UG_VALUE_MASK				0x0F			//0000 1111

#define MAX_CARDVALUE		0x1A + 1
#define MAX_WEAVE			7						//最大组合

#define GAME_MODE			15						//15胡息摸试

struct CheckHuStruct
{
	int iData[MAX_CARDVALUE];
	int iCardCount;

	void AddCard(BYTE byPai)				//加入牌
	{
		if(byPai < MAX_CARDVALUE)
		{
			iData[byPai] ++;
			iCardCount ++;
		}
	}
	void AddCard(BYTE byPai,int iNumber)
	{
		if(byPai < MAX_CARDVALUE)
		{
			iCardCount += iNumber;
			iData[byPai] += iNumber;
		}
	}
	void DecCard(BYTE byPai,int iNumber)
	{
		if(byPai < MAX_CARDVALUE)
		{
			iCardCount -= iNumber;
			iData[byPai] -= iNumber;
		}
	}

	int GetPaiCount(BYTE byPai)			//取得指定牌个数
	{
		if(byPai < MAX_CARDVALUE){ return iData[byPai]; }
		return 0;
	}

	int GetAllCardCount()
	{
		return iCardCount;
	}
	void Clear()
	{
		::memset(this,0,sizeof(CheckHuStruct));
	}

	CheckHuStruct()
	{
		Clear();
	}
};

//逻辑类 
class CGameLogic
{
	//变量定义
protected:
	GameData						*m_pGameData;						//游戏数据
	static const BYTE				m_cbyCardDataArray[84];				//扑克数据
	int								m_iMinHuXi;							//最小胡息
	int								m_iMaxDun[5];						//最大囤数(0:普通胡,1:王钓,2:王闯,3:三王闯,4:王炸)
	//[胡牌检测用到的变量]
protected:	
	int								m_iJingNum;							//金牌个数
	CheckHuStruct					m_tCheckHuChangleData;				//转换牌数据,方便胡牌检测(此数组不含财神)

	tagHuCardInfo					m_tHuCardInforTemp;					//用于保存检测胡牌前的一些数据

	tagAnalyseItem					m_AnalyseItemTemp[5];				//临时保存分析的胡牌数据(0:普通胡,1:王钓,2:王闯,3:三王闯,4:王炸)
	tagAnalyseItem					m_AnalyseItemHu;					//保存胡牌数据

	BYTE							m_byXingPai;						//醒牌值
	BYTE							m_byPai;							//胡的那张牌
	BYTE							m_byStation;						//胡牌玩家位置
	BYTE							m_byBeStation;						//被胡牌玩家位置
	int								m_iRedCardNum;						//红牌个数
	int								m_iCPTWPXing;						//吃碰提偎跑的醒个数
	int								m_iCPTWPHuXi;						//吃碰杠胡息
	int								m_iCPGWPTRedCardNum;				//吃碰杠偎提跑牌值

	bool							m_bZiMo;							//当前是否是自摸
	bool							m_bHuFlag;							//当前胡牌标志
	bool							m_bWeaveReplace;					//偎牌替换
	tagWeaveItem					m_tCPTWPArray[7];					//组合扑克(保存:吃碰偎提跑)

	GameRuler						m_tGameRules;						//游戏规则
	BYTE							m_realJingNum[PLAY_COUNT];			//实际精牌个数
public:
	//构造函数
	CGameLogic(void);
	~CGameLogic(void);
	//[设置值相关]
public:
	//初始化
	void Init(GameData *pGameData);
	//设置最小胡息
	void SetMinHuXi(int iMinHuXi){ m_iMinHuXi = iMinHuXi;}
	//设置吃碰杠提偎跑红字个数
	void SetCPGTWPRedCardNum(int iRedWordNum){m_iCPGWPTRedCardNum = iRedWordNum;}
	//设置吃碰提偎跑醒个数
	void SetCPTWPXingNum(int iXingNum){m_iCPTWPXing = iXingNum;}
	//醒牌值
	void SetXingPai(BYTE byXingPai) { m_byXingPai = byXingPai; }
	//[洗牌与删除函数]
public:
	//混乱扑克
	void RandCardData(BYTE byCardData[], BYTE byMaxCount);
	//删除扑克
	bool RemoveCard(BYTE byCardIndex[MAX_INDEX], BYTE byRemoveCard);
	//删除扑克
	bool RemoveCard(BYTE byCardIndex[MAX_INDEX], BYTE byRemoveCard[], BYTE byRemoveCount);
	//删除扑克
	bool RemoveCard(BYTE byCardData[], BYTE byCardCount, BYTE byRemoveCard[], BYTE byRemoveCount);
	//[转换函数]
public:
	//扑克转换
	BYTE SwitchToCardData(BYTE byCardIndex);
	//扑克转换
	BYTE SwitchToCardIndex(BYTE cbCardData);
	//扑克转换
	BYTE SwitchToCardData(BYTE cbCardIndex[MAX_INDEX], BYTE cbCardData[], BYTE bMaxCount,BYTE byJinNum = 0);
	//扑克转换
	BYTE SwitchToCardIndex(BYTE byCardData[], BYTE byCardCount, BYTE byCardIndex[MAX_INDEX],BYTE &byJinNum);
	//[辅助函数]
public:
	//有效判断
	bool IsValidCard(BYTE byCardData);
	//检测当前是否有牌
	bool CheckIsHaveCard(bool bCardIndex[MAX_INDEX],BYTE byCurrentCard);
	//扑克数目
	BYTE GetCardCount(BYTE byCardIndex[MAX_INDEX],BYTE byJinNum);
	//获取胡息
	BYTE GetWeaveHuXi(const tagWeaveItem & WeaveItem,bool bWei=false);
	//当前是否是红字(二，七，十)
	bool IsEQS(BYTE byCard);
	//取得手牌里面的红字总数
	int  GetHandRedWordNum();
	//设置最大胡牌最大分
	void SetHuMaxCardData(int iIndex,int iXing,BYTE byHuXiCount,int iTunSum,int iFan,int iHuType);
	///判断是否能提取指定的牌型
	bool  JudgeCanTrackOutEatType(BYTE byCardIndex[MAX_INDEX],BYTE byEatCardIndex,BYTE byFirstCard,BYTE byCenterCard,BYTE byEndCard,int &iMinCount);
	//计算醒牌个数
	int  GetXingCount();
	//王个数
	int GetWangCount();
	//[逻辑函数]
public:
	//提牌判断
	BYTE GetAcitonTiCard(BYTE byCardIndex[MAX_INDEX], BYTE byTiCardIndex[5]);
	//畏牌判断
	BYTE GetActionWeiCard(BYTE byCardIndex[MAX_INDEX], BYTE byWeiCardIndex[7]);
	//吃牌判断
	BYTE GetActionChiCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard, EatCardMemory MemeoryEatCard[6]);
	//[判断函数]
	//是否吃牌
	bool IsChiCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard);
	//是否提跑
	bool IsTiPaoCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard);
	//是否偎碰
	bool IsWeiPengCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard);
	//能否碰牌
	bool IsPengCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard);
	//胡牌结果
	bool GetHuCardInfo(BYTE byStation,BYTE byBeStation,BYTE byCardIndex[MAX_INDEX], int iJinNum,BYTE byCurrentCard, BYTE byHuXiWeave, tagHuCardInfo tHuCardInfo[],bool bZiMo,tagWeaveItem tWeave[],bool bOutCard=false, bool bCheckAnyHu = false);
	//检测癞子胡
	bool CheckAnyHu(BYTE byStation,BYTE byCardIndex[MAX_INDEX], int iJinNum,BYTE byCurrentCard, BYTE byHuXiWeave, tagHuCardInfo tHuCardInfo[],bool bZiMo,int *pHuAction,tagWeaveItem tWeave[]);
	//检测听牌
	bool CanTing(BYTE byStation,BYTE byBeStation,BYTE byCardIndex[MAX_INDEX], int iJinNum,BYTE byHuXiWeave, tagHuCardInfo tHuCardInfo[],bool bZiMo,tagWeaveItem tWeave[], BYTE byHuCardsIndex[MAX_INDEX]);
	//[内部函数]
public:
	//提取吃牌
	BYTE TakeOutChiCard(BYTE byCardIndex[MAX_INDEX], BYTE byCurrentCard, BYTE byResultCard[3]);
	//[胡牌检测相关]
public:
	void ChangeHandPaiData(BYTE byHandpai[],BYTE byPai);								//转换牌数据
	bool AnalyseCard(BYTE cbCardIndex[MAX_INDEX],BYTE byPai,int iJingNum = 0);			//检测胡牌
	bool MakePingHu(CheckHuStruct &PaiData,tagAnalyseItem &hupaistruct,int &recursiveDepth, int iJingNum=0);	//组胡牌数据(组成三个或一句话或绞或2710)
	bool GetMaxHuXi();																	//取得最大胡息
	//[胡牌牌型]
public:
	//------------------------------------------------------------------
	//十五胡玩法
	bool IsShiShiRedHu_15(int iRedNum);				//十红胡
	bool IsOneRedHu_15(int iRedNum);				//一个红胡
	bool IsDianHu_15(int iRedNum);					//点红胡
	bool IsShiWuRedHu_15(int iRedNum);				//十五红胡
	bool IsAllBlackHu_15(int iRedNum);				//全黑
	//------------------------------------------------------------------
	//六胡玩法
	bool IsRedHu_6(int iRedNum);					//红胡
	bool IsOneRedHu_6(int iRedNum);					//一个红
	bool IsAllRedHu_6(int iRedNum);					//全红
	bool IsJiuRedHu_6(int iRedNum);					//九红胡
	bool IsAllBlackHu_6(int iRedNum);				//全黑
	//------------------------------------------------------------------
	bool IsWanDiao();					//是否是王钓
	bool IsWangChuang();				//是否是王闯
	int IsSanWangChuang();				//三王闯
	bool IsWangZha();					//当前是否是王炸

	//设置游戏规则
	void SetGameRule(GameRuler &gameRule);
	//设置玩家实际精牌个数
	void SetRealJingNum(BYTE realJingNum[]);
};
