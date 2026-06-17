#include <iostream>
#include <format>
#include <coroutine>
#include <new>
#include <utility>

// Глобальный счётчик аллокаций через перехват operator new/delete
static long g_alloc_count{};
static long g_alloc_bytes{};

void* operator new(std::size_t _n) {
    g_alloc_count++;
    g_alloc_bytes += _n;
    if (void* p = std::malloc(_n)) return p;
    throw std::bad_alloc{};
}
void operator delete(void* _p) noexcept { std::free(_p); }
void operator delete(void* _p, std::size_t) noexcept { std::free(_p); }

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if (h) h.destroy(); }
};

Task coro(int _input) {
    int local = _input * 2; // локальные переменные -> часть coroutine frame
    co_await std::suspend_always();
    std::cout << std::format("local: {}\n", local);

    co_return;
}

int main() {
    std::cout << std::format("before: allocs = {}, bytes = {}\n", g_alloc_count, g_alloc_bytes);
    {
        Task t = coro(21); // СОЗДАНИЕ корутины -> аллокация frame
        std::cout << std::format("after create: allocs = {}, bytes = {}\n", g_alloc_count, g_alloc_bytes);
        t.h.resume();
        t.h.resume(); // довести до конца
    }
    std::cout << std::format("after: allocs = {}, bytes = {}\n", g_alloc_count, g_alloc_bytes);

    return 0;
}
