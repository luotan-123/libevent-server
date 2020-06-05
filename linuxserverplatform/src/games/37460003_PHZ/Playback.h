#pragma once
#include "UpGradeLogic.h"

class CServerGameDesk;
class CGameLogic;

//升级逻辑类 
class CPlayback
{
public:
	//构造函数
	CPlayback(void);
	~CPlayback(void);

public:
	void Clear();
	void SetDeskLogicPoint(CGameLogic *pGameLogic) { m_pGameLogic = pGameLogic;};
	void SaveUserInfor(CServerGameDesk *pDesk);
	void SendCard(BYTE byDataIndex[PLAY_COUNT][MAX_INDEX],int iJinNum[PLAY_COUNT]);												//发牌
	void SaveCPGTWP(tagWeaveItem *tCPGTWP,BYTE byCardData,int iMenNums,BYTE byDataIndex[MAX_INDEX],int iJinNum,BYTE byDesk);	//拦牌	
	void SaveGetCard(BYTE byGetCardData,BYTE byCurrendUser,BYTE iMenNums,BYTE byDataIndex[MAX_INDEX],int iJinNum);				//取牌			
	void SaveOutCard(BYTE byGetCardData,BYTE byCurrendUser,BYTE iMenNums,BYTE byDataIndex[MAX_INDEX],int iJinNum);				//出牌
	void SaveNotic();
	void SaveHuInfor();
	void SaveToHardDisk(CServerGameDesk *pDesk,char cTempCode[]);
private:
	ofstream						m_fVideoFile;
	int								m_iVideoIndex;
	Json::Value						m_element;
	Json::Value						m_root;
	char							m_VideoCode[20];
	CGameLogic						*m_pGameLogic;
public:
	char							m_szTempCode[20];
};
