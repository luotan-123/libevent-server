#pragma once

#include "GameMainManage.h"

//房间启动信息结构
struct GameRoomInitStruct
{
	//进程模式
	UINT				  uRunStation;					//运行状态
	HANDLE				  hProcessHandle;				//进程句柄

	//DLL 模式
	HINSTANCE			  hDllInstance;					//组件句柄
	IModuleManageService* pIManageService;				//服务接口

	//设置信息
	bool				  bEnable;						//是否启用
	UINT				  uKindID;						//类型 ID
	UINT				  uSortID;						//排列 ID
	UINT				  uServiceID;					//组件 ID
	TCHAR				  szRoomNote[50];				//房间备注
	ManageInfoStruct	  InitInfo;						//初始化信息
	ServerDllInfoStruct	  ServiceInfo;					//组件信息

	GameRoomInitStruct()
	{
		memset(this, 0, sizeof(GameRoomInitStruct));
	}
};

// 房间管理模块
class CLoaderServerModule
{
public:
	CLoaderServerModule();
	~CLoaderServerModule();

	int StartAllRoom();
	void StopAllRoom();
	void StartRoomByRoomID(int roomID);
	void StopRoomByRoomID(int roomID);
	void UpdateAllRoom();
	void UpdateRoomByRoomID(int roomID);

private:
	bool LoadServiceInfo(GameRoomInitStruct* pGameRoomInfo); // 获取动态库信息
	void MakeInitData(GameRoomInitStruct* pGameRoomInfo, int roomID); //初始化数据
	bool StartGameRoom(GameRoomInitStruct* pGameRoomInfo); //启动游戏房间
	bool StopGameRoom(GameRoomInitStruct* pGameRoomInfo); //停止游戏房间
private:
	std::map<int, GameRoomInitStruct> m_allRoomInfoMap;
	char m_szError[255]; //错误信息
};