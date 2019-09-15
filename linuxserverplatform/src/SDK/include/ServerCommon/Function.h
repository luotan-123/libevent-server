#pragma once

#include "KernelDefine.h"

//计算数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//安全删除指针
#define  SafeDelete(pData)	{ try { delete pData; } catch (...) { ERROR_LOG("CATCH:%s with %s\n",__FILE__,__FUNCTION__);} pData = NULL; } 

//安全删除指针
#define  SafeDeleteArray(pData)	{ try { delete [] pData; } catch (...) { ERROR_LOG("CATCH:%s with %s\n",__FILE__,__FUNCTION__);} pData = NULL; } 

//安全删除指针
#define SAFE_DELETE(ptr) { if(ptr){	try{ delete ptr; }catch(...){ ERROR_LOG("CATCH: *** SAFE_DELETE(%s) crash! *** %s %d\n",#ptr,__FILE__, __LINE__); } ptr = 0; } }

//判断大小函数
#define Min_(x,y) ((x)>(y)?(y):(x))
#define Max_(x,y) ((x)>(y)?(x):(y))

//获取系统时间
extern void GetLocalTime(SYSTEMTIME* sysTime);

//获取线程pid,线程内部id  pthread库函数
extern pthread_t GetCurrentThreadId();

//获取线程pid,内核线程id
extern pthread_t GetCurrentSysThreadId();