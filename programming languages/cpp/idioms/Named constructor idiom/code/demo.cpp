#include <iostream>
#include <cmath>

class Point {
private:
    double x, y;

    Point(double _x, double _y): 
        x{_x},
        y{_y} {}

public:
    static Point Cartesian(double _x, double _y) {
        return Point(_x, _y);
    }

    static Point Polar(double _r, double _theta) {
        return Point(_r * cos(_theta), _r * sin(_theta));
    }

    void print() const {
        std::cout
            << "Point{"
            << x << ", "
            << y << "}"
            << std::endl;
    }
};

int main() {
    auto p0 = Point::Cartesian(3, 4);
    auto p1 = Point::Polar(5, 3.12159 / 4);

    p0.print();
    p1.print();

    return 0;
}
