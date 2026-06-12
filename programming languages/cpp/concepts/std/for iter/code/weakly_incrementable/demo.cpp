#include <iostream>
#include <iterator>
#include <concepts>

struct NonInc {};

struct Inc {
    using difference_type = std::ptrdiff_t;

    int value{};

    Inc& operator++() {
        ++value;
        return *this;
    }

    Inc operator++(int) {
        Inc tmp = *this;
        ++value;
        return tmp;
    }
};

std::ostream& operator<<(std::ostream& _os, const Inc& _value) {
    return _os << "{" << _value.value << "}";
}

template<std::weakly_incrementable T>
void test(T&& _value, const std::string&& _lbl) {
    std::cout << "[" << _lbl << "][before]" << _value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after]" << _value << std::endl;
}

int main() {
    test(42, "primitive");
    test(Inc(), "custom");

    return 0;
}
