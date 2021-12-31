/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-23 12:37:50
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-23 12:43:44
 * @@Email: 8211180515@csu.edu.cn
 */
#include<iostream>
using namespace std;

int main(int argc, char const *argv[])
{
    int year;
    bool isLeapY;
    cout << "Enter the year:    ";
    cin >> year;

    isLeapY = ( ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0) );
    if(isLeapY)
    {
        cout<<year<<" is a leap year"<<endl;
    }
    else
    {
        cout<<year<<" is not a leap year"<<endl;
    }
    return 0;
}
