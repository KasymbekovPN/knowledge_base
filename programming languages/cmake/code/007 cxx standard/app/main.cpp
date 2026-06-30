#include <mathlib/vec.hpp>

#include <iostream>
#include <optional>

int main() {
    if (auto result = safe_divide(10.0, 3.0)) {
        std::cout << *result << '\n';
    }
}
