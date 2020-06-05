#pragma once

#include "BaseWorkServer.h"

// 定时器ID
enum WorkServerTimerID
{
	LOGON_TIMER_BEGIN = 0,
	LOGON_TIMER_CHECK_REDIS_CONNECTION,			// redis连接性
	LOGON_TIMER_ROUTINE_SAVE_REDIS,				// 定期存储redis数据
	LOGON_TIMER_NORMAL,							// 通用定时器(s)
	LOGON_TIMER_END,
};

const int CHECK_REDIS_SAVE_DB = 61;					// 定期存储redis数据(s)
const int CHECK_REDIS_CONNECTION_SECS = 307;		// 定期检查redis连接(s)
const int NORMAL_TIMER_SECS = 1;					// 通用定时器(s)


class CGameWorkManage : public CBaseWorkServer
{
public:
	//分布式处理相关
	UINT m_uGroupIndex;		//逻辑服集群索引
	UINT m_uGroupCount;		//逻辑服集群数量
	UINT m_uMainGroupIndex;	//当前主要逻辑服集群索引（相比其它服务器集群处理更多数据）

private:
	std::vector<int>			m_buyRoomVec;
	time_t						m_lastNormalTimerTime;
	std::vector<UINT>			m_socketIndexVec;		// socket索引，遍历在线tcpsocket需要

public:
	CGameWorkManage();
	virtual ~CGameWorkManage();

public:
	//数据管理模块启动
	virtual bool OnStart();
	//数据管理模块关闭
	virtual bool OnStop();

	//服务扩展接口函数
private:
	//获取信息函数
	virtual bool PreInitParameter(ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData);
	//SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead* pNetHead, void* pData, UINT uSize, UINT uIndex);
	//SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime, BYTE socketType);
	//异步线程处理结果
	virtual bool OnAsynThreadResult(AsynThreadResultLine* pResultData, void* pData, UINT uSize);
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);


private:
	// 桌子相关
	bool OnHandleGameDeskMessage(int assistID, void* pData, int size, unsigned int socketIdx, int userID);

	bool OnHandleUserBuyDesk(int userID, void* pData, int size, unsigned int socketIdx);
	bool OnHandleUserEnterDesk(int userID, void* pData, int size, unsigned int socketIdx);

	// 其他相关
	bool OnHandleOtherMessage(int assistID, void* pData, int size, unsigned int socketIdx, int userID);

	//刷新个人信息
	bool OnHandleUserInfoFlush(int userID, void* pData, int size, unsigned int socketIdx);
	//机器人取钱
	bool OnHandleRobotTakeMoney(int userID, void* pData, int size, unsigned int socketIdx);
	// 请求玩家信息
	bool OnHandleReqUserInfo(int userID, void* pData, int size, unsigned int socketIdx);

public:
	// 通过索引发送数据
	bool SendData(int index, void* pData, int size, int mainID, int assistID, int handleCode, unsigned int uIdentification);
	// 通知资源变化, value为总值，不是变化值
	void NotifyResourceChange(int userID, int resourceType, long long value, int reason, long long changeValue);

public:
	// 向中心服务器发送消息
	bool SendMessageToCenterServer(UINT msgID, void* pData, UINT size, int userID = 0, UINT mainID = 0, UINT assistID = 0, UINT handleCode = 0);

public:
	// 判断roomID的服务器是否存在
	bool IsRoomIDServerExists(int roomID);
	// 删除程序生成的无效文件
	void DeleteExpireFile();
	// 玩家是否在线
	bool IsUserOnline(int userID);
	// 自动开房
	bool AutoCreateRoom(const SaveRedisFriendsGroupDesk& deskInfo);
	// 获取随机头像
	std::string GetRandHeadURLBySex(BYTE sex);
	//分布式
	bool IsMainDistributedSystem(); //判断是否是当前主要集群系统
	bool IsDistributedSystemCalculate(long long calcID); //这个id是否当前系统计算
private:
	// 初始化需要定期检查的事件
	void InitRounteCheckEvent();
	// 检查redis连接性
	void CheckRedisConnection();
	// 定时存储redis数据到DB updateAll=是否全部更新
	void RountineSaveRedisDataToDB(bool updateAll);
	// 通用定时器
	void OnNormalTimer();
	// 跨天
	void OnServerCrossDay();
	// 凌晨12点
	void OnServerCrossDay12Hour();
	// 跨周
	void OnServerCrossWeek();
	// 清除胜局榜
	void CleanAllUserWinCount();
	// 清理金币钻石排行榜
	void CleanAllUserMoneyJewelsRank();
	// 玩家跨天
	void OnUserCrossDay(int userID, int time);
	// 清理数据库过期数据
	void ClearDataBase();
	// 生成新表记录账单数据
	void AutoCreateNewTable(bool start);
	// 异步执行sql语句
	void ExecuteSqlSecond();

	// 中心服消息相关
private:
	// 处理中心服务器的消息
	virtual bool OnCenterServerMessage(UINT msgID, NetMessageHead* pNetHead, void* pData, UINT size, int userID);
	// 服务器id重复处理
	bool OnCenterRepeatIDMessage(void* pData, int size);
	// 分布式系统信息
	bool OnCenterDistributedSystemInfoMessage(void* pData, int size);
	// 关服处理
	bool OnCenterCloseServerMessage(void* pData, int size);
	// 开服处理
	bool OnCenterOpenServerMessage(void* pData, int size);
	// 中心服自动开房
	bool OnCenterAutoCreateRoomMessage(void* pData, UINT size);

	//向PHP请求接口相关
private:
	// 给php发送消息接口
	void SendHTTPMessage(int userID, const std::string& url, BYTE postType);
};
