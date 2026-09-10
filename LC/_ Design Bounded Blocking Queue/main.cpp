#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <array>
#include <atomic>
#include <format>

namespace {

    template <typename T>
    class BoundedBlockingQueue {
        std::queue<T> buffer;
        size_t capacity;
        std::mutex mtx;
        std::condition_variable not_full;
        std::condition_variable not_empty;

    public:
        explicit BoundedBlockingQueue(const size_t capacity) : capacity{capacity} {}

        void enqueue(const T item) {
            std::unique_lock<std::mutex> lock{mtx};
            not_full.wait(lock, [this] { return buffer.size() < capacity; });
            buffer.push(std::move(item));
            not_empty.notify_one();
        }

        T dequeue() {
            std::unique_lock<std::mutex> lock{mtx};
            not_empty.wait(lock, [this] { return !buffer.empty(); });
            T item = std::move(buffer.front());
            buffer.pop();
            not_full.notify_one();

            return item;
        }

        size_t size() {
            std::lock_guard<std::mutex> lock{mtx};
            return buffer.size();
        }
    };

    template <typename T, size_t CAPACITY>
    class SPSCRingBuffer {
        std::array<T, CAPACITY> buffer;
        std::atomic<size_t> head{0}; // индекс записи, трогает только producer
        std::atomic<size_t> tail{0}; // индекс чтения, трогает только consumer

        public:
            bool try_enqueue(const T& item) {
                const size_t current_head{head.load(std::memory_order_relaxed)};
                const size_t next_head{(current_head + 1) % CAPACITY};
                if (next_head == tail.load(std::memory_order_acquire)) {
                    return false;
                }

                buffer[current_head] = item;
                head.store(next_head, std::memory_order_release);

                return true;
            }

            bool try_dequeue(T& item) {
                const size_t current_tail{tail.load(std::memory_order_relaxed)};
                if (current_tail == head.load(std::memory_order_acquire)) {
                    return false;
                }

                item = buffer[current_tail];
                tail.store((current_tail + 1) % CAPACITY, std::memory_order_release);

                return true;
            }
    };

    void start_test0() {
        BoundedBlockingQueue<int> queue{3};
        std::mutex cout_mtx;

        const auto producer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                const int value{id * 100 + i};
                queue.enqueue(value);
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Producer {} -> enqueue {} (size = {})\n", id, value, queue.size());
                }
            }
        };

        const auto consumer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                const auto value = queue.dequeue();
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Consumer {} -> dequeue {} (size = {})\n", id, value, queue.size());
                }
            }
        };

        std::thread p1(producer, 1, 5);
        std::thread p2(producer, 2, 5);
        std::thread c1(consumer, 1, 5);
        std::thread c2(consumer, 2, 5);

        p1.join();
        p2.join();
        c1.join();
        c2.join();
    }

    void start_test1() {
        SPSCRingBuffer<int, 3> ring_buffer;
        std::mutex cout_mtx;

        const auto producer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                if (const int value{id * 100 + i}; ring_buffer.try_enqueue(value))
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Producer {} -> enqueue {}\n", id, value);
                }
            }
        };

        const auto consumer = [&](const int id, const int count) {
            for (int i{}; i < count; ++i) {
                if (int value; ring_buffer.try_dequeue(value))
                {
                    std::lock_guard<std::mutex> lock{cout_mtx};
                    std::cout << std::format("Consumer {} -> dequeue {}\n", id, value);
                }
            }
        };

        std::thread p1(producer, 1, 5);
        std::thread p2(producer, 2, 5);
        std::thread c1(consumer, 1, 5);
        std::thread c2(consumer, 2, 5);

        p1.join();
        p2.join();
        c1.join();
        c2.join();
    }
}

int main(int argc, char *argv[]) {
    // start_test0();
    start_test1();

    return 0;
}
