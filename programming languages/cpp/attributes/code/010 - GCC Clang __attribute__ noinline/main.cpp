#include <format>
#include <format>
#include <iostream>
#include <format>

namespace {
    __attribute__((noinline))
    void logError(const int code) {
        std::cout << std::format("code: {}\n", code);
    }

    __attribute__((noinline))
    void handleError(const int code) {
        // редкий "холодный" путь — не хотим раздувать icache горячего кода
        logError(code);
        std::abort();
    }

    [[gnu::noinline]]
    void debugBreakpoint(const int value) {
        // нужна отдельная функция для удобной постановки breakpoint / видимости в стеке
        std::cout << std::format("bp: {}\n", value);
    }
}

int main() {
    debugBreakpoint(42);
    handleError(-42);

    return 0;
}
