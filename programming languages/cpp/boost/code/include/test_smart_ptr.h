#pragma once

#include <boost/smart_ptr.hpp>

namespace test_smart_ptr {
    struct Node {
        int value;
        boost::shared_ptr<Node> next;
        boost::weak_ptr<Node> prev;
        Node(int _value): value{_value} {}
    };
    void test();
}
