#include <iostream>
#include <concepts>

struct Meter {
    int value;
};

struct Kilometer {
    int value;
};

bool operator==(const Meter& m, const Kilometer& k) {
    return m.value == k.value * 1000;
}

bool operator==(const Kilometer& k, const Meter& m) {
    return m.value == k.value * 1000;
}

std::ostream& operator<<(std::ostream& os, const Meter& m) {
    return os << "m{" << m.value << "}";
}

std::ostream& operator<<(std::ostream& os, const Kilometer& k) {
    return os << "k{" << k.value << "}";
}

template<typename T, typename U>
concept EqualityComparable =
requires(const T& t, const U& u) {
    { t == u } -> std::convertible_to<bool>;
    { u == t } -> std::convertible_to<bool>;
};

template<typename T, typename U>
requires EqualityComparable<T, U>
void test(const T& a, const U& b) {
    std::cout
        << a << " == " << b
        << std::boolalpha
        << " :: " << (a == b)
        << '\n';
}

int main() {
    test(42, 42);
    test(3.14159, 3.14159f);
    test(Kilometer{1}, Meter{1000});
}