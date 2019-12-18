#pragma once

#pragma pack(1)

///游戏组件信息结构 （服务器动态库组件获取）
struct ServerDllInfoStruct
{
	///游戏组件信息
	UINT						uServiceVer;						///服务版本
	UINT						uNameID;							///游戏名字 ID
	UINT						uSupportType;						///支持类型
	UINT						uDeskPeople;						///游戏人数
	char						szGameName[61];						///游戏名字
	char						szGameTable[61];					///数据库表名字
	char						szDLLFileName[61];					///文件名字

	///辅助信息
	char						szWriter[61];						///软件开发者
	char						szDllNote[51];						///备注信息
};

//公共组件加载信息结构	（服务器设置数据）
struct ManageInfoStruct
{
	bool						bPrivate;							//是否私有房
	char						szGameTable[31];					///信息表名字
	char						szGameRoomName[61];					///服务器名字
	///中心数据库信息
	char						szSQLIP[16];						///中心数据库地址
	char						szSQLName[61];						///中心数据库用户名字
	char						szSQLPass[61];						///中心数据库密码
	///登陆数据库信息
	char						szLogonSQLIP[16];					///登陆数据库地址
	///本地数据库信息
	char						szNativeSQLIP[61];					///本地数据库地址
	///混战场房间信息表
	char						szBattleGameTable[31];
	char						szLockTable[31];					///锁定表名字
	char						szIPRuleTable[31];					///IP 限制表名字
	char						szNameRuleTable[31];				///UserName限制表名字
	///游戏信息
	UINT						uNameID;							///游戏名字 ID
	UINT						dwRoomRule;							///游戏设置规则
	UINT						dwUserPower;						///游戏附加权限
	///设置信息
	UINT						uComType;							///游戏类型
	UINT						uDeskType;							///游戏桌类型，主要为添加百家乐桌
	UINT						uBasePoint;							///游戏基础分
	UINT						uLessPoint;							///游戏的最少分数
	UINT						uMaxPoint;							///游戏的最大分数
	UINT						uMaxLimite;							///最大封顶值
	UINT						uRunPublish;						///逃跑扣分
	UINT						uTax;								///税收比例
	UINT						uListenPort;						///监听端口
	UINT						uMaxPeople;							///最大连接数目
	char						szCenterIP[24];						///中心服ip
	///服务器信息
	UINT						uRoomID;							///游戏服务器 ID 号码
	int							iRoomType;							///房间类型
	int							iRoomSort;							///房间分类
	int							iRoomLevel;							///房间等级
	UINT						uDeskCount;							///游戏桌数目
	UINT						uStopLogon;							///停止登陆房间
	char						bCanCombineDesk;					///是否可以组桌

	ManageInfoStruct()
	{
		memset(this, 0, sizeof(ManageInfoStruct));
	}
};

//内核数据 （服务器 动态库 组件启动设置）
struct KernelInfoStruct
{
	UINT						uNameID;							//游戏名字 ID 号码
	UINT						uDeskPeople;						//游戏人数
	BYTE						bMaxVer;							//软件最高版本
	BYTE						bLessVer;							//软件最低版本
	BYTE						bStartSQLDataBase;					//使用数据库连接
	BYTE						bNativeDataBase;					//使用本地数据库
	BYTE						bLogonDataBase;						//使用登陆数据库
	BYTE						bStartTCPSocket;					//是否启动网络
	UINT						uTimerCount;						//定时器数量

	KernelInfoStruct()
	{
		memset(this, 0, sizeof(KernelInfoStruct));
	}
};

#pragma pack()

