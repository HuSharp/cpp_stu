/*
 * @Descripttion: 
 * @Author: HuSharp
 * @Date: 2022-01-30 13:46:56
 * @LastEditTime: 2022-01-30 18:05:10
 * @@Email: ihusharp@gmail.com
 */
#include <memory>

template <typename T>
class smart_ptr {
public:
    explicit smart_ptr(T* ptr = nullptr)
        : ptr_(ptr) {}
    ~smart_ptr() {
        delete ptr_;
    }

    // ************************************** // unique ptr 实际上是没有 swap 以下语义的
    // 拷贝时转移指针的所有权   
    template <typename U>    
    smart_ptr(smart_ptr<U>& other) {
        ptr_ = other.release();
    }
    smart_ptr& operator=(smart_ptr& rhs) {
        smart_ptr(rhs).swap(*this);
        return *this;
    }
    // ************************************** 

    // move
    // 此处 typename 是为了实现子类到基类的转换
    template <typename U>
    smart_ptr(smart_ptr<U>&& other) {
        ptr_ = other.release();
    }
    smart_ptr& operator=(smart_ptr&& rhs) {
        rhs.swap(*this);
        return *this;
    }

    // 浅拷贝 release 来手工释放所有权
    T* release() {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    void swap(smart_ptr& rhs) {
        using std::swap;
        swap(ptr_, rhs.ptr_);
    }


    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    operator bool() const { return ptr_; } 
    T* get() const { return ptr_; }

private:
    T* ptr_;
};


int main(int argc, char const *argv[])
{
    smart_ptr<int> ptr1;
    smart_ptr<int> ptr2{ptr1}; // 编译出错
    smart_ptr<int> ptr3;
    ptr3 = ptr1;                            // 编译出错
    ptr3 = std::move(ptr1);                 // OK，可以
    smart_ptr<int> ptr4{std::move(ptr3)}; // OK，可以
    return 0;
}
