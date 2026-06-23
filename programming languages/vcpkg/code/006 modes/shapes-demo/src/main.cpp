/*
# из каталога shapes-demo
cmake --preset default     # на этом шаге vcpkg сам поставит fmt в vcpkg_installed/
cmake --build build        # компиляция проекта

# запуск
./build/shapes-demo        # Linux/macOS
.\build\shapes-demo.exe    # Windows
*/


#include "shapes.h"

#include <fmt/core.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<shapes::Point> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const shapes::Point& p, format_context& ctx) const {
        return fmt::format_to(ctx.out(), "({:.1f}, {:.1f})", p.x, p.y);
    }
};

int main() {
    using shapes::Point;
    using shapes::Rect;

    Point tl{0.0, 4.0};
    Point br{3.0, 0.0};
    Rect rect{tl, br};

    fmt::print("Rectangle corners: {} -> {}\n",
               rect.top_left(), rect.bottom_right());
    fmt::print("Width:     {:.2f}\n", rect.width());
    fmt::print("Height:    {:.2f}\n", rect.height());
    fmt::print("Area:      {:.2f}\n", rect.area());
    fmt::print("Perimeter: {:.2f}\n", rect.perimeter());
    fmt::print("Center:    {}\n", rect.center());

    return 0;
}
