#include <future>
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include <format>

using namespace std::chrono_literals;

int main(int argc, char *argv[]) {
    std::atomic<bool> flag{true};

    std::thread t{[&flag]() {
        std::this_thread::sleep_for(100ms);
        std::cout << "flag switched off\n";
        flag.store(false, std::memory_order_release);
    }};

    long count{0};
    std::future_status st{};
    std::future<int> f{std::async(std::launch::deferred, [] { return 1; })};
    while (flag.load(std::memory_order_acquire)) {
        st = f.wait_for(10ms);
        ++count;
        if (st == std::future_status::ready) break;
    }

    std::cout << std::format("[ready] count= {}, is deferred= {}\n",
        count,
        st == std::future_status::deferred ? "YES" : "NO");

    t.join();

    return 0;
}
