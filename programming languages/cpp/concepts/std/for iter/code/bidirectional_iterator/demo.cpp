#include <iostream>
#include <list>
#include <concepts>

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

    CounterIterator& operator--() {
        --value;
        return *this;
    }

    CounterIterator operator--(int) {
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

template<std::bidirectional_iterator T>
void test(T b, T e) {
    while (b != e) {
        --e;
        std::cout << *e << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::list l = std::list({1, 2, 3});
    test(l.begin(), l.end());

    CounterIterator ci = CounterIterator();
    ++ci;
    test(ci.begin(), ci.end());

    return 0;
}
