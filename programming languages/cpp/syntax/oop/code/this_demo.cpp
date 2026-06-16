#include <iostream>

class Point {
private:
    int x;
    int y;
public:
    Point(int x, int y);
    Point* move(int x, int y);
    Point* moveX(int x);
    Point* moveY(int y);
    void display();
};

Point::Point(int x, int y) {
    this->x = x;
    this->y = y;
}

Point* Point::move(int x, int y) {
    this->x += x;
    this->y += y;

    return this;
}

Point* Point::moveX(int x) {
    return move(x, 0);
}

Point* Point::moveY(int y) {
    return move(0, y);
}

void Point::display() {
    std::cout
        << "{" << x
        << ", " << y
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Point* point = new Point{10, 20};
    point->display();

    point
        ->move(5, -1)
        ->moveX(10)
        ->moveY(9);
    point->display();

    return 0;
}
