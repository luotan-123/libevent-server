#include "stdafx.h"
#include "CheckHuPai.h"

CheckHuPai::CheckHuPai(void)
{
	InitData();
}

CheckHuPai::~CheckHuPai(void)
{

}
///名称：InitData
///描述：初始化数据
///@param 
///@return 
void CheckHuPai::InitData()
{
	///玩家是否自摸胡牌
	m_bZimo = false;
	m_byStation = 255;///当前检测玩家的位置
	m_byBeStation = 255;
	m_tHuPaiStruct.Init();
	m_tHuTempData.Init();	//糊牌检测临时数据
	m_tNoJingHuTempData.Init();///糊牌检测临时数据(无财神)	
	m_byJingNum = 0;  //精牌总数
	//最后自摸或点炮的牌
	m_byLastPai = 255;
	ClearPingHuData();
	memset(m_byTypeFans, 0, sizeof(m_byTypeFans));
	m_iMaxFan = -1;
}
/*------------------------------------------------------------------------------*/
///名称：ChangeHandPaiData
///描述：转换手牌数据的存储方式，为了方便糊牌检测
///@param handpai[] 手牌数组, pai 别人打出的牌
///@return 
void CheckHuPai::ChangeHandPaiData(GameDataEx *pGameData, BYTE handpai[])
{
	m_tHuTempData.Init();
	m_tNoJingHuTempData.Init();///糊牌检测临时数据(无财神)	
	m_byJingNum = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (handpai[i] != 255)
		{
			if (!pGameData->m_StrJing.CheckIsJing(handpai[i]))//财神牌
			{
				m_tNoJingHuTempData.Add(handpai[i]);
			}
			else
			{
				m_byJingNum++;
			}
			m_tHuTempData.Add(handpai[i]);
		}
	}
}


///名称：ChangeHandPaiData
///描述：转换手牌数据的存储方式，为了方便糊牌检测
///@param handpai[] 手牌数组, pai 别人打出的牌
///@return 
void CheckHuPai::ChangeHandPaiDataEx(GameDataEx *pGameData, BYTE handpai[])
{
	m_tHuTempData.Init();
	m_tNoJingHuTempData.Init();///糊牌检测临时数据(无财神)	
	m_byJingNum = 0;
	int iAddLaiZiToHand = 0;		//加癞子到手牌中
	bool bAddLaiZiFlag = false;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (handpai[i] == 255)
			continue;
		//  非财神		
		if ((iAddLaiZiToHand == 0 &&							//未增加 
			m_bZimo == false &&								//非自摸
			pGameData->m_StrJing.CheckIsJing(handpai[i]) &&//癞子
			handpai[i] == m_byLastPai))
		{
			bAddLaiZiFlag = true;
		}

		if (!pGameData->m_StrJing.CheckIsJing(handpai[i]) || bAddLaiZiFlag)//财神牌
		{
			if (bAddLaiZiFlag)
			{
				iAddLaiZiToHand = 1;
				bAddLaiZiFlag = false;
			}
			m_tNoJingHuTempData.Add(handpai[i]);
		}
		else
		{
			m_byJingNum++;
		}
		m_tHuTempData.Add(handpai[i]);
	}
}

///名称：AddAPaiToArr
///描述：添加一张牌到临时数组中
///@param pai 要加的牌
///@return 
void CheckHuPai::AddAPaiToArr(BYTE pai)
{
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_tHuTempData.data[i][0] == 255)
		{
			m_tHuTempData.data[i][0] = pai;
			m_tHuTempData.data[i][1] = 1;
			m_tHuTempData.count++;
			break;
		}
		else if (m_tHuTempData.data[i][0] == pai)
		{
			m_tHuTempData.data[i][1]++;
			break;
		}
	}
}

///名称：SetJingPai
///描述：设置精牌，减掉精牌值，增加精牌数量
///@param 
///@return 
void CheckHuPai::SetJingPai()
{
}

///名称：ReSetJingPai
///描述：恢复精牌，将精牌恢复到临时数组中
///@param 
///@return 
void CheckHuPai::ReSetJingPai()
{
}

///名称：CanTing
///描述：检测能否听牌
///@param  handpai[] 手牌数组, pai 最后一张牌, gcpData[] 吃碰杠数组, MainJing 正精, LessJing 副精, CanOut[] 打出后能听牌的牌（可以有多张）
///@return true 能听，false 不能听
bool CheckHuPai::CanTing(GameDataEx *pGameData, BYTE station, BYTE CanOut[])
{
	if (!pGameData->m_mjAction.bTing)//不能听牌
	{
		return false;
	}
	m_byStation = station;
	///玩家是否自摸胡牌
	m_bZimo = true;
	memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
	memset(CanOut, 255, sizeof(CanOut));
	int num = 0;

	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		m_byArHandPai[i] = pGameData->m_byArHandPai[station][i];
	}
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255)
			break;
		BYTE pai = 255;
		pai = m_byArHandPai[i];
		m_byArHandPai[i] = 255;
		//从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
		///牌数据转换
		ChangeHandPaiData(pGameData, m_byArHandPai);

		if (CheckPingHu(pGameData))
		{
			CanOut[num] = pai;
			num++;
		}
		m_byArHandPai[HAND_CARD_NUM - 1] = pai;

	}
	if (num > 0)
		return true;

	return false;
}
///名称：ChiTing
///检测能否吃听牌：吃后听牌
///@param  station 吃听位置, CanOut[] 打出后能听牌的牌（每张吃牌只保存一张），paip[][3] 能吃的牌组合
///@return true 能听，false 不能听
bool CheckHuPai::ChiTing(GameDataEx *pGameData, BYTE station, BYTE CanOut[], BYTE pai[][3])
{
	m_byStation = station;

	memset(CanOut, 255, sizeof(CanOut));

	int num = 0, num1 = 0;
	for (int i = 0;i < 3;i++)
	{
		if (pai[i][0] != 255)
		{
			num++;
		}
	}

	for (int i = 0;i < num;i++)
	{
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
		for (int j = 0;j < HAND_CARD_NUM;j++)
		{
			m_byArHandPai[j] = pGameData->m_byArHandPai[station][j];
		}
		///删除吃牌

		///从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);

		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (m_byArHandPai[i] == 255 || CanOut[num] != 255)
				continue;
			BYTE pai = 255;
			pai = m_byArHandPai[i];
			m_byArHandPai[i] = 255;
			//从小到大排序
			CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
			///牌数据转换
			ChangeHandPaiData(pGameData, m_byArHandPai);

			if (CheckPingHu(pGameData))
			{
				CanOut[num] = pai;
				num1++;
			}
			m_byArHandPai[HAND_CARD_NUM - 1] = pai;
		}
	}
	if (num1 > 0)
		return true;
	return false;
}
///名称：GangTing
///检测杠后能否保存听牌状态
///@param  station 杠听位置, CanTing[] 对应的组合杠后能否保存听，paip[][4] 杠牌的组合
///@return true 能听，false 不能听
bool CheckHuPai::GangTing(GameDataEx *pGameData, BYTE station, tagCPGNotify &cpg, BYTE ps, bool CanTing[])
{
	m_byStation = station;

	memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
	memset(CanTing, 0, sizeof(CanTing));
	int num = 0, num1 = 0;
	for (int i = 0;i < 4;i++)
	{
		if (cpg.iGangData[i][1] != 255)
			num++;
	}
	for (int i = 0;i < num;i++)
	{
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
		for (int j = 0;j < HAND_CARD_NUM;j++)
		{
			m_byArHandPai[j] = pGameData->m_byArHandPai[station][j];
		}
		///删除杠牌

		///从小到大排序
		CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);

		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (m_byArHandPai[i] == 255 || CanTing[num])
				continue;
			BYTE pai = 255;
			pai = m_byArHandPai[i];
			m_byArHandPai[i] = 255;
			//从小到大排序
			CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
			///牌数据转换
			ChangeHandPaiData(pGameData, m_byArHandPai);

			if (CheckPingHu(pGameData))
			{
				CanTing[num1] = true;
				num1++;
			}
			m_byArHandPai[HAND_CARD_NUM - 1] = pai;
		}
	}
	if (num1 > 0)
		return true;

	return false;
}
///名称：CanHu
///描述：详细检测糊牌
///@param station 糊牌检测位置，hutype[] 能糊牌的类型，zimo 是否自摸
///@return true  能糊， false 不能
bool CheckHuPai::CanHu(GameDataEx *pGameData, BYTE station, BYTE lastpai, bool zimo)
{
	//最后自摸或点炮的牌
	m_byLastPai = lastpai;
	m_byStation = station;
	memset(m_byArHandPai, 255, sizeof(m_byArHandPai));
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		m_byArHandPai[i] = pGameData->m_byArHandPai[station][i];
		if (pGameData->m_byArHandPai[station][i] == 255 && lastpai != 255 && !zimo)
		{
			m_byArHandPai[i] = lastpai;
			break;
		}
	}
	///从小到大排序
	CLogicBase::SelectSort(m_byArHandPai, HAND_CARD_NUM, false);
	bool bb = false;
	///牌数据转换
	ChangeHandPaiData(pGameData, m_byArHandPai);

	if (CheckPingHu(pGameData))
	{
		bb = true;
		//检测上金等
		///添加吃碰杠数据
		//AddGCPData(pGameData->m_UserGCPData[station],&m_tHuPaiStruct );
		//if(zimo)
		//{
		//	SetAHupaiType(HUPAI_TYPE_ZiMo,hutype);///自摸
		//}
		//CheckPingHuData();
	}
	return bb;
}

///名称：AddGCPData
///描述：将吃碰杠数据加入到糊牌结构中
///@param gcpData[] 玩家刚吃碰数组 , tempstruct  糊牌数据结构
///@return 
void CheckHuPai::AddGCPData(TCPGStruct gcpData[], PingHuStruct * tempstruct)//m_tHuPaiStruct
{
	BYTE pai[4];
	bool jing[4];
	memset(jing, 0, sizeof(jing));
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == gcpData[i].byType)
			continue;
		for (int j = 0;j < 4;j++)
		{
			pai[j] = gcpData[i].byData[j];
		}
		tempstruct->AddData(gcpData[i].byType, pai, jing);
	}
}

///名称：CheckPingHu
///描述：平糊检测
///@param  jing 是否带精, FengShunZi 风牌能否组顺子, JianShunZi 箭牌是否能组顺子, AddJing 额外的一张精（听牌时用）
///@return true 能组成平糊， false 不能组成平糊
//原理:删牌顺序将牌->刻子->顺子
bool CheckHuPai::CheckPingHu(GameDataEx *pGameData)
{
	//// 统计消耗
	//LARGE_INTEGER BegainTime ;     
	//LARGE_INTEGER EndTime ;     
	//LARGE_INTEGER Frequency ;     
	//QueryPerformanceFrequency(&Frequency);     
	//QueryPerformanceCounter(&BegainTime) ;

	//初始化话相应数据
	m_tTempHupaistruct.Init();
	m_tHuPaiStruct.Init();
	m_iMaxFan = -1;
	BYTE pai[4] = { 255 };
	bool caishen[4] = { 0 };
	memset(pai, 255, sizeof(pai));
	memset(caishen, 0, sizeof(caishen));
	//拥有两个以上的财神牌，选两个财神做将牌
	if (m_byJingNum >= 2 && m_byJingNum != 255)//
	{
		m_tHuPaiStruct.Init();//初始化糊牌结构
		memset(caishen, 0, sizeof(caishen));
		memset(pai, 255, sizeof(pai));
		pai[0] = pGameData->m_StrJing.byPai[0];
		pai[1] = pGameData->m_StrJing.byPai[0];
		caishen[0] = true;
		caishen[1] = true;
		m_tHuPaiStruct.AddData(TYPE_JINGDIAO_JIANG, pai, caishen);//添加将头
		MakePingHu(pGameData, m_tNoJingHuTempData, m_tHuPaiStruct, m_byJingNum - 2);//胡牌检测递归函数：在递归中完成全部检测
	}
	//1个财神的检测,一个财神和一张牌做将
	if (m_byJingNum >= 1 && m_byJingNum != 255)
	{
		for (int i = 0;i < m_tNoJingHuTempData.count;i++)
		{
			if (m_tNoJingHuTempData.data[i][1] < 1 || m_tNoJingHuTempData.data[i][1] == 255 || m_tNoJingHuTempData.data[i][0] == 255)
				continue;
			m_tHuPaiStruct.Init();//初始化糊牌结构
			m_tNoJingHuTempData.data[i][1] -= 1;
			memset(caishen, 0, sizeof(caishen));
			memset(pai, 255, sizeof(pai));
			pai[0] = m_tNoJingHuTempData.data[i][0];
			pai[1] = m_tNoJingHuTempData.data[i][0];
			caishen[0] = true;
			m_tHuPaiStruct.AddData(TYPE_JIANG_PAI, pai, caishen);
			MakePingHu(pGameData, m_tNoJingHuTempData, m_tHuPaiStruct, m_byJingNum - 1);//胡牌检测递归函数：在递归中完成全部检测
			m_tNoJingHuTempData.data[i][1] += 1;
		}
	}
	//正常牌做的检测,(无精的麻将m_tHuTempData 与 m_tNoJingHuTempData是一样的数据)
	for (int i = 0;i < m_tNoJingHuTempData.count;i++)
	{
		if (m_tNoJingHuTempData.data[i][1] < 2 || m_tNoJingHuTempData.data[i][0] == 255)
			continue;
		m_tHuPaiStruct.Init();//初始化糊牌结构
		m_tNoJingHuTempData.data[i][1] -= 2;
		memset(caishen, 0, sizeof(caishen));
		memset(pai, 255, sizeof(pai));
		pai[0] = m_tNoJingHuTempData.data[i][0];
		pai[1] = m_tNoJingHuTempData.data[i][0];
		m_tHuPaiStruct.AddData(TYPE_JIANG_PAI, pai, caishen);
		MakePingHu(pGameData, m_tNoJingHuTempData, m_tHuPaiStruct, m_byJingNum);//胡牌检测递归函数：在递归中完成全部检测
		m_tNoJingHuTempData.data[i][1] += 2;
	}

	////输出运行时间
	//QueryPerformanceCounter(&EndTime);
	//GameLog::OutputFile("==CheckPingHu耗时：%.1f==",(double)( EndTime.QuadPart - BegainTime.QuadPart )/ Frequency.QuadPart * 1000000);

	if (m_iMaxFan >= 0)
	{
		m_tHuPaiStruct = m_tTempHupaistruct;
		return true;
	}
	return false;
}

///平胡的情况下：获取最大的组合牌型
bool CheckHuPai::GetHuPaiTypes(GameDataEx *pGameData, bool zimo)
{
	//根据不同的麻将，翻数可能相乘或者相加
	//相乘番数初始化为1，相加1
	int fan = 0;
	BYTE byTempHuType[MAX_HUPAI_TYPE];
	ClearHupaiType(byTempHuType);
	bool bDaHu = false;
	CheckPingHuData(pGameData);//先获取牌组合

	if (CheckQingYiSe(pGameData))//清一色
	{
		bDaHu = true;
		fan = 9;
		SetAHupaiType(HUPAI_TYPE_QingYiSe, byTempHuType);
	}
	else if (CheckYiTiaoLong())//一条龙
	{
		bDaHu = true;
		fan = 9;
		SetAHupaiType(MJ_HUPAI_TYPE_YI_TIAO_LONG, byTempHuType);
	}

	if (!bDaHu) //平胡，鸡胡
	{
		SetAHupaiType(HUPAI_TYPE_PingHu, byTempHuType);
		fan = 1;
	}

	if (fan > m_iMaxFan)
	{
		m_iMaxFan = fan;
		m_tTempHupaistruct = m_tHuPaiStruct;
		memcpy(m_byHuType, byTempHuType, sizeof(m_byHuType));
	}
	return true;
}

//描述：平糊组牌
//@param csnum 精牌数, m_tHuPaiStruct 糊牌结构, FengShunZi 风牌能否组顺子, JianShunZi 箭牌是否能组顺子
//@return true 能组成平糊， false 不能组成平糊
bool CheckHuPai::MakePingHu(GameDataEx *pGameData, CheckHuStruct &PaiData, PingHuStruct &HuPaiStruct, int iJingnum)
{
	if (PaiData.GetAllPaiCount() <= 0)//已经糊牌
	{
		//检测牌型，记录最大牌型
		return GetHuPaiTypes(pGameData, m_bZimo);
	}
	if (iJingnum > HAND_CARD_NUM)
	{
		iJingnum = 0;
	}
	//取牌
	BYTE pai = 255;
	int count = 0;
	for (int i = 0;i < PaiData.count;i++)
	{
		if (0 == PaiData.data[i][1])
		{
			continue;
		}
		pai = PaiData.data[i][0];   //记录牌值
		count = PaiData.data[i][1]; //记录牌数
		break;
	}
	BYTE data[4];
	bool caishen[4];
	memset(data, 255, sizeof(data));
	memset(caishen, 0, sizeof(caishen));
	data[0] = pai;
	data[1] = pai;
	data[2] = pai;
	//先删除刻子再删除顺子
	if (iJingnum >= 2)//一张牌和 2财神
	{
		caishen[0] = true;
		caishen[1] = true;
		HuPaiStruct.AddData(TYPE_AN_KE, data, caishen);//添加组牌
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 2);//递归
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_AN_KE, data, caishen);//恢复组牌
	}
	if (iJingnum >= 1 && count >= 2)//一张财神和两张牌组刻子
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[0] = true;
		HuPaiStruct.AddData(TYPE_AN_KE, data, caishen);//添加组牌
		PaiData.SetPaiCount(pai, count - 2);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 1);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_AN_KE, data, caishen);//恢复组牌
	}

	if (count >= 3)//三张牌组刻子
	{
		memset(caishen, 0, sizeof(caishen));
		HuPaiStruct.AddData(TYPE_AN_KE, data, caishen);//添加组牌
		PaiData.SetPaiCount(pai, count - 3);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_AN_KE, data, caishen);//恢复组牌
	}
	if (pai >= CMjEnum::MJ_TYPE_FD)//字牌，只考虑刻子
		return false;
	//万同条：考虑顺子
	if (iJingnum >= 2 && CMjRef::GetCardPaiDian(pai) >= 3)//组成 xx+pai 的顺子（x是财神）
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[0] = true;
		caishen[1] = true;
		data[0] = pai - 2;
		data[1] = pai - 1;
		data[2] = pai;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 2);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌
	}

	if (iJingnum >= 1 && CMjRef::GetCardPaiDian(pai) >= 2 && CMjRef::GetCardPaiDian(pai) < 9 && PaiData.GetPaiCount(pai + 1) >= 1)//组成 x+pai+(pai+1) 的顺子（x是财神）
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[0] = true;
		data[0] = pai - 1;
		data[1] = pai;
		data[2] = pai + 1;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		PaiData.SetPaiCount(pai + 1, PaiData.GetPaiCount(pai + 1) - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 1);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		PaiData.SetPaiCount(pai + 1, PaiData.GetPaiCount(pai + 1) + 1);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌
	}

	if (iJingnum >= 2 && CMjRef::GetCardPaiDian(pai) >= 2 && CMjRef::GetCardPaiDian(pai) < 9)//组成 x+pai+(x) 的顺子（x是财神）
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[0] = true;
		caishen[2] = true;
		data[0] = pai - 1;
		data[1] = pai;
		data[2] = pai + 1;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 2);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌
	}

	if (CMjRef::GetCardPaiDian(pai) < 8 && PaiData.GetPaiCount(pai + 1) >= 1 && PaiData.GetPaiCount(pai + 2) >= 1)//组成 pai+(pai+1)+(pai+2) 的顺子
	{
		memset(caishen, 0, sizeof(caishen));
		data[0] = pai;
		data[1] = pai + 1;
		data[2] = pai + 2;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		PaiData.SetPaiCount(pai + 1, PaiData.GetPaiCount(pai + 1) - 1);//设置牌
		PaiData.SetPaiCount(pai + 2, PaiData.GetPaiCount(pai + 2) - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		PaiData.SetPaiCount(pai + 1, PaiData.GetPaiCount(pai + 1) + 1);//设置牌
		PaiData.SetPaiCount(pai + 2, PaiData.GetPaiCount(pai + 2) + 1);//设置牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌	
	}

	if (iJingnum >= 1 && CMjRef::GetCardPaiDian(pai) < 8 && PaiData.GetPaiCount(pai + 1) < 1 && PaiData.GetPaiCount(pai + 2) >= 1)//组成 pai+(x)+(pai+2) 的顺子
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[1] = true;
		data[0] = pai;
		data[1] = pai + 1;
		data[2] = pai + 2;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		PaiData.SetPaiCount(pai + 2, PaiData.GetPaiCount(pai + 2) - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 1);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		PaiData.SetPaiCount(pai + 2, PaiData.GetPaiCount(pai + 2) + 1);//设置牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌	
	}

	if (iJingnum >= 1 && CMjRef::GetCardPaiDian(pai) < 8 && PaiData.GetPaiCount(pai + 1) >= 1 && PaiData.GetPaiCount(pai + 2) < 1)//组成 pai+(pai+1)+(x) 的顺子
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[2] = true;
		data[0] = pai;
		data[1] = pai + 1;
		data[2] = pai + 2;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		PaiData.SetPaiCount(pai + 1, PaiData.GetPaiCount(pai + 1) - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 1);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		PaiData.SetPaiCount(pai + 1, PaiData.GetPaiCount(pai + 1) + 1);//设置牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌	
	}

	if (iJingnum >= 2 && CMjRef::GetCardPaiDian(pai) < 8)//组成 pai+(x)+(x) 的顺子
	{
		memset(caishen, 0, sizeof(caishen));
		caishen[1] = true;
		caishen[2] = true;
		data[0] = pai;
		data[1] = pai + 1;
		data[2] = pai + 2;
		HuPaiStruct.AddData(TYPE_SHUN_ZI, data, caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai, count - 1);//设置牌
		MakePingHu(pGameData, PaiData, HuPaiStruct, iJingnum - 2);
		PaiData.SetPaiCount(pai, count);//检测完毕恢复牌
		HuPaiStruct.DeleteData(TYPE_SHUN_ZI, data, caishen);//恢复组牌	
	}
	return false;
}

///描述：恢复牌数,把减掉的牌还原
///@param num[] 临时保存的原牌数据
///@return 
void CheckHuPai::ReSetPaiNum(BYTE num[], CheckHuStruct &TempPai)
{
	for (int i = 0;i < TempPai.count;i++)
	{
		TempPai.data[i][1] = num[i];
	}
}

///描述：检测连牌是否同一花色
///@param pai1, pai2 要检测的两张牌
///@return true 是， false 不是
bool CheckHuPai::CheckTongSe(BYTE pai1, BYTE pai2)
{
	int count = 0;
	if (pai2 % 10 == 0 || pai1 / 10 != pai2 / 10)
		return false;
	return true;
}

///名称：CheckLianPai
///描述：检测连牌是否连张,num 几连张，1就是上下张，2就是上下连张
///@param pai1, pai2 要检测的牌， num 连张相隔数
///@return true 是连张，flse 不是连张
bool CheckHuPai::CheckLianPai(GameDataEx *pGameData, BYTE pai1, BYTE pai2, BYTE num)
{
	if (!CheckTongSe(pai1, pai2))
		return false;
	if (num == 1)
	{
		if (pai1 == pai2 + 1 || pai1 == pai2 - 1)
		{
			return true;
		}
	}
	else if (num == 2)
	{
		if (pai1 == pai2 + 1 || pai1 == pai2 - 1 || pai1 == pai2 + 2 || pai1 == pai2 - 2)
		{
			if (!pGameData->m_mjAction.bChiFeng && (pai1 >= CMjEnum::MJ_TYPE_FD && pai1 <= CMjEnum::MJ_TYPE_FB || pai2 >= CMjEnum::MJ_TYPE_FD && pai2 <= CMjEnum::MJ_TYPE_FB))
				return false;
			if (!pGameData->m_mjAction.bChiJian && (pai1 >= CMjEnum::MJ_TYPE_ZHONG && pai1 <= CMjEnum::MJ_TYPE_BAI || pai2 >= CMjEnum::MJ_TYPE_ZHONG && pai2 <= CMjEnum::MJ_TYPE_BAI))
				return false;
			if (pai2 >= CMjEnum::MJ_TYPE_FD && pai2 <= CMjEnum::MJ_TYPE_FB)///东南西北
			{
				if (pai1 > CMjEnum::MJ_TYPE_FB || pai1 < CMjEnum::MJ_TYPE_FD)
					return false;
			}
			else if (pai2 >= CMjEnum::MJ_TYPE_ZHONG && pai2 <= CMjEnum::MJ_TYPE_BAI)///中发白
			{
				if (pai1 > CMjEnum::MJ_TYPE_BAI || pai1 < CMjEnum::MJ_TYPE_ZHONG)
					return false;
			}
			return true;
		}
	}
	return false;
}

///名称：SetAHupaiType
///描述：添加一种糊牌类型
///@param   type 要添加的类型 , hupai[] 糊牌类型数组
///@return 
void CheckHuPai::SetAHupaiType(BYTE type, BYTE hupai[])
{
	if (CheckHupaiType(type, hupai))//有了就不添加了
		return;
	for (int i = 0;i < MAX_HUPAI_TYPE;i++)
	{
		if (hupai[i] == 255)
		{
			hupai[i] = type;
			break;
		}
	}
}

///名称：ReSetAHupaiType
///描述：撤销一种糊牌类型
///@param type 要删除的类型 , hupai[] 糊牌类型数组
///@return 
void CheckHuPai::ReSetAHupaiType(BYTE type, BYTE hupai[])
{
	int index = 0;
	for (int i = 0;i < MAX_HUPAI_TYPE;i++)
	{
		if (hupai[i] == type)
		{
			hupai[i] = 255;
			break;
		}
	}
	//去除中间无效元素
	BYTE temp[MAX_HUPAI_TYPE];
	memset(temp, 255, sizeof(temp));
	for (int i = 0;i < MAX_HUPAI_TYPE;i++)
	{
		if (hupai[i] != 255)
		{
			temp[index] = hupai[i];
			index++;
		}
	}
	for (int i = 0;i < MAX_HUPAI_TYPE;i++)
	{
		hupai[i] = temp[i];
	}
}

///名称：ClearHupaiType
///描述：清除所有糊牌类型
///@param type 要删除的类型 , hupai[] 糊牌类型数组
///@return 
void CheckHuPai::ClearHupaiType(BYTE hupai[])
{
	for (int i = 0;i < MAX_HUPAI_TYPE;i++)
	{
		hupai[i] = 255;
	}
}

///名称：CheckHupaiType
///描述：检测是否存在某种糊牌类型
///@param type 要检测的类型 , hupai[] 糊牌类型数组
///@return true 存在，flse 不存在
bool CheckHuPai::CheckHupaiType(BYTE type, BYTE hupai[])
{
	int index = 0;
	for (int i = 0;i < MAX_HUPAI_TYPE;i++)
	{
		if (hupai[i] == type && type != 255)
		{
			return true;
		}
	}
	return false;
}
///名称：ClearPingHuData
///描述：清除组牌数据
///@param 
///@return 
void CheckHuPai::ClearPingHuData()
{
	m_byJiang = 255;			//将牌数据
	m_byShunNum = 0;			//顺子的个数
	memset(m_byShunData, 255, sizeof(m_byShunData));//顺子的数据	
	m_byKeziNum = 0;			//刻子的个数	
	memset(m_byKeziData, 255, sizeof(m_byKeziData));//刻子的数据
	m_byGangNum = 0;			//杠牌的个数
	memset(m_byGangData, 255, sizeof(m_byGangData));	//杠牌的数据
	m_byChiNum = 0;				//吃牌的个数
	m_byPengNum = 0;			//碰牌的个数

	m_byHandShunNum = 0;			//手中顺子的个数，不包括吃
	memset(m_byHandShunData, 255, sizeof(m_byHandShunData));//顺子的数据，不包括吃
}

///名称：CheckPingHuData
///描述：检测平糊数据组合：将牌，吃，碰杠，顺子，暗刻等
///@param 
///@return 
void CheckHuPai::CheckPingHuData(GameDataEx *pGameData)
{
	ClearPingHuData();
	BYTE num = 0;
	for (int i = 0;i < m_tHuPaiStruct.count;i++)
	{
		switch (m_tHuPaiStruct.byType[i])
		{
		case TYPE_JINGDIAO_JIANG:
		case TYPE_JIANG_PAI:
		{
			m_byJiang = m_tHuPaiStruct.data[i][0];
		}
		break;
		case TYPE_SHUN_ZI:
		{
			m_byShunData[m_byShunNum] = m_tHuPaiStruct.data[i][0];
			m_byShunNum++;

			m_byHandShunData[m_byHandShunNum] = m_tHuPaiStruct.data[i][0];
			m_byHandShunNum++;
		}
		break;
		case TYPE_AN_KE:
		{
			m_byKeziData[m_byKeziNum] = m_tHuPaiStruct.data[i][0];
			m_byKeziNum++;
		}
		break;
		default:
			break;
		}
	}
	BYTE pai[4];
	memset(pai, 255, sizeof(pai));
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		for (int j = 0;j < 4;++j)
		{
			pai[j] = pGameData->m_UserGCPData[m_byStation][i].byData[j];
		}
		switch (pGameData->m_UserGCPData[m_byStation][i].byType)
		{
		case ACTION_CHI://吃牌动作
		{
			m_byShunData[m_byShunNum] = pai[0];
			m_byShunNum++;
			m_byChiNum++;
		}
		break;
		case ACTION_PENG://碰牌动作
		{
			m_byKeziData[m_byKeziNum] = pai[0];
			m_byKeziNum++;
			m_byPengNum++;
		}
		break;
		case ACTION_AN_GANG://暗杠动作
		{
			m_byGangData[m_byGangNum][1] = pai[0];
			m_byGangData[m_byGangNum][0] = ACTION_AN_GANG;
			m_byGangNum++;
		}
		break;
		case ACTION_BU_GANG://补杠动作
		{
			m_byGangData[m_byGangNum][1] = pai[0];
			m_byGangData[m_byGangNum][0] = ACTION_BU_GANG;
			m_byGangNum++;
		}
		break;
		case ACTION_MING_GANG://明杠动作
		{
			m_byGangData[m_byGangNum][1] = pai[0];
			m_byGangData[m_byGangNum][0] = ACTION_MING_GANG;
			m_byGangNum++;
		}
		break;
		}
	}
}
/////////////////详细糊牌类型检测//////////////////////////////////////////////////////////
//天糊：
bool CheckHuPai::CheckTianHu(GameDataEx *pGameData)
{
	if (m_byStation != pGameData->m_byNtStation || !m_bZimo)
	{
		return false;
	}
	if (pGameData->m_bHaveOut[m_byStation] || pGameData->GetGCPCount(m_byStation) > 0)
	{
		return false;
	}
	return true;
}
//地糊：
bool CheckHuPai::CheckDiHu(GameDataEx *pGameData)
{
	if (m_byStation == pGameData->m_byNtStation)
	{
		return false;
	}
	if (pGameData->m_bHaveOut[m_byStation] || pGameData->GetGCPCount(m_byStation) > 0 || pGameData->GetOutPaiCount(m_byStation) > 0)
	{
		return false;
	}
	return true;
}
//杠上花：开杠抓进的牌成和牌(不包括补花)不计自摸 
bool CheckHuPai::CheckGangKai(GameDataEx *pGameData)
{
	if (m_byStation == pGameData->m_byNowOutStation && pGameData->m_bGangState[m_byStation])
	{
		return true;
	}
	return false;
}
//杠后炮:摸一张牌,摸到牌以后丢一张牌到桌面,这时候丢出去的牌如果被别人胡了
bool CheckHuPai::CheckGangHouPao(GameDataEx *pGameData)
{
	if (!m_bZimo && m_byBeStation != 255 && pGameData->m_bGangState[m_byBeStation])//处于杠牌状态
	{
		return true;
	}
	return false;
}

//一般高：由一种花色2副相同的顺子组成的牌 ,（包含吃牌）
bool CheckHuPai::CheckYiBanGao()
{
	if (m_byShunNum >= 2)
	{
		for (int i = 0;i < m_byShunNum - 1;++i)
		{
			for (int j = i + 1;j < m_byShunNum;j++)
			{
				if (m_byShunData[i] == m_byShunData[j] && m_byShunData[i] != 255)
					return true;
			}
		}
	}
	return false;
}

//喜相逢：2种花色2副序数相同的顺子  ,（包含吃牌）
bool CheckHuPai::CheckXiXiangFeng()
{
	BYTE shunzi[MAX_CPG_NUM], num = 0, count = 0;
	memset(shunzi, 255, sizeof(shunzi));
	for (int i = 0;i < MAX_CPG_NUM;++i)
	{
		shunzi[i] = m_byShunData[i];
	}
	if (m_byShunNum == 4)
	{
		for (int i = 0;i < m_byShunNum - 1;++i)
		{
			for (int j = i + 1;j < m_byShunNum;j++)
			{
				if (shunzi[i] == shunzi[j] && shunzi[i] != 0)
				{
					shunzi[i] = 0;
					shunzi[j] = 0;
					count++;
				}
			}
		}
		if (count >= 2)
		{
			return true;
		}
	}
	return false;
}

//连六：一种花色6张相连接的序数牌 
bool CheckHuPai::CheckLianLiu()
{
	if (m_byShunNum >= 2)
	{
		for (int i = 0;i < m_byShunNum - 1;++i)
		{
			for (int j = i + 1;j < m_byShunNum;j++)
			{
				if (m_byShunData[i] == m_byShunData[j] + 3 && m_byShunData[i] != 255 && CMjRef::CheckIsTongSe(m_byShunData[i], m_byShunData[j]))
				{
					return true;
				}
			}
		}
	}
	return false;
}

//老少副：一种花色牌的123、789两副顺子  ,（包含吃牌）
bool CheckHuPai::CheckLaoShaoFu()
{
	if (m_byShunNum >= 2)
	{
		for (int i = 0;i < m_byShunNum - 1;++i)
		{
			for (int j = i + 1;j < m_byShunNum;j++)
			{
				if (m_byShunData[i] % 10 == 1 && m_byShunData[j] % 10 == 9 && CMjRef::CheckIsTongSe(m_byShunData[i], m_byShunData[j]))
				{
					return true;
				}
				if (m_byShunData[i] % 10 == 9 && m_byShunData[j] % 10 == 1 && CMjRef::CheckIsTongSe(m_byShunData[i], m_byShunData[j]))
				{
					return true;
				}
			}
		}
	}
	return false;
}

//幺九刻：3张相同的一、九序数牌及字牌组成的刻子(或杠) 
bool CheckHuPai::CheckYaoJiuKe()
{
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if ((m_byKeziData[i] % 10 == 1 || m_byKeziData[i] % 10 == 9) && m_byKeziData[i] < CMjEnum::MJ_TYPE_FD)
			return true;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if ((m_byGangData[i][1] % 10 == 1 || m_byGangData[i][1] % 10 == 9) && m_byGangData[i][1] < CMjEnum::MJ_TYPE_FD)
			return true;
	}
	return false;
}

//明杠：自己有暗刻，碰别人打出的一张相同的牌开杠：或自己抓进一张与碰的明刻相同的牌开杠 
bool CheckHuPai::CheckMingGang()
{
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] == ACTION_MING_GANG || m_byGangData[i][1] == ACTION_BU_GANG)
			return true;
	}
	return false;
}

//缺一门：和牌中缺少一种花色序数牌 
bool CheckHuPai::CheckQueYiMen()
{
	bool hua[4];
	BYTE num = 0;
	memset(hua, 0, sizeof(hua));
	if (m_byJiang != 255)
	{
		hua[CMjRef::GetHuaSe(m_byJiang)] = true;
	}
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] != 255 && m_byShunData[i] < CMjEnum::MJ_TYPE_FD)
		{
			hua[CMjRef::GetHuaSe(m_byShunData[i])] = true;
		}
	}
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] != 255 && m_byKeziData[i] < CMjEnum::MJ_TYPE_FD)
		{
			hua[CMjRef::GetHuaSe(m_byKeziData[i])] = true;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] != 255 && m_byGangData[i][1] < CMjEnum::MJ_TYPE_FD)
		{
			hua[CMjRef::GetHuaSe(m_byGangData[i][1])] = true;
		}
	}
	for (int i = 0;i < 3;++i)
	{
		if (hua[i])
		{
			num++;
		}
	}
	if (num == 2)
	{
		return true;
	}
	return false;
}

//无字：和牌中没有风、箭牌 
bool CheckHuPai::CheckWuZi()
{
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] != 255 && m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD)
			return false;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] != 255 && m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD)
			return false;
	}
	return true;
}

//边张：单和123的3及789的7或1233和3、77879和7都为张。(未处理：手中有12345和3，56789和6不算边张 )
bool CheckHuPai::CheckBianZhang(GameDataEx *pGameData)
{
	if (pGameData->GetOutPaiCount(pGameData->m_byNtStation) == 0 && m_byChiNum == 0 && m_byPengNum == 0 && m_byGangNum == 0)
	{//天糊不算
		return false;
	}
	if (m_byLastPai >= CMjEnum::MJ_TYPE_FD || (m_byLastPai % 10 != 3 && m_byLastPai % 10 != 7))
	{//最后抓到的牌是字牌，或非3,7的牌，或是将牌
		return false;
	}
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] != TYPE_SHUN_ZI)
		{
			continue;
		}
		if (m_tHuPaiStruct.data[i][2] % 10 == 3 && m_tHuPaiStruct.data[i][2] == m_byLastPai)
		{
			return true;
		}
		if (m_tHuPaiStruct.data[i][0] % 10 == 7 && m_tHuPaiStruct.data[i][0] == m_byLastPai)
		{
			return true;
		}
	}
	return false;
}

//坎张：和2张牌之间的牌。4556和5也为坎张。未处理：手中有45567和6不算坎张 
bool CheckHuPai::CheckKanZhang(GameDataEx *pGameData)
{
	if (pGameData->GetOutPaiCount(pGameData->m_byNtStation) == 0 && m_byChiNum == 0 && m_byPengNum == 0 && m_byGangNum == 0)
	{//天糊不算
		return false;
	}
	if (m_byLastPai >= CMjEnum::MJ_TYPE_FD)
	{//最后抓到的牌是字牌
		return false;
	}
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] != TYPE_SHUN_ZI)
		{
			continue;
		}
		if (m_tHuPaiStruct.data[i][1] == m_byLastPai)
		{
			return true;
		}
	}
	return false;
}

//单钓将：钓单张牌作将成和 
bool CheckHuPai::CheckDanDiaoJiang(GameDataEx *pGameData)
{
	if (pGameData->GetOutPaiCount(pGameData->m_byNtStation) == 0 && m_byChiNum == 0 && m_byPengNum == 0 && m_byGangNum == 0)
	{//天糊不算
		return false;
	}
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] == TYPE_JIANG_PAI || m_tHuPaiStruct.byType[i] == TYPE_JINGDIAO_JIANG)
		{
			if (m_tHuPaiStruct.data[i][0] == m_byLastPai)
			{
				return true;
			}
		}
	}
	return false;
}

//箭刻：由中、发、白3张相同的牌组成的刻子 ,（杠不算）
bool CheckHuPai::CheckJianKe()
{
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] == CMjEnum::MJ_TYPE_ZHONG || m_byKeziData[i] == CMjEnum::MJ_TYPE_FA || m_byKeziData[i] == CMjEnum::MJ_TYPE_BAI)
		{
			return true;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] == CMjEnum::MJ_TYPE_ZHONG || m_byGangData[i][1] == CMjEnum::MJ_TYPE_FA || m_byGangData[i][1] == CMjEnum::MJ_TYPE_BAI)
		{
			return true;
		}
	}
	return false;
}

//圈风刻：与圈风相同的风刻 ，（杠不算）
bool CheckHuPai::CheckQuanFengKe(GameDataEx *pGameData)
{
	if (pGameData->m_byQuanFeng == 255)
		return false;
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] == pGameData->m_byQuanFeng)
		{
			return true;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] == pGameData->m_byQuanFeng)
		{
			return true;
		}
	}
	return false;
}

//门风刻：与本门风相同的风刻 ,（杠不算）
bool CheckHuPai::CheckMenFengKe()
{

	return false;
}

//门前清：没有吃、碰、明杠，和别人打出的牌 
bool CheckHuPai::CheckMenQianQing(GameDataEx *pGameData)
{
	if (m_byStation == pGameData->m_byNowOutStation)
	{
		return false;
	}
	if (m_byChiNum > 0 || m_byPengNum > 0)
	{
		return false;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][0] != ACTION_AN_GANG)
		{
			return false;
		}
	}
	return true;
}

//四归一： 和牌中，有4张相同的牌归于一家的顺、刻子、对、将牌中(不包括杠牌) 
bool CheckHuPai::CheckSiGuiYi(GameDataEx *pGameData)
{
	if (m_byStation == pGameData->m_byNowOutStation)//自摸
	{
		for (int i = 0;i < m_tHuPaiStruct.count;++i)
		{
			if (m_tHuPaiStruct.byType[i] == TYPE_JIANG_PAI || m_tHuPaiStruct.byType[i] == TYPE_JINGDIAO_JIANG || m_tHuPaiStruct.byType[i] == TYPE_AN_KE)
			{
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][0]) == 4)
					return true;
			}
			else if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
			{
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][0]) == 4)
					return true;
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][1]) == 4)
					return true;
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][2]) == 4)
					return true;
			}
		}
	}
	else
	{
		for (int i = 0;i < m_tHuPaiStruct.count;++i)
		{
			if (m_tHuPaiStruct.byType[i] == TYPE_JIANG_PAI || m_tHuPaiStruct.byType[i] == TYPE_JINGDIAO_JIANG || m_tHuPaiStruct.byType[i] == TYPE_AN_KE)
			{
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][0]) == 4
					|| (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][0]) == 3 && m_byLastPai == m_tHuPaiStruct.data[i][0]))
					return true;
			}
			else if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
			{
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][0]) == 4
					|| (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][0]) == 3 && m_byLastPai == m_tHuPaiStruct.data[i][0]))
					return true;
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][1]) == 4
					|| (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][1]) == 3 && m_byLastPai == m_tHuPaiStruct.data[i][1]))
					return true;
				if (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][2]) == 4
					|| (pGameData->GetAHandPaiCount(m_byStation, m_tHuPaiStruct.data[i][2]) == 3 && m_byLastPai == m_tHuPaiStruct.data[i][2]))
					return true;
			}
		}
	}
	return false;
}

//双同刻： 2副序数相同的刻子 ,（不包含杠牌）
bool CheckHuPai::CheckShuangTongKe()
{
	if (m_byKeziNum < 2)
	{
		return false;
	}

	BYTE num[9];
	memset(num, 0, sizeof(num));
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		num[m_byKeziData[i] % 10 - 1]++;

		if (num[m_byKeziData[i] % 10 - 1] >= 2)
		{
			return true;
		}
	}
	return false;
}

//双暗刻：2个暗刻 ,算别人点炮的牌,（不包含暗杠牌）
bool CheckHuPai::CheckShuangAnGang()
{
	if ((m_byKeziNum - m_byPengNum) == 2)
		return true;
	return false;
}

//暗杠：自抓4张相同的牌开杠 
bool CheckHuPai::CheckAnGang()
{
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][0] == ACTION_AN_GANG)
		{
			return true;
		}
	}
	return false;
}

//断幺：和牌中没有一、九及字牌 
bool CheckHuPai::CheckDuanYao(GameDataEx *pGameData)
{
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255)
			continue;
		if (m_byArHandPai[i] >= CMjEnum::MJ_TYPE_FD || m_byArHandPai[i] % 10 == 1 || m_byArHandPai[i] == 9)
			return false;
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		if (pGameData->m_UserGCPData[m_byStation][i].byData[0] >= CMjEnum::MJ_TYPE_FD || pGameData->m_UserGCPData[m_byStation][i].byData[0] % 10 == 1 || pGameData->m_UserGCPData[m_byStation][i].byData[0] % 10 == 9)
			return false;
	}
	return true;
}

//全带幺：和牌时，每副牌、将牌都有幺牌 
bool CheckHuPai::CheckQuanDaiYao(GameDataEx *pGameData)
{
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] == TYPE_JIANG_PAI || m_tHuPaiStruct.byType[i] == TYPE_JINGDIAO_JIANG || m_tHuPaiStruct.byType[i] == TYPE_AN_KE)
		{
			if (m_tHuPaiStruct.data[i][0] < CMjEnum::MJ_TYPE_FD && m_tHuPaiStruct.data[i][0] % 10 != 1 && m_tHuPaiStruct.data[i][0] % 10 != 9)
				return false;
		}
		else if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
		{
			bool jiu = false;
			for (int j = 0;j < 3;j++)
			{
				if (m_tHuPaiStruct.data[i][j] % 10 == 1 || m_tHuPaiStruct.data[i][j] % 10 == 9)
				{
					jiu = true;
				}
			}
			if (!jiu)
			{
				return false;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		if (pGameData->m_UserGCPData[m_byStation][i].byData[0] >= CMjEnum::MJ_TYPE_FD)
			continue;
		bool jiu = false;
		for (int j = 0;j < 3;j++)
		{
			if (pGameData->m_UserGCPData[m_byStation][i].byData[j] % 10 == 1 || pGameData->m_UserGCPData[m_byStation][i].byData[j] % 10 == 9)
			{
				jiu = true;
			}
		}
		if (!jiu)
		{
			return false;
		}
	}
	return true;
}

//不求人：4副牌及将中没有吃牌、碰牌(包括明杠)，自摸和牌 
bool CheckHuPai::CheckBuQiuRen(GameDataEx *pGameData)
{
	if (m_byStation != pGameData->m_byNowOutStation)//自摸
	{
		return false;
	}
	if (m_byPengNum > 0)
	{
		return false;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][0] != ACTION_AN_GANG)
		{
			return false;
		}
	}
	return true;
}

//双明杠：2个明杠 
bool CheckHuPai::CheckShuangMingGang()
{
	BYTE num = 0;
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][0] != ACTION_AN_GANG)
		{
			num++;
		}
	}
	if (num == 2)
	{
		return true;
	}
	return false;
}

//和绝张：和牌池、桌面已亮明的3张牌所剩的第4张牌(抢杠和不计和绝张) 
bool CheckHuPai::CheckHuJueZhang()
{

	return false;
}

//碰碰和： 由4副刻子(或杠)、将牌组成的和牌 
bool CheckHuPai::CheckPengPengHu(GameDataEx *pGameData)
{
	if (m_byShunNum == 0 && 0 == pGameData->GetUserChiNum(m_byStation))//顺子个数为0(不能有吃牌)
	{
		return true;
	}
	return false;
}

//一色牌检测（检测手牌和吃碰杠牌，哪里都适用）：0不是一色牌，1混一色，2清一色
int CheckHuPai::CheckYiSe(GameDataEx *pGameData)
{
	bool zi = false;
	BYTE type = 255, pai = 255;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255)
			continue;
		pai = m_byArHandPai[i];
		if (pGameData->m_StrJing.CheckIsJing(pai))
		{
			continue;
		}
		if (CMjRef::GetHuaSe(pai) == 3)
		{
			zi = true;
			continue;
		}
		if (type == 255)
		{
			type = CMjRef::GetHuaSe(pai);
		}
		if (type != CMjRef::GetHuaSe(pai))
		{
			return 0;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (CMjRef::GetHuaSe(pai) == 3)
		{
			zi = true;
			continue;
		}
		if (type == 255)
		{
			type = CMjRef::GetHuaSe(pai);
		}
		if (type != CMjRef::GetHuaSe(pai))
		{
			return 0;
		}
	}
	if (type == 255)//全是字不能算清一色
		return 0;

	if (zi)
	{
		return 1;
	}
	return 2;
}



//三色三步高： 3种花色3副依次递增一位序数的顺子，（包含吃牌）
bool CheckHuPai::CheckSanSeSanBuGao()
{
	if (m_byShunNum < 3)
	{
		return false;
	}
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	CLogicBase::SelectSort(m_byShunData, 4, false);
	for (int i = 0;i < 4;++i)
	{
		if (m_byShunData[i] == 255)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < 7;++i)
	{
		if (shun[0][i] && shun[1][i + 1] && shun[2][i + 2]
			|| shun[1][i] && shun[2][i + 1] && shun[0][i + 2]
			|| shun[2][i] && shun[1][i + 1] && shun[0][i + 2]
			)
		{
			return true;
		}
	}

	return false;
}

//五门齐：和牌时3种序数牌、风、箭牌齐全 
bool CheckHuPai::CheckWuMenQi(GameDataEx *pGameData)
{
	bool wbt[3], feng = false, jian = false;
	memset(wbt, 0, sizeof(wbt));
	BYTE pai = 255;

	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.data[i][0] == 255)
			continue;
		pai = m_tHuPaiStruct.data[i][0];
		if (CMjRef::GetHuaSe(pai) < 3)
		{
			wbt[CMjRef::GetHuaSe(pai)] = true;
		}
		else
		{
			if (pai >= CMjEnum::MJ_TYPE_ZHONG)
			{
				jian = true;
			}
			else if (pai >= CMjEnum::MJ_TYPE_FD)
			{
				feng = true;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (CMjRef::GetHuaSe(pai) < 3)
		{
			wbt[CMjRef::GetHuaSe(pai)] = true;
		}
		else
		{
			if (pai >= CMjEnum::MJ_TYPE_ZHONG)
			{
				jian = true;
			}
			else if (pai >= CMjEnum::MJ_TYPE_FD)
			{
				feng = true;
			}
		}
	}

	if (wbt[0] && wbt[1] && wbt[2] && feng && jian)
	{
		return true;
	}
	return false;
}

//全求人：全靠吃牌、碰牌、单钓别人批出的牌和牌。不计单钓 
bool CheckHuPai::CheckQuanQiuRen(GameDataEx *pGameData)
{
	if (m_byStation == pGameData->m_byNowOutStation)//自摸
	{
		return false;
	}
	if (pGameData->GetHandPaiCount(m_byStation) != 1)
	{
		return false;
	}
	/*for(int i=0;i<m_tHuPaiStruct.count;++i)
	{
	if(m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI || m_tHuPaiStruct.byType[i] == TYPE_AN_KE)
	{
	return false;
	}
	}
	for(int i=0;i<m_byGangNum;++i)
	{
	if(m_byGangData[i][0] == ACTION_AN_GANG)
	{
	return false;
	}
	}*/
	return true;
}

//双暗杠： 2个暗杠 
bool CheckHuPai::CheckShuangAnKe()
{
	BYTE num = 0;
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][0] == ACTION_AN_GANG)
		{
			num++;
		}
	}
	if (num == 2)
	{
		return true;
	}
	return false;
}

//双箭刻：2副箭刻(或杠) 
bool CheckHuPai::CheckShuangJianKe()
{
	BYTE num = 0;
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_ZHONG && m_byGangData[i][1] <= CMjEnum::MJ_TYPE_BAI)
		{
			num++;
		}
	}
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_ZHONG && m_byKeziData[i] <= CMjEnum::MJ_TYPE_BAI)
		{
			num++;
		}
	}
	return false;
}

//花龙： 3种花色的3副顺子连接成1-9的序数牌 ,，（包含吃牌）
bool CheckHuPai::CheckHuaLong()
{
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	if (shun[0][1] && shun[1][4] && shun[2][7]
		|| shun[1][1] && shun[2][4] && shun[0][7]
		|| shun[2][1] && shun[0][4] && shun[1][7])
	{
		return true;
	}
	return false;
}

//推不倒： 由牌面图形没有上下区别的牌组成的和牌，包括1234589饼、245689条、白板。不计缺一门 
bool CheckHuPai::CheckTuiBuDao(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai <= CMjEnum::MJ_TYPE_W9 || pai >= CMjEnum::MJ_TYPE_FD && pai != CMjEnum::MJ_TYPE_BAI)
		{
			return false;
		}
		if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
		{
			if (pai > CMjEnum::MJ_TYPE_B3 && pai != CMjEnum::MJ_TYPE_T4)
			{
				return false;
			}
		}
		else
		{
			if (pai == CMjEnum::MJ_TYPE_B6 || pai == CMjEnum::MJ_TYPE_B7 || pai == CMjEnum::MJ_TYPE_T1 || pai == CMjEnum::MJ_TYPE_T3 || pai == CMjEnum::MJ_TYPE_T7)
			{
				return false;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai <= CMjEnum::MJ_TYPE_W9 || pai >= CMjEnum::MJ_TYPE_FD && pai != CMjEnum::MJ_TYPE_BAI)
		{
			return false;
		}
		if (pGameData->m_UserGCPData[m_byStation][i].byType == ACTION_CHI)
		{
			if (pai > CMjEnum::MJ_TYPE_B3 && pai != CMjEnum::MJ_TYPE_T4)
			{
				return false;
			}
		}
		else
		{
			if (pai == CMjEnum::MJ_TYPE_B6 || pai == CMjEnum::MJ_TYPE_B7 || pai == CMjEnum::MJ_TYPE_T1 || pai == CMjEnum::MJ_TYPE_T3 || pai == CMjEnum::MJ_TYPE_T7)
			{
				return false;
			}
		}
	}
	return true;
}

//三色三同顺：和牌时，有3种花色3副序数相同的顺子 ，（包含吃牌）
bool CheckHuPai::CheckSanSeSanTongShun()
{
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < 9;++i)
	{
		if (shun[0][i] && shun[1][i] && shun[2][i])
		{
			return true;
		}
	}
	return false;
}

//三色三节高：和牌时，有3种花色3副依次递增一位数的刻子，（包含吃牌）
bool CheckHuPai::CheckSanSeSanJieGao()
{
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < 7;++i)
	{
		if (shun[0][i] && shun[1][i + 1] && shun[2][i + 2]
			|| shun[1][i] && shun[2][i + 1] && shun[0][i + 2]
			|| shun[2][i] && shun[1][i + 1] && shun[0][i + 2])
		{
			return true;
		}
	}
	return false;
}

//无番和：和牌后，数不出任何番种分(花牌不计算在内) 
bool CheckHuPai::CheckWuFanHe()
{

	return false;
}

//妙手回春：自摸牌墙上最后一张牌和牌。不计自摸 
bool CheckHuPai::CheckMiaoShouHuiChun(GameDataEx *pGameData)
{
	if (m_byStation == pGameData->m_byNowOutStation && pGameData->m_MenPai.GetMenPaiNum() == 0)//自摸
	{
		return true;
	}
	return false;
}

//海底捞月：和打出的最后一张牌 
bool CheckHuPai::CheckHaiDiLaoYue(GameDataEx *pGameData)
{
	if (m_byStation != pGameData->m_byNowOutStation && pGameData->m_MenPai.GetMenPaiNum() == 0)//自摸
	{
		return true;
	}
	return false;
}

//抢杠和：和别人自抓开明杠的牌。不计和绝张  
bool CheckHuPai::CheckQiangGangHe(GameDataEx *pGameData)
{
	if (m_byStation != pGameData->m_byNowOutStation && ACTION_BU_GANG == pGameData->T_GangPai.byType)
	{
		return true;
	}
	return false;
}

//大于五：由序数牌6-9的顺子、刻子、将牌组成的和牌。不计无字 
bool CheckHuPai::CheckDaYuWu(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pai % 10 < 6)
		{
			return false;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pai % 10 < 6)
		{
			return false;
		}
	}
	return true;
}

//小于五：由序数牌1-4的顺子、刻子、将牌组成的和牌。不计无字 
bool CheckHuPai::CheckXiaoYuWu(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
		{
			if (pai % 10 > 2)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 > 4)
			{
				return false;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pGameData->m_UserGCPData[m_byStation][i].byType == ACTION_CHI)
		{
			if (pai % 10 > 2)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 > 4)
			{
				return false;
			}
		}
	}
	return true;
}

//三风刻：3个风刻 。(不包括杠牌)
bool CheckHuPai::CheckSanFengKe()
{
	BYTE num = 0;
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD && m_byKeziData[i] <= CMjEnum::MJ_TYPE_FB)
		{
			num++;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD && m_byGangData[i][1] <= CMjEnum::MJ_TYPE_FB)
		{
			num++;
		}
	}
	if (num == 3)
	{
		return true;
	}
	return false;
}

//清龙：和牌时，有一种花色1-9相连接的序数牌 ，（包含吃牌）
bool CheckHuPai::CheckQingLong()
{
	if (m_byShunNum < 3)
	{
		return false;
	}
	CLogicBase::SelectSort(m_byShunData, 4, false);
	if (m_byShunData[0] == m_byShunData[1] - 3 == m_byShunData[2] - 6 && m_byShunData[0] % 10 == 1)
	{
		return true;
	}
	return false;
}

//三色双龙会：2种花色2个老少副、另一种花色5作将的和牌。不计喜相逢、老少副、无字、平和 ，（包含吃牌）
bool CheckHuPai::CheckSanSeShuangLongHui()
{
	if (m_byShunNum < 4)
	{
		return false;
	}
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	if (shun[0][0] && shun[0][6] && shun[1][0] && shun[1][6] && m_byJiang == CMjEnum::MJ_TYPE_T5
		|| shun[0][0] && shun[0][6] && shun[2][0] && shun[2][6] && m_byJiang == CMjEnum::MJ_TYPE_B5
		|| shun[2][0] && shun[2][6] && shun[1][0] && shun[1][6] && m_byJiang == CMjEnum::MJ_TYPE_W5)
	{
		return true;
	}
	return false;
}

//一色三步高： 和牌时，有一种花色3副依次递增一位或依次递增二位数字的顺子 ，（包含吃牌）
bool CheckHuPai::CheckYiSeSanBuGao()
{
	if (m_byShunNum < 3)
	{
		return false;
	}
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < 7;++i)
	{
		if (shun[0][i] && shun[0][i + 1] && shun[0][i + 2]
			|| shun[1][i] && shun[1][i + 1] && shun[1][i + 2]
			|| shun[2][i] && shun[2][i + 1] && shun[2][i + 2])
		{
			return true;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (shun[0][i] && shun[0][i + 2] && shun[0][i + 4]
			|| shun[1][i] && shun[1][i + 2] && shun[1][i + 4]
			|| shun[2][i] && shun[2][i + 2] && shun[2][i + 4])
		{
			return true;
		}
	}
	return false;
}

//全带五：每副牌及将牌必须有5的序数牌。不计断幺 
bool CheckHuPai::CheckQuanDaiWu(GameDataEx *pGameData)
{
	BYTE pai = 255, type = 255;
	bool zi = false;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (CMjRef::GetHuaSe(pai) == 3)
		{
			return false;
		}
		if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
		{
			if (pai % 10 > 7 || pai % 10 < 3)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 != 5)
			{
				return false;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (CMjRef::GetHuaSe(pai) == 3)
		{
			return false;
		}
		if (pGameData->m_UserGCPData[m_byStation][i].byType == ACTION_CHI)
		{
			if (pai % 10 > 7 || pai % 10 < 3)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 != 5)
			{
				return false;
			}
		}
	}
	return true;
}

//三同刻：3个序数相同的刻子(杠) 
bool CheckHuPai::CheckSanTongKe()
{
	BYTE num[9];
	memset(num, 0, sizeof(num));
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD)
		{
			continue;
		}
		num[m_byKeziData[i] % 10 - 1]++;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD)
		{
			continue;
		}
		num[m_byGangData[i][1] % 10 - 1]++;
	}
	for (int i = 0;i < 9;++i)
	{
		if (num[i] == 3)
		{
			return true;
		}
	}
	return false;
}

//三暗刻： 3个暗刻 
bool CheckHuPai::CheckSanAnKe()
{
	BYTE num = 0;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] == TYPE_AN_KE)
		{
			num++;
		}
	}
	if (num == 3)
	{
		return true;
	}
	return false;
}

//全双刻： 由2、4、6、8序数牌的刻了、将牌组成的和牌。（不包括杠牌）。不计碰碰和、断幺 
bool CheckHuPai::CheckQuanShuangKe()
{
	if (m_byShunNum > 0)
	{
		return false;
	}
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD || m_byKeziData[i] % 10 != 0)
		{
			return false;
		}
	}
	if (m_byJiang >= CMjEnum::MJ_TYPE_FD || m_byJiang % 10 != 0)
	{
		return false;
	}
	return true;
}

//清一色： 由一种花色的序数牌组成和各牌。不无字 
bool CheckHuPai::CheckQingYiSe(GameDataEx *pGameData)
{
	return CheckYiSe(pGameData) == 2;
}

//清一色一条龙
bool CheckHuPai::CheckQingYiSeYiTiaoLong(GameDataEx *pGameData)
{
	if (CheckQingYiSe(pGameData) && CheckQingLong())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//混一色
bool CheckHuPai::CheckHunYiSe(GameDataEx *pGameData)
{
	return CheckYiSe(pGameData) == 1;
}

//一色三同顺：和牌时有一种花色3副序数相同的顺了。不计一色三节高 。（包含吃牌）
bool CheckHuPai::CheckYiSeSanTongShun()
{
	if (m_byShunNum < 3)
	{
		return false;
	}
	BYTE num[3][9];
	memset(num, 0, sizeof(num));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		num[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1]++;
	}
	for (int i = 0;i < 7;++i)
	{
		if (num[0][i] == 3 || num[1][i] == 3 || num[2][i] == 3)
		{
			return true;
		}
	}
	return false;
}

//一色三节高：和牌时有一种花色3副依次递增一位数字的刻子。不计一色三同顺 。（包含碰牌）
bool CheckHuPai::CheckYiSeSanJieGao()
{
	if (m_byKeziNum < 3)
	{
		return false;
	}
	bool kezi[3][9];
	memset(kezi, 0, sizeof(kezi));
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		kezi[CMjRef::GetHuaSe(m_byKeziData[i])][m_byKeziData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD)
			continue;
		kezi[CMjRef::GetHuaSe(m_byGangData[i][1])][m_byGangData[i][1] % 10 - 1] = true;
	}
	for (int i = 0;i < 7;++i)
	{
		if (kezi[0][i] && kezi[0][i + 1] && kezi[0][i + 2]
			|| kezi[1][i] && kezi[1][i + 1] && kezi[1][i + 2]
			|| kezi[2][i] && kezi[2][i + 1] && kezi[2][i + 2])
		{
			return true;
		}
	}
	return false;
}

//全大：由序数牌789组成的顺了、刻子(杠)、将牌的和牌。不计无字 
bool CheckHuPai::CheckQuanDa(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pai % 10 < 6)
		{
			return false;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pai % 10 < 6)
		{
			return false;
		}
	}
	return true;
}

//全中： 由序数牌456组成的顺子、刻子(杠)、将牌的和牌。不计断幺 
bool CheckHuPai::CheckQuanZhong(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
		{
			if (pai % 10 != 4)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 > 6 || pai % 10 < 4)
			{
				return false;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pGameData->m_UserGCPData[m_byStation][i].byType == ACTION_CHI)
		{
			if (pai % 10 != 4)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 > 6 || pai % 10 < 4)
			{
				return false;
			}
		}
	}
	return true;
}

//全小：由序数牌123组成的顺子、刻子(杠)将牌的的和牌。不计无字 
bool CheckHuPai::CheckQuanXiao(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)
		{
			if (pai % 10 > 1)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 > 3)
			{
				return false;
			}
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (pGameData->m_UserGCPData[m_byStation][i].byType == ACTION_CHI)
		{
			if (pai % 10 > 1)
			{
				return false;
			}
		}
		else
		{
			if (pai % 10 > 3)
			{
				return false;
			}
		}
	}
	return true;
}

//一色四步高： 一种花色4副依次递增一位数或依次递增二位数的顺子 。（包含吃牌）
bool CheckHuPai::CheckYiSeSiBuGao()
{
	if (m_byShunNum < 4)
	{
		return false;
	}
	bool shun[3][9];
	memset(shun, 0, sizeof(shun));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		shun[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < 6;++i)
	{
		if (shun[0][i] && shun[0][i + 1] && shun[0][i + 2] && shun[0][i + 3]
			|| shun[1][i] && shun[1][i + 1] && shun[1][i + 2] && shun[1][i + 3]
			|| shun[2][i] && shun[0][i + 1] && shun[2][i + 2] && shun[2][i + 3])
		{
			return true;
		}
	}
	for (int i = 0;i < 3;++i)
	{
		if (shun[0][i] && shun[0][i + 2] && shun[0][i + 4] && shun[0][i + 6]
			|| shun[1][i] && shun[1][i + 2] && shun[1][i + 4] && shun[1][i + 6]
			|| shun[2][i] && shun[0][i + 2] && shun[2][i + 4] && shun[2][i + 6])
		{
			return true;
		}
	}
	return false;
}

//三杠：3个杠 
bool CheckHuPai::CheckSanGang()
{
	if (m_byGangNum == 3)
	{
		return true;
	}
	return false;
}

//混幺九：由字牌和序数牌一、九的刻子用将牌组成的和牌。不计碰碰和 
bool CheckHuPai::CheckHunYaoJiu(GameDataEx *pGameData)
{
	if (m_byShunNum > 0)
	{
		return false;
	}
	BYTE pai = 255, type = 255;
	bool zi = false;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (CMjRef::GetHuaSe(pai) == 3)
		{
			zi = true;
			continue;
		}
		if (pai % 10 != 1 && pai % 10 != 9)
		{
			return false;
		}
		if (type == 255)
		{
			type = CMjRef::GetHuaSe(pai);
		}
		if (type != CMjRef::GetHuaSe(pai))
		{
			return false;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (CMjRef::GetHuaSe(pai) == 3)
		{
			zi = true;
			continue;
		}
		if (pai % 10 != 1 && pai % 10 != 9)
		{
			return false;
		}
		if (type == 255)
		{
			type = CMjRef::GetHuaSe(pai);
		}
		if (type != CMjRef::GetHuaSe(pai))
		{
			return false;
		}
	}
	if (zi)
	{
		return true;
	}
	return false;
}

//一色四同顺：一种花色4副序数相同的顺子，不计一色三节高、一般高、四归一 。（包含吃牌）
bool CheckHuPai::CheckYiSeSiTongShun()
{
	if (m_byShunNum < 4)
	{
		return false;
	}
	BYTE num[3][9];
	memset(num, 0, sizeof(num));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		num[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1]++;
	}
	for (int i = 0;i < 7;++i)
	{
		if (num[0][i] == 4 || num[1][i] == 4 || num[2][i] == 4)
		{
			return true;
		}
	}
	return false;
}

//一色四节高：一种花色4副依次递增一位数的刻子不计一色三同顺、碰碰和 。（包含吃牌）
bool CheckHuPai::CheckYiSeSiJieGao()
{
	if ((m_byKeziNum + m_byGangNum) < 4)
	{
		return false;
	}
	bool kezi[3][9];
	memset(kezi, 0, sizeof(kezi));
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		kezi[CMjRef::GetHuaSe(m_byKeziData[i])][m_byKeziData[i] % 10 - 1] = true;
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD)
			continue;
		kezi[CMjRef::GetHuaSe(m_byGangData[i][1])][m_byGangData[i][1] % 10 - 1] = true;
	}
	for (int i = 0;i < 6;++i)
	{
		if (kezi[0][i] && kezi[0][i + 1] && kezi[0][i + 2] && kezi[0][i + 3]
			|| kezi[1][i] && kezi[1][i + 1] && kezi[1][i + 2] && kezi[1][i + 3]
			|| kezi[2][i] && kezi[2][i + 1] && kezi[2][i + 2] && kezi[2][i + 3])
		{
			return true;
		}
	}
	return false;
}

//清幺九：由序数牌一、九刻子组成的和牌。不计碰碰和、同刻、无字 
bool CheckHuPai::CheckQingYaoJiu(GameDataEx *pGameData)
{
	if (m_byShunNum > 0)
	{
		return false;
	}
	BYTE pai = 255, type = 255;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		pai = m_tHuPaiStruct.data[i][0];
		if (pai % 10 != 1 && pai % 10 != 9)
		{
			return false;
		}
		if (type == 255)
		{
			type = CMjRef::GetHuaSe(pai);
		}
		if (type != CMjRef::GetHuaSe(pai))
		{
			return false;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pai % 10 != 1 && pai % 10 != 9)
		{
			return false;
		}
		if (type == 255)
		{
			type = CMjRef::GetHuaSe(pai);
		}
		if (type != CMjRef::GetHuaSe(pai))
		{
			return false;
		}
	}
	return true;
}

//小四喜：和牌时有风牌的3副刻子及将牌。不计三风刻 。（不包括杠牌）
bool CheckHuPai::CheckXiaoSiXi()
{
	if ((m_byGangNum + m_byKeziNum) < 3)
	{
		return false;
	}
	BYTE num = 0;
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD && m_byKeziData[i] <= CMjEnum::MJ_TYPE_FB)
		{
			num++;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD && m_byGangData[i][1] <= CMjEnum::MJ_TYPE_FB)
		{
			num++;
		}
	}
	if (num == 3 && m_byJiang >= CMjEnum::MJ_TYPE_FD && m_byJiang <= CMjEnum::MJ_TYPE_FB)
	{
		return true;
	}
	return false;
}

//小三元： 和牌时有箭牌的两副刻子及将牌。不计箭刻 。（不包括杠牌）
bool CheckHuPai::CheckXiaoSanYuan()
{
	if ((m_byGangNum + m_byKeziNum) < 2)
		return false;
	BYTE num = 0;
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_ZHONG && m_byKeziData[i] <= CMjEnum::MJ_TYPE_BAI)
		{
			num++;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_ZHONG && m_byGangData[i][1] <= CMjEnum::MJ_TYPE_BAI)
		{
			num++;
		}
	}
	if (num == 2 && m_byJiang >= CMjEnum::MJ_TYPE_ZHONG && m_byJiang <= CMjEnum::MJ_TYPE_BAI)
	{
		return true;
	}
	return false;
}

//字一色：由字牌的刻子(杠)、将组成的和牌。不计碰碰和 
bool CheckHuPai::CheckZiYiSe(GameDataEx *pGameData)
{
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] != 255 && m_byArHandPai[i] < CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		if (pGameData->m_UserGCPData[m_byStation][i].byData[0] < CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
	}
	return true;
}

//四暗刻：4个暗刻(暗杠)。包括别人打出的牌。不计门前清、碰碰和 
bool CheckHuPai::CheckSiAnKe(GameDataEx *pGameData)
{

	BYTE num = 0;
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] == TYPE_AN_KE)
		{
			num++;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][0] == ACTION_AN_GANG)
		{
			num++;
		}
	}
	if (num == 4)
	{
		return true;
	}

	CheckPingHuData(pGameData);

	return false;
}

//一色双龙会：一种花色的两个老少副，5为将牌。不计平各、七对、清一色 。（包含吃牌）
bool CheckHuPai::CheckYiSeShuangLongHui()
{
	if (m_byShunNum < 4)
	{
		return false;
	}
	BYTE num[3][9];
	memset(num, 0, sizeof(num));
	for (int i = 0;i < m_byShunNum;++i)
	{
		if (m_byShunData[i] >= CMjEnum::MJ_TYPE_FD)
			continue;
		num[CMjRef::GetHuaSe(m_byShunData[i])][m_byShunData[i] % 10 - 1]++;
	}
	if (num[0][1] == 2 && num[0][7] == 2 && m_byJiang == CMjEnum::MJ_TYPE_W5
		|| num[1][1] == 2 && num[1][7] == 2 && m_byJiang == CMjEnum::MJ_TYPE_B5
		|| num[2][1] == 2 && num[2][7] == 2 && m_byJiang == CMjEnum::MJ_TYPE_T5)
	{
	}

	return false;
}

//大四喜：由4副风刻(杠)组成的和牌。不计圈风刻、门风刻、三风刻、碰碰和 。（包括杠牌）
bool CheckHuPai::CheckDaSiXi()
{
	if ((m_byGangNum + m_byKeziNum) < 4)
	{
		return false;
	}
	bool zfb[4];
	memset(zfb, 0, sizeof(zfb));
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_FD && m_byKeziData[i] <= CMjEnum::MJ_TYPE_FB)
		{
			zfb[m_byKeziData[i] - CMjEnum::MJ_TYPE_FD] = true;
		}
	}
	for (int i = 0;i < m_byGangNum;++i)
	{
		if (m_byGangData[i][1] >= CMjEnum::MJ_TYPE_FD && m_byGangData[i][1] <= CMjEnum::MJ_TYPE_FB)
		{
			zfb[m_byGangData[i][1] - CMjEnum::MJ_TYPE_FD] = true;
		}
	}
	if (zfb[0] && zfb[1] && zfb[2] && zfb[3])
	{
		return true;
	}
	return false;
}

//大三元：和牌中，有中发白3副刻子，杠牌。不计箭刻 。（不包括杠牌）
bool CheckHuPai::CheckDaSanYuan()
{
	if ((m_byGangNum + m_byKeziNum) < 3)
		return false;
	bool zfb[3];
	memset(zfb, 0, sizeof(zfb));
	for (int i = 0;i < m_byKeziNum;++i)
	{
		if (m_byKeziData[i] >= CMjEnum::MJ_TYPE_ZHONG && m_byKeziData[i] <= CMjEnum::MJ_TYPE_FA)
		{
			zfb[m_byKeziData[i] - CMjEnum::MJ_TYPE_ZHONG] = true;
		}
	}
	if (zfb[0] && zfb[1] && zfb[2])
	{
		return true;
	}
	return false;
}

//绿一色：由23468条及发字中的任何牌组成的顺子、刻子、将的和牌。不计混一色。如无"发"字组成的各牌，可计清一色 
bool CheckHuPai::CheckLvYiSe(GameDataEx *pGameData)
{
	BYTE pai = 255;
	for (int i = 0;i < HAND_CARD_NUM;i++)//查找手牌
	{
		if (m_byArHandPai[i] == 255)
			continue;
		pai = m_byArHandPai[i];
		if (pai != CMjEnum::MJ_TYPE_T2 && pai != CMjEnum::MJ_TYPE_T3 && pai != CMjEnum::MJ_TYPE_T4 && pai != CMjEnum::MJ_TYPE_T6 && pai != CMjEnum::MJ_TYPE_T8 && pai != CMjEnum::MJ_TYPE_FA)
			return false;
	}
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		if (pGameData->m_UserGCPData[m_byStation][i].byType == ACTION_CHI)
		{
			if (pai != CMjEnum::MJ_TYPE_T2)
				return false;
		}
		else
		{
			if (pai != CMjEnum::MJ_TYPE_T2 && pai != CMjEnum::MJ_TYPE_T3 && pai != CMjEnum::MJ_TYPE_T4 && pai != CMjEnum::MJ_TYPE_T6 && pai != CMjEnum::MJ_TYPE_T8 && pai != CMjEnum::MJ_TYPE_FA)
				return false;
		}
	}
	return true;
}



//四杠（十八罗汉）：4个杠 
bool CheckHuPai::CheckSiGang()
{
	if (m_byGangNum == 4)
		return true;
	return false;
}

//九莲宝灯：由一种花色序数牌子按1112345678999组成的特定牌型，见同花色任何1张序数牌即成和牌。不计清一色。（不包含吃碰牌）
bool CheckHuPai::CheckJiuLianBaoDeng(GameDataEx *pGameData)
{
	if (pGameData->GetGCPCount(m_byStation) > 0)//不能有吃碰杠
	{
		return false;
	}
	BYTE type = 255;
	BYTE num[9];
	memset(num, 0, sizeof(num));
	BYTE hand[HAND_CARD_NUM];
	memset(hand, 255, sizeof(hand));

	for (int i = 0;i < HAND_CARD_NUM;++i)
	{
		if (m_byArHandPai[i] == 255)
			continue;
		hand[i] = m_byArHandPai[i];
	}
	for (int i = 0;i < HAND_CARD_NUM;++i)
	{
		if (hand[i] == 255)
			continue;
		if (hand[i] == m_byLastPai)//最后一张牌不算
		{
			hand[i] = 255;
			break;
		}
	}
	///从小到大排序
	CLogicBase::SelectSort(hand, HAND_CARD_NUM, false);

	for (int i = 0;i < HAND_CARD_NUM;++i)
	{
		if (hand[i] == 255)
			continue;
		if (hand[i] >= CMjEnum::MJ_TYPE_FD)//不能有字牌
			return false;
		num[hand[i] % 10 - 1]++;
		if (type == 255)
		{
			type = hand[i] / 10;
		}
		if (type != hand[i] / 10)//必须是清一色
		{
			return false;
		}
	}
	for (int i = 0;i < 9;i++)
	{
		if (i == 0 || i == 8)
		{
			if (num[i] < 3)
			{
				return false;
			}
		}
		else
		{
			if (num[i] != 1)
			{
				return false;
			}
		}
	}
	return true;
}

/////////////////////////特殊牌型///////////////////////////////////////////////////////////////////////////////////////////////
//全不靠：由单张3种花色147、258、369不能错位的序数牌及东南西北中发白中的任何14张牌组成的和牌（十三烂）。不计五门齐、不求人、单钓(非平糊类型)
bool CheckHuPai::CheckQuanBuKao(GameDataEx *pGameData)
{
	BYTE num = 0;
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		num++;
	}
	if (num > 0)//不能有吃碰杠
	{
		return false;
	}
	BYTE pai = 255;
	for (int i = 0;i < m_tHuTempData.count;++i)
	{
		if (m_tHuTempData.data[i][1] > 1)
			return false;
		if (m_tHuTempData.data[i][0] >= CMjEnum::MJ_TYPE_FD)
		{
			continue;
		}
		if (pai == 255 || pai / 10 != m_tHuTempData.data[i][0] / 10)
		{
			pai = m_tHuTempData.data[i][0];
		}
		else
		{
			if ((m_tHuTempData.data[i][0] - pai) < 3)
			{
				return false;
			}
		}
	}
	return true;
}

//组合龙：3种花色的147、258、369不能错位的序数牌 ，
bool CheckHuPai::CheckZuHeLong(GameDataEx *pGameData)
{
	if (pGameData->GetGCPCount(m_byStation) > 0)//不能有吃碰杠
	{
		return false;
	}

	return false;
}

//此七对可以带癞子
//七对：由7个对子组成和牌。不计不求人、单钓  (非平糊类型)
bool CheckHuPai::CheckQiDui(GameDataEx *pGameData)
{
	if (pGameData->GetGCPCount(m_byStation) > 0)//不能有吃碰杠
	{
		return false;
	}
	int byTempJingNum = m_byJingNum;
	for (int i = 0;i < m_tNoJingHuTempData.count;++i)
	{
		if ((m_tNoJingHuTempData.data[i][1] & 1) == 1)
		{
			byTempJingNum--;
			if (byTempJingNum < 0)
			{
				return false;
			}
		}
	}
	return true;
}

//龙七对
bool CheckHuPai::CheckLongQiDui(GameDataEx *pGameData)
{
	BYTE byPaiNums[50];
	memset(byPaiNums, 0, sizeof(byPaiNums));
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (255 == m_byArHandPai[i])
		{
			continue;
		}
		byPaiNums[m_byArHandPai[i]]++;
	}
	int iLongNums = 0;
	for (int i = 0;i < 50;i++)
	{
		if (4 == byPaiNums[i])
		{
			iLongNums++;
		}
	}
	if (CheckQiDui(pGameData) && iLongNums >= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//清一色七对
bool CheckHuPai::CheckQingYiSeQiDui(GameDataEx *pGameData)
{
	if (CheckQiDui(pGameData) && CheckQingYiSe(pGameData))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//七星不靠：必须有7个单张的东西南北中发白，加上3种花色，数位按147、258、369中的7张序数牌组成没有将牌的和牌。不计五门齐、不求人、单钓 (非平糊类型)
bool CheckHuPai::CheckQiXingBuKao(GameDataEx *pGameData)
{
	if (pGameData->GetGCPCount(m_byStation) > 0)//不能有吃碰杠
	{
		return false;
	}
	BYTE num = 0;
	BYTE pai = 255;
	for (int i = 0;i < m_tHuTempData.count;++i)
	{
		if (m_tHuTempData.data[i][1] > 1)
			return false;
		if (m_tHuTempData.data[i][0] >= CMjEnum::MJ_TYPE_FD)
		{
			num++;
			continue;
		}
		if (pai == 255 || pai / 10 != m_tHuTempData.data[i][0] / 10)
		{
			pai = m_tHuTempData.data[i][0];
		}
		else
		{
			if ((m_tHuTempData.data[i][0] - pai) < 3)
			{
				return false;
			}
		}
	}
	if (num == 7)
	{
		return true;
	}
	return false;
}

//连七对： 由一种花色序数牌组成序数相连的7个对子的和牌。不计清一色、不求人、单钓 （不包含吃碰牌）
bool CheckHuPai::CheckLianQiDui(GameDataEx *pGameData)
{
	if (!CheckQiDui(pGameData))//必须是七对
	{
		return false;
	}
	BYTE duizi[9], num = 0, pai = 255;
	memset(duizi, 255, sizeof(duizi));
	for (int i = 0;i < m_tHuTempData.count;++i)
	{
		if (m_tHuTempData.data[i][1] == 0)
			continue;
		if (m_tHuTempData.data[i][1] != 2)
			return false;
		duizi[num] = m_tHuTempData.data[i][0];
		num++;
	}
	pai = duizi[0];
	if (duizi[1] - 1 == pai && duizi[2] - 2 == pai && duizi[3] - 3 == pai && duizi[4] - 4 == pai && duizi[5] - 5 == pai && duizi[6] - 6 == pai)
	{
		return true;
	}

	return false;
}

//十三幺：由3种序数牌的一、九牌，7种字牌及其中一对作将组成的和牌。不计五门齐、不求人、单钓 （不包含吃碰牌）
bool CheckHuPai::CheckShiSanYao(GameDataEx *pGameData)
{
	if (pGameData->GetGCPCount(m_byStation) > 0)//不能有吃碰杠
	{
		return false;
	}
	int pai[38], duizi = 0;
	memset(pai, 0, sizeof(pai));
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255)
			continue;
		if (m_byArHandPai[i] < CMjEnum::MJ_TYPE_FD && m_byArHandPai[i] % 10 != 1 && m_byArHandPai[i] % 10 != 9)
		{
			return false;
		}
		pai[m_byArHandPai[i]]++;
	}
	for (int i = 0;i < CMjEnum::MJ_TYPE_BAI + 1;++i)
	{
		if (i % 10 == 0 || (i < CMjEnum::MJ_TYPE_FD && i % 10 != 1 && i % 10 != 9))
			continue;
		if (pai[i] <= 0)
		{
			return false;
		}
		if (pai[i] > 1)
		{
			duizi++;
		}
		if (duizi >= 2)
		{
			return false;
		}
	}
	return true;
}

//大飘(源自江西麻将)
bool CheckHuPai::CheckDaPiao(GameDataEx *pGameData)
{
	BYTE byPaiNums[50];
	memset(byPaiNums, 0, sizeof(byPaiNums));
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (255 == m_byArHandPai[i])
		{
			continue;
		}
		byPaiNums[m_byArHandPai[i]]++;
	}
	int iAnKeNums = 0;
	for (int i = 0;i < 50;i++)
	{
		if (3 == byPaiNums[i])
		{
			iAnKeNums++;
		}
	}
	if (CheckQiDui(pGameData) && 4 == iAnKeNums)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//检测将对（258碰碰胡）
bool CheckHuPai::CheckJiangDui(GameDataEx *pGameData)
{
	for (int i = 0;i < m_tHuPaiStruct.count;++i)
	{
		if (m_tHuPaiStruct.byType[i] == TYPE_SHUN_ZI)//顺子
		{
			return false;
		}
		if (m_tHuPaiStruct.data[i][0] % 10 != 2 && m_tHuPaiStruct.data[i][0] % 10 != 5 && m_tHuPaiStruct.data[i][0] % 10 != 8)
		{
			return false;
		}
	}
	for (int i = 0;i < 5;i++)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		BYTE pai = 255;
		pai = pGameData->m_UserGCPData[m_byStation][i].byData[0];
		switch (pGameData->m_UserGCPData[m_byStation][i].byType)
		{
		case ACTION_CHI://吃牌动作
		{
			return false;
		}
		break;
		case ACTION_AN_GANG:
		case ACTION_MING_GANG:
		case ACTION_BU_GANG:
		case ACTION_PENG:
		{
			if (pai % 10 != 2 && pai % 10 != 5 && pai % 10 != 8)
			{
				return false;
			}
		}
		break;
		}
	}
	return true;
}

//混幺九
bool CheckHuPai::CheckYiJiuHu(GameDataEx *pGameData)
{
	//手牌检测
	bool bIsHaveYiJiu = false;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255)
			continue;
		BYTE byDian = CMjRef::GetCardPaiDian(m_byArHandPai[i]);
		if (m_byArHandPai[i] < CMjEnum::MJ_TYPE_FD && byDian != 1 && byDian != 9)
		{
			return false;
		}
		if (CMjRef::IsYiJiuPai(m_byArHandPai[i]))
		{
			bIsHaveYiJiu = true;
		}
	}
	//吃碰杠检测
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		for (int j = 0; j < 4; j++)
		{
			BYTE byPai = pGameData->m_UserGCPData[m_byStation][i].byData[j];
			if (byPai == 255)
			{
				continue;
			}
			BYTE byDian = CMjRef::GetCardPaiDian(byPai);
			if (byPai < CMjEnum::MJ_TYPE_FD && byDian != 1 && byDian != 9)
			{
				return false;
			}
			if (CMjRef::IsYiJiuPai(m_byArHandPai[i]))
			{
				bIsHaveYiJiu = true;
			}
		}
	}
	if (!bIsHaveYiJiu)
	{
		return false;
	}
	return true;
}

//清幺九
bool CheckHuPai::CheckQingYiJiuHu(GameDataEx *pGameData)
{
	//手牌检测
	bool bIsHaveYiJiu = false;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255)
			continue;
		BYTE byDian = CMjRef::GetCardPaiDian(m_byArHandPai[i]);
		if (m_byArHandPai[i] >= CMjEnum::MJ_TYPE_FD || (byDian != 1 && byDian != 9))
		{
			return false;
		}
		if (CMjRef::IsYiJiuPai(m_byArHandPai[i]))
		{
			bIsHaveYiJiu = true;
		}
	}
	//吃碰杠检测
	for (int i = 0;i < 5;++i)
	{
		if (ACTION_NO == pGameData->m_UserGCPData[m_byStation][i].byType)
			continue;
		for (int j = 0; j < 4; j++)
		{
			BYTE byPai = pGameData->m_UserGCPData[m_byStation][i].byData[j];
			if (byPai == 255)
				continue;
			BYTE byDian = CMjRef::GetCardPaiDian(byPai);
			if (byPai >= CMjEnum::MJ_TYPE_FD || (byDian != 1 && byDian != 9))
			{
				return false;
			}
			if (CMjRef::IsYiJiuPai(m_byArHandPai[i]))
			{
				bIsHaveYiJiu = true;
			}
		}
	}
	if (!bIsHaveYiJiu)
	{
		return false;
	}
	return true;
}

//检测所有的牌是否是全是258，将一色
bool CheckHuPai::QiangQiangHu258(GameDataEx *pGameData)
{
	int iPaiValue = 0;
	for (int i = 0;i < HAND_CARD_NUM;i++)
	{
		if (m_byArHandPai[i] == 255 || m_byArHandPai[i] == 0 || pGameData->m_StrJing.CheckIsJing(m_byArHandPai[i]))
		{
			continue;
		}
		iPaiValue = m_byArHandPai[i] % 10;
		if (!((iPaiValue == 2) || (iPaiValue == 5) || (iPaiValue == 8)))
		{
			return false;
		}
	}
	for (int i = 0;i < 5;i++)
	{
		if (pGameData->m_UserGCPData[m_byStation][i].byType != 255)
		{
			if (ACTION_CHI == pGameData->m_UserGCPData[m_byStation][i].byType)		//因为2，5，8不可能组成顺子
			{
				return false;
			}
			iPaiValue = pGameData->m_UserGCPData[m_byStation][i].byData[0] % 10;
			if (!((iPaiValue == 2) || (iPaiValue == 5) || (iPaiValue == 8)))
			{
				return false;
			}
		}
	}
	return true;
}

//一条龙，当胡牌时手中有不同花色的123456789顺子称为一条龙
bool CheckHuPai::CheckYiTiaoLong()
{
	if (m_byHandShunNum < 3)
	{
		return false;
	}
	CLogicBase::SelectSort(m_byHandShunData, MAX_CPG_NUM, false);
	bool bHave1 = false, bHave4 = false, bHave7 = false;
	BYTE byHua1[MAX_CPG_NUM], byHua4[MAX_CPG_NUM], byHua7[MAX_CPG_NUM];
	memset(byHua1, 255, sizeof(byHua1));
	memset(byHua4, 255, sizeof(byHua4));
	memset(byHua7, 255, sizeof(byHua7));
	int iHua1Count = 0, iHua4Count = 0, iHua7Count = 0;
	for (int i = 0; i < m_byHandShunNum; i++)
	{
		int iSize = CMjRef::GetCardPaiDianEx(m_byHandShunData[i]);
		int iHua = CMjRef::GetHuaSe(m_byHandShunData[i]);
		if (iHua >= 3)
		{
			return false;
		}
		if (iSize == 1)
		{
			bHave1 = true;
			byHua1[iHua1Count++] = iHua;
		}
		else if (iSize == 4)
		{
			bHave4 = true;
			byHua4[iHua4Count++] = iHua;
		}
		else if (iSize == 7)
		{
			bHave7 = true;
			byHua7[iHua7Count++] = iHua;
		}
	}
	if (bHave1 && bHave4 && bHave7)
	{
		for (int i = 0; i < 3; i++)
		{
			int iCount = 0;
			for (int j = 0; j < iHua1Count; j++)
			{
				if (byHua1[j] == i)
				{
					iCount++;
					break;
				}
			}
			for (int j = 0; j < iHua4Count; j++)
			{
				if (byHua4[j] == i)
				{
					iCount++;
					break;
				}
			}
			for (int j = 0; j < iHua7Count; j++)
			{
				if (byHua7[j] == i)
				{
					iCount++;
					break;
				}
			}
			if (iCount == 3)
			{
				return true;
			}
		}
	}
	return false;
}