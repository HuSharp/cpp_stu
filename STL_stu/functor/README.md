仿函数只为算法服务。仿函数/函数对象：行为类似函数，可作为算法的某种策略，从实现的角度来看，仿函数是一种重载了 operator() 的 **class 或 class template**。

要模仿函数，所以必须要重载()

> ()是function call operator

按照功能划分为以下三种仿函数。

算数类，逻辑运算类，相对关系类。



### 以操作数的个数划分，可分为一元和二元仿函数

STL 规定仿函数应该有能力被函数配接器(function adapter)修饰，为了拥有配接能力，每一个仿函数必须去继承下面的两种类型之一。

* `unary_function`

`unary_function` 用来呈现一元函数的参数类型和返回值类型。

```cpp
// 一元函数的参数类型和返回值类型
template <class _Arg, class _Result>
struct unary_function {
  typedef _Arg argument_type;
  typedef _Result result_type;
}; 
```

* `binary_function`

`binary_function` 用来呈现二元函数的第一参数类型、第二参数类型，以及返回值类型。

```cpp
// 二元函数的第一个参数类型和第二个参数类型，以及返回值类型
template <class _Arg1, class _Arg2, class _Result>
struct binary_function {
  typedef _Arg1 first_argument_type;
  typedef _Arg2 second_argument_type;
  typedef _Result result_type;
};
```

### 

### 举个栗子：

应用到 sort 中的仿函数继承 binary_function 才能融入到  STL 中。

```cpp
struct myclass{
	bool operator(int i, int j){return i<j;};
}myobj;
```

myclass 没有继承 binary_function<T, T, bool>，那么 myclass 就没有融入STL体系里面。
