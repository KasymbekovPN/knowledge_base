#include <iostream>
#include <type_traits>

union Data {
    std::int32_t n;
    std::uint16_t s[2];
};

struct Vec3 {
    float x, y, z;
};

template<typename T>
void test(const T&);

int main() {
    test(Data{0x1234567});
    test(Vec3());
    test(123);

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_union_v<T>) {
        std::cout << "Union" << std::endl;
    } else {
        std::cout << "Other" << std::endl;
    }
}
