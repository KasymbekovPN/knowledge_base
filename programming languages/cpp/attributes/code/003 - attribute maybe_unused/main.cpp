#include <iostream>
#include <format>
#include <mutex>

static void process([[maybe_unused]] const int logLevel) {
#ifndef NDEBUG
    std::cout << std::format("debug level: {}\n", logLevel);
#endif
}

static int value{};
static std::mutex mtx;

int main() {
    {
        [[maybe_unused]] std::lock_guard<std::mutex> lock(mtx);
    }
    process(1);

    return 0;
}
