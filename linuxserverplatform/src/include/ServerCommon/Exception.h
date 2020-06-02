#pragma once

///***********************************************************************************************///

///系统错误
#define EX_UNKNOWN_ERROR			0						///未知错误
#define EX_VIRTUAL_MEM_LOW			1						///虚拟内存不足
#define EX_HEAP_MEM_LOW				2						///堆内存不足

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