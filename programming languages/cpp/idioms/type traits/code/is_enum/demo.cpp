#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

enum OldStyle {O0 = 0, O1 = 1, O2 = 2};
enum class NewStyle {N0, N1, N2};

int main() {
    test(O1);
    test(NewStyle::N2);
    test(42);
    test("hello");

    return 0;
}

template<typename T>
void test(const T& _value) {
    if constexpr (std::is_enum_v<T>) {
        std::cout
            << "ENUM:"
            << static_cast<std::underlying_type_t<T>>(_value)
            << std::endl;
    } else {
        std::cout << "Other" << std::endl;
    }
}
