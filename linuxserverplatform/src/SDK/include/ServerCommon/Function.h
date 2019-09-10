#pragma once

//计算数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//安全删除指针
#define  SafeDelete(pData)	{ try { delete pData; } catch (...) { ERROR_LOG("CATCH:%s with %s\n",__FILE__,__FUNCTION__);} pData = NULL; } 

//安全删除指针
#define  SafeDeleteArray(pData)	{ try { delete [] pData; } catch (...) { ERROR_LOG("CATCH:%s with %s\n",__FILE__,__FUNCTION__);} pData = NULL; } 

//安全删除指针
#define SAFE_DELETE(ptr) { if(ptr){	try{ delete ptr; }catch(...){ ERROR_LOG("CATCH: *** SAFE_DELETE(%s) crash! *** %s %d\n",#ptr,__FILE__, __LINE__); } ptr = 0; } }

//判断大小函数
#define min(x,y) ((x)>(y)?(y):(x))
#define max(x,y) ((x)>(y)?(x):(y))