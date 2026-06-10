#include <iostream>
#include <thread>

int main() {
    int x{1};
    int y{2};

    std::thread t0 {[x]() {
        std::cout << "x: " << x << std::endl;
    }};
    t0.join();

    std::thread t1 {[=]() {
        std::cout << "x: " << x << ", y: " << y << std::endl;
    }};
    t1.join();
    
    return 0;
}
