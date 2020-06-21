#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <queue>
#include <iostream>
#include <memory>
using namespace std;

extern void IntelligentPointer();
extern void Iterator();
extern int TestLinuxList();
extern void TestLinuxShm();

int main()
{
    printf("测试utf-8\n");

    //TestLinuxList();
    TestLinuxShm();

    //IntelligentPointer();
    //Iterator();


    printf("-----------程序结束------------\n");

    return 0;
}