#ifndef GAMEDATAEX_H
#define GAMEDATAEX_H
#include "GameData.h"
#include <vector>

///游戏数据管理类
class GameDataEx : public GameData
{
public:
	bool		m_bIsHu[PLAY_COUNT];		//是否胡牌
	bool		m_bQiHu[PLAY_COUNT];		//玩家是否弃糊状态
	bool		m_bQiPeng[PLAY_COUNT];		//弃碰
	int			m_iGangScore[PLAY_COUNT];	//杠牌得分
	int			m_iHuaScore[PLAY_COUNT];	//花胡得分
	int         m_iAddScore[PLAY_COUNT];	//每一把的跑数
	int			m_iHuangZhuangCount;//荒庄次数
	int         m_iZhuangWinCount; //庄家连赢次数
	//BYTE		m_byDingQue[PLAY_COUNT];	//定的缺，0表万  1表条 2表筒
	BYTE		m_byQiHuTurn;//弃胡点
	BYTE		m_byPengTurn;//碰牌点
	std::vector<BYTE> m_vecQiPengCard[PLAY_COUNT];
	tagMatchMJ  m_tMatchMJ;
	tagActionTurn  m_tActionTurn[PLAY_COUNT];
	//当前可操作吃碰杠胡的玩家
	BYTE		m_byCurrentOperatorUser;

	//特殊规则
	tagStructGameRuler m_tGameRuler;
	bool		m_bIsGangShangGang[PLAY_COUNT];//是否有杠上杠，
	int			m_bGangZiMo[PLAY_COUNT];//记录杠上自摸，【啥也没哟 0】,【杠 1】，【杠上自摸 2】

	//补花
	vector<BYTE>   m_vHuapaiTemp[PLAY_COUNT]; //起手每个玩家花牌的数量
	bool		   m_bIsBuHua[PLAY_COUNT];	  //是否起手补过花
	bool           m_bIsBuAllHua[PLAY_COUNT]; //是否补完所有花
	BYTE		   m_byFangWei[PLAY_COUNT];	  //方位：东南西北
	BYTE		   m_byHuaPaiType[PLAY_COUNT][2];//花牌类型：春夏秋冬牌，梅兰竹菊牌。每个人两种比如梅和春

	//新的算分统计
	CountHuFenStruct                m_countHuFen[PLAY_COUNT];     //胡分
	CountGangFenStruct              m_countGangFen[PLAY_COUNT];   //杠分
	//CountGenZhuangFenStruct         m_countGenZhuang[PLAY_COUNT]; //跟庄分
	//CountHorseFenStruct				m_countHorseFen[PLAY_COUNT];  //马分

	//保存玩家听牌、胡牌数据，断线重连使用
	BYTE m_byOutCanHuCard[HAND_CARD_NUM];				//打哪张牌可胡
	BYTE m_byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM];//打那张牌胡那张牌
	BYTE m_byCardRemainNum[MAX_MJ_PAI];		//操作玩家显示剩余牌数量
	BYTE m_byOutAfterHuCard[PLAY_COUNT][MAX_CANHU_CARD_NUM]; //左下角显示胡牌
	BYTE m_byCardType[MAX_MJ_KIND_NUM]; //本局所有的牌
public:
	GameDataEx(void);
	~GameDataEx(void);
	//初始化数据
	virtual void InitData();
	//初始化数据
	virtual void LoadIni();

	//////////////////手牌操作/////////////////////////////////////
	///检查是否存在指定的门牌
	virtual	bool IsHaveAMenPai(BYTE pai);

	//换指定的一张手牌
	bool	ChangeAHandPai(BYTE station, BYTE ChangePai, BYTE byNewCard);

	//换指定一张门牌
	bool	ChangeAMenPai(BYTE ChangePai, BYTE byNewCard);

	///是否出牌玩家
	virtual bool IsOutPaiPeople(BYTE station);

	///获取手牌的牌色数
	virtual int GetPaiSeCount(BYTE station);

	///描述：抓一张牌
	virtual BYTE GetPai(int iStation, bool bhead, BYTE *pIndex = NULL, BYTE byGetPai = 255);

	///是否还有缺门的牌
	bool IsHaveQueMen(BYTE station, BYTE type);

	//获取一张data中没有的牌，用来换牌
	BYTE GetChangePai(BYTE station, BYTE data[]);

	///检查某种花色牌的个数（0万 1筒 2条)
	int GetAKindPaiCount(BYTE station, BYTE kind);

	///得到某种花色牌（0万 1筒 2条)
	int GetAKindPai(BYTE station, BYTE kind);

	//////////////////吃碰杠牌操作/////////////////////////////////////
	///添加一组数据到杠吃碰数组中
	virtual void AddToGCP(BYTE station, TCPGStruct *gcpData);

	//////////////////花牌操作///////////////////////////////////////////////////////
	///检测某个人是否那张花牌
	virtual bool CheckIsHaveHuaPai(BYTE station, BYTE pai);

	///////////////////扩展功能///////////////////////////////////////////////////////
	//混乱牌
	virtual bool DisPatchCard();

	///获取下家位置
	BYTE GetNextStation(BYTE station, bool shun);

	//配牌
	BYTE MatchMJ();

	//翻鬼
	void TurningString(BYTE &byPai0, BYTE &byPai1);

	//判断是否满足跟庄规则
	int CheckIsShouZhangGen(bool bIsNext = false);

	//获取两个玩家的相对位置关系
	BYTE GetPlayerStationRelation(BYTE byMeStation, BYTE byOtherStation);

	//获取byMeStation玩家和非byNoStation的关系
	BYTE GetPlayerStationRelationEx(BYTE byMeStation, BYTE byNoStation);

	//获得剩余门牌
	BYTE GetRemainMenPai(BYTE m_byRemainMenPai[MAX_REMAIN_MEN_PAI_NUM]);

	//添加胡分
	//BYTE byNameType[3];					//相应的名字， 枚举类型MJ_HU_FEN_TYPE
	//BYTE byXiangYingStation[3];			//相应玩家，枚举类型MJ_STATION
	//BYTE byXiangYingStationEx[3];		//相应玩家，枚举类型MJ_STATION
	//bool bBaoTing[3];					//点炮玩家：byXiangYingStationEx是否报听，非点炮玩家（自摸玩家）：byXiangYingStation是否报听
	//int iAllFen[3];						//输赢的胡分
	bool AddToHuFenStruct(BYTE byStation, BYTE byNameType, BYTE byXiangYingStation, BYTE byXiangYingStationEx, bool bBaoTing, int iAllFen);

	//添加杠分
	bool AddToGangFenStruct(BYTE byStation, BYTE byGangType, BYTE byXiangYingStation, bool bBaoTing, int iAllFen);

	//清除补杠列表分
	bool DelBuGangFenStruct(BYTE byStation, BYTE byGangType, BYTE byXiangYingStation, bool bBaoTing, int iAllFen);

	//添加马分
	bool AddToHorseFenStruct(BYTE byStation, BYTE byBuyHorseType, BYTE byMaiStation, BYTE byBeiMaiStation, int iAllFen);

	//添加跟庄分
	bool AddToGenZhuangFenStruct(BYTE byStation, BYTE byGenZhuangType, BYTE byXiangYingStation, int iAllFen);
};
#endif