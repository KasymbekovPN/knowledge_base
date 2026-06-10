#include <iostream>
#include <thread>

int main() {
    int x{};
    
    // Error
    // std::thread t0 {[x]() {
    //     ++x;
    // }};

    std::thread t1 {[x]() mutable {
        ++x;
        std::cout << x << std::endl;
    }};
    t1.join();
    
    return 0;
}
