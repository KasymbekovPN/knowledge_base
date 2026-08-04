#include <format>
#include <iostream>
#include <format>

__attribute__((always_inline))
inline int square(const int x) { return x * x; }

// кроссплатформенный вариант, стандартный синтаксис
[[gnu::always_inline]]
inline int cube(const int x) { return x * square(x); }

int main() {
    std::cout << std::format("sqr: {}\n", square(42));
    std::cout << std::format("cude: {}\n", cube(42));

    return 0;
}

