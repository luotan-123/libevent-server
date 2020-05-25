#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <queue>
#include <iostream>
#include <memory>
using namespace std;

class Test
{
public:
	Test(int a = 0) : m_a(a) { }
	~Test()
	{
		cout << "Test析构函数" << endl;
	}
public: 
	int m_a;
};

void func1()
{
	std::auto_ptr<Test> p(new Test(5));
	cout << p->m_a << endl;
}

////////////////////////////////
void func2()
{
	int a = 0, b = 5, c;
	if (a == 0)
	{
		throw "Invalid divisor";
	}
	c = b / a;
	return;
}
void func3()
{
	try
	{
		std::auto_ptr<Test> p(new Test(5));
		func2();
		cout << p->m_a << endl;
	}
	catch (...)
	{
		cout << "Something has gone wrong" << endl;
	}
}
////////////////////////////////

////////////////////////////////
void func3(auto_ptr<Test>& p1)
{
	cout << p1->m_a << endl;
}
void func4()
{
	std::auto_ptr<Test> p(new Test(5));
	func3(p);
	cout << p->m_a << endl;
}
////////////////////////////////


////////////////////////////////
void func5()
{
	std::auto_ptr<Test> p(new Test[5]);
}
////////////////////////////////


////////////////////////////////
void func6(shared_ptr<Test> sptr1)
{
	cout << sptr1->m_a << endl;
	printf("%ld\n", sptr1.use_count());
}
void func7()
{
	Test* pTest = new Test[10];
	shared_ptr<Test> sptr2(pTest);

	//shared_ptr<Test> sptr1(new Test);
	shared_ptr<Test> sptr1 = make_shared<Test>(3211);
	func6(sptr1);


	printf("%d\n", sptr1->m_a);
	printf("%ld\n", sptr1.use_count());
}
////////////////////////////////

////////////////////////////////
void func8()
{
	shared_ptr<Test> sptr1(new Test[5],
		[](Test* p) { delete[] p; });
	printf("%d\n", sptr1->m_a);

	(sptr1.get() + 2)->m_a = 90;
	printf("%d\n", (sptr1.get() + 2)->m_a);
	if (sptr1.unique())
	{

		printf("%d\n", (sptr1.get() + 6)->m_a);
	}

	printf("%d\n", sptr1->m_a);

	printf("%ld\n", sptr1.use_count());
}
////////////////////////////////


////////////////////////////////
void func9()
{
	shared_ptr<int> sptr1(new int);
	printf("%ld\n", sptr1.use_count());

	shared_ptr<int> sptr2 = sptr1;
	printf("%ld\n", sptr1.use_count());

	shared_ptr<int> sptr3;
	printf("%ld\n", sptr1.use_count());

	sptr3 = sptr1;
	printf("%ld\n", sptr1.use_count());
}
////////////////////////////////

////////////////////////////////
void delfunc(Test *p)
{
	shared_ptr<Test> sptr1(p);
}
void func10()
{
	Test* p = new Test;
	delfunc(p);
	delfunc(p);
}
////////////////////////////////


////////////////////////////////
class B;
class A
{
public:
	A() : m_sptrB(nullptr) { };
	~A()
	{
		cout << " A is destroyed" << endl;
	}
	shared_ptr<B> m_sptrB;
};
class B
{
public:
	B() : m_sptrA(nullptr) { };
	~B()
	{
		cout << " B is destroyed" << endl;
	}
	shared_ptr<A> m_sptrA;
};
void func11()
{
	shared_ptr<B> sptrB(new B);
	shared_ptr<A> sptrA(new A);
	sptrB->m_sptrA = sptrA;
	sptrA->m_sptrB = sptrB;
}
////////////////////////////////

void IntelligentPointer()
{
	func11();

	printf("-----------智能指针测试结束------------\n");
}