#include <iostream>
#include <concepts>

int sum(int x, int y) {
    return x + y;
}

auto&& lambda = [](int x, int y) -> int {
    return x + y;
};

struct Sum {
    int operator()(int x, int y) const {
        return x + y;
    }
};

template<std::invocable<int, int>  F>
requires std::invocable<F, int, int>
void test(F func, int x, int y) {
    std::cout << func(x, y) << std::endl;
}

template<typename F, typename... Args>
requires std::invocable<F, Args...>
void test2(F&& func, Args&&... args) {
    std::cout << func(args...) << std::endl;
}

int main() {
    test(sum, 2, 3);
    test(lambda, 10, 20);
    test(Sum(), 42, 43);

    test2(Sum(), 142, 43);

    return 0;
}
