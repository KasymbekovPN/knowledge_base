#include <iostream>
#include <vector>
#include <ranges>

struct Sential {};

struct Iterator {
    using pointer = int*;
    using reference = int&;

    pointer ptr{nullptr};

    Iterator() = default;

    Iterator(pointer _p): ptr{_p} {}

    reference operator*() const {
        return *ptr;
    }

    Iterator& operator++() {
        ++ptr;
        return *this;
    }

    friend bool operator==(const Iterator& it, Sential) {
        return *it.ptr == -1;
    }
};

struct Range {
    Iterator begin() {
        return Iterator(new int{1});
    }
    Sential end() {
        return Sential();
    }
};

int main() {
    std::cout << std::ranges::common_range<std::vector<int>> << std::endl;
    std::cout << std::ranges::common_range<Range> << std::endl;

    return 0;
}
