/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-31 15:39:20
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-31 21:06:35
 * @@Email: 8211180515@csu.edu.cn
 */
#include <cstddef>
#include <iostream>
#include <new>
using namespace std;

class Screen
{
private:
    int i;
    double b;
public:
    Screen(int x) : i(x) {};
    int get() {return i;}


    void* operator new(size_t);
    void operator delete(void *p, size_t);

private:
    Screen* next;
    static Screen* freeStore;
    static const int screenChunk;
};

Screen* Screen::freeStore = 0;
const int Screen::screenChunk = 24;

void* Screen::operator new(size_t size) {
    Screen *p;
    if (!freeStore) {
        size_t chunk = screenChunk * size;
        freeStore = p = reinterpret_cast<Screen*>(new char[chunk]);
        // 切片
        for (; p != &freeStore[screenChunk-1]; p++)
        {
            p->next = p + 1;
        }
        p->next = 0;
    }

    p = freeStore;
    freeStore = freeStore->next;
    return p;
}

void Screen::operator delete(void *p, size_t) {
    // cout << "call no global" << endl;
    // 将 delete object 插回 free list 前端
    (static_cast<Screen*>(p))->next = freeStore;
    freeStore = static_cast<Screen*>(p);


}

#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
    cout << sizeof(Screen) << endl; // 8

    size_t const N = 100;
    Screen* p[N];

    for (int i = 0; i < N; i++)
    {
        p[i] = new Screen(i);
    }


    cout << "------------locate------------" << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << p[i] << endl;
    }

    cout << "------------delete------------" << endl;
    for (int i = 0; i < N; i++)
    {
        delete p[i];
    }

    return 0;
}
