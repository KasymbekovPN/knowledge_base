#include <iostream>
#include <type_traits>

struct Empty {};

class EmptyClass {
    void method() {}
    static int value;
};

struct Base : Empty {};

struct NonEmpty {
    int x;
};

struct HasStatic {
    static const int size = 42;
    void foo() {}
};

template<typename T>
void test_print(const std::string&&);

int main() {
    test_print<Empty>("Empty");
    test_print<EmptyClass>("EmptyClass");
    test_print<Base>("Base");
    test_print<NonEmpty>("NonEmpty");
    test_print<HasStatic>("HasStatic");
    test_print<int>("int");
    test_print<void>("void");

    return 0;
}

template<typename T>
void test_print(const std::string&& _lbl) {
    std::cout << _lbl;
    if constexpr (std::is_empty_v<T>) {
        std::cout << " - empty";
    } else {
        std::cout << " - no empty";
    }
    std::cout << ::std::endl;
}
