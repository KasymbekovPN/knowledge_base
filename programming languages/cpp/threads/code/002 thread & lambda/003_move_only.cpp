#include <iostream>
#include <thread>

int main() {
    auto&& ptr = std::make_unique<int>(42);

    // std::thread t0 {[ptr]() {}}; // Error
    std::thread t1 {[p = std::move(ptr)]() {
        std::cout << *p << std::endl;
    }};
    t1.join();
    
    return 0;
}
