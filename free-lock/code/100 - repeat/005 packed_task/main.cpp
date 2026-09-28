#include <future>
#include <iostream>
#include <format>
#include <atomic>

namespace {
    std::atomic<bool> flag{false};
    std::packaged_task<int(int, int)> ptask;
    std::future<int> f;
}

int main(int argc, char *argv[]) {

    std::thread creation_thread{[] {
        ptask = std::packaged_task<int(int, int)>([](const int a, const int b) { return a + b; });
        f = ptask.get_future();
        flag.store(true, std::memory_order_release);
    }};

    while (!flag.load(std::memory_order_acquire)) {}
    std::thread t{std::move(ptask), 1, 2};

    t.join();
    creation_thread.join();

    std::cout << std::format("result: {}\n", f.get());

    return 0;
}
