#ifndef GAMEDATA_H
#define GAMEDATA_H
#include "GameDesk.h"
#include "UpgradeMessage.h"
///财神数据结构
struct JingStruct
{
	BYTE  byPai[8];    //每张精牌的牌值
	BYTE  byJingNum;   //精牌种类张数
	JingStruct()
	{
		Init();
	}
	void Init()
	{
		memset(this,255,sizeof(JingStruct));
		byJingNum = 0;
	};
	bool CheckIsJing(BYTE pai)
	{
		for(int i=0;i<8;i++)
		{
			if(byPai[i] == pai && pai!=255)
				return true;
		}
		return false;
	};
	bool AddJing(BYTE pai)
	{
		for(int i=0;i<8;i++)
		{
			if(byPai[i] == 255 && pai !=255)
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
		for(int i=0;i<8;i++)
		{
			if(pai==byPai[i] && pai!=255)
			{
				byPai[i] = 255;
				byJingNum--;
				return true;
			}
		}
		return false;
	}
};
///游戏数据管理类:配置数据，游戏基本数据等，对数据进行插入修改等动作
class GameData
{
public:
	tagWeaveItem m_tCPTWPArray[PLAY_COUNT][7];			//组合扑克(保存:吃碰偎提跑)
	JingStruct	 m_tJingData;						//精牌数据
	int			m_bTryBigAction[PLAY_COUNT];		//尝试最大动作
	bool		m_bHuJing[PLAY_COUNT];				//是否摸精牌有胡牌动作
	bool		m_bHuTiOrPao[PLAY_COUNT];			//是否提牌或偎牌跑牌有胡牌动作
	bool		m_bHuWei[PLAY_COUNT];				//是否偎牌有胡牌动作
	bool		m_bBiePaoSpecialHu[PLAY_COUNT];		//是否憋跑状态下有特殊胡
	bool		m_bBiePaoSpecialChi[PLAY_COUNT];	//是否憋跑状态下有胡有吃有跑
	bool		m_bForbidOutCard[PLAY_COUNT];		//不能出牌
	bool        m_bStartHu[PLAY_COUNT];				//起手胡牌
	BYTE		m_byHuPriority[PLAY_COUNT];			//胡牌优先级
	BYTE		m_byChiPriority[2];					//吃牌优先级
	bool		m_bFinish;							//游戏结束
	BYTE        m_byWeiPai[MAX_INDEX];				//偎牌列表		
	BYTE		m_byOutPai[PLAY_COUNT][MAX_INDEX];	//出牌列表
	BYTE		m_byTingCard[PLAY_COUNT][MAX_INDEX];//听这些牌
	bool		m_bTing[PLAY_COUNT];				//听牌
	char		m_szLogFile[256];
public:
	GameData(void);
	~GameData(void);
	//初始化数据
	virtual void InitData();
	//////////////////吃碰偎提跑操作/////////////////////////////////////
	//玩家吃牌次数
	virtual int GetUserChiNum(BYTE station);
	//玩家碰牌次数
	virtual int GetUserPengNum(BYTE station);
	//玩家跑牌次数
	virtual int GetUserPaoNum(BYTE station);
	//玩家提牌次数
	virtual int GetUserTiNum(BYTE station);
	//玩家是否碰了某张牌
	virtual bool IsUserHavePengPai(BYTE station,BYTE pai);
	//添加一组数据到吃碰偎提数组中
	virtual void AddToCPTWP(BYTE station,tagWeaveItem gcpData);
	//删除吃碰偎提数组中指定的数据组
	virtual void DelleteCPTWPData(BYTE station,BYTE type ,BYTE pai);
	//拷贝吃碰偎提数组中的数据组
	virtual int CopyCPTWPData(BYTE station,tagWeaveItem gcpData[]);
	//获取吃碰偎提的节点个数
	virtual int GetCPTWPCount(BYTE station);
	//排序吃碰偎提数组
	virtual void ShortCPTWP(BYTE station);
	//动作序列醒牌个数
	int  GetCPTWPXing(BYTE station,BYTE byXing);
	//动作序列红牌个数
	int  GetCPTWPEQS(BYTE station);
};
#endif