#pragma once

#include <atomic>
#include <new>

template <typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

    alignas(std::hardware_destructive_interference_size)
        std::atomic<size_t> write_pos_{0};
    alignas(std::hardware_destructive_interference_size)
        std::atomic<size_t> read_pos_{0};

    T buffer_[Capacity]{};

public:
    [[nodiscard]] bool push(T value) {
        const size_t w{write_pos_.load(std::memory_order_relaxed)};
        if (const size_t r{read_pos_.load(std::memory_order_acquire)};
            w - r >= Capacity) {
            return false;
        }
        buffer_[w & (Capacity - 1)] = std::move(value);
        write_pos_.store(w + 1, std::memory_order_release);

        return true;
    }

    [[nodiscard]] bool pop(T& result) {
        const size_t r{read_pos_.load(std::memory_order_relaxed)};
        if (const size_t w{write_pos_.load(std::memory_order_acquire)};
            w == r) {
            return false;
        }

        result = std::move(buffer_[r & (Capacity - 1)]);
        read_pos_.store(r + 1, std::memory_order_release);

        return true;
    }
};
