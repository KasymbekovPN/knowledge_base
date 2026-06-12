#include <iostream>
#include <algorithm>
#include <ranges>
#include <vector>

struct Buffer {
    size_t size{};
    int* data{nullptr};

    Buffer(int* _data, size_t _size):
        data{_data},
        size(_size) {}

    Buffer& operator++() {
        data++;
        return *this;
    }

    Buffer operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    int* begin() {
        return data;
    }

    int* end() {
        return data + size;
    }

    bool operator==(const Buffer&) const = default;
    auto operator<=>(const Buffer&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Buffer& _buffer) {
    auto&& delimiter = std::string("");
    _os << "{";
    for (size_t i{}; i < _buffer.size; ++i) {
        _os << delimiter << _buffer.data[i];
        delimiter = " ";
    }
    std::cout << "}";
    
    return _os;
}

template<std::sortable T>
void test(T&& _begin, T&& _end) {
    std::ranges::sort(_begin, _end);
}

int main() {
    std::vector<int> vec{2, 1, 5, 4, 2};
    for (int item: vec) { std::cout << item << " "; }
    std::cout << std::endl;

    test(vec.begin(), vec.end());
    for (int item: vec) { std::cout << item << " "; }
    std::cout << std::endl;

    const size_t SIZE{5};
    int arr[SIZE] {8, 9, 7, 5, 1};
    auto&& buffer = Buffer(arr, SIZE);
    std::cout << buffer << std::endl;

    test(buffer.begin(), buffer.end());
    std::cout << buffer << std::endl;

    return 0;
}
