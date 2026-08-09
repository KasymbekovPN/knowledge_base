#include <iostream>
#include "orders/order.pb.h"

int main() {
    myapp::orders::Order order;
    order.set_order_id(1);
    order.mutable_buyer()->set_name("Alice");
    order.mutable_buyer()->mutable_address()->set_city("Berlin");
    std::cout << order.Utf8DebugString();

    return 0;
}
