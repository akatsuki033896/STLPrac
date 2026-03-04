#include "Vector.hpp"
#include <cstdio>

int main() {
    // Vector vec(4);
    // for (size_t i = 0; i < vec.size(); i++) {
    //     vec[i] = i;
    // }

    // vec.reserve(16);
    // for (size_t i = 4; i < 20; i++) {
    //     vec.push_back(i);
    // }
    // printf("size=%zd, capacity=%zd\n", vec.size(), vec.capacity());

    // Vector vec_cp = vec; // 拷贝
    // vec_cp.erase(4); // 下标方式调用
    // vec_cp.erase(vec_cp.begin() + 4);
    // vec_cp.erase(vec_cp.begin() + 4, vec_cp.begin() + 8);
    //vec_cp.assign(5, 42);
    // for (size_t i = 0; i < vec_cp.size(); i++) {
    //     printf("vec_cp[%zd] = %d\n", i, vec_cp[i]);
    // }

    // 迭代器
    // int first[3] = {1000, 1000, 1000};
    // // vec_cp.assign(first, first + 3);
    // Vector vec_it = Vector(first, first + 3); // 迭代器构造
    // for (size_t i = 0; i < vec_it.size(); i++) {
    //     printf("vec[%zd] = %d\n", i, vec_it[i]);
    // }

    // 初始化列表
    // Vector vec_ini = Vector({111, 222, 333});
    // Vector vec_new = std::move(vec_ini);
    // for (size_t i = 0; i < vec_ini.size(); i++) {
    //     printf("vec[%zd] = %d\n", i, vec_ini[i]);
    // }

    // for (size_t i = 0; i < vec_new.size(); i++) {
    //     printf("vec[%zd] = %d\n", i, vec_new[i]);
    // }

    Vector<int> vec(10);
    vec.insert(vec.begin() + 3, {12, 13, 14});
    for (size_t i = 0; i < vec.size(); i++) {
        printf("vec[%zd] = %d\n", i, vec[i]);
    }

    return 0;
}
