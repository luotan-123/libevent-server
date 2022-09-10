#include "CommonHead.h"
#include "shm_obj_pool.h"
#include "shm_stl.h"

class A:public CXObj
{
public:
    A() {}
    ~A() {}
    //bool CheckMem() { return true; }
    void Init() { /*printf("第一次使用对象内存\n");*/ }
    void Resume() { /*printf("恢复对象内存\n");*/ }
    static A* CreateObject(void* p)
    {
        A* pa = (A*)p;


        printf("val = %d\n", pa->val);


        return pa;
    }
    void Reclaim()
    {
        val = 0;
        printf("回收对象内存\n");
    }
    int val;
};

class BB
{
public:
    BB() {}
    ~BB() {}
    bool CheckMem() { return true; }
    void Init() { /*printf("第一次使用对象内存\n");*/ }
    void Resume() { /*printf("恢复对象内存\n");*/ }
    static BB* CreateObject(void* p)
    {
        BB* pa = (BB*)p;


        printf("val = %d\n", pa->val);


        return pa;
    }
    void Reclaim()
    {
        val = 0;
        printf("回收对象内存\n");
    }
    int val;
    int x;
};

const int a_size = 4;


class TestShmVirtualClass :public CXObj
{
public:
    TestShmVirtualClass() 
    {
        printf("TestShmVirtualClass()\n");
    }
    ~TestShmVirtualClass() 
    {
        printf("~TestShmVirtualClass()\n");
    }
    //bool CheckMem() { return true; }
    void Init() 
    { 
        val = 0;
        val2 = 0;
        printf("第一次使用对象内存\n");
    }
    void Resume() 
    { 
        printf("恢复对象内存 val = %d  val2=%ld \n", val, val2);
    }
    /*static TestShmVirtualClass* CreateObject(void* p)
    {
        TestShmVirtualClass* pa = (TestShmVirtualClass*)p;


        printf("创建对象 val = %d  val2=%ld \n", pa->val, pa->val2);


        return pa;
    }*/
    void Reclaim()
    {
        val = 0;
        val2 = 0;
        printf("回收对象内存\n");
    }

    virtual void PutFuc()
    {
        printf("PutFuc  add (%ld)\n", val + val2);
    }

    int val;
    long val2;
};
class TestShmVirtualClassLV :public TestShmVirtualClass
{
public:
    TestShmVirtualClassLV()
    {
        printf("TestShmVirtualClassLV()\n");
    }
    ~TestShmVirtualClassLV()
    {
        printf("~TestShmVirtualClassLV()\n");
    }
    //bool CheckMem() { return true; }
    void Init()
    {
        TestShmVirtualClass::Init();
        val3 = 0;
        printf("第一次使用对象内存\n");
    }
    void Resume()
    {
        TestShmVirtualClass::Resume();
        printf("恢复对象内存 val = %d  val2=%ld val3=%d\n", val, val2, val3);
    }
    /*static TestShmVirtualClassLV* CreateObject(void* p)
    {
        TestShmVirtualClassLV* pa = (TestShmVirtualClassLV*)p;


        printf("创建对象 val = %d  val2=%ld val3=%d\n", pa->val, pa->val2, pa->val3);


        return pa;
    }*/
    void Reclaim()
    {
        TestShmVirtualClass::Reclaim();
        val3 = 0;
        printf("回收对象内存\n");
    }

    virtual void PutFuc()
    {
        printf("PutFuc  dec (%ld) val3=%d\n", val2 - val, val3);
    }

    DECLARE_DYN;

    int val3;
};

IMPLEMENT_DYN(TestShmVirtualClassLV);

TestShmVirtualClass* m_pstData = NULL;
int ShmVirtual()
{
    CShmem loShmem;
    int iNeedSize = sizeof(TestShmVirtualClass) * 5;

    char* szShmHead = (char*)loShmem.CreateShmem(0x951107, 1024000, iNeedSize, __FILE__, __LINE__);
    if (NULL == szShmHead)
    {
        return -1;
    }

    //判断是否第一次调用。
    ENMShmStartMode enmMode = loShmem.GetStartFlag(0x951107, 1024000);
    if (ESSM_Resume == enmMode)
    {
        m_pstData = (TestShmVirtualClass*)szShmHead;
        printf("CEveryWeekActMgr::InitShm()  Resume!\n");
    }
    else
    {
        memset(szShmHead, 0, iNeedSize);
        m_pstData = (TestShmVirtualClass*)szShmHead;
        printf("CEveryWeekActMgr::InitShm()  Inited!\n");
    }
}

TestShmVirtualClassLV* m_pstDataLV = NULL;
int ShmVirtualLV()
{
    const int CLASS_ASIZE = 4;
    CShmem loShmem;
    int iNeedSize = sizeof(TestShmVirtualClassLV) * CLASS_ASIZE;

    char* szShmHead = (char*)loShmem.CreateShmem(0x951107, 1024000, iNeedSize, __FILE__, __LINE__);
    if (NULL == szShmHead)
    {
        return -1;
    }

    //判断是否第一次调用。
    ENMShmStartMode enmMode = loShmem.GetStartFlag(0x951107, 1024000);
    if (ESSM_Resume == enmMode)
    {
        m_pstDataLV = (TestShmVirtualClassLV*)szShmHead;
        printf("CEveryWeekActMgr::InitShm()  Resume!\n");
    }
    else
    {
        memset(szShmHead, 0, iNeedSize);
        m_pstDataLV = (TestShmVirtualClassLV*)szShmHead;
        printf("CEveryWeekActMgr::InitShm()  Inited!\n");
    }

    for (int i = 0; i < CLASS_ASIZE; i++)
    {
        TestShmVirtualClassLV* lpoObj = &(m_pstDataLV[i]);
        lpoObj = (TestShmVirtualClassLV*)TestShmVirtualClassLV::CreateObject((void*)lpoObj);
        if (ESSM_Init == enmMode)
        {
            lpoObj->Init();
        }
        else
        {
            lpoObj->Resume();
        }

        lpoObj->PutFuc();
    }
    for (int i = 0; i < CLASS_ASIZE; i++)
    {
        m_pstDataLV[i].val = i;
        m_pstDataLV[i].val2 = i + 1;
        m_pstDataLV[i].val3 = i + 9;
    }

}

int TestShm()
{
    //ShmVirtual();
    ShmVirtualLV();

    return 0;

    srand((unsigned int)time(0));

    ShmVector<A> shmvec;
    ShmMap<int,A> shmvmap;

    // 第二个参数是数量
    if (ESSM_Resume == shmvec.Init(0x123456, 3, 0))
    {
        printf("恢复内存\n");
    }
    else
    {
        printf("第一次使用内存\n");
    }
    A a,b;
    a.val = 11111;
    b.val = 22222;
    shmvec.PushBack(a);

    int ret = shmvmap.Init(0x123457, 3, 0);
    if (ESSM_Resume == ret)
    {
        printf("恢复内存\n");
    }
    else if(ESSM_Init == ret)
    {
        printf("第一次使用内存\n");
    }
    else
    {
        printf("出错\n");
        return -1;
    }
    int randkey = rand();
    a.val = rand();
    shmvmap[randkey] = a;
    randkey = rand();
    shmvmap[randkey] = b;

    
    ////////////////////////普通共享内存数组/////////////////////////////////////


    int index = 0;
    BB* p = NULL;
    CShmObjPool<BB> myobj;

    myobj.CreateObjPool(1234, 1234, sizeof(BB), a_size + 1, 0, __FILE__, __LINE__);

    ENMShmStartMode starttype = myobj.GetStartFlag();
    if (starttype == ESSM_Resume)
    {
        printf("恢复内存\n");
    }
    else
    {
        printf("第一次使用内存\n");
    }



    int iCount = rand() % 2 + 2;
    for (int i = 0; i < iCount; i++)
    {
        p = myobj.GetFreeObj(0, &index);
        if (p)
        {
            if (p->val != 0)
            {
                printf("===========出错=======\n");
            }
            p->val = rand();
        }
    }


    printf("使用数量：%d\n", myobj.GetUseNum());
    for (int i = 1; i <= a_size; i++)
    {
        BB* pa = myobj.GetObj(i, 0);
        if (pa)
        {
            printf("%d\n", pa->val);
        }
        else
        {
            printf("0\n");
        }
    }

    /* for (int i = 0; i < a_size;i++)
     {
         int index = -1;
         A* p = myobj.GetFreeObj(0, &index);
         if (p == NULL)
         {
             printf("共享内存分配完毕\n");
         }
         else
         {
             p->val = 125;
         }
     }*/


    printf("hello from TestShm!\n");
    return 0;
}