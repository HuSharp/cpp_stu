/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-19 11:40:55
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-19 17:16:52
 * @@Email: 8211180515@csu.edu.cn
 */
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

int main2(int argc, char const *argv[])
{
    int ia[7] = {27, 210, 12, 47, 109, 83, 40};
    // <>的第二个参数一般都不会写，用默认的分配器；分配器<>要和外层的第一个参数x一致 
    // end 元素指向 last 元素的下一个
    vector<int, allocator<int>> vi(ia, ia+7);        
    // 输出大于等于40的个数
    cout << count_if(vi.begin(), vi.end(), 
                not1(bind2nd(less<int>(), 40)))  << endl;
    
    
    return 0;
}


int main(int argc, char const *argv[])
{
    int ia[7] = {27, 210, 12, 47, 109, 83, 40};
    vector<int> c(ia, ia+7);

    for (auto elem : c)
    {
        cout << elem << endl;
    }

    for (auto &elem : c)
    {
        elem *= 3;
    }

    
    find(c.begin(), )


    vector<int>::iterator ite = c.begin();
    for (; ite != c.end(); ++ite)
    {
        cout << *ite << endl;
    }
    

    return 0;
}

