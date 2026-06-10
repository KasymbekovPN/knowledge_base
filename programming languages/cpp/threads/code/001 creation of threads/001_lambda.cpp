#include <iostream>
#include <thread>

int main() {
    std::thread t0([]() {
        std::cout << "thread executed" << std::endl;
    });
    std::thread t1([](const int _input) {
        std::cout << "thread executed :: " << _input << std::endl;
    }, 42);

    t0.join();
    t1.join();
    
    return 0;
}
