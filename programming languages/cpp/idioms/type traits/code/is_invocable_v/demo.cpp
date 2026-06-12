#include <iostream>
#include <type_traits>

#define TEST(T, ...) \
    std::cout \
        << #T "<" #__VA_ARGS__ ">:" \
        << std::is_invocable_v<T, __VA_ARGS__> << "\n"

void func(int _value) {}

auto lambda = [](double _value) { return 2 * _value; };

struct Callable {
    int operator()(int _a, int _b) { return _a + _b; }
};

int main() {
    std::cout << std::boolalpha;
    
    TEST(decltype(func), int);
    TEST(decltype(lambda), int);
    TEST(decltype(lambda), double);
    TEST(Callable, int, int);

    return 0;
}
