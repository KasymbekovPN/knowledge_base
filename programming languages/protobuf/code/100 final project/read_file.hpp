#pragma once

#include <string>
#include <fstream>
#include <sstream>

namespace myapp {
    inline std::string read_file(const std::string &path) {
        const std::fstream file(path);
        std::stringstream ss;
        ss << file.rdbuf();

        return ss.str();
    }
}

