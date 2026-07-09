#include <coroutine>
#include <optional>
#include <iostream>
#include <format>

// Простой Generator<T> — типичный пример из твоих заметок по корутинам
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T _value) {
            current_value = _value;
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    explicit Generator(handle_type _coro) : coro(_coro) {}
    ~Generator() { if (coro) coro.destroy(); }
    Generator(Generator&&) = delete;
    Generator(const Generator&) = delete;

    bool next() {
        coro.resume();
        return !coro.done();
    }

    T value() const { return coro.promise().current_value; }
};

// Генератор чисел Фибоначчи — суспендится на каждом yield
Generator<long long> fibonacci(int _count) {
    long long a{0}, b{1};
    for (int i{}; i < _count; ++i) {
        co_yield a;
        const long long next{a + b};
        a = b;
        b = next;
    }
}

int main() {
    auto gen = fibonacci(10);
    long long sum{0};
    while (gen.next()) {
        long long v{gen.value()};
        std::cout << std::format("{} ", v);
        sum += v;
    }
    std::cout << std::format("\nSum: {}", sum);

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 47
run
breakpoint list
thread backtrace
frame variable
print __promise

###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:47
run
bt
info locals
print __promise

 */
