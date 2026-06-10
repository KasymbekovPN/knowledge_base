#include <iostream>
#include <thread>

int main() {
    int x{};
    int y{};

    std::thread t0 {[&x]() {
        x++;
    }};
    t0.join();

    std::thread t1 {[&]() {
        x++;
        y++;
    }};
    t1.join();

    std::cout << "x: " << x << ", y: " << y << std::endl;

    // UB
    // std::thread t2;
    // {
    //     int z{42};
    //     t2 = std::thread([&z]() {
    //         z++;
    //         std::cout << "z: " << z << std::endl;
    //     });
    // }
    // t2.join();
    
    return 0;
}
