#include "widget.hpp"

#include <iostream>
#include <format>

int main() {
    Widget w;
    w.setName("gadget");
    std::cout << std::format("{}\n", w.describe());
    std::cout << std::format("{}\n", w.describe());
    std::cout << std::format("sizeof(Widget) = {} bytes\n", sizeof(Widget));

    return 0;
}
