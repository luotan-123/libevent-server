#ifndef CHECKCPGACTION_H
#define CHECKCPGACTION_H
#include "GameDataEx.h"

///吃碰杠检测类
class CheckCPGAction
{
public:
	CheckCPGAction(void);
	~CheckCPGAction(void);
public:
	///初始化数据
	virtual void InitData();

	///能否吃牌
	virtual bool CanChi(GameDataEx *pGameData, BYTE station, BYTE pai, BYTE Result[][3], bool bChi[3]);

	///能否碰牌
	virtual bool CanPeng(GameDataEx *pGameData, BYTE station, BYTE pai);

	//能否坎牌
	virtual bool CanKan(GameDataEx *pGameData, BYTE station, BYTE kan[]);

	///能否杠牌
	virtual bool CanGang(GameDataEx *pGameData, BYTE station, BYTE pai, BYTE Result[][2], bool bMe = false);
};
#endif