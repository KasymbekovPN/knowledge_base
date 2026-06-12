#include <iostream>
#include <concepts>
#include <vector>

class NoDefault {
private:
    std::string data;
public:
    NoDefault(std::string _data):
        data{_data} {}
};

class NoCopy {
public:
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

class Point {
private:
    int x, y;
public:
    Point() = default;
    Point(int _x, int _y):
        x{_x},
        y{_y} {}
    Point(const Point&) = default;

    Point& operator=(const Point&) = default;
};

template<std::semiregular T>
void test() {
    T a{};
    T b = a;
    T c;
    c = a;
    std::cout
        << typeid(a).name()
        << std::endl;
}

int main() {
    test<int>();
    test<std::string>();
    test<std::vector<int>>();
    test<Point>();
    // test<NoDefault>(); // Error
    // test<NoCopy>(); // Error

    return 0;
}
