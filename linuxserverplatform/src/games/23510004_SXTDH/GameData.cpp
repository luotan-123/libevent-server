#include "stdafx.h"
#include "GameData.h"
GameData::GameData(void)
{
	memset(m_byArHandPai, 255, sizeof(m_byArHandPai));	/// 手牌不超过20张，255为结束标志
	memset(m_byArOutPai, 255, sizeof(m_byArOutPai));		/// 出牌不超过160张，255为结束标志
	memset(m_byArHuaPai, 255, sizeof(m_byArHuaPai));		/// 花牌不超过10张，255为结束标志
	memset(m_iArMenPai, 255, sizeof(m_iArMenPai));		/// 门牌不超过40张，255为结束标志	
	memset(m_byCanOutCard, 255, sizeof(m_byCanOutCard));	/// 停牌或其他操作后能打出的牌
	memset(m_byMenFeng, 255, sizeof(m_byMenFeng));		/// 门风	
	memset(m_bHaveOut, 0, sizeof(m_bHaveOut));			///	记录玩家是否出过牌了
	memset(m_byArHuaPaiCount, 0, sizeof(m_byArHuaPaiCount));

	m_StrJing.Init();
	//初始化牌
	m_MenPai.Init();
	m_byQuanFeng = 255;			///圈风
	m_bySelectIndex = 255;		///当前选中排索引
	m_byLastOutPai = 255;		///最后打出的牌
	m_byEndZhuaPaiPalyer = 255;		///当前玩家最后抓拍
	m_byNowOutStation = 255;	///当前出牌位置
	m_byLastOutStation = 255;	///上次出牌位置
	///当前抓牌方向，true 顺时针，false 逆时针
	m_bZhuaPaiDir = true;
	InitData();
	LoadIni();
}
GameData::~GameData(void)
{
}
///初始化数据
void GameData::InitData()
{
	//记录杠牌状态，拥于杠开和杠后炮
	memset(m_bGangState, 0, sizeof(m_bGangState));
	///吃牌牌事件,临时数据
	temp_ChiPai.Init();
	///碰牌牌事件,临时数据
	temp_PengPai.Init();
	///杠牌牌事件,临时数据
	temp_GangPai.Init();
	///掷2颗色子确定起牌位置（点数和）和起牌点（最小点）事件
	T_TwoSeziDirAndGetPai.Init();
	///发牌事件
	//T_SendPai.Init();
	///所有玩家补花事件
	//T_AllBuHua.Init();
	///单个玩家补花事件
	//T_OneBuHua.Init();
	///出牌事件
	T_OutPai.Init();
	///抓牌牌事件
	T_ZhuaPai.Init();
	/////吃碰杠糊牌通知事件牌事件
	for (int i = 0;i < PLAY_COUNT;++i)
	{
		T_CPGNotify[i].Init();//清零
	}
	///吃牌牌事件
	T_ChiPai.Init();
	///碰牌牌事件
	T_PengPai.Init();
	///杠牌牌事件
	T_GangPai.Init();
	///听牌牌事件
	T_TingPai.Init();
	///糊牌牌事件
	T_HuPai.Init();
	///算分事件
	T_CountFen.Init();
	///开始出牌通知事件
	//T_BeginOutPai.Init();
	memset(m_byArHandPai, 255, sizeof(m_byArHandPai));			//手牌不超过20张，255为结束标志
	memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));	//手牌张数
	memset(m_byArOutPai, 255, sizeof(m_byArOutPai));				//出牌不超过160张，255为结束标志
	memset(m_byArOutPaiCount, 0, sizeof(m_byArOutPaiCount));		//出牌张数
	memset(m_byArHuaPai, 255, sizeof(m_byArHuaPai));				//花牌不超过10张，255为结束标志
	memset(m_iArMenPai, 255, sizeof(m_iArMenPai));				//门牌不超过40张，255为结束标志
	memset(m_byMenPaiData, 255, sizeof(m_byMenPaiData));	//转换后的门牌数据
	m_StrJing.Init();//初始化财神
	m_MenPai.Init();//初始化牌
	for (int i = 0;i < PLAY_COUNT;++i)
	{
		m_bHaveOut[i] = false;
		temp_HuPai[i].Init();
	}
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		for (int index = 0;index < 5;index++)
		{
			m_UserGCPData[i][index].Init();
		}
	}
	memset(m_bTing, 0, sizeof(m_bTing));				//玩家是否听牌
	memset(m_byTingType, 0, sizeof(m_byTingType));	//玩家听牌类型
	memset(m_byCanOutCard, 255, sizeof(m_byCanOutCard));///停牌或其他操作后能打出的牌
	memset(m_byMenFeng, 255, sizeof(m_byMenFeng));	///门风	
	memset(m_bHaveOut, 0, sizeof(m_bHaveOut));			///	记录玩家是否出过牌了
	memset(m_byArHuaPaiCount, 0, sizeof(m_byArHuaPaiCount));
	m_bySelectIndex = 255;				///当前选中排索引
	m_byLastOutPai = 255;				///最后打出的牌
	m_byEndZhuaPaiPalyer = 255;			///当前玩家最后抓拍
	m_byNowOutStation = 255;			///当前出牌位置
	m_byLastOutStation = 255;			///上次出牌位置
	///当前抓牌方向，true 顺时针，false 逆时针
	m_bZhuaPaiDir = true;

	memset(m_byRemainMenPai, 255, sizeof(m_byRemainMenPai));//剩余门牌
	m_byRemainMenPaiCount = 0;
	memset(m_byTingOutCardIndex, 255, sizeof(m_byTingOutCardIndex));
	//已经打出去的每张牌数量
	memset(m_byAllOutCardNum, 0, sizeof(m_byAllOutCardNum));
}
/*--------------------------------------------------------------------------*/
///服务器玩法配置
void GameData::LoadIni()
{
	//玩法属性
	//GameData::LoadIni()在CServerGameDesk::InitDeskGameStation()之前调用
	m_mjRule.byAllMjCount = 136;			//麻将的数量 
	m_mjRule.bHaveWan = 1;			//是否有万
	m_mjRule.bHaveTiao = 1;			//是否有条
	m_mjRule.bHaveBing = 1;			//是是有柄
	m_mjRule.bHaveHongZhong = 1;			//是否有红中
	m_mjRule.bHaveFaCai = 1;			//是否有发财
	m_mjRule.bHaveBaiBan = 1;			//是否有白板
	m_mjRule.bHaveFengPai = 1;			//是否有东南西北
	m_mjRule.bHaveFlower = 0;			//是否有花牌
	m_mjRule.bHaveJing = 0;			//有无财神
	m_mjRule.byMenPaiNum[0] = 34;			//各家门牌数
	m_mjRule.byMenPaiNum[1] = 34;			//各家门牌数
	m_mjRule.byMenPaiNum[2] = 34;			//各家门牌数
	m_mjRule.byMenPaiNum[3] = 34;			//各家门牌数
	m_mjRule.byAutoOutTime = 1;			//自动出牌时间	
	m_mjRule.byOutTime = 15;			//出牌时间		
	m_mjRule.byBlockTime = 15;			//拦牌思考时间	
	m_mjRule.bNetCutTuoGuan = 1;			//是否断线托管
	m_mjRule.bForceTuoGuan = 1;			//是否强退托管
	m_mjRule.byGamePassNum = 0;			//流局牌数
	m_mjRule.iGameBase = 1;			//基础分
	m_mjRule.bIsDuoXiang = 1;            //是否存在一炮多响
	m_mjRule.bIsSuanGangHu = 0;            //是否计算抢杠胡所得杠分
	m_mjRule.bChaoShiTuoGuan = 1;           //金币场超时托管
	m_mjRule.bDuanXianTuoGuan = 1;          //非房卡场，断线是否托管
#ifdef _RELEASEEX //运营版不配牌
	m_mjRule.bIsMatchMJ = 0;                //是否配牌
#else
	m_mjRule.bIsMatchMJ = 1;                //是否配牌
#endif // _RELEASEEX

	//动作属性
	m_mjAction.bChi = 0;		//吃
	m_mjAction.bChiJing = 0;        //吃癞子
	m_mjAction.bChiFeng = 0;		//吃风牌
	m_mjAction.bChiJian = 0;		//吃中发白
	m_mjAction.bPeng = 1;		//碰
	m_mjAction.bKan = 0;		//坎
	m_mjAction.bGang = 1;		//杠
	m_mjAction.bBuGang = 1;		//补杠
	m_mjAction.bGangAfterTing = 0;
	m_mjAction.bTing = 1;		//听
	m_mjAction.bQiangChi = 0;		//抢吃(吃后听牌)
	m_mjAction.bQxHh = 1;        //弃胡不能胡(一圈内)
	m_mjAction.bQiPeng = 1;        //弃碰不能碰(一圈内)
	m_mjAction.bQiangGangHu = 1;
	m_mjAction.bFangPao = 1;			//是否能点炮
	m_mjAction.bZiMo = 1;			//是否能自摸
	m_mjAction.bBuHua = 0;			//是否需要补花
}
/*--------------------------------------------------------------------------*/


///名称：IsHaveAHandPai
///描述：检查是否存在指定的手牌
///@param pai 要检测的牌
///@return true 有 false 无
bool GameData::IsHaveAHandPai(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return false;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] == pai)
			return true;
	}
	return false;
}

///名称：GetAHandPaiCount
///描述：检查手牌中某张牌的个数
///@param pai 要检测的牌
///@return 牌数量
int GameData::GetAHandPaiCount(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int count = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (pai != 255 && m_byArHandPai[station][i] == pai)
			count++;
	}
	return count;
}

///名称：GetHandPaiCount
///描述：检查手牌个数
///@param 
///@return 玩家手牌的总数
int GameData::GetHandPaiCount(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int count = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] != 255)
			count++;
	}
	return count;
}

///名称：CopyHandPai
///描述：拷贝玩家的手牌
///@param pai[] 牌数据, station 位置, show 是否拷贝明牌
///@return 玩家手牌总数
int GameData::CopyHandPai(BYTE pai[][HAND_CARD_NUM], BYTE station, bool show)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	//memset(pai,255,sizeof(pai));
	for (int i = 0;i < PLAY_COUNT;++i)
	{
		int count = GetHandPaiCount(station);
		if (i == station)//自己的牌
		{
			for (int j = 0;j < HAND_CARD_NUM;++j)
			{
				pai[i][j] = m_byArHandPai[station][j];
			}
		}
		else
		{
			if (!show)
			{
				for (int j = 0;j < HAND_CARD_NUM;++j)
				{
					if (m_byArHandPai[i][j] != 255)
					{
						pai[i][j] = 255;
					}
					else
					{
						pai[i][j] = m_byArHandPai[i][j];
					}
				}
			}
			else
			{
				for (int j = 0;j < HAND_CARD_NUM;++j)
				{
					pai[i][j] = m_byArHandPai[i][j];
				}
			}
		}
	}
	return 0;
}

///名称：CopyOneUserHandPai
///描述：拷贝某个玩家的手牌
///@param pai[] 牌数据, station 位置
///@return 玩家手牌总数
int GameData::CopyOneUserHandPai(BYTE pai[HAND_CARD_NUM], BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int count = 0;
	for (int j = 0;j < HAND_CARD_NUM;++j)
	{
		if (m_byArHandPai[station][j] != 255)
			count++;
		pai[j] = m_byArHandPai[station][j];
	}
	return count;
}

///名称：SortHandPai
///描述：手牌排序，small是否从小到大排
///@param big 是否从到到小排序。默认从小到大排序
void GameData::SortHandPai(BYTE station, bool big)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	///排序
	CLogicBase::SelectSort(m_byArHandPai[station], HAND_CARD_NUM, big);
	/*BYTE HandPai[HAND_CARD_NUM];
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
	HandPai[j] = m_byArHandPai[station][j];
	}
	int index=0;
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
	if(m_StrJing.CheckIsJing(HandPai[j]))
	{
	m_byArHandPai[station][index] = HandPai[j];
	index++;
	}
	}
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
	if(!m_StrJing.CheckIsJing(HandPai[j]) && index<HAND_CARD_NUM)
	{
	m_byArHandPai[station][index] = HandPai[j];
	index++;
	}
	}*/
}

///名称：AddToHandPai
///描述：添加一张牌到手牌中
///@param pai 添加的手牌值
///@return 
void GameData::AddToHandPai(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	if (pai == 255)
	{
		return;
	}
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] == 255)
		{
			m_byArHandPai[station][i] = pai;
			m_byArHandPaiCount[station]++;
			break;
		}
	}
}

///名称：SetHandPaiData
///描述：设置手牌数据
///@param  pai[] 传入的手牌数组
void GameData::SetHandPaiData(BYTE station, BYTE pai[], BYTE byCount)
{
	if (station < 0 || station >= PLAY_COUNT)
	{
		return;
	}
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		m_byArHandPai[station][i] = pai[i];
	}
	m_byArHandPaiCount[station] = byCount;
}

///名称：DeleteAHandPai
///描述：删除一张指定的手牌
///@param pai 要删除的牌
BYTE GameData::DeleteAHandPai(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT || !CMjRef::IsValidateMj(pai))
		return 255;
	BYTE byIndex = 255;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] == pai)
		{
			byIndex = i;
			m_byArHandPai[station][i] = 255;
			break;
		}
	}
	m_byArHandPaiCount[station]--;
	return byIndex;
	//CLogicBase::SelectSort(m_byArHandPai[station],HAND_CARD_NUM,false);
	//BYTE HandPai[HAND_CARD_NUM];
	//for(int j=0;j<HAND_CARD_NUM;++j)
	//{
	//	HandPai[j] = m_byArHandPai[station][j];
	//}
	//int index=0;
	//for(int j=0;j<HAND_CARD_NUM;++j)
	//{
	//	if(m_StrJing.CheckIsJing(HandPai[j]))
	//	{
	//		m_byArHandPai[station][index] = HandPai[j];
	//		index++;
	//	}
	//}
	//for(int j=0;j<HAND_CARD_NUM;++j)
	//{
	//	if(!m_StrJing.CheckIsJing(HandPai[j]) && 255!=HandPai[j])
	//	{
	//		m_byArHandPai[station][index] = HandPai[j];
	//		index++;
	//	}
	//}
	//m_byArHandPaiCount[station]=index;
}

///名称：SetHandPaiBack
///描述：设置手牌牌背
///@param num设置的数量
///@return 
void GameData::SetHandPaiBack(BYTE station, BYTE num)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	memset(m_byArHandPai[station], 255, sizeof(m_byArHandPai[station]));
	for (int i = 0;i < num;i++)
	{
		m_byArHandPai[station][i] = 0;
	}
}

///名称：GetLastHandPai
///描述：获得最后一张手牌
///@param 
BYTE GameData::GetLastHandPai(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 255;
	for (int i = HAND_CARD_NUM - 1;i >= 0;i--)
	{
		if (m_byArHandPai[station][i] != 255)
		{
			return m_byArHandPai[station][i];
		}
	}
	return 255;
}

///名称：GetSelectHandPai
///描述：获得选中的一张手牌
///@param 
///@return 
BYTE GameData::GetSelectHandPai(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 255;
	if (m_bySelectIndex < HAND_CARD_NUM && m_bySelectIndex != 255)
		return m_byArHandPai[station][m_bySelectIndex];
	return 255;
}

///名称：IsOutPaiPeople
///描述：是否出牌玩家
///@param 
///@return  true 是 ,false 不是
bool GameData::IsOutPaiPeople(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return false;
	if (GetHandPaiCount(station) % 3 == 2)
	{
		return true;
	}
	return false;
}

///名称：IsHuaPaiInHand
///描述：检测手牌中是否存在花牌
///@param 
///@return  true 是 ,false 不是
bool GameData::IsHuaPaiInHand(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return false;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (CheckIsHuaPai(m_byArHandPai[station][i]))
		{
			return true;
		}
	}
	return false;
}

///名称：MoveHuaPaiFormHand
///描述：将手牌中的花牌移到花牌数组中
///@param 
///@return  花牌的数量
int GameData::MoveHuaPaiFormHand(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int hua = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[station][i] == 255)
		{
			break;
		}
		if (CheckIsHuaPai(m_byArHandPai[station][i]))
		{
			hua++;
			AddToHuaPai(station, m_byArHandPai[station][i]);
			DeleteAHandPai(station, m_byArHandPai[station][i]);
		}
	}
	return hua;
}

///通过索引获取一张手牌
BYTE GameData::GetHandPaiFromIndex(BYTE station, int index)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 255;
	if (station < 0 || station>3)
		return 255;
	return m_byArHandPai[station][index];
}

///通过索引设置一张手牌
bool GameData::SetHandPaiFromIndex(BYTE station, int index, BYTE pai)
{
	if (station < 0 || station>3 || !CMjRef::IsValidateMj(pai))
		return false;
	if (m_byArHandPai[station][index] == 255)
		return false;
	m_byArHandPai[station][index] = pai;
	return true;
}

///索引是否有牌
bool GameData::IsHaveHandPaiFromIndex(BYTE station, int index)
{
	if (station < 0 || station>3 || index < 0 || index >= HAND_CARD_NUM)
	{
		return false;
	}
	if (m_byArHandPai[station][index] == 255)
	{
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///名称：GetUserGangNum
///描述：获取玩家杠牌的个数(暗杠，明杠，补杠)
///@param &an 暗杠数量, &ming 明杠数量 ,  &bu 补杠数量
///@return 杠牌的总个数
int GameData::GetUserGangNum(BYTE station, BYTE &an, BYTE &ming, BYTE &bu)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int all = 0;
	an = 0;
	ming = 0;
	bu = 0;
	for (int i = 0;i < 5;i++)
	{
		switch (m_UserGCPData[station][i].byType)
		{
		case ACTION_AN_GANG://暗杠
			an++;
			break;
		case ACTION_BU_GANG://补杠
			bu++;
			break;
		case ACTION_MING_GANG://明杠
			ming++;
			break;
		}
	}
	all = ming + an + bu;
	return all;
}

///名称：GetUserChiNum
///描述：玩家吃牌次数
///@param station 玩家位置
///@return 吃牌的总个数
int GameData::GetUserChiNum(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int num = 0;
	for (int i = 0;i < 5;i++)
	{
		switch (m_UserGCPData[station][i].byType)
		{
		case ACTION_CHI://吃牌
			num++;
			break;
		}
	}
	return num;
}

///名称：GetUserPengNum
///描述：玩家杠牌次数
///@param station 玩家位置
///@return 碰牌的总个数
int GameData::GetUserPengNum(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int num = 0;
	for (int i = 0;i < 5;i++)
	{
		switch (m_UserGCPData[station][i].byType)
		{
		case ACTION_PENG://碰牌
			num++;
			break;
		}
	}
	return num;
}

///名称：IsUserHaveGangPai
///描述：玩家杠了某个牌
///@param pai 牌, &type 类型
///@return  true 是 ,false 不是
bool GameData::IsUserHaveGangPai(BYTE station, BYTE pai, BYTE &type)
{
	if (station < 0 || station >= PLAY_COUNT)
		return false;
	for (int i = 0;i < 5;i++)
	{
		if (m_UserGCPData[station][i].byType >= ACTION_AN_GANG && m_UserGCPData[station][i].byType <= ACTION_MING_GANG && m_UserGCPData[station][i].byData[0] == pai)
		{
			type = m_UserGCPData[station][i].byType;
			return true;
		}
	}
	return false;
}
///
///名称：IsUserHavePengPai
///描述：玩家是否碰了某张牌
///@param pai 牌
///@return  true 是 ,false 不是
bool GameData::IsUserHavePengPai(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return false;
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == m_UserGCPData[station][i].byType)
			continue;
		if (m_UserGCPData[station][i].byType == ACTION_PENG &&  m_UserGCPData[station][i].byData[0] == pai)
		{
			return true;
		}
	}
	return false;
}

///名称：IsUserHaveChiPai
///描述：玩家是否吃过某张牌
///@param pai 牌
///@return  true 是 ,false 不是
bool GameData::IsUserHaveChiPai(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return false;
	for (int i = 0;i < 5;i++)
	{
		if (m_UserGCPData[station][i].byType == ACTION_CHI &&  m_UserGCPData[station][i].iOutpai == pai)
		{
			return true;
		}
	}
	return false;
}

///名称：AddToGCP
///描述：添加一组数据到杠吃碰数组中
///@param gcpData 要添加的吃碰杠数据
void GameData::AddToGCP(BYTE station, TCPGStruct *gcpData)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == m_UserGCPData[station][i].byType)
		{
			m_UserGCPData[station][i].byType = gcpData->byType;
			m_UserGCPData[station][i].iBeStation = gcpData->iBeStation;
			m_UserGCPData[station][i].iOutpai = gcpData->iOutpai;
			m_UserGCPData[station][i].iStation = gcpData->iStation;
			for (int j = 0;j < 4;j++)
			{
				m_UserGCPData[station][i].byData[j] = gcpData->byData[j];
			}
			break;
		}
	}
}

///名称：DeleteAGCPData
///描述：删除杠吃碰数组中指定的数据组
///@param type 类型, pai 牌
void GameData::DeleteAGCPData(BYTE station, BYTE type, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < 5;i++)
	{
		if (m_UserGCPData[station][i].byType == type && (m_UserGCPData[station][i].byData[0] == pai || m_UserGCPData[station][i].byData[1] == pai || m_UserGCPData[station][i].byData[2] == pai))
		{
			m_UserGCPData[station][i].Init();
			break;
		}
	}
	ShortGCP(station);//排序杠吃碰数组
}

///名称：CopyGCPData
///描述：拷贝杠吃碰数组中的数据组
///@param gcpData 吃碰杠数组
///@return 吃碰杠个数
int GameData::CopyGCPData(BYTE station, TCPGStruct gcpData[])
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	//memcpy(gcpData,m_UserGCPData,sizeof(m_UserGCPData));
	for (int i = 0;i < 5;i++)
	{
		gcpData[i] = m_UserGCPData[station][i];
	}
	return GetGCPCount(station);
}

///名称：SetGCPData
///描述：设置杠吃碰数组中的数据组
///@param gcpData 吃碰杠数组
///@return 
void GameData::SetGCPData(BYTE station, TCPGStruct gcpData[])
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < 5;i++)
	{
		m_UserGCPData[station][i] = gcpData[i];
	}
}

///名称：GetGCPCount
///描述：获取杠吃碰的节点个数
///@param 
///@return 获得吃碰杠总个数
int GameData::GetGCPCount(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int count = 0;
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO != m_UserGCPData[station][i].byType)
		{
			count++;
		}
	}
	return count;
}
///名称：GetOnePaiGCPCount
///描述：获取杠吃碰的中某种牌的数量
///@param 
///@return 某种牌的数量
int GameData::GetOnePaiGCPCount(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int count = 0;
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == m_UserGCPData[station][i].byType)
			continue;
		for (int j = 0;j < 4;j++)
		{
			if (m_UserGCPData[station][i].byData[j] == pai && pai != 255)
			{
				count++;
			}
		}
	}
	return count;
}
///名称：ShortGCP
///描述：排序杠吃碰数组
///@param 
void GameData::ShortGCP(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	int index = 0;
	TCPGStruct m_TempGCPData[5];
	for (int i = 0;i < 5;i++)
	{
		m_TempGCPData[i] = m_UserGCPData[station][i];
		m_UserGCPData[station][i].Init();
	}
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO != m_TempGCPData[i].byType)
		{
			m_UserGCPData[station][index] = m_TempGCPData[i];
			index++;
		}
	}
}
///名称：InitGCP
///描述：初始化杠吃碰数组
///@param 
void GameData::InitGCP(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < 5;i++)
	{
		m_UserGCPData[station][i].Init();
	}
}
//////////////////出牌操作///////////////////////////////////////////////////////////////////////////////

///名称：AddToOutPai
///描述：添加一张牌到出牌数组中
///@param pai 牌
void GameData::AddToOutPai(BYTE station, BYTE pai, bool bTingCard)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		if (m_byArOutPai[station][i] == 255)
		{
			if (bTingCard)
			{
				m_byTingOutCardIndex[station] = i;
			}
			else
			{
				m_byAllOutCardNum[pai] ++;
			}
			m_byArOutPai[station][i] = pai;
			break;
		}
	}
}
///名称：DeleteLastOutPai
///描述：删除最后一张出牌
///@param 
void GameData::DeleteLastOutPai(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		if (m_byArOutPai[station][i] == 255 && i > 0)
		{
			m_byArOutPai[station][i - 1] = 255;
			break;
		}
	}
}

///名称：SetOutPaiData
///描述：设置出牌数据
///@param pai[] 出牌数组
///@return 
void GameData::SetOutPaiData(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
	{
		return;
	}
	bool bAdd = false;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		if (255 == m_byArOutPai[station][i])
		{
			m_byArOutPai[station][i] = pai;
			m_byArOutPaiCount[station]++;
			break;
		}
	}
}

///名称：CopyOutPai
///描述：拷贝玩家的出牌
///@param pai[] 出牌数组
///@return 出牌的张数
int GameData::CopyOutPai(BYTE station, BYTE pai[])
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		pai[i] = m_byArOutPai[station][i];
	}
	return CLogicBase::GetPaiNum(m_byArOutPai[station], OUT_CARD_NUM);
}

///名称：GetOutPaiCount
///描述：获取玩家的出牌数量
///@param 
///@return 出牌的总数
int GameData::GetOutPaiCount(BYTE station)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int num = 0;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		if (m_byArOutPai[station][i] != 255)
		{
			num++;
		}
	}
	return num;
}
///名称：GetOutPaiCount
///描述：获取玩家某张牌的出牌数量
///@param 
///@return 某张牌出牌的数量
int GameData::GetOneOutPaiCount(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int num = 0;
	for (int i = 0;i < OUT_CARD_NUM;i++)
	{
		if (m_byArOutPai[station][i] != 255 && m_byArOutPai[station][i] == pai)
		{
			num++;
		}
	}
	return num;
}
/////////////////////花牌操作////////////////////////////////////////////////////////

///名称：AddToHuaPai
///描述：添加一张花牌到花牌数组中
///@param pai 花牌
void GameData::AddToHuaPai(BYTE station, BYTE pai)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < HUA_CARD_NUM;i++)
	{
		if (m_byArHuaPai[station][i] == 255)
		{
			m_byArHuaPai[station][i] = pai;
			m_byArHuaPaiCount[station]++;
			break;
		}
	}
}

///名称：SetHuaPaiData
///描述：设置花牌数据
///@param pai[] 花牌数组
///@return 
void GameData::SetHuaPaiData(BYTE station, BYTE pai[])
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	memcpy(m_byArHuaPai[station], pai, sizeof(m_byArHuaPai[station]));
}

///名称：CopyHuaPai
///描述：拷贝玩家的花牌
///@param pai[] 花牌数组
///@return 
int GameData::CopyHuaPai(BYTE station, BYTE pai[])
{
	if (station < 0 || station >= PLAY_COUNT)
		return 0;
	int count = 0;
	for (int i = 0;i < HUA_CARD_NUM;i++)
	{
		pai[i] = m_byArHuaPai[station][i];
		if (m_byArHuaPai[station][i] != 255)
		{
			count++;
		}
	}
	return count;
}

///检测是否花牌
bool GameData::CheckIsHuaPai(BYTE pai)
{
	if (pai >= CMjEnum::MJ_TYPE_FCHUN && pai != 255)
		return true;
	return false;
}
///////////////////////门牌操作//////////////////////////////////////////////////////////////////

///名称：SetMenPaiData
///描述：设置门牌数据
///@param  pai[] 门牌数组
///@return 
void GameData::SetMenPaiData(BYTE station, BYTE pai[])
{
	if (station < 0 || station >= 4)
		return;
	memcpy(m_iArMenPai[station], pai, sizeof(m_iArMenPai[station]));
}
///名称：CopyMenPai
///描述：拷贝玩家的门牌
///@param  pai[][] 门牌数组
///@return 玩家门牌总数
void GameData::CopyAllMenPai(BYTE pai[][MEN_CARD_NUM])
{
	for (int i = 0;i < 4;i++)
	{
		for (int j = 0;j < MEN_CARD_NUM;j++)
		{
			pai[i][j] = m_iArMenPai[i][j];
		}
	}
}
///名称：CopyMenPai
///描述：拷贝玩家的门牌
///@param  pai[] 门牌数组
///@return 玩家门牌总数
int GameData::CopyMenPai(BYTE station, BYTE pai[])
{
	if (station < 0 || station >= 4)
		return 0;
	for (int i = 0;i < MEN_CARD_NUM;i++)
	{
		pai[i] = m_iArMenPai[station][i];
	}
	return CLogicBase::GetPaiNum(m_iArMenPai[station], MEN_CARD_NUM);
}

///名称：ChangeMenPai
///描述：门牌转换
///@param  pai[] 门牌数组
///@return 玩家门牌总数
void GameData::ChangeMenPai(int iIndex, BYTE byCard)
{
	int iCount_ = 0, i = 0;
	for (i = 0; i < 4; i++)
	{
		iCount_ += m_mjRule.byMenPaiNum[i];
		if (iIndex - iCount_ < 0)
		{
			break;
		}
	}
	m_iArMenPai[i][m_mjRule.byMenPaiNum[i] - (iCount_ - iIndex)] = byCard;
}

//描述：创建门牌
void GameData::CreateMenPai()
{
	int iStart = 0;
	int iEnd = m_mjRule.byMenPaiNum[0];
	int index = 0;
	for (int i = iStart;i < iEnd;++i)//0位置的门牌
	{
		if (m_MenPai.byMenPai[i] != 255)
		{
			m_iArMenPai[0][index++] = 0;
		}
		else
		{
			m_iArMenPai[0][index++] = 255;
		}
	}
	iStart = m_mjRule.byMenPaiNum[0];
	iEnd = m_mjRule.byMenPaiNum[0] + m_mjRule.byMenPaiNum[1];
	index = 0;
	for (int i = iStart;i < iEnd;++i)//1位置的门牌
	{
		if (m_MenPai.byMenPai[i] != 255)
		{
			m_iArMenPai[1][index++] = 0;
		}
		else
		{
			m_iArMenPai[1][index++] = 255;
		}
	}
	iStart = m_mjRule.byMenPaiNum[0] + m_mjRule.byMenPaiNum[1];
	iEnd = m_mjRule.byMenPaiNum[0] + m_mjRule.byMenPaiNum[1] + m_mjRule.byMenPaiNum[2];
	index = 0;
	for (int i = iStart;i < iEnd;++i)//2位置的门牌
	{
		if (m_MenPai.byMenPai[i] != 255)
		{
			m_iArMenPai[2][index++] = 0;
		}
		else
		{
			m_iArMenPai[2][index++] = 255;
		}
	}
	iStart = m_mjRule.byMenPaiNum[0] + m_mjRule.byMenPaiNum[1] + m_mjRule.byMenPaiNum[2];
	iEnd = m_mjRule.byMenPaiNum[0] + m_mjRule.byMenPaiNum[1] + m_mjRule.byMenPaiNum[2] + m_mjRule.byMenPaiNum[3];
	index = 0;
	for (int i = iStart;i < iEnd;++i)//3位置的门牌
	{
		if (m_MenPai.byMenPai[i] != 255)
		{
			m_iArMenPai[3][index++] = 0;
		}
		else
		{
			m_iArMenPai[3][index++] = 255;
		}
	}
}

///描述：获取某玩家的门牌数量
int GameData::GetMenPaiCount(BYTE station)
{
	if (station < 0 || station >= 4)
		return false;
	int count = 0;
	for (int i = 0;i < MEN_CARD_NUM;++i)
	{
		if (m_iArMenPai[station][i] != 255)
		{
			count++;
		}
	}
	return count;
}

///名称：ChangeAllHandPai
///换所有手牌
///@param  station 位置, pai[] 牌, count牌数量
void GameData::ChangeAllHandPai(BYTE station, BYTE pai[], BYTE count)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < count;i++)
	{
		if (!CMjRef::IsValidateMj(pai[i]))
			continue;
		if (m_byArHandPai[station][i] == 255)
			break;
		m_byArHandPai[station][i] = pai[i];
	}
}

///名称：ChangeAllHandPai
///换指定手牌
///@param  station 位置, pai[] 牌, handpai[]要换掉的手牌, count牌数量
void GameData::ChangeSomeHandPai(BYTE station, BYTE pai[], BYTE handpai[], BYTE count)
{
	if (station < 0 || station >= PLAY_COUNT)
		return;
	for (int i = 0;i < count;i++)
	{
		if (!CMjRef::IsValidateMj(pai[i]) || !CMjRef::IsValidateMj(handpai[i]))
			continue;
		if (IsHaveAHandPai(station, handpai[i]))
		{
			DeleteAHandPai(station, handpai[i]);
			AddToHandPai(station, pai[i]);
		}
	}
}

///名称：SetGameNextPai
///设置牌墙的下一张牌
///@param  pai 牌
void GameData::SetGameNextPai(BYTE pai)
{
	if (m_MenPai.byStartIndex == 255)
		return;
	if (m_MenPai.byMenPai[m_MenPai.byStartIndex] == 255)
		return;
	m_MenPai.byMenPai[m_MenPai.byStartIndex] = pai;
}
