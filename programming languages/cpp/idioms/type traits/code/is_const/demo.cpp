#include <iostream>
#include <type_traits>

template<typename T>
void test();

int main() {
    test<int>();
    test<const int>();
    test<int&>();
    test<const int&>();

    return 0;
}

template<typename T>
void test() {
    if constexpr (std::is_const_v<T>) {
        std::cout << "A const";
    } else if(std::is_reference_v<T> &&
              std::is_const_v<std::remove_reference_t<T>>) {
        std::cout << "A const reference";
    } else {
        std::cout << "Not a const";
    }
    std::cout << std::endl;
}
