// UpGradeDLL.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "ServerManage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// 如果使用 lpReserved，请将此移除
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		ERROR_LOG("UpGradeDLL.DLL 正在初始化！\n");

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		ERROR_LOG("UpGradeDLL.DLL 正在终止！\n");
	}
	return 1;   // 确定
}

//获取游戏组件信息
extern "C" __declspec(dllexport) BOOL GetServiceInfo(ServerDllInfoStruct * pServiceInfo, UINT uVer)
{
	//效验信息
	if ((uVer!=DEV_LIB_VER)||(pServiceInfo==NULL)) return FALSE;

	//写入信息
	memset(pServiceInfo,0,sizeof(ServerDllInfoStruct));
	pServiceInfo->uServiceVer=GAME_MAX_VER;
	pServiceInfo->uNameID=NAME_ID;
	pServiceInfo->uDeskPeople=PLAY_COUNT;
	pServiceInfo->uSupportType=SUPPER_TYPE;
	lstrcpy(pServiceInfo->szGameName,GAMENAME);
	///lstrcpy(pServiceInfo->szGameTable,GAME_TABLE_NAME);
	///lstrcpy(pServiceInfo->szDLLFileName,SERVER_DLL_NAME);
	lstrcpy(pServiceInfo->szWriter,TEXT("  "));//龙浩天"));
	lstrcpy(pServiceInfo->szDllNote,TEXT("牌类 -- 斗地主游戏组件"));
	return TRUE;
}

//获取游戏管理接口函数
extern "C" __declspec(dllexport) IModuleManageService * CreateServiceInterface(UINT uVer)
{
	if (uVer==DEV_LIB_VER) 
	{
		try	{ return new CGameModuleTemplate<CServerGameDesk,1,10>; }
		catch (...) {}
	}
	return NULL;
}
