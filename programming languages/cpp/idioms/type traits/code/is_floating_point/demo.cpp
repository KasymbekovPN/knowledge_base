#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

int main() {
    test(3.14159);
    test(42);
    test(2.5f);
    test('A');
    test(true);

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_floating_point_v<T>) {
        std::cout << "FP: " << _value << std::endl;
    } else {
        std::cout << "Other: " << _value << std::endl;
    }
}
