#ifndef GAMEDATA_H
#define GAMEDATA_H
#include "Mj_Message.h"
#include "mj.h"
#include "CLogicBase.h"
///游戏数据管理类:配置数据，游戏基本数据等，对数据进行插入修改等动作
class GameData
{
public:
	MjRuleSet				m_mjRule;		/// 麻将常规属性	  
	ActionRuleSet			m_mjAction;		/// 麻将动作属性        
	BYTE					m_byNtStation;	//	庄家位置	    
	bool					m_bGangState[PLAY_COUNT];//记录杠牌状态，拥于杠开和杠后炮
	////////////基本数据定义/////////////////////////////////////////////////////////

	///吃牌牌事件,临时数据
	tagChiPai				temp_ChiPai;

	///碰牌牌事件,临时数据
	tagPengPai				temp_PengPai;

	///杠牌牌事件,临时数据
	tagGangPai				temp_GangPai;

	///胡牌事件,临时数据(多个人可以胡牌)
	tagHuPai				temp_HuPai[PLAY_COUNT];
public:////////////事件数据对象定义//////////////////////
	///掷2颗色子确定起牌位置（点数和）和起牌点（最小点）事件	
	tagTwoSeziDirAndGetPai	T_TwoSeziDirAndGetPai;

	///发牌事件	
	//tagSendPai			T_SendPai;

	///所有玩家补花事件	
	//tagAllBuHua			T_AllBuHua;

	///出牌事件	
	tagOutPai			T_OutPai;

	///吃碰杠糊牌通知事件牌事件	  
	tagCPGNotify		T_CPGNotify[PLAY_COUNT];

	///抓牌牌事件		
	tagZhuaPai			T_ZhuaPai;

	///单个玩家补花事件	
	//tagOneBuHua			T_OneBuHua;

	///吃牌牌事件	
	tagChiPai			T_ChiPai;

	///碰牌牌事件	
	tagPengPai			T_PengPai;

	///杠牌牌事件	
	tagGangPai			T_GangPai;

	///听牌牌事件	
	tagTingPai			T_TingPai;

	///糊牌牌事件	
	tagHuPai			T_HuPai;

	///算分事件		
	tagCountFen			T_CountFen;
public:
	//门牌数据
	MjMenPaiData		m_MenPai;

	//吃碰杠
	TCPGStruct			m_UserGCPData[PLAY_COUNT][5];

	///精牌数据
	JingStruct			m_StrJing;

	//手牌不超过14张 
	BYTE				m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];

	//手牌张数	  
	BYTE				m_byArHandPaiCount[PLAY_COUNT];

	//门牌不超过40张，0：此墩有门牌，255：此处无门牌（2D麻将不需要显示门牌墩数）
	BYTE				m_iArMenPai[4][MEN_CARD_NUM]; //和游戏人数无关，都是4墩门牌

	//花牌不超过8张
	BYTE				m_byArHuaPai[PLAY_COUNT][HUA_CARD_NUM];

	//花牌个数
	BYTE				m_byArHuaPaiCount[PLAY_COUNT];

	//出牌不超过40张
	BYTE				m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];

	//出牌的张数
	BYTE				m_byArOutPaiCount[PLAY_COUNT];

	//当前操作玩家	  
	BYTE				m_byNowOutStation;

	//记录玩家是否出过牌了(用于检测天胡地胡)
	bool				m_bHaveOut[PLAY_COUNT];

	//玩家是否听牌
	bool				m_bTing[PLAY_COUNT];

	//听牌类型(明楼 还是暗楼--山东麻将特有)
	BYTE				m_byTingType[PLAY_COUNT];

	//转换后的门牌数据
	BYTE				m_byMenPaiData[4][MEN_CARD_NUM / 2][2];

	//最后打出的牌
	BYTE				m_byLastOutPai;

	//抓最后一张牌的玩家
	BYTE				m_byEndZhuaPaiPalyer;

	//当前抓牌方向，true 顺时针，false 逆时针	  
	bool				m_bZhuaPaiDir;

	//上次出牌位置
	BYTE				m_byLastOutStation;

	//停牌或其他操作后能打出的牌
	BYTE				m_byCanOutCard[PLAY_COUNT][HAND_CARD_NUM];

	//圈风
	BYTE				m_byQuanFeng;

	//门风
	BYTE				m_byMenFeng[PLAY_COUNT];

	///当前选中排索引
	BYTE				m_bySelectIndex;

	//剩余门牌
	BYTE                m_byRemainMenPai[MAX_REMAIN_MEN_PAI_NUM];

	//剩余门牌数量
	BYTE                m_byRemainMenPaiCount;

	//特殊玩法
	BYTE		m_byTingOutCardIndex[PLAY_COUNT];//听牌倒扣的索引牌
	BYTE		m_byAllOutCardNum[MAX_MJ_PAI]; //已经打出来的每张牌的数量
public:
	GameData(void);
	~GameData(void);
	//初始化数据
	virtual void InitData();

	///初始化数据
	virtual void LoadIni();

	//////////////////手牌操作/////////////////////////////////////
	//检查是否存在指定的手牌
	virtual bool IsHaveAHandPai(BYTE station, BYTE pai);

	//检查手牌中某张牌的个数
	virtual int GetAHandPaiCount(BYTE station, BYTE pai);

	//检查手牌的张数
	virtual int GetHandPaiCount(BYTE station);

	//拷贝玩家的手牌
	virtual int CopyHandPai(BYTE pai[][HAND_CARD_NUM], BYTE station, bool show);

	//描述：拷贝某个玩家的手牌
	virtual int CopyOneUserHandPai(BYTE pai[HAND_CARD_NUM], BYTE station);

	//手牌排序，small是否从小到大排
	virtual void SortHandPai(BYTE station, bool big);

	//添加一张牌到手牌中
	virtual void AddToHandPai(BYTE station, BYTE pai);

	//设置手牌数据
	virtual void SetHandPaiData(BYTE station, BYTE pai[], BYTE byCount);

	//删除一张指定的手牌
	virtual BYTE DeleteAHandPai(BYTE station, BYTE pai);

	//设置手牌牌背
	virtual void SetHandPaiBack(BYTE station, BYTE num);

	//获得最后一张手牌
	virtual BYTE GetLastHandPai(BYTE station);

	//获得选中的一张手牌
	virtual BYTE GetSelectHandPai(BYTE station);

	//是否出牌玩家
	virtual bool IsOutPaiPeople(BYTE station);

	//检测手牌中是否存在花牌
	virtual bool IsHuaPaiInHand(BYTE station);

	//将手牌中的花牌移到花牌数组中
	virtual int MoveHuaPaiFormHand(BYTE station);

	//通过索引获取一张手牌
	virtual BYTE GetHandPaiFromIndex(BYTE station, int index);

	//通过索引设置一张手牌
	virtual bool SetHandPaiFromIndex(BYTE station, int index, BYTE pai);

	//索引是否有牌
	virtual bool IsHaveHandPaiFromIndex(BYTE station, int index);

	//////////////////吃碰杠牌操作/////////////////////////////////////
	//获取玩家杠牌的个数(暗杠，明杠，补杠)
	virtual int GetUserGangNum(BYTE station, BYTE &an, BYTE &ming, BYTE &bu);

	//玩家吃牌次数
	virtual int GetUserChiNum(BYTE station);

	//玩家碰牌次数
	virtual int GetUserPengNum(BYTE station);

	//玩家杠了某个牌
	virtual bool IsUserHaveGangPai(BYTE station, BYTE pai, BYTE &type);

	//玩家是否碰了某张牌
	virtual bool IsUserHavePengPai(BYTE station, BYTE pai);

	//玩家是否吃过某张牌
	virtual bool IsUserHaveChiPai(BYTE station, BYTE pai);

	//添加一组数据到杠吃碰数组中
	virtual void AddToGCP(BYTE station, TCPGStruct *gcpData);

	//删除刚吃碰数组中指定的数据组
	virtual void DeleteAGCPData(BYTE station, BYTE type, BYTE pai);

	//拷贝杠吃碰数组中的数据组
	virtual int CopyGCPData(BYTE station, TCPGStruct gcpData[]);

	//设置吃碰数组中的数据组
	virtual void SetGCPData(BYTE station, TCPGStruct gcpData[]);

	//获取杠吃碰的节点个数
	virtual int GetGCPCount(BYTE station);

	//描述：获取杠吃碰的中某种牌的数量
	virtual int GetOnePaiGCPCount(BYTE station, BYTE pai);

	//排序杠吃碰数组
	virtual void ShortGCP(BYTE station);

	//描述：初始化杠吃碰数组
	virtual void InitGCP(BYTE station);

	///////////////////出牌操作///////////////////////////////////////////////////////
	//添加一张牌到出牌数组中
	virtual void AddToOutPai(BYTE station, BYTE pai, bool bTingCard = false);

	//删除最后一张出牌
	virtual void DeleteLastOutPai(BYTE station);

	//设置出牌数据
	virtual void SetOutPaiData(BYTE station, BYTE pai);

	//拷贝玩家的出牌
	virtual int CopyOutPai(BYTE station, BYTE pai[]);

	//获取玩家的出牌数量
	virtual int GetOutPaiCount(BYTE station);

	//描述：获取玩家某张牌的出牌数量
	virtual int GetOneOutPaiCount(BYTE station, BYTE pai);

	//////////////////花牌操作///////////////////////////////////////////////////////
	//添加一张花牌到花牌数组中
	virtual void AddToHuaPai(BYTE station, BYTE pai);

	//设置花牌数据
	virtual void SetHuaPaiData(BYTE station, BYTE pai[]);

	//拷贝玩家的花牌
	virtual int CopyHuaPai(BYTE station, BYTE pai[]);

	//检测是否花牌
	virtual bool CheckIsHuaPai(BYTE pai);

	///////////////////门牌牌操作///////////////////////////////////////////////////////
	//设置门牌数据
	virtual void SetMenPaiData(BYTE station, BYTE pai[]);

	//拷贝所有门牌
	virtual void CopyAllMenPai(BYTE pai[][MEN_CARD_NUM]);

	//拷贝玩家的门牌
	virtual int CopyMenPai(BYTE station, BYTE pai[]);

	//描述：门牌转换
	virtual void ChangeMenPai(int iIndex, BYTE byCard = 255);

	//描述：创建门牌
	virtual void CreateMenPai();

	//描述：获取某玩家的门牌数量
	virtual int GetMenPaiCount(BYTE station);

	///////////////////财神操作//////////////////////////////////////////////
	/////检测pai是否财神
	//virtual bool CheckIsCaiShen(BYTE pai);
	/////清空财神结构
	//virtual void InitCaiShen();
	/////////////////超级客户端设置牌操作///////////////////////
	//换所有手牌
	virtual void ChangeAllHandPai(BYTE station, BYTE pai[], BYTE count);

	//换指定手牌
	virtual void ChangeSomeHandPai(BYTE station, BYTE pai[], BYTE handpai[], BYTE count);

	///设置牌墙的下一张牌
	virtual void SetGameNextPai(BYTE pai);
};
#endif