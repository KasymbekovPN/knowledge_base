// clang++.exe -std=c++26 -O0 -fsanitize=address .\demo_str_ref.cpp -o .\demo_str_ref.exe
// компилируя с -fsanitize=address, нужно либо копировать clang_rt.asan_dynamic-x86_64.dll рядом с .exe,
// либо один раз добавить путь к нему в PATH: $env:PATH += ";C:\Program Files\LLVM\lib\clang\22\lib\windows"
#include <iostream>
#include <format>
#include <string>
#include <coroutine>
#include <utility>

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    ~Task() { if (h) h.destroy(); }
};


struct BadAwaiter {
    // ССЫЛКА — опасно, если объект временный
    const std::string& ref;
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> _h)  const noexcept {
        _h.resume(); // имитируем приостановку+возобновление
    }
    // к моменту resume временный объект, на который смотрит ref, может быть мёртв
    std::string await_resume() const { return ref; } // читаем висячую ссылку
};

// helper, возвращающий awaiter по ссылке на ВРЕМЕННЫЙ аргумент
BadAwaiter waitOn(const std::string& s) {
    return BadAwaiter{s};
}

Task run() {
    // co_await с ВРЕМЕННЫМ объектом: std::string{"hello"} — prvalue
    std::string result = co_await waitOn(std::string("temporary"));
    std::cout << std::format("got: {}\n", result);

    co_return;
}

int main() {
    run();

    return 0;
}
