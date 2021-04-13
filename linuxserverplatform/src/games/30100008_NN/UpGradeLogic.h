#pragma once

#include "GameDesk.h"
#include "UpgradeMessage.h"

//操作掩码
#define	UG_HUA_MASK					0xF0			//1111 0000
#define	UG_VALUE_MASK				0x0F			//0000 1111

//牛牛牌型
#define UG_NO_POINT					0x00			//无点牌
#define UG_BULL_ONE                 0x01            //牛一
#define UG_BULL_TWO                 0x02            //牛二
#define UG_BULL_THREE               0x03            //牛三
#define UG_BULL_FOUR                0x04            //牛四
#define UG_BULL_FIVE                0x05            //牛五
#define UG_BULL_SIX                 0x06            //牛六
#define UG_BULL_SEVEN               0x07            //牛七
#define UG_BULL_EIGHT               0x08            //牛八
#define UG_BULL_NINE                0x09            //牛九
#define UG_BULL_BULL				0x0A			//牛牛
//#define UG_BULL_SILVER              0X0B           //银牛
#define UG_SHUN_BULL				0X0C			//顺子牛
#define UG_BULL_GOLD				0X0D            //五花牛就是金牛
#define UG_TONGHUA_BULL				0X0E			//同花牛
#define UG_HULU_BULL				0X0F            //葫芦牛
#define UG_BULL_BOMB				0X10            //炸弹牛
#define UG_FIVE_SMALL				0X11            //五小

#define PAI_TYPE_COUNT				0x12			//牌型数目		

static char G_szPaiType[PAI_TYPE_COUNT][20]={"无点牌","牛一","牛二","牛三","牛四","牛五","牛六","牛七","牛八","牛九","牛牛","银牛","顺子牛","五花牛","同花牛","葫芦牛","炸弹牛","五小牛"};
//升级逻辑类 
class CUpGradeGameLogic
{
	//变量定义
private:
	 // UINT           m_iCardShape;	//支持牌型
	TRoomRulerStruct m_tGRStruct;
	//函数定义
public:
	//构造函数
	CUpGradeGameLogic(void){};
	~CUpGradeGameLogic(void){}
	//功能函数（公共函数）
public:
	//获取扑克数字
	inline int GetCardNum(BYTE iCard) { return (iCard&UG_VALUE_MASK)+1; }
	//获取扑克花色
	BYTE GetCardHuaKind(BYTE iCard);
	//排列扑克
	int SortCard(BYTE iCardList[], BYTE bUp[], int iCardCount);
	//点
	int GetPoint(int Card);
	//辅助函数
public:
	//获取扑克相对大小
	int GetCardBulk(BYTE iCard, int bExtVol = false);
	//对比单牌
	int CompareOnlyOne(BYTE iFirstCard, BYTE iNextCard);
	//服务器专用函数
public:
	//混乱扑克
	BYTE RandCard(BYTE iCard[], int iCardCount,bool bhaveKing=true);
public:
	//比较手中牌大小
	int CompareCard(BYTE iFirstCardList[], int iFirstCardCount,BYTE iFirstUpCard[], BYTE iSecondCardList[], int iSecondCardCount,BYTE iSecondUpCard[]);
	//比较手中牌大小
	int NewCompareCard(BYTE byFirstCard[], int iFirstCount, BYTE bySecondCard[], int iSecondCount);
	//是否可以任取三个和在20,10,0内
	int CanSumIn(BYTE  iCardList[],int iCardCount,int iSelectNum);
	//得到当前牌型
	int GetShape(BYTE iCardList[],int iCardCount, BYTE iUpCard[] = NULL);
	//是牛
	bool IsBull(BYTE byCardList[],int iCardCount);
	//计算牌中的点数
	int CountPoint(BYTE  iCardList[],int iCardCount);
	// 获得牛牌数组
	bool GetBull(BYTE byCardList[], int iCardCount, BYTE iCardResult[]);
	//获取无牛的牌型数组
	bool GetNoBull(BYTE iCardList[], int iCardCount, BYTE iCardResult[]);
	///设置游戏牌型
	// void SetCardShape(UINT iCardShape){m_iCardShape=iCardShape;}
	void SetCardShape(const TRoomRulerStruct tGRStruct){m_tGRStruct =tGRStruct;}
	//内部函数
private:
	///牛牛
	bool IsBullBull(BYTE  byCardList[],int iCardCount=5);
	/// 金牛
	bool IsGoldBull(BYTE  byCardList[],int iCardCount=5);
	/// 银牛
	bool IsSilverBull(BYTE  byCardList[],int iCardCount=5);
	/// 炸弹牛
	bool IsBombBull(BYTE  byCardList[],int iCardCount=5);
	/// 五小
	bool IsFiveSmall(BYTE  byCardList[],int iCardCount=5);
	//是否有点
	int IsHaveNote(BYTE  byCardList[],int iCardCount=5);
	/// 得到牌组中最大的牌
	int GetMaxCard(BYTE  byCardList[],int iCardCount=5);
	/// 得到牌组中最小的牌
	int GetMinCard(BYTE  byCardList[],int iCardCount=5);

	///顺子牛
	bool IsShunBull(BYTE  byCardList[],int iCardCount=5);
	///同花牛
	bool IsTongHuaBull(BYTE  byCardList[],int iCardCount=5);
	///葫芦牛
	bool IsHuLuBull(BYTE  byCardList[],int iCardCount=5);

	// 葫芦牛和炸弹牛和五小牛比较大小   true：byFirstCard > bySecondCard;
	bool CompareCardToSpecial(BYTE byFirstCard[], BYTE bySecondCard[],int iSpecial);
};
