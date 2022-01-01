

## list

SGI STL 的 list 是一个双向链表，同时还是一个环状的双向链表；对于任何位置的元素插入或元素移除，list 永远是常数时间。

### list 的迭代器

list 的节点可以在存储空间中不连续，所以 list 的迭代器必须具备前移、后移的能力，list 提供是 Bidirectional iterator。

iterator要模拟指针，会有大量的运算符重载。5个typedef.

```cpp
// List 迭代器基类
struct _List_iterator_base {
  typedef size_t                     size_type;
  typedef ptrdiff_t                  difference_type;
  typedef bidirectional_iterator_tag iterator_category;  // 双向移动迭代器

  _List_node_base* _M_node; // 迭代器内部当然要有一个普通指针，指向 list 的节点

  _List_iterator_base(_List_node_base* __x) : _M_node(__x) {}
  _List_iterator_base() {}

  // 比较两个容器操作
  bool operator==(const _List_iterator_base& __x) const {
    return _M_node == __x._M_node;
  }
  bool operator!=(const _List_iterator_base& __x) const {
    return _M_node != __x._M_node;
  }
}; 
```

值得注意的是:

i++ 和 ++i 通过参数有无来区分.

```cpp
  // 迭代器先前进一个节点
  _Self& operator++() { 
      //   void _M_incr() { _M_node = _M_node->_M_next; }  // 前驱
    this->_M_incr();
    return *this;
  }
  // 迭代器后增
  _Self operator++(int) { 
    _Self __tmp = *this;
    this->_M_incr();
    return __tmp;
  }
```

C++中 int ，可以连续两次前加加，但是不能连续两次后加加；list 也是这样，所以 list 后加加操作符重载的返回值是 self ，而不是 self&



且通过 node 节点, 来进行前闭后开的实现.

```cpp
struct _List_iterator_base {
	...
    _List_node_base* _M_node; // 迭代器内部当然要有一个普通指针，指向 list 的节点
    ...
}
  // 指向首元素的迭代器
  iterator begin()             { return (_Node*)(_M_node->_M_next); }
  const_iterator begin() const { return (_Node*)(_M_node->_M_next); }

  // 指向容器尾端的迭代器
  iterator end()             { return _M_node; }
  const_iterator end() const { return _M_node; }
```

