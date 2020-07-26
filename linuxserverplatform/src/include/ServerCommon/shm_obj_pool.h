/**
*	@file
*	@brief	共享内存对象池：预分配对象空间，并提供访问接口。
*
*	@note	tab=4.
*/

#ifndef	_XG_SHM_OBJ_POOL_H_
#define	_XG_SHM_OBJ_POOL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shmem.h"
#include "log.h"


//class CZoneConfMgr;
//CZoneConfMgr *GetZoneConfMgr ();



//#include "zone_def.h"

//extern LPTLOGCATEGORYINST GetLogCat();


struct tagObjPoolHead
{
	int	iType;	//<	类型。正整数。
	int	iUint;	//<	单个T的空间大小。
	int	iNum;	//<	个体的数量。
	int	iKey;	//<	shm	key。0 为不起作用。
	int	iSize;	//<	shm	size。0	为不起作用。
	int iOffSet; //< 偏移量。
	int	iFreeHead;	//<	空闲头。head=tail表示空。
	int	iFreeTail;	//<	空闲尾。
	int	iUseNum;	//<	已经使用的大小。
	int* piFreeArray;	//<	空闲数组。
	char* pcUsedArray;   //< 标示是否使用。0 为未使用，1为使用中。和pszData对应的存储对象一一对应。
	char* pszData;	//<	存储空间。
};

typedef	tagObjPoolHead OBJPOOLHEAD;
typedef	tagObjPoolHead* LPOBJPOOLHEAD;

template <class	T>
class	CShmObjPool
{
public:

	CShmObjPool() { ; }
	virtual	~CShmObjPool() { ; }


	/**
	 *	@brief 创建共享内存并分配空间。
	 *  @return >0 为创建改类型对象池的总空间大小.<0 when fail，注意0号位对象不能被存储，实际可使用空间aiNum-1
	 */
	int CreateObjPool(const int aiKey, const int aiTotalSize, const int aiUint, const int aiNum, int aiType, const char* pFile, int iLine);

	ENMShmStartMode GetStartFlag();

	//added by shasharui 20090512 start
	/**
	   *	@brief	通过下标，获取该对象的指针。
	   *	@param	aiIndex	-	需要的对象的下标。
	   *	@param	aiType	-	对象的类型。用于校验。
	   *	@return	 不管用没用，强制取出来，不保证已经被用了，谨慎使用
	   */
	T* ForceGetObj(const	int	aiIndex, const int aiType);
	//added by shasharui 20090512 end

	/**
	   *	@brief	通过下标，获取该对象的指针。
	   *	@param	aiIndex	-	需要的对象的下标。
	   *	@param	aiType	-	对象的类型。用于校验。
	   *	@return		NULL 为查找不到。not NULL为该对象指针。
	   */
	inline T* GetObj(const	int	aiIndex, const int aiType)
	{
		/*
					m_iErrCode = 0;

					if (m_pHead->iType !=	aiType)
					{
						m_iErrCode = -1;
						return NULL;
					}
					if (aiIndex	>= m_pHead->iNum || aiIndex < 0)
					{
						m_iErrCode = -2;
						return NULL;
					}
					if (0 == m_pHead->pcUsedArray[aiIndex])
					{
						m_iErrCode = -3;
						return NULL;
					}else
						return &(m_pObjHead[aiIndex]);
		*/

		if (m_pHead->iType == aiType && aiIndex >= 0 && aiIndex < m_pHead->iNum
			&& 0 != m_pHead->pcUsedArray[aiIndex])
		{
			m_iErrCode = 0;
			return &(m_pObjHead[aiIndex]);
		}
		else
		{
			m_iErrCode = -1;
			return NULL;
		}

	}

	/**
	   *	@brief	获取空闲的对象空间。
	   *	@param	aiType - 对象的类型。
	   *	@param	aiIndex	-	返回该对象的下标。
	   *	@return	NULL 为没有空间分配。not NULL为分配的对象指针。
	   */
	T* GetFreeObj(const	int	aiType, int* aiIndex);

	/**
	   *	@brief 释放对象空间。
	   *	@param	aitype - 对象的类型。
	   *	@param	aiIndex	-	需要释放的对象下标。
	   *	@return	0	when ok。
	   */
	int	FreeObj(const int aiType, const int aiIndex);

	/**
	*	@param aiFlag - 标示是否打印空闲数组和使用标示数组。0为不打印，1为打印。
	*/
	void PrintHead(int aiFlag = 0);

	LPOBJPOOLHEAD GetHead()
	{
		return m_pHead;
	}

	int GetErrCode()
	{
		return m_iErrCode;
	}

	/**
	 * 	@brief	处理所有的节点。
	 * 	@note   ProcessObj返回2120会终止遍历
	 *
	 */
	int ProcessAllObj(int (*ProcessObj)(T*));

	/**
	 * 	@brief	处理所有的节点,可以通过传入的参数执行不同的操作
	 *
	 */
	int ProcessAllObj(int (*ProcessObj)(T*, void*, int, int), void* pArg, int iLen, int iType);

	/**
	 * 	@brief	处理所有的节点,可传入参数和开始遍历的下标
	 * 	@note   ProcessObj返回200会终止遍历
	 *
	 */
	int ProcessAllObj(int (*ProcessObj)(T*, void*, int, int), void* pArg, int iLen, int iType, int iStartIdx);

	/**
	 * 	@brief	返回已使用的obj数量
	 */
	int GetUseNum()
	{
		return m_pHead->iUseNum;
	}

	/**
	 * 	@brief	返回空闲的obj数量
	 */
	int GetFreeNum()
	{
		return (m_pHead->iNum - m_pHead->iUseNum);
	}

	/**
	 * 	@brief	返回单个obj大小
	 */
	int GetUnitSize() { return m_pHead->iUint; }

	/**
	 * 	@brief	返回使用的共享内存大小
	 */
	int GetShmSize()
	{
		return (sizeof(OBJPOOLHEAD) + m_pHead->iNum * (sizeof(int) + sizeof(char) + m_pHead->iUint));
	}


protected:
	/**
	*
	*/
	int InitFreeArray();

	int InitUsedArray();

	int InitAllObj(ENMShmStartMode enmMode);

	bool CheckShm();

protected:


	///	head + 空闲数组队列 + 是否分配的标示 + 对象空间。
	LPOBJPOOLHEAD	 m_pHead;
	T* m_pObjHead;

	int m_iErrCode;
};

template <class T>
int CShmObjPool<T>::InitFreeArray()
{
	///<第一次使用该内存时，需要先把空闲数组的值初始化为Obj空间的下标。

	for (int i = 0; i < m_pHead->iNum; i++)
	{
		m_pHead->piFreeArray[i] = i;
	}
	return 0;
}

template <class T>
int CShmObjPool<T>::InitAllObj(ENMShmStartMode enmMode)
{
	T* lpoObj = NULL;
	for (int i = 0; i < m_pHead->iNum; i++)
	{
		lpoObj = &(m_pObjHead[i]);
		lpoObj = (T*)T::CreateObject((void*)lpoObj);
		if (ESSM_Init == enmMode)
		{
			lpoObj->Init();
		}
		else
		{
			lpoObj->Resume();
		}
	}

	return 0;
}

template <class T>
int CShmObjPool<T>::InitUsedArray()
{
	//<  第一次使用该内存时，需要把是否使用数组初始化为 未使用。建立共享内存时直接memset 为0了。
	return 0;
}

template <class	T>
int	CShmObjPool<T>::CreateObjPool(const int aiKey, const	int	aiTotalSize, const	int	aiUint, const	int	aiNum, int aiType, const char* pFile, int iLine)
{
	//<	判断是否已经创建共享内存。
	CShmem loShmem;

	//< 数据结构： 包头 + 空闲数组 + 标示是否使用数组+实际对象空间。	
	uint64_t  ullNeedSize = sizeof(OBJPOOLHEAD) + sizeof(int) * aiNum + sizeof(char) * aiNum + aiUint * aiNum;
	if (ullNeedSize > 0xFFFFFFFF)
	{
		ERROR_LOG("ullNeedSize = %llu too large!\n", ullNeedSize);
		return 0;
	}

	int iNeedSize = ullNeedSize & 0xFFFFFFFF;

	//GetZoneConfMgr()->RecordShmInfo(aiKey, aiTotalSize, iNeedSize);

	char* lpszShm = (char*)loShmem.CreateShmem(aiKey, aiTotalSize, iNeedSize, pFile, iLine);
	if (lpszShm == NULL)
	{
		return -1;
	}

	///	数据结构： 包头 + 空闲数组 + 标示是否使用数组+实际对象空间。	
	//<	没有校验数据空间大小是否超过共享内存分配的大小。后续增加。	
	ENMShmStartMode enmMode = loShmem.GetStartFlag(aiKey, aiTotalSize);
	if (ESSM_Resume == enmMode)
	{
		m_pHead = (LPOBJPOOLHEAD)lpszShm;

		//检查代码是否改变了对象池的对象大小、个数和类型
		if (aiUint != m_pHead->iUint || aiNum != m_pHead->iNum || aiType != m_pHead->iType)
		{
			ERROR_LOG("error in CreateObjPool, shm needed changed! aiKey(%d), request(%d, %d, %d), shm(%d, %d, %d)\n",
				aiKey, aiUint, aiNum, aiType, m_pHead->iUint, m_pHead->iNum, m_pHead->iType);
			return -1;
		}

		int liOffSet = sizeof(OBJPOOLHEAD);
		m_pHead->piFreeArray = (int*)(lpszShm + liOffSet);

		liOffSet += sizeof(int) * m_pHead->iNum;
		m_pHead->pcUsedArray = (char*)(lpszShm + liOffSet);

		liOffSet += sizeof(char) * m_pHead->iNum;
		m_pHead->pszData = lpszShm + liOffSet;
		m_pObjHead = (T*)m_pHead->pszData;

		if (!CheckShm())
		{
			ERROR_LOG("check shm error! CShmObjPool key[%d], size[%d] , mod[init], address[%p], uint[%d: %d+%d*%d]\n",
				aiKey, aiTotalSize, lpszShm, aiType, iNeedSize, aiUint, aiNum);
			return -1;
		}
		//		RecordLog(GetLogCat(), TLL_TRACE, "shm has existed!key[%d], size[%d]\n", m_pHead->iKey, m_pHead->iSize);
	}
	else
	{
		memset(lpszShm, 0, iNeedSize);

		m_pHead = (LPOBJPOOLHEAD)lpszShm;
		m_pHead->iUint = aiUint;
		m_pHead->iType = aiType;
		m_pHead->iNum = aiNum;
		m_pHead->iKey = aiKey;
		m_pHead->iSize = aiTotalSize;

		int liOffSet = sizeof(OBJPOOLHEAD);
		m_pHead->piFreeArray = (int*)(lpszShm + liOffSet);
		InitFreeArray();

		liOffSet += sizeof(int) * m_pHead->iNum;
		m_pHead->pcUsedArray = (char*)(lpszShm + liOffSet);

		liOffSet += sizeof(char) * m_pHead->iNum;
		m_pHead->pszData = lpszShm + liOffSet;
		m_pObjHead = (T*)m_pHead->pszData;

	}

	InitAllObj(enmMode);

	if (ESSM_Init == enmMode)
	{
		INFO_LOG("%s key[%d], size[%d] , mod[init], address[%p], uint[%d: %d+%d*%d]\n",
			__PRETTY_FUNCTION__, aiKey, aiTotalSize, lpszShm, aiType, iNeedSize, aiUint, aiNum);
	}
	else
	{
		INFO_LOG("%s key[%d], size[%d] , mod[resume], address[%p], uint[%d: %d+%d*%d]\n",
			__PRETTY_FUNCTION__, aiKey, aiTotalSize, lpszShm, aiType, iNeedSize, aiUint, aiNum);
	}

	//int liTotalSize = 0;
	//< 数据结构： 包头 + 空闲数组 + 标示是否使用数组+实际对象空间。	
	//liTotalSize = sizeof(OBJPOOLHEAD) + sizeof(int)*m_pHead->iNum + sizeof(char)*m_pHead->iNum + m_pHead->iUint * m_pHead->iNum;
	return iNeedSize;
}

template <class	T>
ENMShmStartMode	CShmObjPool<T>::GetStartFlag()
{
	CShmem loShmem;
	return loShmem.GetStartFlag(m_pHead->iKey, m_pHead->iSize);
}


template <class T>
void CShmObjPool<T>::PrintHead(int aiFlag)
{
	printf("ShmHead key:%d size:%d num:%d type:%d uint:%d usenum:%d fhead:%d ftail:%d\n", m_pHead->iKey, m_pHead->iSize, m_pHead->iNum, \
		m_pHead->iType, m_pHead->iUint, m_pHead->iUseNum, m_pHead->iFreeHead, m_pHead->iFreeTail);

	if (aiFlag != 0)
	{
		printf("[i,FreeArray,UsedArray] num=%d\n", m_pHead->iNum);
		for (int i = 0; i < m_pHead->iNum; i++)
		{
			printf("[%d,%d,%d]", i, m_pHead->piFreeArray[i], m_pHead->pcUsedArray[i]);
		}
	}
	printf("\nEnd\n");
}

//适用于反复利用同一个节点 
template <class	T>
T* CShmObjPool<T>::ForceGetObj(const	int	aiIndex, const int aiType)
{
	m_iErrCode = 0;

	if (m_pHead->iType != aiType)
	{
		m_iErrCode = -1;
		return NULL;
	}
	if (aiIndex >= m_pHead->iNum || aiIndex < 0)
	{
		m_iErrCode = -2;
		return NULL;
	}

	return &(m_pObjHead[aiIndex]);
}


template <class	T>
T* CShmObjPool<T>::GetFreeObj(const int aiType, int* aiIndex)
{
	if (m_pHead->iType != aiType)
	{
		*aiIndex = -1;
		return NULL;
	}

	if (m_pHead->iUseNum == m_pHead->iNum)
	{
		*aiIndex = -2;
		return NULL;
	}
	//<	获取空闲对象空间。
	int	liFreeHead = (m_pHead->iFreeHead + 1) % m_pHead->iNum;
	if (liFreeHead == m_pHead->iFreeTail)
	{
		//<	full.	no free	node.
		*aiIndex = -3;
		return NULL;
	}

	*aiIndex = m_pHead->piFreeArray[liFreeHead];
	m_pHead->iFreeHead = liFreeHead;

	m_pHead->pcUsedArray[*aiIndex] = 1;
	m_pHead->iUseNum++;

	T* lpoObj = &(m_pObjHead[*aiIndex]);
	lpoObj->Init();

	return lpoObj;
}

template <class	T>
int	CShmObjPool<T>::FreeObj(const int aiType, const int aiIndex)
{
	if (m_pHead->iType != aiType)
		return -1;

	if (aiIndex >= m_pHead->iNum || aiIndex < 0)
		return -2;

	///	回收对象空间。
	if (m_pHead->iFreeHead == m_pHead->iFreeTail)
	{
		return -3;
	}

	if (m_pHead->pcUsedArray[aiIndex] != 1)
		return -4;

	/// 注意顺序，需要先调用对象的函数，然后放入空闲队列
	T* lpo = &(m_pObjHead[aiIndex]);
	lpo->Reclaim();

	m_pHead->piFreeArray[m_pHead->iFreeTail] = aiIndex;
	m_pHead->iFreeTail = (m_pHead->iFreeTail + 1) % m_pHead->iNum;

	m_pHead->pcUsedArray[aiIndex] = 0;

	m_pHead->iUseNum--;

	return 0;
}

template <class T>
int CShmObjPool<T>::ProcessAllObj(int (*ProcessObj)(T*))
{
	int liRet = 0;
	/// 处理已经分配出去的节点。	
	for (int i = 0; i < m_pHead->iNum; i++)
	{
		if (0 == m_pHead->pcUsedArray[i])
			continue;

		/// process.已经分配出去，处理。
		liRet = ProcessObj(&(m_pObjHead[i]));
		if (100 == liRet)
		{
			/// 回收对象。
			liRet = FreeObj(m_pHead->iType, i);
		}
		// 终止遍历
		else if (2120 == liRet)
		{
			break;
		}
	}

	return liRet;
}


//使遍历每个节点的函数可以接受其他参数
//参数意义:
//ProcessObj:对应的处理函数
//pArg:需要传入的其他参数，可以是结构体类型指针
//iLen: 传入的void*指针对应的内存长度
//iType: 操作类型
template <class T>
int CShmObjPool<T>::ProcessAllObj(int (*ProcessObj)(T*, void*, int, int), void* pArg, int iLen, int iType)
{
	int liRet = 0;
	/// 处理已经分配出去的节点。	
	for (int i = 0; i < m_pHead->iNum; i++)
	{
		if (0 == m_pHead->pcUsedArray[i])
			continue;

		/// process.已经分配出去，处理。
		liRet = ProcessObj(&(m_pObjHead[i]), pArg, iLen, iType);
		if (100 == liRet)
		{
			/// 回收对象。
			liRet = FreeObj(m_pHead->iType, i);
		}
	}

	return liRet;
}

//使遍历每个节点的函数可以接受其他参数
//参数意义:
//ProcessObj:对应的处理函数
//pArg:需要传入的其他参数，可以是结构体类型指针
//iLen: 传入的void*指针对应的内存长度
//iType: 操作类型
//iStartIdx: 从StartIdx开始遍历
template <class T>
int CShmObjPool<T>::ProcessAllObj(int (*ProcessObj)(T*, void*, int, int), void* pArg, int iLen, int iType, int iStartIdx)
{
	int liRet = 0;
	/// 处理已经分配出去的节点。	
	for (int i = 0; i < m_pHead->iNum; i++)
	{
		int idx = (i + iStartIdx) % m_pHead->iNum;

		if (0 == m_pHead->pcUsedArray[idx])
			continue;

		/// process.已经分配出去，处理。
		liRet = ProcessObj(&(m_pObjHead[idx]), pArg, iLen, iType);
		if (100 == liRet)
		{
			/// 回收对象。
			liRet = FreeObj(m_pHead->iType, idx);
		}

		if (200 == liRet)
		{
			break;
		}
	}

	return liRet;
}

template <class T>
bool CShmObjPool<T>::CheckShm()
{
	T* lpoObj = NULL;

	for (int i = 0; i < m_pHead->iNum; i++)
	{
		lpoObj = (T*)&(m_pObjHead[i]);
		if (!lpoObj->CheckMem())
		{
			return false;
		}
	}

	return true;
}


#endif

