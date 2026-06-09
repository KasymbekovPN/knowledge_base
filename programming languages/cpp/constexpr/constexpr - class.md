---
tags:
  - programming-language
  - cpp
  - constants
  - constexpr
---
[[programming languages/cpp/constexpr/_|<=]]

```cpp
#include <iostream>

class Point {
private:
    double x, y;

public:
    constexpr Point(double _x, double _y): x{_x}, y{_y} {}
    constexpr double distance() const {
        return x*x + y*y;
    }
};

int main() {
    constexpr Point p0{3, 4};
    constexpr double d0 = p0.distance();
    std::cout << d0 << std::endl;

    constexpr Point p1{4, 5};
    double d1 = p1.distance();
    std::cout << d1 << std::endl;

    Point p2{5, 6};
    double d2 = p2.distance();
    std::cout << d2 << std::endl;

    return 0;
}
```

```
25
41
61
```
