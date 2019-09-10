#pragma once

#include "Interface.h"
#include "ComStruct.h"
#include "DataLine.h"
#include "TCPSocket.h"
#include "RedisCenter.h"
#include "RedisPHP.h"
#include "Exception.h"
#include "GameLogManage.h"

//基础数据管理类
class CBaseCenterServer : public IServerSocketService
{
protected:
	bool			m_bRun;					//运行标志
	bool			m_bInit;				//初始化标志
	HWND			m_hWindow;				//窗口句柄
	HANDLE			m_hWindowThread;		//窗口线程
	HANDLE			m_hHandleThread;		//处理线程
	HANDLE			m_hCompletePort;		//处理完成端口
	CDataLine		m_DataLine;				//数据队列

public:
	ManageInfoStruct						m_InitData;					//初始化数据
	KernelInfoStruct						m_KernelData;				//内核数据
	ServerDllInfoStruct						m_DllInfo;					//DLL信息
	CTCPSocketManage						m_TCPSocket;				//网络模块
	CRedisCenter*							m_pRedis;					//redis
	CRedisPHP*								m_pRedisPHP;				//PHP服务器redis

public:
	CBaseCenterServer();
	virtual ~CBaseCenterServer();

	//公共使用功能函数 （本处理线程调用）
public:
	bool SetTimer(UINT uTimerID, UINT uElapse);
	bool KillTimer(UINT uTimerID);

	//服务接口函数 （主线程调用）
public:
	virtual bool Init(ManageInfoStruct * pInitData);
	virtual bool UnInit();
	virtual bool Start();
	virtual bool Stop();

	//服务接口函数 （主线程调用）
private:
	//获取信息函数 （必须重载）
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData) = 0;
	//数据管理模块初始化
	virtual bool OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData) { return true; };
	//数据管理模块卸载
	virtual bool OnUnInit() { return true; }
	//数据管理模块启动
	virtual bool OnStart() { return true; }
	//数据管理模块关闭
	virtual bool OnStop() { return true; }

	//服务扩展接口函数 （本处理线程调用）
private:
	//SOCKET 数据读取 （必须重载）
	virtual bool OnSocketRead(NetMessageHead * pNetHead, CenterServerMessageHead * pCenterHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID) = 0;
	//SOCKET 关闭 （必须重载）
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, UINT uConnectTime) = 0;
	//定时器消息 （必须重载）
	virtual bool OnTimerMessage(UINT uTimerID) = 0;

	//服务接口函数 （其他服务线程调用）
public:
	//网络关闭处理 （不需要重载）
	virtual bool OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime);
	//网络消息处理 （不需要重载）为Z服务器准备
	virtual bool OnSocketReadEvent(CTCPSocket* pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID);

	virtual CDataLine* GetDataLine() { return &m_DataLine; }

	//内部函数（主线程调用）
private:
	//创建窗口为了生成定时器
	bool CreateWindowsForTimer();
	//定时器通知消息
	bool WindowTimerMessage(UINT uTimerID);
	//队列数据处理线程
	static unsigned __stdcall LineDataHandleThread(LPVOID pThreadData);
	//WINDOW 消息循环线程
	static unsigned __stdcall WindowMsgThread(LPVOID pThreadData);
	//窗口回调函数
	static LRESULT CALLBACK WindowProcFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};