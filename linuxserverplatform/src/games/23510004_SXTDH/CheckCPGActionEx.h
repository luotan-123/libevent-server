#ifndef CHECKCPGACTIONEX_H
#define CHECKCPGACTIONEX_H
#include "CheckCPGAction.h"
#include "CheckHuPaiEx.h"
///吃碰杠检测类(重载)
class CheckCPGActionEx :public CheckCPGAction
{
public:
	CheckCPGActionEx(void);
	~CheckCPGActionEx(void);
public:
	///初始化数据
	virtual void InitData();

	///能否碰牌
	virtual bool CanPeng(GameDataEx *pGameData, BYTE station, BYTE pai);

	///能否杠牌
	bool CanGang(GameDataEx *pGameData, BYTE station, BYTE pai, BYTE Result[][2], bool bMe = false, CheckHuPaiEx *pCheckHuData = NULL);

	//杠牌算分
	virtual void CountGangFen(GameDataEx *pGameData, int iStation, int iBeStation, BYTE byType);

	//计算杠分
	void CountGangFenEx(GameDataEx *pGameData);

	//减去抢杠胡所得杠分,减补杠分
	void MinusGangFen(GameDataEx *pGameData, int iStation, int iBeStation);
};
#endif