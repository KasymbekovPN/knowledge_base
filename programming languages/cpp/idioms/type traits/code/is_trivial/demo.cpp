#include <iostream>
#include <type_traits>
#include <cstring>

struct SimplePoint {
    double x, y;
};

struct Message {
    std::string data; // not a trivial !

    Message(): data{""} {}
    Message(const std::string& _data): data{_data} {}
    Message(const Message&) = default;
    Message& operator=(const Message&) = default;
};

template<typename T>
void test(T&, T&);

int main() {
    SimplePoint p0{1, 2}, p1;
    test(p0, p1);

    Message m0{"hello"}, m1;
    test(m0, m1);

    return 0;
}

template<typename T>
void test(T& _dst, T& _src) {
    if constexpr (std::is_trivial_v<T>) {
        std::memcpy(&_dst, &_src, sizeof(T));
        std::cout << "Copied with memcpy";
    } else {
        _dst = _src;
        std::cout << "Copied with assignment";
    }
    std::cout << std::endl;
}
