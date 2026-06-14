#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<bool> x{};
    std::atomic<bool> y{};
    std::atomic<int> z{};
    {
        std::jthread t0{[&](){ x.store(true, std::memory_order_seq_cst); }};
        std::jthread t1{[&](){ y.store(true, std::memory_order_seq_cst); }};

        std::jthread t2{[&](){
            while(!x.load(std::memory_order_seq_cst));
            if (y.load(std::memory_order_seq_cst)) {
                ++z;
                std::cout << std::format("T2 z: {}\n", z.load());
            }
        }};

        std::jthread t3{[&](){
            while(!y.load(std::memory_order_seq_cst));
            if (x.load(std::memory_order_seq_cst)) {
                ++z;
                std::cout << std::format("T3 z: {}\n", z.load());
            }
        }};
    }
    std::cout << std::format("z: {}\n", z.load());

    return 0;
}
