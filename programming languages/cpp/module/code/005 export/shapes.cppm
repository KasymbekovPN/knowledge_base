module;

#include <iostream>
#include <format>

export module shapes;

export class Circle {

public:
    Circle(double _radius): radius{_radius} {}
    double area() const {
        return 2 * 3.14159 * radius * radius;
    }

private:
    double radius;
};

export struct Point3 {
    double x, y, z;

    void print() const {
        std::cout << std::format("[{}, {}, {}]\n", x, y, z);
    }
};
