#ifndef _CXOBJ_H_
#define _CXOBJ_H_

#include <stdlib.h>

#define byte unsigned char 
#define SHM_CHECK_BYTE  'X'

class CXObj;

#define DECLARE_DYN  public:    static CXObj* CreateObject(void* pMem);
//#define IMPLEMENT_DYN(class_name)  CXObj* class_name::CreateObject( void *pMem ) {    CXObj::pCurrentObj = (byte *)pMem;    return (CXObj *)(new class_name); }
#define IMPLEMENT_DYN(class_name) \
CXObj* class_name::CreateObject( void *pMem ) \
{\
CXObj::pCurrentObj = (byte *)pMem;    \
return (CXObj *)(new class_name); \
}


class CXObj
{
public:
	/**
	*	@brief 分配对象空间时调用，进行初始化工作。
	*/
	virtual void Init() = 0;

	/**
	*	@brief  不清除共享内存启动时调用，负责恢复工作，如指针重置等
	*/
	virtual void Resume() = 0;

	/**
	*	@brief  对象空间被回收时调用，进行清楚工作。
	*/
	virtual void Reclaim() = 0;

	/**
	*	@brief  检测内存是否被破坏
	*/
	inline bool CheckMem() { return (SHM_CHECK_BYTE == m_cCheckByte); }

	CXObj() { m_cCheckByte = SHM_CHECK_BYTE; };
	virtual ~CXObj() {};

protected:
	static void* operator new(size_t nSize)
	{
		if (NULL == pCurrentObj)
		{
			return (void*)NULL;
		}

		return (void*)pCurrentObj;
	}

	static void  operator delete(void* pMem)
	{
		return;
	}

	static byte* pCurrentObj;
	char  m_cCheckByte;

};

#endif

