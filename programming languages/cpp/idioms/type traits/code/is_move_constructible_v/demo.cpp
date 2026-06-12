#include <iostream>
#include <type_traits>
#include <thread>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_move_constructible_v<T> << "\n";

struct Point {
    int x, y;
};

class NonMovable {
    NonMovable() = default;
    NonMovable(NonMovable&&) = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(Point);
    TEST(std::string);
    TEST(std::unique_ptr<int>);
    TEST(std::thread);
    TEST(NonMovable);
    
    return 0;
}
