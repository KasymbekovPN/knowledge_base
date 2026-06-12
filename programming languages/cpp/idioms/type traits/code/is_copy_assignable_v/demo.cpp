#include <iostream>
#include <vector>
#include <type_traits>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_copy_assignable_v<T> << "\n";

struct Point {
    int x, y;
};

class NonCopyAssignable {
public:
    NonCopyAssignable() = default;
    NonCopyAssignable& operator=(const NonCopyAssignable&) = delete;
};

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(std::string);
    TEST(std::vector<int>);
    TEST(NonCopyAssignable);
    TEST(std::unique_ptr<float>);

    return 0;
}
