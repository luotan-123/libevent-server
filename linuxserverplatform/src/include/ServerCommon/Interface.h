#pragma once

/********************************************************************************************/
///服务接口定义
/********************************************************************************************/

class CDataBaseManage;				///<数据库模块类
struct NetMessageHead;				///<网络数据包头
struct ManageInfoStruct;			///<初始化信息结构
struct KernelInfoStruct;			///<内核信息结构
struct DataBaseLineHead;			///<数据库队列头结构
struct AsynThreadResultLine;		///<异步线程结果头结构
class CDataLine;

//服务器网络服务接口 
class IServerSocketService
{
	///接口函数
public:
	//网络关闭处理
	virtual bool OnSocketCloseEvent(ULONG uAccessIP, UINT uIndex, UINT uConnectTime, BYTE socketType) = 0;
	//网络消息处理
	virtual bool OnSocketReadEvent(BYTE socketType, NetMessageHead* pNetHead, void* pData, UINT uSize, UINT uIndex) = 0;
	// 获取dataline
	virtual CDataLine* GetDataLine() = 0;
};

///异步线程结果服务接口
class IAsynThreadResultService
{
	///接口函数
public:
	///异步线程结果处理
	virtual bool OnAsynThreadResultEvent(UINT uHandleKind, UINT uHandleResult, const void* pData, UINT uResultSize, UINT uIndex, UINT uMsgID) = 0;
};

///数据库处理服务接口
class IDataBaseHandleService
{
	///接口函数
public:
	///设置参数
	virtual bool SetParameter(IAsynThreadResultService* pRusultService, CDataBaseManage* pDataBaseManage, ManageInfoStruct* pInitData, KernelInfoStruct* pKernelData) = 0;
	///数据库处理接口
	virtual UINT HandleDataBase(DataBaseLineHead* pSourceData) = 0;
};

///组件管理接口
class IModuleManageService
{
	///接口函数
public:
	///初始化函数 
	virtual bool InitService(ManageInfoStruct* pInitData) = 0;
	///取消初始化函数 
	virtual bool UnInitService() = 0;
	///初始化函数 
	virtual bool StartService(UINT& errCode) = 0;
	///初始化函数 
	virtual bool StoptService() = 0;
	///删除函数
	virtual bool DeleteService() = 0;
	///更新函数
	virtual bool UpdateService() = 0;
};
/********************************************************************************************/
