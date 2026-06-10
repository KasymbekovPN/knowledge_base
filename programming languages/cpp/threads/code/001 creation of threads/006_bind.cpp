#include <iostream>
#include <thread>
#include <functional>

void print_sum(const int a, const int b) {
    std::cout
        << a << " + "  << b
        << " = " << a + b
        << std::endl;
}

int main() {
    auto&& add10 = std::bind(print_sum, 10, std::placeholders::_1);
    add10(42);

    auto&& task = std::bind(print_sum, 1, 2);
    std::thread t{task};
    t.join();
    
    return 0;
}
