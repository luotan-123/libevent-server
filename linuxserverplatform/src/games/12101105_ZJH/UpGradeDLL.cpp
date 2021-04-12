// UpGradeDLL.cpp : 定义 DLL 的初始化例程。
//

#include "ServerManage.h"


//获取游戏组件信息
extern "C"  BOOL GetServiceInfo(ServerDllInfoStruct * pServiceInfo, UINT uVer)
{
	//效验信息
	if ((uVer!=DEV_LIB_VER)||(pServiceInfo==NULL)) return FALSE;

	//写入信息
	memset(pServiceInfo,0,sizeof(ServerDllInfoStruct));
	pServiceInfo->uServiceVer=GAME_MAX_VER;
	pServiceInfo->uNameID=NAME_ID;
	pServiceInfo->uDeskPeople=PLAY_COUNT;
	pServiceInfo->uSupportType=SUPPER_TYPE;
	strcpy(pServiceInfo->szGameName,GAMENAME);
	strcpy(pServiceInfo->szDllNote,"牌类 -- 景德镇两副头游戏组件");
	return TRUE;
}

//获取游戏管理接口函数
extern "C"  IModuleManageService * CreateServiceInterface(UINT uVer)
{
	if (uVer==DEV_LIB_VER) 
	{
		try	{ return new CGameModuleTemplate<CServerGameDesk,1,10>; }
		catch (...) {}
	}
	return NULL;
}
