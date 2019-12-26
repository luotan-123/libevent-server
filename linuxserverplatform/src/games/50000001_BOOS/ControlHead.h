#pragma once

#include "CMD_Game.h"

#define CreateInterface( Hinstance, InterfacePoint, InterfaceName, CallDllName, CallFunctionName )		\
InterfacePoint = NULL;																					\
Hinstance = LoadLibrary( _T( CallDllName ) );															\
if ( Hinstance )																						\
{																										\
	typedef void * (*CREATE)(); 																		\
	CREATE pFunction = (CREATE)GetProcAddress( Hinstance, CallFunctionName ); 							\
	if ( pFunction )																					\
	{																									\
		InterfacePoint = static_cast< InterfaceName * >( pFunction() );									\
	}																									\
}																										



#define ReleaseInterface(Hinstance, InterfacePoint)	\
if( InterfacePoint )									\
{														\
	InterfacePoint->Release();							\
	InterfacePoint = NULL;								\
}														\
if( Hinstance )											\
{														\
	FreeLibrary(Hinstance);								\
	Hinstance = NULL;									\
}								

// 个人难度
typedef unordered_map< unsigned long, double > CMapPersonalDifficulty;

// 接口信息


// 控制回调
class IServerControlCallback
{
	// 返回配置
	virtual void GetCustomRule( tagCustomRule & nConfigInfo ) = 0;
	// 设置配置
	virtual void SetCustomRule( tagCustomRule & nConfigInfo, bool bSaveFile ) = 0;
	// 返回个人难度
	virtual void GetPersonalDifficulty( CMapPersonalDifficulty & MapPersonalDifficulty ) = 0;
	// 设置个人难度
	virtual void SetPersonalDifficulty( unsigned long lPlayID, double dPersonalDifficulty ) = 0;
	// 删除个人难度
	virtual void DeletePersonalDifficulty( unsigned long lPlayID ) = 0;
	// 清空个人难度
	virtual void ClearPersonalDifficulty() = 0;
	// 获取库存
	virtual void GetStock( LONGLONG & lStockInitial, LONGLONG & lStockCurrent ) = 0;
	// 设置桌子库存
	virtual bool SetTableStock( unsigned short TableID, int nTableStock ) = 0;
	// 控制信息
	virtual bool OnControlInfo( void * pIServerUserItem, uint nMessageID, void * pData, uint nSize ) = 0;
};


// 服务器接口
class IServerControl
{
	// 释放接口
	virtual void Release() = 0;
	// 创建函数
	virtual bool Create( IServerControlCallback * pIServerControlCallback ) = 0;
	// 消息函数
	virtual bool OnControlMessage( void * pIServerUserItem, uint nMessageID, void * pData, uint nSize ) = 0;
};


// 客户端消息
#define SUBC_C_GAME_QUERY				1			// 游戏查询
#define SUBC_C_GAME_SET					2			// 游戏设置

#define SUBC_C_SET_TABLE				3			// 桌子设置
#define SUBC_C_SET_PERSONAL				4			// 个人难度
#define SUBC_C_DELETE_PERSONAL			5			// 个人删除
#define SUBC_C_QUERY_PERSONAL			6			// 查询难度
#define SUBC_C_CLEAR_PERSONAL			7			// 清空难度
#define SUBC_C_UPDATA_STOCK				8			// 更新库存

// 设置信息
struct CMDC_C_GameSet
{
	// 配置信息
	tagCustomRule			nConfigInfo;			// 配置信息

	// 是否保存
	bool					bSaveFile;				// 保存文件
};

// 桌子设置
struct CMDC_C_SetTable
{
	unsigned short			TableID;				// 桌子ID
	int						nTableStock;			// 桌子库存
};

// 个人设置
struct CMDC_C_SetPersonal
{
	unsigned long			lPlayID;				// 个人ID
	double					dPersonalDifficulty;	// 个人难度
};

// 个人删除
struct CMDC_C_DeletePersonal
{
	unsigned long			lPlayID;				// 个人ID
};

// 服务器消息
#define SUBC_S_GAME_QUERY				1			// 游戏查询
#define SUBC_S_TEXT						2			// 文字提示
#define SUBC_S_QUERY_PERSONAL			3			// 查询难度
#define SUBC_S_UPDATA_STOCK				4			// 更新库存

// 游戏查询
struct CMDC_S_GameQuery
{
	// 配置信息
	tagCustomRule			nConfigInfo;			// 配置信息

	// 查询信息
	enum EnumQuery
	{
		EnumQuery_Query,
		EnumQuery_Set,
		EnumQuery_SetSave,
	};
	EnumQuery				nEnumQuery;				// 查询结构
};

// 文字提示
struct CMDC_S_Text
{
	// 配置信息
	char					szTextInfo[64];			// 提示信息
};

// 查询难度
struct CMDC_S_QueryPersonal
{
	unsigned long			lPlayID;				// 个人ID
	double					dPersonalDifficulty;	// 个人难度
};

// 更新库存
struct CMDC_S_UpdataStock
{
	LONGLONG				lStockInitial;			// 初始库存
	LONGLONG				lStockCurrent;			// 当前库存
};