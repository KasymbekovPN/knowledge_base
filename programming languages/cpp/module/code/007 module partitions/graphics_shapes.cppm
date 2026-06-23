module;

#include <iostream>
#include <format>

export module graphics:shapes;

export struct Point {
    double x, y;

    void print() const {
        std::cout << std::format("Point [{}, {}]\n", x, y);
    }
};
export struct Rect {
    Point tl, br;

    void print() const {
        std::cout << "Rect\n";
        tl.print();
        br.print();
    }
};
