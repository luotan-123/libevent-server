#include "StdAfx.h"
#include "GameTaskLogic.h"

const  BYTE  byCardShape[MAX_CARD_SHAPE] = {UG_ONLY_ONE ,UG_DOUBLE ,UG_STRAIGHT,UG_THREE_ONE ,UG_THREE_TWO , UG_DOUBLE_SEQUENCE,UG_THREE_ONE_SEQUENCE ,UG_BOMB }  ; 


CGameTaskLogic::CGameTaskLogic(void)
{
	memset(m_byLastCardList , 0 , sizeof(m_byLastCardList)) ; 
	m_byLastCardCount = 0 ; 

	m_byBackCardType = 0 ; 
	m_byTaskType = 0 ; 
	m_bySpecifyShape = 0;
	m_bySpecifyCard = 0 ; 
}

CGameTaskLogic::~CGameTaskLogic(void)
{
}

void CGameTaskLogic::InitGameTask()
{
	memset(m_byLastCardList , 0 , sizeof(m_byLastCardList)) ; 
	m_byLastCardCount = 0 ; 

	m_byBackCardType = 0 ; 
	m_byTaskType = 0 ; 
	m_bySpecifyShape = 0;
	m_bySpecifyCard = 0 ; 
}
///获取底牌牌型
bool  CGameTaskLogic::GetBackCardType(BYTE bbackCardList[] , int  cbCardCount) 
{
	if(3 != cbCardCount)
	{
		return false ; 
	}

	BYTE iTempBackCard[3] ; 
	memset(iTempBackCard , 0 , sizeof(iTempBackCard)) ;
	memcpy(iTempBackCard , bbackCardList , sizeof(iTempBackCard)) ; 
  	
	SortCard(iTempBackCard , NULL ,cbCardCount) ; 

	if(iTempBackCard[0] == 0x4F && iTempBackCard[1] == 0x4E)
	{
		m_byBackCardType = TYPE_ROCKET ; 
	}
	else if(iTempBackCard[0] == 0x4F)
	{
		m_byBackCardType = TYPE_BIG_KING ; 
	}
	else if(iTempBackCard[0] == 0x4E)
	{
		m_byBackCardType = TYPE_SMALL_KING ; 
	}
	else
	{
		if(GetCardHuaKind(iTempBackCard[0] , true) == GetCardHuaKind(iTempBackCard[1] , true) && GetCardHuaKind(iTempBackCard[1] , true) == GetCardHuaKind(iTempBackCard[2] , true))
		{
			m_byBackCardType = TYPE_SAME_HUA ; 
		}
		else if(GetCardBulk(iTempBackCard[0]) == GetCardBulk(iTempBackCard[1]) &&  GetCardBulk(iTempBackCard[1]) == GetCardBulk(iTempBackCard[2]))
		{
			m_byBackCardType = TYPE_TRIPLE_CARD ; 
		}
		else if(GetCardBulk(iTempBackCard[0]) == GetCardBulk(iTempBackCard[1]) || GetCardBulk(iTempBackCard[1]) == GetCardBulk(iTempBackCard[2]))
		{
			m_byBackCardType = TYPE_DOUBLE_CARD; 
		}
		else if(true == IsBackCardStraight(iTempBackCard , cbCardCount))
		{
			m_byBackCardType = TYPE_STRAIT ; 
		}
		else
		{
			m_byBackCardType = TYPE_NONE ; 
		}
	}

	return (m_byBackCardType >0) ; 
}
///底牌是否为顺子
bool  CGameTaskLogic::IsBackCardStraight(BYTE iCardList[],int iCardCount) 
{
	if(iCardCount != 3)
	{
		return false ; 
	}

	BYTE temp[18]={0};

	for(int i= 0;i < iCardCount;i++)
	{
		temp[GetCardBulk(iCardList[i])]++;
	}

	for(int i=0;i<15;i++)
	{
		if(temp[i] != 0)//有值
		{
			for(int j = i;j < i +iCardCount ;j ++)
			{
				if(temp[j] != 1 || j >=15 )
					return false;
			}
			return true;
		}
	}
	return false;
}
//提取1,2,3 or 4张相同数字的牌
int CGameTaskLogic::GetCardNumCount(BYTE iCardList[], int iCardCount, BYTE bCardNum)
{
	if(bCardNum >MAX_CARD_TYPE + 2 ||bCardNum <3 )
	{
		return 0 ; 
	}

	int  temp[18] ;
	memset(temp , 0 ,sizeof(temp)) ; 

	for(int i = 0; i < iCardCount; i++)
	{
		temp[GetCardBulk(iCardList[i])]++;
	}

	return temp[bCardNum] ; 
}
///获取随机任务
void CGameTaskLogic::GetRandTask(int inDex) 
{
	if(TYPE_NONE  != m_byBackCardType)
	{
		return  ; 
	}
	srand((unsigned) time(NULL ) + inDex ) ;
	m_byTaskType = rand()%MAX_TASK_TYPE + 100; 

	if(TYPE_SOME_SHAPE == m_byTaskType)
	{
		m_bySpecifyShape = rand()%MAX_CARD_SHAPE ; 
	}
	else
	{
		m_bySpecifyCard = rand()%MAX_CARD_SHAPE + 3  ; 
	}
}
///设置游戏中的任务
void CGameTaskLogic::SetGameTask(GameTaskStruct &gameTask) 
{
	gameTask.byBackCardType = m_byBackCardType ; 
	gameTask.byTaskType = m_byTaskType ; 
	gameTask.bySpecifyShape = m_bySpecifyShape ;
	gameTask.bySpecifyCard = m_bySpecifyCard ; 
}
///设置底牌数据
void CGameTaskLogic::SetLastCardData(BYTE iCardList[] , BYTE iCardCount) 
{
	if(NULL == iCardList)
	{
		return ; 
	}
	if(0 >=iCardCount)
	{
		return ; 
	}

	if(TYPE_NONE != m_byBackCardType)  ///已经有底牌翻倍了
	{
		return ; 
	}

	m_byLastCardCount = iCardCount ; 

	memcpy(m_byLastCardList , iCardList , sizeof(BYTE)*(iCardCount)) ; 
	
	return ; 

}
///判断是否完成任务了
bool CGameTaskLogic::IsFinishTask(void) 
{
	if(TYPE_NONE != m_byBackCardType) ///底牌有翻倍
	{
		return  false; 
	}
	
	if(TYPE_LAST_NONE ==  m_byTaskType)  ///没有任务
	{
		return false ; 
	}

	if(0>m_bySpecifyShape||m_bySpecifyShape>=MAX_CARD_SHAPE)
	{
		return false ; 
	}

	switch(m_byTaskType)
	{
	case  TYPE_HAVE_A_CARD:
		{
			if(GetCardNumCount(m_byLastCardList , m_byLastCardCount ,m_bySpecifyCard)>0)
			{
				return true ; 
			}
			break;
		}
	case  TYPE_SOME_SHAPE:
		{
			int iShape = GetCardShape(m_byLastCardList , m_byLastCardCount) ;

			if(m_bySpecifyShape == 6)
			{
				if(iShape >= UG_THREE_ONE_SEQUENCE && iShape <=UG_THREE_TWO_SEQUENCE)
				{
					return true ; 
				}
				else
				{
					return false ; 
				}
			}
			else if(m_bySpecifyShape == 7)
			{
				if(iShape >=UG_BOMB)
				{
					return true ; 
				}
				else
				{
					return false ; 
				}
			}
			else
			{
				return (iShape == byCardShape[m_bySpecifyShape]) ; 
			}

			break;
		}
	case  TYPE_SINGLE_SOME_CARD:
	case  TYPE_DOUBLE_SOME_CARD:
		{
			if(m_byLastCardCount != (m_byTaskType - TYPE_SOME_SHAPE))
			{
				return false ;
			}

			int iCardCount = GetCardNumCount(m_byLastCardList , m_byLastCardCount , m_bySpecifyCard) ; 
			return (iCardCount == (m_byTaskType - TYPE_SOME_SHAPE)) ; 
			
		}
	default: 
		{
			break;
		}
	}

	return false ; 
}
///获取底牌倍数
int  CGameTaskLogic:: GetBackCardMytiple(void) 
{
	int  sGamePoint = 1;

	if( TYPE_NONE != m_byBackCardType)
	{
		if(m_byBackCardType == TYPE_ROCKET)
		{
			sGamePoint = 4 ; 
		}
		else if(m_byBackCardType == TYPE_DOUBLE_CARD
			||m_byBackCardType == TYPE_SMALL_KING
			||m_byBackCardType == TYPE_BIG_KING)
		{
			sGamePoint = 2; 
		}
		else
		{
			sGamePoint = 3 ;  //三倍
		}
	}
	else
	{
		return 1 ; 
	}

	return sGamePoint ; 
}

///百分比计算
int  CGameTaskLogic::GetTaskMutiple(bool bFinish) 
{
	if(false == bFinish)
	{
		return 100 ; 
	}

	int  sGamePoint = 100 ;

	if(m_byTaskType == TYPE_LAST_NONE)
	{
		sGamePoint = 100 ; 
	}
	else
	{
		if(m_byTaskType == TYPE_HAVE_A_CARD)
		{
			sGamePoint = 200 ; 
		}
		else
		{
			sGamePoint = 250 ; 
		}
	}

	return sGamePoint ; 
}





