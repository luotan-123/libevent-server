
// 重定义
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long       ulong;
typedef unsigned __int64    uint64;
typedef unsigned __int64    ulonglong;
typedef __int64				longlong;
typedef __int64				int64;
typedef TCHAR				tchar;
typedef WCHAR				wchar;
typedef FILE				file;
typedef WORD				word;
typedef DWORD				dword;
typedef HWND				hwnd;
typedef HANDLE				handle;
typedef HINSTANCE			hinstance;
typedef HRSRC				hrsrc;
typedef COLORREF			colorref;
typedef GUID				guid;
typedef POINT				point;
#define Vector				vector
#define Map					map
#define Static				static
#define int_max				2147483647       
#define int_min				(-2147483647 - 1)   
#define uint_max			0xffffffff     
#define uint_min			0  
#define llong_max			0x7fffffffffffffff
#define llong_min			0x8000000000000000
#define ullong_max			0xffffffffffffffff
#define ullong_min			0

// 标准定义
#define _in
#define _out

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

// 错误定义
#ifdef _DEBUG
#define _Assert(A)				assert(A)
#else
#define _Assert(A)
#endif

// 删除数组
#define DeleteArray( Array )						\
{													\
	while ( Array.GetCount() )						\
	{												\
		SafeDelete(Array[0]);						\
		Array.RemoveAt(0);							\
	};												\
}													
