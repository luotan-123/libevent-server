#pragma once

#include "GameDesk.h"
#include <assert.h>


// 重定义
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long       ulong;
typedef unsigned long long  uint64;
typedef unsigned long long  ulonglong;
typedef long long			longlong;
typedef long long			int64;
typedef unsigned short		word;
typedef unsigned int		dword;
typedef unsigned int		hwnd;
typedef unsigned int		handle;
typedef unsigned int		hinstance;
typedef unsigned char		byte;
typedef unsigned short      WORD;
typedef longlong			LONGLONG;
typedef char				tchar;
typedef char				TCHAR;
typedef unsigned int        DWORD;
typedef void				VOID;
typedef float				FLOAT;
#define Vector				vector
#define Static				static
#define int_max				2147483647       
#define int_min				(-2147483647 - 1)   
#define uint_max			0xffffffff     
#define uint_min			0  
#define llong_max			0x7fffffffffffffff
#define llong_min			0x8000000000000000
#define ullong_max			0xffffffffffffffff
#define ullong_min			0
#define CopyMemory			memcpy
#define ZeroMemory			bzero
#define file				FILE
#define ASSERT				assert

// 标准定义
#define _in
#define _out
#define MAX_PATH			256

// 安全删除
#ifndef SafeRelease
	#define SafeRelease(pObject) { if (pObject!=NULL) { pObject->Release(); pObject=NULL; } }
#endif

#ifndef SafeCloseHandle
	#define SafeCloseHandle(hHandle) { if (hHandle!=NULL) { CloseHandle(hHandle); hHandle=NULL; } }
#endif

#ifndef SafeDelete
	#define SafeDelete(pData) { if ( pData != NULL )  { delete pData;  pData=NULL; } } 
#endif

#ifndef SafeDeleteArray
	#define SafeDeleteArray(pData) { if ( pData != NULL )  { delete[] pData;  pData=NULL; } } 
#endif

#ifndef CountArray
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))
#endif												

typedef struct tagPOINT
{
	int  x;
	int  y;
} POINT;

struct CPoint : public tagPOINT
{

};