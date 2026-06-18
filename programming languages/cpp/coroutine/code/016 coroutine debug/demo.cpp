#include <iostream>
#include <format>
#include <utility>
#include <coroutine>

struct Task {
    struct promise_type {
        int value{};

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _handle): h{_handle} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if (h) h.destroy(); }
};

Task counting_core(int _start) {
    int x{_start}; // локальная переменная корутины
    std::cout <<std::format("step 0: x = {}\n", x);
    co_await std::suspend_always{}; // <-- точка приостановки A

    x += 10;
    std::cout <<std::format("step 1: x = {}\n", x);
    co_await std::suspend_always{}; // <-- точка приостановки B

    x += 100;
    std::cout <<std::format("step 2: x = {}\n", x);

    co_return;
}

int main() {
    Task t = counting_core(42);
    while (!t.h.done()) {
        t.h.resume();
    }

    return 0;
}
