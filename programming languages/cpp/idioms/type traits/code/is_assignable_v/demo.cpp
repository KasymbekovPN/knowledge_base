#include <iostream>
#include <type_traits>

#define TEST(T, U) \
    std::cout \
        << #T " = " #U ": " \
        << std::is_assignable_v<T, U> << "\n"

int main() {
    std::cout << std::boolalpha;

    TEST(int, int);
    TEST(int, double);
    TEST(double, int);

    TEST(int&, int);
    TEST(int&, const int&);
    TEST(const int&, int);

    TEST(int*, int*);
    TEST(void*, int*);
    TEST(int*, void*);
    
    return 0;
}
