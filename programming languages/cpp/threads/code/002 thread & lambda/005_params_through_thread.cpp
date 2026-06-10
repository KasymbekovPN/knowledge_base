#include <iostream>
#include <thread>

int main() {
    std::thread t{
        [](const int a, const int b) {
            std::cout << a + b << std::endl;
        },
        10,
        20
    };
    t.join();
    
    return 0;
}
