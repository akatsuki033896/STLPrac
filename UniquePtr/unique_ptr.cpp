#include <cstdio>
#include <utility>
#include <vector>
#include <concepts>
#include <functional>

// std::exchange的实现
template<class T, class U>
T exchange(T &dst, U &&val) {
    T tmp = std::move(dst);
    // dst = std::move(val);
    dst = std::forward<U>(val); // 转发
    return tmp;
}

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

    template<class U, class UDeleter>
    friend struct UniquePtr; // 相同的类都是朋友
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

    // 子对象指针转为基类对象的指针
    template<class U, class UDeleter>
        requires (std::convertible_to<U*, T*>) // 允许从T指针转回U指针
    UniquePtr(UniquePtr<U, UDeleter> &&that) {
        // m_p = std::exchange(that.m_p, nullptr); // 把原来that.m_p设为nullptr实现移动
        // Uniqueptr U 和 T 不是一个类型，不能互相访问对方私有变量，需要设置友元
        m_p = exchange(that.m_p, nullptr);
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

    // release
    T *release() {
        return exchange(m_p, nullptr);
    }

    void reset(T *p = nullptr) {
        // if (m_p) {
        //     Deleter{} (m_p);
        // }
        // m_p = nullptr;

        // 等价于
        if (m_p) {
            Deleter{} (release());
        }
    }

    // 返回引用
    T &operator*() const {
        return *m_p;
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

template<class T>
UniquePtr<T> make_Unique() {
    return UniquePtr<T>(new T());
}

// 参数打包 传入多个参数
template<class T, class ...Args>
// 改为万能引用，不用理解为什么这样背就对了
UniquePtr<T> make_Unique(Args &&...args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// make_unique_for_overwrite 不接受任何参数 20新标准用的不多
template<class T>
UniquePtr<T> make_Unique_for_overwrite() {
    return UniquePtr<T>(new T);
}

// 以上实现了Unique_ptr

// 常见用途：在vector里存指向多态类的指针（多态类必须依靠指针来使用）
// struct Animal {
//     virtual void speak() = 0;
//     virtual ~Animal() = default;
// };

// struct Dog : Animal {
//     int age;
//     Dog(int age_) : age(age_) {}
//     virtual void speak() {
//         // puts("Bark!");
//         printf("Bark!, I'm %d years old.\n", age);
//     }
// };

// struct Cat : Animal {
//     int &age; // 引用，外部增长了里面也能增长
//     Cat(int &age_) : age(age_) {}
//     virtual void speak() {
//         // puts("Meow!");
//         printf("Meow!, I'm %d years old.\n", age);
//     }
// };

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
    // auto p = make_Unique<Myclass>();
    // // std::cout << p.get()->a << '\n'; // 此时初始化为0，但是要通过get()获取很麻烦，考虑重载 ->
    // std::cout << p->a << '\n';

    // // 考虑传入参数，可能传入很多个参数，定义很多个模板函数很麻烦，可以使用参数打包机制
    // auto p1 = make_Unique<Myclass>(Myclass{42, 43, 44}); // c++20以前的写法
    // auto p1 = make_Unique<Myclass>(42, 43, 44); // c++20 支持()直接构造
    // // 我是傻逼 我忘记给clangd开cpp20标准了 懒得开了就用17写吧知道有这个就好
    // std::cout << p1->a << '\n';
    // std::cout << p1->b << '\n';
    // std::cout << p1->c << '\n';

    // new int 默认初始化，值不确定
    // new int() 值初始化，值为0
    // auto ip = make_Unique_for_overwrite<int>();
    // // std::cout << *ip.get() << '\n'; // 值为 0，重载前必须显式get()
    // std::cout << *ip << '\n'; // 重载乘号运算符返回引用，箭头是返回指针

    // 常见用途：在vector里存指向多态类的指针（多态类必须依靠指针来使用）
    // 还需要把子对象的指针转化为基类对象的指针
    // std::vector<UniquePtr<Animal>> zoo;
    // int age = 3;
    // zoo.push_back(make_Unique<Dog>(age));
    // zoo.push_back(make_Unique<Cat>(age));
    // for (auto const &a: zoo) {
    //     a->speak();
    // }
    // age++;
    // for (auto const &a: zoo) {
    //     a->speak();
    // }
    // 打印引用发现数字不对，Args捕获错误，无法修改狗和猫其中一类然后另一类保持不变
    // 使用万能引用 &&
    // 因此猫的年龄随着main中age增长而增长（传入引用），狗不会（传值，类里面的age只是副本）

    // UniquePtr<FILE> fp(fopen("a.txt", "r"));
    // fp.reset();

    // 此时希望reset可以打开另一个文件

    return 0;
}
