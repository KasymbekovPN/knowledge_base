#include <format>
#include <iostream>
#include <unordered_set>

#if defined(_MSC_VER)
    #define ALLOC_RETURN __declspec(restrict)
#elif defined(__GNUC__)
    #define ALLOC_RETURN __attribute__((malloc))
#else
    #define ALLOC_RETURN
#endif

namespace {

    class Point {
    public:
        static Point* instance(const double x, const double y) {
            const auto p = new Point(x, y);
            points.insert(p);

            return p;
        }

        static void freeAll() {
            const auto copy = points;
            points.clear();

            for (const auto p : copy) {
                delete p;
            }
        }

        Point() = delete;

        double x, y;
    private:
        Point(const double x, const double y) : x(x), y(y) {}

        inline static std::unordered_set<Point*> points;
    };

    ALLOC_RETURN void* pointAlloc() {
        return Point::instance(0, 0);
    }
}

int main() {
    const auto p = static_cast<Point*>(pointAlloc());
    std::cout << std::format("({}, {})\n", p->x, p->y);
    Point::freeAll();

    return 0;
}

