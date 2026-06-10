#include <iostream>
#include <thread>

void print_sum(const int x, const int y) {
    std::cout
        << x << " + " << y
        << " = " << (x + y) 
        << std::endl;
}

int main() {
    std::thread t{print_sum, 1, 42};
    t.join();
    
    return 0;
}
