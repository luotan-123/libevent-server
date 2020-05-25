#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <queue>
#include <iostream>
#include <memory>
using namespace std;

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

}