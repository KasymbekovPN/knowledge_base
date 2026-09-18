#include <atomic>
#include <iostream>
#include <format>
#include <thread>
#include <vector>
#include <chrono>
#include <new>
#include <cassert>

namespace {
    // ============================================================
    // Bounded MPMC queue (алгоритм Dmitry Vyukov, 1024cores.net)
    // ============================================================

    template <typename T>
    class MPMCQueue {
        struct Cell {
            std::atomic<size_t> sequence;
            T data{};
        };

        alignas(std::hardware_destructive_interference_size)
            std::atomic<size_t> enqueue_pos_;
        alignas(std::hardware_destructive_interference_size)
            std::atomic<size_t> dequeue_pos_;

        Cell* buffer_;
        size_t buffer_mask_;

    public:
        explicit MPMCQueue(const size_t capacity):
            buffer_{new Cell[capacity]},
            buffer_mask_{capacity - 1} {

            assert((capacity >= 2) && ((capacity & (capacity - 1)) == 0) && "capacity must be 2^n");

            for (size_t i{}; i < capacity; ++i) {
                buffer_[i].sequence.store(i, std::memory_order_relaxed);
            }
            enqueue_pos_.store(0, std::memory_order_relaxed);
            dequeue_pos_.store(0, std::memory_order_relaxed);
        }

        ~MPMCQueue() { delete[] buffer_; }

        MPMCQueue(const MPMCQueue&) = delete;
        MPMCQueue& operator=(const MPMCQueue&) = delete;

        bool enqueue(T value) {
            Cell* cell;
            size_t pos{enqueue_pos_.load(std::memory_order_relaxed)};

            for (;;) {
                cell = &buffer_[pos & buffer_mask_];
                size_t seq{cell->sequence.load(std::memory_order_acquire)};
                intptr_t dif{static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos)};

                if (dif == 0) {
                    if (enqueue_pos_.compare_exchange_weak(
                        pos,
                        pos + 1,
                        std::memory_order_relaxed)) break;
                } else if (dif < 0) {
                    return false; // очередь полна
                } else {
                    pos = enqueue_pos_.load(std::memory_order_relaxed);
                }
            }

            cell->data = std::move(value);
            cell->sequence.store(pos + 1, std::memory_order_release);
            return true;
        }

        bool dequeue(T& result) {
            Cell* cell;
            size_t pos{dequeue_pos_.load(std::memory_order_relaxed)};

            for (;;) {
                cell = &buffer_[pos & buffer_mask_];
                size_t seq{cell->sequence.load(std::memory_order_acquire)};
                intptr_t dif{static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1)};

                if (dif == 0) {
                    if (dequeue_pos_.compare_exchange_weak(
                        pos,
                        pos + 1,
                        std::memory_order_relaxed)) break;
                } else if (dif < 0) {
                    return false; // очередь пуста
                } else {
                    pos = dequeue_pos_.load(std::memory_order_relaxed);
                }
            }

            result = std::move(cell->data);
            cell->sequence.store(pos + buffer_mask_ + 1, std::memory_order_release);
            return true;
        }
    };

}

int main(int argc, char *argv[]) {
    constexpr size_t CAPACITY{1024};
    MPMCQueue<int> queue{CAPACITY};

    constexpr int NUM_PRODUCERS{4};
    constexpr int NUM_CONSUMERS{4};
    constexpr int ITEM_PER_PRODUCER{250'000};
    constexpr int TOTAL{NUM_PRODUCERS * ITEM_PER_PRODUCER};

    std::atomic<long long> checksum_in{0}, checksum_out{0};
    std::atomic<int> consumed{0};

    std::vector<std::thread> producers, consumers;

    for (int p{}; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&, p] {
            for (int i{}; i < ITEM_PER_PRODUCER; ++i) {
                const int value{p * ITEM_PER_PRODUCER + i};
                while (!queue.enqueue(value)) std::this_thread::yield();
                checksum_in.fetch_add(value, std::memory_order_relaxed);
            }
        });
    }

    for (int c{}; c < NUM_CONSUMERS; ++c) {
        consumers.emplace_back([&] {
            int value{};
            while (consumed.load(std::memory_order_relaxed) < TOTAL) {
                if (queue.dequeue(value)) {
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

    const auto cs_in{checksum_in.load()};
    const auto cs_out{checksum_out.load()};
    std::cout << std::format("checksum: in {} / out {} : {}\n",
        cs_in,
        cs_out,
        cs_in == cs_out ? "success" : "fail");

    return 0;
}
