#pragma once

#include <stdexcept>

namespace calc::detail {
    constexpr int MIN_VALUE{-100};
    constexpr int MAX_VALUE{100};

    bool is_valid_division(int);
    int normalize(int);
}

namespace calc {
    class Calculator {
    public:
        static int add(int, int);
        static int divide(int, int);
        static int normalized_add(int, int);
    };
}
