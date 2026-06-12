#include <iostream>
#include <type_traits>

struct V3 {
    float x, y, z;
};

struct Logger {
    std::string name;
    void log() {
        std::cout << name << std::endl;
    };
};

template<typename T>
void test_copy(T&, const T&);

int main() {
    V3 v0{1, 2, 3}, v1{};
    Logger l0{"Hello"}, l1;

    test_copy(v1, v0);
    test_copy(l1, l0);
    
    return 0;
}

template<typename T>
void test_copy(T& _dst, const T& _src) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(&_dst, &_src, sizeof(T));
        std::cout << "Coried using memcpy" << std::endl;
    } else {
        _dst = _src;
        std::cout << "Coried using assignment" << std::endl;
    }
}
