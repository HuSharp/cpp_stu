### 概念说明

算法：各种常用算法如 sort，search，copy，erase 等，从实现的角度来看，STL 算法是一种 function template。

所有泛型算法的前两个参数都是一对迭代器，STL 习惯使用前闭后开的区间，`[first, last)`。

> 且根据迭代器的行进特性，每个 STL 算法的声明都需要表现出其支持的最低规格的迭代器类型。

最后一个元素的下一位置，称为 end()。

数值的传递由 pass-by-value 改为 pass-by-reference，好处是，在模板中，参数的类型可以任意，当对象一大，传递成本便会上升，所以用 pass-by-reference 可以节省空间。

c++标准库的算法都是以下面的方式和容器做沟通

```cpp
template<typename Iterator>
std::Algorithm(Iterator it1, Iterator it2, ...)
{
	...
}
```

### 调用方式

一般的函数只要满足当然是可以调用的，

值得一提的是可以采用诸如下面代码的 function object，对 operator 进行重载。

```cpp
struct myclass {
	int operator()(int x, int y) {return x+3*y;}
} myobj;

accumulate(nums, nums+3, init, myobj);
```

展示代码

```cpp
int myfunc (int x, int y) {return x+2*y;}

struct myclass {
	int operator()(int x, int y) {return x+3*y;}
} myobj;

void test_accumulate()
{
  cout << "\ntest_accumulate().......... \n";	
  int init = 100;
  int nums[] = {10,20,30};

  cout << "using default accumulate: ";
  cout << accumulate(nums,nums+3,init);  //160
  cout << '\n';

  cout << "using functional's minus: ";
  cout << accumulate(nums, nums+3, init, minus<int>()); //40
  cout << '\n';

  cout << "using custom function: ";
  cout << accumulate(nums, nums+3, init, myfunc);	//220
  cout << '\n';

  cout << "using custom object: ";
  cout << accumulate(nums, nums+3, init, myobj);	//280
  cout << '\n';
}
```

