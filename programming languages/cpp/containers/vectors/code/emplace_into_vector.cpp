#include <iostream>
#include <vector>

class Point {

private:
    float x;
    float y;

public:
    Point(float x, float y) noexcept:
        x{x},
        y{y} {}
    Point(const Point& point):
        x{point.x},
        y{point.y} {
        
        std::cout << "Copy!" << std::endl;
    }
    void print() const {
        std::cout
            << "{" << x
            << ", " << y
            << "}" << std::endl;
    }
};

void print_vector(const std::vector<Point>&);

int main(int argc, char const *argv[]) {
    std::vector<Point> points {{1, 2}, {2, 3}};
    print_vector(points);

    points.emplace(points.begin() + 1, 3, 4);
    print_vector(points);

    return 0;
}

void print_vector(const std::vector<Point>& vector) {
    std::cout << "#######" << std::endl;
    for (const auto &point: vector) {
        point.print();
    }
}
