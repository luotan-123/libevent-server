// GameServer.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "NewServerManage.h"

#ifdef _MANAGED
#error 请阅读 GameServer.cpp 中的说明以使用 /clr 进行编译
// 如果要向您的项目中添加 /clr，必须执行下列操作:
//	1. 移除上述对 afxdllx.h 的包含
//	2. 向没有使用 /clr 且已禁用预编译头的
//	   项目添加一个 .cpp 文件，其中含有下列文本:
//			#include <afxwin.h>
//			#include <afxdllx.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _MANAGED
#pragma managed(push, off)
#endif

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// 如果使用 lpReserved，请将此移除
	UNREFERENCED_PARAMETER(lpReserved);

	return 1;   // 确定
}

//获取游戏组件信息
extern "C" __declspec(dllexport) BOOL GetServiceInfo(ServerDllInfoStruct * pServiceInfo, UINT uVer)
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
	lstrcpy(pServiceInfo->szGameName, GAMENAME);
	lstrcpy(pServiceInfo->szWriter, GAME_DLL_AUTHOR);
	lstrcpy(pServiceInfo->szDllNote, GAME_DLL_NOTE);
	return TRUE;
}

//获取游戏管理接口函数
extern "C" __declspec(dllexport) IModuleManageService * CreateServiceInterface(UINT uVer)
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
#ifdef _MANAGED
#pragma managed(pop)
#endif

