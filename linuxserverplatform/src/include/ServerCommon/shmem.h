#ifndef _SHMEM_H_
#define _SHMEM_H_

#include <vector>

typedef struct ShmRecordNode
{
	int iIdx;
	int iShmKey;
	unsigned int dwTotalSize;
	unsigned int dwNeedSize;
}tagShmRecordNode;

typedef std::vector<tagShmRecordNode>  ShmRecordInfoVec;


//共享内存启动模式
enum ENMShmStartMode
{
	ESSM_Init = 1,   //全新，即原来没有共享内存新创建
	ESSM_Resume = 2,   //恢复，即共享原来已经存在，恢复使用
};

#define MAX_KEY_TO_ADDRESS_NUM      50     //最大共享内存块个数

//共享内存到进程空间地址的映射
struct STShmKeyToAddress
{
	int m_iKey;      //标识key
	int m_iSize;     //共享内存大小
	ENMShmStartMode m_enmStartMode;  //启动模式
	void* m_vpMemAddr;    //对应的地址
	unsigned int m_uiOffset;   //当前已分配的偏移
	int m_iCount;   //申请共享内存次数统计
};

class CShmem
{

public:
	CShmem();
	~CShmem();

	void* CreateShmem(const int aiKey, const unsigned int aiTotalSize, const unsigned int aiNeedSize, const char* pFile, int iLine, int aiMode = 0666);

	int DestroyShmem(const int aiKey, const unsigned int aiTotalSize);

	ENMShmStartMode GetStartFlag(const int aiKey, const int aiTotalSize);

	static void RecordShmInfo(int iShmKey, unsigned int dwTotalSize, unsigned int dwNeedSize);
	static void LogShmInfo();

private:

	//通过共享内存的key和大小获得其进程空间指针
	static STShmKeyToAddress* FindAddrOfKey(const int aiKey, const int aiTotalSize);
	//增加一个共享内存的key和大小与其进程空间指针的对应关系
	static int AddOneKeyToAddress(STShmKeyToAddress& stKeyToAddr);
	//删除一个共享内存的key和大小与其进程空间指针的对应关系
	static int DelOneKeyToAddress(const int aiKey, const int aiTotalSize);

private:

	static int ms_iKeyToAddrNum;    //当前已经存在的共享内存个数
	static STShmKeyToAddress ms_stKeyToAddr[MAX_KEY_TO_ADDRESS_NUM];    //共享内存到地址数组

	static ShmRecordInfoVec  m_ShmRecordInfoVec;
	static bool m_bShmRecordInfoVecInit;

};


#endif

