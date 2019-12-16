#pragma once

#include "GameDesk.h"
#include <assert.h>

#define ASSERT	assert

//////////////////////////////////////////////////////////////////////////////////

//数组模板类
template <class TYPE, class ARG_TYPE=const TYPE &> class CWHArray
{
	//变量定义
protected:
	TYPE *							m_pData;							//数组指针
	UINT							m_nMaxCount;						//缓冲数目
	UINT							m_nGrowCount;						//增长数目
	UINT							m_nElementCount;					//元素数目

	//函数定义
public:
	//构造函数
	CWHArray();
	//析构函数
	virtual ~CWHArray();

	//信息函数
public:
	//是否空组
	bool IsEmpty() const;
	//获取数目
	UINT GetCount() const;

	//功能函数
public:
	//获取缓冲
	TYPE * GetData();
	//获取缓冲
	const TYPE * GetData() const;
	//增加元素
	UINT Add(ARG_TYPE newElement);
	//拷贝数组
	void Copy(const CWHArray & Src);
	//追加数组
	UINT Append(const CWHArray & Src);
	//获取元素
	TYPE & GetAt(UINT nIndex);
	//获取元素
	const TYPE & GetAt(UINT nIndex) const;
	//获取元素
	TYPE & ElementAt(UINT nIndex);
	//获取元素
	//const TYPE & ElementAt(UINT nIndex) const;

	//操作函数
public:
	//设置大小
	void SetSize(UINT nNewSize);
	//设置元素
	void SetAt(UINT nIndex, ARG_TYPE newElement);
	//设置元素
	void SetAtGrow(UINT nIndex, ARG_TYPE newElement);
	////插入数据
	//void InsertAt(UINT nIndex, const CWHArray & Src);
	////插入数据
	//void InsertAt(UINT nIndex, ARG_TYPE newElement, UINT nCount=1);
	//删除数据
	void RemoveAt(UINT nIndex, UINT nCount=1);
	//删除元素
	void RemoveAll();

	//操作重载
public:
	//操作重载
	TYPE & operator[](UINT nIndex);
	//操作重载
	const TYPE & operator[](UINT nIndex) const;

	//内存函数
public:
	//释放内存
	void FreeMemory();
	//申请内存
	void AllocMemory(UINT nNewCount);
};

//////////////////////////////////////////////////////////////////////////////////
// CWHArray<TYPE, ARG_TYPE> 内联函数

//是否空组
template<class TYPE, class ARG_TYPE> 
bool CWHArray<TYPE, ARG_TYPE>::IsEmpty() const
{
	return (m_nElementCount==0);
}

//获取数目
template<class TYPE, class ARG_TYPE>
UINT CWHArray<TYPE, ARG_TYPE>::GetCount() const
{
	return m_nElementCount;
}

//增加元素
template<class TYPE, class ARG_TYPE>
UINT CWHArray<TYPE,ARG_TYPE>::Add(ARG_TYPE newElement)
{
	UINT nIndex=m_nElementCount;
	SetAtGrow(nIndex,newElement);
	return nIndex;
}

//操作重载
template<class TYPE, class ARG_TYPE>
TYPE & CWHArray<TYPE, ARG_TYPE>::operator[](UINT nIndex)
{ 
	return ElementAt(nIndex);
}

//操作重载
template<class TYPE, class ARG_TYPE>
const TYPE & CWHArray<TYPE, ARG_TYPE>::operator[](UINT nIndex) const
{ 
	return GetAt(nIndex);
}

//////////////////////////////////////////////////////////////////////////////////
// CWHArray<TYPE, ARG_TYPE> 外联函数

//构造函数
template<class TYPE, class ARG_TYPE> 
CWHArray<TYPE, ARG_TYPE>::CWHArray()
{
	m_pData=NULL;
	m_nMaxCount=0;
	m_nGrowCount=0;
	m_nElementCount=0;

	return;
}

//构造函数
template<class TYPE, class ARG_TYPE> 
CWHArray<TYPE,ARG_TYPE>::~CWHArray()
{
	if (m_pData!=NULL)
	{
		for (UINT i=0;i<m_nElementCount;i++)	(m_pData+i)->~TYPE();
		delete [] (BYTE *)m_pData;
		m_pData=NULL;
	}

	return;
}

//获取缓冲
template<class TYPE, class ARG_TYPE> 
TYPE * CWHArray<TYPE,ARG_TYPE>::GetData()
{
	return m_pData;
}

//获取缓冲
template<class TYPE, class ARG_TYPE> 
const TYPE * CWHArray<TYPE,ARG_TYPE>::GetData() const
{
	return m_pData;
}

//拷贝数组
template<class TYPE, class ARG_TYPE> 
void CWHArray<TYPE,ARG_TYPE>::Copy(const CWHArray & Src)
{
	//效验参数
	ASSERT(this!=&Src);
	if (this==&Src) return;

	//拷贝数组
	AllocMemory(Src.m_nElementCount);
	if (m_nElementCount>0)
	{
		for (UINT i=0;i<m_nElementCount;i++) (m_pData+i)->~TYPE();
		memset(m_pData,0,m_nElementCount*sizeof(TYPE));
	}
	for (UINT i=0;i<Src.m_nElementCount;i++)	m_pData[i]=Src.m_pData[i];
	m_nElementCount=Src.m_nElementCount;

	return;
}

//追加数组
template<class TYPE, class ARG_TYPE> 
UINT CWHArray<TYPE,ARG_TYPE>::Append(const CWHArray & Src)
{
	//效验参数
	ASSERT(this!=&Src);
	if (this==&Src) throw 0;

	//拷贝数组
	if (Src.m_nElementCount>0)
	{
		UINT nOldCount=m_nElementCount;
		AllocMemory(m_nElementCount+Src.m_nElementCount);
		for (UINT i=0;i<Src.m_nElementCount;i++)	m_pData[m_nElementCount+i]=Src.m_pData[i];
		m_nElementCount+=Src.m_nElementCount;
	}

	return m_nElementCount;
}

//获取元素
template<class TYPE, class ARG_TYPE> 
TYPE & CWHArray<TYPE,ARG_TYPE>::GetAt(UINT nIndex)
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)||(nIndex>=m_nElementCount)) throw 0;
	
	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE> 
const TYPE & CWHArray<TYPE,ARG_TYPE>::GetAt(UINT nIndex) const
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)||(nIndex>=m_nElementCount)) throw 0;
	
	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE> 
TYPE & CWHArray<TYPE,ARG_TYPE>::ElementAt(UINT nIndex)
{
	//ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)||(nIndex>=m_nElementCount)) throw 0;
	
	return m_pData[nIndex];
}

////获取元素
//template<class TYPE, class ARG_TYPE> 
//const TYPE & CWHArray<TYPE,ARG_TYPE>::ElementAt(UINT nIndex) const
//{
//	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
//	if ((nIndex<0)&&(nIndex>=m_nElementCount)) throw 0;
//
//	return m_pData[nIndex];
//}

//设置大小
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::SetSize(UINT nNewSize)
{
	//效验参数
	ASSERT(nNewSize>=0);
	if (nNewSize<0)	throw 0;
	
	//设置大小
	AllocMemory(nNewSize);
	if (nNewSize>m_nElementCount)
	{
		for (UINT i=m_nElementCount;i<nNewSize;i++) new ((void *)(m_pData+i)) TYPE;
	}
	else if (nNewSize<m_nElementCount)
	{
		for (UINT i=nNewSize;i<m_nElementCount;i++) (m_pData+i)->~TYPE();
		memset(m_pData+nNewSize,0,(m_nElementCount-nNewSize)*sizeof(TYPE));
	}
	m_nElementCount=nNewSize;

	return;
}

//设置元素
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::SetAt(UINT nIndex, ARG_TYPE newElement)
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex>=0)&&(nIndex<m_nElementCount)) m_pData[nIndex]=newElement;
	else throw 0;

	return;
}

//设置元素
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::SetAtGrow(UINT nIndex, ARG_TYPE newElement)
{
	//效验参数
	ASSERT(nIndex>=0);
	if (nIndex<0) throw 0;

	//设置元素
	if (nIndex>=m_nElementCount) SetSize(m_nElementCount+1);
	m_pData[nIndex]=newElement;

	return;
}

////插入数据
//template<class TYPE, class ARG_TYPE>
//void CWHArray<TYPE,ARG_TYPE>::InsertAt(UINT nIndex, const CWHArray & Src)
//{
//	//效验参数
//	ASSERT(nIndex >=0);
//	if (nIndex <0) throw 0;
//
//	if (Src.m_nElementCount>0)
//	{
//		//申请数组
//		if (nIndex<m_nElementCount)
//		{
//			UINT nOldCount=m_nElementCount;
//			SetSize(m_nElementCount+Src.m_nElementCount);
//			for (UINT i=0;i<nCount;i++) (m_pData+nOldCount+i)->~TYPE();
//			memmove(m_pData+nIndex+nCount,m_pData+nIndex,(nOldCount-nIndex)*sizeof(TYPE));
//			memset(m_pData+nIndex,0,Src.m_nElementCount*sizeof(TYPE));
//			for (UINT i=0;i<Src.m_nElementCount;i++) new (m_pData+nIndex+i) TYPE();
//		}
//		else SetSize(nIndex+nCount);
//
//		//拷贝数组
//		ASSERT((nIndex+Src.m_nElementCount)<=m_nElementCount);
//		while (nCount--) m_pData[nIndex++]=newElement;
//	}
//
//	return;
//}
//
////插入数据
//template<class TYPE, class ARG_TYPE>
//void CWHArray<TYPE,ARG_TYPE>::InsertAt(UINT nIndex, ARG_TYPE newElement, UINT nCount)
//{
//	//效验参数
//	ASSERT(nIndex>=0);
//	ASSERT(nCount>0);
//	if ((nIndex<0)||(nCount<=0)) throw 0;
//
//	//申请数组
//	if (nIndex<m_nElementCount)
//	{
//		UINT nOldCount=m_nElementCount;
//		SetSize(m_nElementCount+nCount);
//		for (UINT i=0;i<nCount;i++) (m_pData+nOldCount+i)->~TYPE();
//		memmove(m_pData+nIndex+nCount,m_pData+nIndex,(nOldCount-nIndex)*sizeof(TYPE));
//		memset(m_pData+nIndex,0,nCount*sizeof(TYPE));
//		for (UINT i=0;i<nCount;i++) new (m_pData+nIndex+i) TYPE();
//	}
//	else SetSize(nIndex+nCount);
//
//	//拷贝数组
//	ASSERT((nIndex+nCount)<=m_nElementCount);
//	while (nCount--) m_pData[nIndex++]=newElement;
//
//	return;
//}

//删除数据
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::RemoveAt(UINT nIndex, UINT nCount)
{
	//效验参数
	ASSERT(nIndex>=0);
	ASSERT(nCount>=0);
	ASSERT(nIndex+nCount<=m_nElementCount);
	if ((nIndex<0)||(nCount<0)||((nIndex+nCount>m_nElementCount))) throw 0;

	//删除数据
	UINT nMoveCount=m_nElementCount-(nIndex+nCount);
	for (UINT i=0;i<nCount;i++) (m_pData+nIndex+i)->~TYPE();
	if (nMoveCount>0) memmove(m_pData+nIndex,m_pData+nIndex+nCount,nMoveCount*sizeof(TYPE));
	m_nElementCount-=nCount;

	return;
}

//删除元素
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::RemoveAll()
{
	if (m_nElementCount>0)
	{
		for (UINT i=0;i<m_nElementCount;i++) (m_pData+i)->~TYPE();
		memset(m_pData,0,m_nElementCount*sizeof(TYPE));
		m_nElementCount=0;
	}

	return;
}

//释放内存
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::FreeMemory()
{
	if (m_nElementCount!=m_nMaxCount)
	{
		TYPE * pNewData=NULL;
		if (m_nElementCount!=0)
		{
			pNewData=(TYPE *) new BYTE[m_nElementCount*sizeof(TYPE)];
			memcpy(pNewData,m_pData,m_nElementCount*sizeof(TYPE));
		}
		delete [] (BYTE *)m_pData;
		m_pData=pNewData;
		m_nMaxCount=m_nElementCount;
	}

	return;
}

//申请内存
template<class TYPE, class ARG_TYPE>
void CWHArray<TYPE,ARG_TYPE>::AllocMemory(UINT nNewCount)
{
	//效验参数
	ASSERT(nNewCount>=0);

	if (nNewCount>m_nMaxCount)
	{
		//计算数目
		UINT nGrowCount=m_nGrowCount;
		if (nGrowCount==0)
		{
			nGrowCount=m_nElementCount/8;
			nGrowCount=(nGrowCount<4)?4:((nGrowCount>1024)?1024:nGrowCount);
		}
		nNewCount+=nGrowCount;

		//申请内存
		TYPE * pNewData=(TYPE *) new BYTE[nNewCount*sizeof(TYPE)];
		memcpy(pNewData,m_pData,m_nElementCount*sizeof(TYPE));
		memset(pNewData+m_nElementCount,0,(nNewCount-m_nElementCount)*sizeof(TYPE));
		delete [] (BYTE *)m_pData;

		//设置变量
		m_pData=pNewData;
		m_nMaxCount=nNewCount;
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////////