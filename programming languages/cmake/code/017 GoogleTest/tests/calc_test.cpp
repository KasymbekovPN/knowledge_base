/*
cmake -B .build
cmake --build .build
ctest --test-dir .build --output-on-failure
*/

#include <gtest/gtest.h>

#include "calc/calc.hpp"

TEST(CalcTest, Add) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_EQ(add(-1, 1), 0);
}

TEST(CalcTest, Divide) {
    EXPECT_EQ(divide(10, 2), 5);
}

TEST(CalcTest, DivideByZeroThrows) {
    EXPECT_THROW(divide(1, 0), std::invalid_argument);
}
