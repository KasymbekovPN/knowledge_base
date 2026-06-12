#include <iostream>
#include <type_traits>

struct Base {};

struct FinalClass final {};

struct AnotherFinal final {
    int data;
};

template<typename T>
struct Wrapper final {
    T value;
};

template<typename T>
void test(const std::string&&);

int main() {
    test<Base>("Base");
    test<FinalClass>("FinalClass");
    test<AnotherFinal>("AnotherFinal");
    test<Wrapper<int>>("Wrapper<int>");
    test<int>("int");
    
    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    constexpr bool is_final = std::is_final_v<T>;
    std::cout
        << "[" << _lbl << "]: "
        << std::boolalpha
        << is_final
        << std::noboolalpha
        << std::endl;
}
