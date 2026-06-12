#include <iostream>
#include <type_traits>

template<typename T>
void test(const T&);

void func();

int main() {
    void(*pf)() = &func;

    test(42);
    test("Hello");
    test(func);
    test(pf);

    return 0;
}

template<typename T>
void test(const T& _input) {
    if constexpr (std::is_function_v<T>) {
        std::cout << "This is a function type";
    } else if (std::is_pointer_v<T> &&
               std::is_function_v<std::remove_pointer_t<T>>) {
        std::cout << "This is a pointer to function";
    } else {
        std::cout << "Other";
    }
    std::cout << std::endl;
}

void func() {}
