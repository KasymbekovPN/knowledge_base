#include <iostream>
#include <format>
#include <coroutine>
#include <cstdint>
#include <utility>

template <typename T>
struct Generator {
    struct promise_type {
        T current;

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {};}
        std::suspend_always yield_value(T v) noexcept {
            current = v;
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Generator(std::coroutine_handle<promise_type> _h): h{_h} {}
    Generator(Generator&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Generator() { if (h) h.destroy(); }

    bool next() {
        h.resume();
        return !h.done();
    }
    T value() const { return h.promise().current; }

};

Generator<uint64_t> fibonacci() {
    uint32_t a = 0, b = 1;
    while (true) {
        co_yield a;
        uint64_t next = a + b;
        a = b;
        b = next;
    }
}

int main() {
    auto&& gen = fibonacci();
    for (size_t i{}; i < 10; i++) {
        gen.next();
        std::cout << std::format("{} ", gen.value());
    }
    std::cout << "\n";

    return 0;
}
