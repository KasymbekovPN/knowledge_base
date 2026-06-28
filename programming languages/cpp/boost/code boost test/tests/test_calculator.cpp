#include <boost/test/unit_test.hpp>

#include "../src/impl/calculator_impl.hpp"

BOOST_AUTO_TEST_SUITE(public_api_tests)

BOOST_AUTO_TEST_CASE(add_works) {
    BOOST_TEST(calc::Calculator::add(2, 3) == 5);
    BOOST_TEST(calc::Calculator::add(-5, 5) == 0);
}

BOOST_AUTO_TEST_CASE(divide_works) {
    BOOST_TEST(calc::Calculator::divide(10, 2) == 5);
}

BOOST_AUTO_TEST_CASE(divide_by_zero_throws) {
    BOOST_CHECK_THROW(calc::Calculator:divide(1, 0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(normalized_add_clamps) {
    BOOST_TEST(calc::Calculator::normalized_add((60, 60)) == 100);
    BOOST_TEST(calc::Calculator::normalized_add((10, 60)) == 70);
}

BOOST_AUTO_TEST_SUITE_END()