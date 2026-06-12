#include <iostream>
#include <type_traits>

template<typename T>
void test();

int main() {
    test<int>();
    test<const int>();
    test<const volatile int>();
    test<int&>();
    test<const int&>();
    test<const volatile int&>();
    
    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_volatile_v<T>) {
        std::cout << "A volitile";
    } else if (
        std::is_reference_v<T> &&
        std::is_volatile_v<std::remove_reference_t<T>>
    ) {
        std::cout << "A volitile refernce";
    } else {
        std::cout << "Not a volitile";
    }
    std::cout << std::endl;
}
