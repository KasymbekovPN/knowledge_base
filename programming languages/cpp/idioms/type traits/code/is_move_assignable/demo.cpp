#include <iostream>
#include <type_traits>
#include <vector>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_move_assignable_v<T> << "\n"

struct Point {
    int x, y;
};

class NonMoveAssignable {
public:
    NonMoveAssignable() = default;
    NonMoveAssignable& operator=(NonMoveAssignable&&) = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(std::string);
    TEST(std::vector<int>);
    TEST(std::unique_ptr<int>);
    TEST(NonMoveAssignable);
    
    return 0;
}
