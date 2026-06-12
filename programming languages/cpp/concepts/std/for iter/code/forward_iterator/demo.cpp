#include <iostream>
#include <iterator>
#include <concepts>
#include <vector>

struct CounterIterator {
    using value_tyep = int;
    using difference_type = std::ptrdiff_t;

    int value{0};

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

    const int* begin() {
        return &value;
    }

    const int* end() {
        return begin() + 1;
    }
};

std::ostream& operator<<(std::ostream& _os, const CounterIterator& _ci) {
    return _os << "{" << _ci.value << "}";
}

template<std::forward_iterator T>
void test(T b, T e) {
    for (auto it{b}; it != e; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    for (auto it{b}; it != e; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector v = std::vector({1, 2, 3});
    test(v.begin(), v.end());

    auto c = CounterIterator();
    c++;
    test(c.begin(), c.end());

    return 0;
}
