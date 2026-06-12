#include <iostream>
#include <type_traits>

struct SomeStruct{};

template<typename T>
void test(const T&);

int main() {
    test(42);
    test(3.14);
    test(true);
    test(SomeStruct{});

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_fundamental_v<T>) {
        std::cout << "F: " << _value << std::endl;
    } else {
        std::cout << "Other" << std::endl;
    }
}
