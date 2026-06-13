#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<int*> ptr{nullptr};
    int data{};

    {
        std::jthread producer{[&]() {
            data = 100;
            ptr.store(new int{42}, std::memory_order_release);
        }};
        std::jthread consumer{[&]() {
            int* p{nullptr};
            while(!(p = ptr.load(std::memory_order_consume)));
            std::cout << std::format("*ptr: {}, data: {}", *p, data);
            delete p;
        }};
    }

    return 0;
}
