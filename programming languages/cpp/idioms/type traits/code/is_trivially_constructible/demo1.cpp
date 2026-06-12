#include <iostream>
#include <type_traits>

#define TEST(T, ...) \
    std::cout \
        << #T "<" #__VA_ARGS__ ">: " \
        << std::is_trivially_constructible_v<T, __VA_ARGS__> << "\n"

struct Point {
    int x, y;
};

struct TrivialClass {
    int a;
    double b;
};

class NotTrivialClass {
public:
    int value {0};
    NotTrivialClass() {}
};

int main() {
    std::cout << std::boolalpha;

    TEST(int, int);

    TEST(Point, Point);
    TEST(Point, int, int);
    TEST(Point, int, Point);

    TEST(TrivialClass, int, double);
    TEST(TrivialClass, int, std::string);

    TEST(NotTrivialClass, int);

    return 0;
}
