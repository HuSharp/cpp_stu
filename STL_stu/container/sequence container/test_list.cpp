/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-01 10:22:14
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-01 12:18:05
 * @@Email: 8211180515@csu.edu.cn
 */

#include <stdlib.h>
#include <iostream>
#include <list>

using namespace std;


void test(list<int> &data) {
    data.push_back(3);
}

int main(int argc, char const *argv[])
{
    list<int> data;
    for (int i = 0; i < 10; i++)
    {
        data.push_back(i)
    }
    
        test(data);
}
