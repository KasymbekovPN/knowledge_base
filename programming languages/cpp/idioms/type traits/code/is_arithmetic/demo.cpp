#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

int main() {
    test(42);
    test(3.14);
    test('A');
    test(true);
    test(std::string("Hello"));
    
    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_arithmetic_v<T>) {
        std::cout << "ARI: " << _value << std::endl;
    } else {
        std::cout << "Other: " << _value << std::endl;
    }
}
