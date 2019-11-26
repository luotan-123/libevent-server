//#include "StdAfx.h"
//#include "Resource.h"
//#include "TableFrameSink.h"
//#include "GameServiceManager.h"
// 
////////////////////////////////////////////////////////////////////////////////////
//
//
//
////构造函数
//CGameServiceManager::CGameServiceManager()
//{
//	//内核属性
//	m_GameServiceAttrib.wKindID=KIND_ID;
//	m_GameServiceAttrib.wChairCount=PlayChair_Max;
//	m_GameServiceAttrib.wSupporType=(GAME_GENRE_GOLD|GAME_GENRE_MATCH);
//
//	//功能标志
//	m_GameServiceAttrib.cbDynamicJoin=TRUE;
//	m_GameServiceAttrib.cbAndroidUser=TRUE;
//	m_GameServiceAttrib.cbOffLineTrustee=FALSE;
//
//	//服务属性
//	m_GameServiceAttrib.dwServerVersion=VERSION_SERVER;
//	m_GameServiceAttrib.dwClientVersion=VERSION_CLIENT;
//	lstrcpyn(m_GameServiceAttrib.szGameName,GAME_NAME,CountArray(m_GameServiceAttrib.szGameName));
//	lstrcpyn(m_GameServiceAttrib.szDataBaseName,TEXT("QPFishLKDB"),CountArray(m_GameServiceAttrib.szDataBaseName));
//	lstrcpyn(m_GameServiceAttrib.szClientEXEName,TEXT("FishLK.EXE"),CountArray(m_GameServiceAttrib.szClientEXEName));
//	lstrcpyn(m_GameServiceAttrib.szServerDLLName,TEXT("FishLKServer.dll"),CountArray(m_GameServiceAttrib.szServerDLLName));
//
//	return;
//}
//
////析构函数
//CGameServiceManager::~CGameServiceManager()
//{
//
//}
//
////接口查询
//VOID * CGameServiceManager::QueryInterface(const IID & Guid, DWORD dwQueryVer)
//{
//	QUERYINTERFACE(IGameServiceManager,Guid,dwQueryVer);
//
//#ifdef VER_IGameServiceCustomTime
//	QUERYINTERFACE(IGameServiceCustomTime,Guid,dwQueryVer);
//#endif 
//
//	QUERYINTERFACE_IUNKNOWNEX(IGameServiceManager,Guid,dwQueryVer);
//
//	return NULL;
//}
//
////创建桌子
//VOID * CGameServiceManager::CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer)
//{
//
//	//变量定义
//	CTableFrameSink * pTableFrameSink=NULL;
//
//	try
//	{
//		//建立对象
//		pTableFrameSink=new CTableFrameSink();
//		if (pTableFrameSink==NULL) throw TEXT("创建失败");
//
//		//查询接口
//		VOID * pObject=pTableFrameSink->QueryInterface(Guid,dwQueryVer);
//		if (pObject==NULL) throw TEXT("接口查询失败");
//
//		return pObject;
//	}
//	catch (...) {}
//
//	//删除对象
//	SafeDelete(pTableFrameSink);
//
//	return NULL;
//}
//
////创建机器
//VOID * CGameServiceManager::CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer)
//{
//	try
//	{
//		//创建组件
//		if( m_AndroidServiceHelper.GetInterface() == NULL )
//		{
//			m_AndroidServiceHelper.SetModuleCreateInfo( _T("FishLKAndroidService.dll"), "CreateGameServiceManager" );
//
//			if( !m_AndroidServiceHelper.CreateInstance() ) throw 0;
//		}
//
//		//创建机器人
//		VOID *pAndroidObject = m_AndroidServiceHelper->CreateAndroidUserItemSink(Guid,dwQueryVer);
//		if( pAndroidObject == NULL ) throw TEXT("创建机器人失败");
//
//		return pAndroidObject;
//	}
//	catch(...) {}
//
//	return NULL;
//}
//
////创建数据
//VOID * CGameServiceManager::CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer)
//{
//	return NULL;
//}
//
////组件属性
////bool CGameServiceManager::GetServiceAttrib(tagGameServiceAttrib & GameServiceAttrib)
//{
//	//设置变量
//	GameServiceAttrib=m_GameServiceAttrib;
//
//	return true;
//}
//
////调整参数
////bool CGameServiceManager::RectifyParameter(tagGameServiceOption & GameServiceOption)
//{
//	// 修改参数
//	GameServiceOption.lCellScore = __max(1L, GameServiceOption.lCellScore);
//
//	// 最低积分
//	if (GameServiceOption.wServerType&GAME_GENRE_GOLD)
//	{
//		// 最少积分
//		GameServiceOption.lMinTableScore = __max(GameServiceOption.lMinTableScore, GameServiceOption.lCellScore);
//
//		// 积分上限
//		if (GameServiceOption.lRestrictScore != 0L)
//		{
//			GameServiceOption.lRestrictScore = __max(GameServiceOption.lRestrictScore,GameServiceOption.lMinTableScore);
//		}
//	}
//
//	return true;
//}
//
////机器脉冲
//DWORD CGameServiceManager::GetAndroidTimerPulse()
//{
//	return 30;
//}
//
////时间范围
//DWORD CGameServiceManager::GetTableSinkTimeRange()
//{
//	return 20;
//}
//
////时间单元
//DWORD CGameServiceManager::GetTimerEngineTimeCell()
//{
//	return 10;
//}
//
////////////////////////////////////////////////////////////////////////////////////
//
////建立对象函数
//extern "C" __declspec(dllexport) VOID * CreateGameServiceManager(const GUID & Guid, DWORD dwInterfaceVer)
//{
//	//设置对象
//	static CGameServiceManager GameServiceManager;
//	return GameServiceManager.QueryInterface(Guid,dwInterfaceVer);
//}
//
////////////////////////////////////////////////////////////////////////////////////
