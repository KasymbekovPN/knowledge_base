#include <iostream>
#include <unordered_map>
#include <bit>

namespace {
    struct Point {
        int x{};
        int y{};

        bool operator==(const Point& p) const {
            return x == p.x && y == p.y;
        }
    };

    std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }
}

template <>
struct std::hash<Point> {
    std::size_t operator()(const Point& p) const noexcept {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

int main(int argc, char *argv[]) {
    std::unordered_map<Point, std::string> map;
    map[{.x = 1, .y = 2}] = "a";
    map[{.x = 3, .y = 4}] = "b";

    if (const auto it{map.find({.x = 1, .y = 2})};
        it != map.end()) {
        std::cout << it->first << "\n" << std::flush;
    }

    double x{123.456};
    const std::uint64_t r{std::bit_cast<std::uint64_t>(x)};
    std::cout << r << '\n';

    return 0;
}
