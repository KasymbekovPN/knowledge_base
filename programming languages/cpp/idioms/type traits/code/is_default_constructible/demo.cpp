#include <iostream>
#include <type_traits>

struct Point {
    int x, y;
};

class NonDefault {
public:
    explicit NonDefault(int value) {}
};

class Defaultable {
public:
    int x;
    Defaultable() = default;
};

class NoDefault {
public:
    NoDefault() = default;
};

template<typename T>
void test(const std::string&&);

int main() {
    test<int>("int");
    test<double>("double");
    test<Point>("Point");
    test<Defaultable>("Defaultable");
    test<NonDefault>("NonDefault");
    test<NoDefault>("NoDefault");
    test<std::string>("std::string");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    constexpr bool is_dc = std::is_default_constructible_v<T>;
    std::cout << "[" << _lbl << "]: "
        << std::boolalpha
        << is_dc
        << std::noboolalpha
        << std::endl;
}
