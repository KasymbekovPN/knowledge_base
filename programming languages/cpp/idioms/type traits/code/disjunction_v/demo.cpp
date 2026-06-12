#include <iostream>
#include <type_traits>

template<typename T>
std::enable_if_t<
    std::disjunction_v<
        std::is_same<T, int>,
        std::is_same<T, float>,
        std::is_same<T, double>
    >
>
test(T&&);

int main() {
    test(42);
    test(12.34);
    test(2.72f);
    // test("hello"); // Error

    return 0;
}

template<typename T>
std::enable_if_t<
    std::disjunction_v<
        std::is_same<T, int>,
        std::is_same<T, float>,
        std::is_same<T, double>
    >
>
test(T&& _input) {
    std::cout << "V: " << _input << std::endl;
}
