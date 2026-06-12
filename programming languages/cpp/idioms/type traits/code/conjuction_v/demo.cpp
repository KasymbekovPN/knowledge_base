#include <iostream>

template<typename T>
std::enable_if_t<
    std::conjunction_v<
        std::is_integral<T>,
        std::negation<
            std::is_const<T>
        >
    >
>
test(T);

int main() {
    test(42);
    test(12.34); // Error

    return 0;
}

template<typename T>
std::enable_if_t<
    std::conjunction_v<
        std::is_integral<T>,
        std::negation<
            std::is_const<T>
        >
    >
>
test(T _input) {
    std::cout
        << "Value: " << _input << std::endl
        << "Type: " << typeid(_input).name()
        << std::endl;
}
