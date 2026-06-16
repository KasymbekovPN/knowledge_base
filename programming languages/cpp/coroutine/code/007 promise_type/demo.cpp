#include <iostream>
#include <coroutine>

struct Coro {
    struct promise_type {
        Coro get_return_object() { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

Coro hello() {
    std::cout << "before\n";
    co_return;
}

int main() {
    hello();
    std::cout << "after\n";

    return 0;
}
