#pragma once
#include "../GameMessage/UpGradeLogic.h"
class CGameTaskLogic :public CUpGradeGameLogic
{
public:
	CGameTaskLogic(void);
public:
	~CGameTaskLogic(void);

private:

	BYTE   m_byLastCardList[20] ;  ///最后一手牌的列表
	BYTE   m_byLastCardCount ;     ///最后一手牌的数量

	BYTE   m_byBackCardType  ;   ///底牌牌型 

	BYTE   m_byTaskType  ;       ///需随机产生
	BYTE   m_bySpecifyShape ;    ///指定的牌型
	BYTE   m_bySpecifyCard ;     ///指定的牌

public:
	///初始任务变量
	void   InitGameTask(void) ; 
	///获取底牌牌型
	bool   GetBackCardType(BYTE bbackCardList[] , int  cbCardCount) ;  
    ///底牌是否为顺子
	bool   IsBackCardStraight(BYTE iCardList[],int iCardCount) ; 
	// 获取指定牌面值牌的数量
	int    GetCardNumCount(BYTE iCardList[], int iCardCount, BYTE bCardNum) ; 
    ///获取随机任务
	void   GetRandTask(int inDex) ;  ///获取随机任务
	///    设置任务变量
	void   SetGameTask(GameTaskStruct &gameTask) ; 
	///设置最后一手牌的数据
	void   SetLastCardData(BYTE iCardList[], BYTE iCardCount)  ;
	///判断是否完成任务了
	bool   IsFinishTask(void) ; 
	///获取任务倍数
	int   GetTaskMutiple(bool bFinish ) ;
	///获取底牌倍数
	int   GetBackCardMytiple(void) ; 

};
