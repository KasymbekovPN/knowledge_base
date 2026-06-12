#include <iostream>
#include <type_traits>

#define TEST(T, ...) \
    std::cout \
        << #T "<" #__VA_ARGS__ ">:" \
        << std::is_nothrow_constructible_v<T, __VA_ARGS__> << "\n";

struct Trivial {
    int value;
};

struct ThrowClass {
    ThrowClass() {}
};

struct NoThrowClass {
    NoThrowClass() noexcept {}
};

class Wrapper {
    int value;
public:
    explicit Wrapper(int _value) noexcept:
        value{_value} {}
};

int main() {
    std::cout << std::boolalpha;

    TEST(Trivial);
    TEST(Trivial, int);
    TEST(NoThrowClass);
    TEST(ThrowClass);
    TEST(Wrapper, int);
    TEST(int);
    
    return 0;
}
