#include <iostream>
#include <concepts>

int sum(int x, int y) {
    return x + y;
}

auto&& lambda = [](int x, int y) -> int {
    return x + y;
};

struct Sum {
    int value{};

    int operator()(int x, int y) {
        ++value;
        return x + y;
    }
};

template<typename F, typename... Args>
requires std::regular_invocable<F, Args...>
void test(F&& func, Args&&... args) {
    std::cout << func(args...) << std::endl;
}

int main() {
    test(sum, 2, 3);
    test(lambda, 10, 20);
    test(Sum(), 42, 43); // No error !!!

    return 0;
}
