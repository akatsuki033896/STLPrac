#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <utility>

// 最推荐使用全特化的方法
// 第二种方法，全特化，直接在模板delete, deleter是一种类型
template<class T>
struct DefaultDeleter {
    void operator()(T *p) const {
        delete p;
    }
};

// 对数组类型偏特化
template<class T>
struct DefaultDeleter<T[]> {
    void operator()(T *p) const {
        delete[] p;
    }
};

// 对 FILE 类型特化, 这个类型指针由fclose专门操作
template<>
struct DefaultDeleter<FILE> {
    void operator()(FILE *p) const {
        fclose(p);
    }
};

template <class T, class Deleter = DefaultDeleter<T>>
struct UniquePtr {
private:
    T *m_p;
    std::function<void (T *)> m_deleter; // 使用函数指定析构的方式
public:
    // 默认构造函数 无参
    UniquePtr() {
        m_p = nullptr;
        puts(__PRETTY_FUNCTION__);
    }
    // 通过 *p 构造
    // 警告编译器，声明为显式构造，防止错误隐式转化，delete掉栈上指针,例如UniquePtr<int> a = &i;
    explicit UniquePtr(T *p) {
        m_p = p;
        puts(__PRETTY_FUNCTION__);
    }

    // 通过 *p 和 deleter构造
    UniquePtr(T *p, std::function<void (T *)> deleter) {
        m_p = p;
        m_deleter = deleter;
        puts(__PRETTY_FUNCTION__);
    }
    // 析构函数
    ~UniquePtr() {
        puts(__PRETTY_FUNCTION__);
        if (m_p) {
            // fclose(m_p);
            Deleter{}(m_p); // 构造一个Deleter对象并调用，不直接fclose
        }
    }
    // 拷贝构造函数
    UniquePtr(UniquePtr const &that) = delete;
    
    // 拷贝赋值函数
    UniquePtr &operator=(UniquePtr const &that) = delete;
    
    // 移动构造函数
    UniquePtr(UniquePtr &&that) {
        m_p = std::exchange(that.m_p, nullptr);
    }
    
    // 移动赋值
    UniquePtr &operator=(UniquePtr &&that) {
        if (this != &that) [[likely]] {
            if (m_p) {
                // fclose(m_p);
                Deleter{}(m_p); // 构造一个Deleter对象并调用，不直接fclose
            } 
            m_p = std::exchange(that.m_p, nullptr);
            puts(__PRETTY_FUNCTION__);
        }
        return *this;
    }

    T *get() const {
        return m_p;
    }

    // 重载 ->
    T *operator->() const {
        return m_p;
    }
};

// 对 UniquePtr 进行数组的偏特化
// 数组的T要从原来的T继承过来，完全继承，不加东西了
template<class T, class Deleter>
struct UniquePtr<T[], Deleter> : UniquePtr<T, Deleter> {};

struct Myclass {
    int a, b, c;
};

// make_unique

// template<class T>
// UniquePtr<T> make_Unique() {
//     return UniquePtr<T>(new T());
// }

// 参数打包 传入多个参数
template<class T, class ...Args>
UniquePtr<T> make_Unique(Args ...args) {
    return UniquePtr<T>(new T(args...));
}

int main() {
    // auto a = UniquePtr<FILE>(fopen("a.txt", "r"));
    // auto arr = UniquePtr<int[]>(new int[2]);
    // auto arr = UniquePtr<int>(new int(42));
    // std::cout << (char)fgetc(a.get()) << std::endl;
    
    // int i;
    // auto p = UniquePtr<int>(&i); // 不可以，为了防止出错已经用explicit显式声明
    // auto p = UniquePtr<int>(new int(42)); // 可以

    // 对于传入自定义类，要指定每个成员，才能传入UniquePtr很麻烦
    // auto p = UniquePtr<Myclass>(new Myclass(42, 43, 44));
    auto p = make_Unique<Myclass>();
    // std::cout << p.get()->a << '\n'; // 此时初始化为0，但是要通过get()获取很麻烦，考虑重载 ->
    std::cout << p->a << '\n';

    // 考虑传入参数，可能传入很多个参数，定义很多个模板函数很麻烦，可以使用参数打包机制
    auto p1 = make_Unique<Myclass>(Myclass{42, 43, 44}); // c++20以前的写法
    // auto p1 = make_Unique<Myclass>(42, 43, 44); // c++20 支持()直接构造
    // 我是傻逼 我忘记给clangd开cpp20标准了 懒得开了就用17写吧知道有这个就好
    std::cout << p1->a << '\n';
    std::cout << p1->b << '\n';
    std::cout << p1->c << '\n';
}