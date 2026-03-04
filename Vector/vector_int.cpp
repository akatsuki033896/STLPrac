#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <cstdio>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

struct Vector {
    int *m_data;
    size_t m_size;
    size_t m_cap; // 实际已经分配的内存容量

    Vector() noexcept {
        m_data = nullptr;
        m_size = 0;
        m_cap = 0;
    }

    explicit Vector(size_t n, int val = 0) {
        m_data = new int[n] {}; // 初始化后不会有内存的随机值
        m_size = n;
        m_cap = n;
        for (size_t i = 0; i < m_size; i++) {
            m_data[i] = val;
        }
    }

    // 用迭代器构造, 迭代器可以是任意类型
    template <std::random_access_iterator InputIt>
    Vector(InputIt first, InputIt last) {
        size_t n = last - first;
        m_data = new int[n];
        m_size = n;
        m_cap = n;
        for (size_t i = 0; i < n; i++) {
            m_data[i] = *first;
            ++first;
        }
    }

    // 初始化列表构造 C++11新增特性 在构造函数里调用另一个构造函数
    Vector(std::initializer_list<int> ilist) : Vector(ilist.begin(), ilist.end()) {}

    // 迭代器
    int* begin() {
        return m_data;
    }

    int* end() {
        return m_data + m_size;
    }

    int* const begin() const {
        return m_data;
    }

    int* const end() const{
        return m_data + m_size;
    }

    void _recap(size_t n) {
        auto *old_data = m_data;
        auto old_size = m_size;
        if (n == 0) {
            m_data = nullptr;
            m_cap = 0;
        }
        else {
            m_data = new int[n];
            m_cap = n;
        }
        if (old_data) {
            size_t copy_size = std::min(old_size, m_size);
            if (copy_size) {
                memcpy(m_data, old_data, copy_size * sizeof(int));
            }
            delete[] old_data;
        } // 释放旧的数据
    } // O(n)

    void clear() noexcept {
        resize(0);
    } // 仅仅清除size，不清除capacity

    void resize(size_t n, int val = 0) {
        reserve(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; i++) {
                m_data[i] = val;
            }
        }
        m_size = n;
    }

    void reserve(size_t n) {
        if (n <= m_cap) [[likely]] {
            return;
        }
        n = std::max(n, m_cap * 2);
        printf("Grow from %zd to %zd\n", m_cap, n);
        auto old_data = m_data;
        if (n == 0) {
            m_data = nullptr;
            m_cap = 0;
        }
        else {
            m_data = new int[n];
            m_cap = n;
        }
        if (old_data) {
            if (m_size) {
                memcpy(m_data, old_data, m_size * sizeof(int));
            }
            delete [] old_data;
        }
    }

    void shrink_to_fit(size_t n) noexcept {
        m_cap = m_size;
        auto old_data = m_data;
        if (m_data == 0) {
            m_data = nullptr;
        }
        else {
            m_data = new int[m_size];
        }
        if (old_data) {
            if (m_size != 0) {
                memcpy(m_data, old_data, m_size * sizeof(int));
            }
            delete [] old_data;
        }
    }

    int& front() noexcept {
        return operator[](0);
    }

    int const& front() const noexcept {
        return operator[](0);
    }

    int& back() noexcept {
        return operator[](size() - 1);
    }

    int const &back() const noexcept {
        return operator[](size() - 1);
    }

    void push_back(int val) {
        // resize(size() + 1);
        // back() = val;
        reserve(m_size + 1);
        m_data[m_size] = val;
        m_size++;
    }

    size_t size() const noexcept {
        return m_size;
    } // 只读操作要加上const修饰

    size_t capacity() const noexcept {
        return m_cap;
    }

    int& at(size_t i) {
        if (i >= m_size) [[unlikely]] {
            throw std::out_of_range("vector::at");
        }
        return m_data[i];
    }

    int const& at(size_t i) const {
        if (i >= m_size) [[unlikely]] {
            throw std::out_of_range("vector::at");
        }
        return m_data[i];
    }

    // erase 的下标方式调用
    //
    // void erase(size_t i) {
    //     for (size_t j = i + 1; j < m_size; j++) {
    //         m_data[j - 1] = std::move(m_data[j]);
    //     }
    //     resize(m_size - 1);
    // }

    // void erase(size_t ibeg, size_t iend) { // ibeg <= iend
    //     size_t diff = iend - ibeg;
    //     for (size_t j = iend; j < m_size; j++) {
    //         m_data[j - diff] = std::move(m_data[j]);
    //     }
    //     resize(m_size - diff);
    // }

    void erase(int const *it) {
        size_t i = it - m_data;
        for (size_t j = i + 1; j < m_size; j++) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        resize(m_size - 1);
    }

    void erase(int const *first, int const *last) { // ibeg <= iend
        size_t diff = last - first;
        for (size_t j = last - m_data; j < m_size; j++) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        resize(m_size - diff);
    }

    void assign(size_t n, int val) {
        reserve(n);
        m_size = n; // 分配新的大小
        for (size_t i = 0; i < n; i++) {
            m_data[i] = val;
        }
    }

    template <std::random_access_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        size_t n = last - first;
        reserve(n);
        m_size = n; // 分配新的大小
        for (size_t i = 0; i < n; i++) {
            m_data[i] = *first;
            ++first;
        }
    }

    void assign(std::initializer_list<int> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    void insert(int const *it, size_t n, int val) {
        size_t j = it - m_data; // 插入位置
        if (n == 0) [[unlikely]] {
            return;
        }
        reserve(m_size + n);
        m_size += n; // 分配新的大小
        // 移动元素
        for (size_t i = n; i > 0; i--) {
            m_data[j + n + i - 1] = std::move(m_data[j + i - 1]);
        }
        for (size_t i = j; i < j + n; i++) {
            m_data[i] = val;
        }
    }

    template <std::random_access_iterator InputIt>
    void insert(int const *it, InputIt first, InputIt last) {
        size_t j = it - m_data; // 插入位置
        size_t n = last - first; // 插入 n 个元素
        if (n == 0) [[unlikely]] {
            return;
        }
        reserve(m_size + n);
        m_size += n; // 分配新的大小
        // 移动元素
        // j ~ M_size => j + 1 ~ M_size + 1
        for (size_t i = n; i > 0; i--) {
            m_data[j + n + i - 1] = std::move(m_data[j + i - 1]);
        }
        for (size_t i = j; i < j + n; i++) {
            m_data[i] = *first;
            ++first;
        }
    }

    void insert(int const *it, std::initializer_list<int> ilist) {
        insert(it, ilist.begin(), ilist.end());
    }

    int const &operator[](size_t i) const noexcept {
        return m_data[i];
    } // 只读, 返回引用减少开销

    int& operator[](size_t i) noexcept {
        return m_data[i];
    } // 返回左值引用

    Vector (Vector const& that) {
        m_size = that.m_size;
        m_cap = that.m_cap;
        if (m_size) {
            m_data = new int [m_size];
            memcpy(m_data, that.m_data, m_size * sizeof(int));
        }
        else {
            m_data = nullptr;
        }
    }

    Vector &operator=(Vector const& that) {
        clear();
        m_size = that.m_size;
        m_cap = that.m_cap;
        if (m_size) {
            m_data = new int [m_size];
            memcpy(m_data, that.m_data, m_size * sizeof(int));
        }
        return *this;
    }

    Vector (Vector&& that) noexcept {
        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
        // 清空
    }

    Vector &operator=(Vector&& that) noexcept {
        clear();
        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
        return *this;
    }

    ~Vector() noexcept {
        delete[] m_data;
    } // 一旦定义了析构函数，必须同时定义拷贝函数和移动构造函数
};


int main() {
    // 初始化列表
    Vector vec_ini = Vector({111, 222, 333});
    Vector vec_new = std::move(vec_ini);
    for (size_t i = 0; i < vec_ini.size(); i++) {
        printf("vec[%zd] = %d\n", i, vec_ini[i]);
    }

    for (size_t i = 0; i < vec_new.size(); i++) {
        printf("vec[%zd] = %d\n", i, vec_new[i]);
    }

    return 0;
}
