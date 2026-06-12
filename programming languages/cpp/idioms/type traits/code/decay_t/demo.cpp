#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int&, int>();
    test<int&&, int>();
    test<const int&, int>();
    test<int[5], int*>();
    test<char[], char*>();
    test<void(int), void(*)(int)>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::decay_t<T>, R>
        << std::endl;
}
