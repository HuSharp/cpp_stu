/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-19 22:41:55
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-19 23:07:01
 * @@Email: 8211180515@csu.edu.cn
 */
// associative container 包括  Set /  Multiset  Map/  Multimap  unordered containers

#include "util.cpp"
#include <set>
#include <iostream>
#include <algorithm>
using namespace std;

namespace test_multiset
{
	void test_multiset()
	{
		cout << "\ntest_multiset().......... \n";

		multiset<string> c;
		char buf[10];

		clock_t timeStart = clock();
		for (long i = 0; i < ASIZE; ++i)
		{
			try
			{
				sprintf(buf, "%d", rand());
				c.insert(string(buf)); //没有push_back或者push_front
			}
			catch (exception &p)
			{
				cout << "i=" << i << " " << p.what() << endl;
				abort();
			}
		}
		cout << "milli-seconds : " << (clock() - timeStart) << endl;
		cout << "stack.size()= " << c.size() << endl;
		cout << "stack.max_szie()= " << c.max_size() << endl;

		string target = get_a_target_string();
		{
			timeStart = clock();
			auto pItem = find(c.begin(), c.end(), target); //比c.find()慢很多
			cout << "find(), milli-seconds : " << (clock() - timeStart) << endl;
			if (pItem != c.end())
				cout << "found, " << *pItem << endl;
			else
				cout << "not found! " << endl;
		}

		{
			timeStart = clock();
			auto pItem = c.find(target); //比::find()快很多
			cout << "c.find(), milli-seconds : " << (clock() - timeStart) << endl;
			if (pItem != c.end())
				cout << "found, " << *pItem << endl;
			else
				cout << "not found! " << endl;
		}
	}

}

#include <map>
namespace test_multimap
{
	void test_multimap() {
		multimap<long, string> c;
		c.insert(pair<long, string>(1, "aaa"));

		map<long, string> aa;
		aa.insert({1, "aa"});


		clock_t timeStart = clock();	
		string target = get_a_target_string();
		auto pItem = find(c.begin(), c.end(), target); //比c.find()慢很多
		cout << "find(), milli-seconds : " << (clock() - timeStart) << endl;
		if (pItem != c.end())
			cout << "found, " << (*pItem).second << endl;
		else
			cout << "not found! " << endl;
	}
}