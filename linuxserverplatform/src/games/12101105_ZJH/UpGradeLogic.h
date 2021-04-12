#pragma once

#include "GameDesk.h"
#include "UpgradeMessage.h"

//操作掩码
#define	UG_HUA_MASK					0xF0			//1111 0000
#define	UG_VALUE_MASK				0x0F			//0000 1111

//扑克花色
#define UG_FANG_KUAI				0x00			//方块	0000 0000
#define UG_MEI_HUA					0x10			//梅花	0001 0000
#define UG_HONG_TAO					0x20			//红桃	0010 0000
#define UG_HEI_TAO					0x30			//黑桃	0011 0000
#define UG_NT_CARD					0x40			//主牌	0100 0000
#define UG_ERROR_HUA				0xF0			//错误  1111 0000

//扑克出牌类型
#define SH_THREE					7				// 豹子 三条
#define SH_SAME_HUA_CONTINUE		6				// 顺金 同花顺
#define SH_SAME_HUA					5				// 金花  同花
#define SH_CONTINUE					4				// 顺子
#define SH_DOUBLE					3				//对子
#define SH_OTHER					2				//单牌
#define SH_SPECIAL					1				//特殊235
#define UG_ERROR_KIND				0				//错误


//升级逻辑类 
class CUpGradeGameLogic
{
	//变量定义
public:
	bool    m_bLogic235ShaBaoZi;//235杀豹子
	BYTE    m_byLogicHuiPai;//会牌
	int    m_byTeSuPaiXing; ///特殊牌型 1 顺子>同花 ,2 顺子 < 同花    

	//函数定义
public:
	//构造函数
	CUpGradeGameLogic(void);
	//功能函数（公共函数）
public:
	//获取扑克数字
	inline int GetCardNum(BYTE iCard) { return (iCard&UG_VALUE_MASK)+1; }
	//获取扑克花色
	BYTE GetCardHuaKind(BYTE iCard);
	//排列扑克
	BOOL SortCard(BYTE iCardList[], BYTE bUp[], int iCardCount);
	//删除扑克
	int RemoveCard(BYTE iRemoveCard[], int iRemoveCount, BYTE iCardList[], int iCardCount);
	//重新排序
	BOOL ReSortCard(BYTE iCardList[], int iCardCount);
	//得到用中最大的牌数字
	BYTE  GetMaxCardNum(BYTE byCardList[],int iCardCount);
	//辅助函数
public:
	//是否单牌
	inline bool IsOnlyOne(BYTE byCardList[], int iCardCount) { return iCardCount==1; };
	//是否对牌
	bool IsDouble(BYTE byCardList[], int iCardCount);
	//是否三条
	bool IsThree(BYTE byCardList[], int iCardCount);
	//得到牌列花色
	BYTE GetCardListHua(BYTE byCardList[], int iCardCount);
	//获取牌型
	BYTE GetCardShape(BYTE byCardList[], int iCardCount);
	//获取扑克相对大小
	int GetCardBulk(BYTE iCard, BOOL bExtVol);
	//对比单牌
	bool CompareOnlyOne(BYTE iFirstCard, BYTE iNextCard);
	//是否顺子
	bool IsSingleContinue(BYTE byCardList[], int iCardCount);
	//是否同花
	bool IsSameHua(BYTE byCardList[], int iCardCount);
	//是否同花顺
	bool IsSameHuaContinue(BYTE iCardList[], int iCardCount);
	//特殊牌型
	bool IsSpecial(BYTE byCardList[],int iCardCount);
	//其他函数
public:
	//获取扑克
	BYTE GetCardFromHua(int iHuaKind, int iNum);
	//服务器专用函数
public:
	//混乱扑克
	BYTE RandCard(BYTE iCard[], int iCardCount,int iDeskIndex = 0);
	
	//测特殊牌型用
	BYTE RandCard_ShunZi(BYTE iCard[], int iCardCount,int iDeskIndex);	

	//特殊混乱扑克
	void RandCard_CrazyMode(BYTE iCard[], int iCardCount);

	//客户端函数
public:
	//比较手中牌大小
	int CompareCard(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount);
	//比较手中牌大小
	int CompareCardEx(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount);
	//内部函数
private:
	//清除 0 位扑克
	int RemoveNummCard(BYTE iCardList[], int iCardCount);
public:
	//顺子比大小
	int SingleContinueCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff = false);
	//同花、单牌比较
	int OneCardCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff = false);
	//对子比较
	int DoubleCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff = false);
	//三条比较大小
	int ThreeCompare(BYTE iFirstCardList[], int iFirstCardCount, BYTE iSecondCardList[], int iSecondCardCount,bool bDiff = false);
};