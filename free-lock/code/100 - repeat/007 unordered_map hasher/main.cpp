#include <unordered_map>
#include <string>
#include <iostream>

namespace {
    struct Point {
        int x{};
        int y{};

        bool operator==(const Point& p) const {
            return x == p.x && y == p.y;
        }
    };

    struct PointHasher {
        std::size_t operator()(const Point& p) const noexcept {
            return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
        }
    };

    std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }
}

int main() {

    std::unordered_map<Point, std::string, PointHasher> map;
    map[{.x = 1, .y = 2}] = "a";
    map[{.x = 3, .y = 4}] = "b";

    if (const auto it{map.find({.x = 1, .y = 2})};
        it != map.end()) {
        std::cout << it->first << "\n" << std::flush;
    }

    return 0;
}
