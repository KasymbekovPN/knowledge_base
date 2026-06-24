#include "test_smart_ptr.h"

#include <iostream>
#include <format>

namespace test_smart_ptr {

void test() {
    auto&& a = boost::make_shared<Node>(42);
    auto&& b = boost::make_shared<Node>(12);

    a->next = b;
    b->prev = a;

    std::cout << std::format("a.use_count: {}\n", a.use_count());
    std::cout << std::format("b.use_count: {}\n", b.use_count());

    if (auto p = b->prev.lock()) {
        std::cout << std::format("prev value: {}\n", p->value);
    }

    boost::shared_ptr<void> v = a;
    auto&& back = boost::static_pointer_cast<Node>(v);
}

}
