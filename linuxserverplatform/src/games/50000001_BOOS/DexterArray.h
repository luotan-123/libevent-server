#ifndef DEXTER_ARRAY_HEAD_FILE
#define DEXTER_ARRAY_HEAD_FILE

#pragma once

//////////////////////////////////////////////////////////////////////////////////

//数组模板类
template <class TYPE, class ARG_TYPE=const TYPE &> class CDexterArray
{
	//变量定义
protected:
	TYPE *							m_pData;							//数组指针
	INT_PTR							m_nMaxCount;						//缓冲数目
	INT_PTR							m_nGrowCount;						//增长数目
	INT_PTR							m_nElementCount;					//元素数目

	//函数定义
public:
	//构造函数
	CDexterArray();
	//析构函数
	virtual ~CDexterArray();

	//信息函数
public:
	//是否空组
	bool IsEmpty() const;
	//获取数目
	INT_PTR GetCount() const;

	//功能函数
public:
	//获取缓冲
	TYPE * GetData();
	//获取缓冲
	const TYPE * GetData() const;
	//增加元素
	INT_PTR Add(ARG_TYPE newElement);
	//拷贝数组
	VOID Copy(const CDexterArray & Src);
	//追加数组
	INT_PTR Append(const CDexterArray & Src);
	//获取元素
	TYPE & GetAt(INT_PTR nIndex);
	//获取元素
	const TYPE & GetAt(INT_PTR nIndex) const;
	//获取元素
	TYPE & ElementAt(INT_PTR nIndex);
	//获取元素
	const TYPE & ElementAt(INT_PTR nIndex) const;

	//操作函数
public:
	//设置大小
	VOID SetSize(INT_PTR nNewSize);
	//设置元素
	VOID SetAt(INT_PTR nIndex, ARG_TYPE newElement);
	//设置元素
	VOID SetAtGrow(INT_PTR nIndex, ARG_TYPE newElement);
	//插入数据
	VOID InsertAt(INT_PTR nIndex, const CDexterArray & Src);
	//插入数据
	VOID InsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount=1);
	//删除数据
	VOID RemoveAt(INT_PTR nIndex, INT_PTR nCount=1);
	//删除元素
	VOID RemoveAll();

	//操作重载
public:
	//操作重载
	TYPE & operator[](INT_PTR nIndex);
	//操作重载
	const TYPE & operator[](INT_PTR nIndex) const;

	//内存函数
public:
	//释放内存
	VOID FreeMemory();
	//申请内存
	VOID AllocMemory(INT_PTR nNewCount);
};

//////////////////////////////////////////////////////////////////////////////////
// CDexterArray<TYPE, ARG_TYPE> 内联函数

//是否空组
template<class TYPE, class ARG_TYPE> 
AFX_INLINE bool CDexterArray<TYPE, ARG_TYPE>::IsEmpty() const
{
	return (m_nElementCount==0);
}

//获取数目
template<class TYPE, class ARG_TYPE>
AFX_INLINE INT_PTR CDexterArray<TYPE, ARG_TYPE>::GetCount() const
{
	return m_nElementCount;
}

//增加元素
template<class TYPE, class ARG_TYPE>
AFX_INLINE INT_PTR CDexterArray<TYPE,ARG_TYPE>::Add(ARG_TYPE newElement)
{
	INT_PTR nIndex=m_nElementCount;
	SetAtGrow(nIndex,newElement);
	return nIndex;
}

//操作重载
template<class TYPE, class ARG_TYPE>
AFX_INLINE TYPE & CDexterArray<TYPE, ARG_TYPE>::operator[](INT_PTR nIndex)
{ 
	return ElementAt(nIndex);
}

//操作重载
template<class TYPE, class ARG_TYPE>
AFX_INLINE const TYPE & CDexterArray<TYPE, ARG_TYPE>::operator[](INT_PTR nIndex) const
{ 
	return GetAt(nIndex);
}

//////////////////////////////////////////////////////////////////////////////////
// CDexterArray<TYPE, ARG_TYPE> 外联函数

//构造函数
template<class TYPE, class ARG_TYPE> 
CDexterArray<TYPE, ARG_TYPE>::CDexterArray()
{
	m_pData=NULL;
	m_nMaxCount=0;
	m_nGrowCount=0;
	m_nElementCount=0;

	return;
}

//构造函数
template<class TYPE, class ARG_TYPE> 
CDexterArray<TYPE,ARG_TYPE>::~CDexterArray()
{
	if (m_pData!=NULL)
	{
		for (INT_PTR i=0;i<m_nElementCount;i++)	(m_pData+i)->~TYPE();
		delete [] (BYTE *)m_pData;
		m_pData=NULL;
	}

	return;
}

//获取缓冲
template<class TYPE, class ARG_TYPE> 
TYPE * CDexterArray<TYPE,ARG_TYPE>::GetData()
{
	return m_pData;
}

//获取缓冲
template<class TYPE, class ARG_TYPE> 
const TYPE * CDexterArray<TYPE,ARG_TYPE>::GetData() const
{
	return m_pData;
}

//拷贝数组
template<class TYPE, class ARG_TYPE> 
VOID CDexterArray<TYPE,ARG_TYPE>::Copy(const CDexterArray & Src)
{
	//效验参数
	ASSERT(this!=&Src);
	if (this==&Src) return;

	//拷贝数组
	AllocMemory(Src.m_nElementCount);
	if (m_nElementCount>0)
	{
		for (INT_PTR i=0;i<m_nElementCount;i++) (m_pData+i)->~TYPE();
		memset(m_pData,0,m_nElementCount*sizeof(TYPE));
	}
	for (INT_PTR i=0;i<Src.m_nElementCount;i++)	m_pData[i]=Src.m_pData[i];
	m_nElementCount=Src.m_nElementCount;

	return;
}

//追加数组
template<class TYPE, class ARG_TYPE> 
INT_PTR CDexterArray<TYPE,ARG_TYPE>::Append(const CDexterArray & Src)
{
	//效验参数
	ASSERT(this!=&Src);
	if (this==&Src) AfxThrowInvalidArgException();

	//拷贝数组
	if (Src.m_nElementCount>0)
	{
		INT_PTR nOldCount=m_nElementCount;
		AllocMemory(m_nElementCount+Src.m_nElementCount);
		for (INT_PTR i=0;i<Src.m_nElementCount;i++)	m_pData[m_nElementCount+i]=Src.m_pData[i];
		m_nElementCount+=Src.m_nElementCount;
	}

	return m_nElementCount;
}

//获取元素
template<class TYPE, class ARG_TYPE> 
TYPE & CDexterArray<TYPE,ARG_TYPE>::GetAt(INT_PTR nIndex)
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)||(nIndex>=m_nElementCount)) AfxThrowInvalidArgException();

	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE> 
const TYPE & CDexterArray<TYPE,ARG_TYPE>::GetAt(INT_PTR nIndex) const
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)||(nIndex>=m_nElementCount)) AfxThrowInvalidArgException();

	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE> 
TYPE & CDexterArray<TYPE,ARG_TYPE>::ElementAt(INT_PTR nIndex)
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)&&(nIndex>=m_nElementCount)) AfxThrowInvalidArgException();

	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE> 
const TYPE & CDexterArray<TYPE,ARG_TYPE>::ElementAt(INT_PTR nIndex) const
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex<0)&&(nIndex>=m_nElementCount)) AfxThrowInvalidArgException();

	return m_pData[nIndex];
}

//设置大小
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::SetSize(INT_PTR nNewSize)
{
	//效验参数
	ASSERT(nNewSize>=0);
	if (nNewSize<0)	AfxThrowInvalidArgException();

	//设置大小
	AllocMemory(nNewSize);
	if (nNewSize>m_nElementCount)
	{
		for (INT_PTR i=m_nElementCount;i<nNewSize;i++) new ((VOID *)(m_pData+i)) TYPE;
	}
	else if (nNewSize<m_nElementCount)
	{
		for (INT_PTR i=nNewSize;i<m_nElementCount;i++) (m_pData+i)->~TYPE();
		memset(m_pData+nNewSize,0,(m_nElementCount-nNewSize)*sizeof(TYPE));
	}
	m_nElementCount=nNewSize;

	return;
}

//设置元素
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::SetAt(INT_PTR nIndex, ARG_TYPE newElement)
{
	ASSERT((nIndex>=0)&&(nIndex<m_nElementCount));
	if ((nIndex>=0)&&(nIndex<m_nElementCount)) m_pData[nIndex]=newElement;
	else AfxThrowInvalidArgException();

	return;
}

//设置元素
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::SetAtGrow(INT_PTR nIndex, ARG_TYPE newElement)
{
	//效验参数
	ASSERT(nIndex>=0);
	if (nIndex<0) AfxThrowInvalidArgException();

	//设置元素
	if (nIndex>=m_nElementCount) SetSize(m_nElementCount+1);
	m_pData[nIndex]=newElement;

	return;
}

//插入数据
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::InsertAt(INT_PTR nIndex, const CDexterArray & Src)
{
	//效验参数
	ASSERT(nStartIndex>=0);
	if (nStartIndex<0) AfxThrowInvalidArgException();

	if (Src.m_nElementCount>0)
	{
		//申请数组
		if (nIndex<m_nElementCount)
		{
			INT_PTR nOldCount=m_nElementCount;
			SetSize(m_nElementCount+Src.m_nElementCount);
			for (INT_PTR i=0;i<nCount;i++) (m_pData+nOldCount+i)->~TYPE();
			memmove(m_pData+nIndex+nCount,m_pData+nIndex,(nOldCount-nIndex)*sizeof(TYPE));
			memset(m_pData+nIndex,0,Src.m_nElementCount*sizeof(TYPE));
			for (INT_PTR i=0;i<Src.m_nElementCount;i++) new (m_pData+nIndex+i) TYPE();
		}
		else SetSize(nIndex+nCount);

		//拷贝数组
		ASSERT((nIndex+Src.m_nElementCount)<=m_nElementCount);
		while (nCount--) m_pData[nIndex++]=newElement;
	}

	return;
}

//插入数据
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::InsertAt(INT_PTR nIndex, ARG_TYPE newElement, INT_PTR nCount)
{
	//效验参数
	ASSERT(nIndex>=0);
	ASSERT(nCount>0);
	if ((nIndex<0)||(nCount<=0)) AfxThrowInvalidArgException();

	//申请数组
	if (nIndex<m_nElementCount)
	{
		INT_PTR nOldCount=m_nElementCount;
		SetSize(m_nElementCount+nCount);
		for (INT_PTR i=0;i<nCount;i++) (m_pData+nOldCount+i)->~TYPE();
		memmove(m_pData+nIndex+nCount,m_pData+nIndex,(nOldCount-nIndex)*sizeof(TYPE));
		memset(m_pData+nIndex,0,nCount*sizeof(TYPE));
		for (INT_PTR i=0;i<nCount;i++) new (m_pData+nIndex+i) TYPE();
	}
	else SetSize(nIndex+nCount);

	//拷贝数组
	ASSERT((nIndex+nCount)<=m_nElementCount);
	while (nCount--) m_pData[nIndex++]=newElement;

	return;
}

//删除数据
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::RemoveAt(INT_PTR nIndex, INT_PTR nCount)
{
	//效验参数
	ASSERT(nIndex>=0);
	ASSERT(nCount>=0);
	ASSERT(nIndex+nCount<=m_nElementCount);
	if ((nIndex<0)||(nCount<0)||((nIndex+nCount>m_nElementCount))) AfxThrowInvalidArgException();

	//删除数据
	INT_PTR nMoveCount=m_nElementCount-(nIndex+nCount);
	for (INT_PTR i=0;i<nCount;i++) (m_pData+nIndex+i)->~TYPE();
	if (nMoveCount>0) memmove(m_pData+nIndex,m_pData+nIndex+nCount,nMoveCount*sizeof(TYPE));
	m_nElementCount-=nCount;

	return;
}

//删除元素
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::RemoveAll()
{
	if (m_nElementCount>0)
	{
		for (INT_PTR i=0;i<m_nElementCount;i++) (m_pData+i)->~TYPE();
		memset(m_pData,0,m_nElementCount*sizeof(TYPE));
		m_nElementCount=0;
	}

	return;
}

//释放内存
template<class TYPE, class ARG_TYPE>
VOID CDexterArray<TYPE,ARG_TYPE>::FreeMemory()
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
VOID CDexterArray<TYPE,ARG_TYPE>::AllocMemory(INT_PTR nNewCount)
{
	//效验参数
	ASSERT(nNewCount>=0);

	if (nNewCount>m_nMaxCount)
	{
		//计算数目
		INT_PTR nGrowCount=m_nGrowCount;
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

#endif