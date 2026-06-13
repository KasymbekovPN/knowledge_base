#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<int> counter{0};

    {
        std::jthread t0{[&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }};
        std::jthread t1{[&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }};
    }
    std::cout << std::format("result: {}\n", counter.load());

    return 0;
}
