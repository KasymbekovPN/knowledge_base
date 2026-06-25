#include "test_multiprecision.h"

#include <iomanip>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <iostream>

using boost::multiprecision::cpp_int;
using boost::multiprecision::cpp_dec_float_50;

namespace test_multiprecision {

void test() {
    cpp_int power{1};
    power <<= 256;
    std::cout << std::format("2^256 = {}\n", power.str());

    cpp_int fact{1};
    for (int i{1}; i <= 30; ++i) fact *= i;
    std::cout << std::format("30! = {}\n", fact.str());

    cpp_dec_float_50 a{1};
    cpp_dec_float_50 b{3};
    std::cout
        << std::setprecision(50)
        << std::format(
            "{}/{} = {}\n",
            a.str(),
            b.str(),
            cpp_dec_float_50(a / b).str());

    cpp_int n("1000000000000000000000000000057");
    std::cout << std::format("sqrt(n) ~ {}\n", sqrt(n).str());
}

}