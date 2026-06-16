// clang++.exe -std=c++26 -O0 -fsanitize=address .\demo.cpp -o .\demo.exe
// компилируя с -fsanitize=address, нужно либо копировать clang_rt.asan_dynamic-x86_64.dll рядом с .exe,
// либо один раз добавить путь к нему в PATH: $env:PATH += ";C:\Program Files\LLVM\lib\clang\22\lib\windows"
#include <iostream>
#include <format>
#include <coroutine>
#include <string>
#include <utility>

struct Generator {
    struct promise_type {
        int current;
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(int _value) noexcept {
            current = _value;
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;

    explicit Generator(std::coroutine_handle<promise_type> _h): h{_h} {}
    Generator(Generator&& _other)  noexcept: h{std::exchange(_other.h, {})} {}
    ~Generator() { if(h) h.destroy(); }

    bool next() {
        h.resume();
        return !h.done();
    }
    int value() const { return h.promise().current; }
};

static void drain(Generator& _generator, const char* _label) {
    std::cout << std::format("{}: ", _label);
    int guard{};
    while (_generator.next() && guard++ < 10) {
        std::cout << std::format("{} ", _generator.value());
    }
    std::cout << "\n";
}

Generator bad_byref(int _limit) {
    auto lambda = [&]() -> Generator {
        for (int i{}; i < _limit; ++i) {
            co_yield i;
        }
    };
    return lambda();
}

Generator bad_byval(int _limit) {
    auto lambda = [&]() -> Generator {
        for (int i{}; i < _limit; ++i) {
            co_yield i;
        }
    };
    return lambda();
}

Generator good_param(int _limit) {
    for (int i{}; i < _limit; i++) {
        co_yield i;
    }
}

void good_local_scope() {
    int limit{3};
    auto lambda = [limit]() -> Generator {
        for (int i{}; i < limit; i++) {
            co_yield i;
        }
    };
    Generator gen = lambda();
    drain(gen, "[ex3][OK] local scope");
}

int main(int argc, char const *argv[]) {
    int which_one = (argc > 1) ? std::stoi(argv[1]) : 2;

    switch (which_one) {

    case 0: {
        std::cout << "ex0 (UB: capture by reference [&])\n";
        auto gen = bad_byref(3);
        drain(gen, "ex0");
        break;
    }

    case 1: {
        std::cout << "ex1 (UB: capture by value [=], closure dies)\n";
        auto gen = bad_byval(3);
        drain(gen, "ex1");
        break;
    }

    case 2: {
        std::cout << "ex2 (OK: by-value parameter in plain coroutine)\n";
        auto gen = good_param(3);
        drain(gen, "ex2");
        break;
    }

    case 3: {
        std::cout << "ex3 (OK: lambda-coroutine used in same scope)\n";
        good_local_scope();
        break;
    }

    default:
        std::cout << "use: ./demo [0|1|2|3]\n";
        break;
    }

    return 0;
}
