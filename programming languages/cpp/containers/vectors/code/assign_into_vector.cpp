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
    void print(bool rn = false) const {
        std::cout << "{" << x << ", " << y << "} ";
        if (rn) {
            std::cout << std::endl;
        }
    }
};

void print_vector(const std::vector<Point>&);

int main(int argc, char const *argv[]) {
    std::vector<Point> original_vector {{1, 2}, {3, 4}};
    std::vector<Point> target_vector;

    target_vector.assign(original_vector.begin(), original_vector.end());
    print_vector(target_vector);

    target_vector.assign(7, {42, 12});
    print_vector(target_vector);

    target_vector.assign({{1, 1}, {2, 2}, {3, 3}});
    print_vector(target_vector);

    return 0;
}

void print_vector(const std::vector<Point>& vector) {
    for (const auto &point: vector) {
        point.print();
    }
    std::cout << std::endl;
}
