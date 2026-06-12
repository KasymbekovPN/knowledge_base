#include <iostream>
#include <concepts>
#include <iterator>

struct NonIncrementable {
    using differnce_type = std::ptrdiff_t;

    NonIncrementable& operator++() {
        return *this;
    }
};

struct Incrementable {
    using difference_type = std::ptrdiff_t;

    int value{};

    Incrementable& operator++() {
        ++value;
        return *this;
    }

    Incrementable operator++(int) {
        auto tmp = *this;
        ++value;
        return tmp;
    }

    bool operator==(const Incrementable&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Incrementable& _value) {
    return _os << "{" << _value.value << "}";
}

template<std::incrementable T>
void test(T&& _value, const std::string&& _lbl) {
    std::cout << "[" << _lbl << "][before] " << _value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after] " << _value << std::endl;
}

int main() {
    test(42, "primitive");
    test(Incrementable(), "custom");
    // test(NonIncrementable(), ""); // Error

    return 0;
}
