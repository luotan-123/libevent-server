// GameServer.cpp : 
//

#include "NewServerManage.h"

//获取游戏组件信息
extern "C"  BOOL GetServiceInfo(ServerDllInfoStruct * pServiceInfo, UINT uVer)
{
	//效验信息
	if ((DEV_LIB_VER != uVer) || (NULL == pServiceInfo))
	{
		return FALSE;
	}

	//写入信息
	memset(pServiceInfo, 0, sizeof(ServerDllInfoStruct));
	pServiceInfo->uServiceVer = GAME_MAX_VER;
	pServiceInfo->uNameID = NAME_ID;
	pServiceInfo->uDeskPeople = PLAY_COUNT;
	pServiceInfo->uSupportType = SUPPER_TYPE;
	strcpy(pServiceInfo->szGameName, GAMENAME);
	strcpy(pServiceInfo->szWriter, GAME_DLL_AUTHOR);
	strcpy(pServiceInfo->szDllNote, GAME_DLL_NOTE);
	return TRUE;
}

//获取游戏管理接口函数
extern "C" IModuleManageService * CreateServiceInterface(UINT uVer)
{
	if (DEV_LIB_VER == uVer)
	{
		try
		{
			return new CGameModuleTemplate<CNewServerManage, 1, 10>;
		}
		catch (...)
		{
		}
	}
	return NULL;
}
