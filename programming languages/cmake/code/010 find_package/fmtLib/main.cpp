/*
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=C:\projects\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build .build
*/

#include <fmt/core.h>
#include <fmt/color.h>

int main() {
    fmt::print("Hello, {}!\n", "world");

    int x{42};
    fmt::print("dec: {}, hex: {:#x}\n", x, x);

    fmt::print(fg(fmt::color::green), "color text\n");
}
