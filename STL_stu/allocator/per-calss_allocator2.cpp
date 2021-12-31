/*
 * @Descripttion: 
 * @version: 
 * @Author: HuSharp
 * @Date: 2021-12-31 21:08:05
 * @LastEditors: HuSharp
 * @LastEditTime: 2021-12-31 22:03:43
 * @@Email: 8211180515@csu.edu.cn
 */
#include <iostream>
using namespace std;

class Airplane
{
private:
    struct AirplaneRep 
    {
        unsigned long miles;
        char type;
    };
    
private:
    union   
    {
        AirplaneRep rep;    // 针对使用中的 obj
        Airplane* next; // 针对 free list 的 obj, 利用位对齐
    };
    

public:
    unsigned long getMiles() {  return rep.miles;   }
    char getType() {    return rep.type;    }
    void set(unsigned long m, char t) {
        rep.miles = m, rep.type = t;
    }

public:
    static void* operator new(size_t size);
    static void operator delete(void* deadObject, size_t size);

private:
    static const int BLOCK_SIZE;
    static Airplane* headOfFreeList;
};

Airplane* Airplane::headOfFreeList;
const int Airplane::BLOCK_SIZE = 512;


void* Airplane::operator new(size_t size) {
    Airplane* p = headOfFreeList;
    if (p) {
        headOfFreeList = p->next;
    } else {
        // free list is null, apply for malloc
        Airplane* newBlock = static_cast<Airplane*>(::operator new(BLOCK_SIZE * sizeof(Airplane)));

        for (int i = 1; i < BLOCK_SIZE-1; i++)
        {
            newBlock[i].next = &newBlock[i+1];
        }
        newBlock[BLOCK_SIZE - 1].next = 0;
        p = newBlock;
        headOfFreeList = &newBlock[1];
    }

    return p;
}

// 并没有还给 os, 但也没有发生内存泄漏
void Airplane::operator delete(void* deadObject, size_t size) {
    if (deadObject == 0) {
        return;
    }
    if (size != sizeof(Airplane)) {
        ::operator delete(deadObject);
        return;
    }

    Airplane* carcass = static_cast<Airplane*>(deadObject);
    carcass->next = headOfFreeList;
    headOfFreeList = carcass;
}

int main(int argc, char const *argv[])
{
    cout << sizeof(Airplane) << endl;
    size_t const N = 100;
    Airplane* p[N];


    for (int i = 0; i < N; i++)
    {
        p[i] = new Airplane;
    }
    p[1]->set(1000, 'a');
    p[7]->set(2000, 'b');
    p[9]->set(500000, 'b');
    
    for (int i = 0; i < 10; i++)
    {
        cout << p[i] << " " << p[i]->getMiles() << p[i]->getType() << endl;
    }

    for (int i = 0; i < N; i++)
    {
        delete p[i];
    }

    return 0;
}
