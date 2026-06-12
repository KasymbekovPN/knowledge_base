#include <iostream>
#include <concepts>
#include <vector>
#include <iterator>

struct Reader {
    int operator*() {
        return 0;
    }
};

struct Writer {
    using difference_type = std::ptrdiff_t;

    int value{};

    Writer& operator*() {
        return *this;
    }

    Writer& operator=(int _value) {
        value = _value;
        return *this;
    }

    Writer& operator++() {
        value++;
        return *this;
    }

    Writer& operator++(int) {
        value++;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& _os, const Writer& _w) {
    return _os << "{" << _w.value << "}";
}

template<std::output_iterator<int> T>
void test(T&& _value, const std::string&& _lbl) {
    std::cout << "[" << _lbl << "][before] " << *_value << std::endl;
    ++_value;
    std::cout << "[" << _lbl << "][after] " << *_value << std::endl;
}

int main() {
    test(std::vector({1, 2, 3}). begin(), "primitive");
    test(Writer(), "custom");
    // test(Reader(), ""); // Error

    return 0;
}
