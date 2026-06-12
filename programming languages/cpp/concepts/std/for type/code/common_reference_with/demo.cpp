#include <iostream>
#include <concepts>

struct Number {
    int value;

    operator int() const { return value; }
};

template<typename T, typename U>
void test() {
    if constexpr (std::common_reference_with<T, U>) {
        std::cout
            << "Common refertence types exist: "
            << typeid(
                std::common_reference_t<T, U>
            ).name();
    } else {
        std::cout
            << "No common reference between types";
    }
    std::cout << std::endl;
}

int main() {
    test<int, int>();
    test<float, double>();
    test<Number, int>();
    test<int, std::string>();

    return 0;
}
