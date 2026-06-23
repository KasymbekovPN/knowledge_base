#pragma once

namespace shapes {

struct Point {
    double x{};
    double y{};
};


class Rect {

public:
    Rect(const Point& _top_left, const Point& _bottom_right):
        top_left_{_top_left},
        bottom_right_{_bottom_right} {}

    double width() const;
    double height() const;
    double area() const;
    double perimeter() const;
    Point center() const;

    const Point& top_left() const { return top_left_; }
    const Point& bottom_right() const { return bottom_right_; }

private:
    Point top_left_;
    Point bottom_right_;
};

} // namespace shapes
