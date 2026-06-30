#include "mathlib/vec.hpp"

#include <optional>

std::optional<double> safe_divide(const double a, const double b) {
    if (b == 0.0) return std::nullopt;
    return a / b;
}
