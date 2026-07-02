/*
cmake -B .build
cmake --build .build
ctest --test-dir .build --output-on-failure
*/

#include <catch2/catch_test_macros.hpp>
#include "calc/calc.hpp"

TEST_CASE("Add", "[calc]") {
    REQUIRE(add(2, 3) == 5);
    REQUIRE(add(-1, 1) == 0);
}

TEST_CASE("Divide", "[calc]") {
    REQUIRE(divide(10, 2) == 5);

    SECTION("divide by zeto throws exception") {
        REQUIRE_THROWS_AS(divide(1, 0), std::invalid_argument);
    }
}
