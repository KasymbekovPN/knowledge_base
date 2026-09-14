
#include <atomic>
#include <iostream>
#include <thread>

namespace {
    std::atomic<int> x{0}, y{0};
    int r1, r2;

    // !!! С seq_cst НЕВОЗМОЖНО r1 == 0 && r2 == 0 одновременно

    void thread_handler_1() {
        x.store(1, std::memory_order_seq_cst);
        r1 = y.load(std::memory_order_seq_cst);
        std::cout << std::format("r1 => {}\n", r1);
    }

    void thread_handler_2() {
        y.store(1, std::memory_order_seq_cst);
        r2 = x.load(std::memory_order_seq_cst);
        std::cout << std::format("r2 => {}\n", r2);
    }
}

int main() {

    {
        auto t1{std::jthread(thread_handler_1)};
        auto t2{std::jthread(thread_handler_2)};
    }

    return 0;
}
