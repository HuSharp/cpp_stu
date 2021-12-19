/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-19 20:34:15
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-19 22:18:49
 * @@Email: 8211180515@csu.edu.cn
 */
#include <iostream>
using namespace std;

//vs2010,此处如果是500000L，会有overflow错误
const long ASIZE  =   1000000L;

long get_a_target_long() {
    long target = 0;
    
    cout << "target (0~" << RAND_MAX << "): ";
    cin >> target;
    return target;
}

string get_a_target_string() {
    long target = 0;
    char buf[10];
    
    cout << "target (0~" << RAND_MAX << "): ";
    cin >> target;
    // transform a string
    snprintf(buf, 10, "%ld", target);
    return string(buf);
}

// 
int compareLongs(const void* a, const void* b) {
    return (*(long*)a - *(long*)b);
}

int compareStrings(const void* a, const void* b) {
    if (*(string*)a > *(string*)b) {
        return 1;
    } else if (*(string*)a < *(string*)b) {
        return -1;
    } else {
        return 0;
    }
}