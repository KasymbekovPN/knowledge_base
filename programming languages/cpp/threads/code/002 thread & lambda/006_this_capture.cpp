#include <iostream>
#include <thread>

struct Worker {
    int value{42};

    void run0() {
        std::thread t{[this]() {
            std::cout << "run0 " << value << std::endl;
        }};
        t.join();
    }

    void run1() {
        std::thread([this]() {
            std::cout << "run1 " << value << std::endl;
        }).detach();
    }
};

int main() {
    Worker w0;
    w0.run0();

    {
        Worker w1;
        w1.run1();
    }
    
    return 0;
}
