#include <iostream>
#include <concepts>
#include <utility>

template<typename T, typename U>
requires std::swappable_with<T, U>
void exchange(T&& a, U&& b) {
    using std::swap;
    swap(a, b);
}

int main() {
    int a = 10;
    int b = 20;

    exchange(a, b);

    std::cout << a << " " << b << std::endl;
}
