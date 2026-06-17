// clang++.exe -std=c++26 -O0 -fsanitize=address .\demo_string_view.cpp -o .\demo_string_view.exe
// компилируя с -fsanitize=address, нужно либо копировать clang_rt.asan_dynamic-x86_64.dll рядом с .exe,
// либо один раз добавить путь к нему в PATH: $env:PATH += ";C:\Program Files\LLVM\lib\clang\22\lib\windows"

#include <iostream>
#include <format>
#include <string>
#include <string_view>
#include <coroutine>
#include <utility> 

// Task, чей promise ХРАНИТ string_view (ссылку на чужие данные)
struct Task {
    struct promise_type {
        std::string_view stored;
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Task(std::coroutine_handle<promise_type> _h): h{_h} {}
    Task(Task&& _other) noexcept: h{std::exchange(_other.h, {})} {}
    ~Task() { if(h) h.destroy(); }
    void resume() { h.resume(); }
    std::string_view stored() const { return h.promise().stored; }
};


// Корутина принимает string_view ПО ЗНАЧЕНИЮ, но string_view сам по себе
// это лишь указатель+длина: он НЕ продлевает жизнь строки, на которую смотрит.
Task processor(std::string_view _sv) {
    std::cout << std::format("[before suspend] sv: {}\n", _sv); // тут ещё живо
    co_await std::suspend_always();
    // ... к моменту возобновления временная строка может быть мертва ...
    std::cout << std::format("[after suspend] sv: {}\n", _sv);// читаем висячий sv
    co_return;
}

int main() {
    // Передаём string_view на ВРЕМЕННУЮ строку.
    // Временная std::string живёт только до конца ЭТОГО выражения (точка с запятой),
    // а корутина приостановилась внутри и переживёт её.

    std::string_view s0{"temporary 0"};
    Task t0 = processor(s0);
    Task t1 = processor(std::string{"temporary 1"}); // здесь std::string - временный объект

    std::cout << "resuming...\n";
    t0.resume();
    t1.resume();

    // <-- временная std::string УЖЕ уничтожена здесь, но корутина приостановлена
    std::cout << "resuming...\n";
    t0.resume();
    t1.resume(); // возобновляем -> sv смотрит на освобождённую память -> UB

    return 0;
}
