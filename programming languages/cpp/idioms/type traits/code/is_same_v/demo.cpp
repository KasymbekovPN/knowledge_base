#include <iostream>
#include <type_traits>

#define TEST(T0, T1) \
    std::cout \
        << #T0 " == " #T1 ": " \
        << std::is_same_v<T0, T1> << "\n"

int main() {
    std::cout << std::boolalpha;

    TEST(int, int);
    TEST(int, unsigned);
    TEST(int, long);
    TEST(const int, int);
    TEST(int&, int);
    TEST(int&, int&);
    TEST(const char*, const char*);
    
    return 0;
}
