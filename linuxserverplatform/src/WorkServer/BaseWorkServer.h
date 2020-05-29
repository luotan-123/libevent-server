#pragma once

#include "DataBase.h"
#include "RedisPHP.h"
#include "RedisLogon.h"
#include "ServerTimer.h"
#include "TcpConnect.h"
#include "GServerConnect.h"
#include "INIFile.h"
#include "Exception.h"
#include "PlatformMessage.h"
#include "NewMessageDefine.h"


//基础数据管理类
class CBaseWorkServer : public IServerSocketService, public IAsynThreadResultService
{
protected:
	bool			m_bRun;					//运行标志
	bool			m_bInit;				//初始化标志
	HANDLE			m_hHandleThread;		//处理线程
	CDataLine		m_DataLine;				//数据队列
	HANDLE			m_connectCServerHandle;	//与中心服交互线程句柄

public:
	ManageInfoStruct						m_InitData;					//初始化数据
	KernelInfoStruct						m_KernelData;				//内核数据
	ServerDllInfoStruct						m_DllInfo;					//DLL信息
	CDataBaseManage							m_SQLDataManage;			//数据库模块
	CRedisLogon* m_pRedis;					//redis
	CRedisPHP* m_pRedisPHP;					//连接php的redis server
	CTcpConnect* m_pTcpConnect;				//与中心服务器的连接
	CGServerConnect* m_pGServerConnect;		//与网关服务器的连接
	CServerTimer* m_pServerTimer;			//服务器定时器

public:
	CBaseWorkServer();
	virtual ~CBaseWorkServer();

	//公共使用功能函数 （本处理线程调用）
public:
	bool SetTimer(UINT uTimerID, UINT uElapse, BYTE timerType = SERVERTIMER_TYPE_PERISIST);
	bool KillTimer(UINT uTimerID);

	//服务接口函数 （主线程调用）
public:
	virtual bool Init(ManageInfoStruct* pInitData, IDataBaseHandleService* pDataHandleService);
	virtual bool UnInit();
	virtual bool Start();
	virtual bool Stop();

	//服务接口函数 （主线程调用）
private:
	//获取信息函数 （必须重载）
	virtual bool PreInitParameter(ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData) = 0;
	//数据管理模块初始化
	virtual bool OnInit(ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData) { return true; };
	//数据管理模块卸载
	virtual bool OnUnInit() { return true; }
	//数据管理模块启动
	virtual bool OnStart() { return true; }
	//数据管理模块关闭
	virtual bool OnStop() { return true; }

	//服务扩展接口函数 （本处理线程调用）
private:
	//SOCKET 数据读取 （必须重载）
	virtual bool OnSocketRead(NetMessageHead* pNetHead, void* pData, UINT uSize, UINT uIndex) = 0;
	//SOCKET 关闭 （必须重载）
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime, BYTE socketType) = 0;
	//异步线程处理结果 （必须重载）
	virtual bool OnAsynThreadResult(AsynThreadResultLine* pResultData, void* pData, UINT uSize) = 0;
	//定时器消息 （必须重载）
	virtual bool OnTimerMessage(UINT uTimerID) = 0;

	//服务接口函数 （其他服务线程调用）
public:
	//网络关闭处理 
	virtual bool OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime, BYTE socketType);
	//网络消息处理 
	virtual bool OnSocketReadEvent(BYTE socketType, NetMessageHead* pNetHead, void* pData, UINT uSize, UINT uIndex);
	//异步线程结果处理
	virtual bool OnAsynThreadResultEvent(UINT uHandleKind, UINT uHandleResult, const void* pData, UINT uResultSize, UINT uIndex, UINT uMsgID);

	virtual CDataLine* GetDataLine() { return &m_DataLine; }

	//内部函数（主线程调用）
private:
	//队列数据处理线程
	static void* LineDataHandleThread(void* pThreadData);
	//与中心服务器连接线程
	static void* TcpConnectThread(void* pThreadData);

private:
	//处理中心服消息
	virtual bool OnCenterServerMessage(UINT msgID, NetMessageHead* pNetHead, void* pData, UINT size, int userID) = 0;
};