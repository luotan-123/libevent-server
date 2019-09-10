#include "pch.h"
#include "Exception.h"
#include <exception>

//静态变量定义
bool CException::m_bShowError = false;					//是否显示错误


CException::CException(const char * szErrorMessage, unsigned int uErrorCode, bool bAutoDelete)
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
			strcpy(m_szMessage, TEXT("异常错误信息太长"));
		}
	}
	else
	{
		strcpy(m_szMessage, TEXT("没有异常信息"));
	}

	if (m_bShowError == true)
	{
		printf("%s\n", m_szMessage);
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
TCHAR const * CException::GetErrorMessage(TCHAR * szBuffer, int iBufLength) const
{
	if (this != NULL)
	{
		//拷贝错误信息
		if ((szBuffer != NULL) && (iBufLength > 0))
		{
			int iCopyLength = __min(iBufLength - 1, ::lstrlen(m_szMessage));
			szBuffer[iCopyLength] = 0;
			::CopyMemory(szBuffer, m_szMessage, iCopyLength * sizeof(TCHAR));
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

//////////////////////////////////////////////////////////////////////////

// 获得结构化异常信息。
PEXCEPTION_POINTERS CWin32Exception::ExceptionInformation()
{
	return m_pException;
}

// 获得结构化异常代码。
DWORD CWin32Exception::ExceptionCode()
{
	if (NULL != m_pException)
		return m_nSE;
	else
		return 0;
}

// 转化异常函数
void CWin32Exception::TransWin32Exception(unsigned int u, EXCEPTION_POINTERS* pExp)
{
	throw CWin32Exception(u, pExp);
}

// CRT异常
void CWin32Exception::TerminateHandler()
{
	throw EX_CRT_TERMINATE;
}

// 设置异常函数
void CWin32Exception::SetWin32ExceptionFunc()
{
	// 设置异常处理转换函数
	_set_se_translator(CWin32Exception::TransWin32Exception);

	// CRT异常
	set_terminate(CWin32Exception::TerminateHandler);
}

// 根据异常代码获取描述
const char * CWin32Exception::GetDescByCode(unsigned int uCode)
{
	switch (uCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		return "线程违规读写没有适当权限的虚拟地址。";

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		return "线程在底层硬件支持的边界检查下访问数组越界。";

	case EXCEPTION_BREAKPOINT:
		return "遇到一个断点。";

	case EXCEPTION_DATATYPE_MISALIGNMENT:
		return "线程试图在不支持对齐的硬件上读写未对齐的数据。";

	case EXCEPTION_FLT_DENORMAL_OPERAND:
		return "不正规的值表示太小而不能表示一个标准的浮点值。";

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		return "线程除零操作。";

	case EXCEPTION_FLT_INEXACT_RESULT:
		return "浮点结果的操作不能准确的代替小数。";

	case EXCEPTION_FLT_INVALID_OPERATION:
		return "这个异常代表了这个列表里没有列出的其他浮点异常。";

	case EXCEPTION_FLT_OVERFLOW:
		return "浮点操作的指数超过了相应类型的最大值。";

	case EXCEPTION_FLT_STACK_CHECK:
		return "浮点操作的栈越界或下溢出。";

	case EXCEPTION_FLT_UNDERFLOW:
		return "浮点操作的指数没有超过相应类型的最大值。";

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		return "线程试图执行无效指令。";

	case EXCEPTION_INT_OVERFLOW:
		return "整数操作的结果占用了结果的最大符号位。";

	case EXCEPTION_STACK_OVERFLOW:
		return "栈溢出。";

	default:
		return "无描述";
	}
	return "";
}

//输出崩溃日志
void CWin32Exception::OutputWin32Exception(const char * str, ...)
{
	try
	{
		FILE *fp = NULL;
		if (fopen_s(&fp, "C:\\HM-Dump-v1.0\\崩溃记录.log", "a") != 0)
		{
			return;
		}

		if (NULL == fp)
		{
			return;
		}

		char szPath[MAX_PATH] = "";
		GetCurrentDirectory(MAX_PATH, szPath);

		SYSTEMTIME time;
		GetLocalTime(&time);

		char cTime[64] = "";
		sprintf_s(cTime, sizeof(cTime), "[%04d/%02d/%02d-%02d:%02d:%02d] ", time.wYear, time.wMinute, time.wDay, time.wHour, time.wMinute, time.wSecond);
		fprintf_s(fp, cTime);

		va_list arg;

		va_start(arg, str);
		vfprintf_s(fp, str, arg);
		fprintf_s(fp, "【path=%s】\n", szPath);

		fclose(fp);
	}
	catch (...)
	{
		exit(0);
	}
}