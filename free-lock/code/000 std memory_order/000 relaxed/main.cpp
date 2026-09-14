
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

namespace {
    std::atomic<int> counter{0};

    void increment() {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {

    std::vector<std::thread> threads;
    for (int i{}; i < 50; ++i) {
        threads.emplace_back(increment);
    }

    for (auto& t: threads) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << counter.load(std::memory_order_relaxed) << std::endl;

    return 0;
}
