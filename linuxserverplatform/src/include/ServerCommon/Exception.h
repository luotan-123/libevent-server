#pragma once

///***********************************************************************************************///

///系统错误
#define EX_UNKNOWN_ERROR			0						///未知错误
#define EX_VIRTUAL_MEM_LOW			1						///虚拟内存不足
#define EX_HEAP_MEM_LOW				2						///堆内存不足

///数据库错误
#define EX_DATA_CONNECT				100						///数据库连接错误
#define EX_DATA_USER				101						///数据库名字错误
#define EX_DATA_PASS				102						///数据库密码错误
#define EX_DATA_EXEC				103						///数据库执行错误

///网络错误
#define EX_SOCKET_CREATE			200						///网络建立错误
#define EX_SOCKET_SEND_ERROR		201						///网络发送错误
#define EX_SOCKET_RECV_ERROR		202						///网络接受错误

///服务错误
#define EX_SERVICE_START			300						///服务启动
#define EX_SERVICE_PAUSE			301						///服务暂停
#define EX_SERVICE_STOP				302						///服务停止
#define EX_SERVICE_BUSY				303						///服务繁忙
#define EX_SERVICE_UNKNOW			304						///未知服务信息

///CRT 异常
#define EX_CRT_INVALID_PARAM		401						///系统函数调用,检测到非法的参数
#define EX_CRT_PURECALL				402						///纯虚函数调用错误
#define EX_CRT_NEW					403						///分配内存错误
#define EX_CRT_TERMINATE			404						///CRT遇到一个未被处理的C++类型化异常时，它会调用terminate()函数

///自定义错误
///***********************************************************************************************///

//异常类
class CException
{
protected:
	UINT					m_uErrorCode;					///错误码
	bool					m_bAutoDelete;					///是否自动删除
	char					m_szMessage[255];				///错误信息
	static bool				m_bShowError;					///是否显示错误

public:
	CException(const char* szErrorMessage, unsigned int uErrorCode = EX_UNKNOWN_ERROR, bool bAutoDelete = true);
	virtual ~CException();

public:
	//获取错误代码
	virtual UINT GetErrorCode() const;
	//获取错误信息
	virtual char const* GetErrorMessage(char* szBuffer, int iBufLength) const;
	//删除函数
	virtual bool Delete();
	//设置是否显示错误
	static bool ShowErrorMessage(bool bShowError);
};