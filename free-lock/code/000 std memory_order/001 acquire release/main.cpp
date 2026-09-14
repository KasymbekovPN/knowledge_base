
#include <atomic>
#include <iostream>
#include <string>
#include <thread>

namespace {

    std::string payload;
    std::atomic<bool> ready{false};

    void producer() {
        payload = "hello world";
        ready.store(true, std::memory_order_release);
    }

    void consumer() {
        while (!ready.load(std::memory_order_acquire)) {}
        std::cout << payload << std::endl;
    }
}

int main() {

    {
        auto t0{std::jthread(producer)};
        auto t1{std::jthread(consumer)};
    }

    return 0;
}
