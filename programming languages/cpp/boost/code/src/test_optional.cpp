#include "test_optional.h"

#include <iostream>
#include <format>
#include <string>

namespace test_optional {

static boost::optional<int> parse(const std::string& _line) {
    if (_line == "42") { return 42; }
    return boost::none;
}

static void print(boost::optional<int> _value) {
    if (_value) {
        std::cout << std::format("[boost::optional][HAS VALUE] {}\n", *_value);
    } else {
        std::cout << std::format("[boost::optional][NO VALUE] {}\n", _value.value_or(-1));
    }
}

void test() {
    print(parse("42"));
    print(parse("xyz"));
}

}

