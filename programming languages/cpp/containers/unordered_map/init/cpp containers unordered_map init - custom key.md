---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers unordered_map - init|<=]]

```cpp
#include <iostream>
#include <ostream>
#include <unordered_map>

template <typename K, typename V, typename H, typename E>
void _print_map(const std::unordered_map<K, V, H, E>&);

struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
    template<>
    struct hash<Point> {
        size_t operator()(const Point& point) const {
            return
	            std::hash<int>()(point.x) ^
	            (std::hash<int>()(point.y) << 1);
        }
    };
} // namespace std

std::ostream& operator<<(std::ostream& ostr, const Point& point) {
    return ostr << "{" << point.x << ", " << point.y << "} ";
}

int main() {
    std::unordered_map<Point, std::string> points {
        {{0, 0}, "zero"},
        {{1, 1}, "one"}
    };
    _print_map(points);

    return 0;
}

template <typename K, typename V, typename H, typename E>
void _print_map(const std::unordered_map<K, V, H, E>& uomap) {
    for (auto &[key, value]: uomap) {
        std::cout
            << "{ " << key
            << ", " << value
            << " }" << std::endl;
    }
}
```

```
{ {0, 0} , zero }
{ {1, 1} , one }
```
