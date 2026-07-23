#pragma once

#include <string>

namespace edit_distance {
    int min_distance(const std::string&, const std::string&);
    int min_distance_opt(const std::string&, const std::string&);
    void demo();
}