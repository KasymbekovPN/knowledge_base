#include "test_container.h"

#include <iostream>
#include <format>
#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/stable_vector.hpp>

namespace test_container {

namespace bc = boost::container;

static void test_static_vector() {
    bc::static_vector<int, 3> sv;
    sv.push_back(1);
    sv.push_back(2);
    sv.push_back(3);

    std::string delimiter{};
    std::cout << "[boost::container::static_vector] {";
    for (auto item: sv) {
        std::cout << delimiter << item;
        delimiter = ", ";
    }
    std::cout << "}\n";
}

static void test_small_vector() {
    bc::small_vector<int, 2> sv{1, 2, 3};
    sv.push_back(1);
    sv.push_back(2);

    std::string delimiter{};
    std::cout << "[boost::container::small_vector] {";
    for (auto item: sv) {
        std::cout << delimiter << item;
        delimiter = ", ";
    }
    std::cout << "}\n";
}

static void test_flat() {
    bc::flat_map<int, std::string> flatm;
    flatm.reserve(100);
    flatm[3] = "three";
    flatm[1] = "one";
    flatm[2] = "two";

    std::string delimiter{};
    std::cout << "[boost::container::flat_map] {";
    for (auto [k, v]: flatm) {
        std::cout << std::format("{}[{}, {}]", delimiter, k, v);
        delimiter = ", ";
    }
    std::cout << "}\n";
}

static void test_stable_vector() {
    bc::stable_vector<int> sv{1, 2, 3};
    int* p = &sv[1];
    std::cout << std::format("[boost::container::stable_vector] *p = {}\n", *p);

    sv.insert(sv.begin(), 42);
    std::cout << std::format("[boost::container::stable_vector] *p = {}\n", *p);
}


void test() {
    test_static_vector();
    test_small_vector();
    test_flat();
    test_stable_vector();
}

}
