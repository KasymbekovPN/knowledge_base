#include <iostream>
#include <format>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

#if defined(__x86_64__) || defined(_M_AMD64)
#include <x86intrin.h>
#define SPINLOCK_PAUSE() _mm_pause()
#else
#define SPINLOCK_PAUSE()
#endif

namespace {

    class SpinLock {
        // По стандарту изначально false
        std::atomic_flag flag = ATOMIC_FLAG_INIT;

    public:
        void lock() noexcept {
            for (;;) {
                // Оптимизация TTAS: крутимся в локальном кэше в режиме Read-Only,
                // пока флаг занят (C++20 предоставляет метод .test())
                if (!flag.test(std::memory_order_relaxed)) {
                    // Пытаемся захватить лок атомарной записью
                    if (!flag.test_and_set(std::memory_order_acquire)) return; // Успешно захватили
                    SPINLOCK_PAUSE();
                }
            }
        }

        void unlock() noexcept {
            // Освобождаем с семантикой Release
            flag.clear(std::memory_order_release);
        }
    };

    constexpr int NUM_THREADS{8};
    constexpr int ITERATIONS{1'000'000};
    long long global_counter{0};

    SpinLock spin;

    void worker() {
        for (int i{}; i < ITERATIONS; ++i) {
            spin.lock();
            global_counter++;
            spin.unlock();
        }
    }

}

int main() {
    std::vector<std::thread> threads;
    const auto start{std::chrono::high_resolution_clock::now()};

    for (int i{}; i< NUM_THREADS; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) thread.join();

    const auto end{std::chrono::high_resolution_clock::now()};
    std::chrono::duration<double, std::milli> elapsed{end - start};

    std::cout << std::format("Counter: {}\nElapsed time: {} ms",
        global_counter,
        elapsed.count());

    return 0;
}
