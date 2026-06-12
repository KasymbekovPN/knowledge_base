#include <iostream>
#include <type_traits>

struct Interface {
    virtual void do_sth() = 0;
};

struct AbstractBase {
    virtual ~AbstractBase() = 0;
};

struct Animal {
    virtual void speak() = 0;
    virtual ~Animal() = default;
};

struct Dog: Animal {
    void speak() override {}
};

struct Empty {};

template<typename T>
void test(const std::string&&);

int main() {
    test<Interface>("Interface");
    test<AbstractBase>("AbstractBase");
    test<Animal>("Animal");
    test<Dog>("Dog");
    test<Empty>("Empty");
    test<int>("int");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    std::cout << "[" << _lbl << "]: ";
    constexpr bool is_abstract = std::is_abstract_v<T>;
    std::cout
        << std::boolalpha
        << is_abstract
        << std::noboolalpha
        << std::endl;
}
