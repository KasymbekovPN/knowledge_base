#include <iostream>
#include <type_traits>

#define TEST(R, T, ...) \
    std::cout \
        << #R " " #T "(" #__VA_ARGS__ "):" \
        << std::is_invocable_r_v<R, T, __VA_ARGS__> << "\n"

int func (double _input) {
    return static_cast<int>(_input * 2);
}

auto lambda = [](int _a, int _b) -> double {
    return _a + _b;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int, decltype(func), double);
    TEST(double, decltype(func), double);
    TEST(void, decltype(func), double);

    TEST(double, decltype(lambda), int, int);
    TEST(float, decltype(lambda), int, int);
    TEST(int, decltype(lambda), int, int);

    TEST(std::string, decltype(lambda), int, int);

    return 0;
}
