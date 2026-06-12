#include <iostream>
#include <vector>
#include <iterator>
#include <type_traits>

struct Iterator {
    size_t size;
    int* ptr{nullptr};

    Iterator(int* _ptr, size_t _size):
        ptr{_ptr},
        size(_size) {}

    Iterator& operator++() {
        ptr++;
        return *this;
    }

    Iterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    int* begin() {
        return ptr;
    }

    bool operator==(const Iterator&) const = default;
};

template<std::permutable I>
void test(I _it) {
    std::ranges::iter_swap(_it,  _it + 1);
}

template<typename T>
requires std::same_as<T, std::vector<int>>
void print(T& _input) {
    for (auto& item: _input) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

template<typename T>
requires std::same_as<T, Iterator>
void print(T& _input) {
    for (size_t idx{0}; idx < _input.size; ++idx) {
        std::cout << _input.ptr[idx] << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char const *argv[]) {
    std::vector<int> vec = std::vector<int>({1, 2, 3});
    print(vec);

    int arr[3] {100, 101, 102};
    Iterator iter = Iterator(arr, 3);
    print(iter);

    test(vec.begin());
    print(vec);

    test(iter.begin());
    print(iter);

    return 0;
}
