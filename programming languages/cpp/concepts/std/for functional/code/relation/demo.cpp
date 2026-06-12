#include <iostream>
#include <concepts>

struct Compare {
    bool operator()(int a, double b) const { return a < b; }
    bool operator()(double a, int b) const { return a < b; }
    bool operator()(int a, int b) const { return a < b; }
    bool operator()(double a, double b) const { return a < b; }
};

template<
    typename T,
    typename U,
    std::relation<T, U> R
>
void test(T t, U u, R r) {
    std::cout
        << std::boolalpha
        << r(t, u)
        << std::noboolalpha
        << std::endl;
}

int main() {
    test(10, 20.0, std::ranges::less{});
    test(200.0, 42, Compare());

    return 0;
}
