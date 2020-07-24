#include <cstdio>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <queue>
#include <iostream>
#include <memory>
using namespace std;

struct Data
{
	int userid;
	int money;
	Data()
	{
		userid = 0;
		money = 0;
	}
	Data(int a, int b)
	{
		userid = a;
		money = b;
	}

	bool operator < (const Data& b) const
	{
		if (this->userid < b.userid)
		{
			return true;
		}

		if (this->userid == b.userid && this->money < b.money)
		{
			return true;
		}

		return false;
	}
};

// 测试vector遍历删除的正确方法
void stl_func1()
{
	int arr[] = { 0,1,2,3,4,5,6,7,8,9,10 };
	vector<int> a(arr, arr + sizeof(arr) / sizeof(*arr));
	for (auto it = a.begin(); it != a.end(); )
	{
		//printf("1->%d\n", *it);
		if ((*it) % 2 == 1) {
			it = a.erase(it);
			//printf("2->%d\n", *itr1);
		}
		else
		{
			++it;
		}
		//printf("3->%d\n", *it);
	}

	for (auto it = a.begin(); it != a.end(); ++it) {
		printf("%d\n", *it);
	}
}

// 测试vector迭代器失效
void stl_func2()
{
	int arr[] = { 0,1,2,3,4,5,6,7,8,9,10 };
	vector<int> a(arr, arr + sizeof(arr) / sizeof(*arr));
	for (auto it = a.begin(); it != a.end(); ++it)
	{
		printf("%d\n", *it);
		if ((*it) % 2 == 1) {
			a.erase(it);
		}
		printf("%d\n", *it);
	}

	for (auto it = a.begin(); it != a.end(); ++it) {
		printf("%d\n", *it);
	}
}

// 测试vector和map，list删除的时候，end()迭代器
void stl_func3()
{
	map < int, int > a;
	a[0] = 0; a[1] = 1; a[2] = 2; a[3] = 3; a[4] = 4; a[5] = 5; a[6] = 6; a[7] = 7;

	for (auto it = a.begin(); it != a.end(); )
	{
		//printf("1->%d\n", *it);
		if ((it->second) % 2 == 1) {
			it = a.erase(it);
			//printf("2->%d\n", *itr1);
		}
		else
		{
			++it;
		}
		//printf("3->%d\n", *it);
	}

	for (auto it = a.begin(); it != a.end(); ++it) {
		printf("%d\n", *it);
	}
}

void Iterator()
{
	vector<int> vec;
	list<Data> lis;
	map<int, Data> mym;
	set<Data> se;


	map<int, vector<int>::iterator > itermap1;
	map<int, list<Data>::iterator > itermap2;
	map<int, map<int, Data>::iterator > itermap3;
	map<int, set<Data>::iterator > itermap4;


	// vector迭代器失效
	printf("============vector迭代器============\n");
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	vec.push_back(4);
	for (auto iter = vec.begin();iter != vec.end();iter++)
	{
		itermap1[*iter] = iter;
	}
	for (auto iter = vec.begin();iter != vec.end();iter++)
	{
		if (*iter == 2)
		{
			vec.erase(iter);
			break;
		}
	}
	for (auto iter = itermap1.begin();iter != itermap1.end();iter++)
	{
		printf("%d\n",*(iter->second));
	}


	// list迭代器
	printf("============list迭代器============\n");
	Data data;
	data.userid = 1;data.money = 1000;
	lis.push_back(data);
	data.userid = 2;data.money = 121312300;
	lis.push_back(data);
	data.userid = 3;data.money = 1000213;
	lis.push_back(data);
	data.userid = 4;data.money = 1123123000;
	lis.push_back(data);
	for (auto iter = lis.begin();iter != lis.end();iter++)
	{
		itermap2[iter->userid] = iter;
	}
	for (auto iter = lis.begin();iter != lis.end();iter++)
	{
		if (iter->userid == 2)
		{
			lis.erase(iter);
			break;
		}
	}
	lis.emplace_back(345, 66333);
	lis.emplace_back(885885, 638585833);
	for (auto iter = itermap2.begin();iter != itermap2.end();iter++)
	{
		printf("%d\n", iter->second->money);
	}



	// map迭代器
	printf("============map迭代器============\n");
	data.userid = 1;data.money = 1000;
	mym.insert(make_pair(data.money, data));
	data.userid = 2;data.money = 121312300;
	mym.insert(make_pair(data.money, data));
	data.userid = 3;data.money = 1000213;
	mym.insert(make_pair(data.money, data));
	data.userid = 4;data.money = 1123123000;
	mym.insert(make_pair(data.money, data));
	for (auto iter = mym.begin();iter != mym.end();iter++)
	{
		itermap3[iter->second.userid] = iter;
	}
	for (auto iter = mym.begin();iter != mym.end();iter++)
	{
		if (iter->first == 121312300)
		{
			mym.erase(iter);
			break;
		}
	}
	for (auto iter = itermap3.begin();iter != itermap3.end();iter++)
	{
		printf("%d\n", iter->second->second.money);
	}

	mym.clear();

	printf("%d-%d\n", itermap3[3]->second.userid, itermap3[3]->second.money);

	data.userid = 14555;data.money = 1055500;
	mym.emplace(data.money, data);
	data.userid = 1444555;data.money = 1055544400;
	mym.emplace(data.money, data);
	data.userid = 1000;data.money = 10000;
	mym.emplace(data.money, data);
	for (auto iter = itermap3.begin();iter != itermap3.end();iter++)
	{
		printf("%d\n", iter->second->second.money);
	}



	printf("============set迭代器============\n");
	data.userid = 1;data.money = 1000;
	se.insert(data);
	data.userid = 2;data.money = 121312300;
	se.insert(data);
	data.userid = 33;data.money = 1000213;
	se.insert(data);
	data.userid = 4;data.money = 1000006213;
	se.insert(data);
	for (auto iter = se.begin();iter != se.end();iter++)
	{
		itermap4[iter->userid] = iter;
	}
	data.userid = 33;data.money = 1000213;
	se.erase(data);
	
	data.userid = 133;data.money = 100330;
	se.insert(data);

	data.userid = 122;data.money = 1333000;
	se.insert(data);






	printf("end\n");
}