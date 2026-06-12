#include <iostream>
#include <type_traits>

#define TEST(F, T) \
    std::cout \
        << #F " -> " #T ": " \
        << std::is_convertible_v<F, T> << "\n"

struct Animal {
    virtual ~Animal() = default;
};

struct Dog: Animal {};

int main() {
    std::cout << std::boolalpha;

    TEST(int, double);
    TEST(double, int);
    TEST(float, long double);
    TEST(Dog*, Animal*);
    TEST(Animal*, Dog*);
    TEST(void*, int*);
    TEST(int*, void*);
    TEST(bool, int);
    TEST(nullptr_t, void*);
    
    return 0;
}
