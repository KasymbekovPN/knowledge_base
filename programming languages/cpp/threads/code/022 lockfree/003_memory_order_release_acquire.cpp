#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<bool> ready{};
    int data{};

    {
        std::jthread producer{[&]() {
            data = 42;
            ready.store(true, std::memory_order_release);
        }};
        std::jthread consumer{[&]() {
            while (!ready.load(std::memory_order_acquire));
            std::cout << std::format("data: {}\n", data);
        }};
    }

    return 0;
}
