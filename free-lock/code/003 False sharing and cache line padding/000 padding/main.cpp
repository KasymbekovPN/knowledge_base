
#include <atomic>
#include <chrono>
#include <iostream>
#include <new>
#include <thread>
#include <vector>

namespace {
    constexpr int NUM_THREADS{4};
    constexpr int NUM_ITERATION{1'000'000};

    std::atomic<long> counters_non_padded[NUM_THREADS];
    void worker(const int id) {
        for (int i{}; i < NUM_ITERATION; ++i) {
            counters_non_padded[id].fetch_add(1, std::memory_order_relaxed);
        }
    }

    struct alignas(64) PaddedCounter {
        std::atomic<long> value{0};
    };
    PaddedCounter padded_counters[NUM_THREADS];
    void worker_padded_counter(const int id) {
        for (int i{}; i < NUM_ITERATION; ++i) {
            padded_counters[id].value.fetch_add(1, std::memory_order_relaxed);
        }
    }

    struct alignas(std::hardware_destructive_interference_size) PaddedCounter1 {
        std::atomic<long> value{0};
    };
    PaddedCounter1 padded_counters_1[NUM_THREADS];
    void worker1(const int id) {
        for (int i{}; i < NUM_ITERATION; ++i) {
            padded_counters_1[id].value.fetch_add(1, std::memory_order_relaxed);
        }
    }

    template <typename Fn>
    void run_and_measure(const char* label, Fn&& fn) {
        std::vector<std::thread> threads;
        threads.reserve(NUM_THREADS);

        const auto t0{std::chrono::steady_clock::now()};
        for (int i{}; i < NUM_THREADS; ++i) {
            threads.emplace_back(fn, i);
        }
        for (auto& t : threads) t.join();
        const auto t1{std::chrono::steady_clock::now()};

        std::cout << label << ": " << (t1 - t0) << std::endl;
    }
}

int main() {
    run_and_measure("non-padded       ", worker);
    run_and_measure("padded (64)      ", worker_padded_counter);
    run_and_measure("padded (hw_iface)", worker1);

    return 0;
}
