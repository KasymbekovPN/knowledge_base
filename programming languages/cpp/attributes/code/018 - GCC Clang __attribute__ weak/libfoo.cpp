#include "include/libfoo.hpp"

#include <format>
#include <iostream>

__attribute__((weak))
void onError(const int code) {
    std::cout << std::format("[weak] code: {}\n", code);
}

void process(const int value) {
    if (value <= 0) {
        onError(-1);
        return;
    }

    std::cout << "[ok]\n";
}