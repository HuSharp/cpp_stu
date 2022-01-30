/*
 * @Descripttion: 
 * @Author: HuSharp
 * @Date: 2022-01-30 11:16:05
 * @LastEditTime: 2022-01-30 12:21:19
 * @@Email: ihusharp@gmail.com
 */
#include <stdio.h>

class Obj {
public:
    Obj()   {   puts("Obj()");   };
    ~Obj()  {   puts("~Obj()");   };
};

void foo(int n) 
{
    Obj obj;
    if (n == 42)
        throw "life, the universe and everything";
}

int main(int argc, char const *argv[])
{
    try {
        foo(41);
        foo(42);
    } catch (const char *s) {
        puts(s);
    }
    return 0;
}

