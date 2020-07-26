#ifndef _SHMEN_CPP_
#define _SHMEN_CPP_

#include	<errno.h>
#include	<stdlib.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include    <sys/mman.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"shmem.h"
#include    <iostream>
#include    <fstream>
#include    "CommonHead.h"
#include    "log.h"

// #ifdef _DEBUG_ //先只在Debug环境下开
#define MEM_PROTECT_PAGE_TURN_ON
// #endif

#define MEM_PAGE_SIZE 4096 //内存页大小

//内存地址对齐一页
#ifdef MEM_PROTECT_PAGE_TURN_ON
static void* AlignPage(void* pMem, size_t* pAddLen)
{
	void* pNewMem = (void*)(((size_t)pMem + MEM_PAGE_SIZE - 1) / MEM_PAGE_SIZE * MEM_PAGE_SIZE);
	if (pAddLen != NULL)
		*pAddLen = (char*)pNewMem - (char*)pMem;
	return pNewMem;
}
#endif

int CShmem::ms_iKeyToAddrNum = 0;    //当前已经存在的共享内存个数
STShmKeyToAddress CShmem::ms_stKeyToAddr[MAX_KEY_TO_ADDRESS_NUM];    //共享内存到地址数组
bool CShmem::m_bShmRecordInfoVecInit = false;
ShmRecordInfoVec CShmem::m_ShmRecordInfoVec;

CShmem::CShmem()
{}

CShmem::~CShmem()
{}

void* CShmem::CreateShmem(const int aiKey, const unsigned int aiTotalSize, const unsigned int aiNeedSize, const char* pFile, int iLine, int aiMode)
{
	RecordShmInfo(aiKey, aiTotalSize, aiNeedSize);

	STShmKeyToAddress* pstKeyToAddr = FindAddrOfKey(aiKey, aiTotalSize);
	if (NULL != pstKeyToAddr)
	{
#ifdef MEM_PROTECT_PAGE_TURN_ON
		//从第二次分配开始 两次分配之间 增加一个保护页
		size_t uAdditionLen = 0;
		void* pPreMem = ((char*)pstKeyToAddr->m_vpMemAddr + pstKeyToAddr->m_uiOffset);
		void* pProtectMem = AlignPage(pPreMem, &uAdditionLen);
		void* pMem = (char*)pProtectMem + MEM_PAGE_SIZE;
		uAdditionLen += MEM_PAGE_SIZE;

		if (pstKeyToAddr->m_uiOffset + uAdditionLen + aiNeedSize > (unsigned int)pstKeyToAddr->m_iSize)
		{
			ERROR_LOG("CreatShmem fail. offset+additionsize+needsize:%u > keysize:%u\n",
				pstKeyToAddr->m_uiOffset + (unsigned int)uAdditionLen + (unsigned int)aiNeedSize,
				(unsigned int)pstKeyToAddr->m_iSize);
			return NULL;
		}

		//保护页地址用ERR日志打出来，方便定位问题
		fprintf(stderr, "CreateShmem mprotect page: %p %u\n", pProtectMem, MEM_PAGE_SIZE);
		ERROR_LOG("CreateShmem mprotect page: %p %u\n", pProtectMem, MEM_PAGE_SIZE);

		int iRet = mprotect(pProtectMem, MEM_PAGE_SIZE, PROT_NONE); //保护页不可访问
		if (iRet != 0)
		{
			ERROR_LOG("CreateShmem mprotect failed %d\n", iRet);
			perror("CreateShmem mprotect");
			return NULL;
		}

		pstKeyToAddr->m_uiOffset += (unsigned int)uAdditionLen + (unsigned int)aiNeedSize;
		pstKeyToAddr->m_iCount++;

		INFO_LOG("[exist]share memory key:%u, seq:%d, needsize:%u, total(shmall:%u, shmused:%u: shmleft:%u).\n",
			aiKey, pstKeyToAddr->m_iCount, aiNeedSize, aiTotalSize, pstKeyToAddr->m_uiOffset, aiTotalSize - pstKeyToAddr->m_uiOffset);


		//查找是否已经存在本地
		INFO_LOG("%s file %s line %d key %d addr %p size %d", __PRETTY_FUNCTION__, pFile, iLine, aiKey, pMem, aiNeedSize);

		return pMem;
#else
		if (pstKeyToAddr->m_uiOffset + aiNeedSize > (unsigned int)pstKeyToAddr->m_iSize)
		{
			ERROR_LOG("CreatShmem fail. offset+needsize:%u > keysize:%u\n",
				pstKeyToAddr->m_uiOffset + (unsigned int)aiNeedSize,
				(unsigned int)pstKeyToAddr->m_iSize);
			return NULL;
		}
		void* pvMem = (void*)(((char*)pstKeyToAddr->m_vpMemAddr) + pstKeyToAddr->m_uiOffset);
		pstKeyToAddr->m_uiOffset += (unsigned int)aiNeedSize;
		pstKeyToAddr->m_iCount++;

		ErrorLog("%s file %s line %d key %d addr %p size %d", __PRETTY_FUNCTION__, pFile, iLine, aiKey, pvMem, aiNeedSize);

		RecordLog(GetLogCat(), TLL_CRIT, "[exist]share memory key:%u, seq:%d, needsize:%u, total(shmall:%u, shmused:%u: shmleft:%u).\n",
			aiKey, pstKeyToAddr->m_iCount, aiNeedSize, aiTotalSize, pstKeyToAddr->m_uiOffset, aiTotalSize - pstKeyToAddr->m_uiOffset);

		return pvMem;
#endif
	}
	//不在本地则通过系统获得
	int iHandle = -1;
	ENMShmStartMode enmMode = ESSM_Init;
	void* pvMem = NULL;
	if ((iHandle = shmget(aiKey, aiTotalSize, aiMode | IPC_CREAT | IPC_EXCL)) < 0)
	{
		if (errno == EEXIST)
		{
			enmMode = ESSM_Resume;

			if ((iHandle = shmget(aiKey, aiTotalSize, aiMode)) != -1)
			{
				pvMem = (char*)shmat(iHandle, 0, 0);
				if (-1 == (int64_t)pvMem)
				{
					printf("shmat(EEXIST) %m\n");
					ERROR_LOG("shmat(%d) shmget(%d,%d,%d)  err=%m\n", iHandle, aiKey, aiTotalSize, aiMode);
					return NULL;
				}
			}
			else
			{
				printf("shmget(EEXIST) = %d %m\n", iHandle);
				ERROR_LOG("shmget(%d,%d,%d)  err=%m\n", aiKey, aiTotalSize, aiMode);

				return NULL;
			}
		}
		else
		{
			printf("shmget() = %d %m\n", iHandle);
			ERROR_LOG(" errno != EEXIST shmget(%d,%d,%d)  err=%m\n", aiKey, aiTotalSize, aiMode);
			return NULL;
		}
	}
	else
	{
		enmMode = ESSM_Init;
		pvMem = (char*)shmat(iHandle, 0, 0);
		if (-1 == (int64_t)pvMem)
		{
			printf("shmgat(Error) = %m %d\n", iHandle);
			ERROR_LOG("shmat(%d)  err=%m\n", iHandle);

			return NULL;
		}
		//因为这样会导致程序启动必然使用全部分配的空间
		//所以去掉，由分配者自行初始化
		//初始创建置为0
		//memset(pvMem, 0, aiTotalSize);

		/**
		 * 为了预防一些内存没有被初始化的BUG，在测试环境将所有内存初始化为-1，如果有未初始化的可以立刻定位到问题
		 * @add by arrowgu
		 */

#ifdef _DEBUG_  
		memset(pvMem, -1, aiTotalSize);
#endif

	}

	//查找是否已经存在本地
	INFO_LOG("%s file %s line %d key %d addr %p size %d", __PRETTY_FUNCTION__, pFile, iLine, aiKey, pvMem, aiNeedSize);

	STShmKeyToAddress stKeyToAddr = { 0 };
	stKeyToAddr.m_iKey = aiKey;
	stKeyToAddr.m_iSize = aiTotalSize;
	stKeyToAddr.m_enmStartMode = enmMode;
	stKeyToAddr.m_vpMemAddr = pvMem;
	stKeyToAddr.m_iCount = 1;
	if (aiNeedSize > aiTotalSize)
	{
		stKeyToAddr.m_uiOffset = 0;
		pvMem = NULL;
	}
	else
	{
		stKeyToAddr.m_uiOffset = (unsigned int)aiNeedSize;

		INFO_LOG("[first]share memory key:%u, seq:%d, needsize:%u, total(shmall:%u, shmused:%u: shmleft:%u) startmode(%d).\n",
			aiKey, stKeyToAddr.m_iCount, aiNeedSize, aiTotalSize, stKeyToAddr.m_uiOffset, aiTotalSize - stKeyToAddr.m_uiOffset,
			enmMode);
	}

	AddOneKeyToAddress(stKeyToAddr);

	return pvMem;
}

int CShmem::DestroyShmem(const int aiKey, const unsigned int aiTotalSize)
{
	int iHandle = shmget(aiKey, aiTotalSize, 0640 | IPC_EXCL);

	if (iHandle >= 0)
	{
		DelOneKeyToAddress(aiKey, aiTotalSize);
		shmctl(iHandle, IPC_RMID, 0);
	}

	return 0;
}

ENMShmStartMode CShmem::GetStartFlag(const int aiKey, const int aiTotalSize)
{
	//查找是否已经存在
	STShmKeyToAddress* pstKeyToAddr = FindAddrOfKey(aiKey, aiTotalSize);
	if (NULL != pstKeyToAddr)
	{
		return pstKeyToAddr->m_enmStartMode;
	}

	return ESSM_Init;
}

STShmKeyToAddress* CShmem::FindAddrOfKey(const int aiKey, const int aiTotalSize)
{
	for (int i = 0; i < ms_iKeyToAddrNum; ++i)
	{
		if ((ms_stKeyToAddr[i].m_iKey == aiKey) && (ms_stKeyToAddr[i].m_iSize == aiTotalSize))
		{
			return &ms_stKeyToAddr[i];
		}
	}
	return NULL;
}

int CShmem::AddOneKeyToAddress(STShmKeyToAddress& stKeyToAddr)
{
	if (ms_iKeyToAddrNum >= MAX_KEY_TO_ADDRESS_NUM)
	{
		printf("key to address is full\n");
		return -1;
	}

	ms_stKeyToAddr[ms_iKeyToAddrNum] = stKeyToAddr;

	++ms_iKeyToAddrNum;

	return 0;
}

int CShmem::DelOneKeyToAddress(const int aiKey, const int aiTotalSize)
{
	int i = 0;
	for (i = 0; i < ms_iKeyToAddrNum; ++i)
	{
		if ((ms_stKeyToAddr[i].m_iKey == aiKey) && (ms_stKeyToAddr[i].m_iSize == aiTotalSize))
		{
			break;
		}
	}
	if (i < ms_iKeyToAddrNum)
	{
		for (; i < ms_iKeyToAddrNum - 1; ++i)
		{
			ms_stKeyToAddr[i] = ms_stKeyToAddr[i + 1];
		}
		--ms_iKeyToAddrNum;
		return 0;
	}
	return -1;
}

void CShmem::RecordShmInfo(int iShmKey, unsigned int dwTotalSize, unsigned int dwNeedSize)
{
	if (!m_bShmRecordInfoVecInit)
	{
		m_ShmRecordInfoVec.clear();
		m_bShmRecordInfoVecInit = true;
	}

	static int siIdx = 1;
	tagShmRecordNode stNode = { 0 };
	stNode.iIdx = siIdx;
	stNode.iShmKey = iShmKey;
	stNode.dwTotalSize = dwTotalSize;
	stNode.dwNeedSize = dwNeedSize;
	m_ShmRecordInfoVec.push_back(stNode);
	++siIdx;
}

void CShmem::LogShmInfo()
{
	if (!m_bShmRecordInfoVecInit)
	{
		ERROR_LOG("%s, ShmRecordInfoVec not Init", __PRETTY_FUNCTION__);
		return;
	}

	bool bErrLog = true;
	char szLogName[128];
	snprintf(szLogName, sizeof(szLogName), "../log/ZoneShmInfoRecord_%u.log", (unsigned int)time(NULL));
	std::ofstream logfile(szLogName);
	if (logfile.is_open())
	{
		bErrLog = false;
	}

	ShmRecordInfoVec::iterator iter;
	iter = m_ShmRecordInfoVec.begin();
	while (m_ShmRecordInfoVec.end() != iter)
	{
		//bErrLog为true时，记录到zone的error日志里
		if (bErrLog)
		{
			ERROR_LOG("%s Idx %d Key %d TotalSize %u NeedSize %u", __FUNCTION__, iter->iIdx,
				iter->iShmKey, iter->dwTotalSize, iter->dwNeedSize);
		}
		else  //否则就是记录在有时间戳命名的单独文件里
		{
			logfile << iter->iIdx << " " << iter->iShmKey << " " << iter->dwTotalSize << " " << iter->dwNeedSize << "\n";
		}
		++iter;
	}
}


#endif

