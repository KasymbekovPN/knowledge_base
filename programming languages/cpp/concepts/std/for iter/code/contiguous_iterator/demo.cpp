#include <iostream>
#include <iterator>
#include <concepts>
#include <vector>
#include <string>

struct Iterator {
    using value_type = int;
    using element_type = int;
    using difference_type = std::ptrdiff_t;

    using pointer = int*;
    using reference = int&;

    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::contiguous_iterator_tag;

    int* ptr{nullptr};

    Iterator() = default;

    Iterator(int* _ptr): ptr{_ptr} {}

    reference operator*() const {
        return *ptr;
    }

    pointer operator->() const {
        return ptr;
    }

    reference operator[](difference_type _n) const {
        return ptr[_n];
    } 

    Iterator& operator++() {
        ++ptr;
        return *this;
    }

    Iterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    Iterator& operator--() {
        --ptr;
        return *this;
    }

    Iterator operator--(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    Iterator& operator+=(difference_type _n) {
        ptr += _n;
        return *this;
    }

    Iterator& operator-=(difference_type _n) {
        ptr -= _n;
        return *this;
    }

    friend pointer to_address(const Iterator& _it) noexcept {
        return _it.ptr;
    }

    friend Iterator operator+(Iterator _it, difference_type _n) {
        _it += _n;
        return _it;
    }

    friend Iterator operator+(difference_type _n, Iterator _it) {
        _it += _n;
        return _it;
    }

    friend Iterator operator-(Iterator _it, difference_type _n) {
        _it -= _n;
        return _it;
    }

    friend difference_type operator-(const Iterator& _a, const Iterator& _b) {
        return _a.ptr - _b.ptr;
    }

    bool operator==(const Iterator&) const = default;
    auto operator<=>(const Iterator&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Iterator& _it) {
    auto&& p = _it.ptr;
    return _os 
        << "{" 
        << (p ? std::to_string(*p) : "nullptr")
        << "}" << std::endl;
}

template<std::contiguous_iterator T>
void test(T&& _it) {
    std::cout << _it[2] << std::endl;
}

int main() {
    std::vector v = std::vector({101, 102 , 103});
    test(v.begin());

    int arr[] {1, 2, 3};
    test(Iterator(arr));

    return 0;
}
