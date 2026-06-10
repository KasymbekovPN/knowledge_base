#include <iostream>
#include <thread>

int main() {
    std::thread t{[]() {
        std::cout << "In the thread" << std::endl;
    }};
    t.join();
    
    return 0;
}
