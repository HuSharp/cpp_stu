/*
 * @Descripttion: 
 * @Author: HuSharp
 * @Date: 2022-01-30 13:46:56
 * @LastEditTime: 2022-01-30 20:29:08
 * @@Email: ihusharp@gmail.com
 */
#include <memory>
#include "shape.h"

class shared_count
{
public:
    shared_count() : count_(1) {}
    void add_cnt() {
        ++count_;
    }
    long reduce_cnt() {
        return --count_;
    }
    long get_cnt() const {
        return count_;
    }
private:
    long count_;
};


template <typename T>
class smart_ptr {
public:
    template <typename U>
    friend class smart_ptr;
    explicit smart_ptr(T* ptr = nullptr)
        : ptr_(ptr) {
        if (ptr) {
            shared_count_ = new shared_count();
        }
    }
    ~smart_ptr() {
        printf("~smart_ptr(): %p\n", this);
        if (ptr_ && !shared_count_->reduce_cnt()) {
            delete ptr_;
            delete shared_count_;
        }
    }

    // ************************************** //
    // 拷贝时转移指针的所有权     
    template <typename U>
    smart_ptr(smart_ptr<U>& other) {
        ptr_ = other.ptr_;
        printf("smart_ptr(smart_ptr& other): %p, ptr: %p\n", this, ptr_);
        if (ptr_) {
            other.shared_count_->add_cnt();
            shared_count_ = other.shared_count_;
        }
    }
    smart_ptr& operator=(smart_ptr& rhs) {
        printf("operator=(smart_ptr& rhs): %p, ptr: %p\n", this, ptr_);
        smart_ptr(rhs).swap(*this);
        return *this;
    }
    // ************************************** 

    // 指针类型强制转换
    template<typename U>
    smart_ptr(const smart_ptr<U>& other, T* ptr) {
        printf("smart_ptr(const smart_ptr<U>& other, T* ptr): %p, ptr: %p\n", this, ptr_);
        ptr_ = ptr;
        if (ptr_) {
            other.shared_count_->add_cnt();
            shared_count_ = other.shared_count_;
        }
    }

    // move
    // 此处 typename 是为了实现子类到基类的转换
    template <typename U>
    smart_ptr(smart_ptr<U>&& other) {
        printf("smart_ptr(smart_ptr&& other): %p, ptr: %p\n", this, ptr_);
        ptr_ = other.ptr_;
        if (ptr_) {
            shared_count_ = other.shared_count_;
            other.ptr_ = nullptr;
        }
    }
    smart_ptr& operator=(smart_ptr&& rhs) {
        printf("operator=(smart_ptr&& rhs): %p, ptr: %p\n", this, ptr_);
        rhs.swap(*this);
        return *this;
    }

    void swap(smart_ptr& rhs) {
        using std::swap;
        swap(ptr_, rhs.ptr_);
        swap(shared_count_, rhs.shared_count_);
    }


    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    operator bool() const { return ptr_; } 
    T* get() const { return ptr_; }

    long use_count() const {
        if (ptr_) {
            return shared_count_->get_cnt();
        } else {
            return 0;
        }
    }

private:
    T* ptr_;
    shared_count* shared_count_;
};

template<typename T, typename U>
smart_ptr<T> dynamic_pointer_cast(const smart_ptr<U>& other) {
    T* ptr = dynamic_cast<T*>(other.get());
    return smart_ptr<T>(other, ptr);
}

int main(int argc, char const *argv[])
{
    smart_ptr<circle> ptr1(new circle());
    printf("use count of ptr1 is %ld\n", ptr1.use_count());
    smart_ptr<shape> ptr2;
    printf("use count of ptr2 is %ld\n", ptr2.use_count());
    ptr2 = ptr1;
    printf("use count of ptr2 is now %ld\n", ptr2.use_count());
    if (ptr1) {
        puts("ptr1 is not empty");
    }

    smart_ptr<shape> ptr3 = dynamic_pointer_cast<shape>(ptr2);
    printf("use count of ptr3 is %ld\n", ptr3.use_count());
    return 0;
}

