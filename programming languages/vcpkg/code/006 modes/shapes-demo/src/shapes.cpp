#include "shapes.h"

#include <cmath>


namespace shapes {

double Rect::width() const {
    return std::abs(bottom_right_.x - top_left_.x);
}

double Rect::height() const {
    return std::abs(top_left_.y - bottom_right_.y);
}

double Rect::area() const {
    return width() * height();
}

double Rect::perimeter() const {
    return 2.0 * (width() + height());
}

Point Rect::center() const {
    return Point{
        (top_left_.x + bottom_right_.x) / 2.0,
        (top_left_.y + bottom_right_.y) / 2.0
    };
}

} // namespace shapes
