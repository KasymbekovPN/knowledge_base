#include "test_regex.h"

#include <boost/regex.hpp>
#include <iostream>
#include <format>

namespace test_regex {

void test() {
    std::string log =
        "2025-01-15 ERROR disk full\n"
        "2025-01-16 INFO ok\n"
        "2025-01-17 ERROR timeout\n";

    boost::regex re{R"((\d{4}-\d{2}-\d{2})\s+(ERROR|INFO)\s+([^\n]+))"};
    auto begin = boost::sregex_iterator(log.begin(), log.end(), re);
    auto end = boost::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        const boost::smatch& match = *it;
        if (match[2] == "ERROR") {
            std::cout << std::format("{} -> {}\n", match[1].str(), match[3].str());
        }
    }
}

}
