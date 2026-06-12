#include <iostream>
#include <type_traits>

template<typename T>
std::enable_if_t<
    std::negation_v<
        std::is_pointer<T>
    >
>
test(T);

int main() {
    test(42);
    // test("Hello"); // Error

    return 0;
}

template<typename T>
std::enable_if_t<
    std::negation_v<
        std::is_pointer<T>
    >
>
test(T _input) {
    std::cout << "V: " << _input << std::endl;
}
