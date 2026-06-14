#include <coroutine>
#include <iostream>
#include <format>

template <typename T>
struct Task {
    struct promise_type {
        T value;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never  initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()  noexcept { return {}; }
        void return_value(T v) { value = v; }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> handle) : h(handle) {}
    ~Task() { if (h) h.destroy(); }

    T get() { return h.promise().value; }
};

Task<int> compute() {
    co_return 42;
}

int main() {
    Task<int> task = compute();
    std::cout << std::format("Result: {}\n", task.get());

    return 0;
}
