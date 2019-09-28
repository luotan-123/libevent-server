#ifndef CHECKHUPAIEX_H
#define CHECKHUPAIEX_H
#include "CheckHuPai.h"
///事件处理类（重载）
class CheckHuPaiEx :public CheckHuPai
{
public:
	int   m_byTingMaxFan;//听牌可能的最大番
	int   m_iHuFenType[PLAY_COUNT][MAX_HU_FEN_KIND];
public:
	CheckHuPaiEx(void);
	~CheckHuPaiEx(void);
	//初始化数据
	virtual void InitData();

	/////////检测是否能胡牌////////
	virtual bool CanHu(GameDataEx *pGameData, BYTE station, BYTE byBeStation, BYTE lastpai, bool zimo);

	///////////////麻将算分//////////////////
	void CountHuFen(GameDataEx *pGameData);

	//根据胡牌类型，获取胡牌倍数
	int GetHuTypeFans(GameDataEx *pGameData, BYTE m_byHuType[MAX_HUPAI_TYPE]);

	//检测能否听牌
	virtual bool CanTing(GameDataEx *pGameData, BYTE station, BYTE CanOut[]);

	//获取一张可听的牌
	BYTE GetCanTingCard(GameDataEx *pGameData, BYTE station);

	//提取糊牌
	virtual void ExtractHu(GameDataEx *pGameData, BYTE station, BYTE byHuCard[]);

	//从手牌中出一张牌，然后提取能糊的牌
	bool OutCardExtractHu(GameDataEx *pGameData, BYTE station, BYTE byOutCanHuCard[HAND_CARD_NUM],
		BYTE byHuCard[HAND_CARD_NUM][MAX_CANHU_CARD_NUM], BYTE byCardRemainNum[MAX_MJ_PAI]);

	//将将胡
	bool CheckAllJiang(GameDataEx *pGameData);

	///计算七对中四张牌的数量
	int CountLongQi();

	///计算七对中四张牌的数量
	bool CheckHuGen(BYTE lastpai);

	//检测是否幺九七对
	bool CheckYaoJiuQiDui(GameDataEx *pGameData);

	//新七对检测
	bool CheckQiDuiEx(GameDataEx *pGameData);

	//检测两幅手牌的胡牌是否改变
	bool IsTingKouChange(GameDataEx *pGameData, BYTE station, BYTE byChangeCard[HAND_CARD_NUM]);
};
#endif