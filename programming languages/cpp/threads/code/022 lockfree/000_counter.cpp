#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> counter{};

void increment(int _n) {
    for (int i{}; i < _n; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    {
        std::jthread t0{increment, 1000};
        std::jthread t1{increment, 1000};
    }

    std::cout << "counter: " << counter << "\n";

    return 0;
}
