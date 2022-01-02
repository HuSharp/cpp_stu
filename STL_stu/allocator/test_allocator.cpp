/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-31 11:21:52
 * @LastEditors: HuSharp
 * @LastEditTime: 2022-01-01 09:54:25
 * @@Email: 8211180515@csu.edu.cn
 */
#include <memory>
#include <iostream>
using namespace std;

// #define DECLARE_POOL_ALLOC()    \
// public: \
//     void* operator new(size_t size);    \
//     void operator delete(void* pdead, size_t size); \
// protected:  \
//     static allocator myAlloc;

// #define IMPLEMENT_POOL_ALLOC(class_name)    \
// allocator class_name::myAlloc;


// class GOO
// {
// DECLARE_POOL_ALLOC()
// private:
//     ...
// public:
//     GOO(/* args */);
//     ~GOO();
// };
// IMPLEMENT_POOL_ALLOC()

class Foo
{
public:
    static allocator myAlloc;

public:
    static void* operator new(size_t size) {
        return myAlloc.allocate(size);
    }
    static void operator delete(void* pdead, size_t size) {
        return myAlloc.deallocate(pdead, size);
    }
};

Foo::Foo(/* args */)
{
}

Foo::~Foo()
{
}



int a(int p, int c=2) {
    return p + c;
}

int main(int argc, char const *argv[])
{
    
}

