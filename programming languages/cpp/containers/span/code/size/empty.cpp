#include <iostream>
#include <span>

template <typename T>
void _test_empty(std::span<T>&);

int main(int argc, char const *argv[]) {
    int array[] {1, 2, 3, 4, 5};
    std::span<int> not_empty_span {array};
    std::span<int> empty_span;

    _test_empty(not_empty_span);
    _test_empty(empty_span);

    return 0;
}

template <typename T>
void _test_empty(std::span<T>& spn) {
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << spn.empty()
        << std::noboolalpha
        << std::endl;
}
