module;

#include <iostream>
#include <format>

export module geo1;

export namespace geo1 {
    double area(double a) { return a * a; }
    double perimeter(double s) { return 4 * s; }

    struct Point {
        double x{};
        double y{};

        void print() const {
            std::cout << std::format("[{}, {}]\n", x, y);
        }
    };

    constexpr double PI{3.14159};
}
