/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-19 20:55:55
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-29 19:15:05
 * @@Email: 8211180515@csu.edu.cn
 */

#include <stdlib.h>
#include <iostream>
#include <vector>
#include "util.cpp"
#include <algorithm>

using namespace std;

namespace test_vector
{
    void test_vector() {
        cout << "\ntest_vector()---------------------\n";
        vector<string> c;   // 第二个参数必须写 定义长度
        char buf[10];

        clock_t time_start = clock(); 
        for (long i = 0; i < ASIZE; i++)
        {
            try
            {
                sprintf(buf, "%d", rand());
                c.push_back(string(buf));
            }
            catch(exception& e)
            {
                cout << "i=" << i << " " << e.what() << endl;
                abort();
            }
            
        }

        cout << "milli-seconds : " << (clock() - time_start) << endl;	//
		cout << "vector.size()= " << c.size() << endl;		
		cout << "vector.front()= " << c.front() << endl;	
		cout << "vector.back()= " << c.back() << endl;	
		cout << "vector.data()= " << c.data() << endl;	
        cout << "vector.capacity()= " << c.capacity() << endl;

        string target = get_a_target_string();

        time_start = clock();
        auto pItem = find(c.begin(), c.end(), target);
        cout << "find(), milli-seconds : " << (clock()-time_start) << endl;    
        if (pItem != c.end())
			cout << "found, " << *pItem << endl;
		else
			cout << "not found! " << endl;


// cout << "\ntest_vector() 2---------------------\n";


        time_start = clock();
        sort(c.begin(), c.end());
        string* pItem2 = (string*)bsearch(&target, (c.data()), ASIZE, sizeof(string), compareStrings);
        cout << "qsort()+bsearch(), milli-seconds : " << (clock()-time_start) << endl;    
        if (pItem2 != NULL)
			cout << "found, " << *pItem2 << endl;
		else
			cout << "not found! " << endl;
    }
    
} // namespace test_vector
