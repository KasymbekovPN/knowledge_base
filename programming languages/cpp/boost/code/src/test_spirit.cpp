#include "test_spirit.h"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <iostream>
#include <format>
#include <map>

namespace x3 = boost::spirit::x3;

namespace test_spirit {

void test() {
    auto const pair_rule =
        x3::rule<struct pair_id, std::pair<std::string, std::string>>{}
        = x3::lexeme[+x3::alnum] >> '=' >> x3::lexeme[+(x3::char_ - ';')];
    auto grammar = pair_rule % ';';

    std::string input{"name=Alice; age=30; city=NYC"};
    std::map<std::string, std::string> result;
    auto it = input.begin();
    bool ok = x3::phrase_parse(
        it,
        input.end(),
        grammar,
        x3::space,
        result
    );

    if (ok && it == input.end()) {
        for (auto& [key, value]: result) {
            std::cout << std::format("{} -> {}\n", key, value);
        }
    } else {
        std::cout << "Parser error\n";
    }
}

}
