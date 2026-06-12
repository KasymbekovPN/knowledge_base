#include <iostream>
#include <concepts>
#include <vector>
#include <iterator>

struct NonIterator {};

struct CustomIterator {
    using value_type = int;
    using difference_type = std::ptrdiff_t;

    int value{};

    CustomIterator(int _value): value{_value} {};

    const int& operator*() const {
        return value;
    }
};

template<std::indirectly_readable T>
void test(const T& _it) {
    std::cout << *_it << std::endl;
}

int main() {
    int x{42};
    test(&x);

    auto&& v = std::vector<int>({1, 2, 3});
    test(v.begin());

    test(CustomIterator(42));

    // test(NonIterator()); // Error

    return 0;
}
