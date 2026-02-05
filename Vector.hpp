// https://en.cppreference.com/w/cpp/container/vector.html
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

template<class T, class Alloc = std::allocator<T>>
struct Vector {
    using value_type = T;
    using pointer = T*;
    using const_pointer = T* const;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = T const&;
    using iterator = T*;
    using const_iterator = T* const;
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<T* const>;
    using allocator = Alloc;

    // 8 byte 对齐
    T *m_data;
    size_t m_size;
    size_t m_cap; // 实际已经分配的内存容量
    [[no_unique_address]] Alloc m_alloc; // 不一定要有独一无二的地址 Alloc占1byte 若对齐则要补7byte 让编译器放心优化

    Vector() noexcept {
        m_data = nullptr;
        m_size = 0;
        m_cap = 0;
    }

    explicit Vector(size_t n, Alloc const& alloc = Alloc()) : m_alloc(alloc){
        m_data = m_alloc.allocate(n);
        m_size = n;
        m_cap = n;
        for (size_t i = 0; i < m_size; i++) {
            std::construct_at(&m_data[i]);
        }
    }

    Vector(size_t n, T const &val, Alloc const& alloc = Alloc()) : m_alloc(alloc){
        m_data = m_alloc.allocate(n);
        m_size = n;
        m_cap = n;
        for (size_t i = 0; i < m_size; i++) {
            std::construct_at(&m_data[i], val);
        }
    }

    // 用迭代器构造, 迭代器可以是任意类型
    template <std::random_access_iterator InputIt>
    Vector(InputIt first, InputIt last, Alloc const& alloc = Alloc()) : m_alloc(alloc){
        size_t n = last - first;
        m_data = m_alloc.allocate(n);
        m_size = n;
        m_cap = n;
        for (size_t i = 0; i < n; i++) {
            m_data[i] = *first;
            ++first;
        }
    }

    // 初始化列表构造 C++11新增特性 在构造函数里调用另一个构造函数
    // 分配器是拷贝构造
    Vector(std::initializer_list<T> ilist, Alloc const &alloc = Alloc()) : Vector(ilist.begin(), ilist.end(), alloc) {}

    // 迭代器
    T *data() noexcept {
        return m_data;
    }

    T const *data () const noexcept {
        return m_data;
    }

    T const *cdata () const noexcept {
        return m_data;
    }

    T *begin() noexcept {
        return m_data;
    }

    T const *begin() const noexcept {
        return m_data;
    }

    T const *cbegin() const noexcept {
        return m_data;
    }

    // reverse
    std::reverse_iterator<T *> *rbegin() noexcept {
        return *std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<T const*> *rbegin() const noexcept {
        return *std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<T const*> *crbegin() const noexcept {
        return *std::make_reverse_iterator(m_data);
    }

    T *end() noexcept {
        return m_data + m_size;
    }

    T const *end() const noexcept {
        return m_data + m_size;
    }

    T const *cend() const noexcept {
        return m_data + m_size;
    }

    std::reverse_iterator<T *> *rend() noexcept {
        return *std::make_reverse_iterator(m_data + m_size);
    }

    std::reverse_iterator<T const*> *rend() const noexcept {
        return *std::make_reverse_iterator(m_data + m_size);
    }

    std::reverse_iterator<T const*> *crend() const noexcept {
        return *std::make_reverse_iterator(m_data + m_size);
    }

    void _recap(size_t n) {
        auto *old_data = m_data;
        auto old_size = m_size;
        auto old_cap = m_cap;
        if (n == 0) {
            m_data = nullptr;
            m_cap = 0;
        }
        else {
            m_data = m_alloc.allocate(n);
            m_cap = n;
        }
        if (old_data) {
            size_t copy_size = std::min(old_size, m_size);
            if (copy_size) {
                for (size_t i = 0; i < m_size; i++) {
                    std::construct_at(&m_data[i], std::as_const(old_data[i]));
                }
            }
            m_alloc.deallocate(old_data, old_cap);
        } // 释放旧的数据
    } // O(n)

    void clear() noexcept {
        for (size_t i = 0; i < m_size; i++) {
            std::destroy_at(&m_data[i]);
        }
        m_size = 0;
    } // 仅仅清除size，不清除capacity

    void resize(size_t n) {
        reserve(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; i++) {
                std::construct_at(&m_data[i]); // m_data[i] = 0
            }
        }
        m_size = n;
    }

    void resize(size_t n, T const &val) {
        reserve(n);
        if (n > m_size) {
            for (size_t i = m_size; i < n; i++) {
                std::construct_at(&m_data[i], val);
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
        auto old_cap = m_cap;
        if (n == 0) {
            m_data = nullptr;
            m_cap = 0;
        }
        else {
            m_data = m_alloc.allocate(n);
            m_cap = n;
        }
        if (old_data) {
            if (m_size) {
                for (size_t i = 0; i < m_size; i++) {
                    std::construct_at(&m_data[i], std::as_const(old_data[i]));
                }
            }
            m_alloc.deallocate(old_data, old_cap);
        }
    }

    void shrink_to_fit() noexcept {
        auto old_cap = m_cap;
        auto old_data = m_data;
        m_cap = m_size;
        if (m_data == 0) {
            m_data = nullptr;
        }
        else {
            m_data = m_alloc.allocate(m_size);
        }
        if (old_cap) {
            if (m_size != 0) {
                for (size_t i = 0; i < m_size; i++) {
                    std::construct_at(&m_data[i], std::as_const(old_data[i]));
                }
            }
            m_alloc.deallocate(old_data, old_cap);
        }
    }

    T &front() noexcept {
        return operator[](0);
    }

    T const& front() const noexcept {
        return operator[](0);
    }

    T &back() noexcept {
        return operator[](size() - 1);
    }

    T const &back() const noexcept {
        return operator[](size() - 1);
    }

    void push_back(T const &val) {
        reserve(m_size + 1);
        // m_data[m_size] = val;
        std::construct_at(&m_data[m_size], val);
        m_size++;
    } // 拷贝

    void push_back(T &&val) {
        reserve(m_size + 1);
        std::construct_at(&m_data[m_size], std::move(val));
        m_size++;
    } // 对于不许拷贝的类型 e.g. std::thread

    template <class ...Args>
    T &emplace_back(Args &&...args) {
        reserve(m_size + 1);
        T *p = &m_data[m_size];
        std::construct_at(&m_data[m_size], std::forward<Args>(args)...);
        m_size++;
        return *p;
    }

    size_t size() const noexcept {
        return m_size;
    } // 只读操作要加上const修饰

    size_t capacity() const noexcept {
        return m_cap;
    }

    T &at(size_t i) {
        if (i >= m_size) [[unlikely]] {
            throw std::out_of_range("vector::at");
        }
        return m_data[i];
    }

    T const& at(size_t i) const {
        if (i >= m_size) [[unlikely]] {
            throw std::out_of_range("vector::at");
        }
        return m_data[i];
    }

    T *erase(T const *it) noexcept(std::is_nothrow_move_assignable_v<T>){
        size_t i = it - m_data; // 删除位置
        for (size_t j = i + 1; j < m_size; j++) {
            m_data[j - 1] = std::move_if_noexcept(m_data[j]);
        }
        m_size--;
        std::destroy_at(&m_data[i]);
        return const_cast<T *>(it + 1);
    }

    T *erase(T const *first, T const *last) noexcept (std::is_nothrow_move_assignable_v<T>){
        size_t diff = last - first; // 删除区间大小
        for (size_t j = last - m_data; j < m_size; j++) {
            m_data[j - diff] = std::move_if_noexcept(m_data[j]);
        }
        m_size -= diff;
        for (size_t j = m_size; j > m_size + diff; j--) {
            std::destroy_at(&m_data[j]);
        }
        return const_cast<T *>(first);
    }

    void assign(size_t n, T val) {
        clear();
        reserve(n);
        m_size = n; // 分配新的大小
        for (size_t i = 0; i < n; i++) {
            std::construct_at(&m_data[i], val);
        }
    }

    template <std::random_access_iterator InputIt>
    void assign(InputIt first, InputIt last) {
        clear();
        size_t n = last - first;
        reserve(n);
        m_size = n; // 分配新的大小
        for (size_t i = 0; i < n; i++) {
            m_data[i] = *first;
            ++first;
        }
    }

    void assign(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    T *insert(T const *it, T const &val) {
        size_t j = it - m_data; // 插入位置
        reserve(m_size + 1);
        m_size ++; // 分配新的大小
        // 移动元素
        for (size_t i = j; i < m_size; i++) {
            std::construct_at(&m_data[j], std::move(m_data[j]));
        }
        for (size_t i = j; i != j + 1; i++) {
            std::construct_at(&m_data[i], val);
        }
        return m_data + j;
    } // 拷贝

    T *insert(T const *it, T &&val) {
        size_t j = it - m_data; // 插入位置
        reserve(m_size + 1);
        m_size ++; // 分配新的大小
        // 移动元素
        for (size_t i = j; i < m_size; i++) {
            std::construct_at(&m_data[j], std::move(m_data[j]));
        }
        for (size_t i = j; i != j + 1; i++) {
            std::construct_at(&m_data[i], val);
        }
        return m_data + j;
    } // 移动

    T *insert(T const *it, size_t n, T const &val) {
        size_t j = it - m_data; // 插入位置
        if (n == 0) [[unlikely]] {
            return const_cast<T *>(it);
        }
        reserve(m_size + n);
        m_size += n; // 分配新的大小
        // 移动元素 j ~ m_size => j + n ~ m_size + n
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[n + i - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        for (size_t i = j; i < j + n; i++) {
            // m_data[i] = val;
            std::construct_at(&m_data[i], val);
        }
        return m_data + j;
    } // 返回插入成功的第一个值

    T *insert(T const *it, size_t n, T &&val) {
        size_t j = it - m_data; // 插入位置
        if (n == 0) [[unlikely]] {
            return const_cast<T *>(it);
        }
        reserve(m_size + n);
        m_size += n; // 分配新的大小
        // 移动元素 j ~ m_size => j + n ~ m_size + n
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[n + i - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        for (size_t i = j; i < j + n; i++) {
            // m_data[i] = val;
            std::construct_at(&m_data[i], val);
        }
        return m_data + j;
    } // 返回插入成功的第一个值

    template <std::random_access_iterator InputIt>
    T *insert(T const *it, InputIt first, InputIt last) {
        size_t j = it - m_data; // 插入位置
        size_t n = last - first; // 插入 n 个元素
        if (n == 0) [[unlikely]] {
            return const_cast<T *>(it);
        }
        reserve(m_size + n);
        m_size += n; // 分配新的大小
        // 移动元素
        // j ~ M_size => j + 1 ~ M_size + 1
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[n + i - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        for (size_t i = j; i < j + n; i++) {
            std::construct_at(&m_data[i], *first);
            ++first;
        }
        return m_data + j;
    }

    T *insert(T const *it, std::initializer_list<T> ilist) {
        return insert(it, ilist.begin(), ilist.end());
    }

    template<class ...Args>
    T *emplace(T const *it, Args &&...args) {
        size_t j = it - m_data; // 插入位置
        reserve(m_size + 1);
        m_size++; // 分配新的大小
        // 移动元素 j ~ m_size => j + n ~ m_size + n
        for (size_t i = m_size; i > j; i--) {
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        std::construct_at(&m_data[j], std::forward<Args>(args)...);
        return m_data + j;
    }

    void swap(Vector& other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_cap, other.m_cap);
    }

    T const &operator[](size_t i) const noexcept {
        return m_data[i];
    } // 只读, 返回引用减少开销

    T& operator[](size_t i) noexcept {
        return m_data[i];
    } // 返回左值引用

    Vector (Vector const& that) {
        m_size = that.m_size;
        m_cap = that.m_cap;
        if (m_size) {
            m_data = m_alloc.allocate(m_size); // 等于 malloc free
            for (size_t i = 0; i < m_size; i++) {
                // m_data[i] = std::as_const(that.m_data[i]);
                std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
            }
        }
        else {
            m_data = nullptr;
        }
    }

    Vector &operator=(Vector const& that) {
        if (&that == this) [[likely]] {
            return *this;
        }
        clear();
        m_size = that.m_size;
        m_cap = that.m_cap;
        if (m_size) {
            for (size_t i = 0; i < m_size; i++) {
                std::construct_at(&m_data[i], std::as_const(that.m_data[i]));
            }
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
        if (&that == this) [[likely]] {
            return *this;
        }
        if (m_cap != 0) {
            m_alloc.deallocate(m_data, m_cap);
        }
        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;

        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
        return *this;
    }

    ~Vector() noexcept {
        m_alloc.deallocate(m_data, m_cap);
    } // 一旦定义了析构函数，必须同时定义拷贝函数和移动构造函数
};
