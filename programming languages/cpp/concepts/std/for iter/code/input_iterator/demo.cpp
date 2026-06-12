#include <iostream>
#include <vector>
#include <concepts>
#include <iterator>

struct Bad {};

struct CounterIterator {
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    int value{};

    int operator*() const {
        return value;
    }

    CounterIterator& operator++() {
        ++value;
        return *this;
    }

    CounterIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const CounterIterator&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const CounterIterator& _value) {
    return _os << "{" << _value.value << "}";
}

template<std::input_iterator T>
void test(T&& _value, const std::string&& _lbl){
    std::cout << "[" << _lbl << "][before] " << *_value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after] " << *_value << std::endl;
}

int main() {
    test(std::vector<int>({1, 2, 3}).begin(), "primitive");
    test(CounterIterator(), "custom");
    // test(Bad(), ""); // Error

    return 0;
}
