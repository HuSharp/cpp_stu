/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2022-01-14 22:09:39
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-14 23:57:42
 * @@Email: 8211180515@csu.edu.cn
 */
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

int main_cout(int argc, char const *argv[])
{
    std::vector<int> myvector;
    for (int i = 0; i < 10; i++) {
        myvector.push_back(i*10);
    }

    std::ostream_iterator<int> out_it(std::cout, ",");
    std::copy(myvector.begin(), myvector.end(), out_it);
    
    return 0;
}


int main(int argc, char const *argv[])
{
    double value1, value2;
    std::cout << "Please insert two values: ";
    std::istream_iterator<double> eos;		// end of stream
    std::istream_iterator<double> iit(std::cin);	// stdin iterator
    if (iit != eos) {
        value1 = *iit;
    }
    ++iit;
    if (iit != eos) {
        value2 = *iit;
    }

    std::cout << value1 << "*" << value2 << "=" << (value1 * value2) << "\n";
    return 0;
}
