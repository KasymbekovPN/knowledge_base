#include <iostream>
#include <type_traits>

struct Value { int x; };

enum Color { Red };

union U { int i; float f; };

template<typename T>
void test();

int main() {
    test<int>();
    test<bool>();
    test<std::string>();
    test<int*>();
    test<Value>();
    test<Color>();
    test<U>();

    test<void>();
    test<int&>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_object_v<T>) {
        std::cout << "A object type";
    } else {
        std::cout << "Not a object type";
    }
    std::cout << std::endl;
}
