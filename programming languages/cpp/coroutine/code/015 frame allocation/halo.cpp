#include <iostream>
#include <format>
#include <coroutine>
#include <new>
#include <utility>

static long g_alloc_count{};

void* operator new(std::size_t _n) {
    g_alloc_count++;
    if (void* p = std::malloc(_n)) return p;

    throw std::bad_alloc();
}
void operator delete(void* _p) noexcept { std::free(_p); }
void operator delete(void* _p, std::size_t) noexcept { std::free(_p); }

struct Task {
    struct promise_type {
        std::coroutine_handle<> continuation{};
        int result{};

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }

        // symmetric transfer обратно к ожидающему — нужно для inlining/HALO
        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<promise_type> _h
            ) noexcept {
                auto c = _h.promise().continuation;
                return c ? c : std::noop_coroutine();
            }
            void await_resume() noexcept {}
        };
        FinalAwaiter final_suspend() noexcept { return {};}
        void return_value(int _value) { result = _value; }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if (h) h.destroy(); }

    // делает Task awaitable
    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> _caller) noexcept {
        h.promise().continuation = _caller;
        return h; // symmetric transfer к вложенной корутине
    }

    int await_resume() noexcept { return h.promise().result; }
};

// Вложенная корутина — кандидат на HALO: создаётся и сразу же co_await-ится
Task co_inner(int _input) {
    co_return _input * 2;
}

Task co_outer() {
    int a{co_await co_inner(21)}; // inner создаётся, используется и завершается тут
    int b{co_await co_inner(a)}; // ещё одна вложенная

    co_return b;
}

int main() {
    {
        Task t = co_outer();
        t.h.resume(); // запускаем всю цепочку
        std::cout << std::format("result = {}\n", t.h.promise().result);
    }
    // сколько всего аллокаций? outer + 2x inner = до 3, но часть может быть устранена
    std::cout << std::format("allocs = {}\n", g_alloc_count);

    return 0;
}
