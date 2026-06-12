#include <iostream>
#include <type_traits>

template<typename T>
void test_remove_const_t() {
    std::cout
        << typeid(T).name() << ": "
        << typeid(std::remove_const_t<T>).name()
        << std::endl;
}

template<typename T>
void test_remove_cv_t() {
    std::cout
        << typeid(T).name() << ": "
        << typeid(std::remove_cv_t<T>).name()
        << std::endl;
}

template<typename T>
void test_remove_cvref_t() {
    std::cout
        << typeid(T).name() << ": "
        << typeid(std::remove_cvref_t<T>).name()
        << std::endl;
}

int main() {
    std::cout << std::boolalpha;

    test_remove_const_t<const int>();
    test_remove_const_t<int>();
    test_remove_const_t<const double>();
    test_remove_const_t<const int*>();
    test_remove_const_t<int* const>();
    test_remove_const_t<const char*>();

    test_remove_cv_t<const volatile int>();

    test_remove_cvref_t<const std::string&>();

    return 0;
}
