#pragma once

#include "Interface.h"
#include "ComStruct.h"
#include "DataLine.h"
#include "TCPSocket.h"
#include "DataBase.h"
#include "RedisLoader.h"
#include "RedisPHP.h"
#include "TcpConnect.h"
#include "GServerConnect.h"

//基础数据管理类
class KERNEL_CLASS CBaseMainManage : public IServerSocketService, public IAsynThreadResultService
{
protected:
	bool									m_bRun;						//运行标志
	bool									m_bInit;					//初始化标志
	HWND									m_hWindow;					//窗口句柄
	HANDLE									m_hWindowThread;			//窗口线程
	HANDLE									m_hHandleThread;			//处理线程
	HANDLE									m_hCompletePort;			//处理完成端口
	CDataLine								m_DataLine;					//数据队列
	HANDLE									m_connectCServerHandle;		//与中心服交互线程句柄

public:
	ManageInfoStruct						m_InitData;					//初始化数据
	KernelInfoStruct						m_KernelData;				//内核数据
	ServerDllInfoStruct						m_DllInfo;					//DLL信息
	CDataBaseManage							m_SQLDataManage;			//数据库模块
	CRedisLoader*							m_pRedis;					//redis
	CRedisPHP*								m_pRedisPHP;				//连接php的redis server
	CTcpConnect*							m_pTcpConnect;				//与中心服务器的连接
	CGServerConnect*						m_pGServerConnect;			//与登录服务器的连接

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
	bool SetTimer(UINT uTimerID, UINT uElapse);
	//清除定时器
	bool KillTimer(UINT uTimerID);
	//网络关闭处理 
	virtual bool OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime);
	//网络消息处理
	virtual bool OnSocketReadEvent(CTCPSocket * pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID);
	//异步线程结果处理
	virtual bool OnAsynThreadResultEvent(UINT uHandleKind, UINT uHandleResult, void * pData, UINT uResultSize, UINT uDataType, UINT uHandleID);

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
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID) = 0;
	//socket关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime) = 0;
	//异步线程处理结果
	virtual bool OnAsynThreadResult(AsynThreadResultLine * pResultData, void * pData, UINT uSize) = 0;
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID) = 0;
	//内核启动函数
	bool AFCKernelStart();
	//定时器通知消息
	bool WindowTimerMessage(UINT uTimerID);
	//队列数据处理线程
	static unsigned __stdcall LineDataHandleThread(LPVOID pThreadData);
	//window消息循环线程
	static unsigned __stdcall WindowMsgThread(LPVOID pThreadData);
	//窗口回调函数
	static LRESULT CALLBACK WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//与中心服务器连接线程
	static unsigned __stdcall TcpConnectThread(LPVOID pThreadData);

private:
	//处理中心服消息
	virtual bool OnCenterServerMessage(UINT msgID, NetMessageHead * pNetHead, void* pData, UINT size, int userID) = 0;
};