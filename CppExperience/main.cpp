#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <queue>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iterator>
#include <string>
#include <regex>
#include <unistd.h>
using namespace std;

extern void IntelligentPointer();
extern void Iterator();
extern int TestLinuxList();
extern void TestLinuxShm();

class A
{
public:
	A();
	~A();
    int sadfsdf;

     virtual int fun() { return 1; }
private:

};

A::A()
{
}

A::~A()
{
}

class B :public A
{
public:
	B();
	~B();
    int sfdfdggedr;

    virtual int fun() override { return 3; }

private:

};

B::B()
{
}

B::~B()
{
}

void funsw()
{
    int* p = new int[100];
}

void macc_fun()
{
    int* p = (int*)malloc(1469);
}

int main()
{
    IntelligentPointer();
    funsw();
    macc_fun();
    sleep(2);

    //printf("测试utf-8\n");

    //TestLinuxList();
    //TestLinuxShm();

    //IntelligentPointer();
    //Iterator();


    printf("-----------程序结束------------\n");

    return 0;
}