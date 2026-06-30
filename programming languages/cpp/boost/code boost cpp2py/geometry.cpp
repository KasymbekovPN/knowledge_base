/*
cmake --preset default
cmake --build .build
& "C:\projects\vcpkg_installed\kb_cpp2py\x64-windows\tools\python3\python.exe" "C:\projects\knowledge_base\programming languages\cpp\boost\code boost cpp2py\geometry_demo.py"
*/

#include <boost/python.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <format>

class Point {
public:
    Point(): x_{0}, y_{0} {}
    Point(const double _x, const double _y): x_{_x}, y_{_y} {}

    double x() const { return x_; }
    double y() const { return y_; }
    void set_x(const double _x) { x_ = _x; }
    void set_y(const double _y) { y_ = _y; }

    double distance_to(const Point& _other) const {
        double dx = x_ - _other.x_;
        double dy = y_ - _other.y_;
        return std::sqrt(dx * dx + dy * dy);
    }

    std::string to_string() const {
        return std::format("Point({}, {})", x_, y_);
    }
private:
    double x_, y_;
};

double polygon_perimeter(const std::vector<Point>& _points) {
    if (_points.size() < 2) return 0.0;

    double total{0.0};
    for (size_t i{}; i < _points.size(); ++i) {
        const Point& a = _points[i];
        const Point& b = _points[(i + 1) % _points.size()];

        total += a.distance_to(b);
    }

    return total;
}

BOOST_PYTHON_MODULE(geometry) {
    boost::python::class_<Point>(
            "Point",
            boost::python::init<double, double>()
        )
        .def(boost::python::init<>())
        .add_property("x", &Point::x, &Point::set_x)
        .add_property("y", &Point::y, &Point::set_y)
        .def("distance_to", &Point::distance_to)
        .def("__repr__", &Point::to_string);

    boost::python::def("polygon_perimeter", polygon_perimeter);
}
