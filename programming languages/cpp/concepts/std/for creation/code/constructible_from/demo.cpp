#include <iostream>
#include <concepts>
#include <vector>
#include <string>

template<typename T, typename... Args>
requires std::constructible_from<T, Args...>
T create(Args&&... _args) {
    return T(std::forward<Args>(_args)...);
}

template<typename T>
void test(T&& _input) {
    std::cout
        << typeid(_input).name()
        << std::endl;
}

int main() {
    test(create<int>(42));
    test(create<std::string>("42"));
    test(create<std::vector<int>>(std::vector<int>({1, 2, 3})));

    return 0;
}
