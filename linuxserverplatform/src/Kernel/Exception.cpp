#include "CommonHead.h"
#include "Exception.h"
#include "log.h"
#include <exception>

//静态变量定义
bool CException::m_bShowError = false;					//是否显示错误

CException::CException(const char* szErrorMessage, unsigned int uErrorCode, bool bAutoDelete)
{
	m_uErrorCode = uErrorCode;
	m_bAutoDelete = bAutoDelete;
	if ((szErrorMessage != NULL) && (szErrorMessage[0] != 0))
	{
		if (strlen(szErrorMessage) < (sizeof(m_szMessage) / sizeof(m_szMessage[0])))
		{
			strcpy(m_szMessage, szErrorMessage);
		}
		else
		{
			strcpy(m_szMessage, "异常错误信息太长");
		}
	}
	else
	{
		strcpy(m_szMessage, "没有异常信息");
	}

	if (m_bShowError == true)
	{
		CON_ERROR_LOG("%s", m_szMessage);
	}
}


CException::~CException()
{
}

//获取错误代码
UINT CException::GetErrorCode() const
{
	if (this != NULL) return m_uErrorCode;
	return 0;
}

//删除函数
bool CException::Delete()
{
	if ((this != NULL) && (m_bAutoDelete == true))
	{
		delete this;
		return true;
	}
	return false;
}

//获取错误信息
char const* CException::GetErrorMessage(char* szBuffer, int iBufLength) const
{
	if (this != NULL)
	{
		//拷贝错误信息
		if ((szBuffer != NULL) && (iBufLength > 0))
		{
			int iCopyLength = Min_(iBufLength - 1, strlen(m_szMessage));
			szBuffer[iCopyLength] = 0;
			memcpy(szBuffer, m_szMessage, iCopyLength * sizeof(char));
		}
		return m_szMessage;
	}
	return NULL;
}

//设置是否显示错误
bool CException::ShowErrorMessage(bool bShowError)
{
	m_bShowError = bShowError;
	return m_bShowError;
}