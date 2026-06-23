module;

#include <iostream>
#include <format>

export module graphics:gpu;

int allocate_it(int bytes) {
    std::cout << std::format("allocated {}\n", bytes);
    return 0;
}

void free_it(int handle) {
    std::cout << std::format("free {}\n", handle);
}
