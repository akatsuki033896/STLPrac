#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <utility>

// 第一种方法, 利用std::function指定析构形式特化FILE类型指针析构
template <class T>
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
    UniquePtr(T *p) {
        m_p = p;
        m_deleter = [] (T *p) { delete p; }; // 默认使用delete删除
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
            fclose(m_p);
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
                fclose(m_p);
            } 
            m_p = std::exchange(that.m_p, nullptr);
            puts(__PRETTY_FUNCTION__);
        }
        return *this;
    }

    T *get() const {
        return m_p;
    }
};

int main() {
    auto a = UniquePtr<FILE>(fopen("a.txt", "r"), fclose);
    std::cout << (char)fgetc(a.get()) << std::endl;
    return 0;
}