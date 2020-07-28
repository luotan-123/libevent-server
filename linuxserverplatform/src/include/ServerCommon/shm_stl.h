#ifndef _SHM_STL_H_
#define _SHM_STL_H_

#include "CXObj.h"
#include "shm_obj_pool.h"
#include <vector>
#include <map>


template <class CKey, class CData>
class ShmNode : public CXObj
{
public:
	ShmNode();
	virtual ~ShmNode();
	/**
	*	@brief 分配对象空间时调用，进行初始化工作。
	*/
	virtual void Init() {}

	/**
	*	@brief  不清除共享内存启动时调用，负责恢复工作，如指针重置等
	*/
	virtual void Resume() {}

	/**
	*	@brief  对象空间被回收时调用，进行清楚工作。
	*/
	virtual void Reclaim() {}

	static CXObj* CreateObject(void* pMem)
	{
		CXObj::pCurrentObj = (byte*)pMem;
		return (CXObj*)(new ShmNode<CKey, CData>);
	}

public:
	CKey m_oKey;
	CData m_oData;
	int m_iShmIndex;
};

/// -------------- ShmNode ------------------
template <class CKey, class CData>
ShmNode<CKey, CData>::ShmNode()
{
}

template <class CKey, class CData>
ShmNode<CKey, CData>::~ShmNode()
{}


template <class CData>
class ShmVector
{
public:
	typedef ShmNode<uint64_t, CData> ShmVectorNode;
	typedef std::pair<uint64_t, ShmVectorNode*> ShmVectorKey;

public:
	/**
	*  @brief    初始化函数
	*  @note
	*/
	int Init(const int aiKey, int aiNum, int aiType);

	/**
	* 	@brief	返回已使用的obj数量
	*/
	int GetUseNum()
	{
		return m_dataPool.GetUseNum();
	}

	/**
	* 	@brief	返回空闲的obj数量
	*/
	int GetFreeNum()
	{
		return m_dataPool.GetFreeNum();
	}

	/**
	* 	@brief	返回头部的对象指针
	*/
	CData* Front();

	/**
	* 	@brief	删除头部的对象
	*/
	void PopFront();

	/**
	* 	@brief	返回尾部的对象指针
	*/
	CData* Back();

	/**
	* 	@brief	从尾部增加一个对象
	*/
	int PushBack(CData& Node);

	/**
	* 	@brief	从指定位置插入一个对象
	*/
	int Insert(int iPos, CData& Node);

	/**
	* 	@brief	删除尾部的对象
	*/
	void PopBack();

	/**
	* 	@brief	删除指定位置下标的对象
	*/
	int Delete(int iPos);
	int Delete(int iBeginPos, int EndPos);

	/**
	* 	@brief	删除所有的数组对象
	*/
	void Clear();

	/**
	* 	@brief	返回已经使用的数组大小
	*/
	int Size();

	/**
	* 	@brief	获取指定位置下标的对象
	*/
	CData* GetObj(int iPos);

	/**
	* 	@brief	获取所有对象的指针数组
	*/
	void GetAllObj(std::vector<CData*>& vecDatas);

	/**
	* 	@brief	重载[]操作符操作
	*/
	CData& operator[](int i)
	{
		// 如果下标超出范围就直接挂吧
		if (i < 0 || i >(int)m_keyVec.size())
		{
			CData* pData = (CData*)NULL;
			return *pData;
		}

		return m_keyVec[i].second->m_oData;
	}

	/**
	* 	@brief 从共享内存重建数组索引和顺序时的排序比较
	*/
	static bool IDCompare(const ShmVectorKey& a, const ShmVectorKey& b);

protected:
	/**
	*  @brief    从共享内存中恢复函数
	*  @note
	*/
	virtual void Resume();

	static int ResumeData(ShmVectorNode* pData, void* pArgList, int iLen, int iType);

protected:
	std::vector<ShmVectorKey> m_keyVec;
	CShmObjPool<ShmVectorNode> m_dataPool;
	uint64_t m_ullGID;
	int m_aiType;
};

template <class CKey, class CData>
class ShmMap
{
public:
	typedef ShmNode<CKey, CData> ShmMapNode;
	typedef std::map<CKey, ShmMapNode*> ShmMapKeys;

public:
	/**
	*  @brief    初始化函数
	*  @note
	*/
	int Init(const int aiKey, int aiNum, int aiType);

	/**
	* 	@brief	返回已使用的obj数量
	*/
	int GetUseNum()
	{
		return m_dataPool.GetUseNum();
	}

	/**
	* 	@brief	返回空闲的obj数量
	*/
	int GetFreeNum()
	{
		return m_dataPool.GetFreeNum();
	}

	/**
	* 	@brief	根据指定的key查找元素对象
	*/
	CData* Get(const CKey oKey);

	/**
	* 	@brief	插入一个元素对象
	*/
	int Insert(const CKey& oKey, const CData& Node);

	/**
	* 	@brief	删除指定key的元素对象
	*/
	int Delete(const CKey& oKey);

	/**
	* 	@brief	删除所有的数组对象
	*/
	void Clear();

	/**
	* 	@brief	返回已经使用的数组大小
	*/
	int Size();

	/**
	* 	@brief	获取所有对象的map列表
	*/
	void GetAllObj(std::vector<std::pair<CKey, CData*> >& allObjs);

	/**
	* 	@brief	重载[]操作符操作
	*/
	CData& operator[](const CKey& oKey)
	{
		// 模仿map的没有就插入一个空元素
		CData* pData = Get(oKey);
		if (pData == NULL)
		{
			Insert(oKey, CData());
			pData = Get(oKey);
		}

		return *pData;
	}

protected:
	/**
	*  @brief    从共享内存中恢复函数
	*  @note
	*/
	virtual void Resume();

	static int ResumeData(ShmNode<CKey, CData>* pData, void* pArgList, int iLen, int iType);

protected:
	ShmMapKeys m_keyMap;
	CShmObjPool<ShmMapNode> m_dataPool;
	int m_aiType;
};


template <class CData>
class ShmData
{
public:
	/**
	*  @brief    初始化函数
	*  @note
	*/
	int Init(const int aiKey, int aiType)
	{
		m_aiType = aiType;
		int aiNum = 1;
		int aiUint = sizeof(ShmNode<int, CData>);
		uint64_t ullNeedSize = sizeof(OBJPOOLHEAD) + sizeof(int) * aiNum + sizeof(char) * aiNum + aiUint * aiNum + 32;
		int iRet = m_dataPool.CreateObjPool(aiKey, ullNeedSize, aiUint, aiNum, aiType, __FILE__, __LINE__);
		if (iRet < 0)
		{
			printf("%s:Create Shared-Mem Failed,iRet[%d] %m\n", __PRETTY_FUNCTION__, iRet);
			return iRet;
		}
		if (ESSM_Resume == m_dataPool.GetStartFlag())
		{
			//Resume();
		}

		return m_dataPool.GetStartFlag();
	}

	/**
	* 	@brief	获取数据区结构体指针
	*/
	CData* GetDataPtr()
	{
		ShmNode<int, CData>* pNode = m_dataPool.ForceGetObj(0, m_aiType);
		return &pNode->m_oData;
	}

	/**
	* 	@brief	重载->操作符操作
	*/
	CData* operator->()
	{
		ShmNode<int, CData>* pNode = m_dataPool.ForceGetObj(0, m_aiType);
		return &pNode->m_oData;
	}

	/**
	* 	@brief	数据区清0重置
	*/
	void Reset()
	{
		ShmNode<int, CData>* pNode = m_dataPool.ForceGetObj(0, m_aiType);
		memset(&pNode->m_oData, 0, sizeof(CData));
	}

protected:
	CShmObjPool<ShmNode<int, CData> > m_dataPool;
	int m_aiType;
};

/////////////////////////////////ShmVector/////////////////////////////////////////
/**
*  @brief    初始化函数
*  @note
*/
template <class CData>
int ShmVector<CData>::Init(const int aiKey, int aiNum, int aiType)
{
	m_ullGID = 0;
	m_aiType = aiType;
	// 多加一个节点当尾结点
	aiNum += 1;
	int aiUint = sizeof(ShmVectorNode);
	uint64_t ullNeedSize = sizeof(OBJPOOLHEAD) + sizeof(int) * aiNum + sizeof(char) * aiNum + aiUint * aiNum + 32;
	int iRet = m_dataPool.CreateObjPool(aiKey, ullNeedSize, aiUint, aiNum, aiType, __FILE__, __LINE__);
	if (iRet < 0)
	{
		printf("%s:Create Shared-Mem Failed,iRet[%d] %m\n", __PRETTY_FUNCTION__, iRet);
		return iRet;
	}
	if (ESSM_Resume == m_dataPool.GetStartFlag())
	{
		Resume();
	}

	return m_dataPool.GetStartFlag();
}

template <class CData>
int ShmVector<CData>::ResumeData(ShmVectorNode* pData, void* pArgList, int iLen, int iType)
{
	if (NULL == pData || NULL == pArgList)
		return -1;

	ShmVector<CData>* pShmVector = (ShmVector<CData>*)pArgList;
	pShmVector->m_keyVec.push_back(std::make_pair(pData->m_oKey, pData));
	if (pData->m_oKey > pShmVector->m_ullGID)
		pShmVector->m_ullGID = pData->m_oKey;

	return 0;
}

template <class CData>
bool ShmVector<CData>::IDCompare(const ShmVectorKey& a, const ShmVectorKey& b)
{
	return a.first < b.first;
}

/**
*  @brief    从共享内存中恢复函数
*  @note
*/
template <class CData>
void ShmVector<CData>::Resume()
{
	m_keyVec.clear();
	m_dataPool.ProcessAllObj(ResumeData, this, 0, 0);
	std::sort(m_keyVec.begin(), m_keyVec.end(), IDCompare);
}

/**
* 	@brief	返回头部的对象指针
*/
template <class CData>
CData* ShmVector<CData>::Front()
{
	if (m_keyVec.size() == 0)
		return NULL;

	ShmVectorKey& key = m_keyVec.front();
	return &key.second->m_oData;
}

/**
* 	@brief	删除头部的对象
*/
template <class CData>
void ShmVector<CData>::PopFront()
{
	if (m_keyVec.size() == 0)
		return;

	m_dataPool.FreeObj(m_aiType, m_keyVec.front().second->m_iShmIndex);
	m_keyVec.erase(m_keyVec.begin());
}

/**
* 	@brief	返回尾部的对象指针
*/
template <class CData>
CData* ShmVector<CData>::Back()
{
	if (m_keyVec.size() == 0)
		return NULL;

	ShmVectorKey& key = m_keyVec.back();
	return &key.second->m_oData;
}

/**
* 	@brief	从尾部增加一个对象
*/
template <class CData>
int ShmVector<CData>::PushBack(CData& Node)
{
	return Insert(m_keyVec.size(), Node);
}

/**
* 	@brief	从指定位置插入一个对象
*/
template <class CData>
int ShmVector<CData>::Insert(int iPos, CData& Node)
{
	if (iPos < 0 || iPos >(int)m_keyVec.size())
		return -1;

	int aiIndex = 0;
	ShmVectorNode* pNode = m_dataPool.GetFreeObj(m_aiType, &aiIndex);
	if (pNode == NULL)
		return -1;

	pNode->m_oData = Node;
	pNode->m_iShmIndex = aiIndex;
	m_ullGID++;
	pNode->m_oKey = m_ullGID;
	m_keyVec.insert(m_keyVec.begin() + iPos, std::make_pair(pNode->m_oKey, pNode));

	return 0;
}

/**
* 	@brief	删除尾部的对象
*/
template <class CData>
void ShmVector<CData>::PopBack()
{
	if (m_keyVec.size() == 0)
		return;

	m_dataPool.FreeObj(m_aiType, m_keyVec.back().second->m_iShmIndex);
	m_keyVec.pop_back();
}

/**
* 	@brief	删除指定位置下标的对象
*/
template <class CData>
int ShmVector<CData>::Delete(int iPos)
{
	if (iPos < 0 || m_keyVec.size() <= iPos)
		return -1;

	m_dataPool.FreeObj(m_aiType, m_keyVec[iPos].second->m_iShmIndex);
	m_keyVec.erase(m_keyVec.begin() + iPos);

	return 0;
}

template <class CData>
int ShmVector<CData>::Delete(int iBeginPos, int iEndPos)
{
	if (iBeginPos < 0 || iEndPos < 0 || iEndPos < iBeginPos || m_keyVec.size() < iEndPos)
		return -1;

	for (int i = iBeginPos; i < iEndPos; i++)
	{
		m_dataPool.FreeObj(m_aiType, m_keyVec[i].second->m_iShmIndex);
	}

	m_keyVec.erase(m_keyVec.begin() + iBeginPos, m_keyVec.begin() + iEndPos);

	return 0;
}


/**
* 	@brief	删除所有的数组对象
*/
template <class CData>
void ShmVector<CData>::Clear()
{
	for (size_t i = 0; i < m_keyVec.size(); i++)
	{
		m_dataPool.FreeObj(m_aiType, m_keyVec[i].second->m_iShmIndex);
	}

	m_keyVec.clear();
}

/**
* 	@brief	返回已经使用的数组大小
*/
template <class CData>
int ShmVector<CData>::Size()
{
	return m_keyVec.size();
}

/**
* 	@brief	获取指定位置下标的对象
*/
template <class CData>
CData* ShmVector<CData>::GetObj(int iPos)
{
	// 如果下标超出范围就直接挂吧
	if (iPos < 0 || iPos >(int)m_keyVec.size())
	{
		CData* pData = (CData*)NULL;
		return pData;
	}

	return &m_keyVec[iPos].second->m_oData;
}

/**
* 	@brief	获取所有对象的指针数组
*/
template <class CData>
void ShmVector<CData>::GetAllObj(std::vector<CData*>& vecDatas)
{
	vecDatas.clear();
	for (size_t i = 0; i < m_keyVec.size(); i++)
	{
		vecDatas.push_back(m_dataPool.GetObj(m_keyVec[i].second));
	}
}


/////////////////////////////////ShmMap/////////////////////////////////////////
/**
*  @brief    初始化函数
*  @note
*/
template <class CKey, class CData>
int ShmMap<CKey, CData>::Init(const int aiKey, int aiNum, int aiType)
{
	m_aiType = aiType;
	// 多加一个节点当尾结点
	aiNum += 1;
	int aiUint = sizeof(ShmMapNode);
	uint64_t ullNeedSize = sizeof(OBJPOOLHEAD) + sizeof(int) * aiNum + sizeof(char) * aiNum + aiUint * aiNum + 32;
	int iRet = m_dataPool.CreateObjPool(aiKey, ullNeedSize, aiUint, aiNum, aiType, __FILE__, __LINE__);
	if (iRet < 0)
	{
		printf("%s:Create Shared-Mem Failed,iRet[%d] %m\n", __PRETTY_FUNCTION__, iRet);
		return iRet;
	}
	if (ESSM_Resume == m_dataPool.GetStartFlag())
	{
		Resume();
	}

	return m_dataPool.GetStartFlag();
}

/**
* 	@brief	根据指定的key查找元素对象
*/
template <class CKey, class CData>
CData* ShmMap<CKey, CData>::Get(CKey oKey)
{
	if (m_keyMap.find(oKey) == m_keyMap.end())
		return NULL;

	return &m_keyMap[oKey]->m_oData;
}

/**
* 	@brief	插入一个元素对象
*/
template <class CKey, class CData>
int ShmMap<CKey, CData>::Insert(const CKey& oKey, const CData& Node)
{
	// 已经存在的重新赋值
	CData* pData = Get(oKey);
	if (pData != NULL)
	{
		*pData = Node;
	}
	else
	{
		int aiIndex = 0;
		ShmMapNode* pNode = m_dataPool.GetFreeObj(m_aiType, &aiIndex);
		if (pNode == NULL)
			return -1;

		pNode->m_oData = Node;
		pNode->m_iShmIndex = aiIndex;
		pNode->m_oKey = oKey;
		m_keyMap[oKey] = pNode;
	}

	return 0;
}

/**
* 	@brief	删除指定key的元素对象
*/
template <class CKey, class CData>
int ShmMap<CKey, CData>::Delete(const CKey& oKey)
{
	if (m_keyMap.find(oKey) == m_keyMap.end())
		return -1;

	m_dataPool.FreeObj(m_aiType, m_keyMap[oKey]->m_iShmIndex);
	m_keyMap.erase(oKey);

	return 0;
}

/**
* 	@brief	删除所有的数组对象
*/
template <class CKey, class CData>
void ShmMap<CKey, CData>::Clear()
{
	// for循环遍历m_keyMap编译不过，只能下面的方式来实现
	std::vector<int> vIdxList;
	vIdxList.reserve(m_keyMap.size());    //预分配空间
	int iCount = 0;
	int iSize = m_dataPool.GetFreeNum() + m_dataPool.GetUseNum();
	for (int i = 1; i < iSize; ++i)
	{
		if (iCount >= m_dataPool.GetUseNum())
			break;

		ShmMapNode* pDataNode = m_dataPool.GetObj(i, m_aiType);
		if (pDataNode == NULL)
			continue;

		vIdxList.push_back(pDataNode->m_iShmIndex);
		++iCount;
	}

	for (size_t i = 0; i < vIdxList.size(); i++)
	{
		m_dataPool.FreeObj(m_aiType, vIdxList[i]);
	}

	m_keyMap.clear();
}

/**
* 	@brief	返回已经使用的数组大小
*/
template <class CKey, class CData>
int ShmMap<CKey, CData>::Size()
{
	return m_keyMap.size();
}

/**
* 	@brief	获取所有对象的map列表
*/
template <class CKey, class CData>
void ShmMap<CKey, CData>::GetAllObj(std::vector<std::pair<CKey, CData*> >& allObjs)
{
	allObjs.clear();

	// for循环遍历m_keyMap编译不过，只能下面的方式来实现
	allObjs.reserve(m_keyMap.size());    //预分配空间
	int iCount = 0;
	int iSize = m_dataPool.GetFreeNum() + m_dataPool.GetUseNum();
	for (int i = 1; i < iSize; ++i)
	{
		if (iCount >= m_dataPool.GetUseNum())
			break;

		ShmMapNode* pDataNode = m_dataPool.GetObj(i, m_aiType);
		if (pDataNode == NULL)
			continue;

		allObjs.push_back(std::make_pair(pDataNode->m_oKey, &pDataNode->m_oData));
		++iCount;
	}
}

/**
*  @brief    从共享内存中恢复函数
*  @note
*/
template <class CKey, class CData>
void ShmMap<CKey, CData>::Resume()
{
	m_keyMap.clear();
	m_dataPool.ProcessAllObj(ResumeData, this, 0, 0);
}

template <class CKey, class CData>
int ShmMap<CKey, CData>::ResumeData(ShmNode<CKey, CData>* pData, void* pArgList, int iLen, int iType)
{
	if (NULL == pData || NULL == pArgList)
		return -1;

	ShmMap<CKey, CData>* pShmMap = (ShmMap<CKey, CData>*)pArgList;
	if (pShmMap->Get(pData->m_oKey) != NULL)
	{
		printf("%s:Create Shared-Mem Failed,iIndex[%d]\n", __PRETTY_FUNCTION__, pData->m_iShmIndex);
		return -1;
	}

	pShmMap->m_keyMap[pData->m_oKey] = pData;

	return 0;
}

#endif

