#include <iostream>

#include "user.pb.h"

int main() {
    myapp::User user;
    user.set_id(1);
    user.set_name("Classic macro works");
    std::cout << user.Utf8DebugString();

    return 0;
}
