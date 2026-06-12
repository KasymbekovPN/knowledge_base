#include <iostream>
#include <span>

template <typename T>
void _test_size_bytes(std::span<T>&);

int main(int argc, char const *argv[]) {
    int array[] {1, 2, 3, 4, 5};
    std::span<int> not_empty_span {array};
    std::span<int> empty_span;

    _test_size_bytes(not_empty_span);
    _test_size_bytes(empty_span);

    return 0;
}

template <typename T>
void _test_size_bytes(std::span<T>& spn) {
    std::cout << "Size: " << spn.size_bytes() << std::endl;
}
