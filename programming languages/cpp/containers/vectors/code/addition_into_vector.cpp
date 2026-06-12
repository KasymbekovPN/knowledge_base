#include <iostream>
#include <vector>

class Point {

private:
    std::string id;
    float x;
    float y;

public:
    explicit Point(std::string id, float x, float y) noexcept:
        id{id},
        x{x},
        y{y} {}
    virtual void print() const noexcept {
        std::cout
            << id << " {x: " << x
            << ", y: " << y
            << "}" << std::endl;
    }
    virtual void setCoordinates(float x, float y) noexcept {
        this->x = x;
        this->y = y;
    }
};

void print_vector(const std::vector<Point>) noexcept;

int main(int argc, char const *argv[]) {
    Point p0 {"p0", 0, 0};
    Point p1 {"p1", 1, 1};
    Point p2 {"p2", 2, 2};
    p0.print();
    p1.print();
    p2.print();

    std::vector<Point> vector;
    vector.push_back(p0);
    vector.push_back(p1);
    vector.emplace_back(p2);
    ::print_vector(vector);

    p0.setCoordinates(100, 100);
    p1.setCoordinates(101, 101);
    p2.setCoordinates(102, 102);
    ::print_vector(vector);

    return 0;
}

void print_vector(const std::vector<Point> vector) noexcept {
    std::cout << "-----" << std::endl;
    for (auto it {vector.begin()}; it != vector.end(); it++) {
        it->print();
    }
}