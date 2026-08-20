#pragma once

#include <fstream>
#include <vector>

namespace read {
    inline std::vector<uint8_t> wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }
}
