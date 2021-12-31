/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-30 21:16:11
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-30 21:43:56
 * @@Email: 8211180515@csu.edu.cn
 */
template <typename T>
class complex
{
private:
    T re, im;
public:
    complex (T r=0, T i=0) : re(r), im(i){}
    complex& operator += (const complex&);
    T real() const {return re;}
    T imag() const {return im;}
};

template <typename T>
complex::complex(/* args */)
{
}

template <typename T>
complex::~complex()
{
}


int main(int argc, char const *argv[])
{
    complex<double> c1(1.5, 2.5);
    return 0;
}

template <class T>
inline const T& min(const T& a, const T& xb) {
    return b < a ? b : a;
}

