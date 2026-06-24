#include "test_any.h"

#include <iostream>
#include <format>
#include <vector>
#include <string>

namespace test_any {

void test() {
    std::vector<boost::any> items;
    items.push_back(42);
    items.push_back(std::string{"hello"});
    items.push_back(3.14159);

    if (int* p = boost::any_cast<int>(&items[0])) {
        std::cout << std::format("[boost::any][any_cast][0] {}\n", *p);
    }
    if (std::string* p = boost::any_cast<std::string>(&items[1])) {
        std::cout << std::format("[boost::any][any_cast][1] {}\n", *p);
    }

    try {
        boost::any_cast<int>(items[1]);
    } catch (const boost::bad_any_cast& e) {
        std::cout << std::format("[boost::any][boost::bad_any_cast] {}\n", e.what());
    }

    std::cout << std::format("[boost::any][info] {}\n", items[2].type().name());

    auto&& is_empty = [](boost::any& v) {
        std::cout << std::boolalpha << v.empty() << '\n';
    };

    boost::any a;
    is_empty(a);

    a = 1000;
    is_empty(a);

    a.clear();
    is_empty(a);
}

}