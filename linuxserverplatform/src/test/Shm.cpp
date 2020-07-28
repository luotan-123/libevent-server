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

int TestShm()
{
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