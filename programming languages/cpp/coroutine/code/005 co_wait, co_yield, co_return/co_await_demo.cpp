#include <iostream>
#include <format>
#include <coroutine>
#include <optional>
#include <utility>
#include <thread>
#include <future>

struct DetachedTask {
    struct promise_type {
        DetachedTask get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; } // eager
        std::suspend_never final_suspend() noexcept { return {}; } // selfdestroy
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

template<typename T>
struct Task {
    struct promise_type {
        std::optional<T> result;
        std::coroutine_handle<> continuation;

        Task get_return_object() {
            return Task{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        // lazy model
        std::suspend_always initial_suspend() noexcept { return {}; }

        // when finished, we resume the one who co_awaited us
        auto final_suspend() noexcept {
            struct FinalAnswer {
                bool await_ready() noexcept { return false; }
                std::coroutine_handle<> await_suspend(
                    std::coroutine_handle<promise_type> h
                ) noexcept {
                    auto cont = h.promise().continuation;
                    return cont ? cont : std::noop_coroutine();
                }
                void await_resume() noexcept {};
            };
            return FinalAnswer{};
        }

        void return_value(T v)  { result = std::move(v); }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if(h) h.destroy(); }

    // making task awaitable - may use co_await
    bool await_ready() noexcept { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
        h.promise().continuation = caller;
        return h;
    }

    T await_resume() { return std::move(*h.promise().result); }
};

template<typename T>
T sync_wait(Task<T> _task) {
    std::promise<T> p;
    std::future<T> f = p.get_future();

    auto&& runner = [&]() -> DetachedTask {
        p.set_value(co_await _task);
    }();

    f.wait();
    return f.get();
}

Task<int> compute() {
    co_return 42;
}

int main() {
    int result{};
    {
        [&]() {
            return std::jthread{[task = compute(), &result]() mutable {
                result = sync_wait(std::move(task));
            }};
        }();
    }
    std::cout << std::format("result: {}\n", result);

    return 0;
}