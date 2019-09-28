#ifndef MJ_H
#define MJ_H
#define MAKE_A_MJ(hs,pd)  ( (hs) * 10 +(pd))
#define MAX_MJ_KIND_NUM		34		//最大麻将种类数量
#define MAX_MJ_HUA_NUM		8		//花牌种类数量
#define MAX_MJ_PAI_COUNT    144     //一副麻将牌最大数量，目前不能超过255
#define MAX_JING_PAI_NUM    8       //最大精牌数量（精牌指万能牌，癞子）
#define MAX_CPG_NUM         5		//最大吃碰杠数量
//抓牌顺时针，打牌逆时针
class CMjEnum
{
public:
	enum MJ_TYPE_HUA_SE//牌类型
	{
		MJ_TYPE_PAI_NONE = -10,//无花色
		MJ_TYPE_PAI_WAN = 0,//万
		MJ_TYPE_PAI_TIAO = 1,//条
		MJ_TYPE_PAI_BING = 2,//同
		MJ_TYPE_PAI_FENG = 3,//风
		MJ_TYPE_PAI_HUA = 4//花
	};
	enum MJ_TYPE_PAI_DIAN//牌点
	{
		MJ_TYPE_PAI_DIAN_NONE = -1,
		MJ_TYPE_PAI_DIAN_1 = 1,
		MJ_TYPE_PAI_DIAN_2 = 2,
		MJ_TYPE_PAI_DIAN_3 = 3,
		MJ_TYPE_PAI_DIAN_4 = 4,
		MJ_TYPE_PAI_DIAN_5 = 5,
		MJ_TYPE_PAI_DIAN_6 = 6,
		MJ_TYPE_PAI_DIAN_7 = 7,
		MJ_TYPE_PAI_DIAN_8 = 8,
		MJ_TYPE_PAI_DIAN_9 = 9
	};
	enum MJ_TYPE
	{
		MJ_TYPE_NONE = 0,
		MJ_TYPE_FCHUN = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 11),
		MJ_TYPE_FXIA = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 12),
		MJ_TYPE_FQIU = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 13),
		MJ_TYPE_FDONG = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 14),
		MJ_TYPE_FMEI = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 15),
		MJ_TYPE_FLAN = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 16),
		MJ_TYPE_FZHU = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 17),
		MJ_TYPE_FJU = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 18),

		MJ_TYPE_CAISHEN = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 19),
		MJ_TYPE_YUANBAO = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 20),
		MJ_TYPE_MAO = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 21),
		MJ_TYPE_LAOXU = MAKE_A_MJ(MJ_TYPE_PAI_FENG, 22),

		MJ_TYPE_FD = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_1),///东
		MJ_TYPE_FN = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_2),///南
		MJ_TYPE_FX = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_3),///西
		MJ_TYPE_FB = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_4),///北
		MJ_TYPE_ZHONG = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_5),///红中
		MJ_TYPE_FA = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_6),///发财
		MJ_TYPE_BAI = MAKE_A_MJ(MJ_TYPE_PAI_FENG, MJ_TYPE_PAI_DIAN_7),///白板

		MJ_TYPE_W1 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_1),///万
		MJ_TYPE_W2 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_2),
		MJ_TYPE_W3 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_3),
		MJ_TYPE_W4 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_4),
		MJ_TYPE_W5 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_5),
		MJ_TYPE_W6 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_6),
		MJ_TYPE_W7 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_7),
		MJ_TYPE_W8 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_8),
		MJ_TYPE_W9 = MAKE_A_MJ(MJ_TYPE_PAI_WAN, MJ_TYPE_PAI_DIAN_9),

		MJ_TYPE_T1 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_1),///条
		MJ_TYPE_T2 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_2),
		MJ_TYPE_T3 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_3),
		MJ_TYPE_T4 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_4),
		MJ_TYPE_T5 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_5),
		MJ_TYPE_T6 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_6),
		MJ_TYPE_T7 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_7),
		MJ_TYPE_T8 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_8),
		MJ_TYPE_T9 = MAKE_A_MJ(MJ_TYPE_PAI_TIAO, MJ_TYPE_PAI_DIAN_9),

		MJ_TYPE_B1 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_1),///同
		MJ_TYPE_B2 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_2),
		MJ_TYPE_B3 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_3),
		MJ_TYPE_B4 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_4),
		MJ_TYPE_B5 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_5),
		MJ_TYPE_B6 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_6),
		MJ_TYPE_B7 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_7),
		MJ_TYPE_B8 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_8),
		MJ_TYPE_B9 = MAKE_A_MJ(MJ_TYPE_PAI_BING, MJ_TYPE_PAI_DIAN_9)
	};
};
static int stcArr_A_Mj[] =
{
	CMjEnum::MJ_TYPE_W1,
	CMjEnum::MJ_TYPE_W2,
	CMjEnum::MJ_TYPE_W3,
	CMjEnum::MJ_TYPE_W4,
	CMjEnum::MJ_TYPE_W5,
	CMjEnum::MJ_TYPE_W6,
	CMjEnum::MJ_TYPE_W7,
	CMjEnum::MJ_TYPE_W8,
	CMjEnum::MJ_TYPE_W9,

	CMjEnum::MJ_TYPE_B1 ,
	CMjEnum::MJ_TYPE_B2 ,
	CMjEnum::MJ_TYPE_B3 ,
	CMjEnum::MJ_TYPE_B4,
	CMjEnum::MJ_TYPE_B5,
	CMjEnum::MJ_TYPE_B6,
	CMjEnum::MJ_TYPE_B7,
	CMjEnum::MJ_TYPE_B8,
	CMjEnum::MJ_TYPE_B9,


	CMjEnum::MJ_TYPE_T1,
	CMjEnum::MJ_TYPE_T2,
	CMjEnum::MJ_TYPE_T3,
	CMjEnum::MJ_TYPE_T4,
	CMjEnum::MJ_TYPE_T5,
	CMjEnum::MJ_TYPE_T6,
	CMjEnum::MJ_TYPE_T7,
	CMjEnum::MJ_TYPE_T8,
	CMjEnum::MJ_TYPE_T9,

	CMjEnum::MJ_TYPE_FD,
	CMjEnum::MJ_TYPE_FN,
	CMjEnum::MJ_TYPE_FX,
	CMjEnum::MJ_TYPE_FB,
	CMjEnum::MJ_TYPE_ZHONG,
	CMjEnum::MJ_TYPE_FA,
	CMjEnum::MJ_TYPE_BAI

};
//麻将的万
static int stcArr_A_Mj_W[] =
{
	CMjEnum::MJ_TYPE_W1,
	CMjEnum::MJ_TYPE_W2,
	CMjEnum::MJ_TYPE_W3,
	CMjEnum::MJ_TYPE_W4,
	CMjEnum::MJ_TYPE_W5,
	CMjEnum::MJ_TYPE_W6,
	CMjEnum::MJ_TYPE_W7,
	CMjEnum::MJ_TYPE_W8,
	CMjEnum::MJ_TYPE_W9
};
//麻将的条
static int stcArr_A_Mj_T[] =
{
	CMjEnum::MJ_TYPE_T1,
	CMjEnum::MJ_TYPE_T2,
	CMjEnum::MJ_TYPE_T3,
	CMjEnum::MJ_TYPE_T4,
	CMjEnum::MJ_TYPE_T5,
	CMjEnum::MJ_TYPE_T6,
	CMjEnum::MJ_TYPE_T7,
	CMjEnum::MJ_TYPE_T8,
	CMjEnum::MJ_TYPE_T9
};
//麻将的筒
static int stcArr_A_Mj_B[] =
{
	CMjEnum::MJ_TYPE_B1 ,
	CMjEnum::MJ_TYPE_B2 ,
	CMjEnum::MJ_TYPE_B3 ,
	CMjEnum::MJ_TYPE_B4,
	CMjEnum::MJ_TYPE_B5,
	CMjEnum::MJ_TYPE_B6,
	CMjEnum::MJ_TYPE_B7,
	CMjEnum::MJ_TYPE_B8,
	CMjEnum::MJ_TYPE_B9
};
//麻将的风牌
static	int	stcArr_A_Mj_FENG[] =
{
	CMjEnum::MJ_TYPE_FD,//东风
	CMjEnum::MJ_TYPE_FN,//南风
	CMjEnum::MJ_TYPE_FX,//西风
	CMjEnum::MJ_TYPE_FB //北风
};
//麻将的字牌
static	int	stcArr_A_Mj_ZI[] =
{
	CMjEnum::MJ_TYPE_ZHONG,	//红中
	CMjEnum::MJ_TYPE_FA,	//发财
	CMjEnum::MJ_TYPE_BAI	//白板

};
//麻将的花牌
static int stcArr_A_Mj_Other[] =
{
	CMjEnum::MJ_TYPE_FCHUN,//春
	CMjEnum::MJ_TYPE_FXIA, //夏
	CMjEnum::MJ_TYPE_FQIU, //秋
	CMjEnum::MJ_TYPE_FDONG,//冬

	CMjEnum::MJ_TYPE_FMEI, //梅
	CMjEnum::MJ_TYPE_FLAN, //兰
	CMjEnum::MJ_TYPE_FZHU, //竹
	CMjEnum::MJ_TYPE_FJU   //菊
};
//糊牌类型
enum HUPAI_TYPE//胡牌类型枚举结构
{
	HUPAI_TYPE_None = 0,		////没糊	
	HUPAI_TYPE_HuPai = 1,		////成牌
	HUPAI_TYPE_ZiMo = 2,		////自摸	
	HUPAI_TYPE_PingHu = 3,		////平糊	
	HUPAI_TYPE_TianHu = 4,		////天糊	
	HUPAI_TYPE_DiHu = 5,		////地糊	
	HUPAI_TYPE_GangKai = 6,		//杠上花	
	HUPAI_TYPE_GangHaoPao = 7,		//杠后炮
	HUPAI_TYPE_YiBanGao = 8,		//一般高	
	HUPAI_TYPE_XiXiangFeng = 9,		//喜相逢	
	HUPAI_TYPE_LianLiu = 10,	//连六		
	HUPAI_TYPE_LaoShaoFu = 11,	//老少副	
	HUPAI_TYPE_YaoJiuKe = 12,	//幺九刻	
	HUPAI_TYPE_MingGang = 13,	//明杠
	HUPAI_TYPE_QueYiMen = 14,	//缺一门	
	HUPAI_TYPE_WuZi = 15,	//无字
	HUPAI_TYPE_BianZhang = 16,	//边张		
	HUPAI_TYPE_KanZhang = 17,	//坎张		
	HUPAI_TYPE_DanDiaoJiang = 18,	//单钓将
	HUPAI_TYPE_JianKe = 19,	//箭刻
	HUPAI_TYPE_QuanFengKe = 20,	//圈风刻
	HUPAI_TYPE_MenFengKe = 21,	//门风刻
	HUPAI_TYPE_MenQianQing = 22,	//门前清	
	HUPAI_TYPE_SiGuiYi = 23,	//四归一	
	HUPAI_TYPE_ShuangTongKe = 24,	//双同刻	
	HUPAI_TYPE_ShuangAnKe = 25,	//双暗刻	
	HUPAI_TYPE_AnGang = 26,	//暗杠
	HUPAI_TYPE_DuanYao = 27,	//断幺		
	HUPAI_TYPE_QuanDaiYao = 28,	//全带幺	
	HUPAI_TYPE_BuQiuRen = 29,	//不求人	
	HUPAI_TYPE_ShuangMingGang = 30,	//双明杠	
	HUPAI_TYPE_HuJueZhang = 31,	//和绝张	
	HUPAI_TYPE_PengPengHu = 32,	//碰碰和	
	HUPAI_TYPE_HunYiSe = 33,	//混一色
	HUPAI_TYPE_SanSeSanBuGao = 34,	//三色三步高
	HUPAI_TYPE_WuMenQi = 35,	//五门齐
	HUPAI_TYPE_QuanQiuRen = 36,	//全求人	
	HUPAI_TYPE_ShuangAnGang = 37,	//双暗杠	
	HUPAI_TYPE_ShuangJianKe = 38,	//双箭刻
	HUPAI_TYPE_HuaLong = 39,	//花龙		
	HUPAI_TYPE_TuiBuDao = 40,	//推不倒
	HUPAI_TYPE_SanSeSanTongShun = 41,	//三色三同顺
	HUPAI_TYPE_SanSeSanJieGao = 42,	//三色三节高
	HUPAI_TYPE_WuFanHu = 43,	//无番和	
	HUPAI_TYPE_MiaoShouHuiChun = 44,	//妙手回春	
	HUPAI_TYPE_HaiDiLaoYue = 45,	//海底捞月	
	HUPAI_TYPE_QiangGang = 46,	//抢杠和	
	HUPAI_TYPE_QuanBuKao = 47,	//全不靠(十三烂)
	HUPAI_TYPE_ZuHeLong = 48,	//组合龙	
	HUPAI_TYPE_DaYuWu = 49,	//大于五	
	HUPAI_TYPE_XiaoYuWu = 50,	//小于五	
	HUPAI_TYPE_SanFengKe = 51,	//三风刻
	HUPAI_TYPE_QingLong = 52,	//清龙		
	HUPAI_TYPE_SanSeShuangLongHui = 53,	//三色双龙会
	HUPAI_TYPE_YiSeSanBuGao = 54,	//一色三步高
	HUPAI_TYPE_QuanDaiWu = 55,	//全带五	
	HUPAI_TYPE_SanTongKe = 56,	//三同刻	
	HUPAI_TYPE_SanAnKe = 57,	//三暗刻	
	HUPAI_TYPE_QiDui = 58,	//七对		
	HUPAI_TYPE_QiXingBuKao = 59,	//七星不靠
	HUPAI_TYPE_QuanShuangKe = 60,	//全双刻	
	HUPAI_TYPE_QingYiSe = 61,	//清一色	
	HUPAI_TYPE_YiSeSanTongShun = 62,	//一色三同顺
	HUPAI_TYPE_YiSeSanJieGao = 63,	//一色三节高
	HUPAI_TYPE_QuanDa = 64,	//全大		
	HUPAI_TYPE_QuanZhong = 65,	//全中		
	HUPAI_TYPE_QuanXiao = 66,	//全小		
	HUPAI_TYPE_YiSeSiBuGao = 67,	//一色四步高
	HUPAI_TYPE_SanGang = 68,	//三杠		
	HUPAI_TYPE_HunYaoJiu = 69,	//混幺九
	HUPAI_TYPE_YiSeSiTongShun = 70,	//一色四同顺
	HUPAI_TYPE_YiSeSiJieGao = 71,	//一色四节高
	HUPAI_TYPE_QingYaoJiu = 72,	//清幺九	
	HUPAI_TYPE_XiaoSiXi = 73,	//小四喜
	HUPAI_TYPE_XiaoSanYuan = 74,	//小三元
	HUPAI_TYPE_ZiYiSe = 75,	//字一色
	HUPAI_TYPE_SiAnKe = 76,	//四暗刻	
	HUPAI_TYPE_YiSeShuangLongHui = 77,	//一色双龙会
	HUPAI_TYPE_DaSiXi = 78,	//大四喜
	HUPAI_TYPE_DaSanYuan = 79,	//大三元
	HUPAI_TYPE_LvYiSe = 80,	//绿一色
	HUPAI_TYPE_JiuLianBaoDeng = 81,	//九莲宝灯
	HUPAI_TYPE_SiGang = 82,	//四杠（十八罗汉）	
	HUPAI_TYPE_LianQiDui = 83,	//连七对			
	HUPAI_TYPE_ShiSanYao = 84,	//十三幺
	HUPAI_TYPE_HUA = 85,    //花牌
	HUPAI_TYPE_QingYiSeQiDui = 86,	//清一色七对
	HUPAI_TYPE_QingYiSeYiTiaoLong = 87,	//清一色一条龙
	HUPAI_TYPE_LongQiDui = 88,	//龙七对
	HUPAI_TYPE_DaPiao = 89,	//大飘
	HUPAI_TYPE_JingHu = 90,	//4个癞子胡
};

//组牌类型
enum MJ_GROUP_TYPE
{
	TYPE_JIANG_PAI = 15,				//将牌类型
	TYPE_JINGDIAO_JIANG = 16,				//精钓将牌类型
	TYPE_SHUN_ZI = 17,				//顺子类型
	TYPE_AN_KE = 18,				//暗刻类型
	TYPE_JING_KE = 19,				//财神刻子类型
};
class CMjRef
{
public:
	CMjRef() {}
	virtual ~CMjRef() {}
	static bool IsValidateMj(BYTE iCard)//合法牌值
	{
		return iCard >= CMjEnum::MJ_TYPE_FCHUN && iCard <= CMjEnum::MJ_TYPE_LAOXU ||
			iCard >= CMjEnum::MJ_TYPE_FD && iCard <= CMjEnum::MJ_TYPE_BAI ||
			iCard >= CMjEnum::MJ_TYPE_B1 && iCard <= CMjEnum::MJ_TYPE_B9 ||
			iCard >= CMjEnum::MJ_TYPE_W1 && iCard <= CMjEnum::MJ_TYPE_W9 ||
			iCard >= CMjEnum::MJ_TYPE_T1 && iCard <= CMjEnum::MJ_TYPE_T9;
	}
	//获取牌的花色
	static BYTE GetHuaSe(BYTE byCard)
	{
		return (byCard) / 10;
	}
	//获取牌的牌点
	static BYTE GetCardPaiDian(BYTE byCard)
	{
		return (byCard) % 10;
	}
	//获取牌的牌点
	static BYTE GetCardPaiDianEx(BYTE byCard)
	{
		if (byCard >= CMjEnum::MJ_TYPE_FD)
		{
			return 0;
		}
		return (byCard) % 10;
	}
	//判断牌是不是1,9
	static bool IsYiJiuPai(BYTE byCard)
	{
		if (byCard >= CMjEnum::MJ_TYPE_FD)
		{
			return false;
		}
		if (byCard % 10 != 9 && byCard % 10 != 1)
		{
			return false;
		}
		return true;
	}
	//检测2张牌是否同一花色
	static bool CheckIsTongSe(BYTE Card1, BYTE Card2)
	{
		return (Card1 / 10 == Card2 / 10);
	}
	//检测2张牌是否一相连
	static  bool CheckIsYiXiangLian(BYTE Card1, BYTE Card2, bool big = false)
	{
		if (!CheckIsTongSe(Card1, Card2))
			return false;
		if (!big)
			return (Card1 == Card2 + 1);
		return (Card1 == Card2 - 1);
	}
	//检测2张牌是否二相连
	static  bool CheckIsErXiangLian(BYTE Card1, BYTE Card2, bool big = false)
	{
		if (!CheckIsTongSe(Card1, Card2))
			return false;
		if (!big)
			return (Card1 == Card2 + 1 || Card1 == Card2 + 2);
		return (Card1 == Card2 - 1 || Card1 == Card2 - 2);
	}

	static BYTE ExDToB(BYTE byMJ)//逻辑数据转换成控件数据
	{
		if (byMJ == 0)
		{
			return 34;
		}
		else if (byMJ <= 9)	//“万”转换
		{
			return (byMJ - 1);
		}
		else if (byMJ <= 19)	//“同”转换
		{
			return (byMJ + 7);
		}
		else if (byMJ <= 29)	//“条”转换
		{
			return (byMJ - 12);
		}
		else if (byMJ <= 37)	//风牌、“中发白”转换
		{
			if (byMJ == 31)//东
			{
				return 27;
			}
			else if (byMJ == 32)//南
			{
				return 29;
			}
			else if (byMJ == 33)//西
			{
				return 28;
			}
			else if (byMJ == 34)//北
			{
				return 30;
			}
			else
			{
				return byMJ - 4;
			}
		}
		else if (byMJ <= 48)	//“春夏秋冬梅兰竹菊”转换
		{
			return (byMJ - 5);
		}
		return 255;
	}

	static BYTE ExBToD(BYTE byMJ)//控件数据转换成逻辑数据
	{
		if (byMJ < 0)
		{
			return byMJ;
		}
		else if (byMJ <= 8)	//“万”转换
		{
			return (byMJ + 1);
		}
		else if (byMJ <= 17)	//“条”转换
		{
			return (byMJ + 12);
		}
		else if (byMJ <= 26)	//“同”转换
		{
			return (byMJ - 7);
		}
		else if (byMJ <= 33)	//风牌、“中发白”转换
		{
			if (byMJ == 27)//东
			{
				return 31;
			}
			else if (byMJ == 29)//南
			{
				return 32;
			}
			else if (byMJ == 28)//西
			{
				return 33;
			}
			else if (byMJ == 30)//北
			{
				return 34;
			}
			else
			{
				return (byMJ + 4);
			}
		}
		else if (byMJ == 34) //空牌与背牌
		{
			return 0;
		}
		else if (byMJ <= 43)	//“春夏秋冬梅兰竹菊”转换
		{
			return (byMJ + 5);
		}
		return 255;
	}

	static bool ExDToB(BYTE* byT, const BYTE* byS, int nMJSize)//逻辑麻将转换成控件麻将数据（批量转换）
	{
		if (!byT || !byS)
			return false;
		for (int i = 0; i < nMJSize; i++)
		{
			byT[i] = CMjRef::ExDToB(byS[i]);
		}
		return true;
	}
};
#endif