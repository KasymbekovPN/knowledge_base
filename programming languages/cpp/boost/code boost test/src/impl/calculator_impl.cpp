#include "calculator_impl.hpp"

namespace calc::detail {
    bool is_valid_division(const int _division) { return _division != 0; }

    int normalize(const int _value) {
        if (_value < MIN_VALUE) { return MIN_VALUE; }
        if (_value > MAX_VALUE) { return MAX_VALUE; }
        return _value;
    }
}

namespace calc {
    int Calculator::add(const int _a, const int _b) { return _a + _b; }

    int Calculator::divide(const int _a, const int _b) {
        if (!detail::is_valid_division(_b)) {
            throw std::invalid_argument("division by zero");
        }
        return _a / _b;
    }

    int Calculator::normalized_add(const int _a, const int _b) {
        return detail::normalize(_a + _b);
    }
}
