#include <iostream>
#include <type_traits>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_copy_constructible_v<T> << "\n";

struct Point {
    int x, y;
};

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
};

class Copyable {
public:
    Copyable() = default;
    Copyable(const Copyable&) = default;
};

using UniqInteger = std::unique_ptr<int>;

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(NonCopyable);
    TEST(Copyable);
    TEST(UniqInteger);
    TEST(std::string);
    
    return 0;
}
