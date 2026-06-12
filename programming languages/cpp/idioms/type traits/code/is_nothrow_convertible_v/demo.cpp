#include <iostream>
#include <type_traits>

#define TEST(F, T) \
    std::cout \
        << #F " -> " #T " : " \
        << std::is_nothrow_convertible_v<F, T> << "\n"

struct Base {};

struct Derived: Base {};

int main() {
    std::cout << std::boolalpha;

    TEST(int, double);
    TEST(long, int);
    TEST(float, long double);

    TEST(Derived*, Base*);
    TEST(nullptr_t, void*);

    TEST(Base*, Derived*);
    
    return 0;
}
