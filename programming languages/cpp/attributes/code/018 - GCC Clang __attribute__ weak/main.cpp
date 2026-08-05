#include "include/libfoo.hpp"

#include <iostream>
#include <format>

void onError(const int code) {
    std::cout << std::format("[strong] code: {}\n", code);
}

int main() {
    process(100);
    process(-100);

    return 0;
}
