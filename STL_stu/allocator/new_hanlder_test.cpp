/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-31 23:05:24
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-31 23:12:53
 * @@Email: 8211180515@csu.edu.cn
 */
#include <new>
#include <iostream>
#include <cassert>
using namespace std;

void noMoreMemory() {
    cerr << "out of memory" << endl;
    abort();
}


int main(int argc, char const *argv[])
{
    set_new_handler(noMoreMemory);
    int *p = new int[1000000000000];
    assert(p);
    return 0;
}
