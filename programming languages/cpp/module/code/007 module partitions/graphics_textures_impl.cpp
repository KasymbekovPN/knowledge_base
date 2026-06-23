module;

#include <iostream>
#include <format>

module graphics;

void Texture::print() const {
    allocate_it(42);
    free_it(43);
    std::cout << std::format("path: {}\n", path);
}
