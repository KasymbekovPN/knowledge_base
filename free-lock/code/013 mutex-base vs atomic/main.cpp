#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <vector>
#include <chrono>
#include <deque>
#include <mutex>
#include <cassert>
#include <algorithm>
#include <numeric>

namespace {
    // ============================================================
    // MPMC Vyukov (тот же код, что и раньше)
    // ============================================================
    template <typename T>
    class MPMCQueue {
        struct Cell {
            std::atomic<size_t> sequence;
            T data;
        };

        alignas(std::hardware_destructive_interference_size) std::atomic<size_t> enqueue_pos_;
        alignas(std::hardware_destructive_interference_size) std::atomic<size_t> dequeue_pos_;

        Cell* buffer_;
        size_t buffer_mask_;

    public:
        explicit MPMCQueue(const size_t capacity):
            buffer_(new Cell[capacity]),
            buffer_mask_(capacity - 1) {

            for (size_t i{}; i < capacity; ++i) {
                buffer_[i].sequence.store(i, std::memory_order_relaxed);
            }
            enqueue_pos_.store(0, std::memory_order_relaxed);
            dequeue_pos_.store(0, std::memory_order_relaxed);
        }
        ~MPMCQueue() { delete[] buffer_; }

        bool push(T value) {
            Cell* cell;
            size_t pos{enqueue_pos_.load(std::memory_order_relaxed)};
            for (;;) {
                cell = &buffer_[pos & buffer_mask_];
                const size_t seq{cell->sequence.load(std::memory_order_acquire)};
                if (const intptr_t dif{static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos)};
                    dif == 0)
                {
                    if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                        break;
                } else if (dif < 0) return false;
                else {
                    pos = enqueue_pos_.load(std::memory_order_relaxed);
                }
            }

            cell->data = std::move(value);
            cell->sequence.store(pos + 1, std::memory_order_release);

            return true;
        }

        bool pop(T& result) {
            Cell* cell;
            size_t pos{dequeue_pos_.load(std::memory_order_relaxed)};
            for (;;) {
                cell = &buffer_[pos & buffer_mask_];
                const size_t seq{cell->sequence.load(std::memory_order_acquire)};
                if (const intptr_t dif{static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1)};
                    dif == 0)
                {
                    if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
                        break;
                } else if (dif < 0) return false;
                else {
                    pos = dequeue_pos_.load(std::memory_order_relaxed);
                }
            }

            result = std::move(cell->data);
            cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);

            return true;
        }
    };

    // ============================================================
    // Mutex-based аналог -- ТОТ ЖЕ интерфейс push/pop
    // ============================================================
    template <typename T>
    class MutexQueue {
        mutable std::mutex mutex_;
        std::deque<T> deque_;
        size_t capacity_;

    public:
        explicit MutexQueue(const size_t capacity): capacity_(capacity) {}

        bool push(T value) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (deque_.size() >= capacity_) return false;
            deque_.push_back(std::move(value));

            return true;
        }

        bool pop(T& result) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (deque_.empty()) return false;

            result = std::move(deque_.front());
            deque_.pop_front();

            return true;
        }
    };

    // ============================================================
    // Общий бенчмарк-харнесс: throughput + latency percentiles
    // ============================================================
    struct BenchResult {
        double throughput_ops_sec;
        double p50_ns;
        double p99_ns;
        double p999_ns;
    };

    template <typename Q>
    BenchResult run_bench(const int num_producers,
                          const int num_consumers,
                          const int items_per_producer,
                          const size_t capacity) {

        Q queue{capacity};
        const int total{num_producers * items_per_producer};

        std::atomic<int> consumed{0};
        std::atomic<long long> checksum_in{0};
        std::atomic<long long> checksum_out{0};

        std::vector<std::vector<long long>> latencies_per_thread(num_producers);
        for (auto& v: latencies_per_thread) v.reserve(items_per_producer);

        const auto start{std::chrono::steady_clock::now()};
        std::vector<std::thread> producers;
        for (int p{}; p < num_producers; ++p) {
            producers.emplace_back([&, p] {
                auto& lat = latencies_per_thread[p];
                for (int i{}; i < items_per_producer; ++i) {
                    const int value{p * items_per_producer + i};
                    const auto t0{std::chrono::steady_clock::now()};
                    while (!queue.push(value)) std::this_thread::yield();
                    const auto t1{std::chrono::steady_clock::now()};
                    lat.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
                    checksum_in.fetch_add(value, std::memory_order_relaxed);
                }
            });
        }

        std::vector<std::thread> consumers;
        for (int c{}; c < num_consumers; ++c) {
            consumers.emplace_back([&] {
                int value;
                while (consumed.load(std::memory_order_relaxed) < total) {
                    if (queue.pop(value)) {
                        checksum_out.fetch_add(value, std::memory_order_relaxed);
                        consumed.fetch_add(1, std::memory_order_relaxed);
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }

        for (auto& t: producers) t.join();
        for (auto& t: consumers) t.join();

        const auto end{std::chrono::steady_clock::now()};
        const auto sec{std::chrono::duration<double>(end - start).count()};

        assert(checksum_in.load() == checksum_out.load() && "checksum mismatch!");

        std::vector<long long> all_lat;
        for (auto& v: latencies_per_thread) {
            all_lat.insert(all_lat.end(), v.begin(), v.end());
        }
        std::ranges::sort(all_lat);
        const auto percentile{[&](const double p) -> double {
            const size_t idx{static_cast<size_t>(p * static_cast<double>(all_lat.size() - 1))};
            return static_cast<double>(all_lat[idx]);
        }};

        BenchResult r{};
        r.throughput_ops_sec = total / sec;
        r.p50_ns = percentile(0.50);
        r.p99_ns = percentile(0.99);
        r.p999_ns = percentile(0.999);

        return r;
    }

    void print_result(const std::string& name, const BenchResult& r) {
        std::cout << std::format("{}:\n  throughput: {} pos/sec\n  latency p50: {} ns\n  latency p99: {} ns\n  latency p999: {} ns\n",
            name,
            static_cast<long long>(r.throughput_ops_sec),
            r.p50_ns,
            r.p99_ns,
            r.p999_ns);
    }
}

int main() {
    constexpr size_t CAPACITY{4096};
    constexpr int ITEMS{200'000};

    const std::vector<std::pair<int, int>> v{std::pair{1,1}, std::pair{2,2}, std::pair{4,4}, std::pair{8,8}};
    for (auto& [producer, consumer] : v) {
        std::cout << std::format("\n===== producers = {}, consumers = {} =====\n", producer, consumer);
        const auto lock_free{run_bench<MPMCQueue<int>>(
            producer,
            consumer,
            ITEMS,
            CAPACITY)};
        const auto mutex_based{run_bench<MutexQueue<int>>(
            producer,
            consumer,
            ITEMS,
            CAPACITY)};
        print_result("MPMC (lock-free)", lock_free);
        print_result("Mutex + deque", mutex_based);
        std::cout << std::format("  speedup (throughput): {}x\n",
            lock_free.throughput_ops_sec / mutex_based.throughput_ops_sec);
    }

    return 0;
}
