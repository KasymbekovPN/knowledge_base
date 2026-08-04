#include <iostream>

[[deprecated("use new_func instead")]]
static void old_func() { std::cout << "old_func" << std::endl; }

static void new_func() { std::cout << "new_func" << std::endl; }

int main() {
    old_func();
    new_func();

    return 0;
}
