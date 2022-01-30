/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-18 23:14:05
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-14 22:02:09
 * @@Email: 8211180515@csu.edu.cn
 */
const int a = 100;

#include <iostream>
using namespace std;

class Apple
{
private:
    int people[100];
public:
    Apple(int i); 
    const int apple_number;
    void take(int num) const;
    int add(int num);
    int add(int num) const;
    int getCount() const;

};


Apple::Apple(int i):apple_number(i)
{

}
int Apple::add(int num){
    take(num);
    cout << "add func " << num << endl;
}
int Apple::add(int num) const{
    take(num);
    cout << "add func const " << num << endl;
}
// 证明了非 const 对象可以访问任意的成员函数,包括 const 成员函数。
// 但是当成员函数的 const 版本和 非 const 版本同时存在时， 
// const 对象只能调用const 版本， non-const 对象只能调用 non-const 版本
void Apple::take(int num) const
{
    cout << "take func const " << num << endl;
}
int Apple::getCount() const
{
    take(1);
    add(0); // const 对象默认调用 const 成员函数。
    return apple_number;
}
int main(){
    Apple a(2);
    cout << a.getCount() << endl;
    a.add(10);
    const Apple b(3);
    b.add(100);
    return 0;
}