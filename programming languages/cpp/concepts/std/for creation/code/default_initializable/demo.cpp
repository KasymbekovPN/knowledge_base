#include <iostream>
#include <concepts>

struct DefaultConstructible {
    int x, y;
};

struct NoDefaultConstructible {
    int x, y;

    NoDefaultConstructible(int _x, int _y):
        x{_x},
        y{_y} {}
};

template<std::default_initializable T>
T create() {
    T obj{};
    return obj;
}

template<typename T>
void test(T&& _input) {
    std::cout
        << typeid(_input).name()
        << std::endl;
}

int main() {
    test(create<int>());
    test(create<std::string>());
    test(create<DefaultConstructible>());
    // test(create<NoDefaultConstructible>()); // Error

    return 0;
}
