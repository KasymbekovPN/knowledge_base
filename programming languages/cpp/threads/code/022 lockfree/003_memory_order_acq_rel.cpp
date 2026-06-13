#include <iostream>
#include <atomic>
#include <thread>
#include <format>

int main() {
    std::atomic<int> flag{};
    int data{};
    int expected{1};

    {
        std::jthread t0{[&]() {
            data = 42;
            flag.fetch_add(expected, std::memory_order_release);
        }};
        std::jthread t1{[&]() {
            if (flag.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) {
                std::cout << std::format("data: {}\n", data);
            }
        }};
    }

    return 0;
}


// // ─────────────────────────────────────────────
// // 5. seq_cst — полный последовательный порядок
// //    все потоки видят операции в одном порядке
// //    используется по умолчанию, самый медленный
// // ─────────────────────────────────────────────
// void example_seq_cst() {
//     std::atomic<bool> x{false};
//     std::atomic<bool> y{false};
//     std::atomic<int>  z{0};

//     std::jthread t0([&]() { x.store(true, std::memory_order_seq_cst); });
//     std::jthread t1([&]() { y.store(true, std::memory_order_seq_cst); });

//     std::jthread t2([&]() {
//         while (!x.load(std::memory_order_seq_cst));
//         if (y.load(std::memory_order_seq_cst)) ++z;
//     });

//     std::jthread t3([&]() {
//         while (!y.load(std::memory_order_seq_cst));
//         if (x.load(std::memory_order_seq_cst)) ++z;
//     });
//     // гарантия: z >= 1 — хотя бы один из t2/t3 увидит оба флага
// }

// int main() {
//     example_relaxed();
//     example_release_acquire();
//     example_consume();
//     example_acq_rel();
//     example_seq_cst();
//     std::cout << "all ok\n";
// }