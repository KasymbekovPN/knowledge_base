#include <iostream>
#include <type_traits>

struct Point { double x, y; };

enum class Status { Ok, Error };

class Vec3 {
private:
    double x, y, z;
public:
    Vec3(double _x, double _y, double _z): x{_x}, y{_y}, z{_z} {}
};

template<typename T>
void test(const T&);

int main() {
    test(42);
    test("hello");
    test(Vec3{1, 2, 3});
    test(Point());
    test(Status::Ok);

    return 0;
}

template<typename T>
void test(const T& _input) {
    if constexpr (std::is_class_v<T>) {
        std::cout << "A class type";
    } else {
        std::cout << "Not a class type";
    }
    std::cout << std::endl;
}
