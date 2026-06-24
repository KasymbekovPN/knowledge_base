#include "test_variant.h"

#include <iostream>
#include <format>

namespace test_variant {

struct Printer: boost::static_visitor<> {
    void operator()(int _value) {
        std::cout << std::format("[boost::variant][print-int] {}\n", _value);
    }
    void operator()(const std::string& _value) {
        std::cout << std::format("[boost::variant][print-str] {}\n", _value);
    }
};

struct Sizer: boost::static_visitor<std::size_t> {
    std::size_t operator()(int) { return sizeof(int); }
    std::size_t operator()(const std::string& _value) { return _value.size(); }
};

void test() {
    boost::variant<int, std::string> v = 42;
    if (int* p = boost::get<int>(&v)) {
        std::cout << std::format("[boost::variant][has-int] {}\n", *p);
    }

    int value = boost::get<int>(v);
    std::cout << std::format("[boost::variant][value-int] {}\n", value);

    v = std::string{"hello"};
    std::cout << std::format("[boost::variant][which] {}\n", v.which());

    Printer printer;
    boost::apply_visitor(printer, v);

    Sizer sizer;
    std::cout << std::format("[boost::variant][size] {}]\n", boost::apply_visitor(sizer, v));
}

}
