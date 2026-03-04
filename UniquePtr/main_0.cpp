#include <cstdio>
#include <iostream>
#include <ostream>
#include <utility>
#include <type_traits>

struct UniquePtr {
private:
    FILE *p = nullptr;
public:
    // 构造函数
    UniquePtr(const char* path) {
        puts(__PRETTY_FUNCTION__);
        p = fopen(path, "r");
        if (!p) {
            perror(path);
        }
    }
    // 析构函数
    ~UniquePtr() {
        puts(__PRETTY_FUNCTION__);
        if (p) {
            fclose(p);
        }
    }
    
    // 为什么要把拷贝构造和拷贝赋值删掉？因为对于文件指针，拷贝构造和赋值的时候原来的对象还会在，析构的时候不能析构两次，只析构一次则内存泄漏，所以直接不用他们了

    // 拷贝构造函数
    UniquePtr(UniquePtr const &that) = delete;
    
    // 拷贝赋值函数
    UniquePtr &operator=(UniquePtr const &that) = delete;
    
    // 移动构造函数把原来的对象的值移动给新的对象，然后把原来的给删除掉，因为这个操作很常见，标准库就提供了封装好的写法std::exchange
    // 移动构造函数
    UniquePtr(UniquePtr &&that) {
        p = std::exchange(that.p, nullptr);
    }
    
    // 移动赋值
    UniquePtr &operator=(UniquePtr &&that) {
        if (this != &that) [[likely]] {
            if (p) {
                fclose(p);
            } 
            p = std::exchange(that.p, nullptr);
            puts(__PRETTY_FUNCTION__);
        }
        return *this;
    }

    FILE *get() const {
        return p;
    }
};

int main() {
    auto a = UniquePtr("a.txt");
    std::cout << (char)fgetc(a.get()) << std::endl;
    return 0;
}