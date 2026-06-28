#include <boost/test/unit_test.hpp>
#include "../src/impl/calculator_impl.hpp"

BOOST_AUTO_TEST_SUITE(internal_tests)

    BOOST_AUTO_TEST_CASE(is_valid_divisor) {
    BOOST_TEST(calc::detail::is_valid_divisor(5));
    BOOST_TEST(!calc::detail::is_valid_divisor(0));
    BOOST_TEST(calc::detail::is_valid_divisor(-3));
}

BOOST_AUTO_TEST_CASE(normalize_boundaries) {
    BOOST_TEST(calc::detail::normalize(50) == 50);
    BOOST_TEST(calc::detail::normalize(150) == 100);
    BOOST_TEST(calc::detail::normalize(-150) == -100);
    BOOST_TEST(calc::detail::normalize(0) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
