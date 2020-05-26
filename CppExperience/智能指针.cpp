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

//////////////shared_ptr解决删除数组问题//////////////////
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

////////////shared_ptr问题1：裸指针管理问题////////////////////
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


///////////////shared_ptr问题2：循环引用问题/////////////////
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
	shared_ptr<Test> pTest = nullptr;
	//cout << pTest.use_count() << " " << pTest.get() << " " << pTest.unique()<<endl;

	shared_ptr<A> sptrA(new A);
	//printf("%ld\n", sptrA.use_count()); // 1
	//printf("%ld\n", sptrA->m_sptrB.use_count()); // 0


	shared_ptr<B> sptrB(new B);
	//printf("%ld\n", sptrB.use_count()); // 1
	//printf("%ld\n", sptrB->m_sptrA.use_count()); // 0


	sptrB->m_sptrA = sptrA;
	printf("%ld\n", sptrB.use_count()); // 1
	printf("%ld\n", sptrB->m_sptrA.use_count()); // 2
	printf("%ld\n", sptrA.use_count()); // 2
	printf("%ld\n", sptrA->m_sptrB.use_count()); // 0

	cout << sptrB->m_sptrA.get() << " " << sptrA.get() << endl;


	sptrA->m_sptrB = sptrB;
	printf("%ld\n", sptrB.use_count()); // 2
	printf("%ld\n", sptrB->m_sptrA.use_count()); // 2
	printf("%ld\n", sptrA.use_count());// 2
	printf("%ld\n", sptrA->m_sptrB.use_count()); // 2
}
////////////////////////////////

////////////weak_ptr////////////////////
void func12()
{
	shared_ptr<Test> sptr(new Test);
	weak_ptr<Test> wptr(sptr);
	weak_ptr<Test> wptr1 = wptr;
	cout << wptr.use_count() << endl;
	
	// 有效资源
	if (wptr.expired())
	{
		cout << "测试weak_ptr" << endl;
	}

	auto pTest = wptr.lock();
	cout << sptr.get() << " " << pTest << endl;

	printf("%ld\n", sptr.use_count()); // 1
	
}
////////////////////////////////


////////////weak_ptr解决循环引用问题////////////////////
class BB;
class AA
{
public:
	AA() { valAA = 123; };
	~AA()
	{
		cout << " AA is destroyed" << endl;
	}
	int valAA;
	weak_ptr<BB> m_sptrB;
};
class BB
{
public:
	BB() { valBB = 321; };
	~BB()
	{
		cout << " BB is destroyed" << endl;
	}
	int valBB;
	weak_ptr<AA> m_sptrA;
};
void func13()
{
	shared_ptr<AA> sptrA(new AA);
	printf("%ld\n", sptrA.use_count()); // 1
	printf("%ld\n", sptrA->m_sptrB.use_count()); // 0
	sptrA->valAA = 122555555;
	if (!sptrA->m_sptrB.expired())
	{
		cout << sptrA->m_sptrB.lock()->valBB << endl;
	}

	shared_ptr<BB> sptrB(new BB);
	printf("%ld\n", sptrB.use_count()); // 1
	printf("%ld\n", sptrB->m_sptrA.use_count()); // 0
	sptrB->valBB = 644646;
	if (!sptrB->m_sptrA.expired())
	{
		cout << sptrB->m_sptrA.lock()->valAA << endl;
	}


	sptrB->m_sptrA = sptrA;
	printf("%ld\n", sptrB.use_count()); // 1
	printf("%ld\n", sptrB->m_sptrA.use_count()); // 2
	printf("%ld\n", sptrA.use_count()); // 2
	printf("%ld\n", sptrA->m_sptrB.use_count()); // 0
	if (!sptrB->m_sptrA.expired())
	{
		cout << sptrB->m_sptrA.lock()->valAA << endl;
	}

	sptrA->m_sptrB = sptrB;
	printf("%ld\n", sptrB.use_count()); // 2
	printf("%ld\n", sptrB->m_sptrA.use_count()); // 2
	printf("%ld\n", sptrA.use_count());// 2
	printf("%ld\n", sptrA->m_sptrB.use_count()); // 2
	if (!sptrA->m_sptrB.expired())
	{
		cout << sptrA->m_sptrB.lock()->valBB << endl;
	}
}
////////////////////////////////

////////////unique_ptr////////////////////
void func14()
{
	unique_ptr<Test> uptr1(new Test);
	unique_ptr<Test[]> uptr2(new Test[3]);
}
////////////////////////////////

////////////unique_ptr////////////////////
void unifunc(unique_ptr<Test> uptr1)
{
	cout << uptr1->m_a << endl;
	cout << "sss" << endl;
}
void func15()
{
	unique_ptr<Test> uptr1(new Test);
	uptr1->m_a = 12345;
	unifunc(move(uptr1));
	cout << uptr1.get() << endl;
	cout << "sss" << endl;
}
////////////////////////////////

void IntelligentPointer()
{
	//func11();
	//func12();
	//func13();
	//func14();
	func15();

	printf("-----------智能指针测试结束------------\n");
}