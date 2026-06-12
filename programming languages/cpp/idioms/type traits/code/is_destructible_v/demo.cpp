#include <iostream>
#include <type_traits>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_destructible_v<T> << "\n"

struct Point {
    int x, y;
};

class NonDestructible {
public:
    ~NonDestructible() = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(NonDestructible);
    TEST(std::string);
    TEST(std::unique_ptr<int>);

    return 0;
}
