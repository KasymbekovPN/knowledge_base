#include <iostream>
#include <concepts>
#include <vector>

struct NoMove {
    NoMove(NoMove&&) = delete;
    NoMove& operator=(NoMove&&) = delete;
};

class Point {
private:
    int x, y;

public:
    Point(const Point& _other) = default;
    Point(Point&& _other): x{_other.x}, y{_other.y} {
        _other.x = _other.y = 0;
    }
    Point& operator=(const Point&) = default;
    Point& operator=(Point&&) = default;
};

template<std::movable T>
void test() {
    std::cout
        << typeid(T).name()
        << std::endl;
}

int main() {
    test<int>();
    test<double>();
    test<std::string>();
    test<std::vector<int>>();
    test<std::unique_ptr<int>>();
    test<Point>();
    // test<NoMove>(); // Error

    return 0;
}
