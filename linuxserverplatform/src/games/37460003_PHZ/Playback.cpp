#include "ServerManage.h"

CPlayback::CPlayback(void)
{
	Clear();
}

CPlayback::~CPlayback(void)
{

}

void CPlayback::Clear()
{
	m_iVideoIndex=0;
	memset(m_VideoCode,0,sizeof(m_VideoCode));
	m_element.clear();
	m_root.clear();
	m_fVideoFile.close(); 
}
void CPlayback::SaveUserInfor(CServerGameDesk *pDesk)
{
	m_root["GmaeId"] = pDesk->m_pDataManage->m_InitData.uNameID;					//游戏ID
	m_root["BankerPos"] = pDesk->m_byBankerUser;									//庄家位置
	m_root["RoomPassward"] = pDesk->m_szDeskPassWord;								//房间号
	m_root["CurPlayCount"] = pDesk->m_iRunGameCount;								//已经玩的局数
	m_root["AllPlayCount"] = pDesk->m_iVipGameCount;								//房间局数

	Json::Value _index;
	for(int i = 0;i<PLAY_COUNT;i++)
	{
		UserData userData;
		if (!pDesk->GetUserData(i, userData))
		{
			continue;
		}
		char key[10];
		memset(key,0,sizeof(key));
		sprintf(key,"%d",i);
		Json::Value _info;
		_info["nickName"]=Json::Value(userData.name);									//玩家昵称
		_info["bLogoID"]=Json::Value(userData.userID);									//登录ID
		_info["Money"]=Json::Value((UINT)userData.money);										//玩家金币
		_info["bBoy"]=Json::Value(userData.sex);										//男女标志
		_info["szHeadUrl"] =Json::Value(userData.headURL);			//头像网址
		_info["dwUserIP"]=Json::Value(userData.logonIP);		//玩家的IP
		_index[key]=Json::Value(_info);
	}
	m_root["user"]=_index;
}

void CPlayback::SendCard(BYTE byDataIndex[PLAY_COUNT][MAX_INDEX],int iJinNum[PLAY_COUNT])
{
	Json::Value array_1;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		char szkey[10];
		memset(szkey,0,sizeof(szkey));
		sprintf(szkey,"%d",i);
		Json::Value array_2;
		BYTE byCard[MAX_COUNT];
		memset(byCard,0,sizeof(byCard));
		m_pGameLogic->SwitchToCardData(byDataIndex[i],byCard,MAX_COUNT,iJinNum[i]);
		for(int j = 0;j < MAX_COUNT;j++)
		{
			char key_2[10];
			memset(key_2,0,sizeof(key_2));
			sprintf(key_2,"%d",j);
			array_2[key_2]=Json::Value(byCard[j]);
		}
		array_1[szkey]=Json::Value(array_2);
	}
	m_root["SendHandlCard"]=array_1;
}

//拦牌数据
void CPlayback::SaveCPGTWP(tagWeaveItem *tCPGTWP,BYTE byCardData,int iMenNums,BYTE byDataIndex[MAX_INDEX],int iJinNum,BYTE byDesk)
{
	char szEventIndex[10];
	memset(szEventIndex,0,sizeof(szEventIndex));
	sprintf(szEventIndex,"%d",m_iVideoIndex);
	Json::Value element;
	element["EventType"]=Json::Value(tCPGTWP->iWeaveKind);		//吃碰杠偎跑提类型
	element["User"]=Json::Value(byDesk);						//当前是谁的
	element["Menpai"] = Json::Value(iMenNums);					//余下的牌
	element["CenterCard"] = tCPGTWP->byCenterCard;				//中心牌
	element["Count"]  = tCPGTWP->byCardCount;					//牌的总数
	Json::Value array_bydata;
	for(int i = 0;i < 4;i++)
	{
		char key_1[10];
		memset(key_1,0,sizeof(key_1));
		sprintf(key_1,"%d",i);
		array_bydata[key_1]=Json::Value(tCPGTWP->byCardList[i]);
	}
	element["byData"]=Json::Value(array_bydata);

	BYTE byCard[MAX_COUNT];
	memset(byCard,0,sizeof(byCard));
	m_pGameLogic->SwitchToCardData(byDataIndex,byCard,MAX_COUNT,iJinNum);
	Json::Value array_handData1;
	for(int i = 0;i < MAX_COUNT;i++)
	{
		char key_1[10];
		memset(key_1,0,sizeof(key_1));
		sprintf(key_1,"%d",i);
		array_handData1[key_1] = Json::Value(byCard[i]);
	}
	element["handData"] = Json::Value(array_handData1);

	m_element[szEventIndex] = Json::Value(element);
	m_iVideoIndex++;
}

//取牌
void CPlayback::SaveGetCard(BYTE byGetCardData,BYTE byCurrendUser,BYTE iMenNums,BYTE byDataIndex[MAX_INDEX],int iJinNum)
{
	char szEventIndex[10];
	memset(szEventIndex,0,sizeof(szEventIndex));
	sprintf(szEventIndex,"%d",m_iVideoIndex);
	Json::Value element;
	element["EventType"] = Json::Value("Get");			//取牌
	element["User"] = Json::Value(byCurrendUser);			//当前玩家
	element["CardValue"] = Json::Value(byGetCardData);	//取得的牌
	element["Menpai"] = Json::Value(iMenNums);			//门牌余下数量

	BYTE byCard[MAX_COUNT];
	memset(byCard,0,sizeof(byCard));
	m_pGameLogic->SwitchToCardData(byDataIndex,byCard,MAX_COUNT,iJinNum);
	Json::Value array_handData1;
	for(int i = 0;i < MAX_COUNT;i++)
	{
		char key_1[10];
		memset(key_1,0,sizeof(key_1));
		sprintf(key_1,"%d",i);
		array_handData1[key_1]=Json::Value(byCard[i]);
	}

	m_element[szEventIndex]=Json::Value(element);
	m_iVideoIndex++;
}

//出牌
void CPlayback::SaveOutCard(BYTE byGetCardData,BYTE byCurrendUser,BYTE iMenNums,BYTE byDataIndex[MAX_INDEX],int iJinNum)
{
	char szEventIndex[10];
	memset(szEventIndex,0,sizeof(szEventIndex));
	sprintf(szEventIndex,"%d",m_iVideoIndex);
	Json::Value element;
	element["EventType"] = Json::Value("Out");			//取牌
	element["User"] = Json::Value(byCurrendUser);		//当前玩家
	element["CardValue"] = Json::Value(byGetCardData);	//取得的牌
	element["Menpai"] = Json::Value(iMenNums);			//门牌余下数量

	BYTE byCard[MAX_COUNT];
	memset(byCard,0,sizeof(byCard));
	m_pGameLogic->SwitchToCardData(byDataIndex,byCard,MAX_COUNT,iJinNum);
	Json::Value array_handData1;
	for(int i = 0;i < MAX_COUNT;i++)
	{
		char key_1[10];
		memset(key_1,0,sizeof(key_1));
		sprintf(key_1,"%d",i);
		array_handData1[key_1]=Json::Value(byCard[i]);
	}

	m_element[szEventIndex]=Json::Value(element);
	m_iVideoIndex++;
}

void CPlayback::SaveNotic()
{

}

void CPlayback::SaveHuInfor()
{

}

void CPlayback::SaveToHardDisk(CServerGameDesk *pDesk,char cTempCode[])
{
	string fileName="E:/web/json/";
	memset(m_VideoCode,0,sizeof(m_VideoCode));
	pDesk->GetVideoCode(cTempCode,20);
	sprintf(m_VideoCode,"%s.json",cTempCode);
	fileName.append(m_VideoCode);
	m_fVideoFile.open(fileName,ios::app);
	m_root["event"]=m_element;
	m_fVideoFile<<m_root.toStyledString()<<endl;
}
