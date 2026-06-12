#include <iostream>
#include <type_traits>

#define TEST(B, D)\
    std::cout \
        << #B " is base of " #D ": " \
        << std::is_base_of_v<B, D> << "\n"

struct Animal {
    virtual ~Animal() = default;
};

struct Dog: Animal {};

struct Cat: Animal {};

struct Bulldog: public Dog {};


int main() {
    std::cout << std::boolalpha;

    TEST(Animal, Dog);
    TEST(Animal, Bulldog);
    TEST(Dog, Bulldog);
    TEST(Cat, Dog);
    TEST(Dog, Dog);

    return 0;
}
