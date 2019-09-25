#pragma once

#include "DataBase.h"
#include "RedisLoader.h"
#include "RedisPHP.h"
#include "TcpConnect.h"
#include "ServerTimer.h"
#include "GServerConnect.h"
#include "INIFile.h"

//基础数据管理类
class CBaseMainManage : public IAsynThreadResultService
{
protected:
	volatile bool							m_bRun;						//运行标志
	bool									m_bInit;					//初始化标志
	HANDLE									m_hHandleThread;			//处理线程
	HANDLE									m_connectCServerHandle;		//与中心服交互线程句柄
	CDataLine								m_DataLine;					//数据队列
	
public:
	ManageInfoStruct						m_InitData;					//初始化数据
	KernelInfoStruct						m_KernelData;				//内核数据
	ServerDllInfoStruct						m_DllInfo;					//DLL信息
	CDataBaseManage							m_SQLDataManage;			//数据库模块
	CRedisLoader*							m_pRedis;					//redis
	CRedisPHP*								m_pRedisPHP;				//连接php的redis server
	CTcpConnect*							m_pTcpConnect;				//与中心服务器的连接
	CGServerConnect*						m_pGServerConnect;			//与登录服务器的连接
	CServerTimer*							m_pServerTimer;				//服务器定时器
public:
	CBaseMainManage();
	virtual ~CBaseMainManage();

	CRedisLoader* GetRedis() { return m_pRedis; }
	CRedisPHP* GetRedisPHP() { return m_pRedisPHP; }

public:
	//初始化函数
	virtual bool Init(ManageInfoStruct * pInitData, IDataBaseHandleService * pDataHandleService);
	//取消初始化函数
	virtual bool UnInit();
	//启动函数
	virtual bool Start();
	//停止服务
	virtual bool Stop();
	//刷新服务
	virtual bool Update();
	//设定定时器
	bool SetTimer(UINT uTimerID, UINT uElapse, BYTE timerType = SERVERTIMER_TYPE_PERISIST);
	//清除定时器
	bool KillTimer(UINT uTimerID);
	//异步线程结果处理
	virtual bool OnAsynThreadResultEvent(UINT uHandleKind, UINT uHandleResult, void * pData, UINT uResultSize, UINT uDataType, UINT uHandleID);
	//获取队列
	virtual CDataLine* GetDataLine() { return &m_DataLine; }

	//服务接口函数
private:
	//获取信息函数
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData) = 0;
	//数据管理模块初始化
	virtual bool OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData) { return true; }
	//数据管理模块卸载
	virtual bool OnUnInit() { return true; }
	//数据管理模块启动
	virtual bool OnStart() { return true; }
	//数据管理模块关闭
	virtual bool OnStop() { return true; }
	//数据管理刷新模块
	virtual bool OnUpdate() { return true; }
	//socket数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, UINT dwHandleID) = 0;
	//socket关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime) = 0;
	//异步线程处理结果
	virtual bool OnAsynThreadResult(AsynThreadResultLine * pResultData, void * pData, UINT uSize) = 0;
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID) = 0;
	//队列数据处理线程
	static void* LineDataHandleThread(void* pThreadData);
	//与中心服务器连接线程
	static void* TcpConnectThread(void* pThreadData);

private:
	//处理中心服消息
	virtual bool OnCenterServerMessage(UINT msgID, NetMessageHead * pNetHead, void* pData, UINT size, int userID) = 0;
};