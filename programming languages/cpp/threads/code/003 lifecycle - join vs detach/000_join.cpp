#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Main started" << std::endl;

    std::thread t{[]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Thread done" << std::endl;
    }};
    t.join();

    std::cout << "Main finished" << std::endl;
    
    return 0;
}
