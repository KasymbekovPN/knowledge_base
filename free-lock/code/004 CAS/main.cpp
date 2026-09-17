
#include <iostream>
#include <format>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

namespace {

    template <typename T>
    class TreiberStack {
        struct Node {
            T data;
            std::shared_ptr<Node> next;
            explicit Node(T data) : data{std::move(data)} {}
        };

        std::atomic<std::shared_ptr<Node>> head;

    public:
        void push(T value) {
            auto new_value{std::make_shared<Node>(std::move(value))};
            new_value->next = head.load(std::memory_order_relaxed);
            while (!head.compare_exchange_weak(
                new_value->next,
                new_value,
                std::memory_order_acq_rel,
                std::memory_order_relaxed)) {}
        }

        bool pop(T& result) {
            auto old_value{head.load(std::memory_order_acquire)};
            while (old_value && !head.compare_exchange_weak(
                old_value,
                old_value->next,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {}

            if (!old_value) return false;

            result = std::move(old_value->data);
            return true;
        }
    };

    void producer_worker(const int id, TreiberStack<int>& stack) {
        stack.push(id);
    }

    void consumer_worker(const int id, TreiberStack<int>& stack) {
        if (int value{};
            stack.pop(value)) {
            std::cout << std::format("{} ", value);
        }
    }
}

int main() {
    constexpr int NUM_THREADS{10};

    TreiberStack<int> stack;

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS * 2);

    for (int i{}; i < NUM_THREADS * 2; ++i) {
        if (i % 2 == 0)
            threads.emplace_back(producer_worker, i, std::ref(stack));
        else
            threads.emplace_back(consumer_worker, i, std::ref(stack));
    }

    for (auto& thread : threads) thread.join();

    std::cout << "\nDone";

    return 0;
}
