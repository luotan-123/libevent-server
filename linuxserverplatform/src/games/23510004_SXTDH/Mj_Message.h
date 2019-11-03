#ifndef MJ_MESSAGE_H
#define MJ_MESSAGE_H
#include "mj.h"
#include "upgrademessage.h"
#include "CLogicBase.h"

#pragma pack(1) //1字节对齐 

/*--------------------------------------------------------------------*/
///麻将常规属性
struct MjRuleSet
{
	bool	bHaveWan;			//是否有万		
	bool	bHaveTiao;			//是否有条		
	bool	bHaveBing;			//是是有柄		
	bool    bHaveHongZhong;		//是否有红中	
	bool    bHaveFaCai;			//是否有发财	
	bool    bHaveBaiBan;		//是否有白板	
	bool    bHaveFengPai;		//是否有东南西北
	bool    bHaveFlower;		//是否有花牌	
	bool	bHaveJing;			//有无财神		
	bool    bNetCutTuoGuan;		//是否断线托管
	bool	bForceTuoGuan;		//是否强退托管
	bool	bAutoBegin;			//时间到了是否自动开始	  
	BYTE    byAllMjCount;		//麻将的数量 
	BYTE	byHandCardNum;		//手牌张数
	BYTE	byGamePassType;		//流局剩牌留下
	BYTE	byGamePassNum;		//流局牌数
	BYTE	byNextNtType;		//下局庄家的确定类型	
	BYTE    byMenPaiNum[4];		//各家门牌数
	BYTE    byAutoOutTime;		//自动出牌时间	
	BYTE    byOutTime;			//出牌时间		
	BYTE    byBlockTime;		//拦牌思考时间	
	BYTE    byBeginTime;		//开始等待时间	
	int		iGameBase;			//底分
	bool	bIsDuoXiang;        //是否一炮多响
	bool	bIsSuanGangHu;      //是否计算抢杠胡所得杠分
	bool    bChaoShiTuoGuan;    //金币场超时托管
	bool    bDuanXianTuoGuan;   //非房卡场，断线是否托管
	bool	bIsMatchMJ;			//是否配牌
	MjRuleSet()
	{
		memset(this, 0, sizeof(MjRuleSet));
	}

};
/*--------------------------------------------------------------------*/
///动作属性
struct ActionRuleSet
{
	bool bChi;			//吃
	bool bChiFeng;		//吃风牌
	bool bChiJian;		//吃中发白
	bool bChiJing;		//吃财神
	bool bPeng;			//碰
	bool bKan;			//坎
	bool bGang;			//杠
	bool bBuGang;		//补杠
	bool bTing;			//听
	bool bQiangChi;		//抢吃(吃后听牌)
	bool bGangAfterTing;//听牌后还能杠牌
	bool bSeziAfterGang;//杠后打色子抓牌
	bool bQxHh;			//不能弃先糊后（一圈内）
	bool bQiPeng;       //弃碰不能碰(一圈内)
	bool bQxHhBut;		//不能弃先糊后（一圈内），糊不同的牌可以，糊得更大可以
	bool bOutLianZhang;	//吃牌后不能出连张（出的牌不能与吃牌中自己的两张牌组成顺子）
	bool bQiangGangHu;	//抢杠胡
	bool bFangPao;		//是否能点炮
	bool bZiMo;			//是否能自摸
	bool bBuHua;        //是否需要补花
	ActionRuleSet()
	{
		memset(this, 0, sizeof(ActionRuleSet));
	}
};

///糊牌检测临时数据
struct CheckHuStruct
{
	BYTE  data[HAND_CARD_NUM][2];   //牌数据，0牌值，1张数
	BYTE  count;
	CheckHuStruct()
	{
		Init();
	}
	void Init()
	{
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			data[i][0] = 255;
			data[i][1] = 0;
		}
		count = 0;
	};
	void Add(BYTE pai)
	{
		if (GetPaiCount(pai) > 0)
		{
			for (int i = 0;i < HAND_CARD_NUM;i++)
			{
				if (data[i][0] == pai && pai != 255)
					data[i][1]++;
			}
		}
		else
		{
			data[count][0] = pai;
			data[count][1] = 1;
			count++;
		}
	};
	BYTE GetPaiCount(BYTE pai)
	{
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (data[i][0] == pai && pai != 255 && data[i][1] != 0)
				return data[i][1];
		}
		return 0;
	};
	BYTE GetDataCount()
	{
		return count;
	};
	BYTE GetAllPaiCount()
	{
		BYTE num = 0;
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (data[i][0] != 255)
				num += data[i][1];
		}
		return num;
	};
	void SetPaiCount(BYTE pai, BYTE num)
	{
		for (int i = 0;i < HAND_CARD_NUM;i++)
		{
			if (data[i][0] == pai && pai != 255)
			{
				data[i][1] = num;
			}
		}
	}
};

///平糊组牌数据结构
struct PingHuStruct
{
	BYTE  byType[6];    //组牌类型
	BYTE  data[6][4];   //数据
	bool  isjing[6][4];//是否财神替代
	BYTE  count;        //节点数
	PingHuStruct()
	{
		Init();
	};
	void Init()
	{
		memset(byType, 255, sizeof(byType));
		memset(data, 255, sizeof(data));
		memset(isjing, 0, sizeof(isjing));
		count = 0;
	};
	void AddData(BYTE type, BYTE pai[], bool jing[])
	{
		for (int i = 0;i < 6;i++)
		{
			if (byType[i] == 255)
			{
				byType[i] = type;
				data[i][0] = pai[0];data[i][1] = pai[1];data[i][2] = pai[2];data[i][3] = pai[3];
				isjing[i][0] = jing[0];isjing[i][1] = jing[1];isjing[i][2] = jing[2];isjing[i][3] = jing[3];
				count++;
				break;
			}
		}
	};
	void DeleteData(BYTE type, BYTE pai[], bool jing[])//删除元素
	{
		for (int i = 0;i < 6;i++)
		{
			if (byType[i] == type && data[i][0] == pai[0] && data[i][1] == pai[1] && isjing[i][0] == jing[0] && isjing[i][1] == jing[1])
			{
				byType[i] = 255;
				memset(data[i], 255, sizeof(data[i]));
				memset(isjing[i], 0, sizeof(isjing[i]));
				count--;
				break;
			}
		}
	};
	void SortData()
	{
		for (int i = 0;i < 5;i++)
		{
			if (byType[i] != 255)
				continue;
			for (int j = i;j < 5;j++)
			{
				byType[j] = byType[j + 1];
				memcpy(data[j], data[j + 1], sizeof(data[j]));//数据
				memcpy(isjing[j], isjing[j + 1], sizeof(isjing[j]));//是否财神替代
			}
		}
		count = 0;
		for (int i = 0;i < 5;i++)
		{
			if (byType[i] != 255)
				count++;
		}
	};
};

///财神数据结构
struct JingStruct
{
	BYTE  byPai[MAX_JING_PAI_NUM]; //每张精牌的牌值
	BYTE  byJingNum;               //精牌种类张数
	BYTE  byJingGen;
	JingStruct()
	{
		Init();
	}
	void Init()
	{
		memset(byPai, 255, sizeof(byPai));
		byJingNum = 0;
		byJingGen = 255;
	};
	bool CheckIsJing(BYTE pai)
	{
		if (pai == 255)
		{
			return false;
		}
		for (int i = 0;i < MAX_JING_PAI_NUM;i++)
		{
			if (byPai[i] == pai)
				return true;
		}
		return false;
	};
	bool AddJing(BYTE pai)
	{
		if (!CMjRef::IsValidateMj(pai))
		{
			return false;
		}
		for (int i = 0;i < MAX_JING_PAI_NUM;i++)
		{
			if (byPai[i] != 255 && byPai[i] == pai) //已经有这种精牌了
			{
				return false;
			}
			if (byPai[i] == 255)
			{
				byPai[i] = pai;
				byJingNum++;
				return true;
			}
		}
		return false;
	};
	bool DeleteJing(BYTE pai)
	{
		for (int i = 0;i < MAX_JING_PAI_NUM;i++)
		{
			if (pai == byPai[i] && pai != 255)
			{
				byPai[i] = 255;
				byJingNum--;
				return true;
			}
		}
		return false;
	}
	void Sort()//从小到大排序
	{
		CLogicBase::SelectSort(byPai, MAX_JING_PAI_NUM, false);
	}
};

//麻将牌：各家门牌，翻精点，当前拿牌方向
struct MjMenPaiData
{
	BYTE    byMenPai[MAX_MJ_PAI_COUNT];				//麻将的数量，玩家个数，门牌最大敦数，每敦上下2张牌 
	BYTE    byNt;						//庄家位置
	BYTE	byStation;					//当前抓牌玩家的位置
	BYTE	byDir;						//当前抓牌方向(抓那个玩家门前的牌)
	BYTE	byGetPaiDir;				//本局起牌方向
	BYTE	byRemainDun;				//剩余敦数
	BYTE	byGetPai;				    //起牌敦数
	BYTE    byMenPaiDun[PLAY_COUNT];	//每个玩家门前的敦数
	BYTE    byStartIndex;				//门牌起始索引		
	BYTE    byEndIndex;				    //门牌结束索引		
	BYTE    byCaiShenIndex;				//财神位置索引		
	BYTE    byAllPaiNum;				//所有牌的数量
	BYTE    byPaiNum;                   //门牌数量
	MjMenPaiData()
	{
		Init();
	};
	void Init()
	{
		memset(byMenPai, 255, sizeof(byMenPai));
		memset(byMenPaiDun, 0, sizeof(byMenPaiDun));//玩家门前的敦数
		byNt = 255;						//庄家位置
		byStation = 255;					//当前抓牌玩家的位置
		byDir = 255;						//当前抓牌方向(抓那个玩家门前的牌)
		byGetPaiDir = 255;				//本局起牌方向
		byRemainDun = 255;				//剩余敦数
		byGetPai = 255;				    //起牌敦数
		byStartIndex = 255;				//门牌起始索引
		byEndIndex = 255;				    //门牌结束索引
		byAllPaiNum = 0;				    //所有牌的数量
		byCaiShenIndex = 255;				//财神位置索引
		byPaiNum = 0;
	};
	BYTE GetMenPaiNum()
	{
		BYTE num = 0;
		for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
		{
			if (byMenPai[i] != 255)
			{
				num++;
			}
		}
		return num;
	};
	BYTE GetOnePaiNum(BYTE pai)
	{
		if (pai == 255)
		{
			return 0;
		}
		BYTE num = 0;
		for (int i = 0;i < MAX_MJ_PAI_COUNT;i++)
		{
			if (byMenPai[i] == pai)
			{
				num++;
			}
		}
		return num;
	};
	bool DeleteAnMenPai(BYTE pai)
	{
		if (pai == 255)
			return false;
		for (int i = 0;i < MAX_MJ_PAI_COUNT;++i)
		{
			if (byMenPai[i] != 255)
			{
				byMenPai[i] = 255;
				return true;
			}
		}
		return false;
	};
	bool ChangAnMenPai(BYTE oldpai, BYTE newpai)// 门牌中要被换的牌 oldpai,换来的新牌 newpai
	{
		if (oldpai == 255 || newpai == 255)
			return false;
		for (int i = 0;i < MAX_MJ_PAI_COUNT;++i)
		{
			if (byMenPai[i] == oldpai)
			{
				byMenPai[i] = newpai;//换牌成功
				return true;
			}
		}
		return false;
	};
};

/*----------------------------------------------------------------*/
///抓牌牌事件
struct	tagZhuaPai
{
	BYTE	byUser;		//抓牌玩家位置
	BYTE	byPs;		//抓牌的值
	BYTE	byGetPaiIndex;//抓牌的索引
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];	//手牌张数
	BYTE	m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];//出牌不超过40张，0为结束标志
	BYTE	byMenPai[4][MEN_CARD_NUM];//门牌
	BYTE	m_byArHuaPai[PLAY_COUNT][HUA_CARD_NUM];//玩家花牌
	tagZhuaPai()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;		//抓牌玩家位置
		byPs = 255;		//抓牌的值
		byGetPaiIndex = 255;//抓牌的索引
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));	//手牌张数
		memset(m_byArOutPai, 255, sizeof(m_byArOutPai));//出牌数据
		memset(byMenPai, 255, sizeof(byMenPai));	//门牌
		memset(m_byArHuaPai, 255, sizeof(m_byArHuaPai));	//花牌
	}
};
/*----------------------------------------------------------------*/
///单个玩家补花事件
struct tagOneBuHua
{
	bool	bFinish;//是否补花结束
	BYTE	byUser;		//补花玩家位置
	BYTE	byHua;		//抓到得花牌
	BYTE	byPs;		//补花的牌
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];	//手牌张数
	BYTE	byMenPai[4][MEN_CARD_NUM];//门牌
	BYTE	m_byArHuaPai[PLAY_COUNT][HUA_CARD_NUM];
	tagOneBuHua()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;//补花玩家位置
		byHua = 255;	  //抓到得花牌
		byPs = 255;	  //补花的牌
		memset(byMenPai, 255, sizeof(byMenPai));	//门牌
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));	//手牌张数
		memset(m_byArHuaPai, 255, sizeof(m_byArHuaPai));//花牌数据
		bFinish = false;
	}
};
/*----------------------------------------------------------------*/
///掷2颗色子确定起牌位置（点数和）和起牌点（最小点）事件	
struct	tagTwoSeziDirAndGetPai
{
	BYTE	byUser;		//掷色子玩家
	BYTE	bySezi0;	//色子0数据
	BYTE	bySezi1;	//色子1数据
	tagTwoSeziDirAndGetPai()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;	//掷色子玩家
		bySezi0 = 255;	//色子0数据
		bySezi1 = 255;	//色子1数据
	}
};

/*----------------------------------------------------------------*/
///发牌事件
struct	tagSendPai
{
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];		//玩家手牌张数
	BYTE	byMenPai[4][MEN_CARD_NUM];//门牌
	tagSendPai()
	{
		Init();
	}
	void Init()
	{
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));//手牌张数
		memset(byMenPai, 255, sizeof(byMenPai));	//门牌
	}
};
/*----------------------------------------------------------------*/
///所有玩家补花事件
struct	tagAllBuHua
{
	BYTE byBeiBuCard[PLAY_COUNT][MAX_MJ_HUA_NUM];//被补的花牌
	BYTE byBuCard[PLAY_COUNT][MAX_MJ_HUA_NUM];   //补的非花牌
	int iBuCount[PLAY_COUNT];//补牌数量
	tagAllBuHua()
	{
		Init();
	}
	void Init()
	{
		memset(byBeiBuCard, 255, sizeof(byBeiBuCard));
		memset(byBuCard, 255, sizeof(byBuCard));
		memset(iBuCount, 0, sizeof(iBuCount));
	}
};

/*----------------------------------------------------------------*/
///吃碰杠糊通知事件事件
struct	tagCPGNotifyMes
{
	bool	bZhuaPai;			//标记是抓牌后有动作(客户端用于显示倒计时时间的)
	bool	bChi;				//是否能吃
	bool	bPeng;				//是否能碰
	bool	bGang;				//是否能杠
	bool	bKan;				//是否能坎牌
	bool	bSaoHu;				//当前能否扫虎
	bool	bTing;				//当前能否听牌
	bool	bHu;				//是否能糊
	bool    bHaveReturn;		//客户端是否返回的用户选择
	bool    bCanAction;			//玩家是否有动作可做
	bool    bCanChi[3];			//头中尾能够吃
	BYTE    byPeng;				//能碰的牌
	BYTE	byChi[3][3];		//能吃的牌
	BYTE	iGangData[4][2];	//能杠的牌	
	BYTE	byTingCanOut[HAND_CARD_NUM];//听牌能打的牌
	BYTE    byMaxThing;					//玩家能做的最大动作
	BYTE	byHuType[MAX_HUPAI_TYPE];	//糊牌类型
	BYTE    iHuPs;				//玩家胡的牌
	BYTE	byHu;			    //胡牌玩家位置
	BYTE    byFangPao;	  	    //放玩家位置
	BYTE	byBlockTime;		//拦牌时间	
	BYTE	byUser;				//出牌位置
	BYTE	byPs;				//出牌的值
	tagCPGNotifyMes()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;			//出牌玩家位置
		byPs = 255;			//出牌的值
		byBlockTime = 255;		//拦牌思考时间
		bZhuaPai = false;			//是否自己抓牌
		bChi = 0;				//是否能吃
		bPeng = 0;				//是否能碰
		bGang = 0;				//是否能杠
		bKan = 0;				//是否能坎牌
		bSaoHu = 0;				//当前能否扫虎
		bTing = 0;				//当前能否听牌
		bHu = 0;				//是否能糊
		bCanAction = false;		//玩家是否有动作可做
		byMaxThing = ACTION_NO;//玩家能做的最大动作
		bHaveReturn = false;		//客户端是否返回的用户选择
		memset(byChi, 255, sizeof(byChi));//能吃的牌
		memset(iGangData, 255, sizeof(iGangData));//能杠的牌	
		memset(byTingCanOut, 255, sizeof(byTingCanOut));///听牌能打的牌
		memset(byHuType, 255, sizeof(byHuType));///糊牌类型
		iHuPs = 255;///玩家胡的牌
		byHu = 255;
		byFangPao = 255;
		byPeng = 255;//能碰的牌
		memset(bCanChi, false, sizeof(bCanChi));
	}
};

///吃碰杠糊通知事件事件
struct	tagCPGNotify
{
	bool	bZhuaPai;			//标记是抓牌后有动作(客户端用于显示倒计时时间的)
	bool	bChi;				//是否能吃
	bool	bPeng;				//是否能碰
	bool	bGang;				//是否能杠
	bool	bKan;				//是否能坎牌
	bool	bSaoHu;				//当前能否扫虎
	bool	bTing;				//当前能否听牌
	bool	bHu;				//是否能糊
	bool    bHaveReturn;		//客户端是否返回的用户选择
	bool    bCanAction;			//玩家是否有动作可做
	bool    bCanChi[3];			//头中尾能够吃
	BYTE    byPeng;				//能碰的牌
	BYTE	byChi[3][3];		//能吃的牌
	BYTE	iGangData[4][2];	//能杠的牌	
	BYTE	byTingCanOut[HAND_CARD_NUM];//听牌能打的牌
	BYTE    byMaxThing;					//玩家能做的最大动作
	BYTE	byHuType[MAX_HUPAI_TYPE];	//糊牌类型
	BYTE    iHuPs;				//玩家胡的牌
	BYTE	byHu;				//胡牌玩家位置
	BYTE    byFangPao;			//放炮玩家位置
	BYTE	byBlockTime;		//拦牌时间	
	BYTE	byUser;				//出牌位置
	BYTE	byPs;				//出牌的值

	//听牌的时候用到
	BYTE	byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];  //听牌，打那张牌胡那张牌
	BYTE	byCardRemainNum[MAX_MJ_PAI]; //每张牌的剩余数量
	tagCPGNotify()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;			//出牌玩家位置
		byPs = 255;			//出牌的值
		byBlockTime = 255;		//拦牌思考时间
		bZhuaPai = false;			//是否自己抓牌
		bChi = 0;				//是否能吃
		bPeng = 0;				//是否能碰
		bGang = 0;				//是否能杠
		bKan = 0;				//是否能坎牌
		bSaoHu = 0;				//当前能否扫虎
		bTing = 0;				//当前能否听牌
		bHu = 0;				//是否能糊
		bCanAction = false;		//玩家是否有动作可做
		byMaxThing = ACTION_NO;//玩家能做的最大动作
		bHaveReturn = false;		//客户端是否返回的用户选择
		memset(byChi, 255, sizeof(byChi));//能吃的牌
		memset(iGangData, 255, sizeof(iGangData));//能杠的牌	
		memset(byTingCanOut, 255, sizeof(byTingCanOut));///听牌能打的牌
		memset(byHuType, 255, sizeof(byHuType));///糊牌类型
		iHuPs = 255;///玩家胡的牌
		byHu = 255;
		byFangPao = 255;
		byPeng = 255;//能碰的牌
		memset(bCanChi, false, sizeof(bCanChi));

		memset(byHuCard, 255, sizeof(byHuCard));
		memset(byCardRemainNum, 0, sizeof(byCardRemainNum));
	}
};
/*----------------------------------------------------------------*/
///出牌事件
struct	tagOutPai
{
	bool	bTing;
	BYTE	byUser;//出牌玩家位置
	BYTE	byPs;//出牌的值
	tagOutPai()
	{
		Init();
	}
	void Init()
	{
		bTing = false;
		byUser = 255;				//出牌玩家位置
		byPs = 255;				//出牌的值
	}
};

/*----------------------------------------------------------------*/
///吃牌牌事件
struct tagChiPai
{
	BYTE	byDo;		//0未发生，10已结束，中间状态为正在发生
	BYTE	byUser;		//吃牌玩家位置
	BYTE	byBeChi;	//被吃牌玩家位置
	BYTE	byType;		//吃牌的类型
	BYTE	byPs;		//吃的牌值
	BYTE	byChiPs[3];	//吃牌的数组
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];	//手牌张数
	BYTE	m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];	//出牌不超过40张，0为结束标志
	BYTE	m_byArOutPaiCount[PLAY_COUNT];		//出牌的数量
	TCPGStruct m_UserGCPData[PLAY_COUNT][5];     //吃碰杠杠牌
	tagChiPai()
	{
		Init();
	}
	void Init()
	{
		byDo = 255;//0未发生，10已结束，中间状态为正在发生
		byUser = 255;//吃牌玩家位置
		byBeChi = 255;//被吃牌玩家位置
		byType = ACTION_NO;//吃牌的类型
		byPs = 255;	//吃的牌值
		memset(byChiPs, 255, sizeof(byChiPs));//吃牌的数组
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));//手牌张数
		memset(m_byArOutPai, 255, sizeof(m_byArOutPai));//出牌数据
		memset(m_byArOutPaiCount, 0, sizeof(m_byArOutPaiCount));	//出牌数量
		memset(m_UserGCPData, 255, sizeof(m_UserGCPData));//吃碰杠杠牌
	}
};

/*----------------------------------------------------------------*/
///碰牌牌事件
struct tagPengPai
{
	BYTE	byUser;		//碰牌玩家位置
	BYTE	byBePeng;	//被碰牌玩家位置
	BYTE	byPs;		//碰的牌值
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];	//手牌张数
	BYTE	m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];//出牌不超过40张，0为结束标志
	BYTE	m_byArOutPaiCount[PLAY_COUNT];		//出牌的数量
	TCPGStruct m_UserGCPData[PLAY_COUNT][5];     //吃碰杠杠牌
	tagPengPai()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;//吃牌玩家位置
		byBePeng = 255;//被碰牌玩家位置
		byPs = 255;//碰的牌值
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));//手牌张数
		memset(m_byArOutPai, 255, sizeof(m_byArOutPai));//出牌数据
		memset(m_byArOutPaiCount, 0, sizeof(m_byArOutPaiCount));	//出牌数量
		memset(m_UserGCPData, 255, sizeof(m_UserGCPData));//吃碰杠杠牌
	}
};

/*----------------------------------------------------------------*/
///杠牌牌事件
struct tagGangPai
{
	BYTE	byUser;		//杠牌玩家位置
	BYTE	byBeGang;	//被杠牌玩家位置
	BYTE	byType;		//杠牌的类型
	BYTE	byPs;		//杠的牌值
	BYTE	byGangPai[4];//要杠的四张牌
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];			//手牌张数
	BYTE	m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];	//出牌不超过40张，0为结束标志
	BYTE	m_byArOutPaiCount[PLAY_COUNT];			//出牌的数量
	int     iGangFen[PLAY_COUNT];					//杠分
	TCPGStruct m_UserGCPData[PLAY_COUNT][5];		//吃碰杠杠牌
	tagGangPai()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;//杠牌玩家位置
		byBeGang = 255;//被杠牌玩家位置
		byType = ACTION_NO;//杠牌的类型
		byPs = 255;	//杠的牌值
		memset(byGangPai, 255, sizeof(byGangPai));//要杠的四张牌
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));//手牌张数
		memset(m_byArOutPai, 255, sizeof(m_byArOutPai));//出牌数据
		memset(m_byArOutPaiCount, 0, sizeof(m_byArOutPaiCount));	//出牌数量
		memset(m_UserGCPData, 255, sizeof(m_UserGCPData));//吃碰杠杠牌
		memset(iGangFen, 0, sizeof(iGangFen));
	}
};
/*----------------------------------------------------------------*/
///听牌牌事件
struct tagTingPai
{
	bool	bTing;		//是否听牌
	bool	bHitTing[4];//玩家点击了听牌
	BYTE	byUser;		//听牌玩家位置
	BYTE	byTing;		//出牌玩家位置
	BYTE	byPs;		//听牌出的牌值
	BYTE	byTingCanOut[HAND_CARD_NUM];			//听牌能打的牌
	tagTingPai()
	{
		Init();
	}
	void Init()
	{
		byUser = 255;//听牌玩家位置
		byTing = 255;//出牌玩家位置
		byPs = 255;//听牌出的牌值	
		bTing = false;
		memset(bHitTing, 0, sizeof(bHitTing));//玩家点击了听牌
		memset(byTingCanOut, 255, sizeof(byTingCanOut));
	}
};
/*----------------------------------------------------------------*/
///糊牌牌事件
struct tagHuPai
{
	bool	bHaveHu[PLAY_COUNT];//对应的玩家是否胡牌了
	bool    bZimo;				//是否自摸
	bool	bIsLiuJu;			//是否流局
	BYTE	byHu;				//胡牌玩家位置
	BYTE    byFangPao;			//放炮玩家位置
	BYTE	byPs;				//胡的牌
	int	    iUserNums;			//糊牌玩家的个数
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];			//手牌张数
	BYTE	m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];	//出牌不超过40张，0为结束标志
	BYTE    byHuType[PLAY_COUNT][MAX_HUPAI_TYPE];	//各个玩家的糊牌类型
	tagHuPai()
	{
		Init();
	}
	void Init()
	{
		iUserNums = 0;
		byHu = 255;//点炮玩家位置
		byFangPao = 255;
		byPs = 255;	//胡的牌值
		bIsLiuJu = true;	//是否流局
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));//手牌张数
		memset(m_byArOutPai, 255, sizeof(m_byArOutPai));//出牌数据
		memset(byHuType, 255, sizeof(byHuType));//各个玩家的糊牌类型
		memset(bHaveHu, 0, sizeof(bHaveHu));
		bZimo = false;		//是否自摸
	}
};
struct tagHuPaiInfo
{
	int     iHuFenType[PLAY_COUNT][MAX_HU_FEN_KIND];//各个玩家的糊牌分数
	tagHuPaiInfo()
	{
		Init();
	}
	void Init()
	{
		memset(iHuFenType, 0, sizeof(iHuFenType));
	}
};
/*----------------------------------------------------------------*/
///算分事件
struct tagCountFen
{
	bool	bZiMo;								    //是否自摸
	bool	bIsLiuJu;								//是否流局
	bool    bIsHu[PLAY_COUNT];						//哪些玩家胡牌了
	BYTE	byFangPao;								//放炮玩家位置
	BYTE	byNt;									//庄家位置
	bool    m_bQiangGangHu;							 //是否抢杠胡，在有抢杠胡的规则下才存在
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];//手牌数据
	BYTE	m_byArHandPaiCount[PLAY_COUNT];			//手牌张数
	BYTE    byHuType[PLAY_COUNT][MAX_HUPAI_TYPE];	//各个玩家的糊牌类型
	BYTE    byTypeFans[MAX_HUPAI_TYPE];				//类型番数
	int	    byUserNums;								//糊牌玩家的个数
	int		iBase;						//底分
	int		iFanCount[PLAY_COUNT];		//各玩家番数
	int		iZongFen[PLAY_COUNT];		//各玩家总分
	int		iMingGang[PLAY_COUNT];		//明杠的个数
	int		iAnGang[PLAY_COUNT];		//暗杠的个数
	int		iTax;						//服务费（收税）
	int     iHuFenType[PLAY_COUNT][MAX_HU_FEN_KIND];//各个玩家的糊牌分数
	__int64 		i64ZongFen[PLAY_COUNT];			//各个玩家的总分
	__int64 		i64Money[PLAY_COUNT];			//各个玩家的总分
	TCPGStruct	UserGCPData[PLAY_COUNT][5];			//吃碰杠杠牌
	tagHuPaiInfo HupaiInfo[PLAY_COUNT];
	tagCountFen()
	{
		Init();
	}
	void Init()
	{
		byFangPao = 255;		//放炮玩家位置
		byNt = 255;		//庄家位置
		bIsLiuJu = true;		//是否流局
		bZiMo = false;
		memset(i64ZongFen, 0, sizeof(i64ZongFen));
		memset(i64Money, 0, sizeof(i64Money));
		memset(m_byArHandPai, 255, sizeof(m_byArHandPai));//手牌数据	
		memset(m_byArHandPaiCount, 0, sizeof(m_byArHandPaiCount));//手牌张数	
		memset(UserGCPData, 255, sizeof(UserGCPData));
		memset(iFanCount, 0, sizeof(iFanCount));
		memset(iZongFen, 0, sizeof(iZongFen));
		memset(iMingGang, 0, sizeof(iMingGang));
		memset(iAnGang, 0, sizeof(iAnGang));
		memset(byHuType, 255, sizeof(byHuType));
		memset(bIsHu, 0, sizeof(bIsHu));	//哪些玩家胡牌了
		memset(byTypeFans, 0, sizeof(byTypeFans));
		memset(iHuFenType, 0, sizeof(iHuFenType));
		byUserNums = 0;
		iBase = 0;
		iTax = 0;//服务费（收税）
		m_bQiangGangHu = false;
		for (int i = 0;i < PLAY_COUNT;i++)
		{
			HupaiInfo[i].Init();
		}
	}
};

//配牌数据
struct tagMatchMJ
{
	//手牌不超过14张 
	BYTE byArHandPai[PLAY_COUNT][HAND_CARD_NUM];
	//手牌张数	  
	BYTE byArHandPaiCount[PLAY_COUNT];
	tagMatchMJ()
	{
		Init();
	}
	void Init()
	{
		memset(byArHandPai, 255, sizeof(byArHandPai));
		memset(byArHandPaiCount, 0, sizeof(byArHandPaiCount));
	}
};

//动作轮寻事件
struct tagActionTurn
{
	BYTE	byChi;		//吃
	BYTE	byPeng;		//碰
	BYTE	byGang;		//杠
	BYTE    byHu;		//胡
	tagActionTurn()
	{
		Init();
	}
	void Init()
	{
		byChi = 255;
		byPeng = 255;
		byGang = 255;
		byHu = 255;
	}
};
#pragma pack() //1字节对齐
#endif
