#include <iostream>
#include <type_traits>

#define TEST(T, U) \
    std::cout \
        << #T " & " #U ": "\
        << std::boolalpha \
        << std::is_same_v<T, U> \
        << std::noboolalpha \
        << std::endl;

using UType = std::conditional_t<sizeof(void*) == 8, uint64_t, uint32_t>;

int main() {
    TEST(uint32_t, UType);
    TEST(uint64_t, UType);
    std::cout << typeid(UType).name() << std::endl;
    
    return 0;
}
