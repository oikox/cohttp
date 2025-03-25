# 零碎知识点

## GAI_ERRNO

getaddrinfo有单独的一套errno机制GAI_ERRNO

获取需要通过gar_strerrno(int)；并且返回值不是-1，直接就是errno



## 移动构造

==移动语义==是C++11才实现的，它的意义是对于一些即将被销毁的对象，可以不复制它，而是窃取它的资源，以此来减少一次复制，达到大幅度提升性能的效果。

在C++11标准之前，没有直接的方法移动对象，即使某些情况不需要拷贝对象，也不得不拷贝。例如，函数返回非引用的返回值时，会先构造一个临时对象作为返回值，函数调用时将该临时对象赋值给接收对象，调用结束后会销毁该临时对象。这里的临时对象是肯定会被销毁的，如果能够将临时对象的资源直接移交给接收对象，那么就可以减少一次拷贝。旧标准则只能拷贝，然后再销毁临时对象的资源。



C++11标准引入的一种新的引用类型——==右值引用==，来实现移动操作。右值引用只能关联到右值，而右值要么是字面值，要么是求值过程中产生的临时对象。右值引用的定义使用了`&&`，可以参考下面代码：		

```c++
int &&rri = 13; //定义了右值引用rri
```



移动构造函数(move constructor)，是C++11标准增加的在创建对象时移动旧对象资源的构造函数。

类的移动构造函数原型通常是这样的：`类名(类类型 &&);`。以`CDate`类为例，其移动构造函数声明如下：

```c++
CDate(CDate &&date);
```

移动构造函数的几个特点：
1、函数名和类名相同，没有返回值，因为它也是构造函数的一种；
2、第一个参数必须是一个自身类类型的右值引用(&&)，且其他参数都有默认值。
3、第一个参数不能声明为 const 右值引用的原因是该引用在函数内会被修改(移动资源)。
4、移动构造函数执行后，需要保证右值引用的对象能够被正常销毁。

```c++
CDate(CDate &&date)  noexcept; //声明
CDate::CDate(CDate&& date)noexcept
{
    m_year = date.m_year;
    m_mon = date.m_mon;
    m_day = date.m_day;
    str = date.str;
    date.str = NULL;
    cout << "Calling Move Constructor" << ", this = " << this << endl;
}
```

移动构造函数解析：

1、处理普通数据成员，直接使用==旧对象的进行赋值==；
2、处理指向堆内存的指针，直接==堆内存地址给新对象的指针==，旧对象指针指向==NULL==。
语句str = date.str;将旧对象的str直接给到正在创建的对象，而不重新new、复制。
语句date.str = NULL;将旧对象的str赋值为NULL，保证其可以被正常销毁，delete NULL;、delete[] NULL不会造成任何问题。
3、由于移动操作“ 窃取” 资源， 它通常不分配任何资源。 因此， 移动操作通常不会抛出任何异常。不抛出异常的函数应该使用 noexcept 通知标准库，避免编译器为了处理异常而作一些额外的工作。



1、使用移动构造函数就需要弄清楚移动构造函数是什么时候会被调用的？
如果使用一个右值(即将被销毁的对象)去初始化同类型的对象，就会调用该类的移动构造函数。

2、怎样的类需要定义移动构造函数？
如果该类的成员存在指针指向new分配的堆内存，则可以跟进需要定义移动构造函数。

3、什么时候编译器会提供默认移动构造函数？
如果该类没有定义拷贝构造函数，且没有定义移动构造函数，则编译器会提供一个 默认的移动构造函数。
如果该类定义了拷贝构造函数，则编译器不会提供 默认的移动构造函数，需要用到移动构造的地方都会调用拷贝构造函数。
如果该类定义了移动构造函数，则编译器不会提供 默认的拷贝构造函数，需要用到拷贝构造的时候，编译器会报错。

4、默认的移动构造函数做了什么工作？
默认的移动构造函数 所做的工作与默认拷贝构造函数的工作一样，执行逐成员初始化并复制内置类型。如果成员是类对象，将使用相应类的构造函数和赋值运算符，就像参数为右值一样。



## 默认移动构造

在C++中，编译器会为类生成默认的移动构造函数，但某些场景下需要手动编写移动构造函数。主要原因包括：

**资源所有权转移需求**
当类管理独占资源（如裸指针、文件句柄）时，默认移动构造函数仅进行浅拷贝：

```c++
class Buffer {
private:
  int* data_;
  size_t size_;
  
public:
  // 必须手动实现
  Buffer(Buffer&& other) noexcept 
    : data_(other.data_), size_(other.size_) {
    other.data_ = nullptr;  // 关键：置空原对象指针
    other.size_ = 0;
  }
};

```

**非默认移动语义成员**
当类包含无法正确移动的成员时：

```c++
class SpecialMember {
  std::mutex mtx_;  // 互斥锁不可移动
public:
  // 需要手动处理不可移动成员
  SpecialMember(SpecialMember&& other) {
    // 通过其他方式重建状态
  }
};

```

**优化性能**
当可以通过重组数据结构提升移动效率时：

```c++
class Optimized {
  std::vector<std::string> data_;
public:
  Optimized(Optimized&& other) noexcept 
    : data_(std::move(other.data_)) {
      other.data_.clear();  // 主动清空减少析构开销
  }
};

```

**维护类不变式**
需要保证移动后的源对象处于有效但空的状态：

```c++
class Connection {
  Socket socket_;
public:
  Connection(Connection&& other) 
    : socket_(std::move(other.socket_)) {
      if (!socket_.is_valid()) 
        throw std::runtime_error("移动后连接失效");
  }
};

```

**处理继承体系**
确保基类部分正确移动：

```c++
class Derived : public Base {
  int extra_data_;
public:
  Derived(Derived&& other) 
    : Base(std::move(other)),  // 显式移动基类
      extra_data_(other.extra_data_) {}
};

```

实际开发中建议：

- 优先使用默认移动语义
- 在需要资源管理时优先使用==智能指针等RAII对象==
- 通过 `= default` 显式声明默认移动操作
- 使用 `noexcept` 声明增强编译器优化能力
- 通过静态断言检查类型是否可移动：`static_assert(std::is_move_constructible_v<T>)`

### =default

```c++
class Widget {
private:
    std::vector<int> buffer_;
    FILE* logfile_ = nullptr;
    
public:
    // 情况1：当需要保留默认移动语义时
    Widget(Widget&&) = default;            // 移动构造函数
    Widget& operator=(Widget&&) = default; // 移动赋值运算符

    // 情况2：用户定义拷贝操作后仍需移动语义
    Widget(const Widget&) { /* 自定义拷贝逻辑 */ }
    Widget& operator=(const Widget&) { /* 自定义拷贝逻辑 */ }

    // 显式声明默认移动操作（必须同时声明移动构造和移动赋值）
    Widget(Widget&&) = default;
    Widget& operator=(Widget&&) = default;
};

```

**需要显式声明的情形**

|        场景        |                           代码示例                           |
| :----------------: | :----------------------------------------------------------: |
| 定义拷贝构造函数后 | `MyClass(const MyClass&) { ... }; MyClass(MyClass&&) = default;` |
|     存在虚函数     |        `virtual ~Base() {}; Base(Base&&) = default;`         |
|  需要明确接口约定  | `MyType(MyType&&) noexcept = default;`（添加noexcept优化容器操作性能） |
|     模板类特化     | `template<> Specialized<int>::Specialized(Specialized&&) = default;` |

**最佳实践**

```c++
// 推荐写法：明确移动语义+异常安全
class ResourceHolder {
public:
    std::unique_ptr<int[]> data;
    size_t size;

    // 显式声明移动操作（带noexcept）
    ResourceHolder(ResourceHolder&& other) noexcept
        : data(std::move(other.data)), size(other.size) {
        other.size = 0;
    }

    ResourceHolder& operator=(ResourceHolder&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            size = other.size;
            other.size = 0;
        }
        return *this;
    }

    // 显式保留拷贝操作（删除或自定义）
    ResourceHolder(const ResourceHolder&) = delete;
    ResourceHolder& operator=(const ResourceHolder&) = delete;
};

```

