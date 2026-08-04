#include <iostream>
#include <format>
#include <stdexcept>

[[noreturn]] static void fail(const char* msg) {
    throw std::runtime_error(msg);
}

static int process(const int x) {
    if (x < 0) fail("negative value");
    return 2 * x;
}

int main() {
    try {
        std::cout << process(-1) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << std::format("caught: {}\n", e.what());
    }
}
