/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-19 21:49:50
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-02 22:06:17
 * @@Email: 8211180515@csu.edu.cn
 */
#include <stdlib.h>
#include <iostream>
#include <list>
#include "util.cpp"
#include <algorithm>

using namespace std;

namespace test_list
{
    void test_list() {
        cout << "\ntest_list()---------------------\n";
        list<string> c;   // 第二个参数必须写 定义长度
        char buf[10];

        clock_t time_start = clock(); 
        for (long i = 0; i < ASIZE; i++)
        {
            try
            {
                sprintf(buf, "%d", rand());
                c.push_back(string(buf));
                // c.push_front(string(buf))
            }
            catch(exception& e)
            {
                cout << "i=" << i << " " << e.what() << endl;
                abort();
            }
            
        }

        cout << "milli-seconds : " << (clock() - time_start) << endl;	//
		cout << "list.size()= " << c.size() << endl;		
		cout << "list.front()= " << c.front() << endl;	
		cout << "list.back()= " << c.back() << endl;	
        cout << "list.max_size()= " << c.max_size() << endl;

        string target = get_a_target_string();

        time_start = clock();
        auto pItem = find(c.begin(), c.end(), target);
        cout << "find(), milli-seconds : " << (clock()-time_start) << endl;    
        if (pItem != c.end())
			cout << "found, " << *pItem << endl;
		else
			cout << "not found! " << endl;


        auto pItem2 = find(c.begin(), c.end(), target);
        cout << "find(), milli-seconds : " << (clock()-time_start) << endl;    
        if (pItem != c.end())
			cout << "found, " << *pItem2 << endl;
		else
			cout << "not found! " << endl;

        time_start = clock();
        c.sort();
        cout << "sort(), milli-seconds : " << (clock()-time_start) << endl; 
    }

} // namespace test_list


#include <forward_list>
namespace test_forward_list
{
    void test_forward_list() {
        // just notice only have push_front
        forward_list<string> c;
        c.push_front("aaa");
        cout << "list.size()= " << c.max_size() << endl;	
    }
} // namespace test_forward_list


#include <deque>
#include<stack>
namespace test_deque
{
    void test_queue() {
        // 同样类似 list
        deque<string> c;
        c.push_front("aaa");
        cout << "list.size()= " << c.max_size() << endl;	

        sort(c.begin(), c.end());

        stack<int, list<int>> c2;
        c2.push(3);

        
    }    
} // namespace test_forward_list

