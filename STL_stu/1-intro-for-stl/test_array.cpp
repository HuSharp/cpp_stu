/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-19 18:48:09
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-29 19:49:20
 * @@Email: 8211180515@csu.edu.cn
 */
#include <stdlib.h>
#include <iostream>
#include <array>
#include "util.cpp"

using namespace std;


namespace test_array
{
    void test_array() {
        cout << "\ntest_array()---------------------\n";
        array<long, ASIZE> c;   // 第二个参数必须写 定义长度

        clock_t time_start = clock(); 
        for (long i = 0; i < ASIZE; i++)
        {
            c[i] = rand();
        }

        cout << "milli-seconds : " << (clock() - time_start) << endl;	//
		cout << "array.size()= " << c.size() << endl;		
		cout << "array.front()= " << c.front() << endl;	
		cout << "array.back()= " << c.back() << endl;	
		cout << "array.data()= " << c.data() << endl;	
        

        long target = get_a_target_long();

        time_start = clock();
        qsort(c.data(), ASIZE, sizeof(long), compareLongs);// 先排序
        long* pItem = (long*)bsearch(&target, (c.data()), ASIZE, sizeof(long), compareLongs);
        
        cout << "qsort()+bsearch(), milli-seconds : " << (clock()-time_start) << endl;    
        if (pItem != NULL)
			cout << "found, " << *pItem << endl;
		else
			cout << "not found! " << endl;
    }
    
} // namespace test_array




